//         Name: Vector.cpp
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

// static variable definitions
float CVec::m_fTolerance = 0.1f;
float CVec3::m_fTolerance(0.1f);


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec( )
{
    m_afData[0] = m_afData[1] = m_afData[2] = m_afData[3] = m_afData[4] = 0.0f;
    m_nSize = 3;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec(int nSize)
{
    m_afData[0] = m_afData[1] = m_afData[2] = m_afData[3] = m_afData[4] = 0.0f;
    if (nSize > 5)
        nSize = 5;
    m_nSize = nSize;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec(float fX, float fY)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = m_afData[3] = m_afData[4] = 0.0f;
    m_nSize = 2;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec(float fX, float fY, float fZ)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = fZ;
    m_afData[3] = m_afData[4] = 0.0;
    m_nSize = 3;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec(float fX, float fY, float fZ, float fW)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = fZ;
    m_afData[3] = fW;
    m_afData[4] = 0.0;
    m_nSize = 4;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::CVec definition
//

CVec::CVec(float fX, float fY, float fZ, float fW, float fV)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = fZ;
    m_afData[3] = fW;
    m_afData[4] = fV;
    m_nSize = 5;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::~CVec definition
//

CVec::~CVec( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CVec::Magnitude definition
//

float CVec::Magnitude( ) const
{
    float fTemp = 0.0f;
    for (int i = 0; i < m_nSize; ++i)
        fTemp += m_afData[i] * m_afData[i];

    return sqrtf(fTemp);
}


///////////////////////////////////////////////////////////////////////  
//  CVec::MaxAxis definition
//

float CVec::MaxAxis( ) const
{
    float fMax = -FLT_MAX;
    for (int i = 0; i < m_nSize; ++i)
        if (m_afData[i] > fMax)
            fMax = m_afData[i];

    return fMax;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::MinAxis definition
//

float CVec::MinAxis( ) const
{
    float fMin = FLT_MAX;
    for (int i = 0; i < m_nSize; ++i)
        if (m_afData[i] < fMin)
            fMin = m_afData[i];

    return fMin;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::Normalize definition
//

void CVec::Normalize( )
{
    float fMag = Magnitude( );
    if (fMag != 0.0f)
        for (int i = 0; i < m_nSize; ++i)
            m_afData[i] /= fMag;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::RoughlyEqual definition
//

bool CVec::RoughlyEqual(const CVec &cVec) const
{
    bool bValue = true;
    for (int i = 0; i < m_nSize; ++i)
        if (fabs(m_afData[i] - cVec.m_afData[i]) > m_fTolerance)
        {
            bValue = false;
            break;
        }

    return bValue;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::AbsoluteValue definition
//

CVec CVec::AbsoluteValue( ) const
{
    CVec cTemp;
    for (int i = 0; i < m_nSize; ++i)
        if (m_afData[i] < 0.0f)
            cTemp.m_afData[i] = -m_afData[i];
        else
            cTemp.m_afData[i] = m_afData[i];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator<< definition
//

ostream& operator<<(ostream& cStream, const CVec& cVec)
{
    cStream << "(";
    for (int i = 0; i < cVec.m_nSize; ++i)
    {
        cStream << cVec.m_afData[i];
        if (i != cVec.m_nSize - 1)
            cStream << ' ';
    }
    cStream << ")";

    return cStream;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator+ definition
//

CVec CVec::operator+(const CVec& cVec) const
{
    int nSize = -1;
    if (m_nSize < cVec.m_nSize)
        nSize = m_nSize;
    else
        nSize = cVec.m_nSize;

    CVec cTemp(nSize);
    for (int i = 0; i < m_nSize; ++i)
        cTemp.m_afData[i] = m_afData[i] + cVec.m_afData[i];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator+= definition
//

CVec& CVec::operator+=(const CVec& cVec)
{
    *this = *this + cVec;

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator- definition
//

CVec CVec::operator-(const CVec& cVec) const
{
    int nSize = -1;
    if (m_nSize < cVec.m_nSize)
        nSize = m_nSize;
    else
        nSize = cVec.m_nSize;

    CVec cTemp(nSize);
    for (int i = 0; i < m_nSize; ++i)
        cTemp.m_afData[i] = m_afData[i] - cVec.m_afData[i];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator-= definition
//

CVec& CVec::operator-=(const CVec& cVec)
{
    *this = *this - cVec;

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator- definition
//

CVec CVec::operator-( ) const
{
    CVec cTemp(m_nSize);
    for (int i = 0; i < m_nSize; ++i)
        cTemp.m_afData[i] = -m_afData[i];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator^ definition
//

float CVec::operator^(const CVec& cVec) const
{
    float fTemp = 0.0f;
    for (int i = 0; i < m_nSize; ++i) 
        fTemp += m_afData[i] * cVec.m_afData[i];

    return fTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator* definition
//

CVec CVec::operator*(const CVec& cVec) const
{
    CVec cTemp(3);

    cTemp.m_afData[0] = m_afData[1] * cVec.m_afData[2] - m_afData[2] * cVec.m_afData[1];
    cTemp.m_afData[1] = m_afData[2] * cVec.m_afData[0] - m_afData[0] * cVec.m_afData[2];
    cTemp.m_afData[2] = m_afData[0] * cVec.m_afData[1] - m_afData[1] * cVec.m_afData[0];

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator*= definition
//

CVec& CVec::operator*=(const CVec& cVec)
{
    *this = *this * cVec;

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator* definition
//

CVec CVec::operator*(float fValue) const
{
    CVec cTemp(m_nSize);
    for (int i = 0; i < m_nSize; ++i)
        cTemp.m_afData[i] = m_afData[i] * fValue;

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator*= definition
//

CVec& CVec::operator*=(float fValue)
{
    *this = *this * fValue;

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator* definition
//

CVec CVec::operator*(const CTransform& cTransform) const
{
    CVec cTemp(m_nSize);
    for (int i = 0; i < m_nSize; ++i)
    {
        cTemp.m_afData[i] = 0.0;
        for (int j = 0; j < 4; ++j)
            if (j < m_nSize)
                cTemp.m_afData[i] += m_afData[j] * cTransform.m_afData[j][i];
            else if (j == 3)
                cTemp.m_afData[i] += cTransform.m_afData[j][i];
    }

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator/ definition
//

CVec CVec::operator/(float fValue) const
{
    CVec cTemp(m_nSize);

    for (int i = 0; i < m_nSize; ++i)
        cTemp.m_afData[i] = m_afData[i] / fValue;

    return cTemp;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator/= definition
//

CVec& CVec::operator/=(float fValue)
{
    *this = *this / fValue;

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator== definition
//

bool CVec::operator==(const CVec& cVec) const
{
    bool bValue = true;

    for (int i = 0; i < m_nSize; ++i)
        if (m_afData[i] != cVec.m_afData[i])
        {
            bValue = false;
            break;
        }

    return bValue;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator!= definition
//

bool CVec::operator!=(const CVec& cVec) const
{
    bool bValue = true;

    for (int i = 0; i < m_nSize; ++i)
        if (m_afData[i] != cVec.m_afData[i])
        {
            bValue = false;
            break;
        }

    return !bValue;
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator< definition
//

bool operator<(const CVec& CVecA, const CVec& CVecB)
{
    return (CVecA.m_afData[0] < CVecB.m_afData[0] ||
            CVecA.m_afData[1] < CVecB.m_afData[1] ||
            CVecA.m_afData[2] < CVecB.m_afData[2]);
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator< definition
//

bool CVec::operator<(const CRegion& cRegion) const
{
    if (m_nSize == 2)
        return ((m_afData[0] < cRegion.m_cMax[0] && m_afData[0] > cRegion.m_cMin[0]) && (m_afData[1] < cRegion.m_cMax[1] && m_afData[1] > cRegion.m_cMin[1]));
    else
        return ((m_afData[0] < cRegion.m_cMax[0] && m_afData[0] > cRegion.m_cMin[0]) && (m_afData[1] < cRegion.m_cMax[1] && m_afData[1] > cRegion.m_cMin[1]) && (m_afData[2] < cRegion.m_cMax[2] && m_afData[2] > cRegion.m_cMin[2]));
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator<= definition
//

bool CVec::operator<=(const CRegion& cRegion) const
{
    if (m_nSize == 2)
        return ((m_afData[0] <= cRegion.m_cMax[0] && m_afData[0] >= cRegion.m_cMin[0]) && (m_afData[1] <= cRegion.m_cMax[1] && m_afData[1] >= cRegion.m_cMin[1]));
    else
        return ((m_afData[0] <= cRegion.m_cMax[0] && m_afData[0] >= cRegion.m_cMin[0]) && (m_afData[1] <= cRegion.m_cMax[1] && m_afData[1] >= cRegion.m_cMin[1]) && (m_afData[2] <= cRegion.m_cMax[2] && m_afData[2] >= cRegion.m_cMin[2]));
}


///////////////////////////////////////////////////////////////////////  
//  CVec::Distance definition
//

float CVec::Distance(const CVec& cPoint) const
{
    if (m_nSize == 2)
        return sqrtf(
                    ((cPoint[0] - m_afData[0]) * (cPoint[0] - m_afData[0])) +
                    ((cPoint[1] - m_afData[1]) * (cPoint[1] - m_afData[1]))
                    );
    else
        return sqrtf(
                    ((cPoint[0] - m_afData[0]) * (cPoint[0] - m_afData[0])) +
                    ((cPoint[1] - m_afData[1]) * (cPoint[1] - m_afData[1])) +
                    ((cPoint[2] - m_afData[2]) * (cPoint[2] - m_afData[2]))
                    );
}


///////////////////////////////////////////////////////////////////////  
//  CVec::AngleBetween definition
//

float CVec::AngleBetween(const CVec& cVec) const
{
    return (acosf((*this ^ cVec) / (Magnitude( ) * cVec.Magnitude( ))));
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator< definition
//

bool CVec::operator<(const CVec& cVec) const
{
    if (m_afData[0] < cVec.m_afData[0])
        return true;
    else if (m_afData[0] > cVec.m_afData[0])
        return false;
    else
    {
        if (m_afData[1] < cVec.m_afData[1])
            return true;
        else if (m_afData[1] > cVec.m_afData[1])
            return false;
        else
            return m_afData[2] < cVec.m_afData[2];
    }
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator<= definition
//

bool CVec::operator<=(const CVec& cVec) const
{
    if (m_afData[0] <= cVec.m_afData[0])
    {
        return true;
    }
    else
    {
        return false;
    }

}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator> definition
//

bool CVec::operator>(const CVec& cVec) const
{
    if (m_afData[0] > cVec.m_afData[0])
        return true;
    else if (m_afData[0] < cVec.m_afData[0])
        return false;
    else
    {
        if (m_afData[1] > cVec.m_afData[1])
            return true;
        else if (m_afData[1] < cVec.m_afData[1])
            return false;
        else
            return m_afData[2] > cVec.m_afData[2];
    }
}


///////////////////////////////////////////////////////////////////////  
//  CVec::operator>= definition
//

bool CVec::operator>=(const CVec& cVec) const
{
    return (m_afData[0] >= cVec.m_afData[0]);
}