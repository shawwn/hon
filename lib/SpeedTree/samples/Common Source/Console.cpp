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

#pragma warning (disable: 4786 4244)
#include "Console.h"
#include "extgl.h"
#include <GL/glu.h>
#include <ctime>
using namespace std;

const float c_fTabSpacing = 60.0f;
const int c_nConsoleBorder = 8;
const int c_nMaxWords = 2000;

///////////////////////////////////////////////////////////////////////  
//  CConsole::CConsole

CConsole::CConsole( ) :
    m_pfMessageCallback(NULL),
    m_nScroll(0),
    m_fUpTime(10000000.0f),
    m_fFadeTime(0.0f),
    m_fBaseTime(0.0f),
    m_uiIndent(0),
    m_bActive(true)
{
    m_afCurrentColor[0] = m_afCurrentColor[1] = m_afCurrentColor[2] = 1.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::~CConsole

CConsole::~CConsole( )
{
}

///////////////////////////////////////////////////////////////////////  
//  IsTerminator

inline bool IsTerminator(char chChar)
{
    return (isspace(chChar) || chChar == '\b');
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::Print

void CConsole::Print(const char* pField, ...)
{
    va_list vlArgs;
    char szVargsBuffer[1024], szTmp[1024];
    
    va_start(vlArgs, pField);
    vsprintf(szTmp, pField, vlArgs);

    for (unsigned int i = 0; i < m_uiIndent; ++i)
        szVargsBuffer[i] = '\t';
    szVargsBuffer[m_uiIndent] = '\0';
    strcat(szVargsBuffer, szTmp);

    if (strlen(szVargsBuffer) > 0)
    {
        int nLength = strlen(szVargsBuffer);
        int i = 0;
        while (i < nLength)
        {
            bool bWhitespaceWord = IsTerminator(szVargsBuffer[i]);

            char szWord[1024];
            if (bWhitespaceWord)
            {
                // scan until non-terminator found
                int nStart = i;
                while (i < nLength && IsTerminator(szVargsBuffer[i]))
                    i++;

                // add this whitespace word
                strncpy(szWord, szVargsBuffer + nStart, i - nStart);
                szWord[i - nStart] = '\0';
            }
            else
            {
                // scan until terminator found
                int nStart = i;
                while (i < nLength && !IsTerminator(szVargsBuffer[i]))
                    i++;

                // add this word
                strncpy(szWord, szVargsBuffer + nStart, i - nStart);
                szWord[i - nStart] = '\0';
            }

            m_vMessageBuffer.push_back(SStringEntry(szWord, m_afCurrentColor));
            if (m_vMessageBuffer.size( ) > c_nMaxWords)
                m_vMessageBuffer.erase(m_vMessageBuffer.begin( ));
        }

        // text has changed, build new lines
        DetermineLines( );

        if (m_pfMessageCallback)
            (*m_pfMessageCallback) ( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::PrintError

void CConsole::PrintError(const char* pField, ...)
{
    va_list vlArgs;
    char szVargsBuffer[1024];
    
    va_start(vlArgs, pField);
    vsprintf(szVargsBuffer, pField, vlArgs);

    // store previous value
    float afTmpColor[3];
    memcpy(afTmpColor, m_afCurrentColor, 3 * sizeof(float));

    m_afCurrentColor[0] = 1.0f;
    m_afCurrentColor[1] = 0.0f;
    m_afCurrentColor[2] = 0.0f;

    Print(szVargsBuffer);

    // restore color value
    memcpy(m_afCurrentColor, afTmpColor, 3 * sizeof(float));
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::Draw

void CConsole::Draw(void)
{
    if (!m_bActive)
        return;

    float fAlpha = ProcessTimeout( );
    if (fAlpha > 0.0f)
    {
        glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        DrawBackground(fAlpha);
        m_cFont.StartFlatProjection( );

        glColor4f(1.0f, 1.0f, 1.0f, fAlpha);

        // query attributes of normal face and bold fonts
        m_cFont.SetActiveFont("verdana bold");
        const CIdvFont::SAttrib& sBoldAttrib = m_cFont.GetFontAttributes( );
        m_cFont.SetActiveFont("verdana");
        const CIdvFont::SAttrib& sFontAttrib = m_cFont.GetFontAttributes( );

        const float c_fFontHeight = m_cFont.GetHeight( );
        const CIdvFont::SAttrib* pCurrentAttrib = &sFontAttrib;

        float x = m_uiConsoleLeft + c_nConsoleBorder;
        float y = m_uiConsoleBottom - 2 * c_fFontHeight;
        glTranslatef(x, y, 0.0f);

        bool bBoldFace = false; // what if there's a line feed between bolds and the
                                // first line printed is already bold?
        for (int nLine = m_vLines.size( ) - 1 - m_nScroll; (m_vLines.size( ) - nLine - 1 - m_nScroll) < m_uiNumVisibleLines && nLine > 0; --nLine)
        {
            float fPrevX = x;
            for (unsigned int nWord = 0; nWord < m_vLines[nLine].size( ); ++nWord)
            {
                glColor4f(m_vLines[nLine][nWord].m_afColor[0],
                          m_vLines[nLine][nWord].m_afColor[1],
                          m_vLines[nLine][nWord].m_afColor[2],
                          fAlpha);
                for (unsigned int nChar = 0; nChar < m_vLines[nLine][nWord].m_strString.length( ); ++nChar)
                {
                    float fAdvance = 0.0f;
                    char chChar = m_vLines[nLine][nWord].m_strString[nChar];

                    // handle tabs
                    if (chChar == '\t')
                    {
                        fAdvance = TabOffset(x - m_uiConsoleLeft - c_nConsoleBorder);
                        glTranslatef(fAdvance, 0.0f, 0.0f);
                    }
                    // toggling between bold and normal face
                    else if (chChar == '\b')
                    {
                        bBoldFace = !bBoldFace;
                        pCurrentAttrib = bBoldFace ? &sBoldAttrib : &sFontAttrib;
                        m_cFont.SetActiveFont(bBoldFace ? "verdana bold" : "verdana");
                    }
                    // all remaining characters
                    else
                        fAdvance = pCurrentAttrib->m_asChars[chChar].m_fWidth;

                    m_cFont.Draw(chChar);
                    x += fAdvance;
                }
            }

            y -= c_fFontHeight;
            glTranslatef(fPrevX - x, -c_fFontHeight, 0.0f);
            x = fPrevX;
        }

        m_cFont.EndFlatProjection( );

        glPopAttrib( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::InitGraphics

void CConsole::InitGraphics(void)
{
    const char* c_pBoldFont = "../../data/fonts/verdana_bold.fnt";
    if (m_cFont.LoadFont("verdana bold", c_pBoldFont))
        m_cFont.SetActiveFont("verdana bold");
    else
        fprintf(stderr, "failed to load font [%s]\n", c_pBoldFont);

    const char* c_pNormalFont = "../../data/fonts/verdana.fnt";
    if (m_cFont.LoadFont("verdana", c_pNormalFont))
        m_cFont.SetActiveFont("verdana");
    else
        fprintf(stderr, "failed to load font [%s]\n", c_pNormalFont);

    ResizeEvent( );
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::ResizeEvent

void CConsole::ResizeEvent(void)
{
    const float c_fConsoleWidthRatio = 0.9f;
    const float c_fConsoleHeightRatio = 0.6f;
    const float c_fConsoleLeftRatio = 0.5f - 0.5f * c_fConsoleWidthRatio;
    const float c_fConsoleRightRatio = 0.5f + 0.5f * c_fConsoleWidthRatio;
    const float c_fConsoleBottomRatio = 0.99f;
    const float c_fConsoleTopRatio = c_fConsoleBottomRatio - c_fConsoleHeightRatio;

    GLint anViewport[4];
    glGetIntegerv(GL_VIEWPORT, anViewport);
    m_uiWindowWidth = anViewport[2];
    m_uiWindowHeight = anViewport[3];

    m_uiConsoleWidth = m_uiWindowWidth * c_fConsoleWidthRatio;
    m_uiConsoleHeight = m_uiWindowHeight * c_fConsoleHeightRatio;
    m_uiConsoleLeft = m_uiWindowWidth * c_fConsoleLeftRatio;
    m_uiConsoleRight = m_uiWindowWidth * c_fConsoleRightRatio;
    m_uiConsoleTop = m_uiWindowHeight * c_fConsoleTopRatio;
    m_uiConsoleBottom = m_uiWindowHeight * c_fConsoleBottomRatio;

    const float c_fFontHeight = m_cFont.GetActiveFont( ).empty( ) ? 14: m_cFont.GetFontAttributes( ).m_uiFontHeight;

    m_uiNumVisibleLines = m_uiConsoleHeight / c_fFontHeight - 1;

    // text has changed, build new lines
    DetermineLines( );

    m_cFont.ResizeEvent( );
    m_nScroll = 0;
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::SetColor

void CConsole::SetColor(const float* pColor)
{
    memcpy(m_afCurrentColor, pColor, 3 * sizeof(float));
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::SetColor

void CConsole::SetColor(float fRed, float fGreen, float fBlue)
{
    m_afCurrentColor[0] = fRed;
    m_afCurrentColor[1] = fGreen;
    m_afCurrentColor[2] = fBlue;
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::SetMessageCallback

void CConsole::SetMessageCallback(void (*pfCallback) (void))
{
    m_pfMessageCallback = pfCallback;
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::SetTimeout

void CConsole::SetTimeout(float fUpTime, float fFadeTime)
{
    m_bActive = true;
    m_fUpTime = fUpTime;
    m_fFadeTime = fFadeTime;
    m_fBaseTime = clock( ) / float(CLOCKS_PER_SEC);
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::DrawBackground

void CConsole::DrawBackground(float fAlpha) const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity( );
    gluOrtho2D(0.0f, m_uiWindowWidth, m_uiWindowHeight, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );

    glColor4f(0.0f, 0.0f, 0.0f, fAlpha * 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(m_uiConsoleRight, m_uiConsoleTop);
        glVertex2f(m_uiConsoleLeft, m_uiConsoleTop);
        glVertex2f(m_uiConsoleLeft, m_uiConsoleBottom);
        glVertex2f(m_uiConsoleRight, m_uiConsoleBottom);
    glEnd( );

    glColor4f(0.0f, 0.0f, 0.0f, fAlpha);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(m_uiConsoleRight, m_uiConsoleTop);
        glVertex2f(m_uiConsoleLeft, m_uiConsoleTop);
        glVertex2f(m_uiConsoleLeft, m_uiConsoleBottom);
        glVertex2f(m_uiConsoleRight, m_uiConsoleBottom);
    glEnd( );

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::ProcessTimeout

float CConsole::ProcessTimeout(void)
{
    float fAlpha = 1.0f;

    float fDifference = (clock( ) / float(CLOCKS_PER_SEC)) - m_fBaseTime;
    if (fDifference > m_fUpTime)
    {
        if (fDifference < m_fUpTime + m_fFadeTime)
            fAlpha = 1.0f - ((fDifference - m_fUpTime) / m_fFadeTime);
        else
        {
            fAlpha = 0.0f;
            m_bActive = false;
        }
    }

    return fAlpha;
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::DetermineLines

void CConsole::DetermineLines(void)
{
    if (!m_cFont.ValidFont( ))
        return;

    m_vLines.clear( );

    // query attributes of normal face and bold fonts
    m_cFont.SetActiveFont("verdana");
    const CIdvFont::SAttrib& sFontAttrib = m_cFont.GetFontAttributes( );
    m_cFont.SetActiveFont("verdana bold");
    const CIdvFont::SAttrib& sBoldAttrib = m_cFont.GetFontAttributes( );

    bool bBoldFace = false;
    CIdvFont::SAttrib sCurrentAttrib = sFontAttrib;

    const float c_fConsoleWidth = m_uiConsoleWidth;

    // run through every message
    if (!m_vMessageBuffer.empty( ))
    {
        const float* pCurrentColor = m_vMessageBuffer[0].m_afColor;

        float x = m_uiConsoleLeft + c_nConsoleBorder;
        vector<SStringEntry> vSingleLine;
        string strLine;
        for (vector<SStringEntry>::iterator i = m_vMessageBuffer.begin( ); i != m_vMessageBuffer.end( ); ++i)
        {
            // if color has changed, store line and make a new one
            if (pCurrentColor[0] != i->m_afColor[0] ||
                pCurrentColor[1] != i->m_afColor[1] ||
                pCurrentColor[2] != i->m_afColor[2])
            {
                vSingleLine.push_back(SStringEntry(strLine, pCurrentColor));
                pCurrentColor = i->m_afColor;
                strLine = "";
            }

            // run through every character of each message
            int nLength = i->m_strString.length( );
            for (int j = 0; j < nLength; ++j)
            {
                float fAdvance = 0.0f;

                char chChar = i->m_strString[j];

                // handle tabs
                if (chChar == '\t')
                {
                    fAdvance = TabOffset(x);
                    strLine += chChar;
                }
                // toggling between bold and normal face
                else if (chChar == '\b')
                {
                    bBoldFace = !bBoldFace;
                    sCurrentAttrib = bBoldFace ? sBoldAttrib : sFontAttrib;
                    strLine += chChar;
                }
                // carriage returns
                else if (chChar == '\n')
                {
                    // store current string
                    vSingleLine.push_back(SStringEntry(strLine, pCurrentColor));
                    m_vLines.push_back(vSingleLine);
                    vSingleLine.clear( );
                    strLine = "";
                    x = 0.0f;
                }
                // all remaining characters
                else
                {
                    fAdvance = sFontAttrib.m_asChars[chChar].m_fWidth;
                    strLine += chChar;
                }

                // advance x and wrap when necessary
                if (fAdvance > 0.0f)
                {
                    // will word cause it to wrap?
                    if (x + fAdvance >= c_fConsoleWidth - c_nConsoleBorder)
                    {
                        x = 0.0f;
                        vSingleLine.push_back(SStringEntry(strLine, pCurrentColor));
                        strLine = "";
                    }
                    else
                    {
                        x += fAdvance;
                    }
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CConsole::TabOffset

float CConsole::TabOffset(float fCurrentPos) const
{
    float fDistFromLeftMargin = fCurrentPos;
    int nTabColumn = fDistFromLeftMargin / c_fTabSpacing;
    
    int nNextTabColumn = nTabColumn + 1;
    float fNextDistFromLeft = nNextTabColumn * c_fTabSpacing;

    return fNextDistFromLeft - fDistFromLeftMargin;
}

