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

#include <d3d8.h>
#include <d3d8types.h>
#include <d3dx8.h>
#include "D3DUtil.h"
#include "DXUtil.h"
#include "SpeedTreeConfig.h"
#include "SpeedTreeMaterial.h"
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
        void                        SetWindStrength(float fStrength)                { m_pSpeedTree->SetWindStrength(fStrength); }
        void                        Advance(void);

        // utility
static  void                        SetRenderingDevice(LPDIRECT3DDEVICE8 pDevice)   { m_pDx = pDevice; }
        LPDIRECT3DTEXTURE8          GetBranchTexture(void)  const                   { return m_texBranchTexture; }
        void                        CleanUpMemory(void);

private:
        void                        SetupBuffers(void);
        void                        SetupBranchBuffers(void);
        void                        SetupFrondBuffers(void);
        void                        SetupLeafBuffers(void);
        void                        PositionTree(void) const;
static  LPDIRECT3DTEXTURE8          LoadTexture(const char* pFilename);
        void                        SetShaderConstants(const float* pMaterial) const;


private:
        // SpeedTreeRT data
        CSpeedTreeRT*                   m_pSpeedTree;                   // the SpeedTree object
        CSpeedTreeRT::STextures*        m_pTextureInfo;                 // texture info cache
        bool                            m_bIsInstance;                  // is this an instance?
        std::vector<CSpeedTreeWrapper*> m_vInstances;                   // what is an instance of us
        CSpeedTreeWrapper*              m_pInstanceOf;                  // which tree is this an instance of

        // geometry cache
        CSpeedTreeRT::SGeometry*        m_pGeometryCache;               // cache for pulling geometry from SpeedTree
                                                                        //  avoids lots of reallocation

        // branch buffers
        LPDIRECT3DVERTEXBUFFER8         m_pBranchVertexBuffer;          // branch vertex buffer
        unsigned int                    m_unBranchVertexCount;          // number of vertices in branches
        LPDIRECT3DINDEXBUFFER8          m_pBranchIndexBuffer;           // branch index buffer
        unsigned short*                 m_pBranchIndexCounts;           // number of indexes per branch LOD level
        
        // frond buffers
        LPDIRECT3DVERTEXBUFFER8         m_pFrondVertexBuffer;           // frond vertex buffer
        unsigned int                    m_unFrondVertexCount;           // number of vertices in frond
        LPDIRECT3DINDEXBUFFER8*         m_pFrondIndexBuffers;           // frond index buffer
        unsigned short*                 m_pFrondIndexCounts;            // number of indexes per frond LOD level
		unsigned int					m_unNumFrondLods;				// number of frond LODs

        // leaf buffers
        unsigned short                  m_usNumLeafLods;                // the number of leaf LODs
        LPDIRECT3DVERTEXBUFFER8*        m_pLeafVertexBuffer;            // leaf vertex buffer
        bool*                           m_pLeavesUpdatedByCpu;          // stores which LOD's have been updated already per frame

        // tree properties
        float                           m_afPos[3];                     // tree position
        float                           m_afBoundingBox[6];             // tree bounding box
        
        // materials
        CSpeedTreeMaterial              m_cBranchMaterial;              // branch material
        CSpeedTreeMaterial              m_cLeafMaterial;                // leaf material
        CSpeedTreeMaterial              m_cFrondMaterial;               // frond material

        // branch texture
        LPDIRECT3DTEXTURE8              m_texBranchTexture;             // branch texture object
static  LPDIRECT3DTEXTURE8              m_texShadow;                    // shadow texture object (used if shadows are enabled)

        // texture loading
static  unsigned int                                m_unNumWrappersActive;

        // rendering context
static  LPDIRECT3DDEVICE8               m_pDx;                          // rendering context
};
