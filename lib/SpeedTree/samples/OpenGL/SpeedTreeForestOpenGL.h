///////////////////////////////////////////////////////////////////////  
//  CSpeedTreeForestOpenGL Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is provided to illustrate one way to incorporate
//  SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
//  calls that must be made on a per tree basis are done by this class.
//  Calls that apply to all trees (i.e. static SpeedTreeRT functions)
//  are made in the functions in main.cpp.
//
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


///////////////////////////////////////////////////////////////////////  
//  Include Files

#include "SpeedTreeForest.h"
#include "VertexProgram.h"


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeForestOpenGL declaration

class CSpeedTreeForestOpenGL : public CSpeedTreeForest
{
public:
        CSpeedTreeForestOpenGL( );
        ~CSpeedTreeForestOpenGL( );

        void                UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const;
        bool                InitGraphics(void);
        void                Render(unsigned long ulRenderBitVector) const;

private:
        bool                InitVertexShaders(void);

        CVertexProgram      m_cBranchVertexProgram; // IDV utility class for abstracting branch vertex program mechanics
        CVertexProgram      m_cLeafVertexProgram;   // IDV utility class for abstracting leaf vertex program mechanics
        unsigned int        m_uiCompositeMap;       // texture index for composite map holding leaf, frond, and billboard images
};