///////////////////////////////////////////////////////////////////////  
//  CConsole Class
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
//

#pragma once
#pragma warning (disable : 4786)
#include <string>
#include "IdvFont.h"



///////////////////////////////////////////////////////////////////////  
//  class CConsole declaration

class CConsole
{
public:
        CConsole( );
        ~CConsole( );

//      void                        Activate(bool bFlag)        { m_bActive = bFlag; }
        bool                        IsActive(void) const        { return m_bActive; }

        void                        Print(const char* pField, ...);
        void                        PrintError(const char* pField, ...);
        void                        Draw(void);
        void                        InitGraphics(void);
        void                        ResizeEvent(void);
        void                        Scroll(int nDelta)                      { m_nScroll += nDelta; m_nScroll = max(0, m_nScroll); }
        void                        SetColor(const float* pColor);
        void                        SetColor(float fRed, float fGreen, float fBlue);
        void                        SetMessageCallback(void (*pfCallback) (void));
        void                        SetTimeout(float fUpTime, float fFadeTime);

        void IncreaseIndent(void)
        {
            m_uiIndent++;
        }
        void DecreaseIndent(void)
        {
            if (m_uiIndent != 0)
                m_uiIndent--;
        }

private:
        // message entries in the console are stored using this structure
        struct SStringEntry
        {
            SStringEntry(std::string strString, const float* pColor) :
                m_strString(strString)
            {
                memcpy(m_afColor, pColor, 3 * sizeof(float));
            }

            std::string m_strString;
            float       m_afColor[3];
        };

        void                        DrawBackground(float fAlpha) const;
        float                       ProcessTimeout(void);

        void                        DetermineLines(void);
        float                       TabOffset(float fCurrentPos) const;

        float                       m_afCurrentColor[3];
        int                         m_nStartingLine;
        int                         m_nScroll;
        std::vector<SStringEntry>   m_vMessageBuffer;
        void                        (*m_pfMessageCallback) (void);
        CIdvFont                    m_cFont;

        std::vector<std::vector<SStringEntry> > m_vLines;

        // window related
        unsigned int                m_uiConsoleLeft;
        unsigned int                m_uiConsoleRight;
        unsigned int                m_uiConsoleTop;
        unsigned int                m_uiConsoleBottom;
        unsigned int                m_uiConsoleWidth;
        unsigned int                m_uiConsoleHeight;
        unsigned int                m_uiWindowWidth;
        unsigned int                m_uiWindowHeight;
        unsigned int                m_uiNumVisibleLines;

        float                       m_fUpTime;
        float                       m_fFadeTime;
        float                       m_fBaseTime;

        unsigned int                m_uiIndent;
        bool                        m_bActive;
};
