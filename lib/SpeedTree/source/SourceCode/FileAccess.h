///////////////////////////////////////////////////////////////////////  
//         Name: FileAccess.h
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
#include "LibSpline_Source/IdvSpline.h"
#include "Endian.h"


///////////////////////////////////////////////////////////////////////  
//  enum EFileToken definition
//
//  Every piece of information written to an SPT file has a token preceeding
//  it.  The tokens are organized into groups as follows.

enum EFileToken
{
    // general tokens
    File_Unknown,

    File_BeginFile = 1000,
    File_EndFile,
    File_BeginTreeInfo,
    File_EndTreeInfo,
    File_BeginGeneralLeafInfo,
    File_EndGeneralLeafInfo,
    File_LeafLevelCount,
    File_BeginLeafLevel,
    File_EndLeafLevel,
    File_BeginSingleLeafInfo,
    File_EndSingleLeafInfo,
    File_BeginWindInfo,
    File_EndWindInfo,
    File_BranchLevelCount,
    File_BeginBranchInfo,
    File_EndBranchInfo,
    File_BeginBranchLevel,
    File_EndBranchLevel,
    File_String,

    // tree tokens
    File_Tree_BranchTexture = 2000,
    File_Tree_Far,
    File_Tree_LOD,
    File_Tree_Near,
    File_Tree_RenderStyle,
    File_Tree_RandomSeed,
    File_Tree_Size,
    File_Tree_Size_Variance,

    // general leaf tokens
    File_Leaf_Blossom_Distance = 3000,
    File_Leaf_Blossom_Level,
    File_Leaf_Blossom_Weighting,
    File_Leaf_ColorShift,
    File_Leaf_ColorShiftScalar,
    File_Leaf_LightScalar,
    File_Leaf_Lit,
    File_Leaf_SpacingTolerance,
    File_Leaf_CollisionDetection,
    File_Leaf_Dimming,
    File_Leaf_DimmingScalar,

    // single leaf tokens
    File_Leaf_Blossom = 4000,
    File_Leaf_Color,
    File_Leaf_ColorVariance,
    File_Leaf_Filename,
    File_Leaf_Origin,
    File_Leaf_Size,
    File_Leaf_SizeUsed,
    File_Leaf_Ratio,

    // wind tokens
    File_Wind_Direction = 5000,
    File_Wind_BranchOscillation,
    File_Wind_LeafOscillation,
    File_Wind_BranchFactors,
    File_Wind_LeafFactors,
    File_Wind_Strength,
    File_Wind_Enabled,

    // branch info
    File_Branch_Disturbance = 6000,
    File_Branch_EndAngle,
    File_Branch_Flexibility,
    File_Branch_FlexibilityScale,
    File_Branch_Length,
    File_Branch_Radius,
    File_Branch_RadiusScale,
    File_Branch_StartAngle,
    File_Branch_CrossSectionSegments,
    File_Branch_Segments,
    File_Branch_FirstBranch,
    File_Branch_LastBranch,
    File_Branch_Frequency,
    File_Branch_STile,
    File_Branch_TTile,
    File_Branch_STileAbsolute,
    File_Branch_TTileAbsolute,
    File_Branch_AngleProfile,

    // The following tokens were added to the original spt definition.
    // They must appear after "File_EndFile" to maintain backward compatibility.

    // leaf cluster info
    File_BeginLeafCluster = 7000,
    File_EndLeafCluster,
    File_BeginLeafLodLevel,
    File_EndLeafLodLevel,
    File_BeginBillboardLeaf,
    File_EndBillboardLeaf,
    File_BillboardLeaf_Color,
    File_BillboardLeaf_AngleIndex,
    File_BillboardLeaf_ColorScale,
    File_BillboardLeaf_FrequencyScalar,
    File_BillboardLeaf_Normal,
    File_BillboardLeaf_TextureIndex,
    File_BillboardLeaf_WindWeight,
    File_BillboardLeaf_WindGroup,
    File_BillboardLeaf_BranchPercent,
    File_BillboardLeaf_Pos,
    File_BillboardLeaf_TimeOffset,

    // lighting info
    File_BeginLightingInfo = 8000,
    File_EndLightingInfo,
    File_BranchLightingMethod,
    File_BranchMaterial,
    File_LeafLightingMethod,
    File_LeafMaterial,
    File_LeafLightingAdjustment,
    File_StaticLightingStyle,
    File_FrondLightingMethod,
    File_FrondMaterial,

    // lod info
    File_BeginLodInfo = 9000,
    File_EndLodInfo,
    File_LeafTransitionMethod,
    File_LeafTransitionRadius,
    File_LeafCurveExponent,
    File_BeginEngineLodInfo,
    File_EndEngineLodInfo,
    File_BranchNumLods,
    File_BranchMinVolumePercent,
    File_LeafSizeIncreaseFactor,
    File_LeafReductionFactor,
    File_LeafNumLods,
    File_BranchMaxVolumePercent,
    File_BranchReductionFuzziness,
    File_LargeBranchPercent,

    // texture coordinate info
    File_BeginTextureCoordInfo = 10000,
    File_EndTextureCoordInfo,
    File_LeafTextureCoords,
    File_BillboardTextureCoords,
    File_FrondTextureCoords,
    File_CompositeFilename,
    File_HorizontalBillboard,
    File_360Billboard,

    // wind info
    File_BeginNewWindInfo = 11000,
    File_EndNewWindInfo,
    File_WindLevel,

    // collision object info
    File_BeginCollisionInfo = 12000,
    File_EndCollisionInfo,
    File_CollisionSphere,
    File_CollisionCylinder,
    File_CollisionBox,

    // frond info
    File_BeginFrondInfo = 13000,
    File_EndFrondInfo,
    File_FrondLevel,
    File_FrondType,
    File_FrondNumBlades,
    File_FrondProfile,
    File_FrondProfileSegments,
    File_FrondEnabled,
    File_FrondNumTextures,
    File_FrondNumLodLevels,
    File_FrondMaxSurfaceAreaPercent,
    File_FrondMinSurfaceAreaPercent,
    File_FrondReductionFuzziness,
    File_FrondLargeFrondPercent,

    // frond textures
    File_BeginFrondTexture = 14000,
    File_EndFrondTexture,
    File_FrondTextureFilename,
    File_FrondTextureAspectRatio,
    File_FrondTextureSizeScale,
    File_FrondTextureMinAngleOffset,
    File_FrondTextureMaxAngleOffset,
    File_FrondMinLengthSegments,
    File_FrondMinCrossSegments,

    // texture controls
    File_BeginTextureControls = 15000,
    File_EndTextureControls,
    File_TextureOffset,
    File_TextureTwist,

    // flare info
    File_BeginFlareInfo = 16000,
    File_EndFlareInfo,
    File_FlareSegmentPackingExponent,
    File_FlareNumFlares,
    File_FlareBalance,
    File_FlareRadialInfluence,
    File_FlareRadialInfluenceVariance,
    File_FlareRadialExponent,
    File_FlareRadialDistance,
    File_FlareRadialDistanceVariance,
    File_FlareLengthDistance,
    File_FlareLengthDistanceVariance,
    File_LengthExponent,

    // flare seed
    File_FlareSeed,

    // new smooth_1 component
    File_LeafTransitionFactor,

    // shadow projection info
    File_BeginShadowProjectionInfo = 18000,
    File_EndShadowProjectionInfo,
    File_ShadowRightVector,
    File_ShadowUpVector,
    File_ShadowOutVector,
    File_ShadowMap,

    // user data
    File_BeginUserData = 19000,
    File_EndUserData,
    File_UserData,

    // file compatibility fix
    File_BeginSupplementalTexCoordInfo = 20000,
    File_EndSupplementalTexCoordInfo,
    File_SupplementalCompositeFilename,
    File_SupplementalHorizontalBillboard,
    File_Supplemental360Billboard,
    File_SupplementalShadowTexCoords,

    // speedwind support
    File_SpeedWindRockScalar = 21000,
    File_SpeedWindRustleScalar = 21001,

    // flexibility propagation
    File_PropagateFlexibility = 22000
};


///////////////////////////////////////////////////////////////////////  
//  class EFileToken declaration

class CTreeFileAccess
{
public:
        CTreeFileAccess( ) :
            m_nIndex(0)
        {
        }

        CTreeFileAccess(const unsigned char* pMemory, int nNumBytes) :
            m_nIndex(0)
        {
            m_vMemory.resize(nNumBytes);
            for (int i = 0; i < nNumBytes; ++i)
                m_vMemory[i] = pMemory[i];
        }

        unsigned char*          ConvertMemoryToArray(unsigned int& nNumBytes);

        bool                    ParseBool(void)                                 { return m_vMemory[m_nIndex++] != 0; }
        CIdvBezierSpline*       ParseBranchParameter(void)                      { return new CIdvBezierSpline(ParseString( )); }
        unsigned char           ParseByte(void)                                 { return m_vMemory[m_nIndex++]; }
        float                   ParseFloat(void);
        int                     ParseInt(void);
        unsigned long           ParseLong(void);
        short                   ParseShort(void);
        std::string             ParseString(void);
        EFileToken              ParseToken(void)                                { return static_cast<EFileToken>(ParseInt( )); }
        CVec                    ParseVector(int nSize);
        CVec3                   ParseVector3(void);

        void                    Rewind(void)                                    { m_nIndex = 0; }

        void                    SaveBool(bool bBool)                            { m_vMemory.push_back((unsigned char) (bBool)); }
        void                    SaveBranchParameter(const CIdvBezierSpline* pInfo) { SaveString(pInfo->Save( )); }
        void                    SaveByte(unsigned char chByte)                  { m_vMemory.push_back((unsigned char) (chByte)); }
        void                    SaveFloat(float fFloat);
        void                    SaveInt(int nInt);
        void                    SaveLong(unsigned long ulLong);
        void                    SaveShort(short sShort);
        void                    SaveString(std::string strString);
        void                    SaveToken(EFileToken eToken)                    { SaveInt(static_cast<int>(eToken)); }
        void                    SaveVector(const CVec& cVec, int nSize);
        void                    SaveVector3(const CVec3& cVec3);    

        EFileToken              PeekToken(void);
        bool                    EndOfFile(void) const                           { return (unsigned int) m_nIndex >= m_vMemory.size( ); }


private:
        int                         m_nIndex;           // index of current position in m_vMemory vector
        std::vector<unsigned char>  m_vMemory;          // used to store SPT file contents in RAM
};



