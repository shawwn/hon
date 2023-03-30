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

#pragma warning (disable : 4786)


///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <cstdlib>
#include <cstdio>
#include "SpeedTreeWrapper.h"
#include "Random.h"
#include "Filename.h"
#include "nv_dds.h"
#include <crtdbg.h>
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  Constants

static  const   float   c_fNearLodScale = 3.0f;
static  const   float   c_fFarLodScale = 6.0f;


///////////////////////////////////////////////////////////////////////  
//  Static member variables

#ifdef WRAPPER_RENDER_VERTEX_OBJECTS
    unsigned int    CSpeedTreeWrapper::m_uiTmpCoordsBufferSize = 0;
    float*          CSpeedTreeWrapper::m_pTmpCoordsBuffer = NULL;
#endif
unsigned int    CSpeedTreeWrapper::m_uiRefCount = 0;


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::CSpeedTreeWrapper

CSpeedTreeWrapper::CSpeedTreeWrapper( ) :
    m_pSpeedTree(NULL),
    m_bIsInstance(false),
    m_pInstanceOf(NULL),
    #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
    m_pBranchVertexBuffer(NULL),
    m_pFrondVertexBuffer(NULL),
    m_pLeafVertexBuffers(NULL),
    m_pLeavesUpdatedByCpu(NULL),
    #endif
    m_uiBranchTexture(0),
    m_uiSelfShadowTexture(0)
{
    m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;
    m_afBoundingBox[0] = m_afBoundingBox[1] = m_afBoundingBox[2] = 
    m_afBoundingBox[3] = m_afBoundingBox[4] = m_afBoundingBox[5] = 0.0f;

    m_uiRefCount++;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::~CSpeedTreeWrapper

CSpeedTreeWrapper::~CSpeedTreeWrapper( )
{
    delete m_pSpeedTree;

    // since instances share the pointers with the original SpeedTree
    // instantiation, we only delete the memory if it is original
    if (!IsInstance( ))
    {
        #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            delete m_pBranchVertexBuffer;
            delete m_pFrondVertexBuffer;
            delete[] m_pLeafVertexBuffers;
            delete[] m_pLeavesUpdatedByCpu;
        #endif
    }

    // deleted or not, the pointers need to be set to null
    m_pSpeedTree = NULL;
#ifdef WRAPPER_RENDER_VERTEX_OBJECTS
    m_pBranchVertexBuffer = NULL;
    m_pFrondVertexBuffer = NULL;
    m_pLeafVertexBuffers = NULL;
    m_pLeavesUpdatedByCpu = NULL;
#endif

    if (!--m_uiRefCount)
    {
        #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            delete[] m_pTmpCoordsBuffer;
            m_pTmpCoordsBuffer = 0;
            m_uiTmpCoordsBufferSize = 0;
        #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::LoadTree

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, unsigned int nSeed, float fSize, float fSizeVariance)
{
    bool bSuccess = false;

    // allocate the SpeedTreeRT object
    m_pSpeedTree = new CSpeedTreeRT;

    // wind states
    m_pSpeedTree->SetLocalMatrices(0, c_nNumWindMatrices);

    // load the tree file
    CSpeedTreeRT::SetTextureFlip(false);
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

        // override the size, if necessary
        if (fSize >= 0.0f && fSizeVariance >= 0.0f)
            m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

        // set same number of leaf rocking groups for all trees
        m_pSpeedTree->SetNumLeafRockingGroups(c_nNumLeafRockingGroups);

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
            m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial( ));
            m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial( ));

            // adjust lod distances
            float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
            m_pSpeedTree->SetLodLimits(fHeight * c_fNearLodScale, fHeight * c_fFarLodScale);

            // load branch texture
            CSpeedTreeRT::STextures sTextures;
            m_pSpeedTree->GetTextures(sTextures);
            m_uiBranchTexture = LoadDDS((CFilename(pszSptFile).GetPath( ) + CFilename(sTextures.m_pBranchTextureFilename).NoExtension( ) + ".dds").c_str( ));

            #ifdef WRAPPER_RENDER_SELF_SHADOWS
            if (sTextures.m_pSelfShadowFilename)
                m_uiSelfShadowTexture = LoadDDS((CFilename(pszSptFile).GetPath( ) + CFilename(sTextures.m_pSelfShadowFilename).NoExtension( ) + ".dds").c_str( ));
            else
                m_uiSelfShadowTexture = 0;
            #endif

            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            SetupVertexBuffers( );
            #endif

            // everything appeared to go well
            bSuccess = true;
        }
        else // tree failed to compute
            fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError( ));

    }
    else // tree failed to load
        fprintf(stderr, "SpeedTreeRT Error: %s\n", CSpeedTreeRT::GetCurrentError( ));

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::Advance

void CSpeedTreeWrapper::Advance(void)
{
    if (m_pSpeedTree)
    {
        // compute LOD level (based on distance from camera)
        m_pSpeedTree->ComputeLodLevel( );

        // compute wind
        #ifdef WRAPPER_USE_CPU_WIND
            m_pSpeedTree->ComputeWindEffects(true, true, true);
        #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::MakeInstance

CSpeedTreeWrapper* CSpeedTreeWrapper::MakeInstance(void)
{
    // create a new object
    CSpeedTreeWrapper* pInstance = NULL;
    
    if (m_pSpeedTree)
    {
        pInstance = new CSpeedTreeWrapper;

        // make an instance of this object's SpeedTree
        pInstance->m_bIsInstance = true;
        pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance( );
        if (pInstance->m_pSpeedTree)
        {
            pInstance->m_pInstanceOf = this;
        
            // copy rendering and state information
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
                pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
                pInstance->m_pLeafVertexBuffers = m_pLeafVertexBuffers;
                pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
                pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;
            #endif
            pInstance->m_uiBranchTexture = m_uiBranchTexture;

            #ifdef WRAPPER_RENDER_SELF_SHADOWS
            pInstance->m_uiSelfShadowTexture = m_uiSelfShadowTexture;
            #endif

            memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
            memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
            pInstance->m_cBranchMaterial = m_cBranchMaterial;
            pInstance->m_cLeafMaterial = m_cLeafMaterial;
            pInstance->m_cFrondMaterial = m_cFrondMaterial;

        }
        else
        {
            fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError( ));
            delete pInstance;
            pInstance = NULL;
        }
    }

    return pInstance;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderBranches

void CSpeedTreeWrapper::RenderBranches(void) const
{
    if (m_pSpeedTree)
    {
    #ifdef WRAPPER_RENDER_IMMEDIATE_MODE
        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_BranchGeometry);

        if (sGeometry.m_fBranchAlphaTestValue >= 0.0f)
        {
            // translate to the proper location
            glPushMatrix( );
            glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

            // cancel the leaf lighting adjustment factor (only used for leaves)
            #ifdef LEAVES_USE_SHADERS
                glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, c_nVertexShader_LeafLightingAdjustment, 0.0f, 0.0f, 0.0f, 0.0f);
            #endif

            glAlphaFunc(GL_GREATER, sGeometry.m_fBranchAlphaTestValue / 255.0f); 
            for (int i = 0; i < sGeometry.m_sBranches.m_usNumStrips; ++i)
            {
                glBegin(GL_TRIANGLE_STRIP);
                for (int j = 0; j < sGeometry.m_sBranches.m_pStripLengths[i]; ++j)
                {
                    unsigned short usIndex = sGeometry.m_sBranches.m_pStrips[i][j];

                    // wind information is passed in on the 3rd texture layer
                    #ifdef WRAPPER_USE_GPU_WIND
                        // texcoord x = wind matrix index
                        // texcoord y = wind matrix weight
                        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,
                            sGeometry.m_sBranches.m_pWindMatrixIndices[usIndex] * 4.0f,
                            sGeometry.m_sBranches.m_pWindWeights[usIndex]);
                    #endif

                    #ifdef WRAPPER_RENDER_SELF_SHADOWS
                        // branch texture coordinates in 1st layer
                        glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, sGeometry.m_sBranches.m_pTexCoords0 + usIndex * 2);
                        // shadow texture coordinates in 2nd layer
                        if (sGeometry.m_sBranches.m_pTexCoords1)
                            glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, sGeometry.m_sBranches.m_pTexCoords1 + usIndex * 2);
                    #else
                        // branch texture coordinates in 1st layer
                        glTexCoord2fv(sGeometry.m_sBranches.m_pTexCoords0 + usIndex * 2);
                    #endif

                    // pass either vertex normal or color depending on lighting mode
                    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                        glNormal3fv(sGeometry.m_sBranches.m_pNormals + usIndex * 3);
                    #else
                        glColor4ubv(reinterpret_cast<const unsigned char*>(sGeometry.m_sBranches.m_pColors + usIndex));
                    #endif

                    // 3D coordinate always goes in
                    glVertex3fv(sGeometry.m_sBranches.m_pCoords + usIndex * 3);
                }
                glEnd( );
            }

            // translate back
            glPopMatrix( );
        }

    #else
        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_BranchGeometry);

        if (sGeometry.m_fBranchAlphaTestValue >= 0.0f)
        {
            // translate to the proper location
            glPushMatrix( );
            glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

            glAlphaFunc(GL_GREATER, sGeometry.m_fBranchAlphaTestValue / 255.0f); 

            #ifdef WRAPPER_USE_CPU_WIND
                if (sGeometry.m_sBranches.m_usNumStrips > 0 &&
                    m_pBranchVertexBuffer)
                {
                    // update the coordinates form the geometry structure since they've changed for CPU wind
                    (void) m_pBranchVertexBuffer->UpdatePartialBuffer(0, sGeometry.m_sBranches.m_pCoords,
                        3 * sizeof(float) * sGeometry.m_sBranches.m_usVertexCount);
                }
            #endif

            for (int i = 0; i < sGeometry.m_sBranches.m_usNumStrips; ++i)
                glDrawElements(GL_TRIANGLE_STRIP, sGeometry.m_sBranches.m_pStripLengths[i], GL_UNSIGNED_SHORT, sGeometry.m_sBranches.m_pStrips[i]);

            // translate back
            glPopMatrix( );
        }
    #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderFronds

void CSpeedTreeWrapper::RenderFronds(void) const
{
    if (m_pSpeedTree)
    {
    #ifdef WRAPPER_RENDER_IMMEDIATE_MODE
        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_FrondGeometry);

        if (sGeometry.m_fFrondAlphaTestValue >= 0.0f)
        {
            // translate to the proper location
            glPushMatrix( );
            glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

            // cancel the leaf lighting adjustment factor (only used for leaves)
            #ifdef LEAVES_USE_SHADERS
                glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, c_nVertexShader_LeafLightingAdjustment, 0.0f, 0.0f, 0.0f, 0.0f);
            #endif

            glAlphaFunc(GL_GREATER, sGeometry.m_fFrondAlphaTestValue / 255.0f); 
            for (int i = 0; i < sGeometry.m_sFronds.m_usNumStrips; ++i)
            {
                glBegin(GL_TRIANGLE_STRIP);
                for (int j = 0; j < sGeometry.m_sFronds.m_pStripLengths[i]; ++j)
                {
                    unsigned short usIndex = sGeometry.m_sFronds.m_pStrips[i][j];

                    // wind information is passed in on the 3rd texture layer
                    #ifdef WRAPPER_USE_GPU_WIND
                        // texcoord x = wind matrix index
                        // texcoord y = wind matrix weight
                        glMultiTexCoord2fARB(GL_TEXTURE2_ARB,
                            sGeometry.m_sFronds.m_pWindMatrixIndices[usIndex] * 4.0f,
                            sGeometry.m_sFronds.m_pWindWeights[usIndex]);
                    #endif

                    #ifdef WRAPPER_RENDER_SELF_SHADOWS
                        // frond texture coordinates in 1st layer
                        glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, sGeometry.m_sFronds.m_pTexCoords0 + usIndex * 2);
                        // shadow texture coordinates in 2nd layer
                        if (sGeometry.m_sFronds.m_pTexCoords1)
                            glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, sGeometry.m_sFronds.m_pTexCoords1 + usIndex * 2);
                    #else
                        // frond texture coordinates in 1st layer
                        glTexCoord2fv(sGeometry.m_sFronds.m_pTexCoords0 + usIndex * 2);
                    #endif

                    // pass either vertex normal or color depending on lighting mode
                    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                        glNormal3fv(sGeometry.m_sFronds.m_pNormals + usIndex * 3);
                    #else
                        glColor4ubv(reinterpret_cast<const unsigned char*>(sGeometry.m_sFronds.m_pColors + usIndex));
                    #endif

                    // 3D coordinate always goes in
                    glVertex3fv(sGeometry.m_sFronds.m_pCoords + usIndex * 3);
                }
                glEnd( );
            }

            // translate back
            glPopMatrix( );
        }
    #else
        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_FrondGeometry);

        if (sGeometry.m_fFrondAlphaTestValue >= 0.0f)
        {
            // translate to the proper location
            glPushMatrix( );
            glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

            glAlphaFunc(GL_GREATER, sGeometry.m_fFrondAlphaTestValue / 255.0f); 

            #ifdef WRAPPER_USE_CPU_WIND
                if (sGeometry.m_sFronds.m_usNumStrips > 0 &&
                    m_pFrondVertexBuffer)
                {
                    // update the coordinates form the geometry structure since they've changed for CPU wind
                    (void) m_pFrondVertexBuffer->UpdatePartialBuffer(0, sGeometry.m_sFronds.m_pCoords,
                        3 * sizeof(float) * sGeometry.m_sFronds.m_usVertexCount);
                }
            #endif

            for (int i = 0; i < sGeometry.m_sFronds.m_usNumStrips; ++i)
                glDrawElements(GL_TRIANGLE_STRIP, sGeometry.m_sFronds.m_pStripLengths[i], GL_UNSIGNED_SHORT, sGeometry.m_sFronds.m_pStrips[i]);

            // translate back
            glPopMatrix( );
        }
    #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderLeaves

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
inline float GetLeafTableIndex(unsigned int uiLeafMapIndex, unsigned int uiCorner)
{
    return c_nVertexShader_LeafTables + uiLeafMapIndex * 4 + uiCorner;
}
#endif

void CSpeedTreeWrapper::RenderLeaves(void) const
{
    if (m_pSpeedTree)
    {
    #ifdef WRAPPER_RENDER_IMMEDIATE_MODE
        // translate to the proper location
        glPushMatrix( );
        glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

        // load leaf lighting adjustment factor
        #ifdef LEAVES_USE_SHADERS
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, c_nVertexShader_LeafLightingAdjustment, m_pSpeedTree->GetLeafLightingAdjustment( ), 0.0f, 0.0f, 0.0f);
        #endif

        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_LeafGeometry);

        // may need to draw both
        for (int nLod = 0; nLod < 2; ++nLod)
        {
            // pick the lod structure
            const CSpeedTreeRT::SGeometry::SLeaf& sLod = !nLod ? sGeometry.m_sLeaves0 : sGeometry.m_sLeaves1;

            if (sLod.m_bIsActive)
            {
                // when using smooth transitions the alpha controls fading
                glAlphaFunc(GL_GREATER, sLod.m_fAlphaTestValue / 255.0f); 

                // set up data pointers
                const unsigned long* pColors = sLod.m_pColors;
                const float* pNormals = sLod.m_pNormals;
                const float* pPos = sLod.m_pCenterCoords;
                const unsigned char* pWindMatrixIndices = sLod.m_pWindMatrixIndices;
                const float* pWindWeights = sLod.m_pWindWeights;

                // draw leaves as quads
				glBegin(GL_QUADS);
                for (int i = 0; i < sLod.m_usLeafCount; ++i)
                {
                    // get coords and tex coords from leaf tables
                    const float* pClusterTexCoords = sLod.m_pLeafMapTexCoords[i];
                    const float* pClusterCoords = sLod.m_pLeafMapCoords[i];

                    // pass either vertex normal or color depending on lighting mode
                    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                        glNormal3fv(pNormals + i * 3);
                    #else
                        glColor4ubv(reinterpret_cast<const unsigned char*>(pColors + i));
                    #endif

                    #ifdef LEAVES_USE_SHADERS
                        float afShaderParameters[4] = { 0.0f };
                    #endif

                    // wind information is passed in on the 3rd texture layer
                    #ifdef WRAPPER_USE_GPU_WIND
                        // texcoord x = wind matrix index
                        // texcoord y = wind matrix weight
                        afShaderParameters[0] = pWindMatrixIndices[i] * 4.0f;
                        afShaderParameters[1] = pWindWeights[i];
                    #endif

                    // specify quad vertices offsetting table positions with actual leaf position
                    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                        // texcoord z = leaf cluster index
                        // texcoord w = LOD scale value
                        afShaderParameters[2] = GetLeafTableIndex(sLod.m_pLeafClusterIndices[i], 0);
                        afShaderParameters[3] = m_pSpeedTree->GetLeafLodSizeAdjustments( )[sLod.m_nDiscreteLodLevel];
                        glMultiTexCoord4fvARB(GL_TEXTURE2_ARB, afShaderParameters);
                        glTexCoord2fv(pClusterTexCoords);
                        glVertex3fv(pPos);

                        afShaderParameters[2] = GetLeafTableIndex(sLod.m_pLeafClusterIndices[i], 1);
                        glMultiTexCoord4fvARB(GL_TEXTURE2_ARB, afShaderParameters);
                        glTexCoord2fv(pClusterTexCoords + 2);
                        glVertex3fv(pPos);

                        afShaderParameters[2] = GetLeafTableIndex(sLod.m_pLeafClusterIndices[i], 2);
                        glMultiTexCoord4fvARB(GL_TEXTURE2_ARB, afShaderParameters);
                        glTexCoord2fv(pClusterTexCoords + 4);
                        glVertex3fv(pPos);

                        afShaderParameters[2] = GetLeafTableIndex(sLod.m_pLeafClusterIndices[i], 3);
                        glMultiTexCoord4fvARB(GL_TEXTURE2_ARB, afShaderParameters);
                        glTexCoord2fv(pClusterTexCoords + 6);
                        glVertex3fv(pPos);
                    #else
                        #if defined(WRAPPER_USE_GPU_WIND) || defined(WRAPPER_USE_GPU_LEAF_PLACEMENT)
                            glMultiTexCoord4fvARB(GL_TEXTURE2_ARB, afShaderParameters);
                        #endif
                        glTexCoord2fv(pClusterTexCoords);
                        glVertex3f(pClusterCoords[0] + pPos[0], pClusterCoords[1] + pPos[1], pClusterCoords[2] + pPos[2]);
                        glTexCoord2fv(pClusterTexCoords + 2);
                        glVertex3f(pClusterCoords[4] + pPos[0], pClusterCoords[5] + pPos[1], pClusterCoords[6] + pPos[2]);
                        glTexCoord2fv(pClusterTexCoords + 4);
                        glVertex3f(pClusterCoords[8] + pPos[0], pClusterCoords[9] + pPos[1], pClusterCoords[10] + pPos[2]);
                        glTexCoord2fv(pClusterTexCoords + 6);
                        glVertex3f(pClusterCoords[12] + pPos[0], pClusterCoords[13] + pPos[1], pClusterCoords[14] + pPos[2]);
                    #endif
                    pPos += 3;
                }
                glEnd( );
            }
        }

        // translate back
        glPopMatrix( );
    #else
        if (m_pLeafVertexBuffers && m_pLeavesUpdatedByCpu)
        {
            // translate to the proper location
            glPushMatrix( );
            glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

            // load leaf lighting adjustment factor
            #ifdef LEAVES_USE_SHADERS
                glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, c_nVertexShader_LeafLightingAdjustment, m_pSpeedTree->GetLeafLightingAdjustment( ), 0.0f, 0.0f, 0.0f);
            #endif

            // query the geometry (should be done once per unique tree)
            CSpeedTreeRT::SGeometry sGeometry;
            m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_LeafGeometry);

            // may need to draw both
            for (int nLod = 0; nLod < 2; ++nLod)
            {
                // pick the lod structure
                const CSpeedTreeRT::SGeometry::SLeaf& sLod = !nLod ? sGeometry.m_sLeaves0 : sGeometry.m_sLeaves1;

                if (sLod.m_bIsActive)
                {
                    #if defined(WRAPPER_USE_GPU_LEAF_PLACEMENT) && defined(WRAPPER_USE_GPU_WIND)
                        // do nothing
                    #else
                        if (!m_pLeavesUpdatedByCpu[sLod.m_nDiscreteLodLevel])
                        {
                            // We need a temporary buffer to update the coords, but we can't
                            // easily be sure how big needs to be.  So we reallocate it if
                            // it's not big enough
                            unsigned int uiBufferSize = sLod.m_usLeafCount * 4 * 3;
                            if (uiBufferSize > m_uiTmpCoordsBufferSize)
                            {
                                m_uiTmpCoordsBufferSize = uiBufferSize;
                                delete[] m_pTmpCoordsBuffer;
                                m_pTmpCoordsBuffer = new float[m_uiTmpCoordsBufferSize];
                            }

                            float* pVertex = m_pTmpCoordsBuffer;
                            const float* pCenterPos = sLod.m_pCenterCoords;
                            for (int nLeaf = 0; nLeaf < sLod.m_usLeafCount; ++nLeaf)
                            {
                                #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                                    float pClusterCoords[15] = { 0.0f };
                                #else
                                    const float* pClusterCoords = sLod.m_pLeafMapCoords[nLeaf];
                                #endif

                                // vertex 0
                                *pVertex++ = pClusterCoords[0] + pCenterPos[0];
                                *pVertex++ = pClusterCoords[1] + pCenterPos[1];
                                *pVertex++ = pClusterCoords[2] + pCenterPos[2];

                                // vertex 1
                                *pVertex++ = pClusterCoords[4] + pCenterPos[0];
                                *pVertex++ = pClusterCoords[5] + pCenterPos[1];
                                *pVertex++ = pClusterCoords[6] + pCenterPos[2];

                                // vertex 2
                                *pVertex++ = pClusterCoords[8] + pCenterPos[0];
                                *pVertex++ = pClusterCoords[9] + pCenterPos[1];
                                *pVertex++ = pClusterCoords[10] + pCenterPos[2];

                                // vertex 3
                                *pVertex++ = pClusterCoords[12] + pCenterPos[0];
                                *pVertex++ = pClusterCoords[13] + pCenterPos[1];
                                *pVertex++ = pClusterCoords[14] + pCenterPos[2];

                                pCenterPos += 3;
                            }
                            (void) m_pLeafVertexBuffers[sLod.m_nDiscreteLodLevel].UpdatePartialBuffer(0, m_pTmpCoordsBuffer, uiBufferSize * sizeof(float));
                            m_pLeavesUpdatedByCpu[sLod.m_nDiscreteLodLevel] = true;
                        }
                    #endif

                    // when using smooth transitions the alpha controls fading
                    glDisable(GL_CULL_FACE);
                    glAlphaFunc(GL_GREATER, sLod.m_fAlphaTestValue / 255.0f); 

                    if (sLod.m_nDiscreteLodLevel >= 0 &&
                        sLod.m_usLeafCount > 0)
                    {
                        (void) m_pLeafVertexBuffers[sLod.m_nDiscreteLodLevel].Bind( );
                        glDrawArrays(GL_QUADS, 0, sLod.m_usLeafCount * 4);
                    }
                }
            }

            // translate back
            glPopMatrix( );
        }
    #endif
    }
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
    uiEntryCount /= 4;
    for (unsigned int i = 0; i < uiEntryCount; ++i)
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, c_nVertexShader_LeafTables + i, pTable + i * 4);
}
#endif


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::RenderBillboards

void CSpeedTreeWrapper::RenderBillboards(void) const
{
    if (m_pSpeedTree)
    {
        // translate to the proper location
        glPushMatrix( );
        glTranslatef(m_afPos[0], m_afPos[1], m_afPos[2]);

        // query the geometry (should be done once per unique tree)
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_BillboardGeometry);

        // may need to draw both billboards (cross-fading 360 mode)
        for (int nBB = 0; nBB < 2; ++nBB)
        {
            // pick the billboard structure
            const CSpeedTreeRT::SGeometry::SBillboard& sBB= !nBB ? sGeometry.m_sBillboard0 : sGeometry.m_sBillboard1;

            if (sBB.m_bIsActive)
            {
                // when using smooth transitions the alpha controls fading
                glAlphaFunc(GL_GREATER, sBB.m_fAlphaTestValue / 255.0f); 

                const float* pTexCoords = sBB.m_pTexCoords;
                const float* pCoords = sBB.m_pCoords;

                glBegin(GL_QUADS);
                    glTexCoord2fv(pTexCoords);
                    glVertex3fv(pCoords);
                    glTexCoord2fv(pTexCoords + 2);
                    glVertex3fv(pCoords + 3);
                    glTexCoord2fv(pTexCoords + 4);
                    glVertex3fv(pCoords + 6);
                    glTexCoord2fv(pTexCoords + 6);
                    glVertex3fv(pCoords + 9);
                glEnd( );
            }
        }

        #ifdef WRAPPER_RENDER_HORIZONTAL_BILLBOARD
            // horizontal billboard
            if (sGeometry.m_sHorizontalBillboard.m_bIsActive)
            {
                // when using smooth transitions the alpha controls fading
                glAlphaFunc(GL_GREATER, sGeometry.m_sHorizontalBillboard.m_fAlphaTestValue / 255.0f); 

                const float* pTexCoords = sGeometry.m_sHorizontalBillboard.m_pTexCoords;
                const float* pCoords = sGeometry.m_sHorizontalBillboard.m_pCoords;

                glBegin(GL_QUADS);
                    glTexCoord2fv(pTexCoords);
                    glVertex3fv(pCoords);
                    glTexCoord2fv(pTexCoords + 2);
                    glVertex3fv(pCoords + 3);
                    glTexCoord2fv(pTexCoords + 4);
                    glVertex3fv(pCoords + 6);
                    glTexCoord2fv(pTexCoords + 6);
                    glVertex3fv(pCoords + 9);
                glEnd( );
            }
        #endif

        // translate back
        glPopMatrix( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetPos

void CSpeedTreeWrapper::SetPos(const float* pPos)
{
    if (m_pSpeedTree)
    {
        m_afPos[0] = pPos[0];
        m_afPos[1] = pPos[1];
        m_afPos[2] = pPos[2];

        m_pSpeedTree->SetTreePosition(pPos[0], pPos[1], pPos[2]);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchForTreeType

void CSpeedTreeWrapper::SetupBranchForTreeType(void)
{
    GetBranchMaterial( ).Activate( );

    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_uiSelfShadowTexture)
        {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glBindTexture(GL_TEXTURE_2D, m_uiSelfShadowTexture);
            glEnable(GL_TEXTURE_2D);

            glActiveTextureARB(GL_TEXTURE0_ARB);
            glBindTexture(GL_TEXTURE_2D, m_uiBranchTexture);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, GetBranchTexture( ));
        }
    #else
        glBindTexture(GL_TEXTURE_2D, GetBranchTexture( ));
    #endif

    #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        if (m_pBranchVertexBuffer)
            (void) m_pBranchVertexBuffer->Bind( );
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondForTreeType

void CSpeedTreeWrapper::SetupFrondForTreeType(void)
{
    GetFrondMaterial( ).Activate( );

    #ifdef WRAPPER_RENDER_SELF_SHADOWS
        if (m_uiSelfShadowTexture)
        {
            glActiveTextureARB(GL_TEXTURE1_ARB);
            glBindTexture(GL_TEXTURE_2D, m_uiSelfShadowTexture);
            glEnable(GL_TEXTURE_2D);

            glActiveTextureARB(GL_TEXTURE0_ARB);
        }
    #endif

    #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        if (m_pFrondVertexBuffer)
            (void) m_pFrondVertexBuffer->Bind( );
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafForTreeType

void CSpeedTreeWrapper::SetupLeafForTreeType(void)
{
    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        UploadLeafTables(c_nVertexShader_LeafTables);
    #endif
    GetLeafMaterial( ).Activate( );

    #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        if (m_pSpeedTree && m_pLeavesUpdatedByCpu)
        {
            // reset the update flags
            for (int i = 0; i < m_pSpeedTree->GetNumLeafLodLevels( ); ++i)
                m_pLeavesUpdatedByCpu[i] = false;
        }
    #endif
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::BeginBranchRenderingStates

#ifdef WRAPPER_RENDER_VERTEX_OBJECTS
void CSpeedTreeWrapper::BeginBranchRenderingStates(void) const
{
    if (m_pBranchVertexBuffer)
    {
        #ifdef WRAPPER_USE_CPU_WIND
            CIdvVertexBuffer::Enable(false); // use dynamic buffer
        #else
            CIdvVertexBuffer::Enable(true); // use static buffer
        #endif

        m_pBranchVertexBuffer->EnableClientStates( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::EndBranchRenderingStates

void CSpeedTreeWrapper::EndBranchRenderingStates(void) const
{
    if (m_pBranchVertexBuffer)
        m_pBranchVertexBuffer->DisableClientStates( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::BeginFrondRenderingStates

void CSpeedTreeWrapper::BeginFrondRenderingStates(void) const
{
    if (m_pFrondVertexBuffer)
    {
        #ifdef WRAPPER_USE_CPU_WIND
            CIdvVertexBuffer::Enable(false); // use dynamic buffer
        #else
            CIdvVertexBuffer::Enable(true); // use static buffer
        #endif

        m_pFrondVertexBuffer->EnableClientStates( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::EndFrondRenderingStates

void CSpeedTreeWrapper::EndFrondRenderingStates(void) const
{
    if (m_pFrondVertexBuffer)
        m_pFrondVertexBuffer->DisableClientStates( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::BeginLeafRenderingStates

void CSpeedTreeWrapper::BeginLeafRenderingStates(void) const
{
    if (m_pLeafVertexBuffers)
    {
        #ifdef WRAPPER_USE_CPU_WIND
            CIdvVertexBuffer::Enable(false); // use dynamic buffer
        #else
            CIdvVertexBuffer::Enable(true); // use static buffer
        #endif

        m_pLeafVertexBuffers[0].EnableClientStates( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::EndLeafRenderingStates

void CSpeedTreeWrapper::EndLeafRenderingStates(void) const
{
    if (m_pLeafVertexBuffers)
        m_pLeafVertexBuffers[0].DisableClientStates( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupVertexBuffers

void CSpeedTreeWrapper::SetupVertexBuffers(void)
{
    SetupBranchBuffer( );
    SetupFrondBuffer( );
    SetupLeafBuffers( );
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupBranchBuffer

void CSpeedTreeWrapper::SetupBranchBuffer(void)
{
    if (m_pSpeedTree)
    {
        // define branch vertex size based on sample configuration
        const int c_nVertexSize = 
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            3 * sizeof(float) +     // normal
        #else
            sizeof(unsigned long) + // color
        #endif
            2 * sizeof(float) +     // texcoords0
        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            2 * sizeof(float) +     // texcoords1 (self-shadow)
        #endif
        #ifdef WRAPPER_USE_GPU_WIND
            2 * sizeof(float) +     // texcoords2 (wind)
        #endif
            3 * sizeof(float);      // coords

        // allocate buffer object that will house everything
        m_pBranchVertexBuffer = new CIdvVertexBuffer;

        // get highest LOD branch vertices
        m_pSpeedTree->SetLodLevel(1.0f);

        // query geometry
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_BranchGeometry);
        const CSpeedTreeRT::SGeometry::SIndexed& sBranches = sGeometry.m_sBranches;

        // allocate overall buffer
        unsigned char* pBuffer = new unsigned char[c_nVertexSize * sBranches.m_usVertexCount];

        unsigned int uiOffset = 0;

        // copy coords
        memcpy(pBuffer + uiOffset, sBranches.m_pCoords, sBranches.m_usVertexCount * 3 * sizeof(float));
        uiOffset = sBranches.m_usVertexCount * 3 * sizeof(float);

        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            // copy normals
            unsigned int uiNormalOffset = uiOffset;
            memcpy(pBuffer + uiOffset, sBranches.m_pNormals, sBranches.m_usVertexCount * 3 * sizeof(float));
            uiOffset += sBranches.m_usVertexCount * 3 * sizeof(float);
        #else
            // copy colors
            unsigned int uiColorOffset = uiOffset;
            memcpy(pBuffer + uiOffset, sBranches.m_pColors, sBranches.m_usVertexCount * sizeof(unsigned long));
            uiOffset += sBranches.m_usVertexCount * sizeof(unsigned long);
        #endif

        // copy texcoords0
        unsigned int uiTexCoords0Offset = uiOffset;
        memcpy(pBuffer + uiOffset, sBranches.m_pTexCoords0, sBranches.m_usVertexCount * 2 * sizeof(float));
        uiOffset += sBranches.m_usVertexCount * 2 * sizeof(float);

        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            // copy texcoords1
            unsigned int uiTexCoords1Offset = uiOffset;
            if (sBranches.m_pTexCoords1)
            {
                memcpy(pBuffer + uiOffset, sBranches.m_pTexCoords1, sBranches.m_usVertexCount * 2 * sizeof(float));
                uiOffset += sBranches.m_usVertexCount * 2 * sizeof(float);
            }
            else
            {
                // if no coordinates are present, just use zeros
                float afZeros[2] = { 0.0f, 0.0f };
                for (int i = 0; i < sBranches.m_usVertexCount; ++i)
                {
                    memcpy(pBuffer + uiOffset, afZeros, 2 * sizeof(float));
                    uiOffset += 2 * sizeof(float);
                }
            }
        #endif

        #ifdef WRAPPER_USE_GPU_WIND
            // copy texcoords2 (wind attributes)
            unsigned int uiTexCoords2Offset = uiOffset;
            for (int i = 0; i < sBranches.m_usVertexCount; ++i)
            {
                float afWind[2] = { sBranches.m_pWindMatrixIndices[i] * 4.0f, sBranches.m_pWindWeights[i] };
                memcpy(pBuffer + uiOffset, afWind, 2 * sizeof(float));
                uiOffset += 2 * sizeof(float);
            }
        #endif

        // pass buffer parameters into CIdvVertexBuffer
        #if defined(WRAPPER_USE_CPU_LEAF_PLACEMENT) || defined(WRAPPER_USE_CPU_WIND)
            (void) m_pBranchVertexBuffer->SetBuffer(pBuffer, c_nVertexSize * sBranches.m_usVertexCount, false);
        #else
            (void) m_pBranchVertexBuffer->SetBuffer(pBuffer, c_nVertexSize * sBranches.m_usVertexCount, true);
        #endif

        // coords
        m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_POSITION, 3, GL_FLOAT, 0, 0);

        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            // normals
            m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_NORMAL, 3, GL_FLOAT, 0, uiNormalOffset);
        #else
            // color
            m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_COLOR, 4, GL_UNSIGNED_BYTE, 0, uiColorOffset);
        #endif

        // texcoords0
        m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD0, 2, GL_FLOAT, 0, uiTexCoords0Offset);

        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            // texcoords1
            m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD1, 2, GL_FLOAT, 0, uiTexCoords1Offset);
        #endif

        #ifdef WRAPPER_USE_GPU_WIND
            // texcoords2 carry the vertex wind matrix index and wind weight
            m_pBranchVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD2, 2, GL_FLOAT, 0, uiTexCoords2Offset);
        #endif

        // release buffer (it was copied when pass into m_pBranchVertexBuffer)
        delete[] pBuffer;
        pBuffer = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupFrondBuffer

void CSpeedTreeWrapper::SetupFrondBuffer(void)
{
    if (m_pSpeedTree)
    {
        // define frond vertex size based on sample configuration
        const int c_nVertexSize = 
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            3 * sizeof(float) +     // normal
        #else
            sizeof(unsigned long) + // color
        #endif
            2 * sizeof(float) +     // texcoords0
        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            2 * sizeof(float) +     // texcoords1 (self-shadow)
        #endif
        #ifdef WRAPPER_USE_GPU_WIND
            2 * sizeof(float) +     // texcoords2 (wind)
        #endif
            3 * sizeof(float);      // coords

        // allocate buffer object that will house everything
        m_pFrondVertexBuffer = new CIdvVertexBuffer;

        // get highest LOD Frond vertices
        m_pSpeedTree->SetLodLevel(1.0f);

        // query geometry
        CSpeedTreeRT::SGeometry sGeometry;
        m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_FrondGeometry);
        const CSpeedTreeRT::SGeometry::SIndexed& sFronds = sGeometry.m_sFronds;

        // allocate overall buffer
        unsigned char* pBuffer = new unsigned char[c_nVertexSize * sFronds.m_usVertexCount];

        unsigned int uiOffset = 0;

        // copy coords
        memcpy(pBuffer + uiOffset, sFronds.m_pCoords, sFronds.m_usVertexCount * 3 * sizeof(float));
        uiOffset = sFronds.m_usVertexCount * 3 * sizeof(float);

        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            // copy normals
            unsigned int uiNormalOffset = uiOffset;
            memcpy(pBuffer + uiOffset, sFronds.m_pNormals, sFronds.m_usVertexCount * 3 * sizeof(float));
            uiOffset += sFronds.m_usVertexCount * 3 * sizeof(float);
        #else
            // copy colors
            unsigned int uiColorOffset = uiOffset;
            memcpy(pBuffer + uiOffset, sFronds.m_pColors, sFronds.m_usVertexCount * sizeof(unsigned long));
            uiOffset += sFronds.m_usVertexCount * sizeof(unsigned long);
        #endif

        // copy texcoords0
        unsigned int uiTexCoords0Offset = uiOffset;
        memcpy(pBuffer + uiOffset, sFronds.m_pTexCoords0, sFronds.m_usVertexCount * 2 * sizeof(float));
        uiOffset += sFronds.m_usVertexCount * 2 * sizeof(float);

        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            // copy texcoords1
            unsigned int uiTexCoords1Offset = uiOffset;
            if (sFronds.m_pTexCoords1)
            {
                memcpy(pBuffer + uiOffset, sFronds.m_pTexCoords1, sFronds.m_usVertexCount * 2 * sizeof(float));
                uiOffset += sFronds.m_usVertexCount * 2 * sizeof(float);
            }
            else
            {
                // if no coordinates are present, just use zeros
                float afZeros[2] = { 0.0f, 0.0f };
                for (int i = 0; i < sFronds.m_usVertexCount; ++i)
                {
                    memcpy(pBuffer + uiOffset, afZeros, 2 * sizeof(float));
                    uiOffset += 2 * sizeof(float);
                }
            }
        #endif

        #ifdef WRAPPER_USE_GPU_WIND
            // copy texcoords2 (wind attributes)
            unsigned int uiTexCoords2Offset = uiOffset;
            for (int i = 0; i < sFronds.m_usVertexCount; ++i)
            {
                float afWind[2] = { sFronds.m_pWindMatrixIndices[i] * 4.0f, sFronds.m_pWindWeights[i] };
                memcpy(pBuffer + uiOffset, afWind, 2 * sizeof(float));
                uiOffset += 2 * sizeof(float);
            }
        #endif

        // pass buffer parameters into CIdvVertexBuffer
        #if defined(WRAPPER_USE_CPU_LEAF_PLACEMENT) || defined(WRAPPER_USE_CPU_WIND)
            (void) m_pFrondVertexBuffer->SetBuffer(pBuffer, c_nVertexSize * sFronds.m_usVertexCount, false);
        #else
            (void) m_pFrondVertexBuffer->SetBuffer(pBuffer, c_nVertexSize * sFronds.m_usVertexCount, true);
        #endif

        // coords
        m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_POSITION, 3, GL_FLOAT, 0, 0);

        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            // normals
            m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_NORMAL, 3, GL_FLOAT, 0, uiNormalOffset);
        #else
            // color
            m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_COLOR, 4, GL_UNSIGNED_BYTE, 0, uiColorOffset);
        #endif

        // texcoords0
        m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD0, 2, GL_FLOAT, 0, uiTexCoords0Offset);

        #ifdef WRAPPER_RENDER_SELF_SHADOWS
            // texcoords1
            m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD1, 2, GL_FLOAT, 0, uiTexCoords1Offset);
        #endif

        #ifdef WRAPPER_USE_GPU_WIND
            // texcoords2 carry the vertex wind matrix index and wind weight
            m_pFrondVertexBuffer->SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD2, 2, GL_FLOAT, 0, uiTexCoords2Offset);
        #endif

        // release buffer (it was copied when pass into m_pFrondVertexBuffer)
        delete[] pBuffer;
        pBuffer = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeWrapper::SetupLeafBuffers

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
    if (m_pSpeedTree)
    {
        // define leaf vertex size based on sample configuration
        const int c_nVertexSize = 
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            3 * sizeof(float) +     // normal
        #else
            sizeof(unsigned long) + // color
        #endif
            2 * sizeof(float) +     // texcoords0
        #if defined(WRAPPER_USE_GPU_LEAF_PLACEMENT) || defined(WRAPPER_USE_GPU_WIND)
            4 * sizeof(float) +     // texcoords2 (wind & leaf cluster)
        #endif
            3 * sizeof(float);      // coords

        // allocate buffer object that will house everything
        unsigned short usLodCount = m_pSpeedTree->GetNumLeafLodLevels( );
        m_pLeafVertexBuffers = new CIdvVertexBuffer[usLodCount];
        m_pLeavesUpdatedByCpu = new bool[usLodCount];

        // determine total leaf count in all LODs combined
        for (int nLod = 0; nLod < usLodCount; ++nLod)
        {
            // initialize flags
            m_pLeavesUpdatedByCpu[nLod] = false;

            CSpeedTreeRT::SGeometry sGeometry;
            m_pSpeedTree->GetGeometry(sGeometry, SpeedTree_LeafGeometry, -1, -1, nLod);
            const CSpeedTreeRT::SGeometry::SLeaf& sLeaves = sGeometry.m_sLeaves0;

            // allocate buffer
            unsigned int uiBufferSize = c_nVertexSize * 4 * sLeaves.m_usLeafCount; // 4 for corners
            unsigned char* pBuffer = new unsigned char[uiBufferSize];

            // copy coords
            unsigned int uiOffset = 0;
			for (int nLeaf = 0; nLeaf < sLeaves.m_usLeafCount; ++nLeaf)
			{
				#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
					for (int nCorner = 0; nCorner < 4; ++nCorner)
					{
						memcpy(pBuffer + uiOffset, sLeaves.m_pCenterCoords + nLeaf * 3, 3 * sizeof(float));
						uiOffset += 3 * sizeof(float);
					}
				#else
					// CPU leaves will be updated in RenderLeaves
					uiOffset += 4 * 3 * sizeof(float); // corners * xyz * float
				#endif
			}

            // copy lighting information (static or dynamic)
            unsigned int uiColorOffset = uiOffset;
            unsigned int uiNormalOffset = uiOffset;
            for (int nLeaf = 0; nLeaf < sLeaves.m_usLeafCount; ++nLeaf)
            {
                #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                    // copy normals
                    for (int nCorner = 0; nCorner < 4; ++nCorner)
                    {
                        memcpy(pBuffer + uiOffset, sLeaves.m_pNormals + nLeaf * 3, 3 * sizeof(float));
                        uiOffset += 3 * sizeof(float);
                    }
                #else
                    // copy colors
                    for (int nCorner = 0; nCorner < 4; ++nCorner)
                    {
                        memcpy(pBuffer + uiOffset, sLeaves.m_pColors + nLeaf, sizeof(unsigned long));
                        uiOffset += sizeof(unsigned long);
                    }
                #endif
            }

            // copy texture layer 0 coordinates
            unsigned int uiTexCoords0Offset = uiOffset;
            for (int nLeaf = 0; nLeaf < sLeaves.m_usLeafCount; ++nLeaf)
            {
                memcpy(pBuffer + uiOffset, sLeaves.m_pLeafMapTexCoords[nLeaf], 8 * sizeof(float));
                uiOffset += 8 * sizeof(float);
            }

            // setup up wind and leaf index parameters in 3rd texture layer
            #if defined(WRAPPER_USE_GPU_WIND) || defined(WRAPPER_USE_GPU_LEAF_PLACEMENT)
                unsigned int uiTexCoords2Offset = uiOffset;
                for (int nLeaf = 0; nLeaf < sLeaves.m_usLeafCount; ++nLeaf)
                {
                    for (int nCorner = 0; nCorner < 4; ++nCorner)
                    {
                        float afShaderParameters[4] = { 0.0f };
                        #ifdef WRAPPER_USE_GPU_WIND
                            afShaderParameters[0] = sLeaves.m_pWindMatrixIndices[nLeaf] * 4.0f;
                            afShaderParameters[1] = sLeaves.m_pWindWeights[nLeaf];
                        #endif

                        #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                            afShaderParameters[2] = GetLeafTableIndex(sLeaves.m_pLeafClusterIndices[nLeaf], nCorner);
                            afShaderParameters[3] = m_pSpeedTree->GetLeafLodSizeAdjustments( )[nLod];
                        #endif

                        memcpy(pBuffer + uiOffset, afShaderParameters, 4 * sizeof(float));
                        uiOffset += 4 * sizeof(float);
                    }
                }
            #endif

            // pass buffer parameters into CIdvVertexBuffer
            (void) m_pLeafVertexBuffers[nLod].SetBuffer(pBuffer, uiBufferSize, true);

            // coords
            m_pLeafVertexBuffers[nLod].SetStride(CIdvVertexBuffer::VERTEX_POSITION, 3, GL_FLOAT, 0, 0);

            // lighting parameters
            #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                // normals
                m_pLeafVertexBuffers[nLod].SetStride(CIdvVertexBuffer::VERTEX_NORMAL, 3, GL_FLOAT, 0, uiNormalOffset);
            #else
                // color
                m_pLeafVertexBuffers[nLod].SetStride(CIdvVertexBuffer::VERTEX_COLOR, 4, GL_UNSIGNED_BYTE, 0, uiColorOffset);
            #endif

            // texcoords0
            m_pLeafVertexBuffers[nLod].SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD0, 2, GL_FLOAT, 0, uiTexCoords0Offset);

            // texcoords2
            #if defined(WRAPPER_USE_GPU_WIND) || defined(WRAPPER_USE_GPU_LEAF_PLACEMENT)
                m_pLeafVertexBuffers[nLod].SetStride(CIdvVertexBuffer::VERTEX_TEXCOORD2, 4, GL_FLOAT, 0, uiTexCoords2Offset);
            #endif

            // release buffer (it was copied when pass into m_pFrondVertexBuffer)
            delete[] pBuffer;
            pBuffer = NULL;
        }
    }
}
#endif