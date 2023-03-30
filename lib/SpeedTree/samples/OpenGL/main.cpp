///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRT OpenGL Example
//
//  (c) 2003 IDV, Inc.
//
//  This example demonstrates how to render trees using SpeedTreeRT
//  and OpenGL.  Techniques illustrated include ".spt" file parsing,
//  static lighting, dynamic lighting, and LOD implementation,
//  instances, and clones.
//
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

#ifdef _lint
#define lint
#endif
//lint -e14
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <vector>
#include "SpeedTreeForestOpenGL.h"
#include "SpeedTreeWrapper.h"
#include "nv_dds.h"
#include "VertexProgram.h"
#include "Navigation.h"
using namespace std;

///////////////////////////////////////////////////////////////////////  
//  Prototypes

        int     main(int argc, char** argv);
        bool    InitWindow(void);
        void    Display(void);
        void    Idle(void);
        void    Visible(int nVisible);
        void    Key(unsigned char chKey, int nX, int nY);
        void    MouseClick(int nButton, int nState, int nX, int nY);
        void    MouseMotion(int nX, int nY);
        void    Reshape(int nWidth, int nHeight);
        void    InitSpeedTreeLighting(void);
        void    SetupWindMatrices(void);
        void    PrintKeyCommands(void);
        void    ExitApp(void);


///////////////////////////////////////////////////////////////////////  
//  Variables

// window
static  int                     g_nWindow = 0;
static  float                   g_fAspectRatio = 4.0f / 3.0f;
static  float                   g_fNearClipPlane = 1.0f;
static  float                   g_fFarClipPlane = 1.0f;
                        
// wind                 
static  float                   g_fTime = 0.0f;
static  float                   g_fWindStrength = 0.3f;

// new from version 1.5.1
static  CSpeedTreeForestOpenGL* g_pForest = NULL;
static  unsigned long           g_ulRenderBitVector = Forest_RenderAll;
static  CNavigation*            g_pNavigation = NULL;


///////////////////////////////////////////////////////////////////////  
//  main

int main(int argc, char **argv)
{
    // identify the program
    printf("\n\nSpeedTreeRT OpenGL Example v1.7.0 (c) 2004 IDV, Inc.\n\n");

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
    #ifdef WRAPPER_FOREST_FROM_STF
        printf("\t- Trees will be placed from an STF file\n");
    #else
        printf("\t- Trees will be placed procedurally\n");
    #endif
    #ifdef WRAPPER_RENDER_IMMEDIATE_MODE
        printf("\t- Rendering in immediate mode\n");
    #else
        printf("\t- Rendering with vertex buffer objects\n");
    #endif

    printf("\n");

    PrintKeyCommands( );

    glutInit(&argc, argv);
    glutInitDisplayString("rgb double depth>=16");

    // full screen
//  glutGameModeString("1024x768:32@60");
//  glutEnterGameMode( );

    // windowed
    glutInitWindowSize(1024, 768);
    (void) glutCreateWindow("SpeedTreeRT OpenGL Example v1.7.0");

    if (InitWindow( ))
        glutMainLoop( );

    return 0;
}


///////////////////////////////////////////////////////////////////////  
//  InitWindow

bool InitWindow(void)
{
    bool bSuccess = false;

    // initialize OpenGL extensions (for DDS files, compression, etc)
    (void) extgl_Initialize( );

    // glut setup
    glutDisplayFunc(Display);
    glutVisibilityFunc(Visible);
    glutKeyboardFunc(Key);
    glutMouseFunc(MouseClick);
    glutMotionFunc(MouseMotion);
    glutReshapeFunc(Reshape);
    g_nWindow = glutGetWindow();

    // OpenGL setup
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glAlphaFunc(GL_GREATER, 0.33f);
    glEnable(GL_ALPHA_TEST);

    #ifdef WRAPPER_USE_FOG
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        float afFogColor[] = { 0.2f, 0.3f, 0.4f, 1.0f };
        glFogfv(GL_FOG_COLOR, afFogColor);      
        glHint(GL_FOG_HINT, GL_DONT_CARE);      
        glFogf(GL_FOG_START, c_fFogStart);              
        glFogf(GL_FOG_END, c_fFogEnd);              
    #endif

    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        glEnable(GL_LIGHTING);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c_afLightGlobalAmbient);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_AMBIENT, c_afLightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, c_afLightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, c_afLightSpecular);
    #else // static lighting
        glDisable(GL_LIGHTING);
        // lights must be initialized prior to tree loading
        InitSpeedTreeLighting( ); 
    #endif

    // load forest of trees
    g_pForest = new CSpeedTreeForestOpenGL;
#ifdef WRAPPER_FOREST_FROM_STF
    if (g_pForest->Load(c_pStfFilename))
#else
    if (g_pForest->Load(NULL))
#endif
    {
        // the camera starts in the center of the scene
        const float* pExtents = g_pForest->GetExtents( );
        float afCenter[3] =
        {
            0.5f * (pExtents[0] + pExtents[3]),
            0.5f * (pExtents[1] + pExtents[4]),
            0.5f * (pExtents[2] + pExtents[5])
        };
        g_pNavigation = new CNavigation;
        g_pNavigation->SetCameraPos(afCenter);

        // reasonable navigation speed based on scene size
        float fLongestExtent = __max(pExtents[3] - pExtents[0], pExtents[4] - pExtents[1]);
        g_pNavigation->SetSpeedScales(fLongestExtent / 1000.0f);

        // setup near/far clip values
        g_fFarClipPlane = fLongestExtent * 20.0f;

        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  Display

void Display(void)
{
    // scene setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity( );
    gluPerspective(40.0f, g_fAspectRatio, g_fNearClipPlane, g_fFarClipPlane);

    glMatrixMode(GL_MODELVIEW);
    const float* pCameraMatrix = g_pNavigation->GetCameraTransform( );
    glLoadMatrixf(pCameraMatrix);

    // send SpeedTree's camera pos and direction
    float afCameraDir[3] = { pCameraMatrix[2], pCameraMatrix[6], pCameraMatrix[10] };
    CSpeedTreeRT::SetCamera(g_pNavigation->GetCameraPos( ), afCameraDir);

    // light setup
    #ifdef WRAPPER_USE_DYNAMIC_LIGHTING
        glLightfv(GL_LIGHT0, GL_POSITION, c_afLightPosition);
        glEnable(GL_LIGHTING);
    #endif

    g_pForest->Render(g_ulRenderBitVector);

    // show extents of forest
    const float* pExtents = g_pForest->GetExtents( );
    glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
            glVertex3f(pExtents[0], pExtents[1], pExtents[2]);
            glVertex3f(pExtents[3], pExtents[1], pExtents[2]);
            glVertex3f(pExtents[3], pExtents[4], pExtents[2]);
            glVertex3f(pExtents[0], pExtents[4], pExtents[2]);
        glEnd( );

    glPopAttrib( );

    glutSwapBuffers( );
}


///////////////////////////////////////////////////////////////////////  
//  Idle

void Idle(void)
{
    static float fLastFrameTime = 0.0f;
    static int nFrameCount = 0;
    const int c_nFrameInterval = 30;

    float fTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // advance navigation
    (void) g_pNavigation->Advance(fTime - fLastFrameTime);

    // advance wind
    CSpeedTreeRT::SetTime(fTime);
    g_pForest->SetupWindMatrices(fTime);

    // report frame rate
    ++nFrameCount;
    if (!(nFrameCount % c_nFrameInterval))
    {
        static float fLastReportTime = fTime;
        float fTimeSinceLastReport = fTime - fLastReportTime;

        if (fTimeSinceLastReport > 0.0f)
        {
            float fRate = c_nFrameInterval / fTimeSinceLastReport;
            fLastReportTime = fTime;
            printf("\t%.1f Hz\n", fRate);
        }
    }

    fLastFrameTime = fTime;

    glutPostWindowRedisplay(g_nWindow);
}


///////////////////////////////////////////////////////////////////////  
//  Visible

void Visible(int nVisible)
{
    if (nVisible == GLUT_VISIBLE)
        glutIdleFunc(Idle);
    else
        glutIdleFunc(NULL);
}


///////////////////////////////////////////////////////////////////////  
//  Key

void Key(unsigned char chKey, int nX, int nY)
{
    UNREFERENCED_PARAMETER(nX);
    UNREFERENCED_PARAMETER(nY);

    switch (chKey)
    {
    case 27: // quit
        ExitApp( );
        break;
    case 'b': case 'B': // toggle branches
        g_ulRenderBitVector ^= Forest_RenderBranches;
        break;
    case 'l': case 'L': // toggle leaves
        g_ulRenderBitVector ^= Forest_RenderLeaves;
        break;
    case 'f': case 'F': // toggle fronds
        g_ulRenderBitVector ^= Forest_RenderFronds;
        break;
    case 'z': case 'Z': // toggle billboards
        g_ulRenderBitVector ^= Forest_RenderBillboards;
        break;
    case 'w': // decrease wind strength
        {
            float fStrength = g_pForest->GetWindStrength( );
            fStrength = max(0.0f, fStrength - 0.01f);
            g_pForest->SetWindStrength(fStrength);
        }
        break;
    case 'W': // increase wind strength
        {
            float fStrength = g_pForest->GetWindStrength( );
            fStrength = min(1.0f, fStrength + 0.01f);
            g_pForest->SetWindStrength(fStrength);
        }
        break;
    }
}


///////////////////////////////////////////////////////////////////////  
//  MouseClick

void MouseClick(int nButton, int nState, int nX, int nY)
{
    CNavigation::EMouseButton eNavButton = CNavigation::BUTTON_NONE;
    if (nButton == 0)
        eNavButton = CNavigation::BUTTON_LEFT;
    else if (nButton == 1)
        eNavButton = CNavigation::BUTTON_MIDDLE;
    else if (nButton == 2)
        eNavButton = CNavigation::BUTTON_RIGHT;

    g_pNavigation->MouseButton(eNavButton, !nState, nX, nY);
}


///////////////////////////////////////////////////////////////////////  
//  MouseMotion

void MouseMotion(int nX, int nY)
{
    g_pNavigation->MouseMotion(nX, nY);
}


///////////////////////////////////////////////////////////////////////  
//  Reshape

void Reshape(int nWidth, int nHeight)
{
    glViewport(0, 0, nWidth, nHeight);
    g_fAspectRatio = nWidth / static_cast<float>(nHeight);
}


///////////////////////////////////////////////////////////////////////  
//  InitSpeedTreeLighting
//
//  Light configuration applies to all subsequently loaded trees.

void InitSpeedTreeLighting( )
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
//  PrintKeyCommands

void PrintKeyCommands( )
{
    printf("\n\nKey Commands\n====================================\n");
    printf("L or l\tToggle leaves\n");
    printf("B or b\tToggle branches\n");
    printf("Esc   \tQuit the application\n");
    printf("\n\n");
}


///////////////////////////////////////////////////////////////////////  
//  ExitApp

void ExitApp( )
{
    delete g_pForest;
    g_pForest = NULL;

    delete g_pNavigation;
    g_pNavigation = NULL;

    exit(0);
}

