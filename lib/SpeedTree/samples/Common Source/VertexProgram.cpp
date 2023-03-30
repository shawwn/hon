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

#include "Console.h"
#include "VertexProgram.h"


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::CVertexProgram

CVertexProgram::CVertexProgram( ) :
    m_bLoaded(false),
    m_uiProgram(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::~CVertexProgram

CVertexProgram::~CVertexProgram( )
{
    if (m_uiProgram)
    {
        glDeleteProgramsARB(1, &m_uiProgram);
        m_uiProgram = 0;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::Bind

bool CVertexProgram::Bind(void) const
{
    bool bSuccess = false;

    if (glBindProgramARB && m_bLoaded)
    {
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiProgram);
        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::Disable

bool CVertexProgram::Disable(void)
{
    bool bSuccess = false;

    if (glBindProgramARB)
    {
        glDisable(GL_VERTEX_PROGRAM_ARB);
        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::Enable

bool CVertexProgram::Enable(void)
{
    bool bSuccess = false;

    if (glBindProgramARB)
    {
        glEnable(GL_VERTEX_PROGRAM_ARB);
        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CVertexProgram::Load

bool CVertexProgram::Load(const char* pCodeString)
{
    m_uiProgram = 0;
    m_bLoaded = false;

    // load vertex programs
    if (extgl_Extensions.ARB_vertex_program)
    {
        glGenProgramsARB(1, &m_uiProgram); 
        glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_uiProgram);
        glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(pCodeString), pCodeString);

        const GLubyte* pError = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        if (strlen((char*) pError) > 0)
        {
            fprintf(stderr, "Failed to compile vertex shader (%s)\n", pError);
            m_uiProgram = 0;
        }
        else
        {
            fprintf(stderr, "successfully loaded vertex program\n");
            m_bLoaded = true;
        }
    }
    else
    {
        fprintf(stderr, "ARB_vertex_program extension not found (hardware or driver doesn't support hardware vertex shaders)\n");
        m_uiProgram = 0;
    }

    return m_bLoaded;
}

