//         Name: BranchGeometry.cpp
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

#include "SpeedTreeRT.h"
#include "WindEngine.h"
#include "BranchGeometry.h"
#include "UpVector.h"
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::CBranchGeometry definition

CBranchGeometry::CBranchGeometry(CWindEngine* pWindEngine, bool bRetainTexCoords) :
    m_bRetainTexCoords(bRetainTexCoords),
    m_bValid(true),
    m_pWindEngine(pWindEngine),
    m_nNumLodLevels(0),
    m_bVertexWeighting(false),
    m_bManualLighting(false),
    m_nNumVertices(0),
    m_nVertexCounter(0),
    m_pNumStrips(NULL),
    m_nStripCounter(0),
    m_pStripLengths(NULL),
    m_pStrips(NULL),
    m_nVertexSize(0),
    m_pVertexWindComputed(NULL),
    m_pOrigCoords(NULL),
    m_pOrigTexCoords(NULL),
    m_pTexIndices(NULL),
    m_pTriangleCounts(NULL),
#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
    m_pInterleavedVertexes(NULL),
#else
    m_pColors(NULL),
    m_pCoords(NULL),
    m_pNormals(NULL),
    m_pTexCoords(NULL),
#endif
    m_pWeights(NULL),
    m_pWindGroups(NULL)
{
    st_assert(pWindEngine);
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::~CBranchGeometry definition

CBranchGeometry::~CBranchGeometry( )
{
    DeleteAll( );
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::CombineStrips definition

void CBranchGeometry::CombineStrips(void)
{
    // no need to concat empty strips
    if (m_nNumVertices > 0)
    {
        // these will replace the class member variables m_pNumStrips, m_pStripLengths, and m_pStrips
        unsigned short* pCompositeNumStrips = new unsigned short[m_nNumLodLevels];
        unsigned short** pCompositeStripLengths = new unsigned short*[m_nNumLodLevels];
        unsigned short*** pCompositeStrips = new unsigned short**[m_nNumLodLevels];
        unsigned int* pCompositeTriangleCounts = new unsigned int[m_nNumLodLevels];

        // for each LOD level, concat the strips together as one
        for (int nLod = 0; nLod < m_nNumLodLevels; ++nLod)
        {
            // how many indexes are there for this LOD?
            int nCompositeLength = 0;
			int nStrip;
            for (nStrip = 0; nStrip < m_pNumStrips[nLod]; ++nStrip)
                nCompositeLength += m_pStripLengths[nLod][nStrip];

            // add four more indexes between each individual strip
            int nNumStrips = m_pNumStrips[nLod];
            if (nNumStrips > 0)
                nNumStrips--;
            nCompositeLength += 4 * nNumStrips;

            if (nCompositeLength > 0)
            {
                // make new composite strip
                unsigned short* pCompositeStrip = new unsigned short[nCompositeLength];
                unsigned short* pStripPointer = pCompositeStrip;
                for (nStrip = 0; nStrip < m_pNumStrips[nLod]; ++nStrip)
                {
                    // copy the strip
                    unsigned short usStripLength = m_pStripLengths[nLod][nStrip];
                    memcpy(pStripPointer, m_pStrips[nLod][nStrip], usStripLength * sizeof(unsigned short));
                    pStripPointer += usStripLength; 

                    // don't bridge if this is the last strip
                    if (nStrip < m_pNumStrips[nLod] - 1)
                    {
                        // add last vertex again
                        *pStripPointer++ = m_pStrips[nLod][nStrip][usStripLength - 1];

                        // add first vertex of next strip twice
                        *pStripPointer++ = m_pStrips[nLod][nStrip + 1][0];
                        *pStripPointer++ = m_pStrips[nLod][nStrip + 1][0];

                        // add second vertex of next strip
                        st_assert(m_pStripLengths[nLod][nStrip + 1] > 1);
                        *pStripPointer++ = m_pStrips[nLod][nStrip + 1][1];
                    }
                }

                // force values to reflect single strip data
                pCompositeNumStrips[nLod] = (m_pNumStrips[nLod] == 0) ? 0 : 1;
                pCompositeStripLengths[nLod] = new unsigned short[1];
                pCompositeStripLengths[nLod][0] = nCompositeLength;
                pCompositeStrips[nLod] = new unsigned short*[1];
                pCompositeStrips[nLod][0] = pCompositeStrip;
                pCompositeTriangleCounts[nLod] = nCompositeLength - 2;
            }
            else
            {
                pCompositeNumStrips[nLod] = 0;
                pCompositeStripLengths[nLod] = NULL;
                pCompositeStrips[nLod] = NULL;
                pCompositeTriangleCounts[nLod] = 0;
            }
        }

        // remove all of the old separate strips
        DeleteIndexData( );

        // replace member vars with new composite strip data
        m_pNumStrips = pCompositeNumStrips;
        m_pStripLengths = pCompositeStripLengths;
        m_pStrips = pCompositeStrips;
        m_pTriangleCounts = pCompositeTriangleCounts;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::ComputeExtents definition

void CBranchGeometry::ComputeExtents(CRegion& cExtents) const
{
    for (int i = 0; i < m_nNumVertices; ++i)
    {
        const float* pCoord = GetVertexCoord(i);
        CVec3 cCoord(pCoord[0], pCoord[1], pCoord[2]);

        // operator^ expands cExtents to include cCoord
        cExtents = cExtents ^ cCoord;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::SetNumLodLevels definition

void CBranchGeometry::SetNumLodLevels(unsigned short nLevels)
{
    st_assert(!m_pNumStrips);
    st_assert(!m_pStripLengths);
    st_assert(!m_pStrips);

    m_nNumLodLevels = nLevels;

    m_pNumStrips = new unsigned short[nLevels];
    m_pStripLengths = new unsigned short*[nLevels];
    m_pStrips = new unsigned short**[nLevels];
    m_pTriangleCounts = new unsigned int[nLevels];

    for (int i = 0; i < nLevels; ++i)
    {
        m_pNumStrips[i] = 0;
        m_pStripLengths[i] = NULL;
        m_pStrips[i] = NULL;
        m_pTriangleCounts[i] = 0;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::Transform definition

void CBranchGeometry::Transform(const CTransform& cTransform)
{
    for (int i = 0; i < m_nNumVertices; ++i)
    {
        // transform coordinates
        float* pCoord = GetVertexCoord(i);
        CVec3 cCoord(pCoord[0], pCoord[1], pCoord[2]);
        cCoord = cCoord * cTransform;
        memcpy(pCoord, cCoord.m_afData, 3 * sizeof(float));

        // orig coordinates are used as a basis for wind computations,
        // need to transform those too, if they are being used
        if (m_pOrigCoords && m_bVertexWeighting)
        {
            float* pCoord = GetOrigVertexCoord(i);
            cCoord.Set(pCoord[0], pCoord[1], pCoord[2]);
            cCoord = cCoord * cTransform;
            memcpy(pCoord, cCoord.m_afData, 3 * sizeof(float));
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::SetNumStrips definition

void CBranchGeometry::SetNumStrips(unsigned short nLodLevel, unsigned short nNumStrips)
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(m_pNumStrips);
    st_assert(m_pStripLengths);
    st_assert(m_pStrips);

    m_pNumStrips[nLodLevel] = nNumStrips;
    delete[] m_pStripLengths[nLodLevel];
    m_pStripLengths[nLodLevel] = new unsigned short[nNumStrips];
    delete[] m_pStrips[nLodLevel];
    m_pStrips[nLodLevel] = new unsigned short*[nNumStrips];

    for (int i = 0; i < nNumStrips; ++i)
    {
        m_pStripLengths[nLodLevel][i] = NULL;
        m_pStrips[nLodLevel][i] = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddStrip definition

void CBranchGeometry::AddStrip(unsigned short nLodLevel, unsigned short* pStrip, unsigned short nStripLength, unsigned int nTriangleCount)
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(pStrip);
    st_assert(m_nStripCounter < m_pNumStrips[nLodLevel]);
    st_assert(m_pStrips);
    st_assert(m_pStripLengths);

    m_pStripLengths[nLodLevel][m_nStripCounter] = nStripLength;
    delete[] m_pStrips[nLodLevel][m_nStripCounter];
    m_pStrips[nLodLevel][m_nStripCounter] = pStrip;
    m_pTriangleCounts[nLodLevel] += nStripLength - 2;
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::SetNumVertices definition

void CBranchGeometry::SetNumVertices(unsigned short nNumVertexes)
{
    m_nNumVertices = nNumVertexes;

    if (m_bVertexWeighting &&
        m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
    {
        m_pVertexWindComputed = new bool[nNumVertexes];
        m_pOrigCoords = new float[3 * nNumVertexes];
    }

    if (m_bRetainTexCoords)
    {
        m_pOrigTexCoords = new float[2 * nNumVertexes];
        m_pTexIndices = new unsigned char[nNumVertexes];
    }

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    if (m_bManualLighting)
        m_nVertexSize = (2 + 3) * sizeof(float) + 4;    // texcoords(s,t) + coords(x,y,z) + unsigned long color 
    else
        m_nVertexSize = (2 + 3 + 3) * sizeof(float);    // texcoords(s,t) + normal(nx,ny,nz) + coords(x,y,z)

    if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
        m_nVertexSize += sizeof(float) + 1;             // weight (single float) + wind group (1 byte)
    else if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
    {
        m_pWeights = new float[nNumVertexes];
        m_pWindGroups = new byte[nNumVertexes];
    }
    st_assert(m_nVertexSize > 0);
    m_pInterleavedVertexes = new byte[m_nVertexSize * nNumVertexes];
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    if (m_bManualLighting)
        m_nVertexSize = (2 + 3) * sizeof(float) + 4;    // texcoords(s,t) + coords(x,y,z) + unsigned long color
    else
        m_nVertexSize = (3 + 3 + 2) * sizeof(float);    // coords(x,y,z) + normal(nx,ny,nz) + texcoords(s,t)

    if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
        m_nVertexSize += sizeof(float) + 4;             // weight (single float) + wind group (4 byte DWORD)
    else if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
    {
        m_pWeights = new float[nNumVertexes];
        m_pWindGroups = new byte[nNumVertexes];
    }
    st_assert(m_nVertexSize > 0);
    m_pInterleavedVertexes = new byte[m_nVertexSize * nNumVertexes];
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    m_nVertexSize = 0;                                  // each vertex attribute is stored in a separate array
    m_pCoords = new float[3 * nNumVertexes];
    m_pTexCoords = new float[2 * nNumVertexes];
    if (!m_bManualLighting)
        m_pNormals = new float[3 * nNumVertexes];
    if (m_bVertexWeighting)
    {
        m_pWeights = new float[nNumVertexes];
        m_pWindGroups = new byte[nNumVertexes];
    }

    if (m_bManualLighting)
        m_pColors = new float[4 * nNumVertexes];
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddVertexColor definition

void CBranchGeometry::AddVertexColor(const float* pColor)
{
    st_assert(m_nVertexCounter < m_nNumVertices);

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    unsigned long lColor = ColorFloatsToLong(pColor);
    memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 2 * sizeof(float), &lColor, sizeof(unsigned long));
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    unsigned long lColor = ColorFloatsToLong(pColor);
    if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
        memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float) + 8, &lColor, sizeof(unsigned long));
    else
        memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float), &lColor, sizeof(unsigned long));
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pColors);
    memcpy(m_pColors + m_nVertexCounter * 4, pColor, 4 * sizeof(float));
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddVertexCoord definition

void CBranchGeometry::AddVertexCoord(const float* pCoord)
{
    st_assert(m_nVertexCounter < m_nNumVertices);

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    if (m_bVertexWeighting &&
        m_pOrigCoords)
    {
        Assign3d(m_pOrigCoords + m_nVertexCounter * 3, pCoord);
    }
    if (m_bManualLighting)
    {
        Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 8 + 4), pCoord);
    }
    else
    {
        Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 5 * sizeof(float)), pCoord);
    }
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    if (m_bVertexWeighting &&
        m_pOrigCoords)
    {
        Assign3d(m_pOrigCoords + m_nVertexCounter * 3, pCoord);
    }
    Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 0), pCoord);
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pCoords);
    if (m_bVertexWeighting &&
        m_pOrigCoords)
    {
        Assign3d(m_pOrigCoords + m_nVertexCounter * 3, pCoord);
    }
    Assign3d(m_pCoords + m_nVertexCounter * 3, pCoord);
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddVertexNormal definition

void CBranchGeometry::AddVertexNormal(const float* pNormal)
{
    st_assert(m_nVertexCounter < m_nNumVertices);

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 2 * sizeof(float)), pNormal);
#endif
    
#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    if (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
    {
        Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float) + 8), pNormal);
    }
    else
    {
        Assign3d(reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float)), pNormal);
    }
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pNormals);
    Assign3d(m_pNormals + m_nVertexCounter * 3, pNormal);
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddVertexTexCoord definition

void CBranchGeometry::AddVertexTexCoord(const float* pTexCoord, short sIndex)
{
    st_assert(m_nVertexCounter < m_nNumVertices);

    // retain original values
    if (sIndex > -1 &&
        m_bRetainTexCoords)
    {
        st_assert(m_pOrigTexCoords);
        st_assert(m_pTexIndices);
        m_pTexIndices[m_nVertexCounter] = static_cast<unsigned char>(sIndex);
        memcpy(m_pOrigTexCoords + m_nVertexCounter * 2, pTexCoord, 2 * sizeof(float));
    }

    // store tex coord
#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 0 * sizeof(float), pTexCoord, 2 * sizeof(float));
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    byte* pWrite = m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float);
    pWrite += (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU) ? 8 : 0;
    pWrite += m_bManualLighting ? 4 : 3 * sizeof(float);

    reinterpret_cast<float*>(pWrite)[0] = pTexCoord[0];
    reinterpret_cast<float*>(pWrite)[1] = -pTexCoord[1];
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pTexCoords);
    m_pTexCoords[m_nVertexCounter * 2] = pTexCoord[0];
    m_pTexCoords[m_nVertexCounter * 2 + 1] = -pTexCoord[1];
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::AddVertexWind definition

void CBranchGeometry::AddVertexWind(float fWeight, unsigned char chGroup)
{
    st_assert(m_nVertexCounter < m_nNumVertices);
    fWeight = 1.0f - fWeight;

    // chGroup spans a wide range of matrices, mod it down to size
    unsigned int nStartingMatrix = 0, nMatrixSpan = 1;
    m_pWindEngine->GetLocalMatrices(nStartingMatrix, nMatrixSpan);

    chGroup = nStartingMatrix + chGroup % nMatrixSpan;

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    if (m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
    {
        st_assert(m_pInterleavedVertexes);
        byte* pWriteLocation = m_pInterleavedVertexes + (m_nVertexCounter + 1) * m_nVertexSize - 5;
        memcpy(pWriteLocation, &fWeight, sizeof(float));
        memcpy(pWriteLocation + 4, &chGroup, 1);
    }
    else if (m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
    {
        m_pWeights[m_nVertexCounter] = fWeight;
        m_pWindGroups[m_nVertexCounter] = chGroup;
    }
#endif
#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    if (m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU)
    {
        st_assert(m_pInterleavedVertexes);
        memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float), &fWeight, sizeof(float));
        unsigned long nWindGroup = chGroup + 1;
        memcpy(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float) + 4, &nWindGroup, sizeof(unsigned long));
    }
    else if (m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_CPU)
    {
        m_pWeights[m_nVertexCounter] = fWeight;
        m_pWindGroups[m_nVertexCounter] = chGroup;
    }
#endif
#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pWeights);
    st_assert(m_pWindGroups);
    m_pWeights[m_nVertexCounter] = fWeight;
    m_pWindGroups[m_nVertexCounter] = chGroup;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::ResetStripCounter definition

void CBranchGeometry::ResetStripCounter(unsigned short nLodLevel)
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(m_pTriangleCounts);

    m_nVertexCounterLodLevel = nLodLevel; 
    m_nStripCounter = 0;
    m_pTriangleCounts[nLodLevel] = 0;
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::GetStrip definition

unsigned short* CBranchGeometry::GetStrip(unsigned short nLodLevel, unsigned short& nStripLength) const
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(m_nStripCounter < m_pNumStrips[nLodLevel]);
    st_assert(m_pStrips);
    st_assert(m_pStripLengths);

    nStripLength = m_pStripLengths[nLodLevel][m_nStripCounter];

    return m_pStrips[nLodLevel][m_nStripCounter];
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::GetTriangleCount definition

unsigned int CBranchGeometry::GetTriangleCount(unsigned short nLodLevel) const
{
    st_assert(nLodLevel < m_nNumLodLevels);
    st_assert(m_pNumStrips);
    st_assert(m_pStripLengths);

    return m_pTriangleCounts[nLodLevel];
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::ComputeWindEffect definition
//
//  This is the code for the CPU branch wind solution.  It takes two 
//  points per vertex: the original vertex, and that vertex multiplied
//  times the appropriate wind matrix, and then interpolates between them
//  based on the wind weight value (0.0 to 1.0).

bool CBranchGeometry::ComputeWindEffect(unsigned short nLodLevel)
{
    st_assert(m_bVertexWeighting);
    st_assert(m_pVertexWindComputed);
    st_assert(m_pNumStrips && m_pStripLengths);
    st_assert(m_pWindEngine);
#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
    st_assert(m_pInterleavedVertexes);
#endif
    st_assert(m_pOrigCoords);

    bool bGeometryUpdated = false;
    if (!IsValid( ))
    {
        for (int i = 0; i < m_nNumVertices; ++i)
            m_pVertexWindComputed[i] = false; // makes sure we don't recompute any vertex

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
        unsigned short nCoordOffset = m_bManualLighting ? 12 : 20;
#endif
#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
        unsigned short nCoordOffset = 0;
#endif

        CVec3 cFullWindEffect, cInterpWindEffect;
        for (int nStrip = 0; nStrip < m_pNumStrips[nLodLevel]; ++nStrip)
        {
            for (int nVertex = 0; nVertex < m_pStripLengths[nLodLevel][nStrip]; ++nVertex)
            {
                unsigned short nIndex = m_pStrips[nLodLevel][nStrip][nVertex];
                if (!m_pVertexWindComputed[nIndex])
                {
                    float fWeight = m_pWeights[nIndex];
                    float* pOrigCoord = m_pOrigCoords + 3 * nIndex;

                    cFullWindEffect.Set(pOrigCoord[0], pOrigCoord[1], pOrigCoord[2]);
                    int nWindGroup = m_pWindGroups[nIndex];
                    cFullWindEffect = cFullWindEffect * m_pWindEngine->GetWindMatrix(nWindGroup);
                    cInterpWindEffect.Set(VecInterpolate(pOrigCoord[0], cFullWindEffect[0], fWeight),
                                          VecInterpolate(pOrigCoord[1], cFullWindEffect[1], fWeight),
                                          VecInterpolate(pOrigCoord[2], cFullWindEffect[2], fWeight));

#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
                    memcpy(m_pInterleavedVertexes + nIndex * m_nVertexSize + nCoordOffset, cInterpWindEffect.m_afData, 3 * sizeof(float));
#endif
#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
                    memcpy(m_pCoords + nIndex * 3, cInterpWindEffect.m_afData, 3 * sizeof(float));
#endif
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
//  CBranchGeometry::DeleteLodStrip definition

void CBranchGeometry::DeleteLodStrip(unsigned short sLodLevel)
{
    for (int i = 0; i < m_pNumStrips[sLodLevel]; ++i)
    {
        delete[] m_pStrips[sLodLevel][i];
        m_pStrips[sLodLevel][i] = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::DeleteAll definition

void CBranchGeometry::DeleteAll(void)
{
    delete[] m_pVertexWindComputed;
    m_pVertexWindComputed = NULL;
    delete[] m_pOrigCoords;
    m_pOrigCoords = NULL;

    delete[] m_pTexIndices;
    m_pTexIndices = NULL;
    delete[] m_pOrigTexCoords;
    m_pOrigTexCoords = NULL;

    // vertex data
#ifdef SPEEDTREE_DATA_IS_INTERLEAVED
    delete[] m_pInterleavedVertexes;
    m_pInterleavedVertexes = NULL;
#else
    delete[] m_pColors;
    delete[] m_pCoords;
    delete[] m_pNormals;
    delete[] m_pTexCoords;

    m_pColors = NULL;
    m_pCoords = NULL;
    m_pNormals = NULL;
    m_pTexCoords = NULL;
#endif

    delete[] m_pWeights;
    delete[] m_pWindGroups;
    m_pWeights = NULL;
    m_pWindGroups = NULL;

    DeleteIndexData( );
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::DeleteIndexData definition

void CBranchGeometry::DeleteIndexData(void)
{
    if (m_pNumStrips)
    {
        for (int i = 0; i < m_nNumLodLevels; ++i)
        {
            for (int j = 0; j < m_pNumStrips[i]; ++j)
            {
                delete[] m_pStrips[i][j];
                m_pStrips[i][j] = NULL;
            }
            delete[] m_pStrips[i];
            m_pStrips[i] = NULL;

            delete[] m_pStripLengths[i];
            m_pStripLengths[i] = NULL;
        }

        delete[] m_pStrips;
        m_pStrips = NULL;
        delete[] m_pStripLengths;
        m_pStripLengths = NULL;
        delete[] m_pNumStrips;
        m_pNumStrips = NULL;
        delete[] m_pTriangleCounts;
        m_pTriangleCounts = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::GetVertexCoord definition

float* CBranchGeometry::GetVertexCoord(unsigned int nVertexIndex) const
{
    st_assert(nVertexIndex < m_nNumVertices);

    float* pVertex = NULL;

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    if (m_bManualLighting)
        pVertex = reinterpret_cast<float*>(m_pInterleavedVertexes + nVertexIndex * m_nVertexSize + 8 + 4);
    else
        pVertex = reinterpret_cast<float*>(m_pInterleavedVertexes + nVertexIndex * m_nVertexSize + 5 * sizeof(float)); 
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    pVertex = reinterpret_cast<float*>(m_pInterleavedVertexes + nVertexIndex * m_nVertexSize + 0);
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pCoords);
    pVertex = m_pCoords + nVertexIndex * 3;
#endif

    return pVertex;
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::GetVertexTexCoord definition

float* CBranchGeometry::GetVertexTexCoord(unsigned int nVertexIndex) const
{
    st_assert(nVertexIndex < m_nNumVertices);

    float* pTexCoord = NULL;

#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
    st_assert(m_pInterleavedVertexes);
    pTexCoord = reinterpret_cast<float*>(m_pInterleavedVertexes + nVertexIndex * m_nVertexSize);
#endif

#ifdef SPEEDTREE_DATA_FORMAT_DIRECTX
    st_assert(m_pInterleavedVertexes);
    pTexCoord = reinterpret_cast<float*>(m_pInterleavedVertexes + m_nVertexCounter * m_nVertexSize + 3 * sizeof(float));
    pTexCoord += (m_bVertexWeighting && m_pWindEngine->GetBranchWindMethod( ) == CSpeedTreeRT::WIND_GPU) ? 2 : 0;
    pTexCoord += m_bManualLighting ? 1 : 3;
#endif

#ifdef SPEEDTREE_DATA_FORMAT_NETIMMERSE
    st_assert(m_pTexCoords);
    pTexCoord = m_pTexCoords + nVertexIndex * 2;
#endif

    return pTexCoord;
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::GetOrigVertexCoord definition

float* CBranchGeometry::GetOrigVertexCoord(unsigned int nVertexIndex) const
{
    float* pVertex = NULL;

    if (m_bVertexWeighting && m_pOrigCoords)
        pVertex = m_pOrigCoords + nVertexIndex * 3;

    return pVertex;
}


///////////////////////////////////////////////////////////////////////  
//  CBranchGeometry::ChangeTexCoord definition

void CBranchGeometry::ChangeTexCoord(unsigned char chChangedMapIndex, float* pNewTexCoords)
{
    st_assert(m_bRetainTexCoords);
    st_assert(m_pOrigTexCoords);
    st_assert(m_pTexIndices);

    if (chChangedMapIndex == m_pTexIndices[m_nVertexCounter])
    {
        float afTexCoords[2] = 
        {
#ifdef SPEEDTREE_DATA_FORMAT_OPENGL
            VecInterpolate(pNewTexCoords[2], pNewTexCoords[0], m_pOrigTexCoords[m_nVertexCounter * 2]),
            VecInterpolate(pNewTexCoords[5], pNewTexCoords[1], m_pOrigTexCoords[m_nVertexCounter * 2 + 1])
#else
            VecInterpolate(pNewTexCoords[2], pNewTexCoords[0], m_pOrigTexCoords[m_nVertexCounter * 2]),
            -VecInterpolate(pNewTexCoords[5], pNewTexCoords[1], m_pOrigTexCoords[m_nVertexCounter * 2 + 1])
#endif
        };

        AddVertexTexCoord(afTexCoords);
    }
}

