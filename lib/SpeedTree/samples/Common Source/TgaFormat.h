//         Name: TgaFormat.h
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

#pragma once

///////////////////////////////////////////////////////////////////////  
//  class CTgaFormat
//
//  This class is part of an internal mechanism for making off-line movies
//  of an automated flythrough of the forest.  
//
//  Currently only reads uncompressed 24 and 32-bit TGA files.  Raw data
//  format is either RGB or RGBA, one byte per color component.  Data is
//  packed by rows (first width-number of entries are all the same row).
//
//  Pixel functions deal in small arrays of pixels, either RGB or RGBA

class CTgaFormat
{
public:
        CTgaFormat( );
        CTgaFormat(unsigned char* pImageData, int nWidth, int nHeight, int nDepth, bool bCopyData = true);
        ~CTgaFormat( );

        bool                Read(const char* pFilename);    // returns true on success
        bool                Write(const char* pFilename);   // returns true on success

        unsigned int        GetWidth(void) const            { return m_nWidth; }
        unsigned int        GetHeight(void) const           { return m_nHeight; }
        unsigned int        GetDepth(void) const            { return m_nDepth; } // in bytes (3 = RGB, 4 = RGBA)

        unsigned char*      GetPixel(unsigned int nWidth, unsigned int nHeight) const;
        void                SetPixel(const unsigned char* pValue, unsigned int nWidth, unsigned int nHeight);

        // raw data format is either RGB or RGBA, one byte per color component
        unsigned char*      GetRawData(void) const          { return m_pImageData; }
private:

        unsigned int        m_nHeight;          // in pixels
        unsigned int        m_nWidth;           // in pixels
        unsigned int        m_nDepth;           // in bytes (3 = RGB, 4 = RGBA)
        unsigned char*      m_pImageData;       // raw data in GL_ABGR_EXT format
        bool                m_bDeleteData;      // flag - make a copy of the data passed in from 2nd constructor?
};


