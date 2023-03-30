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

///////////////////////////////////////////////////////////////////////  
//  Include Files

#include "Navigation.h"
#include <cmath>
#include <cstdio>


///////////////////////////////////////////////////////////////////////  
//  CNavigation::CNavigation
//
//  Utility function

inline float NavDeg2Rad(float fDeg)
{
    return fDeg / 57.295779513082320876798154814105f;
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::CNavigation

CNavigation::CNavigation( ) :
    m_fAzimuth(180.0f),
    m_fPitch(-90.0f),
    m_eMouseButton(BUTTON_NONE),
    m_nBaseX(0),
    m_nBaseY(0),
    m_nDeltaX(0),
    m_nDeltaY(0),
    m_nLastX(0),
    m_nLastY(0),
    m_fTranslateSpeed(0.08f),
    m_fRotateSpeed(0.15f)
{
    // mouse pressed state
    for (int nButton = BUTTON_NONE; nButton < BUTTON_COUNT; ++nButton)
        m_abMousePressed[nButton] = false;

    // arbitrary camera position
    m_afCameraPos[0] = 0.0f;
    m_afCameraPos[1] = 0.0f;
    m_afCameraPos[2] = 0.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::~CNavigation

CNavigation::~CNavigation( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::Advance
//
//  Returns feet / sec speed

float CNavigation::Advance(float fFrameTime)
{
    float fSpeed = 0.0f; // miles / hour
    if (m_eMouseButton == BUTTON_LEFT)
    {
        // camera continues to move in direction set in MouseMotion()
        float afLastCam[2] = { m_afCameraPos[0], m_afCameraPos[1] };
        m_afCameraPos[0] -= m_fTranslateSpeed * fFrameTime * m_nDeltaY * cosf(NavDeg2Rad(m_fAzimuth));
        m_afCameraPos[1] -= m_fTranslateSpeed * fFrameTime * m_nDeltaY * sinf(NavDeg2Rad(m_fAzimuth));

        float fDistance = sqrtf((afLastCam[0] - m_afCameraPos[0]) * (afLastCam[0] - m_afCameraPos[0]) +
                                (afLastCam[1] - m_afCameraPos[1]) * (afLastCam[1] - m_afCameraPos[1]));
        fSpeed = (fDistance / 5280.0f) / (fFrameTime / 3600.0f);
    }

    return fSpeed;
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::MouseButton

void CNavigation::MouseButton(EMouseButton eButton, bool bPressed, int nX, int nY)
{
    m_eMouseButton = eButton;
    m_abMousePressed[eButton] = bPressed;

    if (bPressed)
    {
        m_nDeltaX = 0;
        m_nDeltaY = 0;
        m_nBaseX = nX;
        m_nBaseY = nY;
    }

    m_nLastX = nX;
    m_nLastY = nY;
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::MouseMotion

void CNavigation::MouseMotion(int nX, int nY)
{
    if (m_abMousePressed[BUTTON_LEFT] && m_abMousePressed[BUTTON_MIDDLE])
    {
        const float c_fDeg2Rad = 57.2957795f;
        m_afCameraPos[0] -= (nX - m_nLastX) * cosf((m_fAzimuth + 90.0f) / c_fDeg2Rad);
        m_afCameraPos[1] -= (nX - m_nLastX) * sinf((m_fAzimuth + 90.0f) / c_fDeg2Rad);
    }
    else if (m_abMousePressed[BUTTON_MIDDLE])
    {
        m_fPitch += (nY - m_nLastY) * m_fRotateSpeed;
    }
    else if (m_abMousePressed[BUTTON_LEFT])
    {
        m_fAzimuth -= (nX - m_nLastX) * m_fRotateSpeed;
        m_nDeltaY = nY - m_nBaseY;
    }
    else if (m_abMousePressed[BUTTON_RIGHT])
    {
        m_afCameraPos[2] -= (nY - m_nLastY) * m_fTranslateSpeed;
    }

    m_nLastX = nX;
    m_nLastY = nY;
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::GetCameraTransform

const float* CNavigation::GetCameraTransform(void)
{
    m_cTransform.LoadIdentity( );

    m_cTransform.Rotate(NavDeg2Rad(m_fPitch), 'x');
    m_cTransform.Rotate(NavDeg2Rad(90.0f - m_fAzimuth), 'z');
    m_cTransform.Translate(-m_afCameraPos[0], -m_afCameraPos[1], -m_afCameraPos[2]);

    return reinterpret_cast<float*>(m_cTransform.m_afData);
}


///////////////////////////////////////////////////////////////////////  
//  CNavigation::SetSpeedScales

void CNavigation::SetSpeedScales(float fTranslate, float fRotate)
{
    if (fTranslate != -1.0f)
        m_fTranslateSpeed = fTranslate;

    if (fRotate != -1.0f)
        m_fRotateSpeed = fRotate;
}

