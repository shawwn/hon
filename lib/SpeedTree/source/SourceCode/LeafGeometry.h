///////////////////////////////////////////////////////////////////////  
//         Name: LeafGeometry.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2004 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.

#pragma once
#include "Debug.h"
#include "SpeedTreeRT.h"

//
// forward references
//

class CWindEngine;


///////////////////////////////////////////////////////////////////////  
//  class CLeafGeometry declaration
//
//  This class is used as a geometry abstraction mechanism.  The leaves are
//  computed in the CIdvBranch and CTreeEngine classes, but the geometry
//  is stored and accessed with CLeafGeometry.  It contains the code
//  necessary for formatting the leaf data for use within different rendering
//  APIs and/or coordinate systems.

class CLeafGeometry
{
public:
        CLeafGeometry(CWindEngine* pWindEngine);
        ~CLeafGeometry( );

        void                    Invalidate(void);
        void                    EnableManualLighting(bool bFlag)                    { m_bManualLighting = bFlag; }
        void                    EnableVertexWeighting(bool bFlag)                   { m_bVertexWeighting = bFlag; }
        const float*            GetLeafBillboardTable(unsigned int& nEntryCount) const;
        void                    ComputeExtents(CRegion& cExtents) const;
        void                    GetTextureCoords(unsigned int nLeafMapIndex, float* pTexCoords) const;
        unsigned int            GetTriangleCount(unsigned int nLodLevel) const      { st_assert(m_pLods); return m_pLods[nLodLevel].m_usLeafCount * 2; }
        void                    Transform(const CTransform& cTransform);

        void                    Init(unsigned short usNumLeafLods, const vector<CBillboardLeaf*>* pvLeafLods, const SIdvLeafInfo& sLeafInfo);
        bool                    IsVertexWeightingEnabled(void) const                { return m_bVertexWeighting; }
        void                    SetTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords);
        void                    Update(CSpeedTreeRT::SGeometry::SLeaf& sGeometry, unsigned short usLodLevel, float fAzimuth, float fPitch, float fSizeIncreaseFactor);


        ///////////////////////////////////////////////////////////////////////  
        //  struct SLodGeometry declaration
        //
        //  This structure is used to store a single leaf LOD - an array of them
        //  is stored within the CLeafGeometry class.  The member variables change
        //  depending on the API/engine defined during compilation.

        struct SLodGeometry : public CSpeedTreeRT::SGeometry::SLeaf
        {
            SLodGeometry( );
            ~SLodGeometry( );

            void                Invalidate(void)                                    { m_bValid = false; }
            bool                IsValid(void) const                                 { return m_bValid; }
            void                Validate(void)                                      { m_bValid = true; }

            bool                m_bValid;               // used in CPU wind and billboard computations to eliminate redundant computations
            float*              m_pOrigCenterCoords;    // used in CPU wind computations as reference point
        };

private:
        void                    ComputeWindEffect(unsigned short nLodLevel);
        void                    InitLods(unsigned short usNumLeafLods, const vector<CBillboardLeaf*>* pvLeafLods);

        bool                    m_bManualLighting;      // flag for manual lighting mode
        bool                    m_bVertexWeighting;     // flag for vertex weight mode

        // wind information
        CWindEngine*            m_pWindEngine;          // pointer used to modify leaf geometry when WIND_CPU is used

        // compute tables
        unsigned short          m_usNumRockingGroups;   // quantity of separate leaf rocking groups
        float*                  m_pTimeOffsets;         // time offsets for trig functions to keep groups from rocking in sync
        float**                 m_pLeafVertexes;        // points to the table of leaf rocking groups (all centered at origin)
        float*                  m_pLeafTexCoords;       // points to the table of texture coordinates for the leaf rocking groups
        float*                  m_pVertexProgramTable;  // used for vertex shaders to place leaf maps

        // texture information
        unsigned short          m_usNumTextures;         // number of unique textures used to render leaves
        CVec3*                  m_pLeafDimensions;      // dimensions for each leaf type (m_nNumTexture long)
        CVec3*                  m_pLeafOrigins;         // origin for each leaf type (m_nNumTexture long)
        
        // LODs
        unsigned short          m_usNumLods;            // total number of leaf LOD levels
        SLodGeometry*           m_pLods;                // pointer to m_nNumLods LOD levels
};
