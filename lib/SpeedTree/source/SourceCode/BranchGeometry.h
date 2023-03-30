//         Name: BranchGeometry.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
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
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may not 
//  be copied or disclosed except in accordance with the terms of that 
//  agreement.

#pragma once
#include "Debug.h"
#include "LibVector_Source/IdvVector.h"
typedef unsigned char byte;


///////////////////////////////////////////////////////////////////////  
//  class CBranchGeometry declaration
//
//  This class is used as a geometry abstraction mechanism.  The branches are
//  computed in the CIdvBranch and CTreeEngine classes, but the geometry
//  is stored and accessed with CBranchGeometry.  It contains the code
//  necessary for formatting the branch data for use within different rendering
//  APIs and/or coordinate systems.

class CBranchGeometry
{
public:
        CBranchGeometry(CWindEngine* pWindEngine, bool bRetainTexCoords = false);
        ~CBranchGeometry( );

        ///////////////////////////////////////////////////////////////////////  
        //  Setting the geometry

        // general
        void                    EnableManualLighting(bool bFlag)                    { m_bManualLighting = bFlag; }
        void                    EnableVertexWeighting(bool bFlag)                   { m_bVertexWeighting = bFlag; }
        void                    CombineStrips(void);
        void                    ComputeExtents(CRegion& cExtents) const;
        bool                    IsVertexWeightingEnabled(void) const                { return m_bVertexWeighting; }
        void                    SetNumLodLevels(unsigned short nLevels);
        void                    Transform(const CTransform& cTransform);

        // indexing 
        void                    SetNumStrips(unsigned short nLodLevel, unsigned short nNumStrips);
        void                    AddStrip(unsigned short nLodLevel, unsigned short* pStrip, unsigned short nStripLength, unsigned int nTriangleCount);

        // vertex 
        void                    SetNumVertices(unsigned short nNumVertexes);
        void                    AddVertexColor(const float* pColor);                            // 4 floats: (r, g, b, a)
        void                    AddVertexCoord(const float* pCoord);                            // 3 floats: (x, y, z)
        void                    AddVertexNormal(const float* pNormal);                          // 3 floats: (nx, ny, nz)
        void                    AddVertexTexCoord(const float* pTexCoord, short sIndex = -1);   // 2 floats: (s, t)
        void                    AddVertexWind(float fWeight, unsigned char chGroup);

        // setting & retrieving
        void                    AdvanceStripCounter(void)                           { m_nStripCounter++; st_assert(m_pNumStrips); st_assert(m_nStripCounter <= m_pNumStrips[m_nVertexCounterLodLevel]); }
        unsigned short          GetStripCounter(void)                               { return m_nStripCounter; }
        void                    ResetStripCounter(unsigned short nLodLevel);
        void                    SetVertexCounter(unsigned short nCounter)           { m_nVertexCounter = nCounter; }
        unsigned short          GetNumLodLevels( )                                  { return m_nNumLodLevels; }

        void                    AdvanceVertexCounter(void)                          { m_nVertexCounter++; st_assert(m_nVertexCounter <= m_nNumVertices); }
        unsigned short          GetVertexCounter(void)                              { return m_nVertexCounter; }
        void                    ResetVertexCounter(void)                            { m_nVertexCounter = 0; }

        ///////////////////////////////////////////////////////////////////////  
        //  Retrieving the geometry

        // indexing
        unsigned short          GetNumStrips(unsigned short nLodLevel) const                            { st_assert(nLodLevel < m_nNumLodLevels); st_assert(m_pNumStrips); return m_pNumStrips[nLodLevel]; }
        unsigned short*         GetStrip(unsigned short nLodLevel, unsigned short& nStripLength) const;
        unsigned short*         GetStripLengthsPointer(unsigned short nLodLevel) const                  { st_assert(nLodLevel < m_nNumLodLevels); st_assert(m_pStripLengths); return m_pStripLengths[nLodLevel]; }
        unsigned short**        GetStripsPointer(unsigned short nLodLevel) const                        { st_assert(nLodLevel < m_nNumLodLevels); st_assert(m_pStrips); return m_pStrips[nLodLevel]; }
#if defined SPEEDTREE_DATA_FORMAT_DIRECTX || defined SPEEDTREE_DATA_FORMAT_NETIMMERSE
        unsigned short*         GetContinuousStrip(unsigned short nLodLevel, unsigned int& nTotalTriangleCount);
#endif

        // vertex
        unsigned int            GetTriangleCount(unsigned short nLodLevel) const;
        float*                  GetVertexCoord(unsigned int nVertexIndex) const;
        unsigned short          GetVertexCount(void) const                          { return m_nNumVertices; }
        unsigned short          GetVertexSize(void) const                           { return m_nVertexSize; }
        unsigned char*          GetVertexTexIndices(void) const                     { return m_pTexIndices; }
        float*                  GetVertexTexCoord(unsigned int nVertexIndex) const;

        // tables
#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
        byte*                   GetVertexTable(void) const                          { return m_pInterleavedVertexes; }
#else
        float*                  GetVertexColorTable(void) const                     { return m_pColors; }
        float*                  GetVertexCoordTable(void) const                     { return m_pCoords; }
        float*                  GetVertexNormalTable(void) const                    { return m_pNormals; }
        float*                  GetVertexTexCoordTable(void) const                  { return m_pTexCoords; }
        float*                  GetVertexWindWeights(void) const                    { return m_pWeights; }
        unsigned char*          GetVertexWindGroups(void) const                     { return m_pWindGroups; }
#endif
        
        ///////////////////////////////////////////////////////////////////////  
        //  Misc

        bool                    ComputeWindEffect(unsigned short nLodLevel);
        void                    DeleteLodStrip(unsigned short nLodLevel);

        void                    Invalidate(void)                                    { m_bValid = false; }
        bool                    IsValid(void) const                                 { return m_bValid; }
        void                    Validate(void)                                      { m_bValid = true; }

        void                    ChangeTexCoord(unsigned char chChangedMapIndex, float* pNewTexCoords);

private:
        unsigned long           ColorFloatsToLong(const float* pColor) const        { return (int(pColor[0] * 255.0f) << 0) + (int(pColor[1] * 255.0f) << 8) + (int(pColor[2] * 255.0f) << 16) + 0xff000000; }
        void                    DeleteAll(void);
        void                    DeleteIndexData(void);
        float*                  GetOrigVertexCoord(unsigned int nVertexIndex) const;

        // general
        bool                    m_bRetainTexCoords;     // used for remapping frond texture coordinates
        CWindEngine*            m_pWindEngine;          // pointer used to modify branch geometry when WIND_CPU is used
        bool                    m_bVertexWeighting;     // flag for vertex weight mode
        bool                    m_bManualLighting;      // flag for manual lighting mode
        bool*                   m_pVertexWindComputed;  // for WIND_CPU - one entry per branch vertex to avoid redundant computations
        unsigned short          m_nVertexSize;          // size of vertex in bytes
        bool                    m_bValid;               // is geometry valid for current frame

        // geometry
        unsigned short          m_nNumLodLevels;        // number of discrete branch LODs
        unsigned short          m_nNumVertices;         // total number of vertices, shared by all LODs
        unsigned short          m_nVertexCounter;       // current vertex - where values are stored when AddVertex*() functions are invoked
        unsigned short          m_nVertexCounterLodLevel; // which LOD level the m_nVertexCount is referencing
        unsigned short*         m_pNumStrips;           // number of strips, one entry for each separate LOD
        unsigned short          m_nStripCounter;        // current strip - where strips are stored when AddStrip() is invoked
        unsigned short**        m_pStripLengths;        // each LOD has numerous strips, lengths for all LODs are stored here
        unsigned short***       m_pStrips;              // actual strips for all LODs
        float*                  m_pOrigCoords;          // used as basis for wind
        float*                  m_pOrigTexCoords;       // 2 times num vertices
        unsigned char*          m_pTexIndices;          // num vertices

        unsigned int*           m_pTriangleCounts;

#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
        byte*                   m_pInterleavedVertexes; // interleaved branch data used for OpenGL and DirectX
#else
        float*                  m_pColors;              // colors, three values per vertex (r, g, b)
        float*                  m_pCoords;              // coordinates, three values per vertex (x, y, z)
        float*                  m_pNormals;             // normals, three values per vertex (nx, ny, nz)
        float*                  m_pTexCoords;           // textures, two values per vertex (s, t)
#endif
        float*                  m_pWeights;             // wind weights, one value per vertex
        byte*                   m_pWindGroups;          // wind groups, one value per group
};
