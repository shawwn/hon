///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9 Class
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


///////////////////////////////////////////////////////////////////////  
//  includes

#include "SpeedTreeForestDirectX9.h"
#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include "D3DUtil.h"
#include "DXUtil.h"
#include "VertexShaders.h"


///////////////////////////////////////////////////////////////////////  
//  static member variables

LPDIRECT3DDEVICE9 CSpeedTreeForestDirectX9::m_pDx = NULL;
LPD3DXEFFECT CSpeedTreeForestDirectX9::m_pEffect = NULL;
float CSpeedTreeForestDirectX9::m_afCameraPos[3];
float CSpeedTreeForestDirectX9::m_afCameraDir[3];



///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::CSpeedTreeForestDirectX9

CSpeedTreeForestDirectX9::CSpeedTreeForestDirectX9( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9

CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9( )
{
    // release our texture and shaders
    SAFE_RELEASE(m_texCompositeMap);
    SAFE_RELEASE(m_pEffect);
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::~CSpeedTreeForestDirectX9

void CSpeedTreeForestDirectX9::SetCamera(const float* pCameraPos, const float* pCameraDir)
{
	if (pCameraPos != NULL)
		memcpy(m_afCameraPos, pCameraPos, 3 * sizeof(float));
	if (pCameraDir != NULL)
		memcpy(m_afCameraDir, pCameraDir, 3 * sizeof(float));
}
                                   

///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::UploadWindMatrix

void CSpeedTreeForestDirectX9::UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const
{
    // set wind matrices to vertex shader
    VERIFY(m_pEffect->SetMatrix(m_pEffect->GetParameterElement("g_amWindMatrices", uiLocation),
            reinterpret_cast<const D3DXMATRIX*>(pMatrix)));
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::UploadLeafRockMatrix

void CSpeedTreeForestDirectX9::UploadLeafRockMatrix(unsigned int uiLocation, const float* pMatrix) const
{
    // set wind matrices to vertex shader
    VERIFY(m_pEffect->SetMatrix(m_pEffect->GetParameterElement("g_amLeafAngleMatrices", uiLocation),
            reinterpret_cast<const D3DXMATRIX*>(pMatrix)));
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::InitGraphics

bool CSpeedTreeForestDirectX9::InitGraphics(void)
{
    bool bSuccess = false;

    // load the composite map
    if (D3DUtil_CreateTexture(m_pDx, c_pCompositeMapFilename, &m_texCompositeMap) != S_OK)
        MessageBox(NULL, "Failed to create leaf texture.", "File Error", MB_ICONEXCLAMATION);
    else
    {
        // init the vertex shaders, if needed
        bSuccess = InitShaders( );
        if (bSuccess)
            CSpeedTreeWrapper::SetEffect(m_pEffect);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::Render

void CSpeedTreeForestDirectX9::Render(unsigned long ulRenderBitVector) const
{
    for (int nLayer = 0; nLayer < 3; ++nLayer)
    {
        m_pDx->SetSamplerState(nLayer, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pDx->SetSamplerState(nLayer, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pDx->SetSamplerState(nLayer, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    // advance all trees' LOD and wind
    for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
        m_vAllTrees[i]->Advance( );

    if (!m_vAllTrees.empty( ))
    {
        // setup general graphics states
        m_pDx->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        m_pDx->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
        m_pDx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pDx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pDx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        m_pDx->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

        // set up fog, if enabled
        m_pDx->SetRenderState(D3DRS_FOGENABLE, TRUE);
        m_pDx->SetRenderState(D3DRS_FOGCOLOR, 0xFF334C66);
        m_pDx->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
        m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);

        unsigned int uiNumPasses = 0;

        // render branches
        if (ulRenderBitVector & Forest_RenderBranches)
        {
            bool bEffectActive = false;
            m_pDx->SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
            VERIFY(m_pEffect->SetTechnique("TBranch"));
            VERIFY(m_pEffect->Begin(&uiNumPasses, 0));
            VERIFY(m_pEffect->BeginPass(0));
            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupBranchForTreeType( );
                m_vAllTrees[i]->RenderBranches( );
            }
            VERIFY(m_pEffect->EndPass( ));
            VERIFY(m_pEffect->End( ));
        }

        // bind composite map & set render states
        VERIFY(m_pEffect->SetTexture("g_texCompositeLeafMap", m_texCompositeMap));
        m_pDx->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

        // render fronds
        if (ulRenderBitVector & Forest_RenderFronds)
        {
            m_pDx->SetFVF(D3DFVF_SPEEDTREE_FROND_VERTEX);
            VERIFY(m_pEffect->SetTechnique("TFrond"));
            VERIFY(m_pEffect->Begin(&uiNumPasses, 0));
            VERIFY(m_pEffect->BeginPass(0));
            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupFrondForTreeType( );
                m_vAllTrees[i]->RenderFronds( );
            }
            VERIFY(m_pEffect->EndPass( ));
            VERIFY(m_pEffect->End( ));
        }

        // render leaves
        if (ulRenderBitVector & Forest_RenderLeaves)
        {
            bool bEffectActive = false;
            m_pDx->SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
            VERIFY(m_pEffect->SetTechnique("TLeaf"));
            VERIFY(m_pEffect->Begin(&uiNumPasses, 0));
            VERIFY(m_pEffect->BeginPass(0));
            for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
            {
                if (!m_vAllTrees[i]->InstanceOf( ))
                    m_vAllTrees[i]->SetupLeafForTreeType( );
                m_vAllTrees[i]->RenderLeaves( );
            }
            VERIFY(m_pEffect->EndPass( ));
            VERIFY(m_pEffect->End( ));
        }

        #ifdef WRAPPER_BILLBOARD_MODE
			CSpeedTreeRT::SetCamera(m_afCameraPos, m_afCameraDir);
            if (ulRenderBitVector & Forest_RenderBillboards)
            {
                m_pDx->SetVertexShader(0);
                m_pDx->SetPixelShader(0);
                m_pDx->SetTexture(0, m_texCompositeMap);
                m_pDx->SetRenderState(D3DRS_LIGHTING, FALSE);
                m_pDx->SetRenderState(D3DRS_COLORVERTEX, FALSE);

                m_pDx->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
                m_pDx->SetRenderState(D3DRS_FOGSTART, *((DWORD*)(&c_fFogStart)));
                m_pDx->SetRenderState(D3DRS_FOGEND, *((DWORD*)(&c_fFogEnd)));   

                for (unsigned int i = 0; i < m_vAllTrees.size( ); ++i)
                    m_vAllTrees[i]->RenderBillboards( );

                m_pDx->SetRenderState(D3DRS_LIGHTING, TRUE);
            }
        #endif
    }
}


///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestDirectX9::InitShaders

bool CSpeedTreeForestDirectX9::InitShaders(void)
{
    if (!m_pEffect)
    {
        LPD3DXBUFFER pBuffer;
        if (SUCCEEDED(D3DXCreateEffectFromFile(m_pDx, "../../data/fx files/DirectX9_HLSL.fx", NULL, NULL, 0, NULL, &m_pEffect, &pBuffer)))
        {
        }
        else
        {
            SAFE_RELEASE(m_pEffect);
            LPVOID pCompileErrors = pBuffer->GetBufferPointer( );
            MessageBox(NULL, (const char*)pCompileErrors, "Failed to compile fx file", MB_OK);
        }
    }

    return true;
}
