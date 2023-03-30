///////////////////////////////////////////////////////////////////////  
//  CNavigation Class
//
//  (c) 2003 IDV, Inc.
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

#include <memory.h>
#include "Transform.h"


///////////////////////////////////////////////////////////////////////  
//  CNavigation
//
//  Geared mostly for terrain-style navigation, not object inspection

class CNavigation  
{
public: 
        enum EMouseButton
        {
            BUTTON_NONE, BUTTON_LEFT, BUTTON_MIDDLE, BUTTON_RIGHT, BUTTON_COUNT
        };

        CNavigation( );
        ~CNavigation( );

        float           Advance(float fFrameTime);
        float           GetAzimuth(void) const          { return 90.0f - m_fAzimuth; }
        const float*    GetCameraPos(void) const        { return m_afCameraPos; }
        const float*    GetCameraTransform(void);
        void            SetCameraPos(const float* pPos) { memcpy(m_afCameraPos, pPos, 3 * sizeof(float)); }
        void            MouseButton(EMouseButton eButton, bool bPressed, int nX, int nY);
        void            MouseMotion(int nX, int nY);
        void            SetSpeedScales(float fTranslate = -1.0f, float fRotate = -1.0f);

private:
        // orientation
        float           m_afCameraPos[4];
        float           m_fAzimuth;
        float           m_fPitch;
        CTransform      m_cTransform;

        // mouse mechanics
        EMouseButton    m_eMouseButton;
        bool            m_abMousePressed[BUTTON_COUNT];
        int             m_nBaseX;
        int             m_nBaseY;
        int             m_nDeltaX;
        int             m_nDeltaY;
        int             m_nLastX;
        int             m_nLastY;

        // speeds
        float           m_fTranslateSpeed;
        float           m_fRotateSpeed;
};
