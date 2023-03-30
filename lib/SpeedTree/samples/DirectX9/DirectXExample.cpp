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

#pragma warning(disable:4723)


///////////////////////////////////////////////////////////////////////  
//  Includes

#define STRICT
#pragma warning (disable : 4786)
#include "DirectXExample.h"
#include "VertexShaders.h"
#include "Navigation.h"
#include "SpeedTreeForestDirectX9.h"
#include <D3DX9.h>
#include "D3DApp.h"
#include "D3DUtil.h"
#include "DXUtil.h"
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  Global Variables

CMyD3DApplication*                  g_pApp              = NULL;
HINSTANCE                           g_hInst             = NULL;


///////////////////////////////////////////////////////////////////////  
//  WinMain
//
// Entry point to the program. Initializes everything, and goes into a
// message-processing loop. Idle time is used to render the scene.

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    CMyD3DApplication d3dApp;

    g_pApp  = &d3dApp;
    g_hInst = hInst;

    if (FAILED(d3dApp.Create(hInst)))
        return 0;

    return d3dApp.Run( );
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::CMyD3DApplication

CMyD3DApplication::CMyD3DApplication( ) : 
    m_pForest(NULL)
{
    // create console
    AllocConsole( );                 // Allocate console window
    freopen("CONOUT$", "a", stderr); // Redirect stderr to console
    freopen("CONOUT$", "a", stdout); // Redirect stdout also
    freopen("CONIN$", "r", stdin);
    SetConsoleTitle("Interactive Data Visualization Console Window");

    // identify the program
    printf("\n\nSpeedTreeRT DirectX 9.0 Example v1.7.0 (c) 2004 IDV, Inc.\n\n");

    // print the configuration
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        printf("\t- Dynamic lighting enabled\n");
    #else
        printf("\t- Static lighting enabled\n");
    #endif
    #ifdef WRAPPER_USE_GPU_WIND
        printf("\t- GPU-based wind enabled\n");
    #endif
    #ifdef WRAPPER_USE_CPU_WIND
        printf("\t- CPU-based wind enabled\n");
    #endif
    #ifdef WRAPPER_USE_NO_WIND
        printf("\t- No wind enabled (leaves will rock, but not branches)\n");
    #endif
    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        printf("\t- GPU-based leaf placement enabled\n");
    #else
        printf("\t- CPU-based leaf placement enabled\n");
    #endif
    printf("\n");

    // print keyboard commands
    PrintKeyCommands( );

    // scene configuration
    m_dwCreationWidth = 1024;
    m_dwCreationHeight = 768;
    m_strWindowTitle = TEXT("SpeedTreeRT DirectX 9.0 Sample");
    m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
    m_bStartFullscreen          = false;
    m_bShowCursorWhenFullscreen = true;

    // we are in loading mode
    m_bLoadingApp = TRUE;

    // init global time
    m_fAccumTime = 0.0f;

    // render everything
    m_ulRenderBitVector = Forest_RenderAll;

    // make new forest
    m_pForest = new CSpeedTreeForestDirectX9;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::OneTimeSceneInit
//
// Called during initial app startup, this function performs all the
// permanent initialization.

HRESULT CMyD3DApplication::OneTimeSceneInit( )
{
    // Drawing loading status message until app finishes loading
    SendMessage(m_hWnd, WM_PAINT, 0, 0);

    // no longer in loading mode
    m_bLoadingApp = FALSE;

    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::ConfirmDevice
//
// Called during device initialization, this code checks the display device
// for some minimum set of capabilities.

HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT Format)
{
    UNREFERENCED_PARAMETER(Format);
    UNREFERENCED_PARAMETER(dwBehavior);
    UNREFERENCED_PARAMETER(pCaps);
    
    BOOL bCapsAcceptable;

    // TODO: Perform checks to see if these display caps are acceptable.
    bCapsAcceptable = TRUE;

    if( bCapsAcceptable )         
        return S_OK;
    else
        return E_FAIL;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::InitDeviceObjects
//
// Initialize scene objects.

HRESULT CMyD3DApplication::InitDeviceObjects( )
{
    bool bSuccess = false;

    // give out the redering context
    CSpeedTreeForestDirectX9::SetRenderingDevice(m_pd3dDevice);
    CSpeedTreeWrapper::SetRenderingDevice(m_pd3dDevice);

    // initialize lighting
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
        m_pd3dDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);

        D3DUtil_InitLight(m_sLight, D3DLIGHT_DIRECTIONAL, -c_afLightPosition[0], -c_afLightPosition[1], -c_afLightPosition[2]);
        m_sLight.Ambient.r = c_afLightAmbient[0];
        m_sLight.Ambient.g = c_afLightAmbient[1];
        m_sLight.Ambient.b = c_afLightAmbient[2];
        m_sLight.Ambient.a = c_afLightAmbient[3];
        m_sLight.Diffuse.r = c_afLightDiffuse[0];
        m_sLight.Diffuse.g = c_afLightDiffuse[1];
        m_sLight.Diffuse.b = c_afLightDiffuse[2];
        m_sLight.Diffuse.a = c_afLightDiffuse[3];
        m_sLight.Specular.r = c_afLightSpecular[0];
        m_sLight.Specular.g = c_afLightSpecular[1];
        m_sLight.Specular.b = c_afLightSpecular[2];
        m_sLight.Specular.a = c_afLightSpecular[3];
        m_pd3dDevice->SetLight(0, &m_sLight);
        m_pd3dDevice->LightEnable(0, TRUE);
    #else
        InitSpeedTreeLighting( );
        m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
        m_pd3dDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE);
    #endif

    // load the tree
    #ifdef WRAPPER_FOREST_FROM_STF
        if (m_pForest->Load(c_pStfFilename))
    #else
        if (m_pForest->Load(NULL))
    #endif
    {
        // the camera starts in the center of the scene
        const float* pExtents = m_pForest->GetExtents( );
        float afCenter[3] =
        {
            0.5f * (pExtents[0] + pExtents[3]),
            0.5f * (pExtents[1] + pExtents[4]),
            0.5f * (pExtents[2] + pExtents[5])
        };
        m_cNavigation.SetCameraPos(afCenter);

        // reasonable navigation speed based on scene size
        float fLongestExtent = max(pExtents[3] - pExtents[0], pExtents[4] - pExtents[1]);
        m_cNavigation.SetSpeedScales(fLongestExtent / 1000.0f);

        // setup near/far clip values
        c_fFarClipPlane = fLongestExtent * 3.0f;

        bSuccess = true;
    }

    // clean up no longer needed memory
    if (!bSuccess)
        SendMessage(m_hWnd, WM_CLOSE, 0, 0);

    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::RestoreDeviceObjects
//
// Restores scene objects.

HRESULT CMyD3DApplication::RestoreDeviceObjects( )
{
    // set up states for texture level 1 (shadow) if enabled
    #ifdef WRAPPER_RENDER_SELF_SHADOWS
    if (m_pd3dDevice != NULL)
    {
        m_pd3dDevice->SetRenderState(D3DRS_WRAP1, D3DWRAP_U | D3DWRAP_V);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        m_pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }
    #endif
    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::FrameMove
//
// Called once per frame, the call is the entry point for animating
// the scene.

HRESULT CMyD3DApplication::FrameMove( )
{
    static float fLastFrameTime = 0.0f;
    static int nFrameCount = 0;
    const int c_nFrameInterval = 30;

    // compute time
    m_fAccumTime += m_fElapsedTime;

    // advance navigation
    m_cNavigation.Advance(m_fElapsedTime);

    // advance wind
    CSpeedTreeRT::SetTime(m_fAccumTime);
    m_pForest->SetupWindMatrices(m_fAccumTime);

    // report frame rate
    ++nFrameCount;
    if (!(nFrameCount % c_nFrameInterval))
    {
        static float fLastReportTime = m_fAccumTime;
        float fTimeSinceLastReport = m_fAccumTime - fLastReportTime;

        if (fTimeSinceLastReport > 0.0f)
        {
            float fRate = c_nFrameInterval / fTimeSinceLastReport;
            fLastReportTime = m_fAccumTime;
            printf("\t%.1f Hz\n", fRate);
        }
    }
    fLastFrameTime = m_fAccumTime;

    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::Render
//
// Called once per frame, the call is the entry point for 3d
// rendering. This function sets up render states, clears the
// viewport, and renders the scene.

HRESULT CMyD3DApplication::Render( )
{
    // clear the viewport
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF334C66, 1.0f, 0L);

    // begin the scene
    if (SUCCEEDED(m_pd3dDevice->BeginScene( )))
    {
        // set up view
        static D3DXMATRIX matProjection;
        D3DXMatrixPerspectiveFovRH(&matProjection, D3DXToRadian(40.f), (FLOAT)m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height, c_fNearClipPlane, c_fFarClipPlane);
        m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
        m_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)(m_cNavigation.GetCameraTransform( )));
        
        // setup composite matrix for shader
        D3DXMATRIX matBlendShader;
        D3DXMatrixMultiply(&matBlendShader,  (D3DXMATRIX*)(m_cNavigation.GetCameraTransform( )), &matProjection);
                
        // get camera direction from modelview matrix
        float afDirection[3];
        afDirection[0] = matBlendShader.m[0][2];
        afDirection[1] = matBlendShader.m[1][2];
        afDirection[2] = matBlendShader.m[2][2];
        CSpeedTreeRT::SetCamera(m_cNavigation.GetCameraPos( ), afDirection);

        // send matrix to shader
        D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
        m_pd3dDevice->SetVertexShaderConstantF(c_nVertexShader_CompoundMatrix, (float*)&matBlendShader, 4);

        // setup shader constants for light
        const float afLighting[] =
        {
            c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2], 1.0f,                     // [71] normalized light direction
            c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2], c_afLightAmbient[3],         // [72] light ambient color
            c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2], c_afLightDiffuse[3]          // [73] light diffuse color
        };
        m_pd3dDevice->SetVertexShaderConstantF(c_nVertexShader_Light, afLighting, 3);

        // send fog data to shader if enabled
        #ifdef WRAPPER_USE_FOG
            D3DXVECTOR4 vecConstant(c_fFogStart, c_fFogEnd, c_fFogLinearScale, 0.0f);
            m_pd3dDevice->SetVertexShaderConstantF(c_nVertexShader_Fog, (float*)&vecConstant, 1);
        #endif

        // render the forest
        m_pForest->Render(m_ulRenderBitVector);

        // end the scene
        m_pd3dDevice->EndScene( );
    }

    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::MsgProc
//
// Overrrides the main WndProc, so the sample can do custom message
// handling (e.g. processing mouse, keyboard, or menu commands).

LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    unsigned int nX = 0, nY = 0;
    CNavigation::EMouseButton eNavButton = CNavigation::BUTTON_NONE;

    // handle keyboard and mouse events
    switch(msg)
    {
    case WM_LBUTTONDOWN:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_LEFT;
        m_cNavigation.MouseButton(eNavButton, true, nX, nY);
        SetCapture(m_hWnd);
        break;
    case WM_LBUTTONUP:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_LEFT;
        m_cNavigation.MouseButton(eNavButton, false, nX, nY);
        ReleaseCapture( );
        break;
    case WM_RBUTTONDOWN:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_RIGHT;
        m_cNavigation.MouseButton(eNavButton, true, nX, nY);
        SetCapture(m_hWnd);
        break;
    case WM_RBUTTONUP:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_RIGHT;
        m_cNavigation.MouseButton(eNavButton, false, nX, nY);
        ReleaseCapture( );
        return true; // override the context menu
        break;
    case WM_MBUTTONDOWN:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_MIDDLE;
        m_cNavigation.MouseButton(eNavButton, true, nX, nY);
        SetCapture(m_hWnd);
        break;
    case WM_MBUTTONUP:
        nX = (int)LOWORD(lParam);
        nY = (int)HIWORD(lParam);
        eNavButton = CNavigation::BUTTON_MIDDLE;
        m_cNavigation.MouseButton(eNavButton, false, nX, nY);
        ReleaseCapture( );
        break;
    case WM_MOUSEMOVE:
        nX = (int)LOWORD(lParam);
        if (nX > 3000)
            nX = nX - 65535;
        nY = (int)HIWORD(lParam);
        if (nY > 3000)
            nY = nY - 65535;
        m_cNavigation.MouseMotion(nX, nY);
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'W': // adjust wind strength
            {
                bool bCaps = ( (GetKeyState(VK_CAPITAL) > 0) && !(GetKeyState(VK_SHIFT) < 0)) ||  
                             (!(GetKeyState(VK_CAPITAL) > 0) &&  (GetKeyState(VK_SHIFT) < 0));
                if (!bCaps)
                {
                    float fStrength = m_pForest->GetWindStrength( );
                    fStrength = max(0.0f, fStrength - 0.01f);
                    m_pForest->SetWindStrength(fStrength);
                }
                else
                {
                    float fStrength = m_pForest->GetWindStrength( );
                    fStrength = min(1.0f, fStrength + 0.01f);
                    m_pForest->SetWindStrength(fStrength);
                }
            }
            break;
        case 'L': // toggle leaf visibility
            m_ulRenderBitVector ^= Forest_RenderLeaves;
            break;
        case 'F': // toggle frond visibility
            m_ulRenderBitVector ^= Forest_RenderFronds;
            break;
        case 'B': // toggle branch visibility       
            m_ulRenderBitVector ^= Forest_RenderBranches;
            break;
        case 'Z':
            m_ulRenderBitVector ^= Forest_RenderBillboards;
            break;
        case 'X': // toggle fullscreen  
            ToggleFullscreen( );
            break;
        case 'S':   // toggle shadows
            break;
        }
        break;
    case WM_PAINT:
        if (m_bLoadingApp)
        {
            // Draw on the window tell the user that the app is loading
            HDC hDC = GetDC(hWnd);
            TCHAR strMsg[MAX_PATH];
            wsprintf(strMsg, TEXT("Loading... Please wait"));
            RECT rct;
            GetClientRect(hWnd, &rct);
            DrawText(hDC, strMsg, -1, &rct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            ReleaseDC(hWnd, hDC);
        }
        break;
    }

    return CD3DApplication::MsgProc(hWnd, msg, wParam, lParam);
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::InvalidateDeviceObjects
//
// Invalidates device objects.  

HRESULT CMyD3DApplication::InvalidateDeviceObjects( )
{
    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::DeleteDeviceObjects
//
// Called when the app is exiting, or the device is being changed,
// this function deletes any device dependent objects.  

HRESULT CMyD3DApplication::DeleteDeviceObjects( )
{
    // clean up forest
    SAFE_DELETE(m_pForest);
    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::FinalCleanup
//
// Called before the app exits, this function gives the app the chance
// to cleanup after itself.

HRESULT CMyD3DApplication::FinalCleanup( )
{
    return S_OK;
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::InitSpeedTreeLighting

void CMyD3DApplication::InitSpeedTreeLighting()
{
    float afLight1[ ] =
    {
        c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2],   // pos
        c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2],      // diffuse
        c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2],      // ambient
        c_afLightSpecular[0], c_afLightSpecular[1], c_afLightSpecular[2],   // specular
        c_afLightPosition[3],                                               // directional flag
        1.0f, 0.0f, 0.0f                                                    // attenuation (constant, linear, quadratic)
    };

    // set light attributes and enable static light 0
    CSpeedTreeRT::SetLightAttributes(0, afLight1);  
    CSpeedTreeRT::SetLightState(0, true);
}


///////////////////////////////////////////////////////////////////////  
//  CMyD3DApplication::PrintKeyCommands
//

void CMyD3DApplication::PrintKeyCommands( )
{
    printf("\n\nKey Commands\n====================================\n");
    printf("L or l\tToggle leaves\n");
    printf("B or b\tToggle branches\n");
    printf("F or f\tToggle fronds\n");
    printf("Z or z\tToggle billboards\n");
    printf("W     \tIncrease wind strength\n");
    printf("w     \tDecrease wind strength\n");
    printf("x or X\tToggle full screen mode\n");
    printf("Esc   \tQuit the application\n");
    printf("\n\n");
}
