//         Name: Region.cpp
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

#include "IdvVector.h"
#include <cfloat>
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  CRegion::CRegion definition
//

CRegion::CRegion( )
{
    Initialize( );
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator<< definition
//

ostream& operator<<(ostream& cStream, const CRegion& cRegion)
{
    cStream << "CRegion" << endl;
    cStream << "m_cMin: " << cRegion.m_cMin << endl;
    cStream << "m_cMax: " << cRegion.m_cMax << endl;

    return cStream;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator+ definition
//

CRegion CRegion::operator+(const CRegion& cRegion) const
{
    CRegion cTemp = *this;

    for (int i = 0; i < 3; i++)
    {
        if (cRegion.m_cMin[i] < m_cMin[i])
            cTemp.m_cMin[i] = cRegion.m_cMin[i];
        if (cRegion.m_cMax[i] > m_cMax[i])
            cTemp.m_cMax[i] = cRegion.m_cMax[i];
    }

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator+ definition
//
CRegion CRegion::operator+(const CVec& cVec) const
{
    CRegion cTemp = *this;

    cTemp.m_cMin = cTemp.m_cMin + cVec;
    cTemp.m_cMax = cTemp.m_cMax + cVec;

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator^ definition
//

CRegion CRegion::operator^(const CVec3& cVec) const
{
    CRegion cTemp = *this;

    if (cVec[0] < cTemp.m_cMin[0])
        cTemp.m_cMin[0] = cVec[0];
    if (cVec[1] < cTemp.m_cMin[1])
        cTemp.m_cMin[1] = cVec[1];
    if (cVec[2] < cTemp.m_cMin[2])
        cTemp.m_cMin[2] = cVec[2];

    if (cVec[0] > cTemp.m_cMax[0])
        cTemp.m_cMax[0] = cVec[0];
    if (cVec[1] > cTemp.m_cMax[1])
        cTemp.m_cMax[1] = cVec[1];
    if (cVec[2] > cTemp.m_cMax[2])
        cTemp.m_cMax[2] = cVec[2];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator* definition
//

CRegion CRegion::operator*(float fScale) const
{
    CRegion cTemp = *this;
    CVec cMidpoint = Midpoint( );

    for (int i = 0; i < 3; i++)
    {
        float fDistance = (m_cMax[i] - m_cMin[i]) * 0.5F;
        cTemp.m_cMin[i] = cMidpoint[i] - fDistance * fScale;
        cTemp.m_cMax[i] = cMidpoint[i] + fDistance * fScale;
    }

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator* definition
//

CRegion CRegion::operator*(const CTransform& cTransform) const
{
    CRegion cTemp;

    cTemp.m_cMin = m_cMin * cTransform;
    cTemp.m_cMax = m_cMax * cTransform;

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::operator< definition
//

bool CRegion::operator<(const CRegion& cRegion) const
{
    bool bReturn = true;
    for (int i = 0; i < 3; i++)
        if (m_cMin[i] <= cRegion.m_cMin[i] || m_cMax[i] >= cRegion.m_cMax[i])
        {
            bReturn = false;
            break;
        }

    return bReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::AverageExtent definition
//

float CRegion::AverageExtent( ) const
{
    float fSum = 0.0f;
    for (int i = 0; i < 3; i++)
        fSum += m_cMax[i] - m_cMin[i];

    return 0.33333333f * fSum;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::Initialized definition
//

void CRegion::Initialize( )
{
    m_cMin[0] = m_cMin[1] = m_cMin[2] = FLT_MAX;
    m_cMax[0] = m_cMax[1] = m_cMax[2] = -FLT_MAX;
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::IsSet definition
//

bool CRegion::IsSet( ) const
{
    return (m_cMin[0] != FLT_MAX || 
            m_cMin[1] != FLT_MAX || 
            m_cMin[2] != FLT_MAX || 
            m_cMax[0] != -FLT_MAX || 
            m_cMax[1] != -FLT_MAX || 
            m_cMax[2] != -FLT_MAX);
}


///////////////////////////////////////////////////////////////////////  
//  CRegion::Scale definition
//

void CRegion::Scale(float fScale)
{
    m_cMin = m_cMin * fScale;
    m_cMax = m_cMax * fScale;
}

