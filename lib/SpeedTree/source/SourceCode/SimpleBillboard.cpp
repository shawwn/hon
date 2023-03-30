///////////////////////////////////////////////////////////////////////  
//         Name: SpeedTreeRT.cpp
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

#include "Debug.h"
#include "SimpleBillboard.h"
#include "UpVector.h"


// static variables
CVec3 CSimpleBillboard::m_acUnitBillboard[4] =
{
    CVec3(0.0f, 0.5f, 1.0f),
    CVec3(0.0f, -0.5f, 1.0f),
    CVec3(0.0f, -0.5f, 0.0f),
    CVec3(0.0f, 0.5f, 0.0f)
};


///////////////////////////////////////////////////////////////////////  
//  CSimpleBillboard::CSimpleBillboard definition

CSimpleBillboard::CSimpleBillboard( ) :
    m_bValid(false)
{
    //lint -esym(1401, CSimpleBillboard::m_acTreeBillboard)  { it's static, so it's initialized above }
}


///////////////////////////////////////////////////////////////////////  
//  CSimpleBillboard::ComputeUnitBillboard definition

void CSimpleBillboard::ComputeUnitBillboard(const float* pCameraDir)
{
    // find the angle that point is on
    CVec3 cAdjustedDirection;

    Assign3d(cAdjustedDirection, pCameraDir);
    float fAzimuth = VecRad2Deg(atan2f(cAdjustedDirection[1], cAdjustedDirection[0]));

    static CVec3 afCorner1(0.0f, 0.5f, 1.0f),
                 afCorner2(0.0f, -0.5f, 1.0f),
                 afCorner3(0.0f, -0.5f, 0.0f),
                 afCorner4(0.0f, 0.5f, 0.0f);
    static CVec3 afRawCorners[4];
    Assign3d(afRawCorners[0], afCorner1);
    Assign3d(afRawCorners[1], afCorner2);
    Assign3d(afRawCorners[2], afCorner3);
    Assign3d(afRawCorners[3], afCorner4);

    CRotTransform cTrans;
#ifdef UPVECTOR_POS_Z
    cTrans.RotateZ(fAzimuth);
#endif
#ifdef UPVECTOR_NEG_Z
    cTrans.RotateZ(-fAzimuth);
#endif
#ifdef UPVECTOR_POS_Y
    cTrans.RotateY(fAzimuth);
#endif
#ifdef UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
    cTrans.RotateZ(-fAzimuth);
#endif

    m_acUnitBillboard[0] = afRawCorners[0] * cTrans;
    m_acUnitBillboard[1] = afRawCorners[1] * cTrans;
    m_acUnitBillboard[2] = afRawCorners[2] * cTrans;
    m_acUnitBillboard[3] = afRawCorners[3] * cTrans;
}



