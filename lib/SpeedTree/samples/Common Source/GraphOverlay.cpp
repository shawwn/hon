///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay Class
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

#include "GraphOverlay.h"
#include <cmath>
using namespace std;

#pragma warning (disable : 4244)

const int c_nGraphBorderHorz = 4;
const int c_nGraphBorderVert = 12;
const int c_nGraphTextSpace = 60.0f;


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::CGraphOverlay

CGraphOverlay::CGraphOverlay( ) :
    m_nActiveDataType(-1),
    m_uiWindowWidth(0),
    m_uiWindowHeight(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::~CGraphOverlay

CGraphOverlay::~CGraphOverlay( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::AddDataPoint

void CGraphOverlay::AddDataPoint(unsigned int uiDataIndex, float fValue)
{
    if (uiDataIndex < m_vDataTypes.size( ))
    {
        vector<float>& vValues = m_vDataTypes[uiDataIndex].m_vValues;

        vValues.push_back(fValue);
        if (vValues.size( ) > c_nNumPoints)
            vValues.erase(vValues.begin( ));
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::AddDataType

void CGraphOverlay::AddDataType(string strName, string strUnits)
{
    SDataType sDataType;

    sDataType.m_strName = strName;
    sDataType.m_strUnits = strUnits;
    m_vDataTypes.push_back(sDataType);
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::Draw

void CGraphOverlay::Draw(void)
{
    glPushAttrib(GL_TRANSFORM_BIT | GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    DrawBackground( );
    DrawGraph( );

    glPopAttrib( );
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::InitGraphics

void CGraphOverlay::InitGraphics(void)
{
    const char* c_pNormal = "../../data/Fonts/verdana.fnt";
    const char* c_pBold = "../../data/Fonts/verdana_bold.fnt";

    if (m_cFont.LoadFont("verdana", c_pNormal))
        m_cFont.SetActiveFont("verdana");
    else
        fprintf(stderr, "Error loading font [%s]\n", c_pNormal);

    if (m_cFont.LoadFont("verdana bold", c_pBold))
        m_cFont.SetActiveFont("verdana bold");
    else
        fprintf(stderr, "Error loading font [%s]\n", c_pBold);
    ResizeEvent( );
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::ResizeEvent

void CGraphOverlay::ResizeEvent(void)
{
    const float c_fGraphWidthRatio = 0.95f;
    const float c_fGraphHeightRatio = 0.15f;
    const float c_fGraphLeftRatio = 0.025f;
    const float c_fGraphRightRatio = c_fGraphLeftRatio + c_fGraphWidthRatio;
    const float c_fGraphTopRatio = 0.05f;
    const float c_fGraphBottomRatio = c_fGraphTopRatio + c_fGraphHeightRatio;

    GLint anViewport[4];
    glGetIntegerv(GL_VIEWPORT, anViewport);
    m_uiWindowWidth = anViewport[2];
    m_uiWindowHeight = anViewport[3];

    m_uiGraphWidth = m_uiWindowWidth * c_fGraphWidthRatio;
    m_uiGraphHeight = m_uiWindowHeight * c_fGraphHeightRatio;
    m_uiGraphLeft = m_uiWindowWidth * c_fGraphLeftRatio;
    m_uiGraphRight = m_uiWindowWidth * c_fGraphRightRatio;
    m_uiGraphTop = m_uiWindowHeight * c_fGraphTopRatio;
    m_uiGraphBottom = m_uiWindowHeight * c_fGraphBottomRatio;
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawBackground

void CGraphOverlay::DrawBackground(void) const
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity( );
    gluOrtho2D(0.0f, m_uiWindowWidth, m_uiWindowHeight, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );

    glColor4f(0.0f, 0.0f, 0.0f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(m_uiGraphRight, m_uiGraphTop);
        glVertex2f(m_uiGraphLeft, m_uiGraphTop);
        glVertex2f(m_uiGraphLeft, m_uiGraphBottom);
        glVertex2f(m_uiGraphRight, m_uiGraphBottom);
    glEnd( );

    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(m_uiGraphRight, m_uiGraphTop);
        glVertex2f(m_uiGraphLeft, m_uiGraphTop);
        glVertex2f(m_uiGraphLeft, m_uiGraphBottom);
        glVertex2f(m_uiGraphRight, m_uiGraphBottom);
    glEnd( );

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
}


///////////////////////////////////////////////////////////////////////  
//  Interpolate

static inline float Interpolate(float fStart, float fEnd, float fPercent)
{
    return fStart + (fEnd - fStart) * fPercent;
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawGraph

void CGraphOverlay::DrawGraph(void)
{
    // find maximum value (assume min is always 0.0)
    float fMax = 0.0f;
    vector<float>& vData = m_vDataTypes[m_nActiveDataType].m_vValues;
    for (unsigned int i = 0; i < vData.size( ); ++i)
        if (vData[i] > fMax)
            fMax = vData[i];

    // determine appropriate graph lines
    char szMin[256], szMax[256];
    FillStrings(0.0f, fMax, szMin, szMax);

    float fIncrement = 1.0f, fFirst = 0.0f;
    FindLines(0.0f, fMax, szMin, szMax, fIncrement, fFirst);

    // draw lines
    m_cFont.SetActiveFont("verdana");
    float fCurrent = fFirst;
    float fFontHeight = m_cFont.GetHeight( );
    while (fCurrent <= fMax)
    {
        glDisable(GL_TEXTURE_2D);
        float fValue = Interpolate(m_uiGraphBottom - c_nGraphBorderVert, m_uiGraphTop + c_nGraphBorderVert, fCurrent / fMax);
        glColor3f(0.4f, 0.4f, 0.4f);
        glBegin(GL_LINES);
            glVertex2f(m_uiGraphLeft + c_nGraphBorderHorz, fValue);
            glVertex2f(m_uiGraphRight - c_nGraphBorderHorz - c_nGraphTextSpace, fValue);
        glEnd( );

        char szCurrentString[256];
        sprintf(szCurrentString, "%0.1f", fCurrent);


        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix( );
            glTranslatef(int(m_uiGraphRight - c_nGraphTextSpace), int(fValue - fFontHeight / 2.0f - 1), 0.0f);
            m_cFont.Draw(szCurrentString);
        glPopMatrix( );

        fCurrent += fIncrement;
    }

    // draw data line
    glDisable(GL_TEXTURE_2D);

    float fPointWidth = (m_uiGraphWidth - 2 * c_nGraphBorderHorz - c_nGraphTextSpace) / float(c_nNumPoints);
    float fValueScalar = (m_uiGraphHeight - 2 * c_nGraphBorderVert) / fMax;
    float x = m_uiGraphRight - c_nGraphBorderHorz - c_nGraphTextSpace - vData.size( ) * fPointWidth;

    glColor3f(0.5f, 0.5f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < vData.size( ); ++i)
    {
        glVertex2f(x, m_uiGraphBottom - c_nGraphBorderVert - vData[i] * fValueScalar);
        x += fPointWidth;
    }
    glEnd( );
    glDisable(GL_LINE_SMOOTH);

    // draw label
    glEnable(GL_TEXTURE_2D);
    m_cFont.SetActiveFont("verdana");

    char szBuf[256];
    if (!m_vDataTypes[m_nActiveDataType].m_strUnits.empty( ))
        sprintf(szBuf, "%s (%s)", m_vDataTypes[m_nActiveDataType].m_strName.c_str( ), m_vDataTypes[m_nActiveDataType].m_strUnits.c_str( ));
    else
        sprintf(szBuf, "%s", m_vDataTypes[m_nActiveDataType].m_strName.c_str( ));

    glPushMatrix( );
        glTranslatef(m_uiGraphLeft + 3.0f, m_uiGraphTop + 3.0f, 0.0f);
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
        m_cFont.Draw(szBuf);
    glPopMatrix( );
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::FillStrings

void CGraphOverlay::FillStrings(float fMin, float fMax, char* szMin, char* szMax)
{
    char szMinTemp[256], szMaxTemp[256];

    if (fMin < 0.0f)
    {
        sprintf(szMinTemp, "%f", 0.0f);
        sprintf(szMaxTemp, "%f", fMax - fMin);
    }
    else
    {
        sprintf(szMinTemp, "%f", fMin);
        sprintf(szMaxTemp, "%f", fMax);
    }

    int nIndex = 0, nMinCount = 0, nMaxCount = 0;

    while (szMinTemp[nIndex] != '.' && szMinTemp[nIndex] != '\0')
    {
        nMinCount++;
        nIndex++;
    }
    nIndex = 0;
    while (szMaxTemp[nIndex] != '.' && szMaxTemp[nIndex] != '\0')
    {
        nMaxCount++;
        nIndex++;
    }

    char szPadBuffer[2000];
    int nPad = (int) fabs(float(nMinCount - nMaxCount));
    szPadBuffer[0] = '\0';
    for (int i = 0; i < nPad; i++)
        strcat(szPadBuffer, "0");

    if (nMaxCount > nMinCount)
    {
        strcpy(szMax, szMaxTemp);
        sprintf(szMin, "%s%s", szPadBuffer, szMinTemp);
    }
    else if (nMaxCount < nMinCount)
    {
        strcpy(szMin, szMinTemp);
        sprintf(szMax, "%s%s", szPadBuffer, szMaxTemp);
    }
    else
    {
        strcpy(szMin, szMinTemp);
        strcpy(szMax, szMaxTemp);
    }

    if (strlen(szMin) < strlen(szMax))
    {
        while (strlen(szMin) < strlen(szMax))
            strcat(szMin, "0");
    }
    else if (strlen(szMax) < strlen(szMin))
    {
        while (strlen(szMax) < strlen(szMin))
            strcat(szMax, "0");
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::FindLines

void CGraphOverlay::FindLines(float fMin, float fMax, char* szMin, char* szMax, float& fIncrement, float& fFirst)
{
    int     nIndex = 0, nDiffer = -1, nDecimal = -1;
    float   fDivision;

    while (szMin[nIndex] != '\0' && szMax[nIndex] != '\0')
    {
        if (szMin[nIndex] == '.' || szMax[nIndex] == '.')
        {
            nDecimal = nIndex;
            szMin[nIndex] = '.';
            szMax[nIndex] = '.';
        }
        if (szMin[nIndex] != szMax[nIndex] && nDiffer == -1)
            nDiffer = nIndex;
        nIndex++;
    }

    if (nDecimal == -1)
        nDecimal = strlen(szMin);

    if (nDiffer == -1)
        nDiffer = nDecimal + 1;

    if (nDiffer < nDecimal)
        fDivision = powf(10.0f, float(nDecimal - nDiffer - 1));
    else
        fDivision = powf(10.0f, float(nDecimal - nDiffer));

    float fDelta = fMax - fMin, fCurrent, fLast, fTarget = 4.0f, fNewDivision;
    fLast = fDelta / fDivision;
    if (fLast > fTarget)
    {
        if (fDelta / (fDivision * 2.0f) < fTarget)
        {
            fNewDivision = fDivision * 2.0f;
            fCurrent = fDelta / fNewDivision;
        }
        else if (fDelta / (fDivision * 5.0f) < fTarget)
        {
            fNewDivision = fDivision * 5.0f;
            fCurrent = fDelta / fNewDivision;
            fDivision *= 2.0f; 
            fLast = fDelta / fDivision;
        }
        else
        {
            fNewDivision = fDivision * 10.0f;
            fCurrent = fDelta / fNewDivision;
            fDivision *= 5.0f; 
            fLast = fDelta / fDivision;
        }
    }
    else
    {
        if ((fDelta / (fDivision / 2.0f)) > fTarget)
        {
            fNewDivision = fDivision / 2.0f;
            fCurrent = fDelta / fNewDivision;
        }
        else if ((fDelta / (fDivision / 5.0f)) > fTarget)
        {
            fNewDivision = fDivision / 5.0f;
            fCurrent = fDelta / fNewDivision;
            fDivision /= 2.0f; 
            fLast = fDelta / fDivision;
        }
        else
        {
            fNewDivision = fDivision / 10.0f;
            fCurrent = fDelta / fNewDivision;
            fDivision /= 5.0f; 
            fLast = fDelta / fDivision;
        }
    }

    if (fabs(fCurrent - fTarget) < fabs(fLast - fTarget))
        fIncrement = fNewDivision;
    else
        fIncrement = fDivision;

    fFirst = fIncrement * float(int(fMin / fIncrement));
}


#ifdef IGNORE_THIS

///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::SetStatusString

void CGraphOverlay::SetStatusString(string strString)
{
    m_strStatusString = strString;
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::Draw

void CGraphOverlay::Draw(EDrawMode eMode)
{
    bool bBlend = glIsEnabled(GL_BLEND);
    bool bAlphaTest = glIsEnabled(GL_ALPHA_TEST);
    if (!bBlend)
    {
        glEnable(GL_BLEND);
    }
    if (bAlphaTest)
    {
        glDisable(GL_ALPHA_TEST);
    }
    if (eMode != DRAW_MODE_NONE)
    {
        PreDraw( );
        switch (eMode)
        {
        case DRAW_MODE_STATUS:
            DrawStatus( );
            break;
        case DRAW_MODE_FRAME_RATE:
            DrawFrameRate(true);
            break;
        case DRAW_MODE_HISTORY:
            DrawHistory( );
            break;
        default:
            throw(IdvSystemError("CGraphOverlay::Draw - unknown case in switch."));
        }
        PostDraw( );
    }
    if (!bBlend)
    {
        glDisable(GL_BLEND);
    }
    if (bAlphaTest)
    {
        glEnable(GL_ALPHA_TEST);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::GetDrawTime

float CGraphOverlay::GetDrawTime(void)
{
    float fFrameRate = GetFrameRate( );

    if (fFrameRate != 0.0)
        return 1.0f / fFrameRate;
    else
        return 0.030f;
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawBox

void CGraphOverlay::DrawBox(CRegion& cBox)
{
    glColor4f(0.2f, 0.2f, 0.2f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(cBox.m_cMin.m_afData[0], cBox.m_cMax.m_afData[1]);
        glVertex2fv(cBox.m_cMax.m_afData);
        glVertex2f(cBox.m_cMax.m_afData[0], cBox.m_cMin.m_afData[1]);
        glVertex2fv(cBox.m_cMin.m_afData);
    glEnd( );
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::PreDraw

void CGraphOverlay::PreDraw(void)
{
    if (m_bFirst)
    {
        int anViewport[4];
        glGetIntegerv(GL_VIEWPORT, anViewport);

        if (anViewport[3] == 1200)
            m_glstrString.SetSize(10.0f);
        else if (anViewport[3] == 1024)
            m_glstrString.SetSize(9.0f);
        else if (anViewport[3] == 768)
            m_glstrString.SetSize(8.0f);
        else if (anViewport[3] == 600)
            m_glstrString.SetSize(7.0f);
        else if (anViewport[3] == 480)
            m_glstrString.SetSize(6.0f);
        else
            m_glstrString.SetSize(8.0f);

        m_glstrString.SetColor(0.8f, 0.8f, 0.8f, 1.0f);
        m_glstrString.SetAlignType(CIdvGLTextBase::TopLeft);
        m_bFirst = false;
    }

    /*
    float fSize = 10.0f;
        
    if (IdvIsRendering( ))
    {
        int nRenderWidth, nRenderHeight;
        IdvGetRenderSize(nRenderWidth, nRenderHeight);
        if (nRenderHeight > 1024)
        {
            fSize = float(nRenderHeight) / 102.4f;
            m_glstrString.SetSize(fSize);
            int nMin = min(nRenderWidth, nRenderHeight);
            glLineWidth(1 + nMin / 1000);
        }
    //  glEnable(GL_LINE_SMOOTH);
    //  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //  glEnable(GL_BLEND);
    }


    glPushAttrib(GL_TRANSFORM_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix( ); 

    //glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*) m_cProjectionMatrix.m_afData);

    int pViewport[4];
    //glGetIntegerv(GL_VIEWPORT, pViewport);
    IdvGetViewport(pViewport[0], pViewport[1], pViewport[2], pViewport[3]);

    m_fWidth = float(pViewport[2]);
    m_fHeight = float(pViewport[3]);
    m_fBorder = (m_fWidth > m_fHeight) ? 0.02f * m_fHeight : 0.02f * m_fWidth;
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

//  glMatrixMode(GL_PROJECTION);
    glLoadIdentity( );
    IdvOrtho(0.0f, m_fWidth, 0.0, m_fHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix( );
    glLoadIdentity( );
    */
} 


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::PostDraw

void CGraphOverlay::PostDraw(void)
{
    /*
    if (IdvIsRendering( ))
    {
        m_glstrString.SetSize(10.0f);
        glLineWidth(1.0f);
    }
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix( );
    glMatrixMode(GL_PROJECTION);
    glPopMatrix( );
//  glLoadMatrixf((GLfloat*) m_cProjectionMatrix.m_afData);
    glPopAttrib( );
    */
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawFrameRate

void CGraphOverlay::DrawFrameRate(bool bDrawBox)
{
    m_nIteration++;
    vector<float> vHistory = GetDrawHistory( );
    m_glstrString.SetAlignType(CIdvGLTextBase::BottomLeft);
//  m_glstrString.SetSize(10.0f);

    if (m_nIteration == nHistorySize)
    {
        sprintf(m_szReportBuffer, "Frame rate: %.1f Hz, %3.1f ms", GetFrameRate( ), vHistory[vHistory.size( ) - 1]);
        CRegion cTemp = m_glstrString.GetExtents(string(m_szReportBuffer));
        m_fStringHeight = cTemp.m_cMax[1] - cTemp.m_cMin[1];
        m_fStringWidth = cTemp.m_cMax[0] - cTemp.m_cMin[0];
        m_nIteration = 0;
    }

    if (m_fStringWidth > -1.0f)
    {
        CRegion cBox;
        cBox.m_cMin.Set(5.0f, m_fHeight - 5.0f);
        cBox.m_cMax.Set(5.0f + m_fStringWidth * 1.03f, m_fHeight - 8.0f - m_fStringHeight);

        if (bDrawBox)
            DrawBox(cBox);
    
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        m_glstrString.Draw(string(m_szReportBuffer), cBox.m_cMin[0] + 1.0f, cBox.m_cMax[1] + 2.0f);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawHistory

void CGraphOverlay::DrawHistory(void)
{
    CRegion cBox;
    cBox.m_cMin.Set(396 / 1600.0f, 1196 / 1200.0f);
    cBox.m_cMax.Set(992 / 1600.0f, 1070 / 1200.0f);
    DrawBox(cBox);

    cBox.m_cMin[1] -= 5 / 1200.0f;
    cBox.m_cMax[1] += 15 / 1200.0f;

    m_glstrString.SetAlignType(CIdvGLTextBase::MidLeft);

    float fMax = 0.0f;
    vector<float> vHistory = GetSwapHistory( );
    int nCount = vHistory.size( );
    if (nCount >= 2)
    {
        for (int i = 0; i < nCount; i++)
        {
            if (vHistory[i] != 0.0f)
                vHistory[i] = 1.0f / vHistory[i];
            if (vHistory[i] > fMax)
                fMax = vHistory[i];
        }

        float fTextOffSet = 70.0f / 1600.0f;

        //
        //  Draw legend
        //
        IdvTmpString szMin;
        IdvTmpString szMax;
        FillStrings(0.0f, fMax, szMin, szMax);

        float fIncrement;
        float fFirst;
        FindLines(0.0f, fMax, szMin, szMax, fIncrement, fFirst);
 
        float fCurrent = fFirst;
        while (fCurrent <= fMax)
        {
            float fValue = VEC_INTERPOLATE(cBox.m_cMax.m_afData[1], cBox.m_cMin.m_afData[1], fCurrent / fMax);
            glColor3f(0.4f, 0.4f, 0.4f);
            glBegin(GL_LINES);
                glVertex2f(cBox.m_cMin.m_afData[0], fValue);
                glVertex2f(cBox.m_cMax.m_afData[0] - fTextOffSet, fValue);
            glEnd( );
            IdvTmpString szCurrentString;
            sprintf(szCurrentString, "%3.0f Hz", fCurrent);
            glColor3f(1.0f, 1.0f, 1.0f);
            m_glstrString.Draw(string(szCurrentString), cBox.m_cMax.m_afData[0] - 0.85f * fTextOffSet, fValue);
            fCurrent += fIncrement;
        }

        //
        //  Draw graph line
        //
        glLineWidth(2.0f);
        float fStep = (cBox.Dimension(0) - fTextOffSet) / float(nCount - 1);
        float fPos = cBox.m_cMin.m_afData[0];

        glColor3f(0.75f, 0.75f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (i = 0; i < nCount; i++)
        {
            float fValue = VEC_INTERPOLATE(cBox.m_cMax.m_afData[1], cBox.m_cMin.m_afData[1], vHistory[i] / fMax);
            if (vHistory[i] > fMax)
                fMax = vHistory[i];
            glVertex2f(fPos, fValue);
            fPos += fStep;
        }
        glEnd( );
        glLineWidth(1.0f);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGraphOverlay::DrawStatus

void CGraphOverlay::DrawStatus(void)
{
//  m_glstrString.SetAlignment(COGLTString::BOTTOM_LEFT);
//  m_glstrString.SetSize(10.0f);
//  m_glstrString.GetExtents(m_strStatusString, m_fStringWidth, m_fStringHeight);

    if (m_fStringWidth > -1.0f)
    {
        CRegion cBox;
        cBox.m_cMin.Set(0.0f, 2.0f + m_fStringHeight + 2.0f);
        cBox.m_cMax.Set(m_fWidth, 0.0f);

        DrawBox(cBox);
//      m_glstrString.Draw(cBox.m_cMin[0] + 5.0f, cBox.m_cMax[1] + 3.0f, m_strStatusString);
    }
}


#endif