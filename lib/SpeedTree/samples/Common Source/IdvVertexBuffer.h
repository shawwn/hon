///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer Class
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
#include "extgl.h"
#include <vector>


///////////////////////////////////////////////////////////////////////  
//  class CIdvVertexBuffer declaration
//
//  This class is used to abstract and facilitate efficient vertex buffer
//  use using OpenGL extensions.  Once the vertex data is fed into the class,
//  it will intelligently chose which OpenGL extensions it should employ
//  to most efficiently render the data.  Our experience has been that
//  rendering using ARB_vertex_buffer_object or NV_vertex_array_range
//  is much quicker than using display lists.
//
//  As of the release of this particular version of the class, nVidia's
//  support of the ARB_vertex_buffer_object is poor.  Consequently, if
//  the class detects that an nVidia videocard is being used, it will try
//  to use nVidia's NV_vertex_array_range extension, even if
//  ARB_vertex_buffer_object is available.
//
//  There is no direct support for ATI_vertex_buffer_object as ATI's
//  implementation of ARB_vertex_buffer_object has been remarkably
//  robust.
//
//  If neither of these extensions are available, or the buffers are
//  too large to fit into video memory, system memory is used instead.
//  Future enhancements to the class could include:
//
//      - Priorities set per requested buffer so that buffers too large
//        to fit into video RAM can be split between video and system RAM
//
//  IDV 12/2/2003

class CIdvVertexBuffer
{
public:
        enum EAttrib
        {
            VERTEX_NORMAL,
            VERTEX_COLOR,
            VERTEX_TEXCOORD0,
            VERTEX_TEXCOORD1,
            VERTEX_TEXCOORD2,
            VERTEX_TEXCOORD3,
            VERTEX_POSITION,
            VERTEX_NUM_ATTRIBS
        };
        enum EMode
        {
            MODE_ARB_VERTEX_BUFFER_OBJECT,
            MODE_NV_VERTEX_ARRAY_RANGE,
            MODE_SYSTEM_RAM,
            MODE_UNKNOWN
        };

        CIdvVertexBuffer( );
        ~CIdvVertexBuffer( );

        bool                            Bind(void);
        bool                            UnBind(void);

        bool                            SetBuffer(const GLvoid* pData, unsigned int uiSize, bool bStatic);
        void                            SetStride(EAttrib eAttrib, unsigned int uiCount, GLenum eDataType, unsigned int uiStride, unsigned int uiOffset);
        bool                            UpdateBuffer(const GLvoid* pData);
        bool                            UpdatePartialBuffer(unsigned int uiOffset, const GLvoid* pData, unsigned int uiSize);

        void                            DisableClientStates(void) const;
        void                            EnableClientStates(void) const;

static  void                            Disable(bool bStatic);
static  void                            Enable(bool bStatic);
static  void                            DoneAllocating(void);

private:

        struct SStrideData
        {
            SStrideData( ) :
                m_eAttrib(VERTEX_POSITION),
                m_eDataType(GL_FLOAT),
                m_nCount(-1),
                m_nOffset(-1),
                m_nStride(-1)
            {
            }

            bool        IsActive(void) const        { return m_nCount > -1; }

            EAttrib     m_eAttrib;
            GLenum      m_eDataType;
            int         m_nCount;
            int         m_nOffset;
            int         m_nStride;
        };

        // general functions
static  EMode                           DetermineMode(void);

        // general variables shared by all buffer types
        unsigned int                    m_uiBufferSize;         // size in bytes of this particular buffer
        bool                            m_bStaticBuffer;        // static or streaming data flag
        unsigned char*                  m_pBufferSegment;       // pointer into unified geometry buffer
        SStrideData                     m_asVertexAttribs[VERTEX_NUM_ATTRIBS]; // tracks which vertex types have been set

        struct SVarBuffer
        {
            SVarBuffer( ) :
                m_eMode(MODE_SYSTEM_RAM),
                m_uiBufferSize(0),
                m_uiBufferObject(0),
                m_pVarBuffer(NULL),
                m_pSystemBuffer(NULL)
            {
            }

            EMode                           m_eMode;            // using ATI, NVIDIA, or standard API for storing geometry

            std::vector<CIdvVertexBuffer*>  m_vBuffers;         // list of all dynamic objects that share m_pSharedVarMemory
            unsigned int                    m_uiBufferSize;     // size in bytes of this unified buffer

            // ARB VBO mode
            GLuint                          m_uiBufferObject;   // used to reference ATI vertex buffer object

            // NVIDIA VAR mode
            unsigned char*                  m_pVarBuffer;       // points to the chunk of memory used by this

            // system RAM mode
            unsigned char*                  m_pSystemBuffer;    // pointer to system RAM if that mode is active

            void Clear(void)
            {
                m_vBuffers.clear( );
                m_uiBufferSize = 0;
                if (m_eMode == MODE_ARB_VERTEX_BUFFER_OBJECT)
                {
                    if (m_uiBufferObject)
                    {
                        glDeleteBuffersARB(1, &m_uiBufferObject);
                        m_uiBufferObject = 0;
                    }
                }
                else if (m_eMode == MODE_NV_VERTEX_ARRAY_RANGE)
                {
                    if (m_pVarBuffer)
                    {
                        wglFreeMemoryNV(m_pVarBuffer);
                        m_pVarBuffer = NULL;
                    }
                }
                else if (m_eMode == MODE_SYSTEM_RAM)
                {
                    if (m_pVarBuffer)
                    {
                        delete[] m_pSystemBuffer;
                        m_pSystemBuffer = NULL;
                    }
                }
            }
        };

        // ARB VBO
static  unsigned int                    m_uiLastBoundObject;    // used to prevent redundant binding

static  SVarBuffer                      m_sDynamicVarBuffer;    // buffer instance for dynamic/streaming data
static  SVarBuffer                      m_sStaticVarBuffer;     // buffer instance for static data
static  unsigned int                    m_uiRefCount;           // used to delete the static data when last object is deleted
};
