///////////////////////////////////////////////////////////////////////  
//         Name: TreeEngine.h
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

#include "LibGlobals_Source/IdvGlobals.h"
#include "LibFilename_Source/IdvFilename.h"
#include "LibVector_Source/IdvVector.h"
#define IDV_SPLINE_NO_GUI // disable GUI components of spline library
#include "LibSpline_Source/IdvSpline.h"
#include "LibRandom_Source/IdvRandom.h"
#include "StructsInfo.h"
#include "StructsSupport.h"
#include "FileAccess.h"


//
//  forward references
//

class CBillboardLeaf;
struct SIdvLeaf;
class CIndexedGeometry;
class CLightingEngine;
class CFrondEngine;


///////////////////////////////////////////////////////////////////////  
//  class CIdvCamera declaration

class CIdvCamera
{
public:
        CIdvCamera( );
        CIdvCamera(const CIdvCamera& cTrans);
virtual ~CIdvCamera( );

        //
        // geometry/position related functions
        //
static  void                    SetCamera(const CVec3& cCameraPos, const CVec3& cCameraDir) { m_cCameraPos = cCameraPos; m_cCameraDir = cCameraDir; }
static  void                    GetCamera(CVec3& cCameraPos, CVec3& cCameraDir)             { cCameraPos = m_cCameraPos; cCameraDir = m_cCameraDir; }

        const CVec3&            GetPosition(void) const                                     { return m_cPosition; }
        void                    SetPosition(const CVec3& cPosition)                         { m_cPosition = cPosition; }

virtual const CIdvCamera&       operator=(const CIdvCamera& cRight);

protected:      
static  CVec3                   m_cCameraPos;               // one camera position shared by the entire library
static  CVec3                   m_cCameraDir;               // one camera direction shared by the entire library
        CVec3                   m_cPosition;                // one position for each derived instance
};


///////////////////////////////////////////////////////////////////////  
//  class CIdvBranch declaration

class CIdvBranch
{
public:
        CIdvBranch(CIdvBranch* pParent);
        ~CIdvBranch( );

        // branch construction
        void                        Compute(unsigned int nSeed, float fSize, int nLevel, const CVec3& cBasePos, float fPercentOfParent, const CRotTransform& cBaseTransform, const CVec3& cZAxis, CIndexedGeometry* pBranchGeometry, vector<CBillboardLeaf*>& vLeaves, float fWindWeight, int nWindGroup, float fParentRadius);
        void                        ComputeBud(const CIdvBranch* pParent, float fSize, int nLevel, const CVec3& cBasePos, float fPercentOfParent, const CRotTransform& cBaseTransform, const CVec3& cZAxis, vector<CBillboardLeaf*>& vLeaves, float fWindWeight, int nWindGroup) const;
        void                        ComputeLod(int nDiscreteLodLevel, CIndexedGeometry* pGeometry) const;

        // wind
static  void                        SetBranchLevelForWeighting(int nLevel)                  { m_nWeightLevel = nLevel; }

        // LOD
        void                        BuildBranchVector(vector<CIdvBranch*>& vAllBranches);
static  void                        SortBranchVector(vector<CIdvBranch*>& vAllBranches);

        // utility
static  void                        AddBranchInfo(const SIdvBranchInfo* pInfo)              { m_vBranchInfo.push_back(pInfo); }
static  void                        ClearBranchInfo(void)                                   { m_vBranchInfo.clear( ); }
static  const vector<const SIdvBranchInfo*>& GetBranchInfoVector(void)                      { return m_vBranchInfo; }
        const CIdvBranch*           GetParent(void) const                                   { return m_pParent; }
        float                       GetVolume(void) const                                   { return m_fVolume; }
        float                       GetFuzzyVolume(void) const                              { return m_fFuzzyVolume; }
        void                        SetFuzzyVolume(const float fVolume)                     { m_fFuzzyVolume = fVolume; }
static  void                        Parse(CTreeFileAccess& cFile);
static  void                        SetComputeLeaves(bool bState)                           { m_bComputeLeaves = bState; }
static  void                        SetLeafData(SIdvLeafInfo* pLeafInfo)                    { m_pLeafInfo = pLeafInfo; }
static  void                        SetLightingEngine(CLightingEngine* pLightingEngine)     { m_pLightingEngine = pLightingEngine; }
static  void                        SetFrondEngine(CFrondEngine* pFrondEngine)              { m_pFrondEngine = pFrondEngine; }
static  void                        SetRoundRobinWindLevel(int nLevel)                      { m_nRoundRobinWindLevel = nLevel; }
static  void                        SetPropagateFlexibility(bool bState)                    { m_bPropagateFlexibility = bState; }
static  bool                        GetPropagateFlexibility( )                              { return m_bPropagateFlexibility; }

private:
        // leaf-related
        void                        BuildBlossomVectors(void);
        bool                        IsBlossom(const CIdvBranch* pParent, float fPercentOfParent) const;
        void                        MakeLeaf(const CVec3& cPos, float fPercent, const CIdvBranch* pParent, const CVec3& cNormal, const CVec3& cParentDir, float fWindWeight, int nWindGroup, vector<CBillboardLeaf*>& vLeaves) const;
        bool                        RoomForLeaf(const CVec3& cPos, int nTextureIndex, const vector<CBillboardLeaf*>& vLeaves) const;

        // utility
        void                        BuildCrossSection(const SIdvBranchVertex* pVertex, float fBranchProgress, int nSegments, CIndexedGeometry* pBranchGeometry, const CVec3& cTile, float fWindWeight, int nWindGroup, float fTileOffset) const;
        void                        ComputeBranchNormals(CIndexedGeometry* pBranchGeometry, unsigned short usSegments);
        float                       ComputeVertexWeight(int nLevel, float fProgress, float fFlexibility);
        float                       ComputeHighLevelVertexWeight(float fBaseWeight, float fFlexibility);
        void                        ComputeVolume(void);
        void                        FillBranch(SIdvBranch& sBranch, float fLength) const;
        float                       GetPercentOfParent(void) const                          { return m_fPercentOfParent; }

        // flare related
        void                        ComputeFlareEntries(const SIdvBranchInfo& sInfo);

        CIdvBranch*                 m_pParent;                  // parent branch
        float                       m_fPercentOfParent;         // percentage along parent branch's length this branch was created
        vector<SIdvBranch>          m_vChildren;                // list of this branch's children

        SIdvBranchVertex*           m_pVertices;                // used by leaf bud
        int                         m_nVertexCount;             // used by leaf bud (can be replaced by constant)

        unsigned short              m_usCrossSectionSegments;   // radial resolution
        int                         m_nStartingOffset;          // marks the start of this branch's data within the branch geometry class
        float                       m_fVolume;                  // volume of this branch's geometry
        float                       m_fFuzzyVolume;             // fuzzy volume used to influence sorting order for lod branch removal

        // flare related
        vector<SIdvBranchFlare>     m_vFlares;                  // branch flares

        // the following variables are static because they need to be shared
        // across all branch levels for a single tree
static  vector<int>                 m_vBlossoms;                // a list of which leaf textures represent blossoms
static  vector<int>                 m_vNonBlossoms;             // a list of which leaf textures do not represent blossoms

static  vector<const SIdvBranchInfo*>   m_vBranchInfo;          // all of the user-specified information for creating each branch level
static  bool                        m_bComputeLeaves;           // set to false is leaves have been read from file, true is they should be generated
static  SIdvLeafInfo*               m_pLeafInfo;                // pointer to struct with leaf information
static  vector<CBillboardLeaf*>     m_vLocalLeaves;             // a list of leaves assigned to a single branch - used in BRANCH collision mode

static  CLightingEngine*            m_pLightingEngine;          // pointer to SpeedTreeRT lighting engine, used to set static branch colors
static  CIdvRandom                  m_cRandom;                  // common single random number generator            
static  int                         m_nWeightLevel;             // level at which wind effect takes place
static  int                         m_nRoundRobinWindLevel;     // used to ensure a good wind matrix distribution

        // frond
static  CFrondEngine*               m_pFrondEngine;             // pointer to SpeedTreeRT frond engine

        // wind
static  bool                        m_bPropagateFlexibility;    // allow levels higher than the wind level to retain flexibility
};



///////////////////////////////////////////////////////////////////////  
//  class CTreeEngine declaration

class CTreeEngine : public CIdvCamera
{
public:
        CTreeEngine(CIndexedGeometry* pBranchGeometry);

        // tree construction
        void                        Compute(float fLeafSizeIncreaseFactor);
        void                        Clone(CTreeEngine* pClone, const CVec3& cPos, unsigned int nSeed = 0) const;
        void                        OverrideTreeSize(float fNewSize, float fNewVariance);

        bool                        Parse(CTreeFileAccess& cFile);
        void                        ParseTreeInfo(CTreeFileAccess& cFile);
        void                        Save(CTreeFileAccess& cFile, bool bSaveLeafLods = false) const;

        void                        FreeTransientData(void);
        bool                        TransientDataIntact(void) const                 { return m_bTransientDataIntact; }

        // leaves
        vector<CBillboardLeaf*>*    GetAllLeaves(void) const                        { return m_vLeafLods; }
        const SIdvLeafInfo&         GetLeafInfo(void) const                         { return m_sLeafInfo; }
        const vector<SIdvLeafTexture>& GetLeafTextures(void) const                  { return m_sLeafInfo.m_vLeafTextures; }
        void                        InitTables( )                                   { m_sLeafInfo.InitTables(m_sLeafInfo.m_vLeafTextures.size( )); }
        void                        SetNumLeafRockingGroups(int nCount)             { m_sLeafInfo.m_nNumRockingGroups = nCount; }

        // wind
        const SIdvWindInfo&         GetWindData(void) const                         { return m_sWindInfo; }
        void                        SetBranchLevelForWeighting(int nLevel)          { m_nBranchesWeightLevel = nLevel; }
        int                         GetBranchLevelForWeighting( )                   { return m_nBranchesWeightLevel; }
        void                        SetWind(float fStrength);
        void                        SetWindData(const SIdvWindInfo& sWindInfo)      { m_sWindInfo = sWindInfo; }

        // utility
        void                        ClearBranchInfo(void)                           { m_vBranchInfo.clear( ); }
        const vector<SIdvBranchInfo*>& GetBranchInfoVector(void)                    { return m_vBranchInfo; }
        const CIdvFilename&         GetBranchTextureFilename(void) const            { return m_sTreeInfo.m_strBranchTextureFilename; }
        int                         GetSeed(void) const                             { return m_sTreeInfo.m_nSeed; }
        void                        GetSize(float& fSize, float& fVariance);
        void                        SetSize(float fSize, float fVariance);
        void                        SetBranchTextureFilename(const char* pFilename);
        void                        SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename);

        // lod
        float                       ComputeLod(void);
        void                        SetLod(float fLod)                              { m_fLod = fLod; }
        float                       GetLod(void) const                              { return m_fLod; }
        void                        GetLodLimits(float& fNear, float& fFar) const;
        void                        SetLodLimits(float fNear, float fFar);

        int                         GetNumLeafLodLevels(void) const                 { return m_sLeafInfo.m_nNumLeafLodLevels; }
        void                        SetNumLeafLodLevels(unsigned int nNumLevels)    { m_sLeafInfo.m_nNumLeafLodLevels = nNumLevels; }
        unsigned int                GetNumBranchLodLevels(void) const               { return m_nNumBranchLodLevels; }
        void                        SetNumBranchLodLevels(unsigned int nNumLevels)  { m_nNumBranchLodLevels = nNumLevels; }

        void                        SetBranchVolumeLimits(float fMin, float fMax)   { m_fMinBranchVolumePercent = fMin; m_fMaxBranchVolumePercent = fMax; }

        // file I/O
        void                        SaveLodInfo(CTreeFileAccess& cFile) const;
        void                        ParseLodInfo(CTreeFileAccess& cFile);
        void                        SaveTextureControls(CTreeFileAccess& cFile) const;
        void                        ParseTextureControls(CTreeFileAccess& cFile);
        void                        SaveFlareInfo(CTreeFileAccess& cFile) const;
        void                        ParseFlareInfo(CTreeFileAccess& cFile);
        void                        SaveFlareSeed(CTreeFileAccess& cFile) const;
        void                        ParseFlareSeed(CTreeFileAccess& cFile);

        // general
        void                        SetSeed(unsigned int nSeed);
private:

        // branch
        void                        AddBranchInfo(SIdvBranchInfo* pInfo)            { m_vBranchInfo.push_back(pInfo); }
        void                        BuildBranchLods(void);
        void                        ParseBranchInfo(CTreeFileAccess& cFile);
        void                        SaveBranchInfo(CTreeFileAccess& cFile) const;
        void                        SetBranchTexture(const CIdvFilename& strFilename)   { m_sTreeInfo.m_strBranchTextureFilename = CIdvFilename(strFilename); }

        // leaves
        void                        BuildLeafLods(float fLeafSizeIncreaseFactor);
        void                        ParseLeafCluster(CTreeFileAccess& cFile);
        void                        SaveLeafCluster(CTreeFileAccess& cFile) const;
        void                        SetLeafTextures(const vector<SIdvLeafTexture>& vTextures)   { m_sLeafInfo.m_vLeafTextures = vTextures; }

        // general
        float                       m_fBillboardSize;                   // size, based on m_cExtents, of a single billboard at lowest LOD
        float                       m_fLod;                             // current LOD value 0.0 = minimum LOD, 1.0 = maximum LOD
        float                       m_fOverrideTreeSize;                // value to use for tree size instead of loaded
        float                       m_fOverrideTreeVariance;            // value to use for tree size variance instead of loaded
        CIdvRandom                  m_cRandom;                          // general-use random number generator
        bool                        m_bTransientDataIntact;             // flag set to true when DeleteTransientData() is invoked
        SIdvTreeInfo                m_sTreeInfo;                        // structure containing a few tree-defining variables

        // branch
        CIdvBranch*                 m_pBranch;                          // pointer to trunk branch, the top of the branch tree
        CIndexedGeometry*           m_pBranchGeometry;                  // branch geometry container class
        vector<SIdvBranchInfo*>     m_vBranchInfo;                      // SpeedTreeCAD branch information for each level of the branches
        unsigned int                m_nNumBranchLodLevels;              // number of discrete branch levels of detail

        // leaves
        vector<CBillboardLeaf*>     m_vBillboardLeaves;                 // vector of all leaves created for tree (highest LOD level)
        SIdvLeafInfo                m_sLeafInfo;                        // structure containing information that define the leaves
        vector<CBillboardLeaf*>*    m_vLeafLods;                        // an array of vectors that contain all of the discrete leaf LODs
        bool                        m_bParsedLeafLods;                  // flag indicating if the leaf LODs were read from the SPT file

        // lod
        float                       m_fMinBranchVolumePercent;          // the last branch LOD's total volume should not be below this
        float                       m_fMaxBranchVolumePercent;          // the first branch LOD's total volume should not be below this
        float                       m_fLeafReductionPercentage;         // percentage of leaves removed at each lod level
        float                       m_fBranchReductionFuzziness;        // controls how strictly branch volume is used for branch removal (0 = branch volume, 1 = random)
        float                       m_fLargeBranchPercent;              // all branches within this percent of the biggest branch will remain at highest lod

        // wind
        int                         m_nBranchesWeightLevel;             // wind effects begin at this branch level
        SIdvWindInfo                m_sWindInfo;                        // structure containing wind information
};
