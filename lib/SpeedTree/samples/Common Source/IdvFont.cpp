///////////////////////////////////////////////////////////////////////  
//  CIdvFont Class
//
//  (c) 2003 IDV, Inc.
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//  The .fnt file format was taken directly from the LFontRenderer class
//  written by Lev Povalahev (http://www.levp.de/3d).
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

#pragma warning (disable : 4786)

#include "IdvFont.h"
#include <fstream>
#include <algorithm>
using namespace std;

// static variables
//map<string, int> CIdvFont::m_mFontNames;
vector<CIdvFont::SAttrib> CIdvFont::m_vFonts;


// constants
const int c_nFileMagicHeader = 6666;


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::CIdvFont definition

CIdvFont::CIdvFont( ) :
    m_nActiveFont(-1),
    m_uiScreenWidth(0),
    m_uiScreenHeight(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::~CIdvFont definition

CIdvFont::~CIdvFont( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::Draw definition

void CIdvFont::Draw(const char* pString, ...) const
{
    va_list vlArgs;
    char szVargsBuffer[1024];
    
    va_start(vlArgs, pString);
    vsprintf(szVargsBuffer, pString, vlArgs);

    if (ValidFont( ))
    {
        int nLength = strlen(szVargsBuffer);

        glListBase(m_vFonts[m_nActiveFont].m_uiDisplayLists);
        glCallLists(nLength, GL_UNSIGNED_BYTE, reinterpret_cast<const GLvoid*>(szVargsBuffer));
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::DrawNoTrans definition

void CIdvFont::DrawNoTrans(const char* pString, ...) const
{
    va_list vlArgs;
    char szVargsBuffer[1024];
    
    va_start(vlArgs, pString);
    vsprintf(szVargsBuffer, pString, vlArgs);

    if (ValidFont( ))
    {
        int nLength = strlen(szVargsBuffer);

        glListBase(m_vFonts[m_nActiveFont].m_uiDisplayLists);
        glCallLists(nLength, GL_UNSIGNED_BYTE, reinterpret_cast<const GLvoid*>(szVargsBuffer));

        float fWidth = GetStringWidth(szVargsBuffer);
        glTranslatef(-fWidth, 0.0f, 0.0f);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::LoadFont definition

bool CIdvFont::LoadFont(string strFontName, string strFontFile)
{
    bool bSuccess = false;

    // has this font already been loaded?
    for (unsigned int i = 0; i < m_vFonts.size( ) && !bSuccess; ++i)
    {
        if (stricmp(m_vFonts[i].m_strFilename.c_str( ), strFontFile.c_str( )) == 0)
        {
            // found one, what was it named?
            if (m_vFonts[i].m_strName != strFontName)
            {
                // add a copy with a different name (not copying the actual
                // texture, just the texture id)
                SAttrib sNewFont = m_vFonts[i];
                sNewFont.m_strName = strFontName;
                m_vFonts.push_back(sNewFont);
            }
            bSuccess = true;
        }
    }

    // if it wasn't already loaded, load it now
    if (!bSuccess)
    {
        SAttrib sFont;
        if (ParseFont(strFontName, strFontFile, sFont))
        {
            sFont.m_strName = strFontName;
            sFont.m_strFilename = strFontFile;

            InitFont(sFont);
            m_vFonts.push_back(sFont);
            bSuccess = true;
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::ResizeEvent definition

void CIdvFont::ResizeEvent(void)
{
    GLint anViewport[4];
    glGetIntegerv(GL_VIEWPORT, anViewport);

    m_uiScreenWidth = anViewport[2];
    m_uiScreenHeight = anViewport[3];
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::SetActiveFont definition

void CIdvFont::SetActiveFont(string strFontName)
{
    int nIndex = LookupName(strFontName);
    if (nIndex > -1)
    {
        m_nActiveFont = nIndex;
        glBindTexture(GL_TEXTURE_2D, m_vFonts[m_nActiveFont].m_uiTextureId);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::StartFlatProjection definition

void CIdvFont::StartFlatProjection(void)
{
    glBindTexture(GL_TEXTURE_2D, m_vFonts[m_nActiveFont].m_uiTextureId);

    if (!m_uiScreenWidth || !m_uiScreenHeight)
        ResizeEvent( );

    glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT); // retain matrix mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix( );
    
    glLoadIdentity( );
    gluOrtho2D(0.0f, m_uiScreenWidth, m_uiScreenHeight, 0.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix( );
    glLoadIdentity( );

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::EndFlatProjection definition

void CIdvFont::EndFlatProjection(void) const
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix( );

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix( );

    glPopAttrib( );
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::GetStringWidth definition

string CIdvFont::GetActiveFont(void) const
{
    string strName = "";

    if (ValidFont( ))
        strName = m_vFonts[m_nActiveFont].m_strName;

    return strName;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::GetStringWidth definition

float CIdvFont::GetStringWidth(string strString) const
{
    float fWidth = 0.0f;

    const SAttrib& sFont = m_vFonts[m_nActiveFont];
    int nLength = strString.length( );
    for (int i = 0; i < nLength; ++i)
        fWidth += sFont.m_asChars[strString[i]].m_fWidth;

    return fWidth;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::InitFont definition

void CIdvFont::InitFont(SAttrib& sFont)
{
    sFont.m_uiDisplayLists = glGenLists(256);
    for (int i = 0; i < 256; ++i)
    {
        const SAttrib::SCharDimensions* pChar = sFont.m_asChars + i;

        glNewList(sFont.m_uiDisplayLists + i, GL_COMPILE);
        if (sFont.m_asChars[i].m_bEnabled)
        {
            glBegin(GL_QUADS);
                glTexCoord2f(pChar->m_fLeft, pChar->m_fTop);
                glVertex2f(0.0f, 0.0f);

                glTexCoord2f(pChar->m_fLeft, pChar->m_fBottom);
                glVertex2f(0.0f, float(sFont.m_uiFontHeight));

                glTexCoord2f(pChar->m_fRight, pChar->m_fBottom);
                glVertex2f(pChar->m_fWidth, float(sFont.m_uiFontHeight));

                glTexCoord2f(pChar->m_fRight, pChar->m_fTop);
                glVertex2f(pChar->m_fWidth, 0.0f);
            glEnd( );
            glTranslatef(pChar->m_fWidth, 0.0f, 0.0f);
        }
        glEndList( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::LookupName definition

int CIdvFont::LookupName(string strFontName)
{
    int nIndex = -1;

    for (vector<SAttrib>::iterator i = m_vFonts.begin( ); i != m_vFonts.end( ) && nIndex == -1; ++i)
        if (i->m_strName == strFontName)
            nIndex = i - m_vFonts.begin( );

    return nIndex;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvFont::ParseFont definition

bool CIdvFont::ParseFont(string strFontName, string strFontFile, SAttrib& sFont)
{
    bool bSuccess = false;
    
    ifstream fsInput(strFontFile.c_str( ), ios::binary);
    if (fsInput)
    {
        // make sure this file is the correct type by checking the header
        unsigned int uiFileId = 0;
        fsInput.read(reinterpret_cast<char*>(&uiFileId), sizeof(unsigned int));
        if (uiFileId == c_nFileMagicHeader)
        {
            // read general font/texture dimensions
            unsigned int uiTextureWidth = 0, uiTextureHeight = 0, uiFontHeight = 0;
            fsInput.read(reinterpret_cast<char*>(&sFont.m_uiTextureWidth), sizeof(unsigned int));
            fsInput.read(reinterpret_cast<char*>(&sFont.m_uiTextureHeight), sizeof(unsigned int));
            fsInput.read(reinterpret_cast<char*>(&sFont.m_uiFontHeight), sizeof(unsigned int));

            // read dimensions for each charactor in 256-char ASCII chart
            for (int i = 0; i < 256; ++i)
            {
                unsigned int uiSize = 0;

                // top
                fsInput.read(reinterpret_cast<char*>(&uiSize), sizeof(unsigned int));
                sFont.m_asChars[i].m_fTop = static_cast<float>(uiSize) / sFont.m_uiTextureHeight;
                // left
                fsInput.read(reinterpret_cast<char*>(&uiSize), sizeof(unsigned int));
                sFont.m_asChars[i].m_fLeft = static_cast<float>(uiSize) / sFont.m_uiTextureWidth;
                // bottom
                fsInput.read(reinterpret_cast<char*>(&uiSize), sizeof(unsigned int));
                sFont.m_asChars[i].m_fBottom = static_cast<float>(uiSize) / sFont.m_uiTextureHeight;
                // right
                fsInput.read(reinterpret_cast<char*>(&uiSize), sizeof(unsigned int));
                sFont.m_asChars[i].m_fRight = static_cast<float>(uiSize) / sFont.m_uiTextureWidth;
                // enabled
                fsInput.read(reinterpret_cast<char*>(&uiSize), sizeof(unsigned int));
                sFont.m_asChars[i].m_bEnabled = (uiSize != 0);
                // width factor
                float fWidthFactor = 1.0f;
                fsInput.read(reinterpret_cast<char*>(&fWidthFactor), sizeof(float));
                sFont.m_asChars[i].m_fWidth = fWidthFactor * sFont.m_uiFontHeight;
            }
        }

        // allocate and read the texture map
        if (!fsInput.eof( ) &&
            !fsInput.fail( ))
        {
            unsigned int uiArea = sFont.m_uiTextureWidth * sFont.m_uiTextureHeight;
            unsigned char* pRawMap = new unsigned char[uiArea];
            fsInput.read(reinterpret_cast<char*>(pRawMap), uiArea);

            // we've only read the luminance values, but we need a luminance + alpha
            // buffer, so we make a new buffer and duplicate the luminance values
            unsigned char* pTexMap = new unsigned char[2 * uiArea];
            unsigned char* pMap = pTexMap;
            for (unsigned int i = 0; i < uiArea; ++i)
            {
                *pMap++ = pRawMap[i];
                *pMap++ = pRawMap[i];
            }
            delete[] pRawMap;
            pRawMap = NULL;

            // make texture map out of new buffer
            glGenTextures(1, &sFont.m_uiTextureId);
            glBindTexture(GL_TEXTURE_2D, sFont.m_uiTextureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, sFont.m_uiTextureWidth, sFont.m_uiTextureHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pTexMap);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glBindTexture(GL_TEXTURE_2D, 0);

            delete[] pTexMap;
            pTexMap = NULL;

            bSuccess = true;
        }

        fsInput.close( );
    }

    return bSuccess;
}
