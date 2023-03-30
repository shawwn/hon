//         Name: Transform.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2002 IDV, Inc.
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

#include "Transform.h"
#include <cmath>
#include <vector>
using namespace std;
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::CTransform definition

CTransform::CTransform( )
{
    LoadIdentity( );
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::Set definition

void CTransform::Set(float afData[4][4])
{
    memcpy(&m_afData[0][0], &afData[0][0], 16 * sizeof(float));
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::LoadIdentity definition

void CTransform::LoadIdentity( )
{
    m_afData[0][0] = 1.0f;
    m_afData[0][1] = 0.0f;
    m_afData[0][2] = 0.0f;
    m_afData[0][3] = 0.0f;
    m_afData[1][0] = 0.0f;
    m_afData[1][1] = 1.0f;
    m_afData[1][2] = 0.0f;
    m_afData[1][3] = 0.0f;
    m_afData[2][0] = 0.0f;
    m_afData[2][1] = 0.0f;
    m_afData[2][2] = 1.0f;
    m_afData[2][3] = 0.0f;
    m_afData[3][0] = 0.0f;
    m_afData[3][1] = 0.0f;
    m_afData[3][2] = 0.0f;
    m_afData[3][3] = 1.0f;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::Translate definition

void CTransform::Translate(float fX, float fY, float fZ)
{
    CTransform  cTemp;

    cTemp.m_afData[3][0] = fX;
    cTemp.m_afData[3][1] = fY;
    cTemp.m_afData[3][2] = fZ;

    *this = cTemp * *this;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::Rotate definition

void CTransform::Rotate(float fAngle, char chAxis)
{
    CTransform cRotMatrix;

    float fCosine = cosf(fAngle);
    float fSine = sinf(fAngle);

    switch (chAxis)
    {
    case 'x': case 'X':
        cRotMatrix.m_afData[1][1] = fCosine;
        cRotMatrix.m_afData[1][2] = fSine;
        cRotMatrix.m_afData[2][1] = -fSine;
        cRotMatrix.m_afData[2][2] = fCosine;
        break;
    case 'y': case 'Y':
        cRotMatrix.m_afData[0][0] = fCosine;
        cRotMatrix.m_afData[0][2] = -fSine;
        cRotMatrix.m_afData[2][0] = fSine;
        cRotMatrix.m_afData[2][2] = fCosine;
        break;
    case 'z': case 'Z':
        cRotMatrix.m_afData[0][0] = fCosine;
        cRotMatrix.m_afData[1][0] = -fSine;
        cRotMatrix.m_afData[0][1] = fSine;
        cRotMatrix.m_afData[1][1] = fCosine;
        break;
    default:
        return;
    }

    *this = cRotMatrix * *this;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::IsIdentity definition

bool CTransform::IsIdentity(void) const
{
    CTransform cId;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (cId.m_afData[i][j] != m_afData[i][j])
                return false;

    return true;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::Scale definition

void CTransform::Scale(float fX)
{
    CTransform  cTemp;

    cTemp.m_afData[0][0] = fX;
    cTemp.m_afData[1][1] = fX;
    cTemp.m_afData[2][2] = fX;

    *this = cTemp * *this;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::Scale definition

void CTransform::Scale(float fX, float fY, float fZ)
{
    CTransform  cTemp;

    cTemp.m_afData[0][0] = fX;
    cTemp.m_afData[1][1] = fY;
    cTemp.m_afData[2][2] = fZ;

    *this = cTemp * *this;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator* definition

CTransform  CTransform::operator*(const CTransform& cTransform) const
{
    CTransform  cTemp;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
        {
            cTemp.m_afData[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k)
                cTemp.m_afData[i][j] += m_afData[i][k] * cTransform.m_afData[k][j];
        }

    return cTemp;
}
    
/*
///////////////////////////////////////////////////////////////////////  
//  CTransform::operator* definition

CVec CTransform::operator*(const CVec& cVec) const
{
    int nSize = cVec.GetSize( );
    CVec cTemp(nSize);

    for (int i = 0; i < nSize; ++i)
    {
        cTemp.m_afData[i] = 0.0f;
        for (int j = 0; j < 4; ++j)
            if (j < nSize)
                    cTemp.m_afData[i] += m_afData[i][j] * cVec.m_afData[j];
            else if (j == 3)
                    cTemp.m_afData[i] += m_afData[i][j];
    }

    return cTemp;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator* definition

CVec3 CTransform::operator*(const CVec3& cVec3) const
{
    CVec3 cTemp;

    for (int i = 0; i < 3; ++i)
    {
        cTemp.m_afData[i] = 0.0f;
        for (int j = 0; j < 4; ++j)
            if (j < 3)
                    cTemp.m_afData[i] += m_afData[i][j] * cVec3.m_afData[j];
            else if (j == 3)
                    cTemp.m_afData[i] += m_afData[i][j];
    }
    return cTemp;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator* definition

CRegion CTransform::operator*(const CRegion& cRegion) const
{
    CRegion cTemp;

    cTemp.m_cMin = *this * cRegion.m_cMin;
    cTemp.m_cMax = *this * cRegion.m_cMax;

    return cTemp;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator+ definition

CTransform CTransform::operator+(const CTransform& cTransform) const
{
    CTransform  cTemp;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            cTemp.m_afData[i][j] = cTransform.m_afData[i][j] + m_afData[i][j];

    return cTemp;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator- definition

CTransform CTransform::operator-(const CTransform& cTransform) const
{
    CTransform  cTemp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            cTemp.m_afData[i][j] = m_afData[i][j] - cTransform.m_afData[i][j];

    return cTemp;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::LookAt definition

void CTransform::LookAt(const CVec& cEye, const CVec& cCenter, const CVec& cUp)
{
    CVec cF = cCenter - cEye;
    cF.Normalize( );

    CVec cUpPrime = cUp;
    cUpPrime.Normalize( );

    CVec cS = cF * cUpPrime;
    CVec cU = cS * cF;

    CTransform cTemp;
    cTemp.m_afData[0][0] = cS[0];
    cTemp.m_afData[1][0] = cS[1];
    cTemp.m_afData[2][0] = cS[2];

    cTemp.m_afData[0][1] = cU[0];
    cTemp.m_afData[1][1] = cU[1];
    cTemp.m_afData[2][1] = cU[2];

    cTemp.m_afData[0][2] = -cF[0];
    cTemp.m_afData[1][2] = -cF[1];
    cTemp.m_afData[2][2] = -cF[2];

    *this = cTemp * *this;

    Translate(-cEye.m_afData[0], -cEye.m_afData[1], -cEye.m_afData[2]);
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::ludcmp definition

CTransform::EInversionCodeType CTransform::ludcmp(float a[5][5], int n, int *indx, float *d)
{
    // code was taken from Numerical Recipes in C

    const double c_dTiny = 1.0e-20;

    int     i,imax,j,k;
    float   big,dum,sum,cTemp;
    float   vv[5];

    *d = 1.0;
    for (i = 1; i <= n; i++)
    {
        big = 0.0;
        for (j = 1; j <= n; j++)
            if ((cTemp = fabs(a[i][j])) > big)
                big = cTemp;
        if (big == 0.0)
        {
            return SINGULAR;
        }
        vv[i] = 1.0 / big;
    }
    for (j = 1; j <= n; j++)
    {
        for (i = 1; i < j; i++)
        {
            sum = a[i][j];
            for (k = 1; k < i; k++)
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
        }
        big=0.0;
        for (i = j; i <= n; i++)
        {
            sum = a[i][j];
            for (k = 1; k < j; k++)
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
            if ((dum = vv[i] * fabs(sum)) >= big)
            {
                big = dum;
                imax = i;
            }
        }
        if (j != imax)
        {
            for (k = 1; k <= n; k++)
            {
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            *d = -(*d);
            vv[imax] = vv[j];
        }
        indx[j] = imax;
        if (a[j][j] == 0.0)
            a[j][j] = c_dTiny;
        if (j != n)
        {
            dum = 1.0 / (a[j][j]);
            for (i = j + 1; i <= n; i++)
                a[i][j] *= dum;
        }
    }

    return OK;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::lubksb definition

void CTransform::lubksb(float a[5][5], int n, int *indx, float b[ ])
{
    // code was taken from Numerical Recipes in C

    int i, ii = 0, ip, j;
    float   sum;

    for (i = 1; i <= n; i++)
    {
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii)
            for (j = ii; j <= i - 1; j++)
                sum -= a[i][j] * b[j];
        else if (sum)
            ii=i;
        b[i] = sum;
    }
    for (i = n; i >= 1 ; i--)
    {
        sum = b[i];
        for (j = i + 1; j <= n; j++) 
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
    }
}
*/
