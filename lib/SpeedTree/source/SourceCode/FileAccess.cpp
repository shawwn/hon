///////////////////////////////////////////////////////////////////////  
//         Name: FileAccess.cpp
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
#include "FileAccess.h"
#include "Endian.h"
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ConvertMemoryToArray definition

unsigned char* CTreeFileAccess::ConvertMemoryToArray(unsigned int& nNumBytes)
{
    unsigned char* pArray = new unsigned char[m_vMemory.size( )];

    for (unsigned int i = 0; i < m_vMemory.size( ); ++i)
        pArray[i] = m_vMemory[i];

    nNumBytes = m_vMemory.size( );

    return pArray;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseFloat definition

float CTreeFileAccess::ParseFloat(void)
{
    st_assert(!EndOfFile( ));

    // floats are written little-endian directly in file
    float* pReturn = reinterpret_cast<float*>(&m_vMemory[m_nIndex]);
    m_nIndex += 4;
#ifdef ST_BIG_ENDIAN
    unsigned long ulRawValue = *reinterpret_cast<long*>(pReturn);
    ulRawValue = EndianSwap32(ulRawValue);
    pReturn = reinterpret_cast<float*>(&ulRawValue);
#endif

    return *pReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseInt definition

int CTreeFileAccess::ParseInt(void)
{
    st_assert(!EndOfFile( ));

    // 4-byte ints are written little-endian directly in file
    int* pReturn = reinterpret_cast<int*>(&m_vMemory[m_nIndex]);
    m_nIndex += 4;
#ifdef ST_BIG_ENDIAN
    int nSwapValue = EndianSwap32(*pReturn);
    pReturn = &nSwapValue;
#endif

    return *pReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseLong definition

unsigned long CTreeFileAccess::ParseLong(void)
{
    st_assert(!EndOfFile( ));

    // 4-byte longs are written little-endian directly in file
    unsigned long* pReturn = reinterpret_cast<unsigned long*>(&m_vMemory[m_nIndex]);
    m_nIndex += 4;
#ifdef ST_BIG_ENDIAN
    unsigned long ulSwapValue = EndianSwap32(*pReturn);
    pReturn = &ulSwapValue;
#endif

    // longs are padded with 4 bytes for future expansion
    m_nIndex += 4;

    return *pReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseShort definition

short CTreeFileAccess::ParseShort(void)
{
    st_assert(!EndOfFile( ));

    // 2-byte shorts are written little-endian directly in file
    short* pReturn = reinterpret_cast<short*>(&m_vMemory[m_nIndex]);
    m_nIndex += 2;
#ifdef ST_BIG_ENDIAN
    short sSwapValue = EndianSwap16(*pReturn);
    pReturn = &sSwapValue;
#endif

    return *pReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseString definition

string CTreeFileAccess::ParseString(void)
{
    st_assert(!EndOfFile( ));

    // a 4-byte string length is stored first, then each character
    // in the string
    string strReturn;
    int nLength = ParseInt( );
    for (int i = 0; i < nLength; ++i)
        strReturn += char(m_vMemory[m_nIndex++]);

    return strReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseVector definition

CVec CTreeFileAccess::ParseVector(int nSize)
{
    st_assert(!EndOfFile( ));

    CVec cVec(nSize);
    for (int i = 0; i < nSize; ++i)
        cVec[i] = ParseFloat( );

    return cVec;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::ParseVector3 definition

CVec3 CTreeFileAccess::ParseVector3(void)
{
    st_assert(!EndOfFile( ));

    CVec3 cVec;
    for (int i = 0; i < 3; ++i)
        cVec[i] = ParseFloat( );

    return cVec;
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveFloat definition

void CTreeFileAccess::SaveFloat(float fFloat)
{
    unsigned char* pFloatBytes = reinterpret_cast<unsigned char*>(&fFloat);

    m_vMemory.push_back(*pFloatBytes++);
    m_vMemory.push_back(*pFloatBytes++);
    m_vMemory.push_back(*pFloatBytes++);
    m_vMemory.push_back(*pFloatBytes++);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveInt definition

void CTreeFileAccess::SaveInt(int nInt)
{
    unsigned char* pIntBytes = reinterpret_cast<unsigned char*>(&nInt);

    m_vMemory.push_back(*pIntBytes++);
    m_vMemory.push_back(*pIntBytes++);
    m_vMemory.push_back(*pIntBytes++);
    m_vMemory.push_back(*pIntBytes++);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveLong definition

void CTreeFileAccess::SaveLong(unsigned long ulLong)
{
    unsigned char* pLongBytes = reinterpret_cast<unsigned char*>(&ulLong);

    m_vMemory.push_back(*pLongBytes++);
    m_vMemory.push_back(*pLongBytes++);
    m_vMemory.push_back(*pLongBytes++);
    m_vMemory.push_back(*pLongBytes++);

    // longs are padded with 4 bytes for future expansion
    for (int i = 0; i < 4; ++i)
        m_vMemory.push_back(0);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveShort definition

void CTreeFileAccess::SaveShort(short sShort)
{
    unsigned char* pShortBytes = reinterpret_cast<unsigned char*>(&sShort);

    m_vMemory.push_back(*pShortBytes++);
    m_vMemory.push_back(*pShortBytes++);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveString definition

void CTreeFileAccess::SaveString(string strString)
{
    SaveInt(strString.length( ));

    for (unsigned int i = 0; i < strString.length( ); ++i)
        m_vMemory.push_back(strString[i]);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveVector definition

void CTreeFileAccess::SaveVector(const CVec& cVec, int nSize)
{
    for (int i = 0; i < nSize; ++i)
        SaveFloat(cVec[i]);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::SaveVector3 definition

void CTreeFileAccess::SaveVector3(const CVec3& cVec)
{
    for (int i = 0; i < 3; ++i)
        SaveFloat(cVec[i]);
}


///////////////////////////////////////////////////////////////////////  
//  CTreeFileAccess::PeekToken definition

EFileToken CTreeFileAccess::PeekToken(void)
{
    st_assert(!EndOfFile( ));

    // 4-byte tokens (ints) are written little-endian directly in file
    int* pReturn = reinterpret_cast<int*>(&m_vMemory[m_nIndex]);
#ifdef ST_BIG_ENDIAN
    int nSwapValue = EndianSwap32(*pReturn);
    pReturn = &nSwapValue;
#endif

    return static_cast<EFileToken>(*pReturn);
}
