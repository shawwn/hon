///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRTExample Class
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


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include "IdvVertexBuffer.h"
#include "glut.h"
#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeRTExample declaration

class CSpeedTreeWrapper
{
public:
        CSpeedTreeWrapper( );
virtual ~CSpeedTreeWrapper( );
           
        // geometry 
        bool                        LoadTree(const char* pszSptFile, unsigned int nSeed = 1, float fSize = -1.0f, float fSizeVariance = -1.0f);
        const float*                GetPos(void) const                      { return m_afPos; }
        void                        SetPos(const float* pPos);
        const float*                GetBoundingBox(void) const              { return m_afBoundingBox; }

        // rendering
        void                        RenderBranches(void) const;
        void                        RenderFronds(void) const;
        void                        RenderLeaves(void) const;
        #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        void                        UploadLeafTables(unsigned int uiLocation) const;
        #endif
        void                        RenderBillboards(void) const;
        void                        SetupBranchForTreeType(void);
        void                        SetupFrondForTreeType(void);
        void                        SetupLeafForTreeType(void);

        // instancing
        CSpeedTreeWrapper*          InstanceOf(void) const                  { return m_pInstanceOf; }
        bool                        IsInstance(void) const                  { return m_bIsInstance; }
        CSpeedTreeWrapper*          MakeInstance(void);
        CSpeedTreeRT*               GetSpeedTree(void) const                { return m_pSpeedTree; }

        // lighting
        const CSpeedTreeMaterial&   GetBranchMaterial(void) const           { return m_cBranchMaterial; }
        const CSpeedTreeMaterial&   GetFrondMaterial(void) const            { return m_cFrondMaterial; }
        const CSpeedTreeMaterial&   GetLeafMaterial(void) const             { return m_cLeafMaterial; }
        float                       GetLeafLightingAdjustment(void) const   { return m_pSpeedTree->GetLeafLightingAdjustment( ); }

        // wind
        void                        SetWindStrength(float fStrength)        { (void) m_pSpeedTree->SetWindStrength(fStrength); }
        void                        Advance(void);

        // utility
        unsigned int                GetBranchTexture(void)  const           { return m_uiBranchTexture; }
        unsigned int                GetSelfShadowTexture(void) const        { return m_uiSelfShadowTexture; }

        // vertex buffers
        #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        void                        BeginBranchRenderingStates(void) const;
        void                        EndBranchRenderingStates(void) const;
        void                        BeginFrondRenderingStates(void) const;
        void                        EndFrondRenderingStates(void) const;
        void                        BeginLeafRenderingStates(void) const;
        void                        EndLeafRenderingStates(void) const;
        #endif

private:
        #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        // initialization
        void                        SetupVertexBuffers(void);
        void                        SetupBranchBuffer(void);
        void                        SetupFrondBuffer(void);
        void                        SetupLeafBuffers(void);
        #endif

        // SpeedTreeRT data
        CSpeedTreeRT*               m_pSpeedTree;           // pointer to main SpeedTreeRT class
        bool                        m_bIsInstance;          // true if "this" is an instance, false otherwise
        CSpeedTreeWrapper*          m_pInstanceOf;          // if "this" is an instance, m_pInstanceOf points to the original tree
static  unsigned int                m_uiRefCount;           // used to delete the static members when last object is deleted

        // rendering
        #ifdef WRAPPER_RENDER_VERTEX_OBJECTS
        CIdvVertexBuffer*           m_pBranchVertexBuffer;  // IDV utility class for fast branch rendering
        CIdvVertexBuffer*           m_pFrondVertexBuffer;   // IDV utility class for fast frond rendering
        CIdvVertexBuffer*           m_pLeafVertexBuffers;   // IDV utility class for fast leaf rendering
        bool*                       m_pLeavesUpdatedByCpu;  // flags, reset per frame, that prohibit redundant computations
static  unsigned int                m_uiTmpCoordsBufferSize;
static  float*                      m_pTmpCoordsBuffer;
        #endif
        GLuint                      m_uiBranchTexture;      // texture index for branch texture
        CSpeedTreeMaterial          m_cBranchMaterial;      // IDV utility class for storing branch lighting material
        CSpeedTreeMaterial          m_cLeafMaterial;        // IDV utility class for storing leaf lighting material
        CSpeedTreeMaterial          m_cFrondMaterial;       // IDV utility class for storing frond lighting material
        GLuint                      m_uiSelfShadowTexture;  // texture index for self-shadow texture

        // tree properties
        float                       m_afPos[3];             // xyz position of tree, usually the center base of the trunk
        float                       m_afBoundingBox[6];     // orthographic bounding box, [0-2] = min xyz, [3-5] = max xyz
};
