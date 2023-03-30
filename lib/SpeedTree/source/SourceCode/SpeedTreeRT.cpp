///////////////////////////////////////////////////////////////////////
//         Name: SpeedTreeRT.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not
//  be copied or disclosed except in accordance with the terms of that
//  agreement.

#include "Debug.h"
#include "LibGlobals_Source/IdvGlobals.h"
#include "SpeedTreeRT.h"
#include "IndexedGeometry.h"
#include "Instances.h"
#include "TreeEngine.h"
#include "WindEngine.h"
#include "SimpleBillboard.h"
#include "LightingEngine.h"
#include "LeafGeometry.h"
#include "FrondEngine.h"
#include "UpVector.h"
#include "ProjectedShadow.h"
#include <time.h>
#include <algorithm>
#include "FileAccess.h"
#define FULL_VERSION
#include "EvalCode.h"
#include <fstream>

#if defined(linux) || defined(__APPLE__)
template <class T>
inline T __max(T a, T b)
{
	return max(a, b);
}

template <class T>
inline T __min(T a, T b)
{
	return min(a, b);
}
#endif


using namespace std;

// static/global variables
vector<CSpeedTreeRT*> SInstanceList::m_vUniqueTrees;
bool CSpeedTreeRT::m_bDropToBillboard = false;
bool CSpeedTreeRT::m_bTextureFlip = false;
unsigned int CSpeedTreeRT::m_uiAllRefCount = 0;
float CSpeedTreeRT::m_fHorizontalFadeStartAngle = 30.0f;
float CSpeedTreeRT::m_fHorizontalFadeEndAngle = 60.0f;
float CSpeedTreeRT::m_fHorizontalFadeValue = 84.0f;

static string g_strError;
static string g_strKey;

// enumerations
enum ESpeedTreeInternalEvent
{
    STIE_CLIENT_CHANGED_WIND,
    STIE_CLIENT_CHANGED_CAMERA,
    STIE_UNKNOWN
};

enum ESpeedTreeSizes
{
    STS_MIN_BOX     = 0,    // (x, y, z) min of orthographic bounding box
    STS_MAX_BOX     = 3,    // (x, y, z) max of orthographic bounding box
    STS_BB_SIZE     = 6,    // width of simple billboard (last LOD)
    STS_COUNT       = 7
};


///////////////////////////////////////////////////////////////////////
//  SEmbeddedTexCoords definition
//
//  Used to store leaf map texture coordinates from a composite leaf map
//  as well as billboard texture coordinates.

struct SEmbeddedTexCoords
{
        SEmbeddedTexCoords( ) :
            m_nNumLeafMaps(0),
            m_pLeafTexCoords(NULL),
            m_nNumBillboards(0),
            m_pBillboardTexCoords(NULL),
            m_nNumFrondMaps(0),
            m_pFrondTexCoords(NULL)
        {
            m_afShadowTexCoords[0] = 1.0f;
            m_afShadowTexCoords[1] = 1.0f;

            m_afShadowTexCoords[2] = 0.0f;
            m_afShadowTexCoords[3] = 1.0f;

            m_afShadowTexCoords[4] = 0.0f;
            m_afShadowTexCoords[5] = 0.0f;

            m_afShadowTexCoords[6] = 1.0f;
            m_afShadowTexCoords[7] = 0.0f;
        }

        ~SEmbeddedTexCoords( )
        {
            delete[] m_pLeafTexCoords;
            m_pLeafTexCoords = NULL;

            delete[] m_pBillboardTexCoords;
            m_pBillboardTexCoords = NULL;

            delete[] m_pFrondTexCoords;
            m_pFrondTexCoords = NULL;
        }

        const SEmbeddedTexCoords& operator=(const SEmbeddedTexCoords& sRight)
        {
            if (&sRight != this)
            {
                // copy leaf texture coordinates
                if (sRight.m_pLeafTexCoords)
                {
                    m_nNumLeafMaps = sRight.m_nNumLeafMaps;
                    m_pLeafTexCoords = new float[m_nNumLeafMaps * 8];
                    memcpy(m_pLeafTexCoords, sRight.m_pLeafTexCoords, m_nNumLeafMaps * 8 * sizeof(float));
                }
                // copy billboard texture coordinates
                if (sRight.m_pBillboardTexCoords)
                {
                    m_nNumBillboards = sRight.m_nNumBillboards;
                    m_pBillboardTexCoords = new float[m_nNumBillboards * 8];
                    memcpy(m_pBillboardTexCoords, sRight.m_pBillboardTexCoords, m_nNumBillboards * 8 * sizeof(float));
                }
                // copy frond texture coordinates
                if (sRight.m_pFrondTexCoords)
                {
                    m_nNumFrondMaps = sRight.m_nNumFrondMaps;
                    m_pFrondTexCoords = new float[m_nNumFrondMaps * 8];
                    memcpy(m_pFrondTexCoords, sRight.m_pFrondTexCoords, m_nNumFrondMaps * 8 * sizeof(float));
                }

                m_strCompositeFilename = sRight.m_strCompositeFilename;
                memcpy(m_afShadowTexCoords, sRight.m_afShadowTexCoords, 8 * sizeof(float));
            }

            return *this;
        }

        int         m_nNumLeafMaps;             // number of leaf texture coordinate sets
        float*      m_pLeafTexCoords;           // array of leaf texture coordinates
        int         m_nNumBillboards;           // number of billboard texture coordinate sets
        float*      m_pBillboardTexCoords;      // array of billboard texture coordinates

        int         m_nNumFrondMaps;            // number of separate frond texture maps
        float*      m_pFrondTexCoords;          // array of frond texture coordinates

        string      m_strCompositeFilename;     // filename of composite map holding leaf, frond, and billboard textures
        float       m_afShadowTexCoords[8];     // self-shadow tex coords (may or may not index a composite map)
};


///////////////////////////////////////////////////////////////////////
//  SShape definition
//
//  Defines the shapes used for collision objects.

struct SShape
{
        SShape( ) :
            m_eType(CSpeedTreeRT::CO_SPHERE)
        {
            m_afPosition[0] = m_afPosition[1] = m_afPosition[2] = 0.0f;
            m_afDimensions[0] = m_afDimensions[1] = m_afDimensions[2] = 0.0f;
        }

        CSpeedTreeRT::ECollisionObjectType  m_eType;            // CO_SPHERE, CO_CYLINDER, or CO_BOX
        float                               m_afPosition[3];    // center of shape
        float                               m_afDimensions[3];  // dimensions of shape (each type interprets differently)
};


///////////////////////////////////////////////////////////////////////
//  SCollisionObjects definition
//
//  A wrapper used to hide the STL vector type from the application.

struct SCollisionObjects
{
        void            TransformAll(CTransform& cTransform)
        {
            for (vector<SShape>::iterator i = m_vObjects.begin( ); i != m_vObjects.end( ); ++i)
            {
                // transform base point
                CVec3 cPos(i->m_afPosition[0], i->m_afPosition[1], i->m_afPosition[2]);
                cPos = cPos * cTransform;
                i->m_afPosition[0] = cPos[0];
                i->m_afPosition[1] = cPos[1];
                i->m_afPosition[2] = cPos[2];

                // transform the dimensions
                if (i->m_eType == CSpeedTreeRT::CO_SPHERE)
                {
                    // the sphere is always meant to be perfectly round,
                    // so we've chosen to scale the radius arbitrarily by
                    // the x-axis scale factor
                    i->m_afDimensions[0] *= cTransform.m_afData[0][0];
                }
                else if (i->m_eType == CSpeedTreeRT::CO_CYLINDER)
                {
                    // the cylinder collision object was not meant to
                    // lie on anything but the up axis, so an arbitrary
                    // transform can wreak some havoc.
                    i->m_afDimensions[0] *= cTransform.m_afData[0][0];
                    i->m_afDimensions[1] *= cTransform.m_afData[0][0];
                }
                else if (i->m_eType == CSpeedTreeRT::CO_BOX)
                {
                    i->m_afDimensions[0] *= cTransform.m_afData[0][0];
                    i->m_afDimensions[1] *= cTransform.m_afData[1][1];
                    i->m_afDimensions[2] *= cTransform.m_afData[2][2];
                }
            }
        }

        vector<SShape>  m_vObjects;         // collection of all collision objects in tree
};


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry constructor definition

CSpeedTreeRT::SGeometry::SGeometry( ) :
    m_fBranchAlphaTestValue(-1.0f),
    m_fFrondAlphaTestValue(-1.0f)
{
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry destructor definition

CSpeedTreeRT::SGeometry::~SGeometry( )
{
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SIndexed constructor definition

CSpeedTreeRT::SGeometry::SIndexed::SIndexed( ) :
    m_usVertexCount(0),
    m_nDiscreteLodLevel(-1),
    m_pColors(NULL),
    m_pNormals(NULL),
    m_pBinormals(NULL),
    m_pTangents(NULL),
    m_pCoords(NULL),
    m_pTexCoords0(NULL),
    m_pTexCoords1(NULL),
    m_pWindWeights(NULL),
    m_pWindMatrixIndices(NULL),
    m_usNumStrips(0),
    m_pStripLengths(NULL),
    m_pStrips(NULL)
{
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SIndexed destructor definition

CSpeedTreeRT::SGeometry::SIndexed::~SIndexed( )
{
    // these tables are deleted in ~CIndexedGeometry( )
    m_pColors = NULL;
    m_pNormals = NULL;
    m_pBinormals = NULL;
    m_pTangents = NULL;
    m_pCoords = NULL;
    m_pTexCoords0 = NULL;
    m_pTexCoords1 = NULL;
    m_pWindWeights = NULL;
    m_pWindMatrixIndices = NULL;
    m_pStripLengths = NULL;
    m_pStrips = NULL;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SLeaf constructor definition

CSpeedTreeRT::SGeometry::SLeaf::SLeaf( ) :
    m_bIsActive(false),
    m_fAlphaTestValue(-1.0f),
    m_nDiscreteLodLevel(-1),
    m_usLeafCount(0),
    m_pLeafMapIndices(NULL),
    m_pLeafClusterIndices(NULL),
    m_pCenterCoords(NULL),
    m_pColors(NULL),
    m_pNormals(NULL),
    m_pBinormals(NULL),
    m_pTangents(NULL),
    m_pLeafMapTexCoords(NULL),
    m_pLeafMapCoords(NULL),
    m_pWindWeights(NULL),
    m_pWindMatrixIndices(NULL)
{
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SLeaf destructor definition

CSpeedTreeRT::SGeometry::SLeaf::~SLeaf( )
{
    // this data is deleted in CLeafGeometry::SLodGeometry::~SLodGeometry( )
    m_pLeafMapIndices = NULL;
    m_pLeafClusterIndices = NULL;
    m_pCenterCoords = NULL;
    m_pColors = NULL;
    m_pNormals = NULL;
    m_pBinormals = NULL;
    m_pTangents = NULL;
    m_pLeafMapTexCoords = NULL;
    m_pLeafMapCoords = NULL;
    m_pWindWeights = NULL;
    m_pWindMatrixIndices = NULL;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SBillboard constructor definition

CSpeedTreeRT::SGeometry::SBillboard::SBillboard( ) :
    m_bIsActive(false),
    m_pTexCoords(NULL),
    m_pCoords(NULL),
    m_fAlphaTestValue(-1.0f)
{
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SGeometry::SBillboard destructor definition

CSpeedTreeRT::SGeometry::SBillboard::~SBillboard( )
{
    m_pTexCoords = NULL; // deleted in SEmbeddedTexCoords
    m_pCoords = NULL; // deleted in CSimpleBillboard
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::STextures constructor definition

CSpeedTreeRT::STextures::STextures( ) :
    m_pBranchTextureFilename(NULL),
    m_uiLeafTextureCount(0),
    m_pLeafTextureFilenames(NULL),
    m_uiFrondTextureCount(0),
    m_pFrondTextureFilenames(NULL),
    m_pCompositeFilename(NULL),
    m_pSelfShadowFilename(NULL)
{
}



///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::STextures destructor definition

CSpeedTreeRT::STextures::~STextures( )
{
    delete[] m_pLeafTextureFilenames;
    m_pLeafTextureFilenames = NULL;

    delete[] m_pFrondTextureFilenames;
    m_pFrondTextureFilenames = NULL;

    m_pBranchTextureFilename = NULL;
    m_pCompositeFilename = NULL;
    m_pSelfShadowFilename = NULL;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::CSpeedTreeRT definition
//
//  This is the default constructor exposed to the application.

CSpeedTreeRT::CSpeedTreeRT( ) :
    m_pBranchGeometry(NULL),
    m_pEngine(NULL),
    m_pLightingEngine(NULL),
    m_pWindEngine(NULL),
    m_pSimpleBillboard(NULL),
    m_pLeafGeometry(NULL),
    m_eLeafLodMethod(LOD_SMOOTH),
    m_fLeafLodTransitionRadius(0.07f),
    m_fLeafLodCurveExponent(0.7f),
    m_fLeafTransitionFactor(0.0f),
    m_pLeafLodSizeFactors(NULL),
    m_pInstanceRefCount(NULL),
    m_pInstanceData(NULL),
    m_pTreeSizes(NULL),
    m_ucTargetAlphaValue(0x54),
    m_bTreeComputed(false),
    m_fLeafSizeIncreaseFactor(0.5f),
    m_pEmbeddedTexCoords(NULL),
    m_pCollisionObjects(NULL),
    m_pFrondEngine(NULL),
    m_nBranchWindLevel(-1),
    m_nFrondLevel(-1),
    m_usNumFrondLodLevels(0),
    m_pProjectedShadow(NULL),
    m_pUserData(NULL),
    m_bHorizontalBillboard(false),
    m_b360Billboard(false)
{
    try
    {
        m_pWindEngine = new CWindEngine;
        m_pBranchGeometry = new CIndexedGeometry(m_pWindEngine);
        m_pEngine = new CTreeEngine(m_pBranchGeometry);
        m_pLightingEngine = new CLightingEngine;
        m_pLeafGeometry = new CLeafGeometry(m_pWindEngine);
        m_pSimpleBillboard = new CSimpleBillboard;
        m_pTreeSizes = new float[STS_COUNT];
        m_pInstanceRefCount = new unsigned int;
        *m_pInstanceRefCount = 1;
        m_pInstanceList = new SInstanceList;

        // fronds
        m_pFrondEngine = new CFrondEngine;
        m_pFrondGeometry = new CIndexedGeometry(m_pWindEngine, true);

        // add to global tree list
        SInstanceList::m_vUniqueTrees.push_back(this);
        m_uiAllRefCount++;

        CIdvBranch::SetLightingEngine(m_pLightingEngine);

        // setup initial tree extents
        m_pTreeSizes[STS_MIN_BOX + 0] = 0.0f;
        m_pTreeSizes[STS_MIN_BOX + 1] = 0.0f;
        m_pTreeSizes[STS_MIN_BOX + 2] = 0.0f;
        m_pTreeSizes[STS_MAX_BOX + 0] = 1.0f;
        m_pTreeSizes[STS_MAX_BOX + 1] = 1.0f;
        m_pTreeSizes[STS_MAX_BOX + 2] = 1.0f;
        m_pTreeSizes[STS_BB_SIZE] = 1.0f;

        // zero out horizontal coords
        for (int i = 0; i < 12; ++i)
            m_afHorizontalCoords[i] = 0.0f;
    }

    SpeedTreeCatch("CSpeedTreeRT::CSpeedTreeRT( )")
    SpeedTreeCatchAll("CSpeedTreeRT::CSpeedTreeRT( )");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::CSpeedTreeRT definition
//
//  This is the constructor used for instancing that is hidden from
//  the application.

CSpeedTreeRT::CSpeedTreeRT(const CSpeedTreeRT* pOrig) :
    // general
    m_pEngine(pOrig->m_pEngine),
    m_pBranchGeometry(pOrig->m_pBranchGeometry),
    m_pLeafGeometry(pOrig->m_pLeafGeometry),
    m_pLightingEngine(pOrig->m_pLightingEngine),
    m_pWindEngine(pOrig->m_pWindEngine),
    m_pSimpleBillboard(pOrig->m_pSimpleBillboard),
    m_pFrondEngine(pOrig->m_pFrondEngine),
    m_pFrondGeometry(pOrig->m_pFrondGeometry),
    m_nBranchWindLevel(pOrig->m_nBranchWindLevel),
    m_nFrondLevel(pOrig->m_nFrondLevel),
    m_usNumFrondLodLevels(pOrig->m_usNumFrondLodLevels),
    // leaf lod
    m_eLeafLodMethod(pOrig->m_eLeafLodMethod),
    m_fLeafLodTransitionRadius(pOrig->m_fLeafLodTransitionRadius),
    m_fLeafLodCurveExponent(pOrig->m_fLeafLodCurveExponent),
    m_fLeafSizeIncreaseFactor(pOrig->m_fLeafSizeIncreaseFactor),
    m_fLeafTransitionFactor(pOrig->m_fLeafTransitionFactor),
    m_pLeafLodSizeFactors(pOrig->m_pLeafLodSizeFactors),
    // instancing
    m_pInstanceRefCount(pOrig->m_pInstanceRefCount),
    m_pInstanceList(pOrig->m_pInstanceList),
    // other
    m_pTreeSizes(pOrig->m_pTreeSizes),
    m_ucTargetAlphaValue(pOrig->m_ucTargetAlphaValue),
    m_bTreeComputed(pOrig->m_bTreeComputed),
    // texcoords
    m_pEmbeddedTexCoords(pOrig->m_pEmbeddedTexCoords),
    m_b360Billboard(pOrig->m_b360Billboard),
    m_bHorizontalBillboard(pOrig->m_bHorizontalBillboard),
    // collision objects
    m_pCollisionObjects(pOrig->m_pCollisionObjects),
    // shadow projection
    m_pProjectedShadow(pOrig->m_pProjectedShadow),
    // user data
    m_pUserData(pOrig->m_pUserData)
{
    try
    {
        st_assert(pOrig);
        st_assert(pOrig->m_pEngine);
        st_assert(m_pInstanceRefCount);

        // add to parent's instance list
        m_pInstanceList->m_vInstances.push_back(this);

        // copy horizontal billboard coordinates
        memcpy(m_afHorizontalCoords, pOrig->m_afHorizontalCoords, 12 * sizeof(float));

        // reference counting for instances
        (*m_pInstanceRefCount)++;
        m_pInstanceData = new STreeInstanceData;
        m_pInstanceData->m_pParent = pOrig;
        m_pInstanceData->m_cPosition = pOrig->m_pEngine->GetPosition( );
        m_pInstanceData->m_fLodLevel = pOrig->GetLodLevel( );

        m_uiAllRefCount++;
    }

    SpeedTreeCatch("CSpeedTreeRT::CSpeedTreeRT(CSpeedTreeRT*)")
    SpeedTreeCatchAll("CSpeedTreeRT::CSpeedTreeRT(CSpeedTreeRT*)");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::~CSpeedTreeRT definition

CSpeedTreeRT::~CSpeedTreeRT( )
{
    // delete tree from global tree list (if not an instance)
    if (!m_pInstanceData)
    {
        vector<CSpeedTreeRT*>::iterator iFind = find(SInstanceList::m_vUniqueTrees.begin( ), SInstanceList::m_vUniqueTrees.end( ), this);

        // there shouldn't be any reason why this won't be found, but we're being careful
        if (iFind != SInstanceList::m_vUniqueTrees.end( ))
            (void) SInstanceList::m_vUniqueTrees.erase(iFind);
    }

    // instance reference counting
    st_assert(m_pInstanceRefCount);
    (*m_pInstanceRefCount)--;

    // if this tree was an instance
    if (m_pInstanceData)
    {
        // find the instance in the parent's instance list and delete it
        st_assert(m_pInstanceList);
        vector<CSpeedTreeRT*>::iterator iFind = find(m_pInstanceList->m_vInstances.begin( ), m_pInstanceList->m_vInstances.end( ), this);
        st_assert(iFind != m_pInstanceList->m_vInstances.end( ));
        (void) m_pInstanceList->m_vInstances.erase(iFind);

        // delete the data
        delete m_pInstanceData;
        m_pInstanceData = NULL;
    }

    // if this is the last class to hold these pointers, they should be deleted
    if (*m_pInstanceRefCount == 0)
    {
        DeleteTransientData( );
        delete m_pBranchGeometry;
        delete m_pLightingEngine;
        delete m_pWindEngine;
        delete m_pLeafGeometry;
        delete m_pSimpleBillboard;
        delete m_pEngine;
        delete m_pInstanceRefCount;
        delete m_pInstanceList;
        delete[] m_pTreeSizes;
        delete m_pCollisionObjects;
        delete m_pEmbeddedTexCoords;
        delete m_pFrondEngine;
        delete m_pFrondGeometry;
        delete m_pLeafLodSizeFactors;
        delete m_pProjectedShadow;
        delete[] m_pUserData;
    }

    // null the pointers because delete has been called on this
    // instance even though memory may not necessarily be freed.

    //lint -save -e672
    m_pBranchGeometry = NULL;
    m_pLightingEngine = NULL;
    m_pWindEngine = NULL;
    m_pLeafGeometry = NULL;
    m_pSimpleBillboard = NULL;
    m_pEngine = NULL;
    m_pInstanceRefCount = NULL;
    m_pTreeSizes = NULL;
    m_pInstanceList = NULL;
    m_pCollisionObjects = NULL;
    m_pEmbeddedTexCoords = NULL;
    m_pFrondEngine = NULL;
    m_pFrondGeometry = NULL;
    m_pLeafLodSizeFactors = NULL;
    m_pProjectedShadow = NULL;
    m_pUserData = NULL;
    //lint -restore

    if (!--m_uiAllRefCount)
        CIdvBezierSpline::ClearCache( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::operator new definition
//
//  These operators ensure that the memory allocation is handled using the
//  C run-time (CRT) library linked by the DLL.  This enables the DLL to be
//  used by applications using a different CRT.

void* CSpeedTreeRT::operator new(size_t nSize)
{
    return ::operator new(nSize);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::operator delete definition
//
//  These operators ensure that the memory allocation is handled using the
//  C run-time (CRT) library linked by the DLL.  This enables the DLL to be
//  used by applications using a different CRT.

void CSpeedTreeRT::operator delete(void* pRawMemory)
{
    ::operator delete(pRawMemory);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::operator new[] definition
//
//  These operators ensure that the memory allocation is handled using the
//  C run-time (CRT) library linked by the DLL.  This enables the DLL to be
//  used by applications using a different CRT.

void* CSpeedTreeRT::operator new[](size_t nSize)
{
    return ::new char[nSize];
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::operator delete[] definition
//
//  These operators ensure that the memory allocation is handled using the
//  C run-time (CRT) library linked by the DLL.  This enables the DLL to be
//  used by applications using a different CRT.

void CSpeedTreeRT::operator delete[](void* pRawMemory)
{
    ::delete[] (char*)pRawMemory;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::Compute definition

bool CSpeedTreeRT::Compute(const float* pTransform, unsigned int nSeed, bool bCompositeStrips)
{
    try
    {
        if (!m_bTreeComputed)
        {
            // compute tree's geometry
            CIdvBranch::SetFrondEngine(m_pFrondEngine);
            m_pEngine->SetSeed(nSeed);
            m_pEngine->Compute(m_fLeafSizeIncreaseFactor);
            m_pFrondEngine->Compute(m_pFrondGeometry, m_pLightingEngine);
            m_usNumFrondLodLevels = m_pFrondGeometry->GetNumLodLevels( );

            m_pEngine->InitTables( );

            // compute static lighting
            if (m_pLightingEngine->GetLeafLightingMethod( ) == CSpeedTreeRT::LIGHT_STATIC)
                ComputeLeafStaticLighting( );

            // prep wind and leaves
            const SIdvWindInfo& sWindInfo = m_pEngine->GetWindData( );
            m_pWindEngine->Init(sWindInfo);
            m_pLeafGeometry->Init(static_cast<unsigned short>(m_pEngine->GetNumLeafLodLevels( )), m_pEngine->GetAllLeaves( ), m_pEngine->GetLeafInfo( ));

            // move embedeed texcoords (from file) into leaf geometry class and fronds
            if (m_pEmbeddedTexCoords)
            {
                for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumLeafMaps; ++i)
                    m_pLeafGeometry->SetTextureCoords(i, &(m_pEmbeddedTexCoords->m_pLeafTexCoords[i * 8]));
                
                for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumFrondMaps; ++i)
                    m_pFrondEngine->SetTextureCoords(m_pFrondGeometry, i, &(m_pEmbeddedTexCoords->m_pFrondTexCoords[i * 8]), m_bTextureFlip);
            }

            // for each branch LOD level, convert all strips to single strip using
            // degenerate triangles
            if (bCompositeStrips)
            {
                m_pBranchGeometry->CombineStrips( );
                m_pFrondGeometry->CombineStrips(false);
            }

            // apply optional transformation
            if (pTransform)
            {
                CTransform cTransform;
                memcpy((void*) cTransform.m_afData, pTransform, 16 * sizeof(float));

                m_pBranchGeometry->Transform(cTransform);
                m_pLeafGeometry->Transform(cTransform);
                m_pFrondGeometry->Transform(cTransform);
                if (m_pCollisionObjects)
                    m_pCollisionObjects->TransformAll(cTransform);
            }

            // retrieve extents from branch and leaf geometry classes
            CRegion cExtents;
            m_pBranchGeometry->ComputeExtents(cExtents);
            m_pLeafGeometry->ComputeExtents(cExtents);
            m_pFrondGeometry->ComputeExtents(cExtents);

            memcpy(m_pTreeSizes + STS_MIN_BOX, cExtents.m_cMin.m_afData, 3 * sizeof(float));
            memcpy(m_pTreeSizes + STS_MAX_BOX, cExtents.m_cMax.m_afData, 3 * sizeof(float));

            // figure out how wide the billboard should be
            CVec3 cOrigin(0.0f, 0.0f, 0.0f);
            //lint -save -e666
#ifdef UPVECTOR_POS_Y
            float fDimension = cOrigin.Distance(CVec3(m_pTreeSizes[0], m_pTreeSizes[2], 0.0f));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[0], m_pTreeSizes[5], 0.0f)));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[3], m_pTreeSizes[2], 0.0f)));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[3], m_pTreeSizes[5], 0.0f)));
#else
            float fDimension = cOrigin.Distance(CVec3(m_pTreeSizes[0], m_pTreeSizes[1], 0.0f));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[0], m_pTreeSizes[4], 0.0f)));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[3], m_pTreeSizes[1], 0.0f)));
            fDimension = __max(fDimension, cOrigin.Distance(CVec3(m_pTreeSizes[3], m_pTreeSizes[4], 0.0f)));
#endif
            //lint -restore
            m_pTreeSizes[STS_BB_SIZE] = fDimension * 2.0f;

            // compute the self-shadow coordinates
            if (m_pProjectedShadow)
                ComputeSelfShadowTexCoords( );

            // setup the horizontal billboard
            SetupHorizontalBillboard( );

            // some operations are not valid once the geometry has been computed
            m_bTreeComputed = true;
        }
        else
            SetError("Compute() called more than once for single tree model (ignored)");
    }

    SpeedTreeCatch("CSpeedTreeRT::Compute")
    SpeedTreeCatchAll("CSpeedTreeRT::Compute");

    return m_bTreeComputed;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::Clone definition

CSpeedTreeRT* CSpeedTreeRT::Clone(float x, float y, float z, unsigned int nSeed) const
{
    CSpeedTreeRT* pClone = NULL;
    try
    {
        if (m_pEngine->TransientDataIntact( ))
        {
            // allocate the clone
            pClone = new CSpeedTreeRT;

            // copy all of the appropriate data
            *pClone->m_pLightingEngine = *m_pLightingEngine;
            *pClone->m_pWindEngine = *m_pWindEngine;
            *pClone->m_pFrondEngine = *m_pFrondEngine;

            pClone->m_pBranchGeometry->EnableVertexWeighting(m_pBranchGeometry->IsVertexWeightingEnabled( ));
            pClone->m_pBranchGeometry->EnableManualLighting(m_pLightingEngine->GetBranchLightingMethod( ) == LIGHT_STATIC);
            pClone->m_pBranchGeometry->SetWindMethod(m_pWindEngine->GetBranchWindMethod( ));

            pClone->m_pFrondGeometry->EnableVertexWeighting(m_pFrondGeometry->IsVertexWeightingEnabled( ));
            pClone->m_pFrondGeometry->EnableManualLighting(m_pLightingEngine->GetFrondLightingMethod( ) == LIGHT_STATIC);
            pClone->m_pFrondGeometry->SetWindMethod(m_pWindEngine->GetFrondWindMethod( ));

            pClone->m_pLeafGeometry->EnableVertexWeighting(m_pLeafGeometry->IsVertexWeightingEnabled( ));
            pClone->m_pLeafGeometry->EnableManualLighting(m_pLightingEngine->GetLeafLightingMethod( ) == LIGHT_STATIC);

            pClone->m_ucTargetAlphaValue = m_ucTargetAlphaValue;

            m_pEngine->Clone(pClone->m_pEngine, CVec3(x, y, z), nSeed);

            // copy embedded coordinate if they exist
            if (m_pEmbeddedTexCoords)
            {
                pClone->m_pEmbeddedTexCoords = new SEmbeddedTexCoords;
                *pClone->m_pEmbeddedTexCoords = *m_pEmbeddedTexCoords;
            }
            // copy projected shadow if it exists
            if (m_pProjectedShadow)
            {
                pClone->m_pProjectedShadow = new CProjectedShadow;
                *pClone->m_pProjectedShadow = *m_pProjectedShadow;
            }
            // copy collision objects if they exist
            if (m_pCollisionObjects)
            {
                pClone->m_pCollisionObjects = new SCollisionObjects;
                *pClone->m_pCollisionObjects = *m_pCollisionObjects;
            }
            // copy m_pLeafLodSizeFactors if allocated
            if (m_pLeafLodSizeFactors)
            {
                pClone->m_pLeafLodSizeFactors = new float[GetNumLeafLodLevels( )];
                memcpy(pClone->m_pLeafLodSizeFactors, m_pLeafLodSizeFactors, GetNumLeafLodLevels( ) * sizeof(float));
            }
            // copy user data if it exists
            if (m_pUserData)
                pClone->m_pUserData = CopyUserData(m_pUserData);

            pClone->m_eLeafLodMethod = m_eLeafLodMethod;
            pClone->m_fLeafLodTransitionRadius = m_fLeafLodTransitionRadius;
            pClone->m_fLeafLodCurveExponent = m_fLeafLodCurveExponent;
            pClone->m_fLeafSizeIncreaseFactor = m_fLeafSizeIncreaseFactor;
            pClone->m_fLeafTransitionFactor = m_fLeafTransitionFactor;
            pClone->m_usNumFrondLodLevels = m_usNumFrondLodLevels;
            pClone->m_b360Billboard = m_b360Billboard;
            pClone->m_bHorizontalBillboard = m_bHorizontalBillboard;

            // copy horizontal billboard coordinates
            memcpy(pClone->m_afHorizontalCoords, m_afHorizontalCoords, 12 * sizeof(float));

            st_assert(pClone);
            st_assert(pClone->m_pEngine);
        }
        else
            SetError("cannot Clone() after calling DeleteTransientData()");
    }

    SpeedTreeCatch("CSpeedTreeRT::Clone")
    SpeedTreeCatchAll("CSpeedTreeRT::Clone");

    return pClone;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::InstanceOf definition

const CSpeedTreeRT* CSpeedTreeRT::InstanceOf(void) const
{
    const CSpeedTreeRT* pParent = NULL;
    if (m_pInstanceData)
        pParent = m_pInstanceData->m_pParent;

    return pParent;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::MakeInstance definition

CSpeedTreeRT* CSpeedTreeRT::MakeInstance(void)
{
    CSpeedTreeRT* pInstance = NULL;
    try
    {
        // we can only make instances if the transient data is intact
        if (m_pEngine->TransientDataIntact( ))
        {
            pInstance = new CSpeedTreeRT(this);
        }
        else
            SetError("cannot MakeInstance() after calling DeleteTransientData()");
    }

    SpeedTreeCatch("CSpeedTreeRT::MakeInstance")
    SpeedTreeCatchAll("CSpeedTreeRT::MakeInstance");

    return pInstance;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::DeleteTransientData definition

void CSpeedTreeRT::DeleteTransientData(void)
{
    if (m_pEngine->TransientDataIntact( ))
        m_pEngine->FreeTransientData( );
    else
        SetError("DeleteTransientData() called with no intact transient data");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::LoadTree definition

bool CSpeedTreeRT::LoadTree(const char* pFilename)
{
#include "EvalTest.h"

    bool bSuccess = false;
    unsigned char* pMemory = NULL;

    try
    {
        if (pFilename)
        {
            FILE* pFile = fopen(pFilename, "rb");
            if (pFile)
            {
                fseek(pFile, 0L, SEEK_END);
                int nNumBytes = ftell(pFile);
                int nErrorCode = fseek(pFile, 0, SEEK_SET);

                if (nNumBytes > 0 &&
                    nErrorCode >= 0)
                {
                    // read all of the file into memory, then pass into other Parse() function
                    pMemory = new unsigned char[nNumBytes];
                    int nBytesRead = fread(pMemory, 1, nNumBytes, pFile);
                    if (nBytesRead == nNumBytes)
                    {
                        bSuccess = LoadTree(pMemory, nNumBytes);

                        delete[] pMemory;
                        pMemory = NULL;
                    }
                    else
#ifdef __GNUC__
						throw(runtime_error(IdvFormatString("only read %d of %d from %s [%s]", nBytesRead, nNumBytes, pFilename, strerror(errno)).c_str( )));
#else
                        throw(exception(IdvFormatString("only read %d of %d from %s [%s]", nBytesRead, nNumBytes, pFilename, strerror(errno)).c_str( )));
#endif

                    // in our experience, fclose doesn't always give accurate return values
                    (void) fclose(pFile);
                }
                else
#ifdef __GNUC__
					throw(runtime_error(IdvFormatString("file seek failed on '%s' [%s]", pFilename, strerror(errno)).c_str( )));
#else
                    throw(exception(IdvFormatString("file seek failed on '%s' [%s]", pFilename, strerror(errno)).c_str( )));
#endif
            }
            else
#ifdef __GNUC__
				throw(runtime_error(IdvFormatString("failed to load file '%s' [%s]", pFilename, strerror(errno)).c_str( )));
#else
                throw(exception(IdvFormatString("failed to load file '%s' [%s]", pFilename, strerror(errno)).c_str( )));
#endif
        }
    }

#ifdef __GNUC__
	catch (runtime_error& cException)
#else
    catch (exception& cException)
#endif
    {
        pMemory = NULL;
        SetError(IdvFormatString("CSpeedTreeRT::Load Tree - %s", cException.what( )).c_str( ));
    }

    catch (...)
    {
        pMemory = NULL;
        SetError(IdvFormatString("CSpeedTreeRT::LoadTree - threw an unknown system exception").c_str( ));
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::LoadTree definition

bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes)
{
//lint -e537  { it's OK that this file is included twice }
#include "EvalTest.h"

    bool bSuccess = false;
    try
    {
        CTreeFileAccess cFile(pBlock, nNumBytes);
        if (m_pEngine->Parse(cFile))
        {
            if (!cFile.EndOfFile( ))
            {
                // New spt data is saved after all original format to
                // maintain backward compatibility

                bool bEndOfFile = false;
                int nToken = cFile.ParseToken( );
                do
                {
                    switch (nToken)
                    {
                    case File_BeginLightingInfo:
                        m_pLightingEngine->Parse(cFile);
                        break;
                    case File_BeginLodInfo:
                        ParseLodInfo(&cFile);
                        break;
                    case File_BeginNewWindInfo:
                        ParseWindInfo(&cFile);
                        break;
                    case File_BeginTextureCoordInfo:
                        ParseTextureCoordInfo(&cFile);
                        break;
                    case File_BeginCollisionInfo:
                        ParseCollisionObjects(&cFile);
                        break;
                    case File_BeginFrondInfo:
                        m_pFrondEngine->Parse(cFile);
                        break;
                    case File_BeginTextureControls:
                        m_pEngine->ParseTextureControls(cFile);
                        break;
                    case File_BeginFlareInfo:
                        m_pEngine->ParseFlareInfo(cFile);
                        break;
                    case File_FlareSeed:
                        m_pEngine->ParseFlareSeed(cFile);
                        break;
                    case File_LeafTransitionFactor:
                        m_fLeafTransitionFactor = cFile.ParseFloat( );
                        break;
                    case File_BeginShadowProjectionInfo:
                        ParseShadowProjectionInfo(&cFile);
                        break;
                    case File_BeginUserData:
                        ParseUserData(&cFile);
                        break;
                    case File_BeginSupplementalTexCoordInfo:
                        ParseSupplementalTexCoordInfo(&cFile);
                        break;
                    case File_SpeedWindRockScalar:
                        m_pWindEngine->SetSpeedWindRockScalar(cFile.ParseFloat( ));
                        break;
                    case File_SpeedWindRustleScalar:
                        m_pWindEngine->SetSpeedWindRustleScalar(cFile.ParseFloat( ));
                        break;
                    case File_PropagateFlexibility:
                        CIdvBranch::SetPropagateFlexibility(cFile.ParseBool( ));
                        break;
                    default:
                        bEndOfFile = true;
                    }
                    if (cFile.EndOfFile( ))
                        bEndOfFile = true;
                    else
                        nToken = cFile.ParseToken( );
                } while (!bEndOfFile);
            }

            // update all lighting data
            SetBranchLightingMethod(m_pLightingEngine->GetBranchLightingMethod( ));
            SetLeafLightingMethod(m_pLightingEngine->GetLeafLightingMethod( ));
            SetFrondLightingMethod(m_pLightingEngine->GetFrondLightingMethod( ));

            // extract other data for later queries
            m_nBranchWindLevel = m_pEngine->GetBranchLevelForWeighting( );
            m_nFrondLevel = m_pFrondEngine->GetLevel( );

            // handle obsolete smooth_2 setting
            if (m_eLeafLodMethod == 2)
            {
                m_eLeafLodMethod = CSpeedTreeRT::LOD_SMOOTH;
                m_fLeafTransitionFactor = 0.5f;
            }

            // make matrix span match number of available matrices
            m_pWindEngine->SetLocalMatrices(0, m_pWindEngine->GetNumWindMatrices( ));

            bSuccess = true;
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::LoadTree")
    SpeedTreeCatchAll("CSpeedTreeRT::LoadTree");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SaveTree definition

unsigned char* CSpeedTreeRT::SaveTree(unsigned int& nNumBytes, bool bSaveLeaves) const
{
    unsigned char* pData = NULL;
    try
    {
        if (m_pEngine->TransientDataIntact( ))
        {
            CTreeFileAccess cFile;
            // save original spt data
            m_pEngine->Save(cFile, bSaveLeaves);

            // save lighting info
            m_pLightingEngine->Save(cFile);

            // save lod info
            cFile.SaveToken(File_BeginLodInfo);
            cFile.SaveToken(File_LeafTransitionMethod);
            cFile.SaveInt(static_cast<int>(m_eLeafLodMethod));
            cFile.SaveToken(File_LeafTransitionRadius);
            cFile.SaveFloat(m_fLeafLodTransitionRadius);
            cFile.SaveToken(File_LeafCurveExponent);
            cFile.SaveFloat(m_fLeafLodCurveExponent);
            cFile.SaveToken(File_LeafSizeIncreaseFactor);
            cFile.SaveFloat(m_fLeafSizeIncreaseFactor);
            m_pEngine->SaveLodInfo(cFile);
            cFile.SaveToken(File_EndLodInfo);

            // save wind info
            cFile.SaveToken(File_BeginNewWindInfo);
            cFile.SaveToken(File_WindLevel);
            cFile.SaveInt(m_pEngine->GetBranchLevelForWeighting( ));
            cFile.SaveToken(File_EndNewWindInfo);

            // save texture coord info
            if (m_pEmbeddedTexCoords)
                SaveTextureCoords(&cFile);

            // save collision objects
            if (m_pCollisionObjects)
                SaveCollisionObjects(&cFile);

            // save frond info
            m_pFrondEngine->Save(cFile);

            // save extras
            m_pEngine->SaveTextureControls(cFile);
            m_pEngine->SaveFlareInfo(cFile);
            m_pEngine->SaveFlareSeed(cFile);
            cFile.SaveToken(File_LeafTransitionFactor);
            cFile.SaveFloat(m_fLeafTransitionFactor);

            // save projected shadows
            if (m_pProjectedShadow)
                m_pProjectedShadow->Save(cFile);

            // save user data
            SaveUserData(&cFile);

            // save supplemental texcoord info
            if (m_pEmbeddedTexCoords)
                SaveSupplementalTexCoordInfo(&cFile);

            // speedwind scalars
            cFile.SaveToken(File_SpeedWindRockScalar);
            cFile.SaveFloat(m_pWindEngine->GetSpeedWindRockScalar( ));
            cFile.SaveToken(File_SpeedWindRustleScalar);
            cFile.SaveFloat(m_pWindEngine->GetSpeedWindRustleScalar( ));

            // flexibility propagation
            cFile.SaveToken(File_PropagateFlexibility);
            cFile.SaveBool(CIdvBranch::GetPropagateFlexibility( ));

            // create spt formatted data
            pData = cFile.ConvertMemoryToArray(nNumBytes);
        }
        else
            SetError("cannot Save() after DeleteTransientData() is called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SaveTree")
    SpeedTreeCatchAll("CSpeedTreeRT::SaveTree");

    return pData;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetTreeSize definition

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const
{
    m_pEngine->GetSize(fSize, fVariance);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetTreeSize definition

void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance)
{
    try
    {
        if (m_pEngine->TransientDataIntact( ))
        {
            if (fNewSize > 0.0f)
                m_pEngine->SetSize(fNewSize, fNewVariance);
            else
                SetError("SetTreeSize() is only valid for size values greater than 0.0");
        }
        else
            SetError("SetTreeSize() has no effect after DeleteTransientData() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetTreeSize")
    SpeedTreeCatchAll("CSpeedTreeRT::SetTreeSize");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetSeed definition

unsigned int CSpeedTreeRT::GetSeed(void) const
{
    unsigned int nSeed = 0;
    try
    {
        if (m_pEngine->TransientDataIntact( ))
            nSeed = m_pEngine->GetSeed( );
        else
            SetError("SetTreeSize() has no effect after DeleteTransientData() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetTreeSize")
    SpeedTreeCatchAll("CSpeedTreeRT::SetTreeSize");

    return nSeed;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetTreePosition definition

const float* CSpeedTreeRT::GetTreePosition(void) const
{
    const float* pPos = NULL;
    if (m_pInstanceData)
        pPos = m_pInstanceData->m_cPosition.m_afData;
    else
        pPos = m_pEngine->GetPosition( ).m_afData;

    return pPos;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetTreePosition definition

void CSpeedTreeRT::SetTreePosition(float x, float y, float z)
{
    if (m_pInstanceData)
        m_pInstanceData->m_cPosition.Set(x, y, z);
    else
        m_pEngine->SetPosition(CVec3(x, y, z));
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafTargetAlphaMask definition

void CSpeedTreeRT::SetLeafTargetAlphaMask(unsigned char ucMask)
{
    m_ucTargetAlphaValue = ucMask;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBranchLightingMethod definition

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetBranchLightingMethod(void) const
{
    return m_pLightingEngine->GetBranchLightingMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetBranchLightingMethod definition

void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod eMethod)
{
    try
    {
        if (!m_bTreeComputed)
        {
            m_pBranchGeometry->EnableManualLighting(eMethod == LIGHT_STATIC);
            m_pLightingEngine->SetBranchLightingMethod(eMethod);
        }
        else
            SetError("SetBranchLightingMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetBranchLightingMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetBranchLightingMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafLightingMethod definition

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetLeafLightingMethod(void) const
{
    return m_pLightingEngine->GetLeafLightingMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafLightingMethod definition

void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod eMethod)
{
    try
    {
        if (!m_bTreeComputed)
        {
            m_pLightingEngine->SetLeafLightingMethod(eMethod);
            m_pLeafGeometry->EnableManualLighting(eMethod == LIGHT_STATIC);
        }
        else
            SetError("SetLeafLightingMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafLightingMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafLightingMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetStaticLightingStyle definition

CSpeedTreeRT::EStaticLightingStyle CSpeedTreeRT::GetStaticLightingStyle(void) const
{
    return m_pLightingEngine->GetStaticLightingStyle( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetStaticLightingStyle definition

void CSpeedTreeRT::SetStaticLightingStyle(EStaticLightingStyle eStyle)
{
    try
    {
        if (!m_bTreeComputed)
        {
            st_assert(m_pLightingEngine);
            m_pLightingEngine->SetStaticLightingStyle(eStyle);
        }
        else
            SetError("SetStaticLightingStyle() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetStaticLightingStyle")
    SpeedTreeCatchAll("CSpeedTreeRT::SetStaticLightingStyle");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafLightingAdjustment definition

float CSpeedTreeRT::GetLeafLightingAdjustment( ) const
{
    return m_pLightingEngine->GetLeafLightingAdjustment( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafLightingAdjustment definition

void CSpeedTreeRT::SetLeafLightingAdjustment(float fScalar)
{
    try
    {
        if (!m_bTreeComputed)
            m_pLightingEngine->SetLeafLightingAdjustment(fScalar);
        else
            SetError("SetLeafLightingAdjustment() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafLightingAdjustment")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafLightingAdjustment");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondLightingMethod definition

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetFrondLightingMethod(void) const
{
    return m_pLightingEngine->GetFrondLightingMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetFrondLightingMethod definition

void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod eMethod)
{
    try
    {
        if (!m_bTreeComputed)
        {
            m_pFrondGeometry->EnableManualLighting(eMethod == LIGHT_STATIC);
            m_pLightingEngine->SetFrondLightingMethod(eMethod);
        }
        else
            SetError("SetFrondLightingMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetFrondLightingMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetFrondLightingMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLightState definition

bool CSpeedTreeRT::GetLightState(unsigned int nLightIndex)
{
    return CLightingEngine::GetLightState(nLightIndex);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLightState definition

void CSpeedTreeRT::SetLightState(unsigned int nLightIndex, bool bLightState)
{
    CLightingEngine::SetLightState(nLightIndex, bLightState);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLightAttributes definition

const float* CSpeedTreeRT::GetLightAttributes(unsigned int nLightIndex)
{
    return CLightingEngine::GetLightAttributes(nLightIndex);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLightAttributes definition

void CSpeedTreeRT::SetLightAttributes(unsigned int nLightIndex, const float* pAttributes)
{
    try
    {
        CLightingEngine::SetLightAttributes(nLightIndex, pAttributes);
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLightAttributes")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLightAttributes");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBranchMaterial definition

const float* CSpeedTreeRT::GetBranchMaterial(void) const
{
    return m_pLightingEngine->GetBranchMaterial( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetBranchMaterial definition

void CSpeedTreeRT::SetBranchMaterial(const float* pBranchMaterial)
{
    try
    {
        if (!m_bTreeComputed)
            m_pLightingEngine->SetBranchMaterial(pBranchMaterial);
        else
            SetError("SetBranchMaterial() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetBranchMaterial")
    SpeedTreeCatchAll("CSpeedTreeRT::SetBranchMaterial");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafMaterial definition

const float* CSpeedTreeRT::GetLeafMaterial(void) const
{
    return m_pLightingEngine->GetLeafMaterial( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafMaterial definition

void CSpeedTreeRT::SetLeafMaterial(const float* pLeafMaterial)
{
    try
    {
        if (!m_bTreeComputed)
            m_pLightingEngine->SetLeafMaterial(pLeafMaterial);
        else
            SetError("SetLeafMaterial() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafMaterial")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafMaterial");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondMaterial definition

const float* CSpeedTreeRT::GetFrondMaterial (void) const
{
    return m_pLightingEngine->GetFrondMaterial( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetFrondMaterial definition

void CSpeedTreeRT::SetFrondMaterial(const float* pLeafMaterial)
{
    try
    {
        if (!m_bTreeComputed)
            m_pLightingEngine->SetFrondMaterial(pLeafMaterial);
        else
            SetError("SetFrondMaterial() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetFrondMaterial")
    SpeedTreeCatchAll("CSpeedTreeRT::SetFrondMaterial");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetCamera definition

void CSpeedTreeRT::GetCamera(float* pPosition, float* pDirection)
{
    if (pPosition && pDirection)
    {
        CVec3 cCameraPos, cCameraDirection;
        CTreeEngine::GetCamera(cCameraPos, cCameraDirection);

        memcpy(pPosition, cCameraPos.m_afData, 3 * sizeof(float));
        memcpy(pDirection, cCameraDirection.m_afData, 3 * sizeof(float));
    }
    else
        SetError("GetCamera() requires non-NULL position and direction values");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetCamera definition

void CSpeedTreeRT::SetCamera(const float *pPosition, const float* pDirection)
{
    try
    {
        if (pPosition && pDirection)
        {
            CVec3 cOldPosition, cOldDirection;
            CTreeEngine::GetCamera(cOldPosition, cOldDirection);

            CVec3 cNewPosition(pPosition[0], pPosition[1], pPosition[2]);
            CVec3 cNewDirection(pDirection[0], pDirection[1], pDirection[2]);

            // only want to recompute billboards if necessary
            if (cOldPosition != cNewPosition ||
                cOldDirection != cNewDirection)
            {
                CTreeEngine::SetCamera(cNewPosition, cNewDirection);
                NotifyAllTreesOfEvent(STIE_CLIENT_CHANGED_CAMERA);
                CSimpleBillboard::ComputeUnitBillboard(cNewDirection.m_afData);
                NotifyAllTreesOfEvent(STIE_CLIENT_CHANGED_CAMERA);

                // determine horizontal billboard fade value
                CVec3 cAdjustedDirection;
                Assign3d(cAdjustedDirection, cNewDirection);
                float fPitch = VecRad2Deg(asinf(cAdjustedDirection[2]));

                float fPercent = 1.0f - (fPitch - m_fHorizontalFadeStartAngle) / (m_fHorizontalFadeEndAngle - m_fHorizontalFadeStartAngle);
                fPercent = __max(0.0f, fPercent);
                fPercent = __min(1.0f, fPercent);

                m_fHorizontalFadeValue = VecInterpolate(84.0f, 255.0f, fPercent);
            }
        }
        else
            SetError("SetCamera() requires non-NULL position and direction values");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetCamera")
    SpeedTreeCatchAll("CSpeedTreeRT::SetCamera");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetTime definition

void CSpeedTreeRT::SetTime(float fTime)
{
    try
    {
        CWindEngine::SetTime(fTime);
        NotifyAllTreesOfEvent(STIE_CLIENT_CHANGED_WIND);
    }

    SpeedTreeCatch("CSpeedTreeRT::SetTime")
    SpeedTreeCatchAll("CSpeedTreeRT::SetTime");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ComputeWindEffects definition

void CSpeedTreeRT::ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds)
{
    try
    {
        float fHighestLod = -1.0f;

        // branches
        if (bBranches &&
            m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
        {
            if (!m_pInstanceData && *m_pInstanceRefCount > 1)
            {
                // If this tree has instances of itself, then compute the highest LOD
                // among itself and its instances.  There's no need for more wind
                // computations than the highest LOD because all of the LODs share
                // the same vertexes.
                fHighestLod = m_pEngine->GetLod( );
                for (vector<CSpeedTreeRT*>::iterator i = m_pInstanceList->m_vInstances.begin( );
                     i != m_pInstanceList->m_vInstances.end( ); ++i)
                {
                    float fInstanceLodLevel = (*i)->GetLodLevel( );
                    if (fInstanceLodLevel > fHighestLod)
                        fHighestLod = fInstanceLodLevel;
                }

                (void) m_pBranchGeometry->ComputeWindEffect(GetDiscreteBranchLodLevel(fHighestLod));
            }
            else
                (void) m_pBranchGeometry->ComputeWindEffect(GetDiscreteBranchLodLevel(GetLodLevel( )));
        }

        // fronds
        if (bFronds &&
            m_pWindEngine->GetFrondWindMethod( ) == CSpeedTreeRT::WIND_CPU)
        {
            if (!m_pInstanceData && *m_pInstanceRefCount > 1)
            {
                // If this tree has instances of itself, then compute the highest LOD
                // among itself and its instances.  There's no need for more wind
                // computations than the highest LOD because all of the LODs share
                // the same vertexes.
                if (fHighestLod == -1.0f) // we don't need to make this calc if it was
                {                         // already done for the branches
                    fHighestLod = m_pEngine->GetLod( );
                    for (vector<CSpeedTreeRT*>::iterator i = m_pInstanceList->m_vInstances.begin( );
                         i != m_pInstanceList->m_vInstances.end( ); ++i)
                    {
                        float fInstanceLodLevel = (*i)->GetLodLevel( );
                        if (fInstanceLodLevel > fHighestLod)
                            fHighestLod = fInstanceLodLevel;
                    }
                }

                (void) m_pFrondGeometry->ComputeWindEffect(GetDiscreteFrondLodLevel(fHighestLod));
            }
            else
                (void) m_pFrondGeometry->ComputeWindEffect(GetDiscreteFrondLodLevel(GetLodLevel( )));
        }

        // leaves
        if (bLeaves &&
            m_pWindEngine->GetLeafWindMethod( ) == CSpeedTreeRT::WIND_CPU)
        {
            m_pLeafGeometry->Invalidate( );
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::ComputeWindEffects")
    SpeedTreeCatchAll("CSpeedTreeRT::ComputeWindEffects");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ResetLeafWindState definition

void CSpeedTreeRT::ResetLeafWindState( )
{
    m_pWindEngine->ResetLeafWindState( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafRockingState definition

bool CSpeedTreeRT::GetLeafRockingState(void) const
{
    return m_pWindEngine->GetLeafRockingState( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafRockingState definition

void CSpeedTreeRT::SetLeafRockingState(bool bFlag)
{
    m_pWindEngine->SetLeafRockingState(bFlag);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetNumLeafRockingGroups definition

void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int nRockingGroups)
{
    try
    {
        if (!m_bTreeComputed)
        {
            if (nRockingGroups == 0)
                nRockingGroups = 1;

            m_pEngine->SetNumLeafRockingGroups(nRockingGroups);
        }
        else
            SetError("SetNumLeafRockingGroups() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafRockingState")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafRockingState");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafWindMethod definition

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetLeafWindMethod(void) const
{
    return m_pWindEngine->GetLeafWindMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafWindMethod definition

void CSpeedTreeRT::SetLeafWindMethod(EWindMethod eMethod)
{
    try
    {
        if (!m_bTreeComputed)
        {
            m_pWindEngine->SetLeafWindMethod(eMethod);
            m_pLeafGeometry->EnableVertexWeighting(eMethod != WIND_NONE);
        }
        else
            SetError("SetLeafWindMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafWindMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafWindMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBranchWindMethod definition

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetBranchWindMethod(void) const
{
    return m_pWindEngine->GetBranchWindMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetBranchWindMethod definition

void CSpeedTreeRT::SetBranchWindMethod(EWindMethod eMethod)
{
    try
    {
        // Because the wind engine is optimized to only calculate the wind
        // effect for the highest visible LOD, the highest overall LOD may
        // not be calcuated.  When the wind is turned off, some of the tree
        // will be positioned according to the last wind calculation and
        // some of it may not be.  We make a call here to force it to
        // calculate the entire tree's geometry for the last wind value.

        if (!m_bTreeComputed)
        {
            if (eMethod == WIND_NONE &&
                m_pWindEngine->GetBranchWindMethod( ) == WIND_CPU)
            {
                m_pBranchGeometry->Invalidate( );
                (void) m_pBranchGeometry->ComputeWindEffect(0);
            }

            m_pWindEngine->SetBranchWindMethod(eMethod);
            m_pBranchGeometry->EnableVertexWeighting(eMethod != WIND_NONE);
            m_pBranchGeometry->SetWindMethod(eMethod);
        }
        else
            SetError("SetBranchWindMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetBranchWindMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetBranchWindMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondWindMethod definition

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetFrondWindMethod(void) const
{
    return m_pWindEngine->GetFrondWindMethod( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetFrondWindMethod definition

void CSpeedTreeRT::SetFrondWindMethod(EWindMethod eMethod)
{
    try
    {
        // Because the wind engine is optimized to only calculate the wind
        // effect for the highest visible LOD, the highest overall LOD may
        // not be calcuated.  When the wind is turned off, some of the tree
        // will be positioned according to the last wind calculation and
        // some of it may not be.  We make a call here to force it to
        // calculate the entire tree's geometry for the last wind value.

        if (!m_bTreeComputed)
        {
            if (eMethod == WIND_NONE &&
                m_pWindEngine->GetFrondWindMethod( ) == WIND_CPU)
            {
                m_pFrondGeometry->Invalidate( );
                (void) m_pFrondGeometry->ComputeWindEffect(0);
            }

            m_pWindEngine->SetFrondWindMethod(eMethod);
            m_pFrondGeometry->EnableVertexWeighting(eMethod != WIND_NONE);
            m_pFrondGeometry->SetWindMethod(eMethod);
        }
        else
            SetError("SetFrondWindMethod() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetFrondWindMethod")
    SpeedTreeCatchAll("CSpeedTreeRT::SetFrondWindMethod");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetWindStrength definition

float CSpeedTreeRT::GetWindStrength(void) const
{
    return m_pWindEngine->GetWindStrength( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetWindStrength definition

float CSpeedTreeRT::SetWindStrength(float fNewStrength, float fOldStrength, float fFrequencyTimeOffset)
{
    float fNewFrequencyTimeShift = 0.0f;

    try
    {
        if (fNewStrength >= 0.0f)
        {
            if (fOldStrength == -1.0f)
                fOldStrength = m_pWindEngine->GetWindStrength( );
            if (fFrequencyTimeOffset == -1.0f)
                fFrequencyTimeOffset = m_pWindEngine->GetFrequencyTimeShift( );

            fNewFrequencyTimeShift = m_pWindEngine->SetWindStrength(fNewStrength, fOldStrength, fFrequencyTimeOffset);

            // invalidate geometry if CPU-based wind
            if (GetBranchWindMethod( ) == WIND_CPU &&
                m_pBranchGeometry)
                m_pBranchGeometry->Invalidate( );

            if (GetFrondWindMethod( ) == WIND_CPU &&
                m_pFrondGeometry)
                m_pFrondGeometry->Invalidate( );

            if (GetLeafWindMethod( ) == WIND_CPU ||
                m_pWindEngine->GetLeafRockingState( ))
                m_pLeafGeometry->Invalidate( );
        }
        else
            SetError("SetWindStrength() expects new wind strength >= 0.0");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetWindStrength")
    SpeedTreeCatchAll("CSpeedTreeRT::SetWindStrength");

    return fNewFrequencyTimeShift;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetWindStrengthAndLeafAngles definition

void CSpeedTreeRT::SetWindStrengthAndLeafAngles(float fNewStrength, const float* pRockingAngles, const float* pRustleAngles, unsigned int uiNumRockingAngles)
{
    try
    {
        if (fNewStrength >= 0.0f)
        {
            m_pWindEngine->SetWindStrengthAndLeafAngles(fNewStrength, pRockingAngles, pRustleAngles, uiNumRockingAngles);

            // invalidate geometry if CPU-based wind
            if (GetBranchWindMethod( ) == WIND_CPU &&
                m_pBranchGeometry)
                m_pBranchGeometry->Invalidate( );

            if (GetFrondWindMethod( ) == WIND_CPU &&
                m_pFrondGeometry)
                m_pFrondGeometry->Invalidate( );

            if (GetLeafWindMethod( ) == WIND_CPU ||
                m_pWindEngine->GetLeafRockingState( ))
                m_pLeafGeometry->Invalidate( );
        }
        else
            SetError("SetWindStrength() expects new wind strength >= 0.0");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetWindStrength")
    SpeedTreeCatchAll("CSpeedTreeRT::SetWindStrength");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetNumWindMatrices definition

void CSpeedTreeRT::SetNumWindMatrices(int nNumMatrices)
{
    try
    {
        CWindEngine::SetNumWindMatrices(static_cast<unsigned short>(nNumMatrices));
    }

    SpeedTreeCatch("CSpeedTreeRT::SetNumWindMatrices")
    SpeedTreeCatchAll("CSpeedTreeRT::SetNumWindMatrices");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetWindMatrix definition

void CSpeedTreeRT::SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix)
{
    st_assert(pMatrix);
    st_assert(nMatrixIndex >= 0 && nMatrixIndex < CWindEngine::GetNumWindMatrices( ));

    try
    {
        if (!pMatrix)
            SetError("SetWindMatrix() requires a non-NULL matrix pointer");
        else if (nMatrixIndex >= CWindEngine::GetNumWindMatrices( ))
            SetError("SetWindMatrix() - matrix index out of range");
        else
        {
            CWindEngine::SetWindMatrix(static_cast<unsigned short>(nMatrixIndex), pMatrix);
            NotifyAllTreesOfEvent(STIE_CLIENT_CHANGED_WIND);
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::SetWindMatrix")
    SpeedTreeCatchAll("CSpeedTreeRT::SetWindMatrix");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLocalMatrices definition

void CSpeedTreeRT::GetLocalMatrices(unsigned int& nStartingMatrix, unsigned int& nMatrixSpan)
{
    m_pWindEngine->GetLocalMatrices(nStartingMatrix, nMatrixSpan);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLocalMatrices definition

void CSpeedTreeRT::SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan)
{
    try
    {
        if (!m_bTreeComputed)
        {
            if (nStartingMatrix + nMatrixSpan - 1 < m_pWindEngine->GetNumWindMatrices( ))
                m_pWindEngine->SetLocalMatrices(nStartingMatrix, nMatrixSpan);
            else
                SetError("SetLocalMatrices() parameters exceed available wind matrices");
        }
        else
            SetError("SetLocalMatrices() has no effect after Compute() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLocalMatrices")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLocalMatrices");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ComputeLodLevel definition

void CSpeedTreeRT::ComputeLodLevel(void)
{
    try
    {
        if (m_pInstanceData)
        {
            // must activate instance's attributes, make the computation,
            // then restore the parent's attributes
            CVec3 cOldPosition = m_pEngine->GetPosition( );
            float fOldLodLevel = m_pEngine->GetLod( );

            // compute
            m_pEngine->SetPosition(m_pInstanceData->m_cPosition);
            m_pInstanceData->m_fLodLevel = m_pEngine->ComputeLod( );

            // restore
            m_pEngine->SetLod(fOldLodLevel);
            m_pEngine->SetPosition(cOldPosition);
        }
        else
            (void) m_pEngine->ComputeLod( );
    }

    SpeedTreeCatch("CSpeedTreeRT::ComputeLodLevel")
    SpeedTreeCatchAll("CSpeedTreeRT::ComputeLodLevel");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLodLevel definition

float CSpeedTreeRT::GetLodLevel(void) const
{
    float fLevel = -1.0f; // default to invalid value

    if (m_pInstanceData)
        fLevel = m_pInstanceData->m_fLodLevel;
    else
        fLevel = m_pEngine->GetLod( );

    return fLevel;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLodLevel definition

void CSpeedTreeRT::SetLodLevel(float fLevel)
{
    if (fLevel >= 0.0f && fLevel <= 1.0f)
    {
        if (m_pInstanceData)
            m_pInstanceData->m_fLodLevel = fLevel;
        else
            m_pEngine->SetLod(fLevel);
    }
    else
        SetError("SetLodLevel() expects a value in the range of 0.0 to 1.0");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetDropToBillboard definition

void CSpeedTreeRT::SetDropToBillboard(bool bFlag)
{
    m_bDropToBillboard = bFlag;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLodLimits definition

void CSpeedTreeRT::GetLodLimits(float& fNear, float& fFar) const
{
    m_pEngine->GetLodLimits(fNear, fFar);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLodLimits definition

void CSpeedTreeRT::SetLodLimits(float fNear, float fFar)
{
    m_pEngine->SetLodLimits(fNear, fFar);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetDiscreteBranchLodLevel definition

short CSpeedTreeRT::GetDiscreteBranchLodLevel(float fLodLevel) const
{
    short sLevel = 0;
    // -1.0f passed in means to use the current interval value
    if (fLodLevel == -1.0f)
        fLodLevel = GetLodLevel( );

    int nNumLodLevels = GetNumBranchLodLevels( );
    sLevel = static_cast<short>((1.0f - fLodLevel) * nNumLodLevels);
    if (sLevel == nNumLodLevels)
        sLevel--;

    st_assert(sLevel >= 0 && sLevel < GetNumBranchLodLevels( ));
    return sLevel;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetDiscreteFrondLodLevel definition

short CSpeedTreeRT::GetDiscreteFrondLodLevel(float fLodLevel) const
{
    short sLevel = 0;
    // -1.0f passed in means to use the current interval value
    if (fLodLevel == -1.0f)
        fLodLevel = GetLodLevel( );

    int nNumLodLevels = GetNumFrondLodLevels( );
    sLevel = static_cast<short>((1.0f - fLodLevel) * nNumLodLevels);
    if (sLevel == nNumLodLevels)
        sLevel--;

    st_assert(sLevel >= 0 && sLevel < GetNumFrondLodLevels( ));
    return sLevel;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetDiscreteLeafLodLevel definition

unsigned short CSpeedTreeRT::GetDiscreteLeafLodLevel(float fLodLevel) const
{
    unsigned short nLevel = 0;
    // -1.0f passed in means to use the current interval value
    if (fLodLevel == -1.0f)
        fLodLevel = GetLodLevel( );

    if (m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0)
    {
        int nNumLodLevels = GetNumLeafLodLevels( ) + 1;
        nLevel = static_cast<unsigned short>((1.0f - fLodLevel) * nNumLodLevels);
        if (nLevel == nNumLodLevels)
            nLevel--;

        st_assert(nLevel >= 0 && nLevel < nNumLodLevels);
    }
    else
    {
        int nNumLodLevels = GetNumLeafLodLevels( );
        nLevel = static_cast<unsigned short>((1.0f - fLodLevel) * nNumLodLevels);
        if (nLevel == nNumLodLevels)
            nLevel--;

        st_assert(nLevel >= 0 && nLevel < nNumLodLevels);
    }

    return nLevel;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetNumBranchLodLevels definition

unsigned short CSpeedTreeRT::GetNumBranchLodLevels(void) const
{
    return static_cast<unsigned short>(m_pEngine->GetNumBranchLodLevels( ));
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetNumLeafLodLevels definition

unsigned short CSpeedTreeRT::GetNumLeafLodLevels(void) const
{
    return static_cast<unsigned short>(m_pEngine->GetNumLeafLodLevels( ));
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetNumFrondLodLevels definition

unsigned short CSpeedTreeRT::GetNumFrondLodLevels(void) const
{
    return m_usNumFrondLodLevels;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::DeleteBranchGeometry definition

void CSpeedTreeRT::DeleteBranchGeometry(void)
{
    try
    {
        if (m_bTreeComputed &&
            m_pBranchGeometry &&
            !m_pInstanceData &&
            *m_pInstanceRefCount == 1)
        {
            delete m_pBranchGeometry;
            m_pBranchGeometry = NULL;
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::DeleteBranchGeometry")
    SpeedTreeCatchAll("CSpeedTreeRT::DeleteBranchGeometry");
}


///////////////////////////////////////////////////////////////////////
//  function RoundToNearestInt definition

inline int RoundToNearestInt(float fValue)
{
    int nIntValue = int(fValue);
    float fDiff = fValue - nIntValue;
    if (fDiff >= 0.5f)
        ++nIntValue;

    return nIntValue;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafBillboardTable definition

const float* CSpeedTreeRT::GetLeafBillboardTable(unsigned int& nEntryCount) const
{
    const float* pBillboardTable = NULL;
    try
    {
        pBillboardTable = m_pLeafGeometry->GetLeafBillboardTable(nEntryCount);
    }

    SpeedTreeCatch("CSpeedTreeRT::GetLeafBillboardTable")
    SpeedTreeCatchAll("CSpeedTreeRT::GetLeafBillboardTable");

    return pBillboardTable;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::Authorize definition

void CSpeedTreeRT::Authorize(const char* pKey)
{
    g_strKey = pKey;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::IsAuthorized definition

bool CSpeedTreeRT::IsAuthorized(void)
{
#ifdef FULL_VERSION
    return true;
#else
    string strFailure;
    return EvalIsValid(g_strKey, strFailure);
#endif
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetCurrentError definition

const char* CSpeedTreeRT::GetCurrentError(void)
{
    return g_strError.c_str( );
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ResetError definition

void CSpeedTreeRT::ResetError(void)
{
  g_strError = "";
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBoundingBox definition

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const
{
    if (pBounds)
        memcpy(pBounds, m_pTreeSizes, 6 * sizeof(float));
    else
        SetError("GetBoundingBox() expects a non-NULL parameter");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafTriangleCount definition

unsigned int CSpeedTreeRT::GetLeafTriangleCount(float fLodLevel) const
{
    int nTriangleCount = 0;
    int nDiscreteLevel = GetDiscreteLeafLodLevel(fLodLevel);
    if (m_eLeafLodMethod == LOD_NONE)
        nDiscreteLevel = 0;
    else if (nDiscreteLevel != -1)
        nTriangleCount = m_pLeafGeometry->GetTriangleCount(nDiscreteLevel);

    return nTriangleCount;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBranchTriangleCount definition

unsigned int CSpeedTreeRT::GetBranchTriangleCount(float fLodLevel) const
{
    unsigned int nTriangleCount = 0;
    try
    {
        int nDiscreteLevel = GetDiscreteBranchLodLevel(fLodLevel);
        if (nDiscreteLevel != -1 &&
            m_pBranchGeometry)
            nTriangleCount = m_pBranchGeometry->GetTriangleCount(static_cast<unsigned short>(nDiscreteLevel));
    }

    SpeedTreeCatch("CSpeedTreeRT::GetBranchTriangleCount")
    SpeedTreeCatchAll("CSpeedTreeRT::GetBranchTriangleCount");

    return nTriangleCount;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondTriangleCount definition

unsigned int CSpeedTreeRT::GetFrondTriangleCount(float fLodLevel) const
{
    unsigned int nTriangleCount = 0;
    try
    {
        int nDiscreteLevel = GetDiscreteFrondLodLevel(fLodLevel);
        if (nDiscreteLevel != -1 &&
            m_pFrondGeometry)
            nTriangleCount = m_pFrondGeometry->GetTriangleCount(static_cast<unsigned short>(nDiscreteLevel));
    }

    SpeedTreeCatch("CSpeedTreeRT::GetFrondTriangleCount")
    SpeedTreeCatchAll("CSpeedTreeRT::GetFrondTriangleCount");

    return nTriangleCount;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ComputeLeafStaticLighting definition

void CSpeedTreeRT::ComputeLeafStaticLighting(void)
{
    try
    {
        float afBoundingBox[6];
        GetBoundingBox(afBoundingBox);
        CVec3 cTreeCenter((afBoundingBox[0] + afBoundingBox[3]) * 0.5f,
                          (afBoundingBox[1] + afBoundingBox[4]) * 0.5f,
                          (afBoundingBox[2] + afBoundingBox[5]) * 0.5f);

        vector<CBillboardLeaf*>* pAllLeaves = m_pEngine->GetAllLeaves( );
        m_pLightingEngine->ComputeLeafStaticLighting(cTreeCenter, pAllLeaves, GetNumLeafLodLevels( ));
    }

    SpeedTreeCatch("CSpeedTreeRT::ComputeLeafStaticLighting")
    SpeedTreeCatchAll("CSpeedTreeRT::ComputeLeafStaticLighting");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::NotifyAllTreesOfEvent definition

void CSpeedTreeRT::NotifyAllTreesOfEvent(int nMessage)
{
    try
    {
        for (vector<CSpeedTreeRT*>::iterator i = SInstanceList::m_vUniqueTrees.begin( ); i != SInstanceList::m_vUniqueTrees.end( ); ++i)
        {
            if (nMessage == STIE_CLIENT_CHANGED_WIND)
            {
                if ((*i)->GetBranchWindMethod( ) == WIND_CPU)
                    (*i)->m_pBranchGeometry->Invalidate( );

                if ((*i)->GetFrondWindMethod( ) == WIND_CPU)
                    (*i)->m_pFrondGeometry->Invalidate( );

                if ((*i)->GetLeafWindMethod( ) == WIND_CPU ||
                    (*i)->m_pWindEngine->GetLeafRockingState( ))
                    (*i)->m_pLeafGeometry->Invalidate( );
            }
            else if (nMessage == STIE_CLIENT_CHANGED_CAMERA) // client changed camera
            {
                (*i)->m_pLeafGeometry->Invalidate( );
                (*i)->m_pSimpleBillboard->Invalidate( );
            }
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::NotifyAllTreesOfEvent")
    SpeedTreeCatchAll("CSpeedTreeRT::NotifyAllTreesOfEvent");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetError definition

void CSpeedTreeRT::SetError(const char* pError)
{
    g_strError = pError;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseLodInfo definition

void CSpeedTreeRT::ParseLodInfo(CTreeFileAccess* pFile)
{
    int nToken = pFile->ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_LeafTransitionMethod:
            m_eLeafLodMethod = static_cast<ELodMethod>(pFile->ParseInt( ));
            break;
        case File_LeafTransitionRadius:
            m_fLeafLodTransitionRadius = pFile->ParseFloat();
            break;
        case File_LeafCurveExponent:
            m_fLeafLodCurveExponent = pFile->ParseFloat( );
            break;
        case File_LeafSizeIncreaseFactor:
            m_fLeafSizeIncreaseFactor = pFile->ParseFloat( );
            break;
        case File_BeginEngineLodInfo:
            m_pEngine->ParseLodInfo(*pFile);
            break;
        default:
            throw(IdvFileError("malformed lod info"));
        }

        if (pFile->EndOfFile( ))
            throw(IdvFileError("premature end of file reached parsing new lod info"));
        else
            nToken = pFile->ParseToken( );

    } while (nToken != File_EndLodInfo);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseWindInfo definition

void CSpeedTreeRT::ParseWindInfo(CTreeFileAccess* pFile)
{
    int nToken = pFile->ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_WindLevel:
            m_pEngine->SetBranchLevelForWeighting(pFile->ParseInt( ));
            break;
        default:
            throw(IdvFileError("malformed new wind info"));
        }

        if (pFile->EndOfFile( ))
            throw(IdvFileError("premature end of file reached parsing new wind info"));
        else
            nToken = pFile->ParseToken( );

    } while (nToken != File_EndNewWindInfo);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseTextureCoordInfo definition

void CSpeedTreeRT::ParseTextureCoordInfo(CTreeFileAccess* pFile)
{
    m_pEmbeddedTexCoords = new SEmbeddedTexCoords;

    int nToken = pFile->ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_LeafTextureCoords:
        {
            m_pEmbeddedTexCoords->m_nNumLeafMaps = pFile->ParseInt( );
            if (m_pEmbeddedTexCoords->m_nNumLeafMaps > 0)
            {
                m_pEmbeddedTexCoords->m_pLeafTexCoords = new float[m_pEmbeddedTexCoords->m_nNumLeafMaps * 8];
                for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumLeafMaps; ++i)
                {
                    for (int j = 0; j < 8; ++j)
                        m_pEmbeddedTexCoords->m_pLeafTexCoords[i * 8 + j] = pFile->ParseFloat( );
                }
            }
        }
        break;
        case File_BillboardTextureCoords:
        {
            m_pEmbeddedTexCoords->m_nNumBillboards = pFile->ParseInt( );
            if (m_pEmbeddedTexCoords->m_nNumBillboards > 0)
            {
                m_pEmbeddedTexCoords->m_pBillboardTexCoords = new float[m_pEmbeddedTexCoords->m_nNumBillboards * 8];
                for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumBillboards; ++i)
                {
                    for (int j = 0; j < 8; ++j)
                    {
                        m_pEmbeddedTexCoords->m_pBillboardTexCoords[i * 8 + j] = pFile->ParseFloat( );
                        if (j % 2 && m_bTextureFlip) // flip T coordinates if necessary
                            m_pEmbeddedTexCoords->m_pBillboardTexCoords[i * 8 + j] = -m_pEmbeddedTexCoords->m_pBillboardTexCoords[i * 8 + j];
                    }
                }
            }
        }
        break;
        case File_FrondTextureCoords:
        {
            m_pEmbeddedTexCoords->m_nNumFrondMaps = pFile->ParseInt( );
            if (m_pEmbeddedTexCoords->m_nNumFrondMaps > 0)
            {
                m_pEmbeddedTexCoords->m_pFrondTexCoords = new float[m_pEmbeddedTexCoords->m_nNumFrondMaps * 8];
                for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumFrondMaps; ++i)
                {
                    for (int j = 0; j < 8; ++j)
                        m_pEmbeddedTexCoords->m_pFrondTexCoords[i * 8 + j] = pFile->ParseFloat( );
                }
            }
        }
        break;
        case File_CompositeFilename:
        {
            CIdvFilename cFile = pFile->ParseString( );
            m_pEmbeddedTexCoords->m_strCompositeFilename = cFile.NoPath( );
        }
        break;
        case File_HorizontalBillboard:
        {
            m_bHorizontalBillboard = pFile->ParseBool( );
        }
        break;
        case File_360Billboard:
        {
            m_b360Billboard = pFile->ParseBool( );
        }
        break;
        default:
            throw(IdvFileError("malformed texture coord info"));
        }

        if (pFile->EndOfFile( ))
            throw(IdvFileError("premature end of file reached parsing texture coordinate info"));
        else
            nToken = pFile->ParseToken( );

    } while (nToken != File_EndTextureCoordInfo);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetCollisionObjectCount definition

unsigned int CSpeedTreeRT::GetCollisionObjectCount( )
{
    unsigned int nCount = 0;

    if (m_pCollisionObjects)
        nCount = m_pCollisionObjects->m_vObjects.size( );

    return nCount;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetCollisionObject definition

void CSpeedTreeRT::GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
    if (m_pCollisionObjects)
    {
        if (nIndex >= m_pCollisionObjects->m_vObjects.size( ))
            SetError(IdvFormatString("collision object index (%d) exceeds maximum index (%d)", nIndex, m_pCollisionObjects->m_vObjects.size( )).c_str( ));
        else
        {
            eType = m_pCollisionObjects->m_vObjects[nIndex].m_eType;
            memcpy(pPosition, m_pCollisionObjects->m_vObjects[nIndex].m_afPosition, 3 * sizeof(float));
            memcpy(pDimensions, m_pCollisionObjects->m_vObjects[nIndex].m_afDimensions, 3 * sizeof(float));
        }
    }
    else
        SetError("no collision objects are stored with this tree");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseCollisionObjects definition

void CSpeedTreeRT::ParseCollisionObjects(CTreeFileAccess* pFile)
{
    m_pCollisionObjects = new SCollisionObjects;

    int nToken = pFile->ParseToken( );
    do
    {
        SShape sShape;
        switch (nToken)
        {
        case File_CollisionSphere:
            sShape.m_eType = CSpeedTreeRT::CO_SPHERE;
            break;
        case File_CollisionCylinder:
            sShape.m_eType = CSpeedTreeRT::CO_CYLINDER;
            break;
        case File_CollisionBox:
            sShape.m_eType = CSpeedTreeRT::CO_BOX;
            break;
        default:
            throw(IdvFileError("malformed collision object info"));
        }

        float afPosition[3];
        afPosition[0] = pFile->ParseFloat( );
        afPosition[1] = pFile->ParseFloat( );
        afPosition[2] = pFile->ParseFloat( );
        Assign3d(sShape.m_afPosition, afPosition);

        switch (sShape.m_eType)
        {
        case CSpeedTreeRT::CO_SPHERE:
            sShape.m_afDimensions[0] = pFile->ParseFloat( );    // radius
            break;
        case CSpeedTreeRT::CO_CYLINDER:
            sShape.m_afDimensions[0] = pFile->ParseFloat( );    // radius
            sShape.m_afDimensions[1] = pFile->ParseFloat( );    // height
            break;
        case CSpeedTreeRT::CO_BOX:
            sShape.m_afDimensions[0] = pFile->ParseFloat( );    // x
            sShape.m_afDimensions[1] = pFile->ParseFloat( );    // y
            sShape.m_afDimensions[2] = pFile->ParseFloat( );    // z
            break;
        default:
            throw(IdvFileError("unknown collision object type"));
        }

        m_pCollisionObjects->m_vObjects.push_back(sShape);

        if (pFile->EndOfFile( ))
            throw(IdvFileError("premature end of file reached parsing collision object info"));
        else
            nToken = pFile->ParseToken( );

    } while (nToken != File_EndCollisionInfo);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SaveTextureCoords definition

void CSpeedTreeRT::SaveTextureCoords(CTreeFileAccess* pFile) const
{
    if (m_pEmbeddedTexCoords)
    {
        pFile->SaveToken(File_BeginTextureCoordInfo);

        // leaves
        pFile->SaveToken(File_LeafTextureCoords);
        pFile->SaveInt(m_pEmbeddedTexCoords->m_nNumLeafMaps);
        for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumLeafMaps; ++i)
            for (int j = 0; j < 8; ++j)
                pFile->SaveFloat(m_pEmbeddedTexCoords->m_pLeafTexCoords[i * 8 + j]);

        // billboards
        pFile->SaveToken(File_BillboardTextureCoords);
        pFile->SaveInt(m_pEmbeddedTexCoords->m_nNumBillboards);
        for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumBillboards; ++i)
            for (int k = 0; k < 8; ++k)
                pFile->SaveFloat(m_pEmbeddedTexCoords->m_pBillboardTexCoords[i * 8 + k]);

        // fronds
        pFile->SaveToken(File_FrondTextureCoords);
        pFile->SaveInt(m_pEmbeddedTexCoords->m_nNumFrondMaps);
        for (int i = 0; i < m_pEmbeddedTexCoords->m_nNumFrondMaps; ++i)
            for (int l = 0; l < 8; ++l)
                pFile->SaveFloat(m_pEmbeddedTexCoords->m_pFrondTexCoords[i * 8 + l]);

    //    pFile->SaveToken(File_CompositeFilename);
    //    pFile->SaveString(m_pEmbeddedTexCoords->m_strCompositeFilename);
    //
    //    pFile->SaveToken(File_HorizontalBillboard);
    //    pFile->SaveBool(m_bHorizontalBillboard);
    //
    //    pFile->SaveToken(File_360Billboard);
    //    pFile->SaveBool(m_b360Billboard);

        pFile->SaveToken(File_EndTextureCoordInfo);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SaveCollisionObjects definition

void CSpeedTreeRT::SaveCollisionObjects(CTreeFileAccess* pFile) const
{
    if (m_pCollisionObjects)
    {
        pFile->SaveToken(File_BeginCollisionInfo);
        for (unsigned int i = 0; i < m_pCollisionObjects->m_vObjects.size( ); ++i)
        {
            SShape& sShape = m_pCollisionObjects->m_vObjects[i];
            switch (sShape.m_eType)
            {
            case CSpeedTreeRT::CO_SPHERE:
                pFile->SaveToken(File_CollisionSphere);
                break;
            case CSpeedTreeRT::CO_CYLINDER:
                pFile->SaveToken(File_CollisionCylinder);
                break;
            case CSpeedTreeRT::CO_BOX:
                pFile->SaveToken(File_CollisionBox);
                break;
            }

            pFile->SaveFloat(sShape.m_afPosition[0]);
            pFile->SaveFloat(sShape.m_afPosition[1]);
            pFile->SaveFloat(sShape.m_afPosition[2]);

            switch (sShape.m_eType)
            {
            case CSpeedTreeRT::CO_SPHERE:
                pFile->SaveFloat(sShape.m_afDimensions[0]);  // radius
                break;
            case CSpeedTreeRT::CO_CYLINDER:
                pFile->SaveFloat(sShape.m_afDimensions[0]);  // radius
                pFile->SaveFloat(sShape.m_afDimensions[1]);  // height
                break;
            case CSpeedTreeRT::CO_BOX:
                pFile->SaveFloat(sShape.m_afDimensions[0]);  // x
                pFile->SaveFloat(sShape.m_afDimensions[1]);  // y
                pFile->SaveFloat(sShape.m_afDimensions[2]);  // z
                break;
            }
        }
        pFile->SaveToken(File_EndCollisionInfo);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::DeleteFrondGeometry definition

void CSpeedTreeRT::DeleteFrondGeometry(void)
{
    try
    {
        if (m_bTreeComputed &&
            m_pFrondGeometry &&
            !m_pInstanceData &&
            m_pInstanceRefCount &&
            *m_pInstanceRefCount == 1)
        {
            delete m_pFrondGeometry;
            m_pFrondGeometry = NULL;
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::DeleteFrondGeometry")
    SpeedTreeCatchAll("CSpeedTreeRT::DeleteFrondGeometry");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondGeometryMapIndexes definition

unsigned char* CSpeedTreeRT::GetFrondGeometryMapIndexes(int nLodLevel) const
{
    unsigned char* pMapIndexes = NULL;
    try
    {
        if (m_pFrondGeometry)
        {
            unsigned short usNumStrips = m_pFrondGeometry->GetNumStrips(static_cast<unsigned short>(nLodLevel));
            if (usNumStrips > 0)
            {
                pMapIndexes = new unsigned char[usNumStrips];

                const unsigned short* pStripLengths = m_pFrondGeometry->GetStripLengthsPointer(static_cast<unsigned short>(nLodLevel));
                const unsigned short** pStrips = const_cast<const unsigned short**>(m_pFrondGeometry->GetStripsPointer(static_cast<unsigned short>(nLodLevel)));
                const unsigned char* pTexIndices = m_pFrondGeometry->GetVertexTexIndices( );

                for (int i = 0; i < usNumStrips; ++i)
                {
                    if (pStripLengths[i] == 0)
                        pMapIndexes[i] = 0;
                    else
                        pMapIndexes[i] = pTexIndices[pStrips[i][0]];
                }
            }
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::GetFrondGeometryMapIndexes( )")
    SpeedTreeCatchAll("CSpeedTreeRT::GetFrondGeometryMapIndexes( )");

    return pMapIndexes;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetGeometry definition

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
    st_assert(m_pEngine);

    try
    {
        if (ulBitVector & SpeedTree_BranchGeometry)
            GetBranchGeometry(sGeometry, sOverrideBranchLodValue); // compute fizzle-alpha value here
        if (ulBitVector & SpeedTree_FrondGeometry)
            GetFrondGeometry(sGeometry, sOverrideFrondLodValue); // compute fizzle-alpha value here
        if (ulBitVector & SpeedTree_LeafGeometry)
		{
			if (ulBitVector & SpeedTree_LeafLods)
	            GetLeafLods(sGeometry, sOverrideLeafLodValue);
			else
				GetLeafGeometry(sGeometry, sOverrideLeafLodValue);
		}
        if (ulBitVector & SpeedTree_BillboardGeometry)
        {
            if (m_b360Billboard && !(ulBitVector & SpeedTree_SimpleBillboardOverride))
                Get360BillboardGeometry(sGeometry, ulBitVector);
            else
                GetSimpleBillboardGeometry(sGeometry);
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::GetGeometry")
    SpeedTreeCatchAll("CSpeedTreeRT::GetGeometry");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetBranchGeometry definition

void CSpeedTreeRT::GetBranchGeometry(SGeometry& sGeometry, short sOverrideLodValue)
{
    if (m_pBranchGeometry)
    {
        SGeometry::SIndexed& sBranches = sGeometry.m_sBranches;

        // get vertex attributes
        sBranches.m_usVertexCount = m_pBranchGeometry->GetVertexCount( );
        sBranches.m_pColors = m_pBranchGeometry->GetVertexColors( );
        sBranches.m_pCoords = m_pBranchGeometry->GetVertexCoords( );
        sBranches.m_pNormals = m_pBranchGeometry->GetVertexNormals( );
        sBranches.m_pBinormals = m_pBranchGeometry->GetVertexBinormals( );
        sBranches.m_pTangents = m_pBranchGeometry->GetVertexTangents( );
        sBranches.m_pTexCoords0 = m_pBranchGeometry->GetVertexTexCoords0( );
        sBranches.m_pTexCoords1 = m_pBranchGeometry->GetVertexTexCoords1( );
        sBranches.m_pWindWeights = m_pBranchGeometry->GetVertexWindWeights( );
        sBranches.m_pWindMatrixIndices = m_pBranchGeometry->GetVertexWindMatrixIndices( );

        // get indexing data
        short sLodLevel = (sOverrideLodValue == -1) ? GetDiscreteBranchLodLevel( ) : sOverrideLodValue;

        sBranches.m_nDiscreteLodLevel = sLodLevel;
        sBranches.m_usNumStrips = m_pBranchGeometry->GetNumStrips(sLodLevel);
        sBranches.m_pStripLengths = m_pBranchGeometry->GetStripLengthsPointer(sLodLevel);
        sBranches.m_pStrips = const_cast<const unsigned short**>(m_pBranchGeometry->GetStripsPointer(sLodLevel));

        // determine alpha test value
        if (m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0)
        {
            unsigned short usLodStageCount = GetNumLeafLodLevels( ) + 1; // add one for billboard
            float fHighLodAlphaValue = -1.0f, fLowLodAlphaValue = -1.0f;
            short sHighLod = -1, sLowLod = -1;
            GetTransitionValues(GetLodLevel( ), usLodStageCount, m_fLeafLodTransitionRadius, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
                float(m_ucTargetAlphaValue), fHighLodAlphaValue, fLowLodAlphaValue, sHighLod, sLowLod);

            if (sHighLod == usLodStageCount - 2)
            {
                sGeometry.m_fBranchAlphaTestValue = fHighLodAlphaValue;
            }
            else if (sHighLod == usLodStageCount - 1)
            {
                sGeometry.m_fBranchAlphaTestValue = 255.0f; // completely off
                sBranches.m_nDiscreteLodLevel = -1;
            }
            else
            {
                sGeometry.m_fBranchAlphaTestValue = m_ucTargetAlphaValue;
            }
        }
        else
            sGeometry.m_fBranchAlphaTestValue = m_ucTargetAlphaValue;
    }
    else
        SetError("no branch geometry exists, possible prior call to DeleteBranchGeometry");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetFrondGeometry definition

void CSpeedTreeRT::GetFrondGeometry(SGeometry& sGeometry, short sOverrideLodValue)
{
    if (m_pFrondGeometry)
    {
        SGeometry::SIndexed& sFronds = sGeometry.m_sFronds;

        // get vertex attributes
        sFronds.m_usVertexCount = m_pFrondGeometry->GetVertexCount( );
        sFronds.m_pColors = m_pFrondGeometry->GetVertexColors( );
        sFronds.m_pCoords = m_pFrondGeometry->GetVertexCoords( );
        sFronds.m_pNormals = m_pFrondGeometry->GetVertexNormals( );
        sFronds.m_pBinormals = m_pFrondGeometry->GetVertexBinormals( );
        sFronds.m_pTangents = m_pFrondGeometry->GetVertexTangents( );
        sFronds.m_pTexCoords0 = m_pFrondGeometry->GetVertexTexCoords0( );
        sFronds.m_pTexCoords1 = m_pFrondGeometry->GetVertexTexCoords1( );
        sFronds.m_pWindWeights = m_pFrondGeometry->GetVertexWindWeights( );
        sFronds.m_pWindMatrixIndices = m_pFrondGeometry->GetVertexWindMatrixIndices( );

        // get indexing data
        short sLodLevel = (sOverrideLodValue == -1) ? GetDiscreteFrondLodLevel( ) : sOverrideLodValue;

        sFronds.m_nDiscreteLodLevel = sLodLevel;
        sFronds.m_usNumStrips = m_pFrondGeometry->GetNumStrips(sLodLevel);
        sFronds.m_pStripLengths = m_pFrondGeometry->GetStripLengthsPointer(sLodLevel);
        sFronds.m_pStrips = (const unsigned short**) m_pFrondGeometry->GetStripsPointer(sLodLevel);

        // determine alpha test value
        if (m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0)
        {
            unsigned short usLodStageCount = GetNumLeafLodLevels( ) + 1; // add one for billboard
            float fHighLodAlphaValue = -1.0f, fLowLodAlphaValue = -1.0f;
            short sHighLod = -1, sLowLod = -1;
            GetTransitionValues(GetLodLevel( ), usLodStageCount, m_fLeafLodTransitionRadius, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
                float(m_ucTargetAlphaValue), fHighLodAlphaValue, fLowLodAlphaValue, sHighLod, sLowLod);

            if (sHighLod == usLodStageCount - 2)
                sGeometry.m_fFrondAlphaTestValue = fHighLodAlphaValue;
            else if (sHighLod == usLodStageCount - 1)
            {
                sGeometry.m_fFrondAlphaTestValue = 255.0f; // completely off
                sFronds.m_nDiscreteLodLevel = -1;
            }
            else
                sGeometry.m_fFrondAlphaTestValue = m_ucTargetAlphaValue;
        }
        else
            sGeometry.m_fFrondAlphaTestValue = m_ucTargetAlphaValue;
    }
    else
        SetError("no frond geometry exists, possible prior call to DeleteFrondGeometry");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafGeometry definition

void CSpeedTreeRT::GetLeafGeometry(SGeometry& sGeometry, short sOverrideLeafLodValue)
{
    st_assert(m_pLeafGeometry);

    unsigned short usLeafLodCount = GetNumLeafLodLevels( );
    unsigned short usLodStageCount = usLeafLodCount + ((m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0) ? 1 : 0);

    // get camera attributes for billboarding
    CVec3 cCameraPos, cCameraDirection, cAdjustedDirection;
    CTreeEngine::GetCamera(cCameraPos, cCameraDirection);

    // direction should be adjusted for application coordinate system
    Assign3d(cAdjustedDirection, cCameraDirection);
    float fAzimuth = VecRad2Deg(atan2f(cAdjustedDirection[1], cAdjustedDirection[0]));
    float fPitch = -VecRad2Deg(asinf(cAdjustedDirection[2]));

    if (sOverrideLeafLodValue > -1)
    {
        st_assert(sOverrideLeafLodValue < GetNumLeafLodLevels( ));

        // higher LOD layer only (since the app is requesting a single discrete LOD)
        m_pLeafGeometry->Update(sGeometry.m_sLeaves0, sOverrideLeafLodValue, fAzimuth, fPitch, m_fLeafSizeIncreaseFactor);
        sGeometry.m_sLeaves0.m_bIsActive = true;
        sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;

        // no LOD layer
        sGeometry.m_sLeaves1.m_bIsActive = false;
    }
    else
    {
        // LOD_SMOOTH is most common
        if (m_eLeafLodMethod == LOD_SMOOTH)
        {
            // determine alpha values for both leaf layers
            float fHighLodAlphaValue = -1.0f, fLowLodAlphaValue = -1.0f;
            short sHighLod = -1, sLowLod = -1;
            GetTransitionValues(GetLodLevel( ), usLodStageCount, m_fLeafLodTransitionRadius, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
                float(m_ucTargetAlphaValue), fHighLodAlphaValue, fLowLodAlphaValue, sHighLod, sLowLod);

            // assign higher LOD layer
            sGeometry.m_sLeaves0.m_bIsActive = (sHighLod != -1 && sHighLod < usLeafLodCount);
            if (sGeometry.m_sLeaves0.m_bIsActive)
            {
                m_pLeafGeometry->Update(sGeometry.m_sLeaves0, sHighLod, fAzimuth, fPitch, m_fLeafSizeIncreaseFactor);
                sGeometry.m_sLeaves0.m_fAlphaTestValue = fHighLodAlphaValue;
            }

            // assign lower LOD layer
            sGeometry.m_sLeaves1.m_bIsActive = (sLowLod != -1 && sLowLod < usLeafLodCount);
            if (sGeometry.m_sLeaves1.m_bIsActive)
            {
                m_pLeafGeometry->Update(sGeometry.m_sLeaves1, sLowLod, fAzimuth, fPitch, m_fLeafSizeIncreaseFactor);
                sGeometry.m_sLeaves1.m_fAlphaTestValue = fLowLodAlphaValue;
            }
        }
        // no LOD is active, always use highest
        else if (m_eLeafLodMethod == LOD_NONE)
        {
            // set leaf layer 0
            m_pLeafGeometry->Update(sGeometry.m_sLeaves0, 0, fAzimuth, fPitch, m_fLeafSizeIncreaseFactor);
            sGeometry.m_sLeaves0.m_bIsActive = true;
            sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;

            // set leaf layer 1
            sGeometry.m_sLeaves1.m_bIsActive = false;
        }
        else // assume LOD_POP (do hard transitions between LODs)
        {
            // set leaf layer 0
            unsigned short usDiscreteLodLevel = GetDiscreteLeafLodLevel( );
            if (usDiscreteLodLevel < GetNumLeafLodLevels( ))
            {
                m_pLeafGeometry->Update(sGeometry.m_sLeaves0, usDiscreteLodLevel, fAzimuth, fPitch, m_fLeafSizeIncreaseFactor);
                sGeometry.m_sLeaves0.m_bIsActive = true;
                sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;
            }

            // set leaf layer 1
            sGeometry.m_sLeaves1.m_bIsActive = false;
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafLods definition

void CSpeedTreeRT::GetLeafLods(SGeometry& sGeometry, short sOverrideLeafLodValue)
{
    st_assert(m_pLeafGeometry);

    unsigned short usLeafLodCount = GetNumLeafLodLevels( );
    unsigned short usLodStageCount = usLeafLodCount + ((m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0) ? 1 : 0);

    // get camera attributes for billboarding
    CVec3 cCameraPos, cCameraDirection, cAdjustedDirection;
    CTreeEngine::GetCamera(cCameraPos, cCameraDirection);

    // direction should be adjusted for application coordinate system
    Assign3d(cAdjustedDirection, cCameraDirection);

    if (sOverrideLeafLodValue > -1)
    {
        st_assert(sOverrideLeafLodValue < GetNumLeafLodLevels( ));

        // higher LOD layer only (since the app is requesting a single discrete LOD)
        sGeometry.m_sLeaves0.m_nDiscreteLodLevel = sOverrideLeafLodValue;
        sGeometry.m_sLeaves0.m_bIsActive = true;
        sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;

        // no LOD layer
        sGeometry.m_sLeaves1.m_bIsActive = false;
    }
    else
    {
        // LOD_SMOOTH is most common
        if (m_eLeafLodMethod == LOD_SMOOTH)
        {
            // determine alpha values for both leaf layers
            float fHighLodAlphaValue = -1.0f, fLowLodAlphaValue = -1.0f;
            short sHighLod = -1, sLowLod = -1;
            GetTransitionValues(GetLodLevel( ), usLodStageCount, m_fLeafLodTransitionRadius, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
                float(m_ucTargetAlphaValue), fHighLodAlphaValue, fLowLodAlphaValue, sHighLod, sLowLod);

            // assign higher LOD layer
            sGeometry.m_sLeaves0.m_bIsActive = (sHighLod != -1 && sHighLod < usLeafLodCount);
            if (sGeometry.m_sLeaves0.m_bIsActive)
            {
                sGeometry.m_sLeaves0.m_nDiscreteLodLevel = sHighLod;
                sGeometry.m_sLeaves0.m_fAlphaTestValue = fHighLodAlphaValue;
            }

            // assign lower LOD layer
            sGeometry.m_sLeaves1.m_bIsActive = (sLowLod != -1 && sLowLod < usLeafLodCount);
            if (sGeometry.m_sLeaves1.m_bIsActive)
            {
                sGeometry.m_sLeaves1.m_nDiscreteLodLevel = sLowLod;
                sGeometry.m_sLeaves1.m_fAlphaTestValue = fLowLodAlphaValue;
            }
        }
        // no LOD is active, always use highest
        else if (m_eLeafLodMethod == LOD_NONE)
        {
            // set leaf layer 0
            sGeometry.m_sLeaves0.m_nDiscreteLodLevel = 0;
            sGeometry.m_sLeaves0.m_bIsActive = true;
            sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;
            
            // set leaf layer 1
            sGeometry.m_sLeaves1.m_bIsActive = false;
        }
        else // assume LOD_POP (do hard transitions between LODs)
        {
            // set leaf layer 0
            unsigned short usDiscreteLodLevel = GetDiscreteLeafLodLevel( );
            if (usDiscreteLodLevel < GetNumLeafLodLevels( ))
            {
                sGeometry.m_sLeaves0.m_nDiscreteLodLevel = usDiscreteLodLevel;
                sGeometry.m_sLeaves0.m_bIsActive = true;
                sGeometry.m_sLeaves0.m_fAlphaTestValue = m_ucTargetAlphaValue;
            }
            
            // set leaf layer 1
            sGeometry.m_sLeaves1.m_bIsActive = false;
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::Get360BillboardGeometry definition

void CSpeedTreeRT::Get360BillboardGeometry(SGeometry& sGeometry, unsigned long ulBitVector)
{
    GetSimpleBillboardGeometry(sGeometry);

    if (sGeometry.m_sBillboard0.m_bIsActive &&
        m_pEmbeddedTexCoords)
    {
        // the call to GetSimpleBillboardGoemetry() determines an alpha value that
        // reflects how much of the billboard should be seen vs. the 3D tree.  we
        // need to convert that to a floating scalar to multiply against the cross
        // fading alpha's determined below
        float fFade = (sGeometry.m_sBillboard0.m_fAlphaTestValue - m_ucTargetAlphaValue) / (255.0f - m_ucTargetAlphaValue);

        // get camera attributes
        CVec3 cCameraPos, cCameraDir;
        CTreeEngine::GetCamera(cCameraPos, cCameraDir);
        cCameraDir = -cCameraDir;
        float fAzimuth = ComputeAzimuth(cCameraDir);
        if (fAzimuth < 0.0f)
            fAzimuth += 360.0f;

        // find blending/LOD values for 360 degree blending between two billboards
        float fPrimaryLodAlphaValue = -1.0f, fSecondaryLodAlphaValue = -1.0f;
        short sPrimaryLod = -1, sSecondaryLod = -1;
        unsigned short usLodStageCount = static_cast<unsigned short>(m_pEmbeddedTexCoords->m_nNumBillboards);
        float fLodWidth = 0.5f / usLodStageCount;
        float fLodValue = VecInterpolate(fLodWidth, 1.0f - fLodWidth, fAzimuth / 360.0f);
        GetTransitionValues(fLodValue, usLodStageCount, 0.5f / usLodStageCount, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
            float(m_ucTargetAlphaValue), fPrimaryLodAlphaValue, fSecondaryLodAlphaValue, sPrimaryLod, sSecondaryLod);

        // reverse them to account for leaf LOD reversed behavior
        sPrimaryLod = static_cast<short>((m_pEmbeddedTexCoords->m_nNumBillboards - 1) - sPrimaryLod);
        sSecondaryLod = static_cast<unsigned short>((m_pEmbeddedTexCoords->m_nNumBillboards - 1) - sSecondaryLod);

        // wrap index value if necessary
        if (sPrimaryLod == usLodStageCount - 1)
            sPrimaryLod = 0;
        if (sSecondaryLod == usLodStageCount - 1)
            sSecondaryLod = 0;

        if (ulBitVector & SpeedTree_Nearest360Override)
        {
            sGeometry.m_sBillboard0.m_bIsActive = true;

            // which billboard is dominant?
            if (fPrimaryLodAlphaValue < fSecondaryLodAlphaValue)
            {
                sGeometry.m_sBillboard0.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + sPrimaryLod * 8;
                sGeometry.m_sBillboard0.m_fAlphaTestValue = VecInterpolate(float(m_ucTargetAlphaValue), 255.0f, fFade);
                sGeometry.m_sBillboard1.m_bIsActive = false;
            }
            else
            {
                sGeometry.m_sBillboard0.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + sSecondaryLod * 8;
                sGeometry.m_sBillboard0.m_fAlphaTestValue = VecInterpolate(float(m_ucTargetAlphaValue), 255.0f, fFade);
                sGeometry.m_sBillboard1.m_bIsActive = false;
            }
        }
        else
        {
            // assign texture coordinates - the 0 (zero) can be a billboard index ranging from
            // zero to m_pEmbeddedTexCoords->m_nNumBillboards - 1
            sGeometry.m_sBillboard0.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + sPrimaryLod * 8;
            sGeometry.m_sBillboard0.m_fAlphaTestValue = VecInterpolate(fPrimaryLodAlphaValue, 255.0f, fFade);

            sGeometry.m_sBillboard1.m_pCoords = sGeometry.m_sBillboard0.m_pCoords;
            sGeometry.m_sBillboard1.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + sSecondaryLod * 8;
            sGeometry.m_sBillboard1.m_bIsActive = true;
            sGeometry.m_sBillboard1.m_fAlphaTestValue = VecInterpolate(fSecondaryLodAlphaValue, 255.0f, fFade);
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetSimpleBillboardGeometry definition

void CSpeedTreeRT::GetSimpleBillboardGeometry(SGeometry& sGeometry)
{
    // determine alpha test value and visibility
    if (m_bDropToBillboard && m_pEmbeddedTexCoords && m_pEmbeddedTexCoords->m_nNumBillboards > 0)
    {
        unsigned short usLodStageCount = GetNumLeafLodLevels( ) + 1; // add one for billboard
        float fHighLodAlphaValue = -1.0f, fLowLodAlphaValue = -1.0f;
        short sHighLod = -1, sLowLod = -1;
        GetTransitionValues(GetLodLevel( ), usLodStageCount, m_fLeafLodTransitionRadius, m_fLeafTransitionFactor, m_fLeafLodCurveExponent,
            float(m_ucTargetAlphaValue), fHighLodAlphaValue, fLowLodAlphaValue, sHighLod, sLowLod);

        // determine if either leaf LOD level should be a billboard
        sGeometry.m_sBillboard0.m_fAlphaTestValue = -1.0f;
        if (sHighLod == usLodStageCount - 1)
            sGeometry.m_sBillboard0.m_fAlphaTestValue = fHighLodAlphaValue;
        else if (sLowLod == usLodStageCount - 1)
            sGeometry.m_sBillboard0.m_fAlphaTestValue = fLowLodAlphaValue;

        // setup first billboard layer
        sGeometry.m_sBillboard0.m_bIsActive = (sGeometry.m_sBillboard0.m_fAlphaTestValue != -1.0f);
        if (sGeometry.m_sBillboard0.m_bIsActive)
        {
            // find appropriate coordinates
            #ifdef UPVECTOR_POS_Z
				sGeometry.m_sBillboard0.m_pCoords = m_pSimpleBillboard->GetBillboardCoords(m_pTreeSizes[STS_BB_SIZE], m_pTreeSizes[STS_MAX_BOX + 2]);
			#elif defined UPVECTOR_NEG_Z
				sGeometry.m_sBillboard0.m_pCoords = m_pSimpleBillboard->GetBillboardCoords(m_pTreeSizes[STS_BB_SIZE], fabs(m_pTreeSizes[STS_MIN_BOX + 2]));
            #else
                sGeometry.m_sBillboard0.m_pCoords = m_pSimpleBillboard->GetBillboardCoords(m_pTreeSizes[STS_BB_SIZE], fabs(m_pTreeSizes[STS_MAX_BOX + 1]));
            #endif

            // assign texture coordinates - the 0 (zero) can be a billboard index ranging from
            // zero to m_pEmbeddedTexCoords->m_nNumBillboards - 1
            sGeometry.m_sBillboard0.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + 0 * 8;;
        }

        // second billboard layer is always off for simple billboard
        sGeometry.m_sBillboard1.m_bIsActive = false;

        // horizontal billboard
        if (m_bHorizontalBillboard)
        {
            // always the plane that bisects the bounding box (computed once at initialization)
            sGeometry.m_sHorizontalBillboard.m_pCoords = m_afHorizontalCoords;

            // the horizontal billboard is always the last billboard in the billboard sequence
            sGeometry.m_sHorizontalBillboard.m_pTexCoords = m_pEmbeddedTexCoords->m_pBillboardTexCoords + (m_pEmbeddedTexCoords->m_nNumBillboards - 1) * 8;
            sGeometry.m_sHorizontalBillboard.m_fAlphaTestValue = sGeometry.m_sBillboard0.m_fAlphaTestValue;
            sGeometry.m_sHorizontalBillboard.m_bIsActive = sGeometry.m_sHorizontalBillboard.m_fAlphaTestValue >= 0.0f;

            // compute the alpha fade value based on the camera angle
            sGeometry.m_sHorizontalBillboard.m_fAlphaTestValue = m_fHorizontalFadeValue;
        }
        else
            sGeometry.m_sHorizontalBillboard.m_bIsActive = false;
    }
    else
    {
        sGeometry.m_sBillboard0.m_bIsActive = false;
        sGeometry.m_sBillboard1.m_bIsActive = false;
        sGeometry.m_sHorizontalBillboard.m_bIsActive = false;
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetTransitionValues definition
//
//  Given the function parameters, this function will return by reference
//  two values that are used for alpha test fading between two discrete LODs.
//
//      fLodLevel               current LOD level, 0.0 = low, 1.0 = high
//      usLodCount              number of discrete LOD levels
//      fOverlapRadius          in LOD units (0.0, 1.0) amount of overlap between two neighboring LODs
//      fTransitionFactor       (0.0, 0.5) point at which overlap occurs
//      fHighValue              alpha value for higher LOD (0, 255)
//      fLowValue               alpha value for lower LOD (0, 255)

void CSpeedTreeRT::GetTransitionValues(float fLodLevel, unsigned short usLodCount, float fOverlapRadius,
                                       float fTransitionFactor, float fCurveExponent, float fTargetAlphaValue,
                                       float& fHighValue, float& fLowValue, short& sHighLod, short& sLowLod)
{
    float fLodWidth = 1.0f / usLodCount;
    unsigned short usDiscreteTransitionPoint = static_cast<unsigned short>(RoundToNearestInt((1.0f - fLodLevel) / fLodWidth));
    float fDistanceToTransitionPoint = (1.0f - fLodLevel) - usDiscreteTransitionPoint * fLodWidth;

    // check to see if there is no overlap
    if (usDiscreteTransitionPoint == 0 ||
        usDiscreteTransitionPoint == usLodCount ||
        fabs(fDistanceToTransitionPoint) > fOverlapRadius)
    {
        // high LOD
        fHighValue = fTargetAlphaValue;
        sHighLod = static_cast<short>((1.0f - fLodLevel) * usLodCount);
        sHighLod = __min(sHighLod, static_cast<short>(usLodCount - 1));

        // low LOD (inactive)
        fLowValue = 255.0f;
        sLowLod = -1;
    }
    else // yes there is an overlap, so two factors are needed
    {
        // transition stage ranges from 0.0 (high LOD side) to 1.0 (low LOD side)
        float fTransitionStage = 1.0f - (fOverlapRadius - fDistanceToTransitionPoint) / (2.0f * fOverlapRadius);

        // high LOD
        float fAmountOfHigh = 1.0f - (fTransitionStage - fTransitionFactor) / (1.0f - fTransitionFactor);
        fAmountOfHigh = __min(fAmountOfHigh, 1.0f); // cap at 1.0
        sHighLod = usDiscreteTransitionPoint - 1;
        fHighValue = VecInterpolate(fTargetAlphaValue, 255.0f, powf((1.0f - fAmountOfHigh), fCurveExponent));

        // low LOD
        float fAmountOfLow = fTransitionStage / (1.0f - fTransitionFactor);
        fAmountOfLow = __min(fAmountOfLow, 1.0f); // cap at 1.0
        sLowLod = usDiscreteTransitionPoint;
        fLowValue = VecInterpolate(fTargetAlphaValue, 255.0f, powf((1.0f - fAmountOfLow), fCurveExponent));
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetLeafLodSizeAdjustments definition

const float* CSpeedTreeRT::GetLeafLodSizeAdjustments(void)
{
    try
    {
        if (!m_pLeafLodSizeFactors)
        {
            int nNumLods = GetNumLeafLodLevels( );
            m_pLeafLodSizeFactors = new float[nNumLods];
            for (int i = 0; i < nNumLods; ++i)
                m_pLeafLodSizeFactors[i] = (nNumLods > 0) ? 1.0f + m_fLeafSizeIncreaseFactor * float(i) : 1.0f;
        }
    }

    SpeedTreeCatch("CSpeedTreeRT::GetLeafLodSizeAdjustments")
    SpeedTreeCatchAll("CSpeedTreeRT::GetLeafLodSizeAdjustments");

    return m_pLeafLodSizeFactors;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseShadowProjectionInfo definition

void CSpeedTreeRT::ParseShadowProjectionInfo(CTreeFileAccess* pFile)
{
    m_pProjectedShadow = new CProjectedShadow;

    m_pProjectedShadow->Parse(*pFile);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ComputeSelfShadowTexCoords definition

void CSpeedTreeRT::ComputeSelfShadowTexCoords(void)
{
    if (m_pBranchGeometry &&
        m_pFrondGeometry &&
        m_pProjectedShadow)
    {
        // compute the radius of the circumscribing sphere
        CVec3 cMin(m_pTreeSizes[0], m_pTreeSizes[1], m_pTreeSizes[2]), cMax(m_pTreeSizes[3], m_pTreeSizes[4], m_pTreeSizes[5]);
        float fRadius = cMin.Distance(cMax) * 0.5f;

        // compute the center
        CVec3 cCenter = (cMin + cMax) * 0.5f;

        if (m_pEmbeddedTexCoords)
        {
            // compute branch tex coords
            m_pProjectedShadow->ComputeTexCoords(m_pBranchGeometry, cCenter, fRadius, m_pEmbeddedTexCoords->m_afShadowTexCoords);

            // compute frond tex coords
            m_pProjectedShadow->ComputeTexCoords(m_pFrondGeometry, cCenter, fRadius, m_pEmbeddedTexCoords->m_afShadowTexCoords);
        }
        else
        {
            // compute branch tex coords
            m_pProjectedShadow->ComputeTexCoords(m_pBranchGeometry, cCenter, fRadius);

            // compute frond tex coords
            m_pProjectedShadow->ComputeTexCoords(m_pFrondGeometry, cCenter, fRadius);
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetTextures definition

void CSpeedTreeRT::GetTextures(CSpeedTreeRT::STextures& sTextures) const
{
    try
    {
        // branch
        sTextures.m_pBranchTextureFilename = m_pEngine->GetBranchTextureFilename( ).c_str( );

        // leaves
        const vector<SIdvLeafTexture>& vLeaves = m_pEngine->GetLeafTextures( );
        sTextures.m_uiLeafTextureCount = vLeaves.size( );
        if (sTextures.m_uiLeafTextureCount > 0)
        {
            sTextures.m_pLeafTextureFilenames = new const char*[sTextures.m_uiLeafTextureCount];

            for (unsigned int i = 0; i < vLeaves.size( ); ++i)
                sTextures.m_pLeafTextureFilenames[i] = vLeaves[i].m_strFilename.c_str( );
        }
        else
            sTextures.m_pLeafTextureFilenames = NULL;

        // fronds
        sTextures.m_uiFrondTextureCount = m_pFrondEngine->GetTextureCount( );
        if (sTextures.m_uiFrondTextureCount > 0)
        {
            sTextures.m_pFrondTextureFilenames = new const char*[sTextures.m_uiFrondTextureCount];
            for (unsigned int i = 0; i < sTextures.m_uiFrondTextureCount; ++i)
                sTextures.m_pFrondTextureFilenames[i] = m_pFrondEngine->GetTextureFilename(i);
        }
        else
            sTextures.m_pFrondTextureFilenames = NULL;

        // self shadow
        if (m_pProjectedShadow)
            sTextures.m_pSelfShadowFilename = m_pProjectedShadow->GetSelfShadowFilename( );

        // composite
        if (m_pEmbeddedTexCoords)
            sTextures.m_pCompositeFilename = m_pEmbeddedTexCoords->m_strCompositeFilename.c_str( );
    }

    SpeedTreeCatch("CSpeedTreeRT::GetTextures")
    SpeedTreeCatchAll("CSpeedTreeRT::GetTextures");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafTextureCoords definition

void CSpeedTreeRT::SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords)
{
    st_assert(m_pEngine);
    st_assert(nLeafMapIndex >= 0);
    st_assert(pTexCoords);
    st_assert(m_pLeafGeometry);

    try
    {
        st_assert(nLeafMapIndex < m_pEngine->GetLeafTextures( ).size( ));
        m_pLeafGeometry->SetTextureCoords(nLeafMapIndex, pTexCoords);
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafTextureCoords")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafTextureCoords");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetFrondTextureCoords definition

void CSpeedTreeRT::SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords)
{
    try
    {
        if (m_pFrondGeometry &&
            pTexCoords)
            m_pFrondEngine->SetTextureCoords(m_pFrondGeometry, nFrondMapIndex, pTexCoords, m_bTextureFlip);
    }

    SpeedTreeCatch("CSpeedTreeRT::SetFrondTextureCoords")
    SpeedTreeCatchAll("CSpeedTreeRT::SetFrondTextureCoords");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SaveUserData definition

void CSpeedTreeRT::SaveUserData(CTreeFileAccess* pFile) const
{
    if (m_pUserData)
    {
        pFile->SaveToken(File_BeginUserData);

        pFile->SaveToken(File_UserData);
        pFile->SaveString(string(m_pUserData));

        pFile->SaveToken(File_EndUserData);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseUserData definition

void CSpeedTreeRT::ParseUserData(CTreeFileAccess* pFile)
{
    int nToken = pFile->ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_UserData:
            m_pUserData = CopyUserData(pFile->ParseString( ).c_str( ));
            break;
        }
        nToken = pFile->ParseToken( );

    } while (nToken != File_EndUserData);
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetUserData definition

const char* CSpeedTreeRT::GetUserData(void) const
{
    return m_pUserData;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetupHorizontalBillboard definition

void CSpeedTreeRT::SetupHorizontalBillboard(void)
{
    if (m_pEmbeddedTexCoords && m_bHorizontalBillboard)
    {
        float afMin[3];;
        Assign3d(afMin, m_pTreeSizes + 0);
        float afMax[3];
        Assign3d(afMax, m_pTreeSizes + 3);

        float fHalfHeight = (afMin[2] + afMax[2]) * 0.5f;

        // ccw from max x, max y to match the stored tex coords
        //m_afHorizontalCoords[0] = m_pTreeSizes[3];
        //m_afHorizontalCoords[1] = m_pTreeSizes[4];
        //m_afHorizontalCoords[2] = fHalfHeight;
        Assign3d(m_afHorizontalCoords + 0, CVec3(afMax[0], afMax[1], fHalfHeight));

        //m_afHorizontalCoords[3] = m_pTreeSizes[0];
        //m_afHorizontalCoords[4] = m_pTreeSizes[4];
        //m_afHorizontalCoords[5] = fHalfHeight;
        Assign3d(m_afHorizontalCoords + 3, CVec3(afMin[0], afMax[1], fHalfHeight));

        //m_afHorizontalCoords[6] = m_pTreeSizes[0];
        //m_afHorizontalCoords[7] = m_pTreeSizes[1];
        //m_afHorizontalCoords[8] = fHalfHeight;
        Assign3d(m_afHorizontalCoords + 6, CVec3(afMin[0], afMin[1], fHalfHeight));

        //m_afHorizontalCoords[9] = m_pTreeSizes[3];
        //m_afHorizontalCoords[10] = m_pTreeSizes[1];
        //m_afHorizontalCoords[11] = fHalfHeight;
        Assign3d(m_afHorizontalCoords + 9, CVec3(afMax[0], afMin[1], fHalfHeight));
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::GetTextureFlip definition

bool CSpeedTreeRT::GetTextureFlip(void)
{
    return m_bTextureFlip;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetTextureFlip definition

void CSpeedTreeRT::SetTextureFlip(bool bFlag)
{
    m_bTextureFlip = bFlag;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::CopyUserData definition
//
//  unsigned int uiSize does NOT include the null terminator

char* CSpeedTreeRT::CopyUserData(const char* pOrig)
{
    int nLength = strlen(pOrig);

    char* pCopy = new char[nLength + 1];
    memcpy(pCopy, pOrig, nLength + 1);

    return pCopy;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetBranchTextureFilename definition

void CSpeedTreeRT::SetBranchTextureFilename(const char* pFilename)
{
    st_assert(m_pEngine);

    try
    {
        if (m_pEngine->TransientDataIntact( ))
            m_pEngine->SetBranchTextureFilename(pFilename);
        else
            SetError("SetBranchTextureFilename() has no effect after DeleteTransientData() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetBranchTextureFilename")
    SpeedTreeCatchAll("CSpeedTreeRT::SetBranchTextureFilename");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetLeafTextureFilename definition

void CSpeedTreeRT::SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename)
{
    st_assert(m_pEngine);

    try
    {
        if (m_pEngine->TransientDataIntact( ))
            m_pEngine->SetLeafTextureFilename(nLeafMapIndex, pFilename);
        else
            SetError("SetLeafTextureFilename() has no effect after DeleteTransientData() has been called");
    }

    SpeedTreeCatch("CSpeedTreeRT::SetLeafTextureFilename")
    SpeedTreeCatchAll("CSpeedTreeRT::SetLeafTextureFilename");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SetFrondTextureFilename definition

void CSpeedTreeRT::SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename)
{
    st_assert(m_pFrondEngine);

    try
    {
        m_pFrondEngine->SetTextureFilename(nFrondMapIndex, pFilename);
    }

    SpeedTreeCatch("CSpeedTreeRT::SetFrondTextureFilename")
    SpeedTreeCatchAll("CSpeedTreeRT::SetFrondTextureFilename");
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::SaveSupplementalTexCoordInfo definition

void CSpeedTreeRT::SaveSupplementalTexCoordInfo(CTreeFileAccess* pFile) const
{
    if (m_pEmbeddedTexCoords)
    {
        pFile->SaveToken(File_BeginSupplementalTexCoordInfo);

        pFile->SaveToken(File_SupplementalCompositeFilename);
        pFile->SaveString(m_pEmbeddedTexCoords->m_strCompositeFilename);

        pFile->SaveToken(File_SupplementalHorizontalBillboard);
        pFile->SaveBool(m_bHorizontalBillboard);

        pFile->SaveToken(File_Supplemental360Billboard);
        pFile->SaveBool(m_b360Billboard);

        pFile->SaveToken(File_SupplementalShadowTexCoords);
        for (int i = 0; i < 8; ++i)
            pFile->SaveFloat(m_pEmbeddedTexCoords->m_afShadowTexCoords[i]);

        pFile->SaveToken(File_EndSupplementalTexCoordInfo);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseSupplementalTexCoordInfo definition

void CSpeedTreeRT::ParseSupplementalTexCoordInfo(CTreeFileAccess* pFile)
{
    if (m_pEmbeddedTexCoords)
    {
        int nToken = pFile->ParseToken( );
        do
        {
            switch (nToken)
            {
            case File_SupplementalCompositeFilename:
            {
                CIdvFilename cFile = pFile->ParseString( );
                m_pEmbeddedTexCoords->m_strCompositeFilename = cFile.NoPath( );
            }
            break;
            case File_SupplementalHorizontalBillboard:
            {
                m_bHorizontalBillboard = pFile->ParseBool( );
            }
            break;
            case File_Supplemental360Billboard:
            {
                m_b360Billboard = pFile->ParseBool( );
            }
            break;
            case File_SupplementalShadowTexCoords:
            {
                for (int i = 0; i < 8; ++i)
                    m_pEmbeddedTexCoords->m_afShadowTexCoords[i] = pFile->ParseFloat( );
            }
            break;
            default:
                throw(IdvFileError("malformed texture coord info"));
            }

            if (pFile->EndOfFile( ))
                throw(IdvFileError("premature end of file reached parsing texture coordinate info"));
            else
                nToken = pFile->ParseToken( );

        } while (nToken != File_EndSupplementalTexCoordInfo);
    }
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseSupplementalTexCoordInfo definition

void CSpeedTreeRT::SetHorzBillboardFadeAngles(float fStart, float fEnd)
{
    m_fHorizontalFadeStartAngle = fStart;
    m_fHorizontalFadeEndAngle = fEnd;
}


///////////////////////////////////////////////////////////////////////
//  CSpeedTreeRT::ParseSupplementalTexCoordInfo definition

void CSpeedTreeRT::GetHorzBillboardFadeAngles(float& fStart, float& fEnd)
{
    fStart = m_fHorizontalFadeStartAngle;
    fEnd = m_fHorizontalFadeEndAngle;
}


