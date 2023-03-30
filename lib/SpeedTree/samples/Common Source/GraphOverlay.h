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

///////////////////////////////////////////////////////////////////////  
//  Include files

#include <string>
#include <vector>
#include "IdvFont.h"


const int c_nNumPoints = 200;

///////////////////////////////////////////////////////////////////////  
//  class CGraphOverlay declaration

class CGraphOverlay
{
public:
        CGraphOverlay( );
        ~CGraphOverlay( );

        void                    AddDataPoint(unsigned int uiDataIndex, float fValue);
        void                    AddDataType(std::string strName, std::string strUnits);
        void                    Draw(void);
        void                    InitGraphics(void);
        void                    ResizeEvent(void);

        // mode
        unsigned int            GetDataType(void) const                 { return m_nActiveDataType; }
        unsigned int            GetNumDataTypes(void) const             { return m_vDataTypes.size( ); }
        void                    SetDataType(unsigned int uiDataIndex)   { m_nActiveDataType = uiDataIndex; }


private:
        void                    DrawBackground(void) const;
        void                    DrawGraph(void);
        void                    FindLines(float fMin, float fMax, char* szMin, char* szMax, float& fIncrement, float& fFirst);
        void                    FillStrings(float fMin, float fMax, char* szMin, char* szMax);

        struct SDataType
        {
            std::string         m_strName;
            std::string         m_strUnits;
            std::vector<float>  m_vValues;
        };

        int                     m_nActiveDataType;
        std::vector<SDataType>  m_vDataTypes;
        CIdvFont                m_cFont;

        // window related
        unsigned int            m_uiGraphLeft;
        unsigned int            m_uiGraphRight;
        unsigned int            m_uiGraphTop;
        unsigned int            m_uiGraphBottom;
        unsigned int            m_uiGraphWidth;
        unsigned int            m_uiGraphHeight;
        unsigned int            m_uiWindowWidth;
        unsigned int            m_uiWindowHeight;


#ifdef IGNORE_THIS
        void            Draw(EDrawMode eMode);
        float           GetDrawTime(void);
        void            SetStatusString(std::string strString);

private:
//      void            DrawBox(CRegion& cBox);

        void            PreDraw(void);
        void            PostDraw(void);

        void            DrawFrameRate(bool bDrawBox);
        void            DrawHistory(void);
        void            DrawStatus(void);


        bool            m_bFirst;
//      IdvTmpString    m_szReportBuffer;
        std::string     m_strStatusString;
//      CIdvBitmapText  m_glstrString;
        float           m_fStringHeight, m_fStringWidth;

        float           m_fBorder;
        float           m_fHeight;
        int             m_nIteration;
//      CTransform      m_cProjectionMatrix;
        float           m_fWidth;
#endif
};
