///////////////////////////////////////////////////////////////////////  
//         Name: IndexedGeometry.cpp
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

#pragma warning (disable : 4786)
#include "Debug.h"
#include "WindEngine.h"
#include "IndexedGeometry.h"
#include "UpVector.h"
#include "Endian.h"
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::CIndexedGeometry definition

CIndexedGeometry::CIndexedGeometry(CWindEngine* pWindEngine, bool bRetainTexCoords) :
    m_bRetainTexCoords(bRetainTexCoords),
    m_bValid(true),
    m_eWindMethod(CSpeedTreeRT::WIND_NONE),
    m_pWindEngine(pWindEngine),
    m_nNumLodLevels(0),
    m_bVertexWeighting(false),
    m_bManualLighting(false),
    m_nVertexCounter(0),
    m_nStripCounter(0),
    m_nVertexSize(0),
    m_pVertexWindComputed(NULL),
    m_nVertexCounterLodLevel(0)
{
    st_assert(pWindEngine);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::~CIndexedGeometry definition

CIndexedGeometry::~CIndexedGeometry( )
{
    delete[] m_pVertexWindComputed;
    m_pVertexWindComputed = NULL;

    DeleteIndexData( );
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::CombineStrips definition

void CIndexedGeometry::CombineStrips(bool bToggleFaceOrdering)
{
    // no need to concat empty strips
    if (GetVertexCount( ) > 0)
    {
        // these will replace the class member variables m_pNumStrips, m_pStripLengths, and m_pStrips
        vector<vector<unsigned short> > vCompositeStripLengths(m_nNumLodLevels);
        vector<vector<unsigned short*> > vCompositeStrips(m_nNumLodLevels);
        vector<int> vCompositeTriangleCounts(m_nNumLodLevels);

        // for each LOD level, concat the strips together as one
        for (unsigned short usLod = 0; usLod < m_nNumLodLevels; ++usLod)
        {
            // how many indexes are there for this LOD?
            int nCompositeLength = 0;
            for (unsigned short usStrip = 0; usStrip < GetNumStrips(usLod); ++usStrip)
                nCompositeLength += m_vStripLengths[usLod][usStrip];

            if (nCompositeLength > 0)
            {
                // more indices between the strips
                if (bToggleFaceOrdering)
                {
                    // need to stitch with different number of vertices for correct 2-sided lighting
                    for (unsigned short usStrip = 0; usStrip < GetNumStrips(usLod) - 1; ++usStrip)
                    {
                        unsigned short usStripLength = m_vStripLengths[usLod][usStrip];
                        if (usStripLength % 2 == 0)
                            nCompositeLength += 2;
                        else
                            nCompositeLength += 3;
                    }
                }
                else
                {
                    // add four more indexes between each individual strip
                    int nNumStrips = GetNumStrips(usLod);
                    if (nNumStrips > 0)
                        nNumStrips--;
                    nCompositeLength += 2 * nNumStrips;
                }

                // make new composite strip
                unsigned short* pCompositeStrip = new unsigned short[nCompositeLength];
                unsigned short* pStripPointer = pCompositeStrip;
                for (unsigned short usStrip = 0; usStrip < GetNumStrips(usLod); ++usStrip)
                {
                    // copy the strip
                    unsigned short usStripLength = m_vStripLengths[usLod][usStrip];
                    memcpy(pStripPointer, m_vStrips[usLod][usStrip], usStripLength * sizeof(unsigned short));
                    pStripPointer += usStripLength; 

                    // don't bridge if this is the last strip
                    if (usStrip < GetNumStrips(usLod) - 1)
                    {
                        if (bToggleFaceOrdering)
                        {
                            // need to stitch with different number of vertices for correct 2-sided lighting
                            if (usStripLength % 2 == 0)
                            {
                                // add last vertex again
                                *pStripPointer++ = m_vStrips[usLod][usStrip][usStripLength - 1];

                                // add first vertex of next strip
                                unsigned short x = m_vStrips[usLod][usStrip + 1][0];
                                *pStripPointer++ = x;
                            }
                            else
                            {
                                // add last vertex again, twice
                                *pStripPointer++ = m_vStrips[usLod][usStrip][usStripLength - 1];
                                *pStripPointer++ = m_vStrips[usLod][usStrip][usStripLength - 1];

                                // add first vertex of next strip 
                                unsigned short x = m_vStrips[usLod][usStrip + 1][0];
                                *pStripPointer++ = x;
                            }
                        }
                        else
                        {
                            // add last vertex again
                            *pStripPointer++ = m_vStrips[usLod][usStrip][usStripLength - 1];

                            // add first vertex of next strip
                            unsigned short x = m_vStrips[usLod][usStrip + 1][0];
                            *pStripPointer++ = x;
                        }
                    }
                }

                // force values to reflect single strip data
                vCompositeStripLengths[usLod].push_back(static_cast<unsigned short>(nCompositeLength));
                vCompositeStrips[usLod].push_back(pCompositeStrip);
                vCompositeTriangleCounts[usLod] = nCompositeLength - 2;
            }
            else
            {
                vCompositeStripLengths[usLod].clear( );
                vCompositeStrips[usLod].clear( );
                vCompositeTriangleCounts[usLod] = 0;
            }
        }

        // remove all of the old separate strips
        DeleteIndexData( );

        // replace member vars with new composite strip data
        m_vStripLengths = vCompositeStripLengths;
        m_vStrips = vCompositeStrips;
        m_vTriangleCounts = vCompositeTriangleCounts;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::ComputeExtents definition

void CIndexedGeometry::ComputeExtents(CRegion& cExtents) const
{
    unsigned short usVertexCount = GetVertexCount( );
    for (int i = 0; i < usVertexCount; ++i)
    {
        const float* pCoord = GetVertexCoord(i);
        CVec3 cCoord(pCoord[0], pCoord[1], pCoord[2]);

        // operator^ expands cExtents to include cCoord
        cExtents = cExtents ^ cCoord;
    }
}



///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::SetNumLodLevels definition

void CIndexedGeometry::SetNumLodLevels(unsigned short nLevels)
{
    m_vStripLengths.clear( );
    m_vStrips.clear( );
    m_vTriangleCounts.clear( );

    m_nNumLodLevels = nLevels;

    m_vStripLengths.resize(nLevels);
    m_vStrips.resize(nLevels);
    m_vTriangleCounts.resize(nLevels);

    for (int i = 0; i < nLevels; ++i)
        m_vTriangleCounts[i] = 0;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::Transform definition

void CIndexedGeometry::Transform(const CTransform& cTransform)
{
    unsigned short usVertexCount = GetVertexCount( );
    for (int i = 0; i < usVertexCount; ++i)
    {
        // transform coordinates
        float* pCoord = const_cast<float*>(GetVertexCoord(i));
        CVec3 cCoord(pCoord[0], pCoord[1], pCoord[2]);
        cCoord = cCoord * cTransform;
        memcpy(pCoord, cCoord.m_afData, 3 * sizeof(float));

        // orig coordinates are used as a basis for wind computations,
        // need to transform those too, if they are being used
        if (m_bVertexWeighting &&
            !m_vOrigCoords.empty( ))
        {
            float* pOrigCoord = const_cast<float*>(GetOrigVertexCoord(i));
            cCoord.Set(pOrigCoord[0], pOrigCoord[1], pOrigCoord[2]);
            cCoord = cCoord * cTransform;
            memcpy(pOrigCoord, cCoord.m_afData, 3 * sizeof(float));
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddStrip definition

void CIndexedGeometry::AddStrip(unsigned short nLodLevel, unsigned short* pStrip, unsigned short nStripLength)
{
    st_assert(nLodLevel < m_nNumLodLevels);
    //st_assert(pStrip);

    // record strip length
    m_vStripLengths[nLodLevel].resize(m_nStripCounter + 1);
    m_vStripLengths[nLodLevel][m_nStripCounter] = nStripLength;

    // record strip pointer
    m_vStrips[nLodLevel].resize(m_nStripCounter + 1);
    m_vStrips[nLodLevel][m_nStripCounter] = pStrip;

    // record triangle count
    m_vTriangleCounts[nLodLevel] += nStripLength - 2;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexColor definition

void CIndexedGeometry::AddVertexColor(const float* pColor)
{
    st_assert(pColor);

    // convert the 4-float value into a single unsigned long
    unsigned long ulColor = EndianSwap32(ColorFloatsToLong(pColor));

    // store new color
    m_vColors.push_back(ulColor);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexCoord definition

void CIndexedGeometry::AddVertexCoord(const float* pCoord)
{
    st_assert(pCoord);

    // adjust for current up vector
    float afNewCoord[3];
    Assign3d(afNewCoord, pCoord);

    // store new coordinate
    m_vCoords.push_back(afNewCoord[0]);
    m_vCoords.push_back(afNewCoord[1]);
    m_vCoords.push_back(afNewCoord[2]);

    // if using CPU-based wind we need to have a copy of the coordinates that are
    // unaffected by wind
    if (m_bVertexWeighting &&
        m_eWindMethod == CSpeedTreeRT::WIND_CPU)
    {
        m_vOrigCoords.push_back(afNewCoord[0]);
        m_vOrigCoords.push_back(afNewCoord[1]);
        m_vOrigCoords.push_back(afNewCoord[2]);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexNormal definition

void CIndexedGeometry::AddVertexNormal(const float* pNormal)
{
    st_assert(pNormal);

    // store new normal
    m_vNormals.push_back(pNormal[0]);
    m_vNormals.push_back(pNormal[1]);
    m_vNormals.push_back(pNormal[2]);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexTexCoord0 definition

void CIndexedGeometry::AddVertexTexCoord0(const float* pTexCoord, short sIndex)
{
    st_assert(pTexCoord);

    // texture coordinates are randomly accessed by the CFrondEngine class, so
    // we put some code here to compensate
    if (m_nVertexCounter == m_vTexCoords0.size( ) / 2)
    {
        m_vTexCoords0.resize(m_vTexCoords0.size( ) + 2);
        if (sIndex > -1 &&
            m_bRetainTexCoords)
        {
            m_vTexIndices0.resize(m_vTexIndices0.size( ) + 1);
            m_vOrigTexCoords0.resize(m_vOrigTexCoords0.size( ) + 2);
        }
    }

    // Copies of the texture coordinates and texture indices are used to facilitate functions
    // that exist solely for use in SpeedTreeCAD, SpeedTreeMAX, and SpeedTreeMAYA.  There's
    // pretty much no reason the end user will need to access these.
    if (sIndex > -1 &&
        m_bRetainTexCoords)
    {
        m_vTexIndices0[m_nVertexCounter] = static_cast<unsigned char>(sIndex);
        m_vOrigTexCoords0[m_nVertexCounter * 2] = pTexCoord[0];
        m_vOrigTexCoords0[m_nVertexCounter * 2 + 1] = pTexCoord[1];
    }

    // store the new texture coords
    m_vTexCoords0[m_nVertexCounter * 2] = pTexCoord[0];
    m_vTexCoords0[m_nVertexCounter * 2 + 1] = CSpeedTreeRT::GetTextureFlip( ) ? -pTexCoord[1] : pTexCoord[1];
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexTexCoord1 definition

void CIndexedGeometry::AddVertexTexCoord1(const float* pTexCoord)
{
    st_assert(pTexCoord);

    // store the new texture coords
    m_vTexCoords1.push_back(pTexCoord[0]);
    if (CSpeedTreeRT::GetTextureFlip( ))
        m_vTexCoords1.push_back(-pTexCoord[1]);
    else
        m_vTexCoords1.push_back(pTexCoord[1]);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexWind definition

void CIndexedGeometry::AddVertexWind(float fWindWeight, unsigned char ucWindMatrixIndex)
{
    // computed values are opposite of those needed by the app
    fWindWeight = 1.0f - fWindWeight;
    
    // chWindMatrixIndex spans a wide range of matrices, mod it down to size
    unsigned int nStartingMatrix = 0, nMatrixSpan = 1;
    m_pWindEngine->GetLocalMatrices(nStartingMatrix, nMatrixSpan);
    ucWindMatrixIndex = static_cast<unsigned char>(nStartingMatrix + ucWindMatrixIndex % nMatrixSpan);

    // store the attributes
    m_vWindWeights.push_back(fWindWeight);
    m_vWindMatrixIndices.push_back(ucWindMatrixIndex);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexTangent definition

void CIndexedGeometry::AddVertexTangent(const float* pTangent)
{
    st_assert(pTangent);

    float afAdjustedTangent[3];
    Assign3d(afAdjustedTangent, pTangent);

    m_vTangents.push_back(afAdjustedTangent[0]);
    m_vTangents.push_back(afAdjustedTangent[1]);
    m_vTangents.push_back(afAdjustedTangent[2]);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::AddVertexBinormal definition

void CIndexedGeometry::AddVertexBinormal(const float* pBinormal)
{
    st_assert(pBinormal);

    float afAdjustedBinormal[3];
    Assign3d(afAdjustedBinormal, pBinormal);

    m_vBinormals.push_back(afAdjustedBinormal[0]);
    m_vBinormals.push_back(afAdjustedBinormal[1]);
    m_vBinormals.push_back(afAdjustedBinormal[2]);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::ResetStripCounter definition

void CIndexedGeometry::ResetStripCounter(unsigned short nLodLevel)
{
//    st_assert(nLodLevel < m_nNumLodLevels);

    m_nVertexCounterLodLevel = nLodLevel; 
    m_nStripCounter = 0;
    m_vTriangleCounts[nLodLevel] = 0;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetStrip definition

unsigned short* CIndexedGeometry::GetStrip(unsigned short nLodLevel, unsigned short& nStripLength) const
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(m_nStripCounter < GetNumStrips(nLodLevel));

    nStripLength = m_vStripLengths[nLodLevel][m_nStripCounter];

    return m_vStrips[nLodLevel][m_nStripCounter];
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetStripsPointer definition

unsigned short** CIndexedGeometry::GetStripsPointer(unsigned short nLodLevel) const
{
    st_assert(nLodLevel < m_nNumLodLevels);
	
    if (m_vStrips[nLodLevel].size() > 0)
        return const_cast<unsigned short**>(&(m_vStrips[nLodLevel][0]));
    else
        return NULL;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetTriangleCount definition

unsigned int CIndexedGeometry::GetTriangleCount(unsigned short nLodLevel) const
{
    st_assert(nLodLevel < m_nNumLodLevels);

    return m_vTriangleCounts[nLodLevel];
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::ComputeWindEffect definition
//
//  This is the code for the CPU branch wind solution.  It takes two 
//  points per vertex: the original vertex, and that vertex multiplied
//  times the appropriate wind matrix, and then interpolates between them
//  based on the wind weight value (0.0 to 1.0).

bool CIndexedGeometry::ComputeWindEffect(unsigned short nLodLevel)
{
    st_assert(m_bVertexWeighting);
    st_assert(m_pWindEngine);

    bool bGeometryUpdated = false;
    if (!IsValid( ))
    {
        unsigned short usVertexCount = GetVertexCount( );
        if (!m_pVertexWindComputed)
            m_pVertexWindComputed = new bool[usVertexCount];
        for (int i = 0; i < usVertexCount; ++i)
            m_pVertexWindComputed[i] = false; // makes sure we don't recompute any vertex

        CVec3 cFullWindEffect, cInterpWindEffect;
        for (int nStrip = 0; nStrip < GetNumStrips(nLodLevel); ++nStrip)
        {
            for (int nVertex = 0; nVertex < m_vStripLengths[nLodLevel][nStrip]; ++nVertex)
            {
                unsigned short nIndex = m_vStrips[nLodLevel][nStrip][nVertex];
                if (!m_pVertexWindComputed[nIndex])
                {
                    float fWeight = m_vWindWeights[nIndex];
                    float* pOrigCoord = &(m_vOrigCoords[0]) + 3 * nIndex;

                    cFullWindEffect.Set(pOrigCoord[0], pOrigCoord[1], pOrigCoord[2]);
                    unsigned short usWindGroup = m_vWindMatrixIndices[nIndex];
                    cFullWindEffect = cFullWindEffect * m_pWindEngine->GetWindMatrix(usWindGroup);
                    cInterpWindEffect.Set(VecInterpolate(pOrigCoord[0], cFullWindEffect[0], fWeight),
                                          VecInterpolate(pOrigCoord[1], cFullWindEffect[1], fWeight),
                                          VecInterpolate(pOrigCoord[2], cFullWindEffect[2], fWeight));

                    memcpy(&(m_vCoords[0]) + nIndex * 3, cInterpWindEffect.m_afData, 3 * sizeof(float));

                    m_pVertexWindComputed[nIndex] = true;
                }
            }
        }

        Validate( );
        bGeometryUpdated = true;
    }

    return bGeometryUpdated;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::DeleteLodStrip definition

void CIndexedGeometry::DeleteLodStrip(unsigned short usLodLevel)
{
    st_assert(usLodLevel < m_nNumLodLevels);

    for (int i = 0; i < GetNumStrips(usLodLevel); ++i)
    {
        delete[] m_vStrips[usLodLevel][i];
        m_vStrips[usLodLevel][i] = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::DeleteIndexData definition

void CIndexedGeometry::DeleteIndexData(void)
{
    for (unsigned int i = 0; i < m_vStrips.size( ); ++i)
    {
        for (unsigned int j = 0; j < m_vStrips[i].size( ); ++j)
        {
            delete[] m_vStrips[i][j];
            m_vStrips[i][j] = NULL;
        }
        m_vStrips[i].clear( );
    }
    m_vStrips.clear( );
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetVertexCoord definition

const float* CIndexedGeometry::GetVertexCoord(unsigned int nVertexIndex) const
{
    st_assert(nVertexIndex < m_vCoords.size( ) / 3);

    return &(m_vCoords[0]) + nVertexIndex * 3;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetVertexTexCoord0 definition

const float* CIndexedGeometry::GetVertexTexCoord0(unsigned int nVertexIndex) const
{
    st_assert(nVertexIndex < m_vTexCoords0.size( ) / 2);

    return &(m_vTexCoords0[nVertexIndex * 2]);
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::GetOrigVertexCoord definition

const float* CIndexedGeometry::GetOrigVertexCoord(unsigned int nVertexIndex) const
{
    st_assert(nVertexIndex < m_vOrigCoords.size( ) / 3);

    return &(m_vOrigCoords[0]) + nVertexIndex * 3;
}


///////////////////////////////////////////////////////////////////////  
//  CIndexedGeometry::ChangeTexCoord definition

void CIndexedGeometry::ChangeTexCoord(unsigned char chChangedMapIndex, float* pNewTexCoords)
{
    st_assert(m_bRetainTexCoords);

    if (chChangedMapIndex == m_vTexIndices0[m_nVertexCounter])
    {
        float afTexCoords[2] = 
        {
            VecInterpolate(pNewTexCoords[2], pNewTexCoords[0], m_vOrigTexCoords0[m_nVertexCounter * 2]),
            VecInterpolate(pNewTexCoords[5], pNewTexCoords[1], m_vOrigTexCoords0[m_nVertexCounter * 2 + 1])
        };

        if (CSpeedTreeRT::GetTextureFlip( ))
            afTexCoords[1] = -afTexCoords[1];

        AddVertexTexCoord0(afTexCoords);
    }
}

