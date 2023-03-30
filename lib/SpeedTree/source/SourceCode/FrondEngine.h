///////////////////////////////////////////////////////////////////////  
//         Name: FrondEngine.h
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
#include "SpeedTreeRT.h"
#include "LibGlobals_Source/IdvGlobals.h"
#include "LibFilename_Source/IdvFilename.h"
#include "FileAccess.h"
#include <vector>


///////////////////////////////////////////////////////////////////////  
//  struct SFrondVertex declaration

struct SFrondVertex
{
        SFrondVertex( ) :
            m_fCrossSectionWeight(0.0f),
            m_nWindGroup(0)
        {
        }

        CVec3           m_cPos;                                     // xyz position of the vertex (computed as a branch)
        CRotTransform   m_cTrans;                                   // orientation of the vertex (computed as a branch)
        float           m_fCrossSectionWeight;                      // wind weight of all vertices around this point (computed as a branch)
        int             m_nWindGroup;                               // wind group that the frond belongs to
};


///////////////////////////////////////////////////////////////////////  
//  struct SFrondGuide declaration

struct SFrondGuide
{
        SFrondGuide( ) :
            m_fLength(0.0f),
            m_fRadius(0.0f),
            m_chFrondMapIndex(0),
            m_fOffsetAngle(0.0f),
            m_fSurfaceArea(0.0f),
            m_fFuzzySurfaceArea(0.0f),
            m_nVertexStartIndex(0),
            m_nVerticesPerGuideVertex(0)
        {
        }

        std::vector<SFrondVertex>   m_vVertices;                    // all of the vertices in this guide (guides are frond "center lines")
        float                       m_fLength;                      // length of the guide
        float                       m_fRadius;                      // radius of the branch that generated this guide
        unsigned char               m_chFrondMapIndex;              // specifies which of the "n" frond maps this frond is associated with
        float                       m_fOffsetAngle;                 // angle by which frond is rotated around the "center line"
        float                       m_fSurfaceArea;                 // surface area of frond (factors heavily in lod)
        float                       m_fFuzzySurfaceArea;            // modified surface area to add randomness to frond disappearance due to lod
        unsigned int                m_nVertexStartIndex;            // location of the fronds position in the indexed geometry class
        unsigned int                m_nVerticesPerGuideVertex;      // number of vertices associated with each guide vertex
};


///////////////////////////////////////////////////////////////////////  
//  SFrondTexture

struct SFrondTexture
{
        SFrondTexture( ) :
            m_fAspectRatio(0.5f),
            m_fSizeScale(1.0f),
            m_fMinAngleOffset(0.0f),
            m_fMaxAngleOffset(0.0f)
        
        {
        }

        CIdvFilename    m_strFilename;                              // texture map filename
        float           m_fAspectRatio;                             // aspect ratio (widht/height) of the texture
        float           m_fSizeScale;                               // fronds are sized based on length of the guide, this value scales that size
        float           m_fMinAngleOffset;                          // minimum angle offset (from SpeedTreeCAD)
        float           m_fMaxAngleOffset;                          // maximum angle offset (from SpeedTreeCAD)
};


///////////////////////////////////////////////////////////////////////  
//  class CFrondEngine declaration

class CFrondEngine
{
public:
        // construction/destruction
        CFrondEngine( );
        CFrondEngine(const CFrondEngine& cRight);
        ~CFrondEngine( );
                                                
        // general                              
        int                                     GetLevel(void) const;
        bool                                    Enabled(void) const;
        void                                    Compute(CIndexedGeometry* pGeometry, CLightingEngine* pLightingEngine);
                                                
        unsigned int                            GetTextureCount(void) const                                     { return m_vTextures.size( ); }
        const char*                             GetTextureFilename(unsigned int nIndex) const                   { return m_vTextures[nIndex].m_strFilename.c_str( ); }
        void                                    SetTextureFilename(unsigned int nIndex, const char* pFilename)  { m_vTextures[nIndex].m_strFilename = pFilename; }
        void                                    SetTextureCoords(CIndexedGeometry* pGeometry, unsigned int nFrondMapIndex, const float* pTexCoords, bool bFlip);
                                                
        // frond guides                         
        void                                    ClearGuides(void);
        void                                    StartGuide(void);
        void                                    AddGuideVertex(CVec3 cPos, CRotTransform cTrans, float fCrossSectionWeight, int nWindGroup);
        void                                    EndGuide(float fLodSizeScalar);
                                                
        // spt parsing                          
        void                                    Parse(CTreeFileAccess& cFile);
        void                                    Save(CTreeFileAccess& cFile) const;
                                                
        // operators                            
const   CFrondEngine&                           operator=(const CFrondEngine& cRight);
                                                
private:                                        
        void                                    BuildBladeVertices(SFrondGuide& sGuide);
        void                                    ComputeBlade(unsigned int nLod, unsigned int nStart, SFrondGuide& sGuide);
        void                                    BuildExtrusionVertices(SFrondGuide& sGuide);
        void                                    ComputeExtrusion(unsigned int nLod, unsigned int nStart, SFrondGuide& sGuide);
        void                                    BuildProfileVectors(SFrondGuide& sGuide, std::vector<CVec3>& vProfile, std::vector<CVec3>& vNormals);
        void                                    SetProfile(CIdvBezierSpline* pProfile);
        void                                    BuildGuideLods(void);

        // general                              
        CIndexedGeometry*                       m_pGeometry;                // this is the class that holds all vertices, texture coordinates, triangles, etc.
        CLightingEngine*                        m_pLightingEngine;          // class used to compute static lighting
                                                
        // frond guides                         
        std::vector<SFrondGuide>                m_vGuides;                  // vector of guides (guides represent the "center line" of a frond)
        std::vector< std::vector<SFrondGuide> > m_vGuideLods;               // this vector holds gradually lower resolution versions of the guides (

        // frond parameters in the .spt data
        enum EFrondType
        {
            FROND_BLADES, FROND_EXTRUSION, NUM_FROND_TYPES
        };

        EFrondType                              m_eFrondType;               // type of frond (blade or extrusion)
        unsigned int                            m_nNumBlades;               // number of blades
        CIdvBezierSpline*                       m_pProfile;                 // shape of cross-section of extruded fronds
        unsigned int                            m_nProfileSegments;         // number of segments in the cross-section
        unsigned int                            m_nLevel;                   // branch level at which fronds become active
        bool                                    m_bEnabled;                 // whether or not fronds are enabled
        std::vector<SFrondTexture>              m_vTextures;                // the CAD-specified frond maps
        unsigned int                            m_nNumLodLevels;            // number of discrete frond LODs
        float                                   m_fMaxSurfaceAreaPercent;   // the percentage of the total frond area present at the highest detail
        float                                   m_fMinSurfaceAreaPercent;   // the percentage of the total frond area present at the lowest 3D detail
        float                                   m_fReductionFuzziness;      // controls how strictly size is used as the LOD judge (higher numbers = more random)
        float                                   m_fLargeRetentionPercent;   // controls the percentage of "large" fronds that must remain
        unsigned int                            m_nMinLengthSegments;       // for LOD computation, this value represents the minimum number of lengthwise sections
        unsigned int                            m_nMinCrossSegments;        // for LOD computation, this value represents the minimum number of cross-sectional sections
};
