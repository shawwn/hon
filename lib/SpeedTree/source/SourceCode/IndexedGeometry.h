///////////////////////////////////////////////////////////////////////  
//         Name: IndexedGeometry.h
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
#pragma warning (push,3)
#include <vector>
#pragma warning (pop)
#pragma warning (disable : 4786)
#include "SpeedTreeRT.h"
#include "LibVector_Source/IdvVector.h"
typedef unsigned char byte;


///////////////////////////////////////////////////////////////////////  
//  class CIndexedGeometry declaration
//
//  This class is used as a geometry abstraction mechanism.  The branches are
//  computed in the CIdvBranch and CTreeEngine classes, but the geometry
//  is stored and accessed with CIndexedGeometry.  It contains the code
//  necessary for formatting the branch data for use within different rendering
//  APIs and/or coordinate systems.

class CIndexedGeometry
{
public:
        CIndexedGeometry(CWindEngine* pWindEngine, bool bRetainTexCoords = false);
        ~CIndexedGeometry( );

        ///////////////////////////////////////////////////////////////////////  
        //  Setting the geometry

        // general
        void                    EnableManualLighting(bool bFlag)                    { m_bManualLighting = bFlag; }
        void                    EnableVertexWeighting(bool bFlag)                   { m_bVertexWeighting = bFlag; }
        void                    CombineStrips(bool bToggleFaceOrdering = false);
        void                    ComputeExtents(CRegion& cExtents) const;
        bool                    IsVertexWeightingEnabled(void) const                { return m_bVertexWeighting; }
        void                    SetNumLodLevels(unsigned short nLevels);
        void                    SetWindMethod(CSpeedTreeRT::EWindMethod eMethod)    { m_eWindMethod = eMethod; }
        void                    Transform(const CTransform& cTransform);

        // indexing 
        void                    AddStrip(unsigned short nLodLevel, unsigned short* pStrip, unsigned short nStripLength);

        // vertex 
        void                    AddVertexColor(const float* pColor);                             // 4 floats: (r, g, b, a)
        void                    AddVertexCoord(const float* pCoord);                             // 3 floats: (x, y, z)
        void                    AddVertexNormal(const float* pNormal);                           // 3 floats: (nx, ny, nz)
        void                    AddVertexTexCoord0(const float* pTexCoord, short sIndex = -1);   // 2 floats: (s, t)
        void                    AddVertexTexCoord1(const float* pTexCoord);                      // 2 floats: (s, t)
        void                    AddVertexWind(float fWindWeight, unsigned char chWindMatrixIndex);

        // bump mapping
        void                    AddVertexTangent(const float* pTangent);
        void                    AddVertexBinormal(const float* pBinormal);

        // setting & retrieving
        void                    AdvanceStripCounter(void)                           { m_nStripCounter++; }
        unsigned short          GetStripCounter(void)                               { return m_nStripCounter; }
        void                    ResetStripCounter(unsigned short nLodLevel);
        void                    SetVertexCounter(unsigned short nCounter)           { m_nVertexCounter = nCounter; }
        unsigned short          GetNumLodLevels( )                                  { return m_nNumLodLevels; }

        // PUT SANITY CHECKS HERE TO MAKE SURE THE VECTORS ARE ADVANCING IN SYNC
        void                    AdvanceVertexCounter(void)                          { m_nVertexCounter++; }
        unsigned short          GetVertexCounter(void)                              { return m_nVertexCounter; }
        void                    ResetVertexCounter(void)                            { m_nVertexCounter = 0; }


        ///////////////////////////////////////////////////////////////////////  
        //  Retrieving the geometry

        // indexing
        unsigned short          GetNumStrips(short sLodLevel) const;
        unsigned short*         GetStrip(unsigned short nLodLevel, unsigned short& nStripLength) const;
        const unsigned short*   GetStripLengthsPointer(short sLodLevel) const;
        unsigned short**        GetStripsPointer(unsigned short nLodLevel) const;

        // tables
        const unsigned long*    GetVertexColors(void) const                         { return m_vColors.empty( ) ? NULL : &(m_vColors[0]); }
        const float*            GetVertexCoords(void) const                         { return m_vCoords.empty( ) ? NULL : &(m_vCoords[0]); }
        const float*            GetVertexNormals(void) const                        { return m_vNormals.empty( ) ? NULL : &(m_vNormals[0]); }
        const float*            GetVertexBinormals(void) const                      { return m_vBinormals.empty( ) ? NULL : &(m_vBinormals[0]); }
        const float*            GetVertexTangents(void) const                       { return m_vTangents.empty( ) ? NULL : &(m_vTangents[0]); }
        const float*            GetVertexTexCoords0(void) const                     { return m_vTexCoords0.empty( ) ? NULL : &(m_vTexCoords0[0]); }
        const float*            GetVertexTexCoords1(void) const                     { return m_vTexCoords1.empty( ) ? NULL : &(m_vTexCoords1[0]); }
        const float*            GetVertexWindWeights(void) const                    { return m_vWindWeights.empty( ) ? NULL : &(m_vWindWeights[0]); }
        const unsigned char*    GetVertexWindMatrixIndices(void) const              { return m_vWindMatrixIndices.empty( ) ? NULL : &(m_vWindMatrixIndices[0]); }

        // vertex utility functions
        unsigned int            GetTriangleCount(unsigned short nLodLevel) const;
        const float*            GetVertexCoord(unsigned int nVertexIndex) const;
        unsigned short          GetVertexCount(void) const                          { return static_cast<unsigned short>(m_vCoords.size( ) / 3); }
        unsigned short          GetVertexSize(void) const                           { return m_nVertexSize; }
        const unsigned char*    GetVertexTexIndices(void) const                     { return m_vTexIndices0.empty( ) ? NULL : &(m_vTexIndices0[0]); }
        const float*            GetVertexTexCoord0(unsigned int nVertexIndex) const;


        ///////////////////////////////////////////////////////////////////////  
        //  Misc

        bool                    ComputeWindEffect(unsigned short nLodLevel);
        void                    DeleteLodStrip(unsigned short nLodLevel);

        void                    Invalidate(void)                                    { m_bValid = false; }
        bool                    IsValid(void) const                                 { return m_bValid; }
        void                    Validate(void)                                      { m_bValid = true; }

        void                    ChangeTexCoord(unsigned char chChangedMapIndex, float* pNewTexCoords);

private:
        // utility functions
        unsigned long           ColorFloatsToLong(const float* pColor) const        { return (int(pColor[0] * 255.0f) << 0) + (int(pColor[1] * 255.0f) << 8) + (int(pColor[2] * 255.0f) << 16) + 0xff000000; }
        void                    DeleteIndexData(void);
        const float*            GetOrigVertexCoord(unsigned int nVertexIndex) const;

        // general
        bool                    m_bRetainTexCoords;                 // used for remapping frond texture coordinates
        CWindEngine*            m_pWindEngine;                      // pointer used to modify branch geometry when WIND_CPU is used
        bool                    m_bVertexWeighting;                 // flag for vertex weight mode
        bool                    m_bManualLighting;                  // flag for manual lighting mode
        bool*                   m_pVertexWindComputed;              // for WIND_CPU - one entry per branch vertex to avoid redundant computations
        unsigned short          m_nVertexSize;                      // size of vertex in bytes
        bool                    m_bValid;                           // is geometry valid for current frame
        CSpeedTreeRT::EWindMethod m_eWindMethod;                    // method used for wind effects
                                                                    
        // geometry                                                 
        unsigned short          m_nNumLodLevels;                    // number of discrete branch LODs
        unsigned short          m_nVertexCounter;                   // current vertex - where values are stored when AddVertex*() functions are invoked
        unsigned short          m_nVertexCounterLodLevel;           // which LOD level the m_nVertexCount is referencing
        unsigned short          m_nStripCounter;                    // current strip - where strips are stored when AddStrip() is invoked
        std::vector<int>        m_vTriangleCounts;                  // used to store the number of triangles per LOD level
        std::vector<std::vector<unsigned short> > m_vStripLengths;  // each LOD level has multiple strips, the sizes are stored in m_vStripsLengths[nLod#][nStrip#]
        std::vector<std::vector<unsigned short*> > m_vStrips;       // each LOD level has multiple strips, pointers to those strips are stored in m_vStrips[nLod#][nStrip#]

        // new geometry storage
        std::vector<unsigned long> m_vColors;                       // table of RGBA colors, one color per entry
        std::vector<float>      m_vCoords;                          // table of xyz values, three floats per coordinate
        std::vector<float>      m_vOrigCoords;                      // table of xyz values used as a basis for CPU wind
        std::vector<float>      m_vNormals;                         // table of 3D normal values, three floats per normal
        std::vector<float>      m_vBinormals;                       // table of 3D binormal values, used for bump mapping, three floats per normal
        std::vector<float>      m_vTangents;                        // table of 3D tangent values, used for bump mapping, three floats per normal
        std::vector<float>      m_vTexCoords0;                      // table of S & T texture coordinates, two floats per coordinate (1st layer)
        std::vector<float>      m_vOrigTexCoords0;                  // used for composite map manipulation in CAD, not needed by end user
        std::vector<unsigned char> m_vTexIndices0;                  // used for composite map manipulation in CAD & MAX/MAYA, not needed by end user
        std::vector<float>      m_vTexCoords1;                      // table of S & T texture coordinates, two floats per coordinate (2nd layer)
        std::vector<float>      m_vWindWeights;                     // table of weights, values from from 0.0 for rigid to 1.0 for flexible
        std::vector<byte>       m_vWindMatrixIndices;               // table of wind matrix indices, ranging from 0 to num_wind_matrices
};


///////////////////////////////////////////////////////////////////////  
//  Inline function definitions

inline unsigned short CIndexedGeometry::GetNumStrips(short sLodLevel) const
{
    st_assert(sLodLevel < m_nNumLodLevels);
    return (sLodLevel > -1) ? static_cast<unsigned short>(m_vStrips[sLodLevel].size( )) : static_cast<unsigned short>(0);
}

inline const unsigned short* CIndexedGeometry::GetStripLengthsPointer(short sLodLevel) const
{
    return (sLodLevel < 0 || sLodLevel >= m_nNumLodLevels || m_vStripLengths[sLodLevel].empty( )) ? NULL : &(m_vStripLengths[sLodLevel][0]);
}
