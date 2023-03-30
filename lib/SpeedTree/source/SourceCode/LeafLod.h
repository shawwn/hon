///////////////////////////////////////////////////////////////////////  
//         Name: LeafLod.h
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
#include "StructsInfo.h"
typedef vector<CBillboardLeaf*> LeafArray;
#pragma warning (disable : 4512)


///////////////////////////////////////////////////////////////////////  
//  class CLeafLodEngine declaration
//
//  Given a set of billboard leaves (CBillboardLeaf), this class will
//  construct a set of fewer, larger leaves for an appropriate LOD
//  step down.

class CLeafLodEngine
{
private:

        ///////////////////////////////////////////////////////////////////////  
        //  internal struct SLodEntry definition

        struct SLodEntry
        {
            SLodEntry(const CBillboardLeaf* pLeaf) :
                m_pLeaf(pLeaf),
                m_pLeafMatch(NULL)
            {
            }

            const CBillboardLeaf*   m_pLeaf;                // main leaf
            CBillboardLeaf*         m_pLeafMatch;           // leaf paired with the main leaf
        };

public:
        CLeafLodEngine(const SIdvLeafInfo& sLeafInfo, float fSpacingTolerance, float fLeafReductionPercentage, float fSizeIncreaseFactor);

        LeafArray               ComputeNextLevel(LeafArray vOrigLeafSet);

private:
        LeafArray               BuildNewLeaves(void);
        void                    FindPairs(LeafArray vOrigLeafSet);

        vector<SLodEntry>       m_vPairs;                       // list of leaf pairs that will combined into one larger leaf       
        float                   m_fLeafSpacingTolerance;        // don't pick leaves any further apart than this distance
        float                   m_fLeafReductionPercentage;     // controls what percentage of leaves are removed at each LOD level
        const SIdvLeafInfo&     m_sLeafInfo;                    // contains leaf sizes, origins, etc.
        float                   m_fLeafSizeIncreaseFactor;      // determines how much the size of reduced leaf set is increased.
};
