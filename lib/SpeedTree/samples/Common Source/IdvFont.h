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
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com

#pragma once
#include <map>
#include <string>
#include <vector>
#include "extgl.h"
#include <GL/glu.h>


///////////////////////////////////////////////////////////////////////  
//  class CIdvFont declaration
//
//  Left undone as of 6/11/2003:
//
//      1. Caching for multiply requested font names/files.
//      2. Destructor not implemented because of static sharing

class CIdvFont
{
public:
        struct SAttrib
        {
            // general
            std::string         m_strName;
            std::string         m_strFilename;
            unsigned int        m_uiTextureWidth;
            unsigned int        m_uiTextureHeight;
            unsigned int        m_uiFontHeight;
            unsigned int        m_uiTextureId;
            unsigned int        m_uiDisplayLists;

            // character dimensions
            struct SCharDimensions
            {
                float           m_fLeft;
                float           m_fRight;
                float           m_fTop;
                float           m_fBottom;
                bool            m_bEnabled;
                float           m_fWidth;
            };
            SCharDimensions     m_asChars[256];

            // operators
            int operator<(const SAttrib& sRight)  { return m_strFilename < sRight.m_strFilename; }
            int operator==(const SAttrib& sRight) { return m_strFilename == sRight.m_strFilename; }
        };

        CIdvFont( );
        ~CIdvFont( );

        // rendering
        void                    BindTexture(void) const                         { glBindTexture(GL_TEXTURE_2D, m_vFonts[m_nActiveFont].m_uiTextureId); }
        void                    Draw(const char* pString, ...) const;
        void                    DrawNoTrans(const char* pString, ...) const;
        void                    Draw(char chChar) const                         { glCallList(m_vFonts[m_nActiveFont].m_uiDisplayLists + chChar); }
        void                    EndFlatProjection(void) const;
        bool                    LoadFont(std::string strFontName, std::string strFontFile);
        void                    ResizeEvent(void);
        void                    SetActiveFont(std::string strFontName);
        void                    StartFlatProjection(void);

        // info
        std::string             GetActiveFont(void) const;
        const SAttrib&          GetFontAttributes(void) const                   { return m_vFonts[m_nActiveFont]; }
        float                   GetHeight(void) const                           { return float(m_vFonts[m_nActiveFont].m_uiFontHeight); }
        float                   GetStringWidth(std::string strString) const;
        bool                    ValidFont(void) const                           { return m_nActiveFont > -1 && m_nActiveFont < int(m_vFonts.size( )); }

private:
        void                    InitFont(SAttrib& sFont);
        int                     LookupName(std::string strName);
        bool                    ParseFont(std::string strFontName, std::string strFontFile, SAttrib& sFont);

        int                     m_nActiveFont;
static  std::vector<SAttrib>    m_vFonts;
        unsigned int            m_uiScreenWidth;
        unsigned int            m_uiScreenHeight;
};
