///////////////////////////////////////////////////////////////////////  
//         Name: StructsInfo.h
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

#define IDV_SPLINE_NO_GUI // disable GUI components of spline library
#include "LibSpline_Source/IdvSpline.h"
#include "LibFilename_Source/IdvFilename.h"


//
//  forward references
//

class CTreeFileAccess;
struct SIdvLeafTexture;


///////////////////////////////////////////////////////////////////////  
//  struct SIdvBranchInfo declaration
//
//  There is one instance of SIdvBranchInfo for each branch level used
//  to create the tree.

struct SIdvBranchInfo
{
        SIdvBranchInfo( );
        SIdvBranchInfo(const SIdvBranchInfo& sRight);
        ~SIdvBranchInfo( );

        SIdvBranchInfo&         operator=(const SIdvBranchInfo& sInfo);
static  SIdvBranchInfo*         Parse(CTreeFileAccess& cFile);
        void                    Save(CTreeFileAccess& cFile) const;

        // spline class get/sets
        const CIdvBezierSpline* GetDisturbance(void) const                  { return m_pDisturbance; }
        const CIdvBezierSpline* GetGravity(void) const                      { return m_pGravity; }
        const CIdvBezierSpline* GetFlexibility(void) const                  { return m_pFlexibility; }
        const CIdvBezierSpline* GetFlexibilityScale(void) const             { return m_pFlexibilityScale; }
        const CIdvBezierSpline* GetLength(void) const                       { return m_pLength; }
        const CIdvBezierSpline* GetRadius(void) const                       { return m_pRadius; }
        const CIdvBezierSpline* GetRadiusScale(void) const                  { return m_pRadiusScale; }
        const CIdvBezierSpline* GetStartAngle(void) const                   { return m_pStartAngle; }
        const CIdvBezierSpline* GetAngleProfile(void) const                 { return m_pAngleProfile; }
        void                    SetDisturbance(CIdvBezierSpline* pSpline);
        void                    SetGravity(CIdvBezierSpline* pSpline);
        void                    SetFlexibility(CIdvBezierSpline* pSpline);
        void                    SetFlexibilityScale(CIdvBezierSpline* pSpline);
        void                    SetLength(CIdvBezierSpline* pSpline);
        void                    SetRadius(CIdvBezierSpline* pSpline);
        void                    SetRadiusScale(CIdvBezierSpline* pSpline);
        void                    SetStartAngle(CIdvBezierSpline* pSpline);
        void                    SetAngleProfile(CIdvBezierSpline* pSpline);

        // resolution-specifiers
        int                     m_nCrossSectionSegments;    // number of circular/radial points used in each branch
        int                     m_nSegments;                // number of points used along the length of the branch

        // branch generation
        float                   m_fFirstBranch;             // percentage along parent's length where children branch start being created
        float                   m_fLastBranch;              // percentage along parent's length where children branch strop being created
        float                   m_fFrequency;               // affects quantity of children branches created

        // tiling/texture controls
        float                   m_fSTile;                   // number of texture repeats around the branch
        float                   m_fTTile;                   // number of texture repeats along the length of the branch
        bool                    m_bSTileAbsolute;           // determines if the tiling based on the radius of the branch or absolute
        bool                    m_bTTileAbsolute;           // determines if the tiling based on the length of the branch or absolute
        bool                    m_bRandomTCoordOffset;      // start map tiling at random T texture coordinate
        float                   m_fTwist;                   // number of times the texture is twisted at the end

        // base flaring controls
        float                   m_fSegmentPackingExponent;  // controls how length segments are distributed along a branch
        int                     m_nNumFlares;               // number of flares
        float                   m_fFlareBalance;            // controls spacing between flares
        float                   m_fRadialInfluence;         // angular measure of each flare's influence
        float                   m_fRadialInfluenceVariance; // variance of radial influence
        float                   m_fRadialExponent;          // controls sharpness of flare points
        float                   m_fRadialDistance;          // distance flare protrudes (percentage of branch radius)
        float                   m_fRadialVariance;          // variance of radial distance
        float                   m_fLengthDistance;          // distance up the branch the flare is visible (percentage of branch length)
        float                   m_fLengthVariance;          // variance of length distance
        float                   m_fLengthExponent;          // controls lengthwise profile of flare segments

private:
        CIdvBezierSpline*       m_pDisturbance;             // used to apply a random angle variation at each segment along the length of the branch
        CIdvBezierSpline*       m_pGravity;                 // controls the max gravitational influence possible along the length of the branch
        CIdvBezierSpline*       m_pFlexibility;             // controls the max flexibility (wind influence) of the branch's children along the length of the branch
        CIdvBezierSpline*       m_pFlexibilityScale;        // controls how the single flexibility value of a branch is scaled along the branch length
        CIdvBezierSpline*       m_pLength;                  // the length varies according to its place along its parent
        CIdvBezierSpline*       m_pRadius;                  // the radius varies according to its place along its parent
        CIdvBezierSpline*       m_pRadiusScale;             // controls how the single radius value of a branch is scaled along the branch length
        CIdvBezierSpline*       m_pStartAngle;              // controls the angle at which new branches grow relative to the parent is growing
        CIdvBezierSpline*       m_pAngleProfile;            // controls (along w/ m_pGravity) how the branch grows from base to tip
};


///////////////////////////////////////////////////////////////////////  
//  struct SIdvLeafInfo declaration
//
//  There is one instance of SIdvLeafInfo per tree that encapsulates most of
//  of the leaf information.

struct SIdvLeafInfo
{
        enum ECollisionType
        {
            NONE, BRANCH, TREE
        };

        SIdvLeafInfo( );
        ~SIdvLeafInfo( );

        void                    InitTables(int nNumTextures);
        void                    Parse(CTreeFileAccess& cFile);
        void                    Save(CTreeFileAccess& cFile) const;


        //  color
        bool                    m_bDimming;                 // determines if the leaves get darker as the approach the tree center
        float                   m_fDimmingScalar;           // if the leaves are to be dimmed, this determines how much darker

        //  general leaves
        ECollisionType          m_eCollisionType;           // when leaves are generated, the collision type determines which algorithm is used
        vector<SIdvLeafTexture> m_vLeafTextures;            // a list of the different leaf textures used
        float                   m_fSpacingTolerance;        // determines how close together leaves can be

        //  blossom
        float                   m_fBlossomDistance;         // how far along a branch's length the blossoms start to occur
        float                   m_fBlossomWeighting;        // determines the percentage of leaves that become blossoms that are eligible
        int                     m_nBlossomLevel;            // controls the branch level to which the distance parameter applies for blossoms and
                                                            // the dimming level applies for dimming calculations

        //  table-related
        int                     m_nNumRockingGroups;        // billboard leaves are in groups that "rock" together - this controls how many groups
        int                     m_nNumLeafLodLevels;        // controls how many discrete number of leaf LOD levels
        int                     m_nNumTextures;             // the number of unique leaf textures used to create the tree
        float**                 m_pLeafVertexes;            // [lod][tex_i * (m_nNumRockingGroups * 20) + angle_i * 20 + corner_i]
        float*                  m_pLeafTexCoords;           // [texture * 8 points to beginning of 8-entry floats]
        float*                  m_pTimeOffsets;             // each rocking group has a different time offset so they don't appear synchronized
};


///////////////////////////////////////////////////////////////////////  
//  struct SIdvWindInfo declaration
//
//  There is one instance of SIdvWindInfo per tree.

struct SIdvWindInfo
{
        enum EOscillation
        {
            MIN, MAX, FREQUENCY
        };

        SIdvWindInfo( );

        void    Parse(CTreeFileAccess& cFile);
        void    Save(CTreeFileAccess& cFile) const;

        CVec3   m_cLeafFactors;                             // [0] = amount of leaf oscillation, [1] = frequency of leaf oscillation
        CVec3   m_cLeafOscillation;                         // [0] = -leaf throw, [1] = leaf throw, [2] = leaf oscillation
        float   m_fStrength;                                // strength of wind from 0.0 to 1.0
};


///////////////////////////////////////////////////////////////////////  
//  struct SIdvTreeInfo declaration
//
//  There is one instance of SIdvTreeInfo per tree.

struct SIdvTreeInfo
{
        SIdvTreeInfo( );

        CIdvFilename            m_strBranchTextureFilename; // filename of the branch texture map

        float                   m_fFar;                     // far LOD distance
        float                   m_fNear;                    // near LOD distance

        int                     m_nSeed;                    // random seed used to compute tree
        float                   m_fSize;                    // global size scalar for tree
        float                   m_fSizeVariance;            // amount of desired size variance
        int                     m_nFlareSeed;               // seed used to compute flares
};
