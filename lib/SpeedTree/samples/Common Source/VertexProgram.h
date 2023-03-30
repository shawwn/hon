///////////////////////////////////////////////////////////////////////  
//  CVertexProgram Class
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


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram

class CVertexProgram
{
public:
        CVertexProgram( );
        ~CVertexProgram( );

        bool            Bind(void) const;
static  bool            Disable(void);
static  bool            Enable(void);
        bool            Load(const char* pCodeString);

private:
        bool            m_bLoaded;      // was the vertex program successfully loaded
        GLuint          m_uiProgram;    // vertex program binding index
};
