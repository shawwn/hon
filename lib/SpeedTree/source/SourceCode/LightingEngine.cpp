///////////////////////////////////////////////////////////////////////  
//         Name: LeafLighting.cpp
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
#include "LightingEngine.h"


bool CLightingEngine::m_abLightEnabled[c_nMaxLights] = { false };
float CLightingEngine::m_afLightAttributes[c_nMaxLights][c_nLightParamSize] =
{
    // 0
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 1
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 2
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 3
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 4
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 5
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 6
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    },
    // 7
    {
        0.0f, 0.0f, 0.0f,       // pos
        1.0f, 1.0f, 1.0f,       // diffuse
        0.0f, 0.0f, 0.0f,       // ambient
        1.0f, 1.0f, 1.0f,       // specular
        1.0f,                   // directional flag
        1.0f, 0.0f, 0.0f        // attenuation (constant, linear, quadratic)
    }
};


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::CLightingEngine definition

CLightingEngine::CLightingEngine( ) :
    m_eBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC),
    m_eLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC),
    m_fLeafLightingAdjustmentScalar(0.5f),
    m_eStaticLightingStyle(CSpeedTreeRT::SLS_BASIC),
    m_eFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC)
{
    float afDefaultBranchMaterial[c_nMaterialParamSize] = 
    {
        0.8f, 0.8f, 0.8f,       // diffuse
        0.2f, 0.2f, 0.2f,       // ambient
        0.0f, 0.0f, 0.0f,       // specular
        0.0f, 0.0f, 0.0f,       // emissive
        0.0f                    // shininess
    };

    float afDefaultLeafMaterial[c_nMaterialParamSize] = 
    {
        1.0f, 1.0f, 1.0f,       // diffuse
        0.5f, 0.5f, 0.5f,       // ambient
        0.0f, 0.0f, 0.0f,       // specular
        0.0f, 0.0f, 0.0f,       // emissive
        0.0f                    // shininess
    };

    float afDefaultFrondMaterial[c_nMaterialParamSize] = 
    {
        1.0f, 1.0f, 1.0f,       // diffuse
        0.5f, 0.5f, 0.5f,       // ambient
        0.0f, 0.0f, 0.0f,       // specular
        0.0f, 0.0f, 0.0f,       // emissive
        0.0f                    // shininess
    };

    memcpy(m_afBranchMaterial, afDefaultBranchMaterial, c_nMaterialParamSize * sizeof(float));
    memcpy(m_afLeafMaterial, afDefaultLeafMaterial, c_nMaterialParamSize * sizeof(float));
    memcpy(m_afFrondMaterial, afDefaultFrondMaterial, c_nMaterialParamSize * sizeof(float));
}


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::ComputeStandardStaticLighting definition

#define Light           m_afLightAttributes[nLight]
#define Clamp(a, b, c)  a < b ? a = b : a > c ? a = c : a = a;

void CLightingEngine::ComputeStandardStaticLighting(const float* pNormal, const float* pCoord, float* pColor, EMaterial eMaterial)
{
    st_assert(pCoord);
    st_assert(pNormal);
    
    CVec3 cColor;
    // compute each component of the lighting equation for each light
    for (int nLight = 0; nLight < c_nMaxLights; ++nLight)
    {
        if (m_abLightEnabled[nLight])
        {
            CVec3 cVertex(pCoord[0], pCoord[1], pCoord[2]);
            CVec3 cNormal(pNormal[0], pNormal[1], pNormal[2]);
            CVec3 cLightPos(Light[c_nLightPos + 0], Light[c_nLightPos + 1], Light[c_nLightPos + 2]);

            // pick the appropriate material
            float* pMaterial;
            switch (eMaterial)
            {
            case MATERIAL_LEAF:
                pMaterial = m_afLeafMaterial;
                break;
            case MATERIAL_FROND:
                pMaterial = m_afFrondMaterial;
                break;
            default: // branch or "other"
                pMaterial = m_afBranchMaterial;
            }

            // emissive
            CVec3 cEmissive(pMaterial[c_nMaterialEmissive + 0],
                            pMaterial[c_nMaterialEmissive + 1],
                            pMaterial[c_nMaterialEmissive + 2]);
            // ambient
            CVec3 cAmbient(Light[c_nLightAmbient + 0] * pMaterial[c_nMaterialAmbient + 0],
                           Light[c_nLightAmbient + 1] * pMaterial[c_nMaterialAmbient + 1],
                           Light[c_nLightAmbient + 2] * pMaterial[c_nMaterialAmbient + 2]);

            // attenuation
            float fAttenuation = 1.0f;
            if (Light[c_nLightDirectional] != 0.0f)
            {
                float fDistance = cVertex.Distance(cLightPos);
                fAttenuation = 1.0f / (Light[c_nLightAttenuation + 0] + 
                                       Light[c_nLightAttenuation + 1] * fDistance +
                                       Light[c_nLightAttenuation + 2] * fDistance * fDistance);
            }
            // diffuse
            CVec3 cLightVector(Light[c_nLightDirectional] == 0.0f ? cLightPos : cLightPos - cVertex);
            cLightVector.Normalize( );
            float fDotProduct = cLightVector.Dot(cNormal);
            if (fDotProduct < 0.0f)
                fDotProduct = 0.0f;
            CVec3 cDiffuse(fDotProduct * Light[c_nLightDiffuse + 0] * pMaterial[c_nMaterialDiffuse + 0],
                           fDotProduct * Light[c_nLightDiffuse + 1] * pMaterial[c_nMaterialDiffuse + 1],
                           fDotProduct * Light[c_nLightDiffuse + 2] * pMaterial[c_nMaterialDiffuse + 2]);

            // add each light's contribution
            cColor += cEmissive + (cAmbient + cDiffuse) * fAttenuation;
        }
    }

    pColor[0] = cColor[0];
    pColor[1] = cColor[1];
    pColor[2] = cColor[2];
    pColor[3] = 1.0f;

    Clamp(pColor[0], 0.0f, 1.0f);
    Clamp(pColor[1], 0.0f, 1.0f);
    Clamp(pColor[2], 0.0f, 1.0f);
}


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::ComputeLeafStaticLighting definition

void CLightingEngine::ComputeLeafStaticLighting(const CVec3& cTreeCenter, vector<CBillboardLeaf*>* pLeafLods, int nNumLeafLods)
{
    if (m_eLeafLightingMethod == CSpeedTreeRT::LIGHT_STATIC &&
        m_eStaticLightingStyle != CSpeedTreeRT::SLS_BASIC)
    {
        // compute all leaf LODs
        for (int nLod = 0; nLod < nNumLeafLods; ++nLod)
        {
            // compute each leaf in each LOD
            for (vector<CBillboardLeaf*>::iterator i = pLeafLods[nLod].begin( ); i != pLeafLods[nLod].end( ); ++i)
            {
                const CVec3& cNormal = (*i)->GetNormal( );
                const CVec3& cVertex = (*i)->GetPosition( );
                CVec3 cDiffuseColor = (*i)->GetColor( );

                CVec3 cDominantLight; // only one light can be used to simulation the shadow effect
                float fMaxDotProduct = -1.0f;
                CVec3 cColor;

                // compute each component of the lighting equation for each light
                for (int nLight = 0; nLight < c_nMaxLights; ++nLight)
                {
                    if (m_abLightEnabled[nLight])
                    {
                        CVec3 cLightPos(Light[c_nLightPos + 0], Light[c_nLightPos + 1], Light[c_nLightPos + 2]);
                        // angle of incidence
                        CVec3 cLightVector(Light[c_nLightDirectional] == 0.0f ? cLightPos : cLightPos - cVertex);
                        cLightVector.Normalize( );
                        float fDotProduct = cLightVector.Dot(cNormal);
                        if (fDotProduct >= fMaxDotProduct) // which light will be used to simulate the shadows?
                        {
                            fMaxDotProduct = fDotProduct;
                            cDominantLight = cLightPos;
                        }
                        if (fDotProduct < 0.0f)
                            fDotProduct = 0.0f;
                        fDotProduct = VecInterpolate(m_fLeafLightingAdjustmentScalar, 1.0f, fDotProduct);

                        if (m_eStaticLightingStyle & CSpeedTreeRT::SLS_USE_LIGHT_SOURCES)
                        {
                            // emissive
                            CVec3 cEmissive(m_afLeafMaterial[c_nMaterialEmissive + 0],
                                            m_afLeafMaterial[c_nMaterialEmissive + 1],
                                            m_afLeafMaterial[c_nMaterialEmissive + 2]);
                            // ambient
                            CVec3 cAmbient(Light[c_nLightAmbient + 0] * m_afLeafMaterial[c_nMaterialAmbient + 0],
                                           Light[c_nLightAmbient + 1] * m_afLeafMaterial[c_nMaterialAmbient + 1],
                                           Light[c_nLightAmbient + 2] * m_afLeafMaterial[c_nMaterialAmbient + 2]);

                            // attenuation
                            float fAttenuation = 1.0f;
                            if (Light[c_nLightDirectional] != 0.0f)
                            {
                                float fDistance = cVertex.Distance(cLightPos);
                                float fDenominator = (Light[c_nLightAttenuation + 0] + 
                                                       Light[c_nLightAttenuation + 1] * fDistance +
                                                       Light[c_nLightAttenuation + 2] * fDistance * fDistance);
                                if (fDenominator != 0.0f)
                                    fAttenuation = 1.0f / fDenominator;
                            }
                            // diffuse
                            CVec3 cDiffuse(fDotProduct * Light[c_nLightDiffuse + 0] * cDiffuseColor[0],
                                           fDotProduct * Light[c_nLightDiffuse + 1] * cDiffuseColor[1],
                                           fDotProduct * Light[c_nLightDiffuse + 2] * cDiffuseColor[2]);

                            cColor += cEmissive + (cAmbient + cDiffuse) * fAttenuation;
                        }
                    }
                }
                
                // clamp color
                Clamp(cColor[0], 0.0f, 1.0f);
                Clamp(cColor[1], 0.0f, 1.0f);
                Clamp(cColor[2], 0.0f, 1.0f);

                if (m_eStaticLightingStyle & CSpeedTreeRT::SLS_USE_LIGHT_SOURCES)
                    (*i)->SetColor(cColor, false);
                if (m_eStaticLightingStyle & CSpeedTreeRT::SLS_SIMULATE_SHADOWS)
                    (*i)->AdjustStaticLighting(cTreeCenter, cDominantLight, m_fLeafLightingAdjustmentScalar);
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::SetLightAttributes definition

void CLightingEngine::SetLightAttributes(unsigned int nLightIndex, const float* pAttributes)
{
    st_assert(nLightIndex < c_nMaxLights);
    st_assert(pAttributes);

    memcpy(m_afLightAttributes[nLightIndex], pAttributes, c_nLightParamSize * sizeof(float));
}


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::Save definition

void CLightingEngine::Save(CTreeFileAccess& cFile)
{
    cFile.SaveToken(File_BeginLightingInfo);

    cFile.SaveToken(File_BranchLightingMethod);
    cFile.SaveInt(m_eBranchLightingMethod);

    cFile.SaveToken(File_BranchMaterial);
    for (int i = 0; i < c_nMaterialParamSize; ++i)
        cFile.SaveFloat(m_afBranchMaterial[i]);

    cFile.SaveToken(File_LeafLightingMethod);
    cFile.SaveInt(m_eLeafLightingMethod);

    cFile.SaveToken(File_LeafMaterial);
    for (int i = 0; i < c_nMaterialParamSize; ++i)
        cFile.SaveFloat(m_afLeafMaterial[i]);

    cFile.SaveToken(File_LeafLightingAdjustment);
    cFile.SaveFloat(m_fLeafLightingAdjustmentScalar);

    cFile.SaveToken(File_StaticLightingStyle);
    cFile.SaveInt(m_eStaticLightingStyle);

    cFile.SaveToken(File_FrondLightingMethod);
    cFile.SaveInt(m_eFrondLightingMethod);

    cFile.SaveToken(File_FrondMaterial);
    for (int i = 0; i < c_nMaterialParamSize; ++i)
        cFile.SaveFloat(m_afFrondMaterial[i]);

    cFile.SaveToken(File_EndLightingInfo);
}


///////////////////////////////////////////////////////////////////////  
//  CLightingEngine::Parse definition

void CLightingEngine::Parse(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
            case File_BranchLightingMethod:
                m_eBranchLightingMethod = static_cast<LightMethod>(cFile.ParseInt( ));
                break;
            case File_BranchMaterial:
                {
                    for (int i = 0; i < c_nMaterialParamSize; ++i)
                        m_afBranchMaterial[i] = cFile.ParseFloat( );
                }
                break;
            case File_LeafLightingMethod:
                m_eLeafLightingMethod = static_cast<LightMethod>(cFile.ParseInt( ));
                break;
            case File_LeafMaterial:
                {
                    for (int i = 0; i < c_nMaterialParamSize; ++i)
                        m_afLeafMaterial[i] = cFile.ParseFloat( );
                }
                break;
            case File_LeafLightingAdjustment:
                m_fLeafLightingAdjustmentScalar = cFile.ParseFloat( );
                break;
            case File_StaticLightingStyle:
                m_eStaticLightingStyle = static_cast<StaticStyle>(cFile.ParseInt( ));
                break;
            case File_FrondLightingMethod:
                m_eFrondLightingMethod = static_cast<LightMethod>(cFile.ParseInt( ));
                break;
            case File_FrondMaterial:
                {
                    for (int i = 0; i < c_nMaterialParamSize; ++i)
                        m_afFrondMaterial[i] = cFile.ParseFloat( );
                }
                break;
            default:
                throw(IdvFileError("malformed lighting information"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndLightingInfo);
}
