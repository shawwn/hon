///////////////////////////////////////////////////////////////////////  
//         Name: LeafLighting.h
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
#include "SpeedTreeRT.h"
#include "BillboardLeaf.h"
#include "Debug.h"

// because the static lighting model is software based, the maximum number of
// lights is theoretically boundless.
const int c_nMaxLights = 8;

// light property constants
const int c_nLightPos = 0;
const int c_nLightDiffuse = c_nLightPos + 3;
const int c_nLightAmbient = c_nLightDiffuse + 3;
const int c_nLightSpecular = c_nLightAmbient + 3;
const int c_nLightDirectional = c_nLightSpecular + 3;
const int c_nLightAttenuation = c_nLightDirectional + 1;
const int c_nLightParamSize = c_nLightAttenuation + 3;

// material constants
const int c_nMaterialDiffuse = 0;
const int c_nMaterialAmbient = c_nMaterialDiffuse + 3;
const int c_nMaterialSpecular = c_nMaterialAmbient + 3;
const int c_nMaterialEmissive = c_nMaterialSpecular + 3;
const int c_nMaterialShininess = c_nMaterialEmissive + 3;
const int c_nMaterialParamSize = c_nMaterialShininess + 1;

// convenience typedefs
typedef CSpeedTreeRT::ELightingMethod LightMethod;
typedef CSpeedTreeRT::EStaticLightingStyle StaticStyle;


///////////////////////////////////////////////////////////////////////  
//  class CLightingEngine declaration

class CLightingEngine
{
public:
        //  enumerations
        enum EMaterial
        {
            MATERIAL_BRANCH, MATERIAL_LEAF, MATERIAL_FROND
        };

        // construction/destruction
        CLightingEngine( );

        //  lighting style
        LightMethod         GetBranchLightingMethod(void) const                     { return m_eBranchLightingMethod; }
        void                SetBranchLightingMethod(LightMethod eMethod)            { m_eBranchLightingMethod = eMethod; }
        LightMethod         GetLeafLightingMethod(void) const                       { return m_eLeafLightingMethod; }
        void                SetLeafLightingMethod(LightMethod eMethod)              { m_eLeafLightingMethod = eMethod; }
        StaticStyle         GetStaticLightingStyle(void) const                      { return m_eStaticLightingStyle; }
        void                SetStaticLightingStyle(StaticStyle eStyle)              { m_eStaticLightingStyle = eStyle; }
        LightMethod         GetFrondLightingMethod(void) const                      { return m_eFrondLightingMethod; }
        void                SetFrondLightingMethod(LightMethod eMethod)             { m_eFrondLightingMethod = eMethod; }

        void                ComputeStandardStaticLighting(const float* pNormal, const float* pCoord, float* pColor, EMaterial eMaterial = MATERIAL_BRANCH);
        void                ComputeLeafStaticLighting(const CVec3& cTreeCenter, vector<CBillboardLeaf*>* pLeafLods, int nNumLeafLods);
        void                SetLeafLightingAdjustment(float fScalar)                { st_assert(fScalar > 0.0f); m_fLeafLightingAdjustmentScalar = fScalar; }
        float               GetLeafLightingAdjustment( )                            { return m_fLeafLightingAdjustmentScalar; }

        //  global lighting state
static  bool                GetLightState(unsigned int nLightIndex)                 { st_assert(nLightIndex < c_nMaxLights); return m_abLightEnabled[nLightIndex]; }
static  void                SetLightState(unsigned int nLightIndex, bool bLightOn)  { st_assert(nLightIndex < c_nMaxLights); m_abLightEnabled[nLightIndex] = bLightOn; }

static  const float*        GetLightAttributes(unsigned int nLightIndex)            { st_assert(nLightIndex < c_nMaxLights); return m_afLightAttributes[nLightIndex]; }
static  void                SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes);

        const float*        GetBranchMaterial(void) const                           { return m_afBranchMaterial; }
        void                SetBranchMaterial(const float* pMaterial)               { st_assert(pMaterial); memcpy(m_afBranchMaterial, pMaterial, c_nMaterialParamSize * sizeof(float)); }
                
        const float*        GetLeafMaterial(void) const                             { return m_afLeafMaterial; }
        void                SetLeafMaterial(const float* pMaterial)                 { st_assert(pMaterial); memcpy(m_afLeafMaterial, pMaterial, c_nMaterialParamSize * sizeof(float)); }

        const float*        GetFrondMaterial(void) const                            { return m_afFrondMaterial; }
        void                SetFrondMaterial(const float* pMaterial)                { st_assert(pMaterial); memcpy(m_afFrondMaterial, pMaterial, c_nMaterialParamSize * sizeof(float)); }

        // I/O
        void                Save(CTreeFileAccess& cFile);
        void                Parse(CTreeFileAccess& cFile);

private:
        // lights
static  bool                m_abLightEnabled[c_nMaxLights];                         // lights on or off
static  float               m_afLightAttributes[c_nMaxLights][c_nLightParamSize];   // attributes for each light

        // branch
        LightMethod         m_eBranchLightingMethod;                                // how the branches will be lit
        float               m_afBranchMaterial[c_nMaterialParamSize];               // opengl-like material for branch

        // leaves
        LightMethod         m_eLeafLightingMethod;                                  // how the leaves will be lit
        float               m_afLeafMaterial[c_nMaterialParamSize];                 // opengl-like material for leaves
        float               m_fLeafLightingAdjustmentScalar;                        // how to darken the leaves on the side opposite the leaves
        StaticStyle         m_eStaticLightingStyle;                                 // style for leaf static lighting

        // fronds
        LightMethod         m_eFrondLightingMethod;                                 // how the branches will be lit
        float               m_afFrondMaterial[c_nMaterialParamSize];                // opengl-like material for branch
};
