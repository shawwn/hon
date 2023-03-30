///////////////////////////////////////////////////////////////////////  
//         Name: LeafLod.cpp
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
#include "LeafLod.h"
#include "BillboardLeaf.h"
#include "LibRandom_Source/IdvRandom.h"
#include <cfloat> // for FLT_MAX


///////////////////////////////////////////////////////////////////////  
//  CLeafLodEngine::CLeafLodEngine definition

CLeafLodEngine::CLeafLodEngine(const SIdvLeafInfo& sLeafInfo, float fSpacingTolerance, float fLeafReductionPercentage, float fSizeIncreaseFactor) :
    m_sLeafInfo(sLeafInfo),
    m_fLeafSpacingTolerance(fSpacingTolerance),
    m_fLeafReductionPercentage(fLeafReductionPercentage),
    m_fLeafSizeIncreaseFactor(fSizeIncreaseFactor)
{
}


///////////////////////////////////////////////////////////////////////  
//  CLeafLodEngine::ComputeNextLevel definition

LeafArray CLeafLodEngine::ComputeNextLevel(LeafArray vOrigLeafSet)
{
    LeafArray vNextLevel;

    FindPairs(vOrigLeafSet);            // pairs of leaves are combined into one
    vNextLevel = BuildNewLeaves( );

    return vNextLevel;
}


///////////////////////////////////////////////////////////////////////  
//  CLeafLodEngine::BuildNewLeaves definition

LeafArray CLeafLodEngine::BuildNewLeaves(void)
{
    static CIdvRandom cDice;
    LeafArray vLeaves;

    for (vector<SLodEntry>::iterator i = m_vPairs.begin( ); i != m_vPairs.end( ); ++i)
    {
        // random call is to randomly remove m_fLeafReductionPercentage of the pairings, resulting in an
        // arbitrary reduction of the leaf count of the next LOD level
        if (cDice.GetUniform(0.0, 1.0) > m_fLeafReductionPercentage)
        {
            CBillboardLeaf* pNewLeaf = i->m_pLeaf->Clone( );

            // adjust position based on new size and origin location
            CVec3 cNewPosition = (i->m_pLeaf->GetPosition( ) + i->m_pLeafMatch->GetPosition( )) * 0.5f;
            const SIdvLeafTexture& sTexture = m_sLeafInfo.m_vLeafTextures[i->m_pLeaf->GetTextureIndex( )];
            float fAdjust = (sTexture.m_cOrigin[1] - 0.5f) * (m_fLeafSizeIncreaseFactor * sTexture.m_cSizeUsed[1] * 0.5f);
            cNewPosition[2] += fAdjust;

            pNewLeaf->SetPosition(cNewPosition);
            pNewLeaf->SetColor((i->m_pLeaf->GetColor( ) + i->m_pLeafMatch->GetColor( )) * 0.5f, false);
            vLeaves.push_back(pNewLeaf);
        }
    }

    return vLeaves;
}


///////////////////////////////////////////////////////////////////////  
//  CLeafLodEngine::FindPairs definition

void CLeafLodEngine::FindPairs(LeafArray vOrigLeafSet)
{
    // initialize an vector of bools parallel to the leaves array (all entries false).  these
    // are used to flag if a leaf has been paired
    vector<bool> vFound(vOrigLeafSet.size( ));
    fill(vFound.begin( ), vFound.end( ), false);

    for (vector<CBillboardLeaf*>::iterator i = vOrigLeafSet.begin( ); i != vOrigLeafSet.end( ); ++i)
    {
        SLodEntry sLod(*i);

        float fShortestDistance = FLT_MAX;
        int nPairedIndex = -1;
        int nSearchIndex = i - vOrigLeafSet.begin( ) + 1;

        // find the closest leaf to *i that:
        //      1. hasn't already been paired 
        //      2. is closer than m_fLeafSpacingTolerance
        for (vector<CBillboardLeaf*>::iterator j = i + 1; j != vOrigLeafSet.end( ) && !vFound[nSearchIndex]; ++j)
        {
            if (!vFound[nSearchIndex])
            {
                float fDistance = (*i)->GetPosition( ).Distance((*j)->GetPosition( ));
                if (fDistance < m_fLeafSpacingTolerance &&
                    fDistance < fShortestDistance)
                {
                    fShortestDistance = fDistance;
                    sLod.m_pLeafMatch = *j;
                    nPairedIndex = nSearchIndex;
                }
            }
            nSearchIndex++;
        }
        
        if (sLod.m_pLeafMatch)
        {
            vFound[nPairedIndex] = true;
            m_vPairs.push_back(sLod); // save each pair for use in BuildNewLeaves()
        }
    }
}

