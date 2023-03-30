///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8 Class
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

#pragma warning(disable : 4786)

#include "SpeedTreeForestDirectX8.h"
#include <d3d8.h>
#include <d3d8types.h>
#include <d3dx8.h>
#include "D3DUtil.h"
#include "DXUtil.h"
#include "VertexShaders.h"


///////////////////////////////////////////////////////////////////////  
//  static member variables

LPDIRECT3DDEVICE8 CSpeedTreeForestDirectX8::m_pDx;


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8( )
{
    SAFE_RELEASE(m_texCompositeMap);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::UploadWindMatrix

void CSpeedTreeForestDirectX8::UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const
{
    m_pDx->SetVertexShaderConstant(uiLocation, pMatrix, 4);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::InitGraphics

bool CSpeedTreeForestDirectX8::InitGraphics(void)
{
    bool bSuccess = false;

    // load the composite map
    if (D3DUtil_CreateTexture(m_pDx, c_pCompositeMapFilename, &m_texCompositeMap) != S_OK)
        MessageBox(NULL, "Failed to create leaf texture.", "File Error", MB_ICONEXCLAMATION);
    else
    {
        // init the vertex shaders (if we need them)
        bSuccess = InitVertexShaders( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::Render

void CSpeedTreeForestDirectX8::Render(unsigned long ulRenderBitVector) const
{
    // advance all trees' LOD and wind
    for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
        m_vAllTrees[i]->Advance( );

    if (!m_vAllTrees.empty( ))
    {
        // setup general graphics states
        m_pDx->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pDx->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        m_pDx->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        m_pDx->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
        m_pDx->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
        m_pDx->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

        // set up fog if it is enabled
        #ifdef WRAPPER_USE_FOG
            m_pDx->SetRenderState(D3DRS_FOGENABLE, TRUE);
            m_pDx->SetRenderState(D3DRS_FOGCOLOR, 0xFF334C66);
            m_pDx->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
            #ifdef WRAPPER_USE_GPU_WIND
                m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); // GPU needs to work on all cards
            #else
                m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
                m_pDx->SetRenderState(D3DRS_FOGSTART, *((DWORD*) (&c_fFogStart)));
                m_pDx->SetRenderState(D3DRS_FOGEND, *((DWORD*) (&c_fFogEnd)));
            #endif  
        #endif

        // choose fixed function pipeline or custom shader for fronds and branches
        m_pDx->SetVertexShader(m_dwBranchVertexShader);

        // render branches
        m_pDx->SetVertexShader(m_dwBranchVertexShader);
        if (ulRenderBitVector & Forest_RenderBranches)
        {
            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupBranchForTreeType( );
                m_vAllTrees[i]->RenderBranches( );
            }
        }

        // bind composite map & set render states
        m_pDx->SetTexture(0, m_texCompositeMap);
        m_pDx->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        // render fronds
        if (ulRenderBitVector & Forest_RenderFronds)
        {
            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupFrondForTreeType( );
                m_vAllTrees[i]->RenderFronds( );
            }
        }

        // render leaves
        if (ulRenderBitVector & Forest_RenderLeaves)
        {
            m_pDx->SetVertexShader(m_dwLeafVertexShader);

            #ifdef WRAPPER_USE_FOG
                #if defined WRAPPER_USE_GPU_WIND || defined WRAPPER_USE_GPU_LEAF_PLACEMENT
                    m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
                #endif  
            #endif

            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupLeafForTreeType( );
                m_vAllTrees[i]->RenderLeaves( );
            }
            for (i = 0; i < m_vAllTrees.size( ); ++i)
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->EndLeafForTreeType( );
        }

        // render billboards
        #ifndef WRAPPER_NO_BILLBOARD_MODE
            if (ulRenderBitVector & Forest_RenderBillboards)
            {
                m_pDx->SetRenderState(D3DRS_LIGHTING, FALSE);
                m_pDx->SetRenderState(D3DRS_COLORVERTEX, FALSE);

                #ifdef WRAPPER_USE_FOG
                    m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
                    m_pDx->SetRenderState(D3DRS_FOGSTART, *((DWORD*) (&c_fFogStart)));
                    m_pDx->SetRenderState(D3DRS_FOGEND, *((DWORD*) (&c_fFogEnd)));  
                #endif

                for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
                    m_vAllTrees[i]->RenderBillboards( );

                #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
                    m_pDx->SetRenderState(D3DRS_LIGHTING, TRUE);
                #else
                    m_pDx->SetRenderState(D3DRS_COLORVERTEX, TRUE);
                #endif
            }
        #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX8::InitVertexShaders

bool CSpeedTreeForestDirectX8::InitVertexShaders(void)
{
    // load the vertex shaders
    m_dwBranchVertexShader = LoadBranchShader(m_pDx);
    m_dwLeafVertexShader = LoadLeafShader(m_pDx);

    return (m_dwBranchVertexShader && m_dwLeafVertexShader);
}

