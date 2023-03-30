///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL Class
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

#include "SpeedTreeForestOpenGL.h"
#include "nv_dds.h"
#include "VP_Basic_StaticLighting_GpuWind.h"
#include "VP_Basic_DynamicLighting_GpuWind.h"
#include "VP_Leaves_StaticLighting_GpuPlacement_CpuWind.h"
#include "VP_Leaves_StaticLighting_GpuPlacement_GpuWind.h"
#include "VP_Leaves_DynamicLighting_GpuPlacement_CpuWind.h"
#include "VP_Leaves_DynamicLighting_GpuPlacement_GpuWind.h"


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::CSpeedTreeForestOpenGL

CSpeedTreeForestOpenGL::CSpeedTreeForestOpenGL( ) :
    m_uiCompositeMap(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::~CSpeedTreeForestOpenGL

CSpeedTreeForestOpenGL::~CSpeedTreeForestOpenGL( )
{
    if (m_uiCompositeMap)
    {
        glDeleteTextures(1, &m_uiCompositeMap);
        m_uiCompositeMap = 0;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::UploadWindMatrix

void CSpeedTreeForestOpenGL::UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const
{
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, uiLocation + 0, pMatrix[0], pMatrix[4], pMatrix[8], pMatrix[12]);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, uiLocation + 1, pMatrix[1], pMatrix[5], pMatrix[9], pMatrix[13]);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, uiLocation + 2, pMatrix[2], pMatrix[6], pMatrix[10], pMatrix[14]);
    glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, uiLocation + 3, pMatrix[3], pMatrix[7], pMatrix[11], pMatrix[15]);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::InitGraphics

bool CSpeedTreeForestOpenGL::InitGraphics(void)
{
    bool bSuccess = false;

    // load leaf texture
    m_uiCompositeMap = LoadDDS(c_pCompositeMapFilename);

    // init graphics
    if (InitVertexShaders( ))
        bSuccess = true;

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::Render

void CSpeedTreeForestOpenGL::Render(unsigned long ulRenderBitVector) const
{
    // advance all trees' LOD and wind
    for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
        m_vAllTrees[i]->Advance( );

    if (!m_vAllTrees.empty( ))
    {
        // setup general graphics states
        glEnable(GL_CULL_FACE);
        glEnable(GL_ALPHA_TEST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        #ifdef BRANCHES_USE_SHADERS
            m_cBranchVertexProgram.Bind( );
            CVertexProgram::Enable( );
        #endif

        // render branches
        if (ulRenderBitVector & Forest_RenderBranches)
        {
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
                m_vAllTrees[0]->BeginBranchRenderingStates( );
            #endif
            for (unsigned int j = 0; j < m_vAllTrees.size( ); ++j)
            {
                if (!m_vAllTrees[j]->IsInstance( ))
                    m_vAllTrees[j]->SetupBranchForTreeType( );
                m_vAllTrees[j]->RenderBranches( );
            }
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            m_vAllTrees[0]->EndBranchRenderingStates( );
            #endif

            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE0_ARB);
            #endif
        }

        // bind composite map & set render states
        glBindTexture(GL_TEXTURE_2D, m_uiCompositeMap);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glDisable(GL_CULL_FACE);

        // render fronds
        if (ulRenderBitVector & Forest_RenderFronds)
        {
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
                m_vAllTrees[0]->BeginFrondRenderingStates( );
            #endif
            for (unsigned int k = 0; k < m_vAllTrees.size( ); ++k)
            {
                if (!m_vAllTrees[k]->IsInstance( ))
                    m_vAllTrees[k]->SetupFrondForTreeType( );
                m_vAllTrees[k]->RenderFronds( );
            }
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            m_vAllTrees[0]->EndFrondRenderingStates( );
            #endif

            #ifdef WRAPPER_RENDER_SELF_SHADOWS
                glActiveTextureARB(GL_TEXTURE1_ARB);
                glDisable(GL_TEXTURE_2D);
                glActiveTextureARB(GL_TEXTURE0_ARB);
            #endif
        }

        // render leaves
        if (ulRenderBitVector & Forest_RenderLeaves)
        {
            #ifdef LEAVES_USE_SHADERS
                m_cLeafVertexProgram.Bind( );
                CVertexProgram::Enable( );
            #endif
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
                m_vAllTrees[0]->BeginLeafRenderingStates( );
            #endif
            for (unsigned int l = 0; l < m_vAllTrees.size( ); ++l)
            {
                if (!m_vAllTrees[l]->IsInstance( ))
                    m_vAllTrees[l]->SetupLeafForTreeType( );
                m_vAllTrees[l]->RenderLeaves( );
            }
            #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
            m_vAllTrees[0]->EndLeafRenderingStates( );
            #endif
        }

        #if defined(BRANCHES_USE_SHADERS) || defined(LEAVES_USE_SHADERS)
            CVertexProgram::Disable( );
        #endif

        // render billboards
        #ifdef WRAPPER_BILLBOARD_MODE
        if (ulRenderBitVector & Forest_RenderBillboards)
        {
            glPushAttrib(GL_ENABLE_BIT);
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 1.0f, 1.0f);
            for (unsigned int m = 0; m < m_vAllTrees.size( ); ++m)
                m_vAllTrees[m]->RenderBillboards( );
            glPopAttrib( );
        }
        #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL::InitVertexShaders

bool CSpeedTreeForestOpenGL::InitVertexShaders(void)
{
    bool bSuccess = true;

    // branches/fronds only use shaders for GPU wind
    #ifdef BRANCHES_USE_SHADERS
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            bSuccess &= m_cBranchVertexProgram.Load(szVP_Basic_DynamicLighting_GpuWind);
        #else
            bSuccess &= m_cBranchVertexProgram.Load(szVP_Basic_StaticLighting_GpuWind);
        #endif
    #endif

    // leaves can use shaders for GPU wind and leaf placement - there are six unique
    // shaders that handle GPU wind, GPU leaf placement, and dynamic/static lighting
    #ifdef WRAPPER_USE_GPU_WIND
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Leaves_DynamicLighting_GpuPlacement_GpuWind);
            #else // cpu leaf placement
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Basic_DynamicLighting_GpuWind);
            #endif
        #else // static lighting
            #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Leaves_StaticLighting_GpuPlacement_GpuWind);
            #else // cpu leaf placement
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Basic_StaticLighting_GpuWind);
            #endif
        #endif
    #else // cpu wind
        // shader for leaves is determined by leaf placement type and lighting type
        #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
            #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Leaves_DynamicLighting_GpuPlacement_CpuWind);
            #else // cpu leaf placement
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Basic_DynamicLighting_GpuWind);
            #endif
        #else // static lighting
            //lint -e{514}
            #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Leaves_StaticLighting_GpuPlacement_CpuWind);
            #else // cpu leaf placement
                bSuccess &= m_cLeafVertexProgram.Load(szVP_Basic_StaticLighting_GpuWind);
            #endif
        #endif
    #endif

    return bSuccess;
}
