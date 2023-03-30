///////////////////////////////////////////////////////////////////////  
//         Name: BillboardLeaf.cpp
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

#include "Debug.h"
#include "BillboardLeaf.h"
#include "FileAccess.h"


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::CBillboardLeaf definition

CBillboardLeaf::CBillboardLeaf(const CVec3& cPos, short nColorScale, int nAngleIndex, float fWindWeight, int nWindGroup) :
    m_ucAngleIndex(static_cast<unsigned char>(nAngleIndex)),
    m_ucColorScale(static_cast<unsigned char>(nColorScale)),
    m_fWindWeight(fWindWeight),
    m_nWindGroup(nWindGroup),
    m_ucTextureIndex(0u),
    m_lColor(0xffffffff) // white and opaque (0xrrggbbaa)
{
    m_cPosition = cPos;
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::CBillboardLeaf definition

CBillboardLeaf::CBillboardLeaf( ) :
    m_ucAngleIndex(0u),
    m_ucColorScale(0u),
    m_fWindWeight(0.0f),
    m_nWindGroup(0),
    m_ucTextureIndex(0u),
    m_lColor(0xffffffff) // white and opaque (0xrrggbbaa)
{
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::AdjustStaticLighting definition
//
//  This function darkens the leaf color depending on the angle of incidence
//  between two vectors:
//
//          vector1 = (leaf_pos - tree_center)
//          vector2 = (light_pos - tree_center)
//
//  This acts as a sort of phong illumination model on the tree's leaf cloud.

void CBillboardLeaf::AdjustStaticLighting(const CVec3& cTreeCenter, const CVec3& cLightPos, float fLightScalar)
{
    CVec3 cLeafAngle = m_cPosition - cTreeCenter;
    cLeafAngle.Normalize( );

    CVec3 cLightAngle = cLightPos - cTreeCenter;
    cLightAngle.Normalize( );

    float fAngleBetween = cosf(cLeafAngle.AngleBetween(cLightAngle));
    float fLightAdjust = VecInterpolate(fLightScalar, 1.0f, (fAngleBetween + 1.0f) * 0.5f);

    SetColor(GetColor( ) * fLightAdjust);
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::operator= definition

const CBillboardLeaf& CBillboardLeaf::operator=(const CBillboardLeaf& cRight)
{
    if (&cRight != this)
    {
        CIdvCamera::operator=(cRight);

        m_ucAngleIndex = cRight.m_ucAngleIndex;
        m_lColor = cRight.m_lColor;
        m_ucColorScale = cRight.m_ucColorScale;
        m_cNormal = cRight.m_cNormal;
        m_ucTextureIndex = cRight.m_ucTextureIndex;
        m_fWindWeight = cRight.m_fWindWeight;
        m_nWindGroup = cRight.m_nWindGroup;

        // bump mapping
        m_cTangent = cRight.m_cTangent;
        m_cBinormal = cRight.m_cBinormal;
    }

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::Clone definition

CBillboardLeaf* CBillboardLeaf::Clone(void) const
{
    CBillboardLeaf* pClone = new CBillboardLeaf;
    *pClone = *this;

    return pClone;
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::GetColor definition
//
//  Converts from the internal long representation to a 3-float RGB value.

CVec3 CBillboardLeaf::GetColor(void) const
{
    return CVec3(((m_lColor & 0x000000ff) >> 0) / 255.0f,
                 ((m_lColor & 0x0000ff00) >> 8) / 255.0f,
                 ((m_lColor & 0x00ff0000) >> 16) / 255.0f);
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::SetColor definition
//
//  To simulate shadow effects, leaves that are closer to the center of the
//  tree are darkened.  This darken scalar value is stored in m_chColorScale and
//  ranges from 0 to 255.

void CBillboardLeaf::SetColor(const CVec3& cColor, bool bApplyDimming)
{
    CVec3 cScaledColor = cColor;
    if (bApplyDimming)
        cScaledColor *= m_ucColorScale / 255.0f;

    for (int i = 0; i < 3; ++i)
    {
        if (cScaledColor[i] > 1.0f)
            cScaledColor[i] = 1.0f;
        if (cScaledColor[i] < 0.0f)
            cScaledColor[i] = 0.0f;
    }

    m_lColor = (int(cScaledColor[0] * 255.0f) << 0) +
               (int(cScaledColor[1] * 255.0f) << 8) +
               (int(cScaledColor[2] * 255.0f) << 16) +
               0xff000000;
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::Save definition

void CBillboardLeaf::Save(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginBillboardLeaf);

    cFile.SaveToken(File_BillboardLeaf_Color);
    cFile.SaveLong(m_lColor);

    cFile.SaveToken(File_BillboardLeaf_AngleIndex);
    cFile.SaveByte(m_ucAngleIndex);

    cFile.SaveToken(File_BillboardLeaf_ColorScale);
    cFile.SaveByte(m_ucColorScale);

    cFile.SaveToken(File_BillboardLeaf_Normal);
    cFile.SaveVector3(m_cNormal);

    cFile.SaveToken(File_BillboardLeaf_TextureIndex);
    cFile.SaveByte(m_ucTextureIndex);

    cFile.SaveToken(File_BillboardLeaf_WindWeight);
    if (m_fWindWeight > 1.0f)
        cFile.SaveByte(255);
    else
        cFile.SaveByte(static_cast<unsigned char>(255.0f * m_fWindWeight));

    cFile.SaveToken(File_BillboardLeaf_WindGroup);
    cFile.SaveInt(m_nWindGroup);

    cFile.SaveToken(File_BillboardLeaf_Pos);
    cFile.SaveVector3(GetPosition( ));

    cFile.SaveToken(File_BillboardLeaf_TimeOffset);
    cFile.SaveFloat(0.0f); // this member var was removed (m_fTimeOffset)

    cFile.SaveToken(File_EndBillboardLeaf);
}


///////////////////////////////////////////////////////////////////////  
//  CBillboardLeaf::Parse definition

CBillboardLeaf* CBillboardLeaf::Parse(CTreeFileAccess& cFile)
{
    CBillboardLeaf* pLeaf = new CBillboardLeaf;

    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
            case File_BillboardLeaf_Color:
                pLeaf->m_lColor = cFile.ParseLong( );
                break;
            case File_BillboardLeaf_AngleIndex:
                pLeaf->m_ucAngleIndex = cFile.ParseByte( );
                break;
            case File_BillboardLeaf_ColorScale:
                pLeaf->m_ucColorScale = cFile.ParseByte( );
                break;
            case File_BillboardLeaf_Normal:
                pLeaf->m_cNormal = cFile.ParseVector3( );
                break;
            case File_BillboardLeaf_TextureIndex:
                pLeaf->m_ucTextureIndex = cFile.ParseByte( );
                break;
            case File_BillboardLeaf_WindWeight:
                pLeaf->m_fWindWeight = cFile.ParseByte( ) / 255.0f;
                break;
            case File_BillboardLeaf_WindGroup:
                pLeaf->m_nWindGroup = cFile.ParseInt( );
                break;
            case File_BillboardLeaf_Pos:
                pLeaf->SetPosition(cFile.ParseVector3( ));
                break;
            case File_BillboardLeaf_TimeOffset:
                (void) cFile.ParseFloat( ); // m_fTimeOffset was removed
                break;
            default:
                throw(IdvFileError("malformed billboard leaf"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndBillboardLeaf);

    return pLeaf;
}
