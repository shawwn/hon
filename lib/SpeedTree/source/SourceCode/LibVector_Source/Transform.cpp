//         Name: Transform.cpp
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
#include <cstring>
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

    float fCosine = VectorCosD(fAngle);
    float fSine = VectorSinD(fAngle);

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
        cerr << "default reached in CTransform::Rotate( ) : " << chAxis << endl;
        return;
    }

    *this = cRotMatrix * *this;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::RotateAxis definition

void CTransform::RotateAxis(float fAngle, CVec cAxis)
{
    cAxis.Normalize( );
    float s = VectorSinD(fAngle);
    float c = VectorCosD(fAngle);
    float t = 1.0f - c;

    float x = cAxis.m_afData[0];
    float y = cAxis.m_afData[1];
    float z = cAxis.m_afData[2];

    CTransform  cRotMatrix;
    cRotMatrix.m_afData[0][0] = t * x * x + c;
    cRotMatrix.m_afData[0][1] = t * x * y + s * z;
    cRotMatrix.m_afData[0][2] = t * x * z - s * y;
    cRotMatrix.m_afData[0][3] = 0.0;
    cRotMatrix.m_afData[1][0] = t * x * y - s * z;
    cRotMatrix.m_afData[1][1] = t * y * y + c;
    cRotMatrix.m_afData[1][2] = t * y * z + s * x;
    cRotMatrix.m_afData[1][3] = 0.0;
    cRotMatrix.m_afData[2][0] = t * x * z + s * y;
    cRotMatrix.m_afData[2][1] = t * y * z - s * x;
    cRotMatrix.m_afData[2][2] = t * z * z + c;
    cRotMatrix.m_afData[2][3] = 0.0;
    cRotMatrix.m_afData[3][0] = 0.0;
    cRotMatrix.m_afData[3][1] = 0.0;
    cRotMatrix.m_afData[3][2] = 0.0;
    cRotMatrix.m_afData[3][3] = 1.0;

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
//  CTransform::operator<< definition

ostream& operator<<(ostream& cStream, const CTransform& cTransform)
{
    cStream << '[';
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
        {
            cStream << cTransform.m_afData[i][j];
            if (i == 3 && j == 3)
                continue;
            cStream << ' ';
        }
    cStream << ']';

    return cStream;
}
    

///////////////////////////////////////////////////////////////////////  
//  CTransform::operator* definition

CTransform  CTransform::operator*(const CTransform& cTransform) const
{
    CTransform  cTemp;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
        {
            cTemp.m_afData[i][j] = 0.0;
            for (int k = 0; k < 4; ++k)
                cTemp.m_afData[i][j] += m_afData[i][k] * cTransform.m_afData[k][j];
        }

    return cTemp;
}
    

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

