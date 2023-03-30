///////////////////////////////////////////////////////////////////////  
//         Name: WindEngine.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.


///////////////////////////////////////////////////////////////////////  
// Preprocessor

#include "Debug.h"
#include "SpeedTreeRT.h"
#include "WindEngine.h"
#include "UpVector.h"


///////////////////////////////////////////////////////////////////////  
// Static member variables

float CWindEngine::m_fTime = 0.0f;
CWindMatrices CWindEngine::m_cWindMatrices;


///////////////////////////////////////////////////////////////////////  
// Constants

// In an effort to keep all of the leaves from facing the same direction,
// small rotations are added to the billboarding matrix in an order to 
// fight depth buffer conflict.  This table contains a series of values
// used as the rotation angles (in degrees)

static  const   int     c_nLeafOffsetCount = 30;
static  const   float   c_fLeafOffsetScalar = 1.0f;
static  const   float   c_afLeafOffsets[c_nLeafOffsetCount] =
{
    c_fLeafOffsetScalar * 6.0f,     c_fLeafOffsetScalar * -10.0f,   c_fLeafOffsetScalar * 16.0f,
    c_fLeafOffsetScalar * -8.0f,    c_fLeafOffsetScalar * 2.0f,     c_fLeafOffsetScalar * -18.0f,
    c_fLeafOffsetScalar * -14.0f,   c_fLeafOffsetScalar * 0.0f,     c_fLeafOffsetScalar * -4.0f,
    c_fLeafOffsetScalar * 10.0f,    c_fLeafOffsetScalar * -6.0f,    c_fLeafOffsetScalar * 8.0f,
    c_fLeafOffsetScalar * 4.0f,     c_fLeafOffsetScalar * -2.0f,    c_fLeafOffsetScalar * 14.0f,
    c_fLeafOffsetScalar * -12.0f,   c_fLeafOffsetScalar * 14.0f,    c_fLeafOffsetScalar * -16.0f,
    c_fLeafOffsetScalar * 4.0f,     c_fLeafOffsetScalar * -18.0f,   c_fLeafOffsetScalar * 13.0f,
    c_fLeafOffsetScalar * 9.0f,     c_fLeafOffsetScalar * 17.0f,    c_fLeafOffsetScalar * -9.0f,
    c_fLeafOffsetScalar * 3.0f,     c_fLeafOffsetScalar * -7.0f,    c_fLeafOffsetScalar * -15.0f,
    c_fLeafOffsetScalar * 7.0f,     c_fLeafOffsetScalar * -5.0f,    c_fLeafOffsetScalar * 15.0f
};

///////////////////////////////////////////////////////////////////////  
//  CWindEngine::CWindEngine definition

CWindEngine::CWindEngine( ) :
    m_bRockingLeaves(true),
    m_eBranchMethod(CSpeedTreeRT::WIND_NONE),
    m_eFrondMethod(CSpeedTreeRT::WIND_NONE),
    m_eLeafMethod(CSpeedTreeRT::WIND_NONE),
    m_fLeafFrequency(0.0f),
    m_fTimeFrequencyShift(0.0f),
    m_fLeafThrow(0.0f),
    m_fWindStrength(0.25f),
    m_nStartingMatrix(0),
    m_nMatrixSpan(4),
    m_uiNumLeafAngles(0),
    m_pRockingAngles(NULL),
    m_pRustleAngles(NULL),
    m_fSpeedWindRockScalar(1.0f),
    m_fSpeedWindRustleScalar(1.0f)
{
    m_fLeafFactors[0] = m_fLeafFactors[1] = 1.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::SetWindStrength definition
//
//  Returns the newly computed frequency time shift

float CWindEngine::SetWindStrength(float fNewStrength, float fOldStrength, float fOldTimeShift)
{
    float fPreviousLeafFrequency = c_fStrengthToFrequencyRatio * fOldStrength * m_fLeafFactors[1];

    m_fWindStrength = fNewStrength;
    m_fLeafFrequency = c_fStrengthToFrequencyRatio * m_fWindStrength * m_fLeafFactors[1];
    m_fLeafThrow = c_fStrengthToThrowRatio * m_fWindStrength * m_fLeafFactors[0];

    m_fTimeFrequencyShift = m_fTime * fPreviousLeafFrequency + fOldTimeShift - m_fTime * m_fLeafFrequency;

    m_pRockingAngles = NULL;
    m_uiNumLeafAngles = 0;

    return m_fTimeFrequencyShift;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::SetWindStrengthAndLeafAngles definition

void CWindEngine::SetWindStrengthAndLeafAngles(float fNewStrength, const float* pRockingAngles, const float* pRustleAngles, unsigned int uiNumLeafAngles)
{
    m_fWindStrength = fNewStrength;

    m_uiNumLeafAngles = uiNumLeafAngles;
    m_pRockingAngles = pRockingAngles;
    m_pRustleAngles = pRustleAngles;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::Init definition

void CWindEngine::Init(const SIdvWindInfo& cWindInfo)
{
    m_fLeafFactors[0] = cWindInfo.m_cLeafFactors[0];
    m_fLeafFactors[1] = cWindInfo.m_cLeafFactors[1];
    m_fLeafFrequency = c_fStrengthToFrequencyRatio * cWindInfo.m_fStrength * m_fLeafFactors[1];
    m_fLeafThrow = c_fStrengthToThrowRatio * cWindInfo.m_fStrength * m_fLeafFactors[0];
    m_fWindStrength = cWindInfo.m_fStrength;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::SetLocalMatrices definition

void CWindEngine::SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan)
{
    m_nStartingMatrix = nStartingMatrix;
    m_nMatrixSpan = nMatrixSpan;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::ResetLeafWindState definition

void CWindEngine::ResetLeafWindState( )
{
    m_fLeafFrequency = 0.0f;
    m_fTimeFrequencyShift = 0.0f;
    m_fLeafThrow = 0.0f;
    m_fWindStrength = 0.25f;
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::RockLeaf definition

void CWindEngine::RockLeaf(float fAzimuth, float fPitch, int nIndex, float fTimeOffset, CRotTransform& cTrans) const
{
    float fWindAngle = 0.0f;

    if (m_bRockingLeaves)
    {
        float fPercent = 0.5f * (sinf(fTimeOffset + (m_fTime * m_fLeafFrequency) + m_fTimeFrequencyShift) + 1.0f);
        fWindAngle = VecInterpolate(-m_fLeafThrow, m_fLeafThrow, fPercent);
    }
    else
    {
        float fPercent = (sinf(fTimeOffset) + 1.0f) * 0.5f; 
        fWindAngle = VecInterpolate(m_fLeafFactors[0] * -0.15f, m_fLeafFactors[0] * 0.15f, fPercent);
    }

    // setup the billboarding/rocking leaf transform
#ifdef UPVECTOR_POS_Z
    cTrans.RotateZ(fAzimuth + c_afLeafOffsets[nIndex]);
    cTrans.RotateY(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateX(fWindAngle);
#endif
#ifdef UPVECTOR_NEG_Z
    cTrans.RotateZ(-(fAzimuth + c_afLeafOffsets[nIndex]));
    cTrans.RotateY(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateX(fWindAngle);
#endif
#ifdef UPVECTOR_POS_Y
    cTrans.RotateY(fAzimuth + c_afLeafOffsets[nIndex]);
    cTrans.RotateZ(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateX(-fWindAngle);
#endif
#ifdef UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
    cTrans.RotateZ(-(fAzimuth + c_afLeafOffsets[nIndex]));
    cTrans.RotateX(-(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]));
    cTrans.RotateY(fWindAngle);
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CWindEngine::RockLeaf definition

void CWindEngine::RockLeaf(float fAzimuth, float fPitch, int nIndex, CRotTransform& cTrans) const
{
    float fRockAngle(0.0f), fRustleAngle(0.0f);
    if (m_bRockingLeaves)
    {
        if (m_pRockingAngles)
            fRockAngle = m_pRockingAngles[nIndex % m_uiNumLeafAngles] * m_fSpeedWindRockScalar;

        if (m_pRustleAngles)
            fRustleAngle = m_pRustleAngles[nIndex % m_uiNumLeafAngles] * m_fSpeedWindRustleScalar;
    }

    // setup the billboarding/rocking leaf transform
#ifdef UPVECTOR_POS_Z
    cTrans.RotateZ(fAzimuth + c_afLeafOffsets[nIndex]);
    cTrans.RotateY(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateZ(fRustleAngle);
    cTrans.RotateX(fRockAngle);
#endif
#ifdef UPVECTOR_NEG_Z
    cTrans.RotateZ(-(fAzimuth + c_afLeafOffsets[nIndex]));
    cTrans.RotateY(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateZ(fRustleAngle);
    cTrans.RotateX(fRockAngle);
#endif
#ifdef UPVECTOR_POS_Y
    cTrans.RotateY(fAzimuth + c_afLeafOffsets[nIndex]);
    cTrans.RotateZ(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]);
    cTrans.RotateY(-fRustleAngle);
    cTrans.RotateX(fRockAngle);
#endif
#ifdef UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
    cTrans.RotateZ(-(fAzimuth + c_afLeafOffsets[nIndex]));
    cTrans.RotateX(-(fPitch + c_afLeafOffsets[c_nLeafOffsetCount - nIndex - 1]));
    cTrans.RotateZ(fRustleAngle);
    cTrans.RotateY(fRockAngle);
#endif
}
