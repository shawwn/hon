///////////////////////////////////////////////////////////////////////  
//         Name: Branch.cpp
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
#include "TreeEngine.h"
#include <fstream>
#include "FileAccess.h"
#include "LibRandom_Source/IdvRandom.h"
#include "BillboardLeaf.h"
#include "SpeedTreeRT.h"
#include "IndexedGeometry.h"
#include "LightingEngine.h"
#include "FrondEngine.h"
#include <algorithm>
#include "Random.h"

// branch-related constants
const CVec3 g_cOut(1.0f, 0.0f, 0.0f);
const CVec3 g_cDown(0.0f, 0.0f, -1.0f);
const CVec3 g_cUp(0.0f, 0.0f, 1.0f);
const float c_fInverseOf90 = 1.0f / 90.0f;
const float c_fParentRadiusFactor = 0.85f;


// static variables
vector<int> CIdvBranch::m_vBlossoms;
vector<int> CIdvBranch::m_vNonBlossoms;

vector<const SIdvBranchInfo*> CIdvBranch::m_vBranchInfo;
bool CIdvBranch::m_bComputeLeaves = true;
SIdvLeafInfo* CIdvBranch::m_pLeafInfo = NULL;
vector<CBillboardLeaf*> CIdvBranch::m_vLocalLeaves;

CLightingEngine* CIdvBranch::m_pLightingEngine = NULL;
CIdvRandom CIdvBranch::m_cRandom;
int CIdvBranch::m_nWeightLevel = 1;
int CIdvBranch::m_nRoundRobinWindLevel = 0;
CFrondEngine* CIdvBranch::m_pFrondEngine = NULL;

bool CIdvBranch::m_bPropagateFlexibility = false;


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::CIdvBranch definition

CIdvBranch::CIdvBranch(CIdvBranch* pParent) :
    m_pParent(pParent),
    m_fPercentOfParent(0.0f),
    m_pVertices(NULL),
    m_nVertexCount(-1),
    m_usCrossSectionSegments(0),
    m_nStartingOffset(-1),
    m_fVolume(0.0f),
    m_fFuzzyVolume(0.0f)
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::~CIdvBranch definition

CIdvBranch::~CIdvBranch( )
{
    delete[] m_pVertices;
    m_pVertices = NULL;
    for (unsigned int i = 0; i < m_vChildren.size( ); ++i)
    {
        delete m_vChildren[i].m_pBranch;
        m_vChildren[i].m_pBranch = NULL;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::Compute definition

void CIdvBranch::Compute(unsigned int nSeed,
                         float fSize,
                         int nLevel,
                         const CVec3& cBasePos,
                         float fPercentOfParent,
                         const CRotTransform& cBaseTransform, 
                         const CVec3& cZAxis,
                         CIndexedGeometry* pBranchGeometry,
                         vector<CBillboardLeaf*>& vLeaves,
                         float fWindWeight,
                         int nWindGroup,
                         float fParentRadius)
{
    // find out if fronds instead of branches will be generated from this level
    bool bFrond = m_pFrondEngine->Enabled( ) && nLevel >= m_pFrondEngine->GetLevel( );

    // determine which wind group will be assigned to this branch (uses round-robin approach).
    // the wind group will be adjusted to fit into the specified wind matrix start and span
    // values after Compute()
    //static int s_nWindGroup = nWindGroup;
    if (nLevel == m_nWeightLevel)
        nWindGroup = m_nRoundRobinWindLevel++;

    // since level 0 only occurs once per tree, we use this condition to initialize
    // the blossom vectors used in creating the leaves
    if (nLevel == 0)
        BuildBlossomVectors( );

    // setup up the texture tiling parameters
    const SIdvBranchInfo& sInfo = *m_vBranchInfo[nLevel];
    if (!bFrond)
        m_nStartingOffset = pBranchGeometry->GetVertexCounter( ); // remember where this branch's geometry is stored in the branch geometry class
    m_fPercentOfParent = fPercentOfParent;

    // compute the flare entries
    if (!bFrond)
        ComputeFlareEntries(sInfo);

    // this branch was created from a parent - fPercentOfParent is the location
    // (from 0.0 to 1.0) along the length of the parent where this branch was created.
    // the attributes that govern this branch start by looking at the parent's
    // attributes at the point of the child's creation.
    float fLength = sInfo.GetLength( )->Evaluate(fPercentOfParent) * fSize;
    float fSegmentLength = fLength / sInfo.m_nSegments;
    float fAzimuth = m_cRandom.GetUniform(-180.0f, 180.0f);
    float fStartAngle = sInfo.GetStartAngle( )->Evaluate(fPercentOfParent);
    float fGravity = sInfo.GetGravity( )->Evaluate(fPercentOfParent);
    float fRadius = sInfo.GetRadius( )->Evaluate(fPercentOfParent) * fSize;
    float fFlexibility = sInfo.GetFlexibility( )->Evaluate(fPercentOfParent);
    m_usCrossSectionSegments = static_cast<unsigned short>(sInfo.m_nCrossSectionSegments);

    // set tex coord tiling
    float fTileOffset = sInfo.m_bRandomTCoordOffset ? ((fAzimuth + 180.0f / 360.0f) + fPercentOfParent) : 0.0f;
    const CVec3 cTile(sInfo.m_bSTileAbsolute ? sInfo.m_fSTile : sInfo.m_fSTile * fRadius * c_fTwoPi,
                      sInfo.m_bTTileAbsolute ? sInfo.m_fTTile : sInfo.m_fTTile * (fLength / fSize),
                      (nSeed + m_vChildren.size( )) % 2 == 0 ? sInfo.m_fTwist : -sInfo.m_fTwist);

    // make sure radius of this child branch does not exceed radius at parent
    float fFirstRadius = fRadius * sInfo.GetRadiusScale( )->Evaluate(0.0f);
    if (fParentRadius > 0.0f &&
        fFirstRadius > fParentRadius * c_fParentRadiusFactor)
        fRadius = fFirstRadius = fParentRadius * c_fParentRadiusFactor;

    // setup geometry information, passing indices into pBranchGeometry
    m_nVertexCount = sInfo.m_nSegments + 1;
    if (m_nVertexCount >= 2)
    {
        m_pVertices = new SIdvBranchVertex[m_nVertexCount];

        if (!bFrond)
        {
            unsigned short nIndexCount = static_cast<unsigned short>((m_nVertexCount - 1) * (2 + 2 * (sInfo.m_nCrossSectionSegments + 1)));
            unsigned short* pBranchStrip = new unsigned short[nIndexCount];
            unsigned short nStart = pBranchGeometry->GetVertexCounter( );

            int nVertexIndex = 0, nIndex2 = 0;
            for (int i = 0; i < m_nVertexCount - 1; ++i)
            {
                int nLoopAround = nIndex2;
                for (int j = 0; j < sInfo.m_nCrossSectionSegments + 1; ++j)
                {
                    pBranchStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + (sInfo.m_nCrossSectionSegments + 1) + nIndex2);
                    pBranchStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + nIndex2);
                    nIndex2++;
                }
                // repeat this vertex so we can combine the multiple segments of one branch
                pBranchStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + (sInfo.m_nCrossSectionSegments + 1) + nLoopAround);
                // repeat it again to counteract the clockwise vertex ordering that would result otherwise
                pBranchStrip[nVertexIndex++] = static_cast<unsigned short>(nStart + (sInfo.m_nCrossSectionSegments + 1) + nLoopAround);
            }
            pBranchGeometry->AddStrip(0, pBranchStrip, nIndexCount);
            pBranchGeometry->AdvanceStripCounter( );
        }

        // compute the first vertex of the branch - it's different than the
        // subsequent vertices.
        SIdvBranchVertex* pFirstVertex = m_pVertices;
        pFirstVertex->m_cPos = cBasePos;
        float fAzimuthDisturbance = sInfo.GetDisturbance( )->ScaledVariance(0.0f);
        float fPitchDisturbance = sInfo.GetDisturbance( )->ScaledVariance(0.0f);
        pFirstVertex->m_cTrans = cBaseTransform;
        CVec3 cRotAxis = cBaseTransform * cZAxis;
        pFirstVertex->m_cTrans.RotateAxis(fAzimuth, cRotAxis);
        pFirstVertex->m_cTrans.RotateYZ(fStartAngle + fPitchDisturbance, fAzimuthDisturbance);
        pFirstVertex->m_cDirection = g_cOut * pFirstVertex->m_cTrans;
        pFirstVertex->m_fRadius = fRadius * sInfo.GetRadiusScale( )->Evaluate(0.0f);
        float fVertexFlexibility = fFlexibility * (sInfo.GetFlexibilityScale( )->Evaluate(0.0f));

        // adjust first vertex for gravity effect
        float fGravityAngle(VecRad2Deg(pFirstVertex->m_cDirection.AngleBetween(g_cDown)));
        float fFacingFactor(1.0f - c_fInverseOf90 * (fabsf(90.0f - fGravityAngle)));
        CVec3 cAdjustmentAxis(pFirstVertex->m_cDirection * g_cDown);
        cAdjustmentAxis.Normalize( );
        float fGravityProfile = -1.0f * ((sInfo.GetAngleProfile( )->Evaluate(0.0f) - 0.5f) * 2.0f); 
        CRotTransform cTransformAdjustment;
        cTransformAdjustment.RotateAxisFromIdentity(fGravityProfile * fGravity * fGravityAngle * fFacingFactor, cAdjustmentAxis);
        pFirstVertex->m_cTrans = pFirstVertex->m_cTrans * cTransformAdjustment;
        pFirstVertex->m_cDirection = g_cOut * pFirstVertex->m_cTrans;

        // build the cross section associated with this first vertex
        float fCrossSectionWeight = fWindWeight;
        if (nLevel <= m_nWeightLevel)
            fCrossSectionWeight = pFirstVertex->m_fWindWeight = ComputeVertexWeight(nLevel, 0.0f, fVertexFlexibility);
        else if (m_bPropagateFlexibility)
            fCrossSectionWeight = pFirstVertex->m_fWindWeight = ComputeHighLevelVertexWeight(fWindWeight, fVertexFlexibility);

        if (!bFrond)
        {
            // regular branch geometry
            BuildCrossSection(pFirstVertex, 0.0f, m_usCrossSectionSegments, pBranchGeometry, cTile, fCrossSectionWeight, nWindGroup, fTileOffset);
        }
        else
        {
            // start a new frond and add a vertex
            m_pFrondEngine->StartGuide( );
            m_pFrondEngine->AddGuideVertex(pFirstVertex->m_cPos, pFirstVertex->m_cTrans, fCrossSectionWeight, nWindGroup);
        }

        float fRunningLength = 0.0f;
        m_pVertices[0].m_fRunningLength = 0.0f;
        // calculate the rest of the branch's vertices
        SIdvBranchVertex* pLastVertex = pFirstVertex;
        for (int i = 1; i < m_nVertexCount; ++i)
        {
            SIdvBranchVertex* pVertex = m_pVertices + i;

            // calculate how far along the branch's length we are
            float fLinearProgress = i / static_cast<float>(m_nVertexCount - 1);
            float fProgress = powf(fLinearProgress, sInfo.m_fSegmentPackingExponent);
            fSegmentLength = (fProgress * fLength) - fRunningLength;

            // evaluate radius and flexibility
            pVertex->m_fRadius = fRadius * sInfo.GetRadiusScale( )->Evaluate(fProgress);
            float fLocalVertexFlexibility = fFlexibility * (sInfo.GetFlexibilityScale( )->Evaluate(fProgress));

            // adjust growth direction, taking disturbance into account
            pVertex->m_cTrans = pLastVertex->m_cTrans;
            pVertex->m_cDirection = g_cOut * pVertex->m_cTrans;

            // adjust for gravity effect
            float fLocalGravityAngle = VecRad2Deg(pVertex->m_cDirection.AngleBetween(g_cDown));
            float fLocalFacingFactor = (1.0f - c_fInverseOf90 * (fabsf(90.0f - fLocalGravityAngle)));
            CVec3 cThisAdjustmentAxis(pVertex->m_cDirection * g_cDown);
            cThisAdjustmentAxis.Normalize( );
            float fLocalGravityProfile = -1.0f * ((sInfo.GetAngleProfile( )->Evaluate(fProgress) - 0.5f) * 2.0f);
            cTransformAdjustment.RotateAxisFromIdentity(fLocalGravityProfile * fGravity * fLocalGravityAngle * fLocalFacingFactor, cThisAdjustmentAxis);
            pVertex->m_cTrans = pVertex->m_cTrans * cTransformAdjustment;

            // keep the branch from being perfectly straight
            float fLocalAzimuthDisturbance = sInfo.GetDisturbance( )->ScaledVariance(fProgress);
            float fLocalPitchDisturbance = sInfo.GetDisturbance( )->ScaledVariance(fProgress);
            pVertex->m_cTrans.RotateYZ(fLocalPitchDisturbance, fLocalAzimuthDisturbance);
            pVertex->m_cDirection = g_cOut * pVertex->m_cTrans;

            // apply final vertex position
            pVertex->m_cPos = pLastVertex ? pLastVertex->m_cPos + (pLastVertex->m_cDirection * fSegmentLength) : cBasePos;

            // pass geometry data into the pBranchGeometry class via BuildCrossSection()
            float fLocalCrossSectionWeight = fWindWeight;
            if (nLevel <= m_nWeightLevel)
                fLocalCrossSectionWeight = pVertex->m_fWindWeight = ComputeVertexWeight(nLevel, fProgress, fLocalVertexFlexibility);
            else if (m_bPropagateFlexibility)
                fLocalCrossSectionWeight = pVertex->m_fWindWeight = ComputeHighLevelVertexWeight(fWindWeight, fLocalVertexFlexibility);

            if (!bFrond)
            {
                // regular branch geometry
                BuildCrossSection(pVertex, fProgress, m_usCrossSectionSegments, pBranchGeometry, cTile, fLocalCrossSectionWeight, nWindGroup, fTileOffset);
            }
            else
            {
                // frond geometry
                m_pFrondEngine->AddGuideVertex(pVertex->m_cPos, pVertex->m_cTrans, fLocalCrossSectionWeight, nWindGroup);
            }

            pLastVertex = pVertex;
            fRunningLength += fSegmentLength;
            m_pVertices[i].m_fRunningLength = fRunningLength;
        }

        if (bFrond)
        {
            m_pFrondEngine->EndGuide(fFirstRadius);
        }
        else
        {
            pBranchGeometry->SetVertexCounter(static_cast<unsigned short>(m_nStartingOffset));
            ComputeBranchNormals(pBranchGeometry, m_usCrossSectionSegments);
        }

        // this branch's geometry is complete, now determine the number of children - 
        // children can be additional branches or "buds" that contain no actual geometry
        // but are used as placeholders for leaf clusters
        int nNumChildBranches = static_cast<int>((sInfo.m_fFrequency / fSize) * fLength);

        ++nLevel; // level 0 = trunk, level 1 = trunk's immediate children, etc.

        // bChildrenAreBuds tells us if buds will be created from this branch
        bool bChildrenAreBuds = nLevel >= static_cast<int>(m_vBranchInfo.size( ) - 1);
        if (!bChildrenAreBuds || m_bComputeLeaves)
        {
            for (int i = 0; i < nNumChildBranches; ++i)
            {
                SIdvBranch sBranch;

                // the random number generator is reseeded here in order to make EstimateCompute give
                // exact predictions of the size of the tree.  it is only reseeded for normal branches
                // and not buds because it is a relatively expensive operation
                if (!bChildrenAreBuds)
                {
                    nSeed += 3;
                    m_cRandom.Reseed(nSeed);
                }

                // make sure that at least one branch (the first one) grows near the end of the parent.
                // trust us, this makes for better looking trees.
                float fPercent = 0.0f;
                if (!bChildrenAreBuds && i == 0)
                    fPercent = m_cRandom.GetUniform(VecInterpolate(sInfo.m_fFirstBranch, sInfo.m_fLastBranch, 0.85f), VecInterpolate(sInfo.m_fFirstBranch, sInfo.m_fLastBranch, 0.95f));
                else
                    fPercent = m_cRandom.GetUniform(sInfo.m_fFirstBranch, sInfo.m_fLastBranch);

                // find out which of the parent's vertexes the child is created between
                FillBranch(sBranch, fPercent * fLength);

                // what's the vertex weight where the child will be spawned?
                float fWeightAtChild = fWindWeight;
                if (nLevel - 1 == m_nWeightLevel || m_bPropagateFlexibility)
                    fWeightAtChild = VecInterpolate(m_pVertices[sBranch.m_nPreVertexIndex].m_fWindWeight, m_pVertices[sBranch.m_nPreVertexIndex + 1].m_fWindWeight, sBranch.m_fPercent);

                // another reseeding operation (for predictability in EstimateCompute)
                if (!bChildrenAreBuds)
                {
                    m_cRandom.Reseed(nSeed);
                    (void) m_cRandom.GetUniform(0.0f, 100.0f);
                }

                // allocate child CIdvBranch 
                CVec3 cPos = VecInterpolate(m_pVertices[sBranch.m_nPreVertexIndex].m_cPos, m_pVertices[sBranch.m_nPreVertexIndex + 1].m_cPos, sBranch.m_fPercent);

				float fProfilePercent = 1.0f;
				if (sInfo.m_fLastBranch != sInfo.m_fFirstBranch)
					fProfilePercent = (fPercent - sInfo.m_fFirstBranch) / (sInfo.m_fLastBranch - sInfo.m_fFirstBranch);
                if (bChildrenAreBuds)
                {
                    // create the bud and possibly the leaf
                    ComputeBud(this, fSize, nLevel, cPos, fProfilePercent, m_pVertices[sBranch.m_nPreVertexIndex].m_cTrans, m_pVertices[sBranch.m_nPreVertexIndex].m_cDirection, vLeaves, fWeightAtChild, nWindGroup);
                }
                else
                {
                    // make a new child branch and recusively call CIdvBranch::Compute()
                    float fRadiusOfParentAtBranch = VecInterpolate(m_pVertices[sBranch.m_nPreVertexIndex].m_fRadius, m_pVertices[sBranch.m_nPreVertexIndex + 1].m_fRadius, sBranch.m_fPercent);
                    sBranch.m_pBranch = new CIdvBranch(this);
                    sBranch.m_pBranch->Compute(nSeed, fSize, nLevel, cPos, fProfilePercent, m_pVertices[sBranch.m_nPreVertexIndex].m_cTrans, m_pVertices[sBranch.m_nPreVertexIndex].m_cDirection, pBranchGeometry, vLeaves, fWeightAtChild, nWindGroup, fRadiusOfParentAtBranch);

                    if (!m_pFrondEngine->Enabled( ) || (nLevel < m_pFrondEngine->GetLevel( )))
                    {
                        m_vChildren.push_back(sBranch);
                    }
                    else
                    {
                        delete sBranch.m_pBranch;
                        sBranch.m_pBranch = NULL;
                    }
                }
            }
        }

        if (bChildrenAreBuds)
            m_vLocalLeaves.clear( );
        ComputeVolume( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeBud definition

void CIdvBranch::ComputeBud(const CIdvBranch* pParent,
                            float fSize,
                            int nLevel,
                            const CVec3& cBasePos,
                            float fPercentOfParent,
                            const CRotTransform& cBaseTransform, 
                            const CVec3& cZAxis,
                            vector<CBillboardLeaf*>& vLeaves,
                            float fWindWeight, 
                            int nWindGroup) const
{
    const SIdvBranchInfo& sInfo = *m_vBranchInfo[nLevel];
    const float c_fStartAngle = 60.0f;

    // always need 2 vertices for a bud - one on the parent branch and another for the leaf location
    const int c_nBudVertexCount = 2;
    SIdvBranchVertex acVertices[c_nBudVertexCount];

    float fLength = sInfo.GetLength( )->Evaluate(fPercentOfParent) * fSize;
    float fSegmentLength = fLength / sInfo.m_nSegments;
    float fAzimuth = m_cRandom.GetUniform(-180.0f, 180.0f);
    CVec3 cRotAxis = cBaseTransform * cZAxis;

    // compute position and orientation of first vertex
    SIdvBranchVertex *pFirstVertex = acVertices + 0;
    pFirstVertex->m_cPos = cBasePos;
    pFirstVertex->m_cTrans = cBaseTransform;
    pFirstVertex->m_cTrans.RotateAxis(fAzimuth, cRotAxis);
    pFirstVertex->m_cTrans.RotateY(c_fStartAngle);
    pFirstVertex->m_cDirection = g_cOut * pFirstVertex->m_cTrans;

    // compute leaf vertex
    SIdvBranchVertex *pVertex = acVertices + 1;
    pVertex->m_cPos = pFirstVertex->m_cPos + (pFirstVertex->m_cDirection * fSegmentLength);

    // compute geometric normal of leaf
    CVec3 cNormal = acVertices[1].m_cPos - acVertices[0].m_cPos;
    cNormal.Normalize( );

    // compute the general direction the parent branch was growing
    CVec3 cParentDir;
    if (pParent == NULL || pParent->m_nVertexCount == 0)
        cParentDir = cNormal;
    else
    {
        CVec3 cBase = pParent->m_pVertices[0].m_cPos;

        // back up to the dimming level if necessary
        if (m_pLeafInfo->m_nBlossomLevel != 0)
        {
            int nLevels = 0;
            const CIdvBranch* pBranch = pParent;

            while (pBranch && nLevels < m_pLeafInfo->m_nBlossomLevel)
            {
                pBranch = pBranch->GetParent( );
                nLevels++;
            }
            if (pBranch)
                cBase = pBranch->m_pVertices[0].m_cPos;
        }

        cParentDir = acVertices[1].m_cPos - cBase;
    }
    cParentDir.Normalize( );

    // attempt to grow leaf based on this bud's parameters
    MakeLeaf(acVertices[c_nBudVertexCount - 1].m_cPos,
             fPercentOfParent,
             this,
             cNormal,
             cParentDir,
             fWindWeight,
             nWindGroup,
             vLeaves);
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeLod definition
//
//  Each of the discrete branch LOD's share the same vertex table.  A lower
//  LOD of the branch structure simply drops some of the lower-volume branches.
//  ComputeLod() assigns the new indexes for the requested LOD.

void CIdvBranch::ComputeLod(int nDiscreteLodLevel, CIndexedGeometry* pGeometry) const
{
    // new cross section count
    if (m_usCrossSectionSegments >= 2)
    {
        int nTargetCrossSection = m_usCrossSectionSegments;
        float fCrossSectionStep = !nTargetCrossSection ? 1.0f : float(m_usCrossSectionSegments) / nTargetCrossSection;

        // new vertex count
        int nTargetVertexCount = m_nVertexCount;

        int nIndexCount = (nTargetVertexCount - 1) * (2 + 2 * (nTargetCrossSection + 1));
        unsigned short* pStrip = new unsigned short[nIndexCount];

        pGeometry->AddStrip(static_cast<unsigned short>(nDiscreteLodLevel), pStrip, static_cast<unsigned short>(nIndexCount));

        int nInterleavedIndex = 0;
        float fCrossSectionIndex = 0.0f;
        for (int i = 0; i < nTargetVertexCount - 1; ++i)
        {
            int nLoopAround = int(fCrossSectionIndex);

            // first vertex in cross section
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + (m_usCrossSectionSegments + 1) + int(fCrossSectionIndex));
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + int(fCrossSectionIndex));
            fCrossSectionIndex += fCrossSectionStep;

            // we use crosssection + 1 because we must loop around and include the first point again (tex coords)
            for (int j = 1; j < nTargetCrossSection; ++j)
            {
                pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + (m_usCrossSectionSegments + 1) + int(fCrossSectionIndex));
                pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + int(fCrossSectionIndex));
                fCrossSectionIndex += fCrossSectionStep;
            }

            // last vertex in cross section
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + (m_usCrossSectionSegments + 1) + nLoopAround + m_usCrossSectionSegments);
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + nLoopAround + m_usCrossSectionSegments);

            // get ready for next cross section
            fCrossSectionIndex = static_cast<float>(nLoopAround + (m_usCrossSectionSegments + 1));

            // repeat this vertex so we can combine the multiple segments of one branch
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + (m_usCrossSectionSegments + 1) + nLoopAround);
            // repeat it again to counteract the clockwise vertex ordering that would result otherwise
            pStrip[nInterleavedIndex++] = static_cast<unsigned short>(m_nStartingOffset + (m_usCrossSectionSegments + 1) + nLoopAround);
        }
        pGeometry->AdvanceStripCounter( );
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::BuildBranchVector definition
//
//  This is a convenience function that builds a linear vector from the
//  branch tree structure.

void CIdvBranch::BuildBranchVector(vector<CIdvBranch*>& vAllBranches)
{
    vAllBranches.push_back(this);

    for (unsigned int i = 0; i < m_vChildren.size( ); ++i)
        m_vChildren[i].m_pBranch->BuildBranchVector(vAllBranches);
}


///////////////////////////////////////////////////////////////////////  
//  class CBranchVolumeSorter definition
//
//  Function object used to sort the branches by volume.

class CBranchVolumeSorter
{
public:
    bool operator()(const CIdvBranch* pLeft, const CIdvBranch* pRight)
    {
        return (pLeft->GetFuzzyVolume( ) > pRight->GetFuzzyVolume( ));
    }
};


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::SortBranchVector definition

void CIdvBranch::SortBranchVector(vector<CIdvBranch*>& vAllBranches)
{
    sort(vAllBranches.begin( ), vAllBranches.end( ), CBranchVolumeSorter( ));
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::Parse definition

void CIdvBranch::Parse(CTreeFileAccess& cFile)
{
    ClearBranchInfo( );

    int nSize = cFile.ParseToken( );
    for (int i = 0; i < nSize; ++i)
    {
        SIdvBranchInfo sInfo;
        int nToken = cFile.ParseToken( );
        if (nToken != File_BeginBranchLevel)
            throw(IdvFileError("malformed branch data"));

        nToken = cFile.ParseToken( );
        do
        {
            switch (nToken)
            {
                case File_Branch_Disturbance:
                    sInfo.SetDisturbance(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_EndAngle:
                    sInfo.SetGravity(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_Flexibility:
                    sInfo.SetFlexibility(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_FlexibilityScale:
                    sInfo.SetFlexibilityScale(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_Length:
                    sInfo.SetLength(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_Radius:
                    sInfo.SetRadius(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_RadiusScale:
                    sInfo.SetRadiusScale(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_StartAngle:
                    sInfo.SetStartAngle(cFile.ParseBranchParameter( ));
                    break;
                case File_Branch_CrossSectionSegments:
                    sInfo.m_nCrossSectionSegments = cFile.ParseInt( );
                    break;
                case File_Branch_Segments:
                    sInfo.m_nSegments = cFile.ParseInt( );
                    break;
                case File_Branch_FirstBranch:
                    sInfo.m_fFirstBranch = cFile.ParseFloat( );
                    break;
                case File_Branch_LastBranch:
                    sInfo.m_fLastBranch = cFile.ParseFloat( );
                    break;
                case File_Branch_Frequency:
                    sInfo.m_fFrequency = cFile.ParseFloat( );
                    break;
                case File_Branch_STile:
                    sInfo.m_fSTile = cFile.ParseFloat( );
                    break;
                case File_Branch_TTile:
                    sInfo.m_fTTile = cFile.ParseFloat( );
                    break;
                case File_Branch_STileAbsolute:
                    sInfo.m_bSTileAbsolute = cFile.ParseBool( );
                    break;
                case File_Branch_TTileAbsolute:
                    sInfo.m_bTTileAbsolute = cFile.ParseBool( );
                    break;
                default:
                    throw(IdvFileError("malformed general branch information"));
            }
            nToken = cFile.ParseToken( );
        } while (nToken != File_EndBranchLevel);
    }

    int nToken = cFile.ParseToken( );
    if (nToken != File_EndBranchInfo)
        throw(IdvFileError("malformed branch data"));
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::BuildBlossomVectors definition
//
//  Each leaf texture's index is added either to m_vBlossoms or
//  m_vNonBlossoms.  Each texture index also has a mirror that is the
//  same texture with the s coordinates mirrored.

void CIdvBranch::BuildBlossomVectors(void)
{
    m_vBlossoms.clear( );
    m_vNonBlossoms.clear( );

    for (unsigned int i = 0; i < m_pLeafInfo->m_vLeafTextures.size( ); ++i)
        if (m_pLeafInfo->m_vLeafTextures[i].m_bBlossom)
        {
            m_vBlossoms.push_back(i * 2);
            m_vBlossoms.push_back(i * 2 + 1);
        }
        else
        {
            m_vNonBlossoms.push_back(i * 2);
            m_vNonBlossoms.push_back(i * 2 + 1);
        }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::IsBlossom definition
//
//  Uses the parameters specified in SpeedTreeCAD to determine if position
//  fPercentOfParent on branch pParent will result in a blossom.

bool CIdvBranch::IsBlossom(const CIdvBranch* pParent, float fPercentOfParent) const
{
    bool bIsBlossom = false;

    float fTestPercent = fPercentOfParent;
    if (!m_pLeafInfo->m_nBlossomLevel)
    {
        // blossoms are created at the branch level m_pLeafInfo->m_nBlossomLevel - pBranch
        // is advanced to the correct level

        int nLevels = 1; // "this" is already pointing to the parent of the bud so start one level up

        const CIdvBranch* pBranch = pParent;
        while (pBranch &&
               pBranch->GetParent( ) &&
               nLevels < m_pLeafInfo->m_nBlossomLevel)
        {
            pBranch = pBranch->GetParent( );
            nLevels++;
        }

        if (pBranch)
            fTestPercent = pBranch->GetPercentOfParent( );
    }

    // random numbers are used because not every leaf should be a blossom - the weighting
    // of which is specified by the user in SpeedTreeCAD
    if (fTestPercent > m_pLeafInfo->m_fBlossomDistance &&
        m_cRandom.GetUniform(0.0f, 1.0f) <= m_pLeafInfo->m_fBlossomWeighting)
        bIsBlossom = true;

    return bIsBlossom;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::MakeLeaf definition

void CIdvBranch::MakeLeaf(const CVec3& cPos,
                          float fPercent,
                          const CIdvBranch* pParent,
                          const CVec3& cNormal,
                          const CVec3& cParentDir,
                          float fWindWeight,
                          int nWindGroup,
                          vector<CBillboardLeaf*>& vLeaves) const
{
    // are there any leaf textures to use?
    if (!m_pLeafInfo->m_vLeafTextures.empty( ))
    {
        int nTextureIndex = 0;

        // determine if this leaf will be a blossom
        bool bIsBlossom = m_vBlossoms.empty( ) ? false : IsBlossom(pParent, fPercent);
        if (bIsBlossom)
        {
            // randomly pick which blossom it will be based on the m_vBlossoms vector
            unsigned int nIndex = 0;
            if (m_vBlossoms.size( ) > 1)
                nIndex = static_cast<unsigned int>(m_cRandom.GetUniform(0.0, 100000.0)) % m_vBlossoms.size( );
            nTextureIndex = m_vBlossoms[nIndex];
        }
        else
        {
            // randomly pick which non-blossom texture will be used
            unsigned int nIndex = 0;
            if (m_vNonBlossoms.size( ) > 1)
            {
                float fRandom = m_cRandom.GetUniform(0.0, 1000000.0);
                nIndex = static_cast<unsigned int>(fRandom) % m_vNonBlossoms.size( );
                nTextureIndex = m_vNonBlossoms[nIndex];
            }
            else if (m_vNonBlossoms.empty( ))
                return;
            else
                nTextureIndex = m_vNonBlossoms[0];
        }

        // test to see if the leaf will be too crowded
        bool bValid = true;
        switch (m_pLeafInfo->m_eCollisionType)
        {
        case SIdvLeafInfo::NONE:    // no leaf will ever be crowded
            break;
        case SIdvLeafInfo::BRANCH:  // test only against leaves that are spawned from a common branch
            bValid = RoomForLeaf(cPos, nTextureIndex / 2, m_vLocalLeaves);
            break;
        case SIdvLeafInfo::TREE:    // test against all leaves in the tree
            bValid = RoomForLeaf(cPos, nTextureIndex / 2, vLeaves);
            break;
        default:
            st_assert("invalid case in switch in CIdvBranch::MakeLeaf()");
            break;
        }

        if (bValid)
        {
            float fColorScale = 1.0f;

            // if dimming is enabled, modify the color scalar to darken it an appropriate amount.
            // m_nBlossomLevel is also used as the dimming level
            float fDimPercent = fPercent;
            if (m_pLeafInfo->m_bDimming)
            {
                if (m_pLeafInfo->m_nBlossomLevel != 0)
                {
                    int nLevels = 0;
                    const CIdvBranch* pBranch = pParent;

                    while (pBranch && nLevels < m_pLeafInfo->m_nBlossomLevel)
                    {
                        fDimPercent *= pBranch->GetPercentOfParent( );
                        pBranch = pBranch->GetParent( );
                        nLevels++;
                    }
                }
                
                fColorScale = VecInterpolate(fDimPercent, 1.0f, 1.0f - m_pLeafInfo->m_fDimmingScalar);
            }

            // actually make the leaf now
            //lint -esym(429,pLeaf)  { pLeaf is not freed or returned from MakeLeaf(), but it is stored in vLeaves for later deletion }
            CBillboardLeaf* pLeaf = new CBillboardLeaf(cPos, short(255.0f * fColorScale), 
                static_cast<int>(m_cRandom.GetUniform(0.0, 10000.0)) % m_pLeafInfo->m_nNumRockingGroups, fWindWeight, nWindGroup);
            vLeaves.push_back(pLeaf);
            if (m_pLeafInfo->m_eCollisionType == SIdvLeafInfo::BRANCH)
                m_vLocalLeaves.push_back(pLeaf);

            const SIdvLeafTexture& sTexture = m_pLeafInfo->m_vLeafTextures[nTextureIndex / 2];
            pLeaf->SetTextureIndex(nTextureIndex);

            // let the color variance perturb the leaf normal (for dynamic lighting)
            CVec3 cAdjustedNormal;
            if (cNormal.Magnitude( ) < 0.0001)
                cAdjustedNormal = cParentDir;
            else
                cAdjustedNormal = VecInterpolate(cParentDir, cNormal, sTexture.m_fColorVariance);
            cAdjustedNormal.Normalize( );

            CVec3 cDown(0.0f, 0.0f, -1.0f);
            if (m_pLeafInfo->m_bDimming && m_pLightingEngine->GetLeafLightingMethod( ) == CSpeedTreeRT::LIGHT_DYNAMIC)
            {
                cDown.Set(0.0f, 0.0f, -(1.0f - fDimPercent) * m_pLeafInfo->m_fDimmingScalar * 2.0f);
                cAdjustedNormal = cAdjustedNormal + cDown;
                cAdjustedNormal.Normalize( );
            }
            pLeaf->SetNormal(cAdjustedNormal);
            
            // bump mapping
            CVec3 cTangent = cAdjustedNormal * cDown;
            cTangent.Normalize( );
            pLeaf->SetTangent(cTangent);

            CVec3 cBinormal = cTangent * cAdjustedNormal;
            cBinormal.Normalize( );
            pLeaf->SetBinormal(cBinormal);

            // directly adjust the color (for static lighting)
            float fOffset = m_cRandom.GetUniform(-sTexture.m_fColorVariance, sTexture.m_fColorVariance);
            CVec3 cColor(sTexture.m_cColor[0] + fOffset, sTexture.m_cColor[1] + fOffset, sTexture.m_cColor[2] + fOffset);
            pLeaf->SetColor(cColor);
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::RoomForLeaf definition
//
//  This function checks against the leaves in the vLeaves vector to see if
//  the leaf at position cPos will be too crowded.

bool CIdvBranch::RoomForLeaf(const CVec3& cPos, int nTextureIndex, const vector<CBillboardLeaf*>& vLeaves) const
{
    bool bIsRoom = true;

    if (!m_pLeafInfo->m_vLeafTextures.empty( ))
    {
        // get the larger of the width and height of the leaf
        const SIdvLeafTexture& sTexture = m_pLeafInfo->m_vLeafTextures[nTextureIndex];
        float fLargerSide = sTexture.m_cSizeUsed[0] > sTexture.m_cSizeUsed[1] ? sTexture.m_cSizeUsed[0] : sTexture.m_cSizeUsed[1];

        // adjust the size by the spacing tolerance (allows adjustment of leaf density)
        fLargerSide *= m_pLeafInfo->m_fSpacingTolerance;
        for (unsigned int i = 0; i < vLeaves.size( ) && bIsRoom; ++i)
        {
            const CVec3& cOther = vLeaves[i]->GetPosition( );
            if ( ((cPos[0] < cOther[0] + fLargerSide) && (cPos[0] > cOther[0] - fLargerSide)) &&
                 ((cPos[1] < cOther[1] + fLargerSide) && (cPos[1] > cOther[1] - fLargerSide)) &&
                 ((cPos[2] < cOther[2] + fLargerSide) && (cPos[2] > cOther[2] - fLargerSide)) )
                 bIsRoom = false;
        }
    }
    else
        bIsRoom = false;

    return bIsRoom;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::BuildCrossSection definition
//
//  This function "skins" the branch at pVertex.

void CIdvBranch::BuildCrossSection(const SIdvBranchVertex* pVertex,
                                   float fBranchProgress, 
                                   int nSegments, 
                                   CIndexedGeometry* pBranchGeometry, 
                                   const CVec3& cTile, 
                                   float fWindWeight, 
                                   int nWindGroup,
                                   float fTileOffset) const
{
    float fProgress = 0.0f;
    const float fProgressInc = 1.0f / nSegments;
    for (int i = 0; i <= nSegments; ++i)
    {
        float fAngle = c_fTwoPi * fProgress;

        // compute and add tex coords
        float afTexCoords[2];
        afTexCoords[0] = fProgress * cTile[0];
        // twist the map
        afTexCoords[0] += cTile[2] * fBranchProgress;
        afTexCoords[1] = (fBranchProgress + fTileOffset) * cTile[1];
        pBranchGeometry->AddVertexTexCoord0(afTexCoords);

        // compute and add normal
        float fCosine = cosf(fAngle);
        float fSine = sinf(fAngle);
        CVec3 cNormal;
        cNormal[0] = fCosine * pVertex->m_cTrans.m_afData[1][0] +
                      fSine * pVertex->m_cTrans.m_afData[2][0];
        cNormal[1] = fCosine * pVertex->m_cTrans.m_afData[1][1] +
                      fSine * pVertex->m_cTrans.m_afData[2][1];
        cNormal[2] = fCosine * pVertex->m_cTrans.m_afData[1][2] +
                      fSine * pVertex->m_cTrans.m_afData[2][2];

        // bump mapping (compute tangent and binormal)
        fCosine = cosf(fAngle + c_fHalfPi);
        fSine = sinf(fAngle + c_fHalfPi);
        CVec3 cTangent;
        cTangent[0] = fCosine * pVertex->m_cTrans.m_afData[1][0] +
                      fSine * pVertex->m_cTrans.m_afData[2][0];
        cTangent[1] = fCosine * pVertex->m_cTrans.m_afData[1][1] +
                      fSine * pVertex->m_cTrans.m_afData[2][1];
        cTangent[2] = fCosine * pVertex->m_cTrans.m_afData[1][2] +
                      fSine * pVertex->m_cTrans.m_afData[2][2];
        pBranchGeometry->AddVertexTangent(cTangent);
        pBranchGeometry->AddVertexBinormal(cTangent * cNormal);

        // adjust for flares
        float fFlareAdjust = 1.0f, fFlareAdditions = 0.0f;

        for (unsigned int j = 0; j < m_vFlares.size( ); ++j)
            fFlareAdditions += m_vFlares[j].Distance(fAngle, fBranchProgress);

        fFlareAdjust += fFlareAdditions;

        // position (uses previous normal calculation)
        float afOriginalPos[3];
        afOriginalPos[0] = pVertex->m_cPos.m_afData[0] + (cNormal[0] * pVertex->m_fRadius);
        afOriginalPos[1] = pVertex->m_cPos.m_afData[1] + (cNormal[1] * pVertex->m_fRadius);
        afOriginalPos[2] = pVertex->m_cPos.m_afData[2] + (cNormal[2] * pVertex->m_fRadius);

        if (fFlareAdjust != 1.0f)
        {
            float afPos[3];
            afPos[0] = pVertex->m_cPos.m_afData[0] + (cNormal[0] * pVertex->m_fRadius * fFlareAdjust);
            afPos[1] = pVertex->m_cPos.m_afData[1] + (cNormal[1] * pVertex->m_fRadius * fFlareAdjust);
            afPos[2] = pVertex->m_cPos.m_afData[2] + (cNormal[2] * pVertex->m_fRadius * fFlareAdjust);

            CVec3 cNewNormal(afPos[0] - afOriginalPos[0], afPos[1] - afOriginalPos[1], afPos[2] - afOriginalPos[2]);
            cNewNormal.Normalize( );
            cNormal = cNewNormal;

            memcpy(afOriginalPos, afPos, 3 * sizeof(float));
        }
        pBranchGeometry->AddVertexCoord(afOriginalPos);

        // set wind information
        if (pBranchGeometry->IsVertexWeightingEnabled( ))
            pBranchGeometry->AddVertexWind(fWindWeight, static_cast<unsigned char>(nWindGroup));

        pBranchGeometry->AdvanceVertexCounter( );
        fProgress += fProgressInc;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeVertexWeight definition

float CIdvBranch::ComputeVertexWeight(int nLevel, float fProgress, float fFlexibility)
{
    float fWeight = 1.0f;

    if (nLevel == m_nWeightLevel)
        fWeight -= fProgress * fFlexibility;

    return fWeight;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeHighLevelVertexWeight definition

float CIdvBranch::ComputeHighLevelVertexWeight(float fBaseWeight, float fFlexibility)
{
    float fWeight = VecInterpolate(fBaseWeight, 0.0f, fFlexibility);

    return fWeight;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeVertexWeight definition
//
//  To compute the volume of the branch, we will add the volumes
//  of each segment.  Each segment is rougly a cone with a non-zero
//  radius at the top.  The volume formula for this type of cone is:
//
//      Volume = PI * (R2^2 + R1 * R2 + R1^2) * Height / 3
//      Cross Sectional Area = Height * (R1 + R2)   

void CIdvBranch::ComputeVolume(void)
{
    if (m_pVertices && m_nVertexCount > 1)
    {
        m_fVolume = 0.0f;
        for (int i = 0; i < m_nVertexCount - 1; ++i)
        {
            SIdvBranchVertex* pVertex1 = m_pVertices + i;
            SIdvBranchVertex* pVertex2 = m_pVertices + i + 1;

            float fDistance = pVertex1->m_cPos.Distance(pVertex2->m_cPos);
            m_fVolume += fDistance * (pVertex1->m_fRadius + pVertex2->m_fRadius);
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::FillBranch definition
//
//  FillBranch() determines the vertex of the parent just before the child
//  is grown - stored as sBranch.m_nPreVertexIndex.  sBranch.m_fPercent
//  is the percentage distance from this vertex to the next where the child
//  is created.

void CIdvBranch::FillBranch(SIdvBranch& sBranch, float fChildGrowthPos) const
{
    if (m_pVertices && m_nVertexCount >= 2)
    {
        sBranch.m_nPreVertexIndex = 0;
        for (int i = 1; i < m_nVertexCount; ++i)
        {
            if (fChildGrowthPos < m_pVertices[i].m_fRunningLength)
            {
                sBranch.m_nPreVertexIndex = i - 1;
                break;
            }
        }

        sBranch.m_fPercent = (fChildGrowthPos - m_pVertices[sBranch.m_nPreVertexIndex].m_fRunningLength) /
                            (m_pVertices[sBranch.m_nPreVertexIndex + 1].m_fRunningLength - m_pVertices[sBranch.m_nPreVertexIndex].m_fRunningLength);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeFlareEntries definition
//
//  This function uses the system random numbers in an attempt to preserve
//  seed compatibility and not disturb EstimateCompute( ).

void CIdvBranch::ComputeFlareEntries(const SIdvBranchInfo& sInfo)
{
    if (sInfo.m_nNumFlares != 0)
    {
        float fInitialAngle = GetRandom(0.0f, c_fTwoPi);
        float fBalanceAngle = c_fTwoPi / (sInfo.m_nNumFlares);

        for (int i = 0; i < sInfo.m_nNumFlares; ++i)
        {
            SIdvBranchFlare sFlare;

            // andgle
            sFlare.m_fAngle = fInitialAngle + (i * GetRandom(sInfo.m_fFlareBalance * fBalanceAngle, fBalanceAngle));
            if (sFlare.m_fAngle > c_fTwoPi)
                sFlare.m_fAngle -= c_fTwoPi;

            // focus
            sFlare.m_fRadialExponent = sInfo.m_fRadialExponent;
            sFlare.m_fLengthExponent = sInfo.m_fLengthExponent;

            // angle influence
            sFlare.m_fRadialInfluence = VecDeg2Rad(GetRandom(sInfo.m_fRadialInfluence - sInfo.m_fRadialInfluenceVariance, sInfo.m_fRadialInfluence + sInfo.m_fRadialInfluenceVariance));

            // distance influence
            sFlare.m_fLengthInfluence = GetRandom(sInfo.m_fLengthDistance - sInfo.m_fLengthVariance, sInfo.m_fLengthDistance + sInfo.m_fLengthVariance);

            // distance
            sFlare.m_fDistance = GetRandom(sInfo.m_fRadialDistance - sInfo.m_fRadialVariance, sInfo.m_fRadialDistance + sInfo.m_fRadialVariance);
            
            m_vFlares.push_back(sFlare);
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CIdvBranch::ComputeBranchNormals definition

void CIdvBranch::ComputeBranchNormals(CIndexedGeometry* pBranchGeometry, unsigned short usSegments)
{
    for (int i = 0; i < m_nVertexCount; ++i)
    {
        int nBase = i * (usSegments + 1);
        for (int j = 0; j <= usSegments; ++j)
        {
            // get horizontal vector
            int nHorizPrev;
            if (j == 0)
                nHorizPrev = nBase + usSegments - 1;
            else
                nHorizPrev = nBase + j - 1;

            int nHorizNext;
            if (j == usSegments)
                nHorizNext = nBase + 1;
            else
                nHorizNext = nBase + j + 1;

            nHorizPrev += m_nStartingOffset;
            nHorizNext += m_nStartingOffset;

            const float* pHorizPrev = pBranchGeometry->GetVertexCoord(nHorizPrev);
            const float* pHorizNext = pBranchGeometry->GetVertexCoord(nHorizNext);
            CVec3 cHoriz = CVec3(pHorizNext[0] - pHorizPrev[0], pHorizNext[1] - pHorizPrev[1], pHorizNext[2] - pHorizPrev[2]);
            cHoriz.Normalize( );

            // get vertical vector
            int nVertPrev;
            if (i == 0)
                nVertPrev = nBase + j;
            else
                nVertPrev = ((i - 1) * (usSegments + 1)) + j;

            int nVertNext;
            if (i == m_nVertexCount - 1)
                nVertNext = nBase + j;
            else
                nVertNext = ((i + 1) * (usSegments + 1)) + j;

            nVertPrev += m_nStartingOffset;
            nVertNext += m_nStartingOffset;

            const float* pVertPrev = pBranchGeometry->GetVertexCoord(nVertPrev);
            const float* pVertNext = pBranchGeometry->GetVertexCoord(nVertNext);
            CVec3 cVert = CVec3(pVertNext[0] - pVertPrev[0], pVertNext[1] - pVertPrev[1], pVertNext[2] - pVertPrev[2]);
            cVert.Normalize( );

            // compute new normal
            CVec3 cNormal = cHoriz * cVert;

            if (m_pLightingEngine->GetBranchLightingMethod( ) == CSpeedTreeRT::LIGHT_STATIC)
            {
                float afColor[4];
                m_pLightingEngine->ComputeStandardStaticLighting(cNormal, pBranchGeometry->GetVertexCoord(nBase), afColor);
                pBranchGeometry->AddVertexColor(afColor);
            }
            else
                pBranchGeometry->AddVertexNormal(cNormal);

            pBranchGeometry->AdvanceVertexCounter( );
        }
    }
}
