///////////////////////////////////////////////////////////////////////  
//         Name: LeafGeometry.cpp
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
#include "SpeedTreeRT.h"
#include "BillboardLeaf.h"
#include "LeafGeometry.h"
#include "WindEngine.h"
#include "UpVector.h"
#include "Endian.h"


///////////////////////////////////////////////////////////////////////  
//  SLodGeometry::SLodGeometry definition

CLeafGeometry::SLodGeometry::SLodGeometry( ) :
    m_bValid(false),
    m_pOrigCenterCoords(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  SLodGeometry::~SLodGeometry definition

CLeafGeometry::SLodGeometry::~SLodGeometry( )
{
    // delete variables inherited from CSpeedTreeRT::SGeometry::SLeaf
    delete[] const_cast<unsigned char*>(m_pLeafMapIndices);
    delete[] const_cast<unsigned char*>(m_pLeafClusterIndices);
    delete[] const_cast<float*>(m_pCenterCoords);
    delete[] const_cast<unsigned long*>(m_pColors);
    delete[] const_cast<float*>(m_pNormals);
    delete[] const_cast<float*>(m_pBinormals);
    delete[] const_cast<float*>(m_pTangents);
    delete[] const_cast<float**>(m_pLeafMapTexCoords);
    delete[] const_cast<float**>(m_pLeafMapCoords);
    delete[] const_cast<float*>(m_pWindWeights);
    delete[] const_cast<unsigned char*>(m_pWindMatrixIndices);

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

    // delete variable declared in SLodGeometry
    delete[] m_pOrigCenterCoords;
    m_pOrigCenterCoords = NULL;
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::CLeafGeometry definition

CLeafGeometry::CLeafGeometry(CWindEngine* pWindEngine) :
    m_bManualLighting(false),
    m_bVertexWeighting(false),
    // wind
    m_pWindEngine(pWindEngine),
    // compute tables
    m_usNumRockingGroups(3),
    m_pTimeOffsets(NULL),
    m_pLeafVertexes(NULL),
    m_pLeafTexCoords(NULL),
    m_pVertexProgramTable(NULL),
    // texture information
    m_usNumTextures(0),
    m_pLeafDimensions(NULL),
    m_pLeafOrigins(NULL),
    // LODs
    m_usNumLods(0),
    m_pLods(NULL)
{
    st_assert(pWindEngine);
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::~CLeafGeometry definition

CLeafGeometry::~CLeafGeometry( )
{
    delete[] m_pLeafDimensions;
    delete[] m_pLeafOrigins;
    delete[] m_pLods;
    delete[] m_pVertexProgramTable;
    
    m_pTimeOffsets = NULL;
    m_pLeafTexCoords = NULL;
    m_pLeafDimensions = NULL;
    m_pLeafOrigins = NULL;
    m_pLods = NULL;
    m_pVertexProgramTable = NULL;
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::Invalidate definition

void CLeafGeometry::Invalidate(void)
{
    if (m_pLods)
        for (int i = 0; i < m_usNumLods; ++i)
            m_pLods[i].Invalidate( );
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::GetLeafBillboardTable definition

const float* CLeafGeometry::GetLeafBillboardTable(unsigned int& nEntryCount) const
{
    // copy the leaf map position coordinates
    float* pTable = m_pVertexProgramTable;
    int nLodSize = m_usNumTextures * m_usNumRockingGroups * 2 * 4 * 4; // 2 for mirrored, 4 corners per leaf, 4 floats per vertex
    if (pTable && m_pLeafVertexes && m_pLeafVertexes[0] && m_pVertexProgramTable)
    {
        memcpy(pTable, m_pLeafVertexes[0], nLodSize * sizeof(float));
        pTable += nLodSize;

        // calculate total floats in table
        nEntryCount = pTable - m_pVertexProgramTable;
    }

    return m_pVertexProgramTable;
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::ComputeExtents definition
//
//  Because the leaf clusters rock backand forth, taking their
//  contribution to the tree's overall bounding box is not
//  very straightforward.
//
//  Each leaf has a position, a size, and a leafmap origin.  The
//  longest diagonal from the origin the four corner is computed
//  for each leaf map type.  Those are then added to the leaf
//  positions throughout the tree in all six orthographic 
//  directions.

void CLeafGeometry::ComputeExtents(CRegion& cExtents) const
{
    if (m_pLeafOrigins &&
        m_pLeafDimensions &&
        m_pLods)
    {
        // for each basec leafmap type (determined by how many leaf texture
        // maps were specified in CAD, determine the longest diagonals.
        vector<float> vLongestDiagonal;
        for (int i = 0; i < m_usNumTextures; ++i)
        {
            const CVec3 cScaledOrigin(m_pLeafOrigins[i][0] * m_pLeafDimensions[i][0], 
                                    m_pLeafOrigins[i][1] * m_pLeafDimensions[i][1]);
            float fLongest = 0.0f;
            if (cScaledOrigin.Distance(CVec3(0.0f, 0.0f)) > fLongest)
                fLongest = cScaledOrigin.Distance(CVec3(0.0f, 0.0f));

            if (cScaledOrigin.Distance(CVec3(m_pLeafDimensions[i][0], 0.0f)) > fLongest)
                fLongest = cScaledOrigin.Distance(CVec3(m_pLeafDimensions[i][0], 0.0f));

            if (cScaledOrigin.Distance(CVec3(m_pLeafDimensions[i][0], m_pLeafDimensions[i][1])) > fLongest)
                fLongest = cScaledOrigin.Distance(CVec3(m_pLeafDimensions[i][0], m_pLeafDimensions[i][1]));

            if (cScaledOrigin.Distance(CVec3(0.0f, m_pLeafDimensions[i][1])) > fLongest)
                fLongest = cScaledOrigin.Distance(CVec3(0.0f, m_pLeafDimensions[i][1]));

            vLongestDiagonal.push_back(fLongest);
        }

        // for each leaf in all leaf LODs, determine maximum space taken
        // in all six orthographic directions.
        for (int nLod = 0; nLod < m_usNumLods; ++nLod)
        {
            SLodGeometry* pLod = m_pLods + nLod;
            for (unsigned int j = 0; j < pLod->m_usLeafCount; ++j)
            {
                CRegion cLeafSwingExtents;

                const float* pPos = pLod->m_pCenterCoords + j * 3;
                const CVec3 cCenter(pPos[0], pPos[1], pPos[2]);
                const float fLongestSide = vLongestDiagonal[pLod->m_pLeafMapIndices[j] / 2];

                // the CRegion operator^ expands the region to include the right-hand
                // CVec3 object.
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0] + fLongestSide, cCenter[1], cCenter[2]);
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0] - fLongestSide, cCenter[1], cCenter[2]);
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0], cCenter[1] + fLongestSide, cCenter[2]);
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0], cCenter[1] - fLongestSide, cCenter[2]);
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0], cCenter[1], cCenter[2] + fLongestSide);
                cLeafSwingExtents = cLeafSwingExtents ^ CVec3(cCenter[0], cCenter[1], cCenter[2] - fLongestSide);

                cExtents = cExtents + cLeafSwingExtents;
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::Transform definition

void CLeafGeometry::Transform(const CTransform& cTransform)
{
    if (m_pLods &&
        m_pLeafDimensions)
    {
        for (int nLod = 0; nLod < m_usNumLods; ++nLod)
        {
            SLodGeometry* pLod = m_pLods + nLod;

            // transform the base positions of the leaves
            for (int i = 0; i < pLod->m_usLeafCount; ++i)
            {
                // transform positions
                float* pCoord = const_cast<float*>(pLod->m_pCenterCoords + i * 3);
                CVec3 cCoord(pCoord[0], pCoord[1], pCoord[2]);
                cCoord = cCoord * cTransform;
                memcpy(pCoord, cCoord.m_afData, 3 * sizeof(float));

                // m_pOrigPositions is used as a basis for wind computations,
                // need to transform those too, if they are being used
                if (m_bVertexWeighting)
                {
                    float* pOrigCoord = pLod->m_pOrigCenterCoords + i * 3;
                    CVec3 cOrigCoord(pOrigCoord[0], pOrigCoord[1], pOrigCoord[2]);
                    cOrigCoord = cOrigCoord * cTransform;
                    memcpy(pOrigCoord, cOrigCoord.m_afData, 3 * sizeof(float));
                }
            }
        }

        // scale the leaves up by x-scale factor (arbitrary)
        float fScalar = cTransform.m_afData[0][0];
        for (int j = 0; j < m_usNumTextures; ++j)
            m_pLeafDimensions[j] *= fScalar;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::Init definition

void CLeafGeometry::Init(unsigned short usNumLeafLods, const vector<CBillboardLeaf*>* pvLeafLods, const SIdvLeafInfo& sLeafInfo)
{
    // set table info
    m_usNumRockingGroups = static_cast<unsigned short>(sLeafInfo.m_nNumRockingGroups);

    // deletion of these three pointers in still handled in the
    // SIdvLeafInfo destructor
    m_pTimeOffsets = sLeafInfo.m_pTimeOffsets;
    m_pLeafVertexes = sLeafInfo.m_pLeafVertexes;
    m_pLeafTexCoords = sLeafInfo.m_pLeafTexCoords;

    // set texture info
    m_usNumTextures = static_cast<unsigned short>(sLeafInfo.m_vLeafTextures.size( ));
    m_pLeafDimensions = new CVec3[m_usNumTextures];
    m_pLeafOrigins = new CVec3[m_usNumTextures];
    for (int i = 0; i < m_usNumTextures; ++i)
    {
        m_pLeafDimensions[i] = sLeafInfo.m_vLeafTextures[i].m_cSizeUsed;
        m_pLeafOrigins[i] = sLeafInfo.m_vLeafTextures[i].m_cOrigin;
    }

    // new leaf vertex shader table
    int nSize = m_usNumTextures * m_usNumRockingGroups * 2 * 4 * 4;  // 2 for mirrored, 4 corners per leaf, 4 floats per vertex
    m_pVertexProgramTable = new float[nSize];

    InitLods(usNumLeafLods, pvLeafLods);
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::SetTextureCoords definition
//
//  When composite leaf maps are used, this function allows CSpeedTreeRT to
//  pass in user-specified texture coordinates for each leaf map.

void CLeafGeometry::SetTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords)
{
    if (m_pLeafTexCoords &&
        pTexCoords)
    {
        float fSign = CSpeedTreeRT::GetTextureFlip( ) ? -1.0f : 1.0f;

        // one leaf map is setup just like the user specified
        float* pEntry = m_pLeafTexCoords + (nLeafMapIndex * 2) * (2 * 4);
        *pEntry++ = pTexCoords[0];
        *pEntry++ = fSign * pTexCoords[1];
        *pEntry++ = pTexCoords[2];
        *pEntry++ = fSign * pTexCoords[3];
        *pEntry++ = pTexCoords[4];
        *pEntry++ = fSign * pTexCoords[5];
        *pEntry++ = pTexCoords[6];
        *pEntry++ = fSign * pTexCoords[7];

        // another is mirrored (by swapping S coordinates) for a more interesting effect
        pEntry = m_pLeafTexCoords + (nLeafMapIndex * 2 + 1) * (2 * 4);
        *pEntry++ = pTexCoords[2];
        *pEntry++ = fSign * pTexCoords[1];
        *pEntry++ = pTexCoords[0];
        *pEntry++ = fSign * pTexCoords[3];
        *pEntry++ = pTexCoords[6];
        *pEntry++ = fSign * pTexCoords[5];
        *pEntry++ = pTexCoords[4];
        *pEntry++ = fSign * pTexCoords[7];
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::Update definition
//
//  Update is generally called once per frame per tree.  It advances the leaf rocking
//  groups by setting up and applying the billboard and rocking computations.  Part of
//  the optimization scheme is have a small number of template leaves that are billboarded
//  and rocked and the rest of the leaves are offset from them.

void CLeafGeometry::Update(CSpeedTreeRT::SGeometry::SLeaf& sGeometry, unsigned short usLodLevel, float fAzimuth, float fPitch, float fSizeIncreaseFactor)
{
    if (m_pLods &&
        usLodLevel < m_usNumLods &&
        m_pLeafVertexes &&
        m_pLeafOrigins &&
        m_pLeafDimensions &&
        m_pWindEngine &&
        m_pTimeOffsets)
    {
        SLodGeometry* pLod = m_pLods + usLodLevel;
        st_assert(pLod);

        if (!pLod->IsValid( ))
        {
            if (m_pWindEngine->GetLeafWindMethod( ) == CSpeedTreeRT::WIND_CPU)
                (void) ComputeWindEffect(usLodLevel);

            // determines how much larger the lower LODs get relative to the original leaves
            float fSizeMultiplier = 1.0f;
            if (usLodLevel > 0)
                fSizeMultiplier = 1.0f + fSizeIncreaseFactor * float(usLodLevel);

            CVec3 acRawCorners[4];
            CVec3 acBillboardedCorners[4];
            for (int i = 0; i < m_usNumTextures * 2; ++i) // X 2 for mirrored textures
            {
                // determine origin and size
                CVec3 cOrigin = m_pLeafOrigins[i / 2];
                if (!(i % 2)) // if this is a mirrored leaf, flip x-value of origin
                    cOrigin[0] = 1.0f - cOrigin[0];
                const CVec3& cSize = m_pLeafDimensions[i / 2];

                // layout the coordinates for the simple, non-billboarded leaf
                float afP1[3] = { 0.0f, fSizeMultiplier * (1.0f - cOrigin[0]) * cSize[0], fSizeMultiplier * (1.0f - cOrigin[1]) * cSize[1] };
                float afP2[3] = { 0.0f, fSizeMultiplier * -(cOrigin[0] * cSize[0]), afP1[2] };
                float afP3[3] = { 0.0f, afP2[1], fSizeMultiplier * -(cOrigin[1] * cSize[1]) };
                float afP4[3] = { 0.0f, afP1[1], afP3[2] };
                
                // adjust the coordinates for alternate coordinate system
                Assign3d(acRawCorners[0].m_afData, afP1);
                Assign3d(acRawCorners[1].m_afData, afP2);
                Assign3d(acRawCorners[2].m_afData, afP3);
                Assign3d(acRawCorners[3].m_afData, afP4);

                // each leaf texture has m_nNumRockingGroups number of leaf groups
                for (int j = 0; j < m_usNumRockingGroups; ++j)
                {
                    // Both billboarding and rocking/rustling are performed in CWindEngine's RockLeaf( ) methods
                    CRotTransform cTrans;
                    if (!m_pWindEngine->UsingExternalRockAngles( ))
                        m_pWindEngine->RockLeaf(fAzimuth, fPitch, i * m_usNumRockingGroups + j, (i + 1) * m_pTimeOffsets[j], cTrans);
                    else
                        m_pWindEngine->RockLeaf(fAzimuth, fPitch, i * m_usNumRockingGroups + j, cTrans);

                   // transform the straight quad into a billboarded/rocking leaf
                    acBillboardedCorners[0] = acRawCorners[0] * cTrans;
                    acBillboardedCorners[1] = acRawCorners[1] * cTrans;
                    acBillboardedCorners[2] = acRawCorners[2] * cTrans;
                    acBillboardedCorners[3] = acRawCorners[3] * cTrans;

                    // copy data into array for use by client application
                    float* pBillboard = m_pLeafVertexes[usLodLevel] + i * (m_usNumRockingGroups * 16) + j * 16;
                    memcpy(pBillboard, acBillboardedCorners[0], 3 * sizeof(float)); pBillboard += 4;
                    memcpy(pBillboard, acBillboardedCorners[1], 3 * sizeof(float)); pBillboard += 4;
                    memcpy(pBillboard, acBillboardedCorners[2], 3 * sizeof(float)); pBillboard += 4;
                    memcpy(pBillboard, acBillboardedCorners[3], 3 * sizeof(float)); pBillboard += 4;
                }
            }

            pLod->Validate( );
        }
        
        // update the appropriate pointers and variables of the resulting computations
        sGeometry = *pLod;
        sGeometry.m_bIsActive = true;
        sGeometry.m_nDiscreteLodLevel = usLodLevel;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::ComputeWindEffect definition
//
//  This is the code for the CPU leaf wind solution.  It takes two 
//  points:  the original leaf position, and the leaf position multiplied
//  times the appropriate wind matrix, and then interpolates between them
//  based on the wind weight value (0.0 to 1.0).

void CLeafGeometry::ComputeWindEffect(unsigned short nLodLevel)
{
    if (m_pWindEngine &&
        nLodLevel < m_usNumLods &&
        m_bVertexWeighting &&
        m_pLods)
    {
        SLodGeometry* pLod = m_pLods + nLodLevel;

        if (pLod &&
            pLod->m_pOrigCenterCoords &&
            pLod->m_pCenterCoords)
        {
            CVec3 cFullWindEffect, cInterpWindEffect;
            for (int i = 0; i < pLod->m_usLeafCount; ++i)
            {
                float fWeight = pLod->m_pWindWeights[i];
                float* pOrigCoord = pLod->m_pOrigCenterCoords + i * 3;

                cFullWindEffect.Set(pOrigCoord[0], pOrigCoord[1], pOrigCoord[2]);
                unsigned char ucWindGroup = pLod->m_pWindMatrixIndices[i];
                cFullWindEffect = cFullWindEffect * m_pWindEngine->GetWindMatrix(ucWindGroup);
                cInterpWindEffect.Set(VecInterpolate(pOrigCoord[0], cFullWindEffect[0], fWeight),
                                    VecInterpolate(pOrigCoord[1], cFullWindEffect[1], fWeight),
                                    VecInterpolate(pOrigCoord[2], cFullWindEffect[2], fWeight));
                memcpy(const_cast<float*>(pLod->m_pCenterCoords + i * 3), cInterpWindEffect.m_afData, 12);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::InitLods definition
//
//  This is where the bulk of the work is done in transfering the leaf data computed
//  by CIdvBranch and CTreeEngine.  The code changes depending on the API/engine 
//  defined during compilation as well as the coordinate system.

void CLeafGeometry::InitLods(unsigned short usNumLeafLods, const vector<CBillboardLeaf*>* pvLeafLods)
{
    if (pvLeafLods &&
        m_pLeafVertexes &&
        m_pWindEngine &&
        m_pLeafTexCoords)
    {
        m_usNumLods = usNumLeafLods;
        m_pLods = new SLodGeometry[usNumLeafLods];

        for (unsigned short usLod = 0; usLod < usNumLeafLods; ++usLod)
        {
            SLodGeometry* pLod = m_pLods + usLod;
            pLod->m_usLeafCount = static_cast<unsigned short>(pvLeafLods[usLod].size( ));

            // coords
            pLod->m_pLeafMapCoords = const_cast<const float**>(new float*[pLod->m_usLeafCount]);
            for (int j = 0; j < pLod->m_usLeafCount; ++j)
                pLod->m_pLeafMapCoords[j] = m_pLeafVertexes[usLod] + pvLeafLods[usLod][j]->GetAltTextureIndex( ) * (m_usNumRockingGroups * 16) + pvLeafLods[usLod][j]->GetAngle( ) * 16;

            // wind related
            if (m_bVertexWeighting)
            {
                // orig positions (basis for wind)
                pLod->m_pOrigCenterCoords = new float[pLod->m_usLeafCount * 3];
                for (int i = 0; i < pLod->m_usLeafCount; ++i)
                    Assign3d(pLod->m_pOrigCenterCoords + i * 3, pvLeafLods[usLod][i]->GetPosition( ).m_afData);

                // wind weights
                pLod->m_pWindWeights = new float[pLod->m_usLeafCount];
                for (int i = 0; i < pLod->m_usLeafCount; ++i)
                    (const_cast<float*>(pLod->m_pWindWeights))[i] = 1.0f - pvLeafLods[usLod][i]->GetWeight( );

                // wind groups
                pLod->m_pWindMatrixIndices = new unsigned char[pLod->m_usLeafCount];
                for (int i = 0; i < pLod->m_usLeafCount; ++i)
                {
                    unsigned char ucGroup = pvLeafLods[usLod][i]->GetWindGroup( );

                    // chGroup spans a wide range of matrices, mod it down to size
                    unsigned int nStartingMatrix = 0, nMatrixSpan = 1;
                    m_pWindEngine->GetLocalMatrices(nStartingMatrix, nMatrixSpan);
                    ucGroup = static_cast<unsigned char>(nStartingMatrix) + ucGroup % static_cast<unsigned char>(nMatrixSpan);

                    (const_cast<unsigned char*>(pLod->m_pWindMatrixIndices))[i] = ucGroup;
                }
            }

            // positions
            pLod->m_pCenterCoords = new float[pLod->m_usLeafCount * 3];
            for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                Assign3d(const_cast<float*>(pLod->m_pCenterCoords + i * 3), pvLeafLods[usLod][i]->GetPosition( ).m_afData);

            // texture indexes
            pLod->m_pLeafMapIndices = new unsigned char[pLod->m_usLeafCount];
            for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                (const_cast<unsigned char*>(pLod->m_pLeafMapIndices))[i] = static_cast<unsigned char>(pvLeafLods[usLod][i]->GetAltTextureIndex( ));

            // cluster indices, used for vertex shaders mostly
            pLod->m_pLeafClusterIndices = new unsigned char[pLod->m_usLeafCount];
            for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                (const_cast<unsigned char*>(pLod->m_pLeafClusterIndices))[i] = static_cast<unsigned char>(pvLeafLods[usLod][i]->GetAltTextureIndex( ) *
                    m_usNumRockingGroups + pvLeafLods[usLod][i]->GetAngle( ));

            // texcoords
            pLod->m_pLeafMapTexCoords = const_cast<const float**>(new float*[pLod->m_usLeafCount]);
            for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                pLod->m_pLeafMapTexCoords[i] = m_pLeafTexCoords + pvLeafLods[usLod][i]->GetAltTextureIndex( ) * 8;

            // colors
            pLod->m_pColors = new unsigned long[pLod->m_usLeafCount];
            for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                (const_cast<unsigned long*>(pLod->m_pColors))[i] = EndianSwap32(pvLeafLods[usLod][i]->GetColorLong( ));

            // normals
            if (!m_bManualLighting)
            {
                pLod->m_pNormals = new float[pLod->m_usLeafCount * 3];
                for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                    Assign3d(const_cast<float*>(pLod->m_pNormals + i * 3), pvLeafLods[usLod][i]->GetNormal( ).m_afData);
            }
            // bump mapping
            if (!m_bManualLighting)
            {
                pLod->m_pTangents = new float[pLod->m_usLeafCount * 3];
                pLod->m_pBinormals = new float[pLod->m_usLeafCount * 3];
                for (unsigned int i = 0; i < pLod->m_usLeafCount; ++i)
                {
                    Assign3d(const_cast<float*>(pLod->m_pTangents + i * 3), pvLeafLods[usLod][i]->GetTangent( ).m_afData);
                    Assign3d(const_cast<float*>(pLod->m_pBinormals + i * 3), pvLeafLods[usLod][i]->GetBinormal( ).m_afData);
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLeafGeometry::GetTextureCoords definition

void CLeafGeometry::GetTextureCoords(unsigned int nLeafMapIndex, float* pTexCoords) const
{
    if (m_pLeafTexCoords &&
        pTexCoords)
    {
        float* pEntry = m_pLeafTexCoords + (nLeafMapIndex * 2) * (2 * 4);
        memcpy(pTexCoords, pEntry, 8 * sizeof(float));
    }
}
