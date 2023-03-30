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
//

#pragma once
#pragma warning (disable : 4786)

///////////////////////////////////////////////////////////////////////  
//  Include files

#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include <vector>

///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeWrapper declaration

class CSpeedTreeWrapper
{
public:
                                    CSpeedTreeWrapper( );
virtual                             ~CSpeedTreeWrapper( );
       
        // geometry 
        bool                        LoadTree(const char* pszSptFile, unsigned int nSeed = 1, float fSize = -1.0f, float fSizeVariance = -1.0f);
        const float*                GetPos(void) const                              { return m_afPos; }
        void                        SetPos(const float* pPos);
        const float*                GetBoundingBox(void) const                      { return m_afBoundingBox; }

        // rendering
        void                        SetupBranchForTreeType(void) const;
        void                        SetupFrondForTreeType(void) const;
        void                        SetupLeafForTreeType(void) const;
        void                        EndLeafForTreeType(void);

    #ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
        void                        UploadLeafTables(unsigned int uiLocation) const;
    #endif

        void                        RenderBranches(void) const;
        void                        RenderFronds(void) const;
        void                        RenderLeaves(void) const;
        void                        RenderBillboards(void) const;


        // instancing
        CSpeedTreeWrapper**         GetInstances(unsigned int& nCount);
        CSpeedTreeWrapper*          InstanceOf(void) const                          { return m_pInstanceOf; }
        CSpeedTreeWrapper*          MakeInstance(void);                             
        CSpeedTreeRT*               GetSpeedTree(void) const                        { return m_pSpeedTree; }
                                                                                    
        // lighting                                                                 
        const CSpeedTreeMaterial&   GetBranchMaterial(void) const                   { return m_cBranchMaterial; }
        const CSpeedTreeMaterial&   GetFrondMaterial(void) const                    { return m_cFrondMaterial; }
        const CSpeedTreeMaterial&   GetLeafMaterial(void) const                     { return m_cLeafMaterial; }
        float                       GetLeafLightingAdjustment(void) const           { return m_pSpeedTree->GetLeafLightingAdjustment( ); }
                                                                                    
        // wind                                                                     
        void                        Advance(void);

        // utility
static  void                        SetRenderingDevice(LPDIRECT3DDEVICE9 pDevice)   { m_pDx = pDevice; }
        LPDIRECT3DTEXTURE9          GetBranchTexture(void)  const                   { return m_texBranchTexture; }
        void                        CleanUpMemory(void);

private:
        void                        SetupBuffers(void);
        void                        SetupBranchBuffers(void);
        void                        SetupFrondBuffers(void);
        void                        SetupLeafBuffers(void);
        void                        PositionTree(void) const;
static  LPDIRECT3DTEXTURE9          LoadTexture(const char* pFilename);
        void                        SetShaderConstants(const float* pMaterial) const;


private:
        // SpeedTreeRT data
        CSpeedTreeRT*                   m_pSpeedTree;                   // SpeedTree object
        CSpeedTreeRT::STextures*        m_pTextureInfo;                 // cached texture info
        bool                            m_bIsInstance;                  // is this tree an instance?
        std::vector<CSpeedTreeWrapper*> m_vInstances;                   // list of instances of this tree
        CSpeedTreeWrapper*              m_pInstanceOf;                  // this tree is an instance of what

        // geometry cache
        CSpeedTreeRT::SGeometry*        m_pGeometryCache;               // geometry cache

        // branch buffers
        LPDIRECT3DVERTEXBUFFER9         m_pBranchVertexBuffer;          // branch vertex buffer
        unsigned int                    m_unBranchVertexCount;          // number of vertices in branches
        LPDIRECT3DINDEXBUFFER9          m_pBranchIndexBuffer;           // branch index buffer
        unsigned short*                 m_pBranchIndexCounts;           // number of indexes per branch LOD level
        
        // frond buffers
        LPDIRECT3DVERTEXBUFFER9         m_pFrondVertexBuffer;           // frond vertex buffer
        unsigned int                    m_unFrondVertexCount;           // number of vertices in frond
		unsigned int					m_unNumFrondLods;				// number of frond LODs
        LPDIRECT3DINDEXBUFFER9*         m_pFrondIndexBuffers;           // frond index buffers
        unsigned short*                 m_pFrondIndexCounts;            // number of indexes per frond LOD level

        // leaf buffers
        unsigned short                  m_usNumLeafLods;                // the number of leaf LODs
        LPDIRECT3DVERTEXBUFFER9*        m_pLeafVertexBuffer;            // leaf vertex buffer
        bool*                           m_pLeavesUpdatedByCpu;          // stores which LOD's have been updated already per frame

        // tree properties
        float                           m_afPos[3];                     // tree position
        float                           m_afBoundingBox[6];             // tree's bounding box
        
        // materials
        CSpeedTreeMaterial              m_cBranchMaterial;              // branch material
        CSpeedTreeMaterial              m_cLeafMaterial;                // leaf material
        CSpeedTreeMaterial              m_cFrondMaterial;               // frond material

        // branch texture
        LPDIRECT3DTEXTURE9              m_texBranchTexture;             // branch texture object
static  LPDIRECT3DTEXTURE9              m_texShadow;                    // shadow texture object

        // texture loading
static  unsigned int                    m_unNumWrappersActive;
        
        // rendering context
static  LPDIRECT3DDEVICE9               m_pDx;                          // rendering context
};
