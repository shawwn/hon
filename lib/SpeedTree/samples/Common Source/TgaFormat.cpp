//         Name: TgaFormat.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
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
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.

#include "TgaFormat.h"
#include <fstream>
using namespace std;

// uncompressed TGA header key
const unsigned char c_acTGAheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::CTgaFormat

CTgaFormat::CTgaFormat( ) :
    m_nHeight(0),
    m_nWidth(0),
    m_nDepth(0),
    m_pImageData(NULL),
    m_bDeleteData(true)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::CTgaFormat

CTgaFormat::CTgaFormat(unsigned char* pImageData, int nWidth, int nHeight, int nDepth, bool bCopyData) :
    m_nHeight(nHeight),
    m_nWidth(nWidth),
    m_nDepth(nDepth),
    m_pImageData(NULL),
    m_bDeleteData(bCopyData)
{
    if (bCopyData)
    {
        int nSize = m_nWidth * m_nHeight * m_nDepth;
        m_pImageData = new unsigned char[nSize];
        memcpy(m_pImageData, pImageData, nSize);
    }
    else
    {
        m_pImageData = pImageData;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::~CTgaFormat

CTgaFormat::~CTgaFormat( )
{
    if (m_bDeleteData)
        delete[] m_pImageData;
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::Read

bool CTgaFormat::Read(const char* pFilename)
{
    if (m_bDeleteData)
        delete m_pImageData;

    bool bSuccess = false;

    ifstream fsFile(pFilename, ios::binary);

    if (fsFile)
    {
        unsigned char acTGAcompare[12];     // used to compare tga header
        unsigned char acDimensionBytes[6];  // first 6 useful bytes from the header

        // are there 12 bytes to read
        fsFile.read((char*) acTGAcompare, sizeof(acTGAcompare));

        // does the header match the key?
        if(memcmp(c_acTGAheader, acTGAcompare, sizeof(c_acTGAheader)) == 0)
        {
            // if so, read the next 6 header bytes
            fsFile.read((char *) acDimensionBytes, sizeof(acDimensionBytes));

            m_nWidth = (acDimensionBytes[1] << 8) + acDimensionBytes[0];
            m_nHeight = (acDimensionBytes[3] << 8) + acDimensionBytes[2];

            if (m_nWidth > 0 || m_nHeight > 0)
            {
                unsigned int nBitsPerPixel = acDimensionBytes[4];

                if (nBitsPerPixel == 24 || nBitsPerPixel == 32)
                {
                    m_nDepth = nBitsPerPixel / 8;
                    int nImageSize = m_nWidth * m_nHeight * m_nDepth;

                    m_pImageData = new unsigned char[nImageSize];
                    fsFile.read((char *)m_pImageData, nImageSize);

                    fsFile.close( );
                    bSuccess = true;
                }
            }
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::Write

bool CTgaFormat::Write(const char* pFilename)
{
    bool bSuccess = false;

    if (m_pImageData)
    {
        ofstream fsFile(pFilename, ios::binary);
        if (fsFile)
        {
            fsFile.write((char*) c_acTGAheader, sizeof(c_acTGAheader));

            unsigned char acDimensionBytes[6];

            // width
            acDimensionBytes[1] = (m_nWidth & 0xff00) >> 8;
            acDimensionBytes[0] = (m_nWidth & 0x00ff);

            // height
            acDimensionBytes[3] = (m_nHeight & 0xff00) >> 8;
            acDimensionBytes[2] = (m_nHeight & 0x00ff);

            // depth
            acDimensionBytes[4] = m_nDepth * 8;
            acDimensionBytes[5] = 0; // unused?

            fsFile.write((char*) acDimensionBytes, sizeof(acDimensionBytes));
            
            // write image data
            fsFile.write((char*) m_pImageData, m_nWidth * m_nHeight * m_nDepth);

            fsFile.close( );
            bSuccess = true;
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::GetPixel

unsigned char* CTgaFormat::GetPixel(unsigned int nWidth, unsigned int nHeight) const
{
    unsigned char* pPixel = NULL;

    if (m_pImageData)
        pPixel = m_pImageData + m_nDepth * (m_nWidth * nHeight + nWidth);

    return pPixel;
}


///////////////////////////////////////////////////////////////////////  
//  CTgaFormat::SetPixel

void CTgaFormat::SetPixel(const unsigned char* pValue, unsigned int nWidth, unsigned int nHeight)
{
    if (m_pImageData)
        memcpy(m_pImageData + m_nDepth * (m_nWidth * nHeight + nWidth), pValue, m_nDepth);
}



