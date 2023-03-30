///////////////////////////////////////////////////////////////////////  
//         Name: BillboardLeaf.h
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
#include "TreeEngine.h"


///////////////////////////////////////////////////////////////////////  
//  class CBillboardLeaf declaration

class CBillboardLeaf : public CIdvCamera
{
public:
        CBillboardLeaf(const CVec3& cPos, short nColorScale, int nAngleIndex, float fWindWeight, int nWindGroup);

        // inspectors
        CBillboardLeaf*     Clone(void) const;
        int                 GetAngle(void) const                                { return m_ucAngleIndex; }
        CVec3               GetColor(void) const;
        unsigned long       GetColorLong(void) const                            { return m_lColor; }
        const CVec3&        GetNormal(void) const                               { return m_cNormal; }
        int                 GetTextureIndex(void) const                         { return m_ucTextureIndex / 2; }
        int                 GetAltTextureIndex(void) const                      { return m_ucTextureIndex; }
        float               GetWeight(void) const                               { return m_fWindWeight; }
        unsigned char       GetWindGroup(void) const                            { return static_cast<unsigned char>(m_nWindGroup); }
const   CVec3&              GetBinormal(void) const                             { return m_cBinormal; }
const   CVec3&              GetTangent(void) const                              { return m_cTangent; }

        // mutators
        void                AdjustStaticLighting(const CVec3& cTreeCenter, const CVec3& cLightPos, float fLightScalar);
        void                SetColor(const CVec3& cColor, bool bApplyDimming = true);
        void                SetNormal(const CVec3& cNormal)                     { m_cNormal = cNormal; }
        void                SetTextureIndex(int nTextureIndex)                  { m_ucTextureIndex = static_cast<unsigned char>(nTextureIndex); }
        void                SetBinormal(const CVec3& cBinormal)                 { m_cBinormal = cBinormal; }
        void                SetTangent(const CVec3& cTangent)                   { m_cTangent = cTangent; }

        // operators
const   CBillboardLeaf&     operator=(const CBillboardLeaf& cRight);

        // file operations
        void                Save(CTreeFileAccess& cFile) const;
static  CBillboardLeaf*     Parse(CTreeFileAccess& cFile);

private:
        CBillboardLeaf( );                      // internal - used by Clone() and Parse() only

        unsigned char       m_ucAngleIndex;     // which leaf angle group this leaf belongs to
        unsigned long       m_lColor;           // leaf color in 0xrrggbbaa format
        unsigned char       m_ucColorScale;     // 0 - 255 (dark to light)
        CVec3               m_cNormal;          // geometrical surface normal
        CVec3               m_cTangent;         // tangent values, used for bump mapping
        CVec3               m_cBinormal;        // binormal values, used for bump mapping
        unsigned char       m_ucTextureIndex;   // which texture index this leaf belongs to
        float               m_fWindWeight;      // wind weighting, 0.0 = rigid, 1.0 = full wind effect
        int                 m_nWindGroup;       // which wind matrix affects this leaf
};

