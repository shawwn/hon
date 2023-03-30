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
//

#pragma warning (disable : 4786) // disable lengthy STL-generated identifiers
//lint -e18
#include "IdvVertexBuffer.h"
#include <cstdio>
#include <GL\glu.h>
#include <cassert>
using namespace std;

// static variables
CIdvVertexBuffer::SVarBuffer CIdvVertexBuffer::m_sDynamicVarBuffer;
CIdvVertexBuffer::SVarBuffer CIdvVertexBuffer::m_sStaticVarBuffer;
unsigned int CIdvVertexBuffer::m_uiLastBoundObject = 0;
unsigned int CIdvVertexBuffer::m_uiRefCount = 0;


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::CIdvVertexBuffer

CIdvVertexBuffer::CIdvVertexBuffer( ) :
    // general use
    m_bStaticBuffer(true),
    m_uiBufferSize(0),
    m_pBufferSegment(NULL)
{
    ++m_uiRefCount;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::~CIdvVertexBuffer

CIdvVertexBuffer::~CIdvVertexBuffer( )
{
    if (--m_uiRefCount == 0)
    {
        m_sDynamicVarBuffer.Clear( );
        m_sStaticVarBuffer.Clear( );
    }

    // segment pointer is freed by Clear() above
    m_pBufferSegment = NULL;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::Bind

bool CIdvVertexBuffer::Bind(void)
{
    bool bSuccess = false;

    SVarBuffer* pBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
    switch(pBuf->m_eMode)
    {
    case MODE_ARB_VERTEX_BUFFER_OBJECT:
        // make the ARB VBO active
        if (pBuf->m_uiBufferObject != m_uiLastBoundObject)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, pBuf->m_uiBufferObject);
            m_uiLastBoundObject = pBuf->m_uiBufferObject;
        }
        // texcoord0 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD0].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD0;
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
        }
        // texcoord1 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD1].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD1;
            glClientActiveTextureARB(GL_TEXTURE1_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // texcoord2 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD2].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD2;
            glClientActiveTextureARB(GL_TEXTURE2_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // texcoord3 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD3].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD3;
            glClientActiveTextureARB(GL_TEXTURE3_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // color attribute
        if (m_asVertexAttribs[VERTEX_COLOR].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_COLOR;
            glColorPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
        }
        // normal attribute
        if (m_asVertexAttribs[VERTEX_NORMAL].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_NORMAL;
            assert(pAttrib->m_nCount == 3);
            glNormalPointer(pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
        }
        // vertex attribute
        if (m_asVertexAttribs[VERTEX_POSITION].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_POSITION;
            glVertexPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, (GLvoid*) (m_pBufferSegment + pAttrib->m_nOffset));
        }
        bSuccess = true;
        break;
    case MODE_NV_VERTEX_ARRAY_RANGE:
    case MODE_SYSTEM_RAM:
        // texcoord0 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD0].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD0;
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
        }
        // texcoord1 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD1].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD1;
            glClientActiveTextureARB(GL_TEXTURE1_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // texcoord2 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD2].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD2;
            glClientActiveTextureARB(GL_TEXTURE2_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // texcoord3 attribute
        if (m_asVertexAttribs[VERTEX_TEXCOORD3].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_TEXCOORD3;
            glClientActiveTextureARB(GL_TEXTURE3_ARB);
            glTexCoordPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
        }
        // color attribute
        if (m_asVertexAttribs[VERTEX_COLOR].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_COLOR;
            glColorPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
        }
        // normal attribute
        if (m_asVertexAttribs[VERTEX_NORMAL].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_NORMAL;
            glNormalPointer(pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
        }
        // vertex attribute
        if (m_asVertexAttribs[VERTEX_POSITION].IsActive( ))
        {
            SStrideData* pAttrib = m_asVertexAttribs + VERTEX_POSITION;
            glVertexPointer(pAttrib->m_nCount, pAttrib->m_eDataType, pAttrib->m_nStride, m_pBufferSegment + pAttrib->m_nOffset);
        }
        bSuccess = true;
        break;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::UnBind

bool CIdvVertexBuffer::UnBind(void)
{
    bool bSuccess = false;

    SVarBuffer* pBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
    switch(pBuf->m_eMode)
    {
    case MODE_ARB_VERTEX_BUFFER_OBJECT:
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        m_uiLastBoundObject = 0;
        bSuccess = true;
        break;
    case MODE_NV_VERTEX_ARRAY_RANGE:
        bSuccess = true;
        break;
    case MODE_SYSTEM_RAM:
        bSuccess = true;
        break;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::SetBuffer

bool CIdvVertexBuffer::SetBuffer(const GLvoid* pData, unsigned int uiSize, bool bStatic)
{
    m_bStaticBuffer = bStatic;

    // allocate temporary RAM copy
    m_pBufferSegment = new unsigned char[uiSize];
    memcpy(m_pBufferSegment, pData, uiSize);

    // record this buffer
    m_uiBufferSize = uiSize;
    if (bStatic)
        m_sStaticVarBuffer.m_vBuffers.push_back(this);
    else
        m_sDynamicVarBuffer.m_vBuffers.push_back(this);

    return true;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::SetStride

void CIdvVertexBuffer::SetStride(EAttrib eAttrib, unsigned int uiCount, GLenum eDataType, unsigned int uiStride, unsigned int uiOffset)
{
    m_asVertexAttribs[eAttrib].m_eAttrib = eAttrib;
    m_asVertexAttribs[eAttrib].m_eDataType = eDataType;
    m_asVertexAttribs[eAttrib].m_nCount = uiCount;
    m_asVertexAttribs[eAttrib].m_nOffset = uiOffset;
    m_asVertexAttribs[eAttrib].m_nStride = uiStride;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::UpdateBuffer

bool CIdvVertexBuffer::UpdateBuffer(const GLvoid* pData)
{
    bool bSuccess = false;

    SVarBuffer* pBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
    switch(pBuf->m_eMode)
    {
    case MODE_ARB_VERTEX_BUFFER_OBJECT:
    {
        SVarBuffer* pArbBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, pArbBuf->m_uiBufferObject);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, int(m_pBufferSegment), m_uiBufferSize, pData);
        bSuccess = true;
    }
        break;
    case MODE_NV_VERTEX_ARRAY_RANGE:
    case MODE_SYSTEM_RAM:
        if (m_pBufferSegment && pData)
            memcpy(m_pBufferSegment, pData, m_uiBufferSize);
        bSuccess = true;
        break;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::UpdatePartialBuffer

bool CIdvVertexBuffer::UpdatePartialBuffer(unsigned int uiOffset, const GLvoid* pData, unsigned int uiSize)
{
    bool bSuccess = false;

    SVarBuffer* pBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
    switch(pBuf->m_eMode)
    {
    case MODE_ARB_VERTEX_BUFFER_OBJECT:
    {
        SVarBuffer* pArbBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, pArbBuf->m_uiBufferObject);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, int(m_pBufferSegment) + uiOffset, uiSize, pData);
        bSuccess = true;
    }
        break;
    case MODE_NV_VERTEX_ARRAY_RANGE:
    case MODE_SYSTEM_RAM:
        if (m_pBufferSegment && pData)
            memcpy(m_pBufferSegment + uiOffset, pData, uiSize);
        bSuccess = true;
        break;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::DisableClientStates

void CIdvVertexBuffer::DisableClientStates(void) const
{
    for (int nAttrib = 0; nAttrib < VERTEX_NUM_ATTRIBS; ++nAttrib)
    {
        const SStrideData* pAttrib = m_asVertexAttribs + nAttrib;

        if (pAttrib->IsActive( ))
        {
            switch (EAttrib(nAttrib))
            {
            case VERTEX_COLOR:
                glDisableClientState(GL_COLOR_ARRAY);
                break;
            case VERTEX_POSITION:
                glDisableClientState(GL_VERTEX_ARRAY);
                break;
            case VERTEX_NORMAL:
                glDisableClientState(GL_NORMAL_ARRAY);
                break;
            case VERTEX_TEXCOORD0:
                glActiveTextureARB(GL_TEXTURE0_ARB);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                break;
            case VERTEX_TEXCOORD1:
            case VERTEX_TEXCOORD2:
            case VERTEX_TEXCOORD3:
            {
                unsigned int uiLayer = nAttrib - VERTEX_TEXCOORD0;
                glClientActiveTextureARB(GL_TEXTURE0_ARB + uiLayer);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                // make sure base layer is active upon return
                glClientActiveTextureARB(GL_TEXTURE0_ARB);
            }
                break;
            default:
                assert(false);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::EnableClientStates

void CIdvVertexBuffer::EnableClientStates(void) const
{
    SVarBuffer* pBuf = m_bStaticBuffer ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;

    for (int nAttrib = 0; nAttrib < VERTEX_NUM_ATTRIBS; ++nAttrib)
    {
        const SStrideData* pAttrib = m_asVertexAttribs + nAttrib;

        if (pAttrib->IsActive( ))
        {
            switch (EAttrib(nAttrib))
            {
            case VERTEX_COLOR:
                glEnableClientState(GL_COLOR_ARRAY);
                break;
            case VERTEX_POSITION:
                glEnableClientState(GL_VERTEX_ARRAY);
                break;
            case VERTEX_NORMAL:
                glEnableClientState(GL_NORMAL_ARRAY);
                break;
            case VERTEX_TEXCOORD0:
                if (pBuf->m_eMode == MODE_ARB_VERTEX_BUFFER_OBJECT)
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                break;
            case VERTEX_TEXCOORD1:
            case VERTEX_TEXCOORD2:
            case VERTEX_TEXCOORD3:
            {
                unsigned int uiLayer = nAttrib - VERTEX_TEXCOORD0;
                glClientActiveTextureARB(GL_TEXTURE0_ARB + uiLayer);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                // make sure base layer is active upon return
                glClientActiveTextureARB(GL_TEXTURE0_ARB);
            }
                break;
            default:
                assert(false);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::Disable

void CIdvVertexBuffer::Disable(bool bStatic)
{
    SVarBuffer* pBuf = bStatic ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;

    // disable the NVIDIA extension
    if (pBuf->m_eMode == MODE_NV_VERTEX_ARRAY_RANGE)
        glDisableClientState(GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV);
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::Enable

void CIdvVertexBuffer::Enable(bool bStatic)
{
    SVarBuffer* pBuf = bStatic ? &m_sStaticVarBuffer : &m_sDynamicVarBuffer;

    // enable the NVIDIA extension
    if (pBuf->m_eMode == MODE_NV_VERTEX_ARRAY_RANGE)
    {
        if (bStatic)
            glVertexArrayRangeNV(m_sStaticVarBuffer.m_uiBufferSize, m_sStaticVarBuffer.m_pVarBuffer);
        else
            glVertexArrayRangeNV(m_sDynamicVarBuffer.m_uiBufferSize, m_sDynamicVarBuffer.m_pVarBuffer);

        glEnableClientState(GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::DoneAllocating

void CIdvVertexBuffer::DoneAllocating(void)
{
    SVarBuffer* apBuffers[2] = 
    {
        &m_sStaticVarBuffer,
        &m_sDynamicVarBuffer
    };

    // how big are all of the static/dynamic buffers together?
    bool bStatic = true;
    for (int nBuffer = 0; nBuffer < 2; ++nBuffer)
    {
        SVarBuffer* pBuf = apBuffers[nBuffer];
        pBuf->m_uiBufferSize = 0;
        for (vector<CIdvVertexBuffer*>::iterator i = pBuf->m_vBuffers.begin( ); i != pBuf->m_vBuffers.end( ); ++i)
            pBuf->m_uiBufferSize += (*i)->m_uiBufferSize;

        if (pBuf->m_uiBufferSize > 0)
        {
            // copy all of the buffers into one contiguous array
            unsigned char* pCompoundBuffer = new unsigned char[pBuf->m_uiBufferSize];
            unsigned char* pCopy = pCompoundBuffer;
			for (std::vector<CIdvVertexBuffer*>::iterator i = pBuf->m_vBuffers.begin( ); i != pBuf->m_vBuffers.end( ); ++i)
            {
                memcpy(pCopy, (*i)->m_pBufferSegment, (*i)->m_uiBufferSize);
                pCopy += (*i)->m_uiBufferSize;
            }

            // depending on the hardware, allocate the video RAM
            bool bModeDetermined = false;
            pBuf->m_eMode = DetermineMode( );
        
            while (!bModeDetermined)
            {
                switch(pBuf->m_eMode)
                {
                case MODE_ARB_VERTEX_BUFFER_OBJECT:
                {
                    glGenBuffersARB(1, &pBuf->m_uiBufferObject);
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, pBuf->m_uiBufferObject);

                    GLenum ePrevError = glGetError( );
                    glBufferDataARB(GL_ARRAY_BUFFER_ARB, pBuf->m_uiBufferSize, pCompoundBuffer, bStatic ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);
                    GLenum eNextError = glGetError( );

                    // check to see if the buffer allocation failed
                    if (eNextError == GL_OUT_OF_MEMORY &&
                        eNextError != ePrevError)
                    {
                        // can't use MODE_ARB_VBO, delete resources already allocated
                        glDeleteBuffersARB(1, &pBuf->m_uiBufferObject);
                        pBuf->m_uiBufferObject = 0;

                        //g_cConsole.PrintError("Failed to allocate video RAM [%.1f MB] using ARB_vertex_buffer_objects\n", pBuf->m_uiBufferSize / (1024.0f * 1024.0f));
                        pBuf->m_eMode = MODE_SYSTEM_RAM;
                    }
                    else
                    {
                        //g_cConsole.Print("Successfully allocated %.1f MB of ARB_vertex_buffer_object memory\n", pBuf->m_uiBufferSize / (1024.0f * 1024.0f));
                        bModeDetermined = true;
                    }
                }
                    break;
                case MODE_NV_VERTEX_ARRAY_RANGE:
                {
                    pBuf->m_pVarBuffer = static_cast<unsigned char*>(wglAllocateMemoryNV(pBuf->m_uiBufferSize, 0.0f, 0.0f, 1.0f));
                    if (!pBuf->m_pVarBuffer)
                    {
                        //g_cConsole.PrintError("Failed to allocate video RAM [%.1f MB] using wglAllocateMemoryNV, falling back system RAM (will be slower)\n", pBuf->m_uiBufferSize / (1024.0f * 1024.0f));
                        pBuf->m_eMode = MODE_SYSTEM_RAM;
                    }
                    else
                    {
                        memcpy(pBuf->m_pVarBuffer, pCompoundBuffer, pBuf->m_uiBufferSize);
                        //g_cConsole.Print("Successfully allocated %.1f MB using wglAlloc\n", pBuf->m_uiBufferSize / (1024.0f * 1024.0f));
                        bModeDetermined = true;
                    }
                }
                    break;
                case MODE_SYSTEM_RAM:
                    pBuf->m_pVarBuffer = new unsigned char[pBuf->m_uiBufferSize];
                    //g_cConsole.Print("Successfully allocated %.1f MB of system memory\n", pBuf->m_uiBufferSize / (1024.0f * 1024.0f));
                    memcpy(pBuf->m_pVarBuffer, pCompoundBuffer, pBuf->m_uiBufferSize);
                    bModeDetermined = true;
                    break;
                }
            }

            // delete temporary buffer
            if (bModeDetermined)
                delete[] pCompoundBuffer;
        }

        // run through the individual buffers, updating the separate smaller buffer pointers
        unsigned char* pSegmentBuffer = (pBuf->m_eMode == MODE_NV_VERTEX_ARRAY_RANGE || pBuf->m_eMode == MODE_SYSTEM_RAM) ? pBuf->m_pVarBuffer : 0;
		for (std::vector<CIdvVertexBuffer*>::iterator i = pBuf->m_vBuffers.begin( ); i != pBuf->m_vBuffers.end( ); ++i)
        {
            delete[] (*i)->m_pBufferSegment;
            (*i)->m_pBufferSegment = pSegmentBuffer;
            //lint -e{613}  { possible NULL pointer use is OK for ARB_VBO mode }
            pSegmentBuffer += (*i)->m_uiBufferSize;
        }

        bStatic = !bStatic;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvVertexBuffer::DetermineMode

CIdvVertexBuffer::EMode CIdvVertexBuffer::DetermineMode(void)
{
    EMode eMode = MODE_SYSTEM_RAM;

    const unsigned char* pVendor = glGetString(GL_VENDOR);
    bool bNvidiaVendor = (strstr(reinterpret_cast<const char*>(pVendor), "nvidia") != NULL) ||
                         (strstr(reinterpret_cast<const char*>(pVendor), "NVIDIA") != NULL) || 
                         (strstr(reinterpret_cast<const char*>(pVendor), "NVIDIA") != NULL) || 
                         (strstr(reinterpret_cast<const char*>(pVendor), "NVIDIA") != NULL);
    bool bARB_Supported = (glBindBufferARB != NULL);
    bool bVAR_Supported = (wglAllocateMemoryNV != NULL);
//  bool bATI_VBO_Support = (glNewObjectBufferATI != NULL);

    if (bNvidiaVendor && bVAR_Supported)
        eMode = MODE_NV_VERTEX_ARRAY_RANGE;
    else if (bARB_Supported)
        eMode = MODE_ARB_VERTEX_BUFFER_OBJECT;

    return eMode;
}




