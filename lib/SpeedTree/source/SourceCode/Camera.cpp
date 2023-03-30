///////////////////////////////////////////////////////////////////////  
//         Name: Camera.cpp
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
#include "TreeEngine.h"


// static variables

CVec3 CIdvCamera::m_cCameraPos(0.0f, 0.0f, 0.0f);
CVec3 CIdvCamera::m_cCameraDir(0.577f, 0.577f, 0.577f);


///////////////////////////////////////////////////////////////////////  
//  CIdvCamera::CIdvCamera function

CIdvCamera::CIdvCamera( ) : 
    m_cPosition(0.0f, 0.0f, 0.0f)
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvCamera::CIdvCamera function

CIdvCamera::CIdvCamera(const CIdvCamera& cRightSide)
{
    *this = cRightSide;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvCamera::~CIdvCamera function

CIdvCamera::~CIdvCamera( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvCamera::operator= function

const CIdvCamera& CIdvCamera::operator=(const CIdvCamera& cRight)
{
    if (&cRight != this)
        m_cPosition = cRight.m_cPosition;

    return *this;
}
