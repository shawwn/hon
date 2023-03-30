///////////////////////////////////////////////////////////////////////  
//         Name: FrondEngine.cpp
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
#include "FrondEngine.h"
#include "IndexedGeometry.h"
#include "LightingEngine.h"
#include <algorithm>

#if defined(linux)
#include <limits.h>
#endif

using namespace std;


///////////////////////////////////////////////////////////////////////  
//  Constants

const float c_fRepeatPreventionFactor = 0.999f;
const float c_fStabilityThreshold = 0.1f;


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::CFrondEngine definition

CFrondEngine::CFrondEngine( ) :
    m_nLevel(1),
    m_pGeometry(NULL),
    m_pLightingEngine(NULL),
    m_eFrondType(CFrondEngine::FROND_EXTRUSION),
    m_nNumBlades(2),
    m_nProfileSegments(4),
    m_bEnabled(false),
    m_nNumLodLevels(4),
    m_fMaxSurfaceAreaPercent(1.0f),
    m_fMinSurfaceAreaPercent(0.0f),
    m_fReductionFuzziness(0.0f),
    m_fLargeRetentionPercent(0.05f),
    m_nMinLengthSegments(2),
    m_nMinCrossSegments(1)
{
    m_pProfile = new CIdvBezierSpline(string("BezierSpline 0.0 1.0 0.0 { 3 0 0.00138887 0.337009 0.941501 0.132767 0.493215 0.998903 1 0.00102074 0.23702 1 -6.24607e-008 0.307222 -0.951638 0.126974 }"));
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::CFrondEngine

CFrondEngine::CFrondEngine(const CFrondEngine& cRight)
{
    *this = cRight;
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::~CFrondEngine definition

CFrondEngine::~CFrondEngine( )
{
    delete m_pProfile;
    m_pProfile = NULL;
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::GetLevel definition

int CFrondEngine::GetLevel(void) const
{
    return m_nLevel;
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::Enabled definition

bool CFrondEngine::Enabled(void) const
{
    return m_bEnabled;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvFrondInfo::Parse definition

void CFrondEngine::Parse(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_FrondLevel:
            m_nLevel = cFile.ParseInt( );
            break;
        case File_FrondType:
            m_eFrondType = static_cast<EFrondType>(cFile.ParseInt( ));
            break;
        case File_FrondNumBlades:
            m_nNumBlades = cFile.ParseInt( );
            break;
        case File_FrondProfile:
            SetProfile(cFile.ParseBranchParameter( ));
            break;
        case File_FrondProfileSegments:
            m_nProfileSegments = cFile.ParseInt( );
            break;
        case File_FrondEnabled:
            m_bEnabled = cFile.ParseBool( );
            break;
        case File_FrondNumLodLevels:
            m_nNumLodLevels = cFile.ParseInt( );
            break;
        case File_FrondMaxSurfaceAreaPercent:
            m_fMaxSurfaceAreaPercent = cFile.ParseFloat( );
            break;
        case File_FrondMinSurfaceAreaPercent:
            m_fMinSurfaceAreaPercent = cFile.ParseFloat( );
            break;
        case File_FrondReductionFuzziness:
            m_fReductionFuzziness = cFile.ParseFloat( );
            break;
        case File_FrondLargeFrondPercent:
            m_fLargeRetentionPercent = cFile.ParseFloat( );
            break;
        case File_FrondMinLengthSegments:
            m_nMinLengthSegments = cFile.ParseInt( );
            break;
        case File_FrondMinCrossSegments:
            m_nMinCrossSegments = cFile.ParseInt( );
            break;
        case File_FrondNumTextures:
            {
                m_vTextures.clear( );
                int nNumTextures = cFile.ParseInt( );
                for (int i = 0; i < nNumTextures; ++i)
                {
                    SFrondTexture sFrond;
                    nToken = cFile.ParseToken( ); // File_BeginFrondTexture
                    nToken = cFile.ParseToken( );
                    do
                    {
                        switch (nToken)
                        {
                            case File_FrondTextureFilename:
                                sFrond.m_strFilename = cFile.ParseString( );
                                sFrond.m_strFilename = sFrond.m_strFilename.NoPath( );
                                break;
                            case File_FrondTextureAspectRatio:
                                sFrond.m_fAspectRatio = cFile.ParseFloat( );
                                break;
                            case File_FrondTextureSizeScale:
                                sFrond.m_fSizeScale = cFile.ParseFloat( );
                                break;
                            case File_FrondTextureMinAngleOffset:
                                sFrond.m_fMinAngleOffset = cFile.ParseFloat( );
                                break;
                            case File_FrondTextureMaxAngleOffset:
                                sFrond.m_fMaxAngleOffset = cFile.ParseFloat( );
                                break;
                            default:
                                throw(IdvFileError(IdvFormatString("malformed frond texture information (token %d)", nToken)));
                                break;
                        }
                        nToken = cFile.ParseToken( );
                    } while (nToken != File_EndFrondTexture);

                    m_vTextures.push_back(sFrond);
                }
            }
            break;
        default:
            throw(IdvFileError(IdvFormatString("malformed frond info (token %d)", nToken)));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndFrondInfo);
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::Save definition

void CFrondEngine::Save(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginFrondInfo);

    cFile.SaveToken(File_FrondLevel);
    cFile.SaveInt(m_nLevel);

    cFile.SaveToken(File_FrondType);
    cFile.SaveInt(static_cast<int>(m_eFrondType));

    cFile.SaveToken(File_FrondNumBlades);
    cFile.SaveInt(m_nNumBlades);

    cFile.SaveToken(File_FrondProfile);
    cFile.SaveBranchParameter(m_pProfile);

    cFile.SaveToken(File_FrondProfileSegments);
    cFile.SaveInt(m_nProfileSegments);

    cFile.SaveToken(File_FrondEnabled);
    cFile.SaveBool(m_bEnabled);

    cFile.SaveToken(File_FrondNumLodLevels);
    cFile.SaveInt(m_nNumLodLevels);

    cFile.SaveToken(File_FrondMaxSurfaceAreaPercent);
    cFile.SaveFloat(m_fMaxSurfaceAreaPercent);

    cFile.SaveToken(File_FrondMinSurfaceAreaPercent);
    cFile.SaveFloat(m_fMinSurfaceAreaPercent);

    cFile.SaveToken(File_FrondReductionFuzziness);
    cFile.SaveFloat(m_fReductionFuzziness);

    cFile.SaveToken(File_FrondLargeFrondPercent);
    cFile.SaveFloat(m_fLargeRetentionPercent);

    cFile.SaveToken(File_FrondMinLengthSegments);
    cFile.SaveInt(m_nMinLengthSegments);

    cFile.SaveToken(File_FrondMinCrossSegments);
    cFile.SaveInt(m_nMinCrossSegments);

    cFile.SaveToken(File_FrondNumTextures);
    cFile.SaveInt(m_vTextures.size( ));

    for (unsigned int i = 0; i < m_vTextures.size( ); ++i)
    {
        cFile.SaveToken(File_BeginFrondTexture);

        cFile.SaveToken(File_FrondTextureFilename);
        cFile.SaveString(m_vTextures[i].m_strFilename);

        cFile.SaveToken(File_FrondTextureAspectRatio);
        cFile.SaveFloat(m_vTextures[i].m_fAspectRatio);

        cFile.SaveToken(File_FrondTextureSizeScale);
        cFile.SaveFloat(m_vTextures[i].m_fSizeScale);

        cFile.SaveToken(File_FrondTextureMinAngleOffset);
        cFile.SaveFloat(m_vTextures[i].m_fMinAngleOffset);

        cFile.SaveToken(File_FrondTextureMaxAngleOffset);
        cFile.SaveFloat(m_vTextures[i].m_fMaxAngleOffset);

        cFile.SaveToken(File_EndFrondTexture);
    }

    cFile.SaveToken(File_EndFrondInfo);
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::ClearGuides definition

void CFrondEngine::ClearGuides(void)
{
    m_vGuides.clear( );
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::StartGuide definition

void CFrondEngine::StartGuide(void)
{
    SFrondGuide sGuide;
    m_vGuides.push_back(sGuide);
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::AddGuideVertex definition

void CFrondEngine::AddGuideVertex(CVec3 cPos, CRotTransform cTrans, float fCrossSectionWeight, int nWindGroup)
{
    SFrondVertex sVertex;
    sVertex.m_cPos = cPos;
    sVertex.m_cTrans = cTrans;
    sVertex.m_fCrossSectionWeight = fCrossSectionWeight;
    sVertex.m_nWindGroup = nWindGroup;

    m_vGuides[m_vGuides.size( ) - 1].m_vVertices.push_back(sVertex);
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::EndGuide definition

void CFrondEngine::EndGuide(float fLodSizeScalar)
{
    SFrondGuide& sGuide = m_vGuides[m_vGuides.size( ) - 1];

	unsigned int i(0);
    for (; i < sGuide.m_vVertices.size( ) - 1; ++i)
        sGuide.m_fLength += sGuide.m_vVertices[i].m_cPos.Distance(sGuide.m_vVertices[i + 1].m_cPos);

    // choose a frond map, compute the radius and start angle
    CIdvRandom cRandom;
    if (m_vTextures.size( ) == 0)
    {
        m_vGuides[i].m_chFrondMapIndex = 0;
        sGuide.m_fRadius = sGuide.m_fLength * 0.5f;
        sGuide.m_fOffsetAngle = 0.0f;
    }
    else
    {
        sGuide.m_chFrondMapIndex = static_cast<unsigned char>(static_cast<unsigned int>(cRandom.GetUniform(0.0f, 100000.0f)) % m_vTextures.size( ));
        sGuide.m_fRadius = sGuide.m_fLength * m_vTextures[static_cast<int>(sGuide.m_chFrondMapIndex)].m_fAspectRatio * 0.5f;
        sGuide.m_fRadius *= m_vTextures[sGuide.m_chFrondMapIndex].m_fSizeScale;
        sGuide.m_fOffsetAngle = cRandom.GetUniform(m_vTextures[sGuide.m_chFrondMapIndex].m_fMinAngleOffset, m_vTextures[sGuide.m_chFrondMapIndex].m_fMaxAngleOffset);
        if (m_vGuides.size( ) % 2 == 1)
            sGuide.m_fOffsetAngle *= -1.0f;
    }

    sGuide.m_fSurfaceArea = sGuide.m_fLength * /* sGuide.m_fRadius * */ fLodSizeScalar;
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::SetProfile definition

void CFrondEngine::SetProfile(CIdvBezierSpline* pProfile)
{
    if (m_pProfile != pProfile)
    {
        delete m_pProfile;
        m_pProfile = pProfile;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::Compute definition

void CFrondEngine::Compute(CIndexedGeometry* pGeometry, CLightingEngine* pLightingEngine)
{
    m_pGeometry = pGeometry;
    m_pLightingEngine = pLightingEngine;

    st_assert(m_pGeometry);
    st_assert(m_pLightingEngine);

    int nVertices = 0;
    BuildGuideLods( );
    for (unsigned int j = 0; j < m_vGuideLods[0].size( ); ++j)
    {
        switch (m_eFrondType)
        {
        case FROND_BLADES:
            // all blade lods reference the same vertices so only report them once
            nVertices += m_vGuideLods[0][j].m_vVertices.size( ) * 2 * m_nNumBlades;
            break;
        case FROND_EXTRUSION:
            // all extrusion lods reference the same vertices so only report them once
            nVertices += m_vGuideLods[0][j].m_vVertices.size( ) * ((m_nProfileSegments * 2) - 1);
            break;
        default:
            throw(IdvFileError("default reached in CFrondEngine::Compute()"));
        }
    }

    if (nVertices > USHRT_MAX)
        throw(runtime_error(IdvFormatString("frond vertices exceed %d", USHRT_MAX)));

    m_pGeometry->SetNumLodLevels(static_cast<unsigned short>(m_nNumLodLevels));

    switch (m_eFrondType)
    {
    case FROND_BLADES:
    {
        // build vertices for highest lod (lower lods share them)
        for (unsigned int i = 0; i < m_vGuideLods[0].size( ); ++i)
            BuildBladeVertices(m_vGuideLods[0][i]);

        // build all strips
        for (unsigned short m = 0; m < m_vGuideLods.size( ); ++m)
        {
            m_pGeometry->ResetStripCounter(m);
            for (unsigned int k = 0; k < m_vGuideLods[m].size( ); ++k)
                ComputeBlade(m, m_vGuideLods[0][k].m_nVertexStartIndex, m_vGuideLods[m][k]);
        }
        break;
    }
    case FROND_EXTRUSION:
    {
        // build vertices for highest lod (lower lods share them)
        for (unsigned int i = 0; i < m_vGuideLods[0].size( ); ++i)
            BuildExtrusionVertices(m_vGuideLods[0][i]);

        for (unsigned short l = 0; l < m_vGuideLods.size( ); ++l)
        {
            m_pGeometry->ResetStripCounter(l);
            for (unsigned int k = 0; k < m_vGuideLods[l].size( ); ++k)
            {
                m_vGuideLods[l][k].m_nVerticesPerGuideVertex = m_vGuideLods[0][k].m_nVerticesPerGuideVertex;
                ComputeExtrusion(l, m_vGuideLods[0][k].m_nVertexStartIndex, m_vGuideLods[l][k]);
            }
        }
        break;
    }
    default:
        throw(IdvFileError("default reached in CFrondEngine::Compute()"));
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::BuildBladeVertices definition

void CFrondEngine::BuildBladeVertices(SFrondGuide& sGuide)
{
    if (m_pGeometry && m_pLightingEngine)
    {
        float fAngleBetween = 180.0f / m_nNumBlades;

        sGuide.m_nVertexStartIndex = m_pGeometry->GetVertexCounter( );
        sGuide.m_nVerticesPerGuideVertex = 2;
        for (unsigned int k = 0; k < m_nNumBlades; ++k)
        {
            int nLocalStart = m_pGeometry->GetVertexCounter( );
            // add vertices
            vector<CVec3> vRightDirections;
            for (unsigned int i = 0; i < sGuide.m_vVertices.size( ); ++i)
            {
                // get the matrix for these vertices
                CRotTransform cTrans;
                if (i < sGuide.m_vVertices.size( ) - 1)
                    cTrans = sGuide.m_vVertices[i].m_cTrans;
                else
                    cTrans = sGuide.m_vVertices[i - 1].m_cTrans;
                cTrans.RotateX(fAngleBetween * k + sGuide.m_fOffsetAngle);

                // compute directions
                CVec3 cRightDir = CVec3(0.0f, 1.0f, 0.0f) * cTrans;
                vRightDirections.push_back(cRightDir);
                
                // compute normal
                CVec3 cNormal;
                if (i == 0)
                    cNormal = sGuide.m_vVertices[i + 1].m_cPos - sGuide.m_vVertices[0].m_cPos;
                else
                    cNormal = sGuide.m_vVertices[i].m_cPos - sGuide.m_vVertices[0].m_cPos;
                cNormal.Normalize( );

                // compute vertices
                CVec3 cRight, cLeft;

                if (i == sGuide.m_vVertices.size( ) - 1 && sGuide.m_vVertices.size( ) > 0)
                {
                    cRight = sGuide.m_vVertices[i].m_cPos + vRightDirections[i - 1] * sGuide.m_fRadius;
                    cLeft = sGuide.m_vVertices[i].m_cPos - vRightDirections[i - 1] * sGuide.m_fRadius;
                }
                else if (i > 0 && i < sGuide.m_vVertices.size( ) - 1)
                {
                    CVec3 cTemp;

                    // right
                    cTemp = sGuide.m_vVertices[i].m_cPos + vRightDirections[i - 1] * sGuide.m_fRadius;
                    cRight = sGuide.m_vVertices[i].m_cPos + cRightDir * sGuide.m_fRadius;
                    cRight = (cRight + cTemp) * 0.5f;

                    // left
                    cTemp = sGuide.m_vVertices[i].m_cPos - vRightDirections[i - 1] * sGuide.m_fRadius;
                    cLeft = sGuide.m_vVertices[i].m_cPos - cRightDir * sGuide.m_fRadius;
                    cLeft = (cLeft + cTemp) * 0.5f;
                }
                else
                {
                    cRight = sGuide.m_vVertices[i].m_cPos + cRightDir * sGuide.m_fRadius;
                    cLeft = sGuide.m_vVertices[i].m_cPos - cRightDir * sGuide.m_fRadius;
                }

                float fLengthPercent = static_cast<float>(i) / (sGuide.m_vVertices.size( ) - 1.0f);

                // add right vertex
                m_pGeometry->AddVertexCoord(cRight.m_afData);

                float afTexCoords[2];
                afTexCoords[0] = 1.0f;
                afTexCoords[1] = fLengthPercent;
                m_pGeometry->AddVertexTexCoord0(afTexCoords, sGuide.m_chFrondMapIndex);

                if (m_pLightingEngine->GetFrondLightingMethod( ) == CSpeedTreeRT::LIGHT_STATIC)
                {
                    float afColor[4];
                    m_pLightingEngine->ComputeStandardStaticLighting(cNormal.m_afData, cRight.m_afData, afColor, CLightingEngine::MATERIAL_FROND);
                    m_pGeometry->AddVertexColor(afColor);
                }
                else
                {
                    m_pGeometry->AddVertexNormal(cNormal.m_afData);

                    // bump mapping
                    m_pGeometry->AddVertexTangent(cRightDir);
                    CVec3 cBinormal = cRightDir * cNormal;
                    if (cBinormal.Magnitude( ) < c_fStabilityThreshold)
                        cBinormal= cNormal;
                    else
                        cBinormal.Normalize( );
                    m_pGeometry->AddVertexBinormal(cBinormal);
                }

                if (m_pGeometry->IsVertexWeightingEnabled( ))
                    m_pGeometry->AddVertexWind(sGuide.m_vVertices[i].m_fCrossSectionWeight, static_cast<unsigned char>(sGuide.m_vVertices[i].m_nWindGroup));

                m_pGeometry->AdvanceVertexCounter( );

                // add left vertex
                m_pGeometry->AddVertexCoord(cLeft.m_afData);

                afTexCoords[0] = 0.0f;
                m_pGeometry->AddVertexTexCoord0(afTexCoords, sGuide.m_chFrondMapIndex);

                if (m_pLightingEngine->GetFrondLightingMethod( ) == CSpeedTreeRT::LIGHT_STATIC)
                {
                    float afColor[4];
                    m_pLightingEngine->ComputeStandardStaticLighting(cNormal.m_afData, cLeft.m_afData, afColor, CLightingEngine::MATERIAL_FROND);
                    m_pGeometry->AddVertexColor(afColor);
                }
                else
                {
                    m_pGeometry->AddVertexNormal(cNormal.m_afData);

                    // bump mapping
                    m_pGeometry->AddVertexTangent(cRightDir);
                    CVec3 cBinormal = cRightDir * cNormal;
                    if (cBinormal.Magnitude( ) < c_fStabilityThreshold)
                        cBinormal= cNormal;
                    else
                        cBinormal.Normalize( );
                    m_pGeometry->AddVertexBinormal(cBinormal);
                }

                if (m_pGeometry->IsVertexWeightingEnabled( ))
                    m_pGeometry->AddVertexWind(sGuide.m_vVertices[i].m_fCrossSectionWeight, static_cast<unsigned char>(sGuide.m_vVertices[i].m_nWindGroup));

                m_pGeometry->AdvanceVertexCounter( );
            }
            
            // compute lengths for t coord adjustment
            float fRightLength = 0.0f, fLeftLength = 0.0f;
            vector<float> vRightLengths, vLeftLengths;
            unsigned int nSize = sGuide.m_vVertices.size( );
            for (unsigned int i = 0; i < nSize - 1; ++i)
            {
                const float* pThisCoord = m_pGeometry->GetVertexCoord(i * 2 + 0 + sGuide.m_nVertexStartIndex);
                const float* pNextCoord = m_pGeometry->GetVertexCoord(i * 2 + 2 + sGuide.m_nVertexStartIndex);
                CVec3 cThis(pThisCoord[0], pThisCoord[1], pThisCoord[2]);
                CVec3 cNext(pNextCoord[0], pNextCoord[1], pNextCoord[2]);
                fRightLength += cThis.Distance(cNext);
                vRightLengths.push_back(fRightLength);

                pThisCoord = m_pGeometry->GetVertexCoord(i * 2 + 1 + sGuide.m_nVertexStartIndex);
                pNextCoord = m_pGeometry->GetVertexCoord(i * 2 + 3 + sGuide.m_nVertexStartIndex);
                cThis.Set(pThisCoord[0], pThisCoord[1], pThisCoord[2]);
                cNext.Set(pNextCoord[0], pNextCoord[1], pNextCoord[2]);
                fLeftLength += cThis.Distance(cNext);
                vLeftLengths.push_back(fLeftLength);
            }

            // set new, "unpinched" t coords (skip the first one since it is always zero)
            m_pGeometry->SetVertexCounter(static_cast<unsigned short>(nLocalStart + 2));
            for (unsigned int i = 1; i < nSize - 1; ++i)
            {
                const float* pTexCoord = m_pGeometry->GetVertexTexCoord0(i * 2 + 0 + sGuide.m_nVertexStartIndex);
                float afRightTexCoord[ ] = { pTexCoord[0], c_fRepeatPreventionFactor * (vRightLengths[i - 1] / fRightLength) };
                m_pGeometry->AddVertexTexCoord0(afRightTexCoord, sGuide.m_chFrondMapIndex);
                m_pGeometry->AdvanceVertexCounter( );

                pTexCoord = m_pGeometry->GetVertexTexCoord0(i * 2 + 1 + sGuide.m_nVertexStartIndex);
                float afLeftTexCoord[ ] = { pTexCoord[0], c_fRepeatPreventionFactor * (vLeftLengths[i - 1] / fLeftLength) };
                m_pGeometry->AddVertexTexCoord0(afLeftTexCoord, sGuide.m_chFrondMapIndex);
                m_pGeometry->AdvanceVertexCounter( );
            }

            // acount for unmodified end coords
            m_pGeometry->AdvanceVertexCounter( );
            m_pGeometry->AdvanceVertexCounter( );
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::ComputeBlade definition

void CFrondEngine::ComputeBlade(unsigned int nLod, unsigned int nStart, SFrondGuide& sGuide)
{
    if (m_pGeometry)
    {
        for (unsigned int k = 0; k < m_nNumBlades; ++k)
        {
            // build strips
            unsigned short usIndexCount = static_cast<unsigned short>(sGuide.m_vVertices.size( ) * 2);
            unsigned short* pStrip = new unsigned short[usIndexCount];

            int nVertexIndex = 0;
            for (unsigned int i = 0; i < sGuide.m_vVertices.size( ) * 2; ++i)
                pStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + (k * 2 * sGuide.m_vVertices.size( )) + i);

            m_pGeometry->AddStrip(static_cast<unsigned short>(nLod), pStrip, usIndexCount);
            m_pGeometry->AdvanceStripCounter( );
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::BuildExtrusionVertices definition

void CFrondEngine::BuildExtrusionVertices(SFrondGuide& sGuide)
{
    if (m_pGeometry && m_pLightingEngine)
    {
        vector<CVec3> vProfile, vNormals;
        BuildProfileVectors(sGuide, vProfile, vNormals);

        // build vertex table
        sGuide.m_nVerticesPerGuideVertex = vProfile.size( );
        sGuide.m_nVertexStartIndex = m_pGeometry->GetVertexCounter( );

        // run through points defining the spine of the frond
        for (unsigned int i = 0; i < sGuide.m_vVertices.size( ); ++i)
        {
            // T tex coord
            float fT = static_cast<float>(i) / (sGuide.m_vVertices.size( ) - 1.0f);

            // run through the points on either side of point in spine (calculate
            // two points, one on each side)
            for (unsigned int j = 0; j < sGuide.m_nVerticesPerGuideVertex; ++j)
            {
                // S tex coord
                float fS = j / (sGuide.m_nVerticesPerGuideVertex - 1.0f);

                // position
                CRotTransform cTrans = sGuide.m_vVertices[i].m_cTrans;
                CRotTransform cPreviousTrans = i ? sGuide.m_vVertices[i - 1].m_cTrans : cTrans;

                cTrans.RotateX(sGuide.m_fOffsetAngle);
                cPreviousTrans.RotateX(sGuide.m_fOffsetAngle);

                CVec3 cPos1 = vProfile[j] * cTrans, cPos2 = vProfile[j] * cPreviousTrans;
                CVec3 cPos = sGuide.m_vVertices[i].m_cPos + ((cPos1 + cPos2) * 0.5f);

                CVec3 cTangent = cPos1 + cPos2;
                cTangent.Normalize( );

                m_pGeometry->AddVertexCoord(cPos.m_afData);
                
                float afTexCoords[2];
                afTexCoords[0] = fS;
                afTexCoords[1] = fT;
                m_pGeometry->AddVertexTexCoord0(afTexCoords, sGuide.m_chFrondMapIndex);

                // normal
                CVec3 cNormal1 = vNormals[j] * cTrans, cNormal2 = vNormals[j] * cPreviousTrans;
                CVec3 cNormal = (cNormal1 + cNormal2) * 0.5f;
                cNormal.Normalize( );

                if (m_pLightingEngine->GetFrondLightingMethod( ) == CSpeedTreeRT::LIGHT_STATIC)
                {
                    float afColor[4];
                    m_pLightingEngine->ComputeStandardStaticLighting(cNormal.m_afData, cPos.m_afData, afColor, CLightingEngine::MATERIAL_FROND);
                    m_pGeometry->AddVertexColor(afColor);
                }
                else
                {
                    m_pGeometry->AddVertexNormal(cNormal);

                    // bump mapping
                    m_pGeometry->AddVertexTangent(cTangent);

                    CVec3 cBinormal = cNormal * cTangent;
//                  if (cBinormal.Magnitude( ) < c_fStabilityThreshold)
//                      cBinormal= cNormal;
//                  else
                        cBinormal.Normalize( );
                    m_pGeometry->AddVertexBinormal(cBinormal);
                }

                if (m_pGeometry->IsVertexWeightingEnabled( ))
                    m_pGeometry->AddVertexWind(sGuide.m_vVertices[i].m_fCrossSectionWeight, static_cast<unsigned char>(sGuide.m_vVertices[i].m_nWindGroup));

                m_pGeometry->AdvanceVertexCounter( );
            }
        }

        // compute lengths for t coord adjustment
        vector<float> vLengths;
        vector< vector<float> > vRunningLengths;

        for (unsigned int i = 0; i < sGuide.m_nVerticesPerGuideVertex; ++i)
        {
            float fLength = 0.0f;
            vector<float> vRunning;
            vRunning.push_back(0.0f);
            for (unsigned int j = 1; j < sGuide.m_vVertices.size( ); ++j)
            {
                int nThisIndex = sGuide.m_nVertexStartIndex + (j * sGuide.m_nVerticesPerGuideVertex) + i;
                int nPrevIndex = sGuide.m_nVertexStartIndex + ((j - 1) * sGuide.m_nVerticesPerGuideVertex) + i;

                const float* pThisCoord = m_pGeometry->GetVertexCoord(nThisIndex);
                const float* pPrevCoord = m_pGeometry->GetVertexCoord(nPrevIndex);
                CVec3 cThis(pThisCoord[0], pThisCoord[1], pThisCoord[2]);
                CVec3 cPrev(pPrevCoord[0], pPrevCoord[1], pPrevCoord[2]);
                fLength += cThis.Distance(cPrev);
                vRunning.push_back(fLength);
            }
            vRunningLengths.push_back(vRunning);
            vLengths.push_back(fLength);
        }

        // set new, "unpinched" t coords (skip the first one since it is always zero)
        for (unsigned int i = 0; i < sGuide.m_nVerticesPerGuideVertex; ++i)
        {
            for (unsigned int j = 1; j < sGuide.m_vVertices.size( ); ++j)
            {
                unsigned short usThisIndex = static_cast<unsigned short>(sGuide.m_nVertexStartIndex + (j * sGuide.m_nVerticesPerGuideVertex) + i);
                m_pGeometry->SetVertexCounter(usThisIndex);

                const float* pTexCoord = m_pGeometry->GetVertexTexCoord0(usThisIndex);
                float afNewTexCoord[ ] = { pTexCoord[0], c_fRepeatPreventionFactor * (vRunningLengths[i][j] / vLengths[i]) };
                m_pGeometry->AddVertexTexCoord0(afNewTexCoord, sGuide.m_chFrondMapIndex);
                m_pGeometry->AdvanceVertexCounter( );
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::ComputeExtrusion definition

void CFrondEngine::ComputeExtrusion(unsigned int nLod, unsigned int nStart, SFrondGuide& sGuide)
{
    if (m_pGeometry)
    {
        // compute lod info
        float fLodPercent;
        if (m_nNumLodLevels == 1)
            fLodPercent = 1.0f;
        else
            fLodPercent = 1.0f - (nLod / (m_nNumLodLevels - 1.0f));

        int nNumLengthVertices = static_cast<int>(VecInterpolate(static_cast<float>(m_nMinLengthSegments + 1), static_cast<float>(sGuide.m_vVertices.size( )), fLodPercent)); // plus one for vertex conversion
        float fLengthStep = sGuide.m_vVertices.size( ) / static_cast<float>(nNumLengthVertices - 1);  // -1 because one of them doesn't "step"

        int nNumCrossVertices = static_cast<int>(VecInterpolate(static_cast<float>((m_nMinCrossSegments * 2) + 1), static_cast<float>(sGuide.m_nVerticesPerGuideVertex), fLodPercent)); // * 2 for mirroring, plus one for vertex conversion
        float fCrossStep = (sGuide.m_nVerticesPerGuideVertex) / static_cast<float>(nNumCrossVertices - 1);  // -1 because one of them doesn't "step"

        // build strips
        int nSegments = nNumLengthVertices - 1;
        int nIndexCount = (nSegments * (nNumCrossVertices * 2 + 1)) - 1; // + 1 for repeated vertex, - 1 because we do not need to repeat the last one
        unsigned short* pStrip = new unsigned short[nIndexCount];

        int nVertexIndex = 0;
        for (int i = 0; i < nNumLengthVertices - 1; ++i)
        {
            int nThisLengthIndex = static_cast<int>(i * fLengthStep);
            int nNextLengthIndex = static_cast<int>((i + 1) * fLengthStep);

            if (i == nNumLengthVertices - 2 || nNextLengthIndex > static_cast<int>(sGuide.m_vVertices.size( ) - 1))
                nNextLengthIndex = sGuide.m_vVertices.size( ) - 1;

            int nSkipFactor = nNextLengthIndex - nThisLengthIndex;

            if (i % 2 == 0)
            {
                int nBase = nThisLengthIndex * sGuide.m_nVerticesPerGuideVertex;
                for (int j = 0; j < nNumCrossVertices; ++j)
                {
                    unsigned int nThisCrossVertex = static_cast<unsigned int>(j * fCrossStep);

                    if (j == nNumCrossVertices - 1)
                        nThisCrossVertex = sGuide.m_nVerticesPerGuideVertex - 1;

                    if (nThisCrossVertex > sGuide.m_nVerticesPerGuideVertex - 1)
                        nThisCrossVertex = sGuide.m_nVerticesPerGuideVertex - 1;

                    pStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + nBase + nThisCrossVertex);
                    pStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + nBase + nThisCrossVertex + (nSkipFactor * sGuide.m_nVerticesPerGuideVertex));
                }

                // repeat this vertex so we can combine the multiple segments of one frond
                if (i < nNumLengthVertices - 2)
                {
                    pStrip[nVertexIndex] = pStrip[nVertexIndex - 1];
                    ++nVertexIndex;
                }
            }
            else
            {
                int nBase = nThisLengthIndex * sGuide.m_nVerticesPerGuideVertex;
                for (int j = nNumCrossVertices - 1; j >= 0; --j)
                {
                    unsigned int nThisCrossVertex = static_cast<unsigned int>((j * fCrossStep));

                    if (j == nNumCrossVertices - 1)
                        nThisCrossVertex = sGuide.m_nVerticesPerGuideVertex - 1;

                    if (nThisCrossVertex > sGuide.m_nVerticesPerGuideVertex - 1)
                        nThisCrossVertex = sGuide.m_nVerticesPerGuideVertex - 1;

                    pStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + nBase + nThisCrossVertex);
                    pStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + nBase + nThisCrossVertex + (nSkipFactor * sGuide.m_nVerticesPerGuideVertex));
                }

                // repeat this vertex so we can combine the multiple segments of one frond
                if (i < nNumLengthVertices - 2)
                {
                    pStrip[nVertexIndex] = pStrip[nVertexIndex - 1];
                    ++nVertexIndex;
                }
            }
        }
        m_pGeometry->AddStrip(static_cast<unsigned short>(nLod), pStrip, static_cast<unsigned short>(nIndexCount));
        m_pGeometry->AdvanceStripCounter( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::BuildProfileVectors definition

void CFrondEngine::BuildProfileVectors(SFrondGuide& sGuide, vector<CVec3>& vProfile, vector<CVec3>& vNormals)
{
    // left side
    float fAdjust = m_pProfile->Evaluate(0.0f) * sGuide.m_fRadius;
    for (unsigned int i = 0; i < m_nProfileSegments; ++i)
    {
        float fPercent = 1.0f - (i / (m_nProfileSegments - 1.0f));
        CVec3 cVertex;
        cVertex[0] = 0.0f;
        cVertex[1] = -fPercent * sGuide.m_fRadius;
        cVertex[2] = (m_pProfile->Evaluate(fPercent) * sGuide.m_fRadius) - fAdjust;

        vProfile.push_back(cVertex);
    }

    // reverse for right side w/o duplicating center point
    for (int j = m_nProfileSegments - 2; j >= 0; --j)
    {
        CVec3 cVertex = vProfile[j];
        cVertex[1] *= -1.0f;
        vProfile.push_back(cVertex);
    }

    // build normal profile
    for (unsigned int i = 0; i < vProfile.size( ); ++i)
    {
        float fAngle;
        if (i == static_cast<unsigned int>(m_nProfileSegments - 1))
        {
            fAngle = c_fHalfPi;
        }
        else if (i == 0)
        {
            float fNextAngle = atan2f(vProfile[i + 1][2] - vProfile[i][2], vProfile[i + 1][1] - vProfile[i][1]);
            fAngle = fNextAngle + c_fHalfPi;
        }
        else if (i == vProfile.size( ) - 1)
        {
            float fPreviousAngle = atan2f(vProfile[i][2] - vProfile[i - 1][2], vProfile[i][1] - vProfile[i - 1][1]);
            fAngle = fPreviousAngle + c_fHalfPi;
      }
        else
        {
            float fNextAngle = atan2f(vProfile[i + 1][2] - vProfile[i][2], vProfile[i + 1][1] - vProfile[i][1]);
            float fPreviousAngle = atan2f(vProfile[i][2] - vProfile[i - 1][2], vProfile[i][1] - vProfile[i - 1][1]);
            fAngle = (0.5f * (fPreviousAngle + fNextAngle)) + c_fHalfPi;
        }
        CVec3 cNormal(0.0f, cosf(fAngle), sinf(fAngle));
        cNormal.Normalize( );
        vNormals.push_back(cNormal);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::SetTextureCoords definition

void CFrondEngine::SetTextureCoords(CIndexedGeometry* pGeometry, unsigned int nFrondMapIndex, const float* pTexCoords, bool bFlip)
{
    // flip the coords if necessary
    float afCoords[8];
    memcpy(afCoords, pTexCoords, 8 * sizeof(float));
    if (bFlip)
    {
        for (int i = 1; i < 8; i +=2)
            afCoords[i] = -afCoords[i];
    }

    // check all of the vertices
    unsigned char chIndex = static_cast<unsigned char>(nFrondMapIndex);
    pGeometry->ResetVertexCounter( );
    for (int i = 0; i < pGeometry->GetVertexCount( ); ++i)
    {
        pGeometry->ChangeTexCoord(chIndex, afCoords);
        pGeometry->AdvanceVertexCounter( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::operator= definition

const CFrondEngine& CFrondEngine::operator=(const CFrondEngine& cRight)
{
    if (&cRight != this)
    {
        /*
        // general
        //lint -e{ 1555 }  { pointer copy OK here }
        m_pGeometry = cRight.m_pGeometry;
        //lint -e{ 1555 }  { pointer copy OK here }
        m_pLightingEngine = cRight.m_pLightingEngine;

        // frond guides
        m_vGuides = cRight.m_vGuides;
        m_vGuideLods = cRight.m_vGuideLods;
        */

        // frond parameters
        m_eFrondType = cRight.m_eFrondType;
        m_nNumBlades = cRight.m_nNumBlades;
        //delete m_pProfile;
        //m_pProfile = new CIdvBezierSpline;
        *m_pProfile = *cRight.m_pProfile;
        m_nProfileSegments = cRight.m_nProfileSegments;
        m_nLevel = cRight.m_nLevel;
        m_bEnabled = cRight.m_bEnabled;
        m_vTextures = cRight.m_vTextures;
        m_nNumLodLevels = cRight.m_nNumLodLevels;
        m_fMaxSurfaceAreaPercent = cRight.m_fMaxSurfaceAreaPercent;
        m_nNumLodLevels = cRight.m_nNumLodLevels;
        m_fMaxSurfaceAreaPercent = cRight.m_fMaxSurfaceAreaPercent;
        m_fMinSurfaceAreaPercent = cRight.m_fMinSurfaceAreaPercent;
        m_fReductionFuzziness = cRight.m_fReductionFuzziness;
        m_fLargeRetentionPercent = cRight.m_fLargeRetentionPercent;
        m_nMinLengthSegments = cRight.m_nMinLengthSegments;
        m_nMinCrossSegments = cRight.m_nMinLengthSegments;
    }

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  class CGuideSorter definition
//
//  Function object used to sort the guides by surface area.

class CGuideSorter
{
public:
    bool operator()(const SFrondGuide& cLeft, const SFrondGuide& cRight)
    {
        return (cLeft.m_fFuzzySurfaceArea > cRight.m_fFuzzySurfaceArea);
    }
};


///////////////////////////////////////////////////////////////////////  
//  CFrondEngine::BuildGuideLods definition

void CFrondEngine::BuildGuideLods( )
{
    // compute total surface area of all fronds
    float fTotalArea = 0.0f;
    float fLargestArea = 0.0f;
    for (unsigned int i = 0; i < m_vGuides.size( ); ++i)
    {
        fTotalArea += m_vGuides[i].m_fSurfaceArea;

        if (m_vGuides[i].m_fSurfaceArea > fLargestArea)
            fLargestArea = m_vGuides[i].m_fSurfaceArea;
    }

    // increase area to make fuzziness more effective (fronds do not typically benefit from the huge trunk)
    float fSquaredArea = fLargestArea * fLargestArea;

    // implement fuzziness
    CIdvRandom cRandom;
    float fSavePercent = 1.0f - m_fLargeRetentionPercent;
    vector<SFrondGuide> vSavedFronds;
    for (unsigned int i = 0; i < m_vGuides.size( ); ++i)
    {
        float fArea = m_vGuides[i].m_fSurfaceArea;

        if (fArea > fLargestArea * fSavePercent)
        {
            vSavedFronds.push_back(m_vGuides[i]);
            (void) m_vGuides.erase(m_vGuides.begin( ) + i--);
        }
        else
        {
            // compute a new, fuzzy volume
            float fFuzziness = cRandom.GetUniform(0.0f, m_fReductionFuzziness), fOneMinusFuzziness = 1.0f - fFuzziness;
            fArea = (fOneMinusFuzziness * fArea) + (fFuzziness * fSquaredArea);
            m_vGuides[i].m_fFuzzySurfaceArea = fArea;
        }
    }

    // sort branches in descending order of volume and add insert save branches at the beginning
    sort(m_vGuides.begin( ), m_vGuides.end( ), CGuideSorter( ));
    sort(vSavedFronds.begin( ), vSavedFronds.end( ), CGuideSorter( ));
    for (unsigned int i = 0; i < vSavedFronds.size( ); ++i)
        (void) m_vGuides.insert(m_vGuides.begin( ), vSavedFronds[i]);

    // for each LOD level, compute a target volume and use enough
    // branches to reach that target
    for (unsigned int i = 0; i < m_nNumLodLevels; ++i)
    {
        // figure what part of the branch structure stays for lod level i, computed by volume
        float fPercent;
        if (m_nNumLodLevels == 1)
            fPercent = m_fMaxSurfaceAreaPercent;
        else
            fPercent = VecInterpolate(m_fMaxSurfaceAreaPercent, m_fMinSurfaceAreaPercent, i / float(m_nNumLodLevels - 1));

        float fTargetVolume = fPercent * fTotalArea;

        // which branches will contribute to lod level i
        float fLodArea = 0.0f;

        vector<SFrondGuide> vLodLevel;
        if (fTargetVolume > 0.0f)
        {
            for (unsigned int j = 0; j < m_vGuides.size( ) && fLodArea <= fTargetVolume; ++j)
            {
                fLodArea += m_vGuides[j].m_fSurfaceArea;
                vLodLevel.push_back(m_vGuides[j]);
            }
        }
        m_vGuideLods.push_back(vLodLevel);
    }
}
