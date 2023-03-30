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

#pragma once


///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <map>
#include "SpeedTreeForest.h"
#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeForestDirectX9 declaration

class CSpeedTreeForestDirectX9 : public CSpeedTreeForest
{
public:
                                    CSpeedTreeForestDirectX9( );
                                    ~CSpeedTreeForestDirectX9( );
                                    
        void                        UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const;
        bool                        InitGraphics(void);
        void                        Render(unsigned long ulRenderBitVector) const;
static  void                        SetRenderingDevice(LPDIRECT3DDEVICE9 pDevice) { m_pDx = pDevice; }
                                    
private:                                
        bool                        InitVertexShaders( );

private:
static  LPDIRECT3DDEVICE9           m_pDx;                              // rendering context
        LPDIRECT3DTEXTURE9          m_texCompositeMap;                  // composite texture

        LPDIRECT3DVERTEXSHADER9     m_dwBranchVertexShader;             // branch vertex shader
        LPDIRECT3DVERTEXSHADER9     m_dwLeafVertexShader;               // leaf vertex shader
};