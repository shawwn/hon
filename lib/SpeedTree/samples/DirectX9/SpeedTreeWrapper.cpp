///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//

///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <stdlib.h>
#include <stdio.h>
#include "Random.h"
#include "Filename.h"
#include "VertexShaders.h"
#include "SpeedTreeWrapper.h"

///////////////////////////////////////////////////////////////////////  
//  Static member variable declarations

LPDIRECT3DDEVICE9               CSpeedTreeWrapper::m_pDx = NULL;
unsigned int                    CSpeedTreeWrapper::m_unNumWrappersActive = 0;
LPDIRECT3DTEXTURE9              CSpeedTreeWrapper::m_texShadow = NULL;


///////////////////////////////////////////////////////////////////////  
//  Color conversion macro

#define AGBR2ARGB(dwColor) (dwColor & 0xff00ff00) + ((dwColor & 0x00ff0000) >> 16) + ((dwColor & 0x000000ff) << 16)


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper( ) :
    m_pSpeedTree(new CSpeedTreeRT),
    m_bIsInstance(false),
    m_pInstanceOf(NULL),
    m_pGeometryCache(NULL),
    m_usNumLeafLods(0),
	m_unNumFrondLods(0),
    m_pBranchIndexCounts(NULL),
    m_pBranchIndexBuffer(NULL),
    m_pBranchVertexBuffer(NULL),
    m_pFrondIndexCounts(NULL),
    m_pFrondIndexBuffers(NULL),
    m_pFrondVertexBuffer(NULL),
    m_pLeafVertexBuffer(NULL),
    m_pLeavesUpdatedByCpu(NULL),
    m_texBranchTexture(NULL)
{
    m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    m_unNumWrappersActive++;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper( )
{
    // if this is not an instance, then clean up
    if (!m_bIsInstance)
    {
        if (m_unBranchVertexCount > 0)
        {
            SAFE_RELEASE(m_pBranchVertexBuffer);
            SAFE_RELEASE(m_pBranchIndexBuffer);
            SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
        }

        if (m_unFrondVertexCount > 0)
        {   
            SAFE_RELEASE(m_pFrondVertexBuffer);
			for (unsigned int i = 0; i < m_unNumFrondLods; ++i)
				if (m_pFrondIndexCounts[i] > 0)
					SAFE_RELEASE(m_pFrondIndexBuffers[i]);
			SAFE_DELETE_ARRAY(m_pFrondIndexBuffers);
            SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
        }
        
        for (unsigned short i = 0; i < m_usNumLeafLods; ++i)
        {
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);
            if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
                SAFE_RELEASE(m_pLeafVertexBuffer[i]);
        }
        SAFE_DELETE_ARRAY(m_pLeafVertexBuffer);
        SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
        
        SAFE_DELETE(m_pTextureInfo);

        SAFE_RELEASE(m_texBranchTexture);
    }

    // always delete the SpeedTree
    SAFE_DELETE(m_pSpeedTree);

    --m_unNumWrappersActive;
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_unNumWrappersActive == 0)
            SAFE_RELEASE(m_texShadow);
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, unsigned int nSeed, float fSize, float fSizeVariance)
{
    bool bSuccess = false;

    // directx, so allow for flipping of the texture coordinate
    #ifdef WRAPPER_FLIP_T_TEXCOORD
        m_pSpeedTree->SetTextureFlip(true);
    #endif

    // load the tree file
    if (m_pSpeedTree->LoadTree(pszSptFile))
    {
        // override the lighting method stored in the spt file
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
            m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
            m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
        #else
            m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
            m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
            m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        #endif

        // set the wind method
        #ifdef WRAPPER_USE_GPU_WIND
            m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
            m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
            m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
        #endif
        #ifdef WRAPPER_USE_CPU_WIND
            m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_CPU);
            m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_CPU);
            m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_CPU);
        #endif
        #ifdef WRAPPER_USE_NO_WIND
            m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
            m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
            m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
        #endif

        m_pSpeedTree->SetNumLeafRockingGroups(1);

        // override the size, if necessary
        if (fSize >= 0.0f && fSizeVariance >= 0.0f)
            m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

        // generate tree geometry
        if (m_pSpeedTree->Compute(NULL, nSeed))
        {
            // get the dimensions
            m_pSpeedTree->GetBoundingBox(m_afBoundingBox);
        
            // make the leaves rock in the wind
            m_pSpeedTree->SetLeafRockingState(true);

            // billboard setup
            #ifdef WRAPPER_BILLBOARD_MODE
                CSpeedTreeRT::SetDropToBillboard(true);
            #else
                CSpeedTreeRT::SetDropToBillboard(false);
            #endif

            // query & set materials
            m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial( ));
            m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial( ));
            m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial( ));

            // adjust lod distances
            float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
            m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodFactor, fHeight * c_fFarLodFactor);

            // query textures
            m_pTextureInfo = new CSpeedTreeRT::STextures;
            m_pSpeedTree->GetTextures(*m_pTextureInfo);

            // load branch textures
            m_texBranchTexture = LoadTexture((CFilename(pszSptFile).GetPath( ) + CFilename(m_pTextureInfo->m_pBranchTextureFilename).NoExtension( ) + ".dds").c_str( ));
            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                if (m_pTextureInfo->m_pSelfShadowFilename != NULL && m_texShadow == NULL)
                    m_texShadow = LoadTexture((CFilename(pszSptFile).GetPath( ) + CFilename(m_pTextureInfo->m_pSelfShadowFilename).NoExtension( ) + ".dds").c_str( ));
            #endif

            // setup the vertex and index buffers
            SetupBuffers( );

            // everything appeared to go well
            bSuccess = true;
        }
        else // tree failed to compute
            fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError( ));

    }
    else // tree failed to load
    {
        fprintf(stderr, "SpeedTreeRT Error: %s\n", CSpeedTreeRT::GetCurrentError( ));
        Sleep(2000);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBuffers

void CSpeedTreeWrapper::SetupBuffers(void)
{
    // read all the geometry for highest LOD into the geometry cache
    m_pSpeedTree->SetLodLevel(1.0f);
    if (m_pGeometryCache == NULL)
        m_pGeometryCache = new CSpeedTreeRT::SGeometry;
    m_pSpeedTree->GetGeometry(*m_pGeometryCache);

    // setup the buffers for each tree part
    SetupBranchBuffers( );
    SetupFrondBuffers( );
    SetupLeafBuffers( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchBuffers

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
    // reference to the branch structure
    CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
    m_unBranchVertexCount = pBranches->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has branches
    if (m_unBranchVertexCount > 1)
    {
        // create the vertex buffer for storing branch vertices
        #ifdef WRAPPER_USE_GPU_WIND
            m_pDx->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_DEFAULT, &m_pBranchVertexBuffer, NULL);
        #else
            m_pDx->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_DEFAULT, &m_pBranchVertexBuffer, NULL);
        #endif

        // fill the vertex buffer by interleaving SpeedTree data
        SFVFBranchVertex* pVertexBuffer = NULL;
        m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        for (unsigned int i = 0; i < m_unBranchVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

            // normal or color
            #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));
            #else
                pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pBranches->m_pColors[i]);
            #endif

            // texcoords for layer 0
            pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
            pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

            // texcoords for layer 1 (if enabled)
            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                pVertexBuffer->m_fShadowCoords[0] = pBranches->m_pTexCoords1[i * 2];
                pVertexBuffer->m_fShadowCoords[1] = pBranches->m_pTexCoords1[i * 2 + 1];
            #endif

            // gpu wind data
            #ifdef WRAPPER_USE_GPU_WIND
                pVertexBuffer->m_fWindIndex = 4.0f * pBranches->m_pWindMatrixIndices[i];
                pVertexBuffer->m_fWindWeight = (float)pBranches->m_pWindWeights[i];
            #endif

            ++pVertexBuffer;
        }
        m_pBranchVertexBuffer->Unlock( );

        // create and fill the index counts for each LOD
        unsigned int unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels( );
        m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
        for (unsigned int i = 0; i < unNumLodLevels; ++i)
        {
            // force geometry update for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, (short)i);

            // check if this LOD has branches
            if (pBranches->m_usNumStrips > 0)
                m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
            else
                m_pBranchIndexCounts[i] = 0;
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);

        // the first LOD level contains the most indices of all the levels, so
        // we use its size to allocate the index buffer
        m_pDx->CreateIndexBuffer(m_pBranchIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pBranchIndexBuffer, NULL);

        // fill the index buffer
        unsigned short* pIndexBuffer = NULL;
        m_pBranchIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
        memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
        m_pBranchIndexBuffer->Unlock( );
    }
}

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondBuffers

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
    // reference to frond structure 
    CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
    m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

    // check if this tree has fronds
    if (m_unFrondVertexCount > 1)
    {
        // create the vertex buffer for storing frond vertices
        #ifdef WRAPPER_USE_GPU_WIND
            m_pDx->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_DEFAULT, &m_pFrondVertexBuffer, NULL);
        #else
            m_pDx->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_DEFAULT, &m_pFrondVertexBuffer, NULL);
        #endif
        
        // fill the vertex buffer by interleaving SpeedTree data
        SFVFBranchVertex* pVertexBuffer = NULL;
        m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        for (unsigned int i = 0; i < m_unFrondVertexCount; ++i)
        {
            // position
            memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

            // normal or color
            #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));
            #else
                pVertexBuffer->m_dwDiffuseColor = AGBR2ARGB(pFronds->m_pColors[i]);
            #endif

            // texcoords for layer 0
            pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
            pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

            // texcoords for layer 1 (if enabled)
            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                pVertexBuffer->m_fShadowCoords[0] = pFronds->m_pTexCoords1[i * 2];
                pVertexBuffer->m_fShadowCoords[1] = pFronds->m_pTexCoords1[i * 2 + 1];
            #endif
        
            // gpu wind data
            #ifdef WRAPPER_USE_GPU_WIND
                pVertexBuffer->m_fWindIndex = 4.0f * pFronds->m_pWindMatrixIndices[i];
                pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];
            #endif

            ++pVertexBuffer;
        }
        m_pFrondVertexBuffer->Unlock( );

        // create and fill the index counts for each LOD
        m_unNumFrondLods = m_pSpeedTree->GetNumFrondLodLevels( );
        m_pFrondIndexCounts = new unsigned short[m_unNumFrondLods];
		m_pFrondIndexBuffers = new LPDIRECT3DINDEXBUFFER9[m_unNumFrondLods];

        for (unsigned int i = 0; i < m_unNumFrondLods; ++i)
        {
            // force update of geometry for this LOD
            m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, i);

            // check if this LOD has fronds
            if (pFronds->m_usNumStrips > 0)
                m_pFrondIndexCounts[i] = pFronds->m_pStripLengths[0];
            else
                m_pFrondIndexCounts[i] = 0;

			if (m_pFrondIndexCounts[i] > 0)
			{
				m_pDx->CreateIndexBuffer(m_pFrondIndexCounts[i] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pFrondIndexBuffers[i], NULL);
				
				// fill the index buffer
				unsigned short* pIndexBuffer = NULL;
				m_pFrondIndexBuffers[i]->Lock(0, 0, reinterpret_cast<void**>(&pIndexBuffer), 0);
				memcpy(pIndexBuffer, pFronds->m_pStrips[0], m_pFrondIndexCounts[i] * sizeof(unsigned short));
				m_pFrondIndexBuffers[i]->Unlock( );
			}
        }
        // force update of geometry to highest LOD
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
    // set up constants
    const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };

    // set up the leaf counts for each LOD
    m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels( );

    // create arrays for the number of LOD levels
    m_pLeafVertexBuffer = new LPDIRECT3DVERTEXBUFFER9[m_usNumLeafLods];
    m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];

    for (unsigned int unLod = 0; unLod < m_usNumLeafLods; ++unLod)
    {
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, unLod);

        m_pLeavesUpdatedByCpu[unLod] = false;

        // if this level has no leaves, then skip it
        unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;
        if (usLeafCount < 1)
            continue;

        // create the vertex buffer for storing leaf vertices
        #if defined(WRAPPER_USE_GPU_WIND) && defined(WRAPPER_USE_GPU_LEAF_PLACEMENT)
            m_pDx->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_DEFAULT, &m_pLeafVertexBuffer[unLod], NULL);
        #else
            m_pDx->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_DEFAULT, &m_pLeafVertexBuffer[unLod], NULL);
        #endif
    
        // fill the vertex buffer by interleaving SpeedTree data
        SFVFLeafVertex* pVertexBuffer = NULL;
        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
        SFVFLeafVertex* pVertex = pVertexBuffer;
        for (unsigned int unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
        {
            const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
            for (unsigned int unVert = 0; unVert < 6; ++unVert)  // 6 verts == 2 triangles
            {
                // position
                memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

                #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                    // normal
                    memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
                #else
                    // color
                    pVertex->m_dwDiffuseColor = AGBR2ARGB(pLeaf->m_pColors[unLeaf]);
                #endif

                // tex coord
                memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));

                // wind weights
                #ifdef WRAPPER_USE_GPU_WIND
                    pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
                    pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
                #endif

                // gpu leaf placement data
                #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                    pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
                    pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments( )[unLod];
                #endif

                ++pVertex;
            }
            
        }
        m_pLeafVertexBuffer[unLod]->Unlock( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
    // compute LOD level (based on distance from camera)
    m_pSpeedTree->ComputeLodLevel( );

    // compute wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->ComputeWindEffects(true, true, true);
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    // create a new object
    CSpeedTreeWrapper* pInstance = new CSpeedTreeWrapper;

    // make an instance of this object's SpeedTree
    pInstance->m_bIsInstance = true;
    pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance( );
    if (pInstance->m_pSpeedTree)
    {
        // use the same materials
        pInstance->m_cBranchMaterial = m_cBranchMaterial;
        pInstance->m_cLeafMaterial = m_cLeafMaterial;
        pInstance->m_cFrondMaterial = m_cFrondMaterial;
        pInstance->m_texBranchTexture = m_texBranchTexture;
        pInstance->m_texShadow = m_texShadow;
        pInstance->m_pTextureInfo = m_pTextureInfo;

        // use the same geometry cache
        pInstance->m_pGeometryCache = m_pGeometryCache;
        
        // use the same buffers
        pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
        pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
        pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
        pInstance->m_unBranchVertexCount = m_unBranchVertexCount;

        pInstance->m_pFrondIndexBuffers = m_pFrondIndexBuffers;
		pInstance->m_unNumFrondLods = m_unNumFrondLods;
        pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
        pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
        pInstance->m_unFrondVertexCount = m_unFrondVertexCount;

        pInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
        pInstance->m_usNumLeafLods = m_usNumLeafLods;
        pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;
        
        // new stuff
        memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
        memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
        pInstance->m_pInstanceOf = this;
        m_vInstances.push_back(pInstance);
    }
    else
    {
        fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError( ));
        delete pInstance;
        pInstance = NULL;
    }

    return pInstance;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::GetInstances

CSpeedTreeWrapper** CSpeedTreeWrapper::GetInstances(unsigned int& nCount)
{
    nCount = m_vInstances.size( );
    return &(m_vInstances[0]);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchForTreeType

void CSpeedTreeWrapper::SetupBranchForTreeType(void) const
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cBranchMaterial.Activate(m_pDx);
        SetShaderConstants(m_pSpeedTree->GetBranchMaterial( ));
    #endif

    // update the branch geometry for CPU wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);
        if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
        {
            // update the vertex array
            SFVFBranchVertex* pVertexBuffer = NULL;
            m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
            for (unsigned int i = 0; i < m_unBranchVertexCount; ++i)
            {
                memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
            }
            m_pBranchVertexBuffer->Unlock( );
        }
    #endif

    // set texture map
    if (m_texBranchTexture)
        m_pDx->SetTexture(0, m_texBranchTexture);

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_texShadow)
            m_pDx->SetTexture(1, m_texShadow);
    #endif

    // if tree has branches, bind the buffers
    if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
    {
        // activate the branch vertex buffer
        m_pDx->SetStreamSource(0, m_pBranchVertexBuffer, 0, sizeof(SFVFBranchVertex));
        // set the index buffer
        m_pDx->SetIndices(m_pBranchIndexBuffer);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderBranches

void CSpeedTreeWrapper::RenderBranches(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

    if (m_pGeometryCache->m_fBranchAlphaTestValue > 0.0f)
    {
        PositionTree( );

        // set alpha test value
        m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));

        // if tree has branches at this LOD, draw them
        if (m_pBranchIndexCounts && m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1 && m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
            m_pDx->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondForTreeType

void CSpeedTreeWrapper::SetupFrondForTreeType(void) const
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cFrondMaterial.Activate(m_pDx);
        SetShaderConstants(m_pSpeedTree->GetFrondMaterial( ));
    #endif

    // update the frond geometry for CPU wind
    #ifdef WRAPPER_USE_CPU_WIND
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
        if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
        {
            // update the vertex array
            SFVFBranchVertex* pVertexBuffer = NULL;
            m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBuffer), 0);
            for (unsigned int i = 0; i < m_unFrondVertexCount; ++i)
            {
                memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
            }
            m_pFrondVertexBuffer->Unlock( );
        }
    #endif

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_texShadow)
            m_pDx->SetTexture(1, m_texShadow);
    #endif

    // if tree has fronds, bind the buffers
    if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
    {
        // activate the frond vertex buffer
        m_pDx->SetStreamSource(0, m_pFrondVertexBuffer, 0, sizeof(SFVFBranchVertex));
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderFronds

void CSpeedTreeWrapper::RenderFronds(void) const
{
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

    if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
    {
        PositionTree( );
        m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

        // if tree has fronds, draw them
        if (m_pFrondIndexCounts && m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel > -1 && m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
		{
			m_pDx->SetIndices(m_pFrondIndexBuffers[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel]);
			m_pDx->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2);
		}
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafForTreeType

void CSpeedTreeWrapper::SetupLeafForTreeType(void) const
{
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        // set lighting material
        m_cLeafMaterial.Activate(m_pDx);
        SetShaderConstants(m_pSpeedTree->GetLeafMaterial( ));
    #endif

    // send leaf tables to the gpu
    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        UploadLeafTables(c_nVertexShader_LeafTables);
    #endif

    // bind shadow texture (if enabled)
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_texShadow)
            m_pDx->SetTexture(1, 0);
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::UploadLeafTables

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
void CSpeedTreeWrapper::UploadLeafTables(unsigned int uiLocation) const
{
    // query leaf cluster table from RT
    unsigned int uiEntryCount = 0;
    const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);

    // upload for vertex shader use
    m_pDx->SetVertexShaderConstantF(uiLocation, pTable, uiEntryCount / 4);
}
#endif


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderLeaves

void CSpeedTreeWrapper::RenderLeaves(void) const
{
    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, 0);
    #endif
    m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

    // update the LOD level vertex arrays we need
    #if defined(WRAPPER_USE_GPU_LEAF_PLACEMENT) && defined(WRAPPER_USE_GPU_WIND)
        // do nothing
    #else
        #if !defined WRAPPER_USE_NO_WIND || defined WRAPPER_USE_CPU_LEAF_PLACEMENT
            // might need to draw 2 LOD's
            for (unsigned int i = 0; i < 2; ++i)
            {
                // reference to leaves structure
                const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (i == 0) ? &m_pGeometryCache->m_sLeaves0 : &m_pGeometryCache->m_sLeaves1;
                int unLod = pLeaf->m_nDiscreteLodLevel;

                #if defined WRAPPER_USE_GPU_LEAF_PLACEMENT
                    if (pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0 && !m_pLeavesUpdatedByCpu[unLod])
                    {
                        // update the centers
                        SFVFLeafVertex* pVertex = NULL;
                        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertex), 0);
                        for (unsigned int unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
                        {
                            D3DXVECTOR3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 0
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 1
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 2
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 0
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 2
                            (pVertex++)->m_vPosition = vecCenter;       // vertex 3
                        }
                        m_pLeafVertexBuffer[unLod]->Unlock( );
                        m_pLeavesUpdatedByCpu[unLod] = true;
                    }
                #else
                    if (pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
                    { 
                        // update the vertices
                        SFVFLeafVertex* pVertex = NULL;
                        m_pLeafVertexBuffer[unLod]->Lock(0, 0, reinterpret_cast<void**>(&pVertex),0);
                        for (unsigned int unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
                        {
                            D3DXVECTOR3 vecCenter(&(pLeaf->m_pCenterCoords[unLeaf * 3]));
                            D3DXVECTOR3 vec0(&pLeaf->m_pLeafMapCoords[unLeaf][0]);
                            D3DXVECTOR3 vec1(&pLeaf->m_pLeafMapCoords[unLeaf][4]);
                            D3DXVECTOR3 vec2(&pLeaf->m_pLeafMapCoords[unLeaf][8]);
                            D3DXVECTOR3 vec3(&pLeaf->m_pLeafMapCoords[unLeaf][12]);

                            (pVertex++)->m_vPosition = vecCenter + vec0;        // vertex 0
                            (pVertex++)->m_vPosition = vecCenter + vec1;        // vertex 1
                            (pVertex++)->m_vPosition = vecCenter + vec2;        // vertex 2
                            (pVertex++)->m_vPosition = vecCenter + vec0;        // vertex 0
                            (pVertex++)->m_vPosition = vecCenter + vec2;        // vertex 2
                            (pVertex++)->m_vPosition = vecCenter + vec3;        // vertex 3
                        }
                        m_pLeafVertexBuffer[unLod]->Unlock( );
                    }
                #endif
            }
        #endif
    #endif

    PositionTree( );

    // might need to draw 2 LOD's
    for (unsigned int unLeafLevel = 0; unLeafLevel < 2; ++unLeafLevel)
    {
        const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = (unLeafLevel == 0) ?
            &m_pGeometryCache->m_sLeaves0 : pLeaf = &m_pGeometryCache->m_sLeaves1;

        int unLod = pLeaf->m_nDiscreteLodLevel;

        // if this LOD is active and has leaves, draw it
        if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
        {
            m_pDx->SetStreamSource(0, m_pLeafVertexBuffer[unLod], 0, sizeof(SFVFLeafVertex));
            m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));
            m_pDx->DrawPrimitive(D3DPT_TRIANGLELIST, 0, pLeaf->m_usLeafCount * 2);
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::EndLeafForTreeType

void CSpeedTreeWrapper::EndLeafForTreeType(void)
{
    // reset flags for CPU data copying
    for (unsigned int i = 0; i < m_usNumLeafLods; ++i)
        m_pLeavesUpdatedByCpu[i] = false;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderBillboards

void CSpeedTreeWrapper::RenderBillboards(void) const
{
    // draw billboards in immediate mode (as close as directx gets to immediate mode)
    #ifdef WRAPPER_BILLBOARD_MODE
        PositionTree( );    

        struct SBillboardVertex 
        {
            float fX, fY, fZ;
            DWORD dColor;
            float fU, fV;
        };

        m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);
        if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
        {
            const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
            const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
            SBillboardVertex sVertex[4] = 
            {
                { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
            };
            m_pDx->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
            m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));
            m_pDx->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
        }

        // if there is a 360 degree billboard, then we need to draw the second one
        if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
        {
            const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
            const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
            SBillboardVertex sVertex[4] = 
            {
                { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
            };
            m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));
            m_pDx->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
        }

        // if we have a horizontal bilboard and it is enabled, draw it too 
        #ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
            if (m_pGeometryCache->m_sHorizontalBillboard.m_bIsActive)
            {   
                const float* pCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pCoords;
                const float* pTexCoords = m_pGeometryCache->m_sHorizontalBillboard.m_pTexCoords;
                SBillboardVertex sVertex[4] = 
                {
                    { pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
                    { pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
                    { pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
                    { pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
                };
                m_pDx->SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sHorizontalBillboard.m_fAlphaTestValue));
                m_pDx->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
            }

        #endif
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CleanUpMemory

void CSpeedTreeWrapper::CleanUpMemory(void)
{
    if (!m_bIsInstance)
        m_pSpeedTree->DeleteTransientData( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetPos

void CSpeedTreeWrapper::SetPos(const float* pPos)
{
    m_afPos[0] = pPos[0];
    m_afPos[1] = pPos[1];
    m_afPos[2] = pPos[2];

    m_pSpeedTree->SetTreePosition(pPos[0], pPos[1], pPos[2]);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::PositionTree

void CSpeedTreeWrapper::PositionTree(void) const
{
    D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition( );
    D3DXMATRIX matTranslation;
    D3DXMatrixIdentity(&matTranslation);
    D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);
    
    // store translation for client-side transformation
    m_pDx->SetTransform(D3DTS_WORLD, &matTranslation);

    // store translation for use in vertex shader
    D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
    m_pDx->SetVertexShaderConstantF(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::LoadTexture

LPDIRECT3DTEXTURE9 CSpeedTreeWrapper::LoadTexture(const char* pFilename)
{
    LPDIRECT3DTEXTURE9 pTexture = NULL;

    if (D3DUtil_CreateTexture(m_pDx, const_cast<char*>(pFilename), &pTexture) != S_OK)
        fprintf(stderr, "Warning: failed to load texture [%s]\n", pFilename);

    return pTexture;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetShaderConstants

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
    const float afUsefulConstants[] = 
    {
        m_pSpeedTree->GetLeafLightingAdjustment( ), 0.0f, 0.0f, 0.0f
    };
    m_pDx->SetVertexShaderConstantF(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

    const float afMaterial[] = 
    {
        pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
        pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
    };
    m_pDx->SetVertexShaderConstantF(c_nVertexShader_Material, afMaterial, 2);
}
