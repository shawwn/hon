///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRT DirectX Example
//
//  (c) 2003 IDV, Inc.
//
//  This example demonstrates how to render trees using SpeedTreeRT
//  and DirectX.  Techniques illustrated include ".spt" file parsing,
//  static lighting, dynamic lighting, LOD implementation, cloning,
//  instancing, and dynamic wind effects.
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

#pragma once


///////////////////////////////////////////////////////////////////////  
//  Include Files

#include "SpeedTreeForestDirectX8.h"
#include "Navigation.h"
#include <vector>


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication
//
//  Application class. The base class (CD3DApplication) provides the 
//  generic functionality needed in all Direct3D samples. CMyD3DApplication 
//  adds functionality specific to this sample program.

class CMyD3DApplication : public CD3DApplication
{
public:
                                            CMyD3DApplication( );

        LRESULT                             MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
        HRESULT                             OneTimeSceneInit( );
        HRESULT                             InitDeviceObjects( );
        HRESULT                             RestoreDeviceObjects( );
        HRESULT                             InvalidateDeviceObjects( );
        HRESULT                             DeleteDeviceObjects( );
        HRESULT                             Render( );
        HRESULT                             FrameMove( );
        HRESULT                             FinalCleanup( );
        HRESULT                             ConfirmDevice(D3DCAPS8*, DWORD, D3DFORMAT);

private:
        void                                InitSpeedTreeLighting( );
        void                                PrintKeyCommands( );

private:
        float                               m_fAccumTime;               // the global time
        D3DLIGHT8                           m_sLight;                   // the scene's light object
        CNavigation                         m_cNavigation;              // the navigation class (handles movement)
        CSpeedTreeForestDirectX8*           m_pForest;                  // the SpeedTreeForest class, handles trees
        unsigned long                       m_ulRenderBitVector;        // what to render
        BOOL                                m_bLoadingApp;              // know when we are still loading
};
