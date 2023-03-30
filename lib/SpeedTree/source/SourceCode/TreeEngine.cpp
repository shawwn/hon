///////////////////////////////////////////////////////////////////////
//         Name: TreeEngine.cpp
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

#ifdef WIN32
#pragma warning(disable : 4786)
#endif
#include "Debug.h"
#include "TreeEngine.h"
#include "BillboardLeaf.h"
#include "WindEngine.h"
#include "LeafLod.h"
#include "IndexedGeometry.h"
#include <cfloat>

#if defined(linux) || defined(__APPLE__)
#include <stdlib.h>

template <class T>
inline T __max(T a, T b)
{
	return max(a, b);
}
#endif

const char* g_pszHeader = "__IdvSpt_02_";


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::CTreeEngine definition

CTreeEngine::CTreeEngine(CIndexedGeometry* pBranchGeometry) :
    m_fBillboardSize(1.0f),
    m_pBranch(NULL),
    m_fLod(1.0f),
    m_fOverrideTreeSize(-1.0f),
    m_fOverrideTreeVariance(0.0f),
    m_nBranchesWeightLevel(1),
    m_nNumBranchLodLevels(6),
    m_pBranchGeometry(NULL),
    m_fMinBranchVolumePercent(0.5f),
    m_fMaxBranchVolumePercent(1.0f),
    m_vLeafLods(NULL),
    m_bParsedLeafLods(false),
    m_bTransientDataIntact(true),
    m_fLeafReductionPercentage(0.3f),
    m_fBranchReductionFuzziness(0.0f),
    m_fLargeBranchPercent(0.05f)
{
    m_pBranchGeometry = pBranchGeometry;

    SetWind(0.25f);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::Compute definition

void CTreeEngine::Compute(float fLeafSizeIncreaseFactor)
{
    CIdvBranch::SetComputeLeaves(!m_bParsedLeafLods);

    // allocate needed memory
    m_pBranch = new CIdvBranch(NULL);
    if (!m_bParsedLeafLods)
        m_vLeafLods = new vector<CBillboardLeaf*>[m_sLeafInfo.m_nNumLeafLodLevels];

    // setup branches for later computing
    CIdvBranch::SetBranchLevelForWeighting(m_nBranchesWeightLevel);
    CIdvBranch::ClearBranchInfo( );
    CIdvBranch::SetLeafData(&m_sLeafInfo);
    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
        CIdvBranch::AddBranchInfo(m_vBranchInfo[i]);

    // determine tree size using tree's seed
    m_cRandom.Reseed(m_sTreeInfo.m_nSeed);
    float fTreeSize = m_cRandom.GetUniform(m_sTreeInfo.m_fSize - m_sTreeInfo.m_fSizeVariance,
                                           m_sTreeInfo.m_fSize + m_sTreeInfo.m_fSizeVariance);
    srand(m_sTreeInfo.m_nFlareSeed);

    // adjust relative leaf sizes
    if (m_sTreeInfo.m_fSize > 0.0f)
        for (unsigned int j = 0; j < m_sLeafInfo.m_vLeafTextures.size( ); ++j)
        {
            SIdvLeafTexture& sLeaf = m_sLeafInfo.m_vLeafTextures[j];
            sLeaf.m_cSizeUsed[0] = m_sTreeInfo.m_fSize * sLeaf.m_cSize[0];
            sLeaf.m_cSizeUsed[1] = m_sTreeInfo.m_fSize * sLeaf.m_cSize[1];
        }

    // compute the branches and leaves
    m_cRandom.Reseed(m_sTreeInfo.m_nSeed);
    m_pBranchGeometry->SetNumLodLevels(static_cast<unsigned short>(GetNumBranchLodLevels( )));
    m_pBranchGeometry->ResetStripCounter(0);
    m_pBranch->SetRoundRobinWindLevel(m_sTreeInfo.m_nSeed);
    m_pBranch->Compute(m_sTreeInfo.m_nSeed,
                       fTreeSize,
                       0,                               // starting level, 0 = trunk
                       GetPosition( ),                  // grow tree from this position
                       0.0f,                            // percent of parent (0.0 since there's no parent for trunk)
                       CRotTransform( ),                // identity rotation matrix
                       CVec3(0.0f, 0.0f, 1.0f),         // perturbed z-axis (starts as straight z)
                       m_pBranchGeometry,
                       m_vBillboardLeaves,
                       1.0f,                            // wind weight
                       m_sTreeInfo.m_nSeed,             // wind group
                       -1.0f);                          // parent radius (no parent, so -1)

    // build the discrete branch levels-of-detail
    BuildBranchLods( );

    // if the leaf LODs were not already read from the file, build them here
    if (!m_bParsedLeafLods)
        BuildLeafLods(fLeafSizeIncreaseFactor);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::Clone definition

void CTreeEngine::Clone(CTreeEngine* pClone, const CVec3& cPos, unsigned int nSeed) const
{
    // copy the branchinfo vector members
    pClone->m_vBranchInfo.clear( );
    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
    {
        //lint -esym(429,pInfo)  { pInfo is not freed or returned from Clone(), but it
        //is stored in pClone->m_vBranchInfo for later deletion }
        SIdvBranchInfo* pInfo = new SIdvBranchInfo;
        *pInfo = *m_vBranchInfo[i];
        pClone->m_vBranchInfo.push_back(pInfo);
    }

    // copy the rest of the important member variables
    pClone->SetPosition(cPos);
    pClone->m_nBranchesWeightLevel = m_nBranchesWeightLevel;
    pClone->m_sTreeInfo = m_sTreeInfo;
    pClone->m_sLeafInfo = m_sLeafInfo;
    pClone->m_sLeafInfo.m_pLeafTexCoords = NULL;
    pClone->m_sLeafInfo.m_pLeafVertexes = NULL;
    pClone->m_sLeafInfo.m_pTimeOffsets = NULL;
    pClone->m_sWindInfo = m_sWindInfo;
    pClone->SetBranchTexture(m_sTreeInfo.m_strBranchTextureFilename);
    pClone->SetLeafTextures(GetLeafTextures( ));
    pClone->m_fMinBranchVolumePercent = m_fMinBranchVolumePercent;
    pClone->m_fMaxBranchVolumePercent = m_fMaxBranchVolumePercent;

    // compute this new cloned tree based on the parameters set above
    pClone->SetSeed(nSeed);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::OverrideTreeSize definition

void CTreeEngine::OverrideTreeSize(float fNewSize, float fNewVariance)
{
    m_fOverrideTreeSize = fNewSize;
    m_fOverrideTreeVariance = fNewVariance;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::Parse definition

bool CTreeEngine::Parse(CTreeFileAccess& cFile)
{
    bool bSuccess = false;

    int nToken = cFile.ParseToken( );
    if (nToken != File_BeginFile)
        throw(IdvFileError("missing begin_file token"));

    string strHeader = cFile.ParseString( );
    if (strHeader != string(g_pszHeader))
        throw(IdvFileError("not a valid SpeedTree SPT file"));

    nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_BeginTreeInfo:
            ParseTreeInfo(cFile);
            break;
        case File_BeginGeneralLeafInfo:
            m_sLeafInfo.Parse(cFile);
            break;
        case File_BeginWindInfo:
            m_sWindInfo.Parse(cFile);
            break;
        default:
            throw(IdvFileError("malformed SpeedTree SPT file"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndFile);

    if (!cFile.EndOfFile( ) && cFile.PeekToken( ) == File_BeginLeafCluster)
    {
        (void) cFile.ParseToken( );
        ParseLeafCluster(cFile);
    }

    SetBranchTexture(m_sTreeInfo.m_strBranchTextureFilename);
    SetLeafTextures(GetLeafTextures( ));

    if (m_fOverrideTreeSize > -1.0f)
        SetSize(m_fOverrideTreeSize, m_fOverrideTreeVariance);
    bSuccess = true;

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::Save definition
//
//  Saves a binary description of the tree in the unsigned char memory block it
//  returns.  If this block is saved directly to a binary file, SpeedTreeCAD will
//  read it.  The block can also be embedded in a client-defined structure or
//  memory block.

void CTreeEngine::Save(CTreeFileAccess& cFile, bool bSaveLeafLods) const
{
    cFile.SaveToken(File_BeginFile);

    // magic id header
    cFile.SaveString(g_pszHeader);

    // general tree info
    cFile.SaveToken(File_BeginTreeInfo);

    cFile.SaveToken(File_Tree_BranchTexture);
    cFile.SaveString(m_sTreeInfo.m_strBranchTextureFilename);

    cFile.SaveToken(File_Tree_Far);
    cFile.SaveFloat(m_sTreeInfo.m_fFar);

    cFile.SaveToken(File_Tree_LOD);
    cFile.SaveBool(false); // m_bLOD is no longer used

    cFile.SaveToken(File_Tree_Near);
    cFile.SaveFloat(m_sTreeInfo.m_fNear);

    cFile.SaveToken(File_Tree_RandomSeed);
    cFile.SaveInt(m_sTreeInfo.m_nSeed);

    cFile.SaveToken(File_Tree_Size);
    cFile.SaveFloat(m_sTreeInfo.m_fSize);

    cFile.SaveToken(File_Tree_RenderStyle); // no longer used
    cFile.SaveInt(0);

    cFile.SaveToken(File_Tree_Size_Variance);
    cFile.SaveFloat(m_sTreeInfo.m_fSizeVariance);

    SaveBranchInfo(cFile);

    cFile.SaveToken(File_EndTreeInfo);

    // leaf info
    m_sLeafInfo.Save(cFile);

    // wind info
    m_sWindInfo.Save(cFile);

    cFile.SaveToken(File_EndFile);

    // leaf lod data
    if (bSaveLeafLods)
        SaveLeafCluster(cFile);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::FreeTransientData definition
//
//  This function deletes memory that will not be needed once the tree is
//  loaded and ready to render.

void CTreeEngine::FreeTransientData(void)
{
    if (TransientDataIntact( ))
    {
        // will recursively delete all branches in the tree
        delete m_pBranch;
        m_pBranch = NULL;

        // delete all of the discrete LOD leaves
        for (int i = 0; i < m_sLeafInfo.m_nNumLeafLodLevels; ++i)
        {
            if (m_vLeafLods)
            {
                for (unsigned int j = 0; j < m_vLeafLods[i].size( ); ++j)
                {
                    delete m_vLeafLods[i][j];
                    m_vLeafLods[i][j] = NULL;
                }
                m_vLeafLods[i].clear( );
            }
        }
        delete[] m_vLeafLods;
        m_vLeafLods = NULL;

        // delete all associated branch info structures
        for (unsigned int j = 0; j < m_vBranchInfo.size( ); ++j)
        {
            delete m_vBranchInfo[j];
            m_vBranchInfo[j] = NULL;
        }
        m_vBranchInfo.clear( );

        // make sure than functions that need this data know it's gone
        m_bTransientDataIntact = false;
    }
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetWind definition

void CTreeEngine::SetWind(float fStrength)
{
    m_sWindInfo.m_fStrength = fStrength;

    float fLeafFrequency = c_fStrengthToFrequencyRatio * fStrength * m_sWindInfo.m_cLeafFactors[1];
    float fLeafThrow = c_fStrengthToThrowRatio * fStrength * m_sWindInfo.m_cLeafFactors[0];

    m_sWindInfo.m_cLeafOscillation.Set(-fLeafThrow, fLeafThrow, fLeafFrequency);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::GetSize definition

void CTreeEngine::GetSize(float& fSize, float& fVariance)
{
    fSize = m_sTreeInfo.m_fSize;
    fVariance = m_sTreeInfo.m_fSizeVariance;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetSize definition

void CTreeEngine::SetSize(float fSize, float fVariance)
{
    m_sTreeInfo.m_fSize = fSize;
    m_sTreeInfo.m_fSizeVariance = fVariance;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ComputeLod definition

float CTreeEngine::ComputeLod(void)
{
    float fDistance = m_cCameraPos.Distance(m_cPosition);

    m_fLod = 1.0f - (fDistance - m_sTreeInfo.m_fNear) / (m_sTreeInfo.m_fFar - m_sTreeInfo.m_fNear);

    if (m_fLod > 1.0f)
        m_fLod = 1.0f;
    else if (m_fLod < 0.0f)
        m_fLod = 0.0f;

    return m_fLod;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::GetLodLimits definition

void CTreeEngine::GetLodLimits(float& fNear, float& fFar) const
{
    fNear = m_sTreeInfo.m_fNear;
    fFar = m_sTreeInfo.m_fFar;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetLodLimits definition

void CTreeEngine::SetLodLimits(float fNear, float fFar)
{
    m_sTreeInfo.m_fNear = fNear;
    m_sTreeInfo.m_fFar = fFar;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseTreeInfo definition

void CTreeEngine::ParseTreeInfo(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
            case File_Tree_BranchTexture:
                m_sTreeInfo.m_strBranchTextureFilename = cFile.ParseString( );
                m_sTreeInfo.m_strBranchTextureFilename = m_sTreeInfo.m_strBranchTextureFilename.NoPath( );
                break;
            case File_Tree_RandomSeed:
                {
                    int nSeed = cFile.ParseInt( );
                    SetSeed(static_cast<unsigned int>(nSeed));
                }
                break;
            case File_Tree_Near:
                m_sTreeInfo.m_fNear = cFile.ParseFloat( );
                break;
            case File_Tree_Far:
                m_sTreeInfo.m_fFar = cFile.ParseFloat( );
                break;
            case File_Tree_RenderStyle:
                (void) cFile.ParseInt( );
                break;
            case File_Tree_LOD:
                (void) cFile.ParseBool( ); // m_bLod is no longer used
                break;
            case File_Tree_Size:
                m_sTreeInfo.m_fSize = cFile.ParseFloat( );
                break;
            case File_Tree_Size_Variance:
                m_sTreeInfo.m_fSizeVariance = cFile.ParseFloat( );
                break;
            case File_BeginBranchInfo:
                ParseBranchInfo(cFile);
                break;
            default:
                throw(IdvFileError("malformed general tree information"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndTreeInfo);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetSeed definition
//
//  Seed values:
//
//       0 = pick a random seed
//       1 = use existing internal seed (from m_sTreeInfo.m_nSeed)
//      >1 = use nSeed parameter as seed value

void CTreeEngine::SetSeed(unsigned int nSeed)
{
    if (nSeed == 0)
    {
        // pick a random seed
        const int c_nMaxSeed = 1000000;

        m_cRandom.Reseed( );
        m_sTreeInfo.m_nSeed = static_cast<int>(m_cRandom.GetUniform(2.0f, static_cast<float>(c_nMaxSeed)));
    }
    else if (nSeed == 1)
    {
        // do nothing
    }
    else
    {
        // use parameter as new seed
        m_sTreeInfo.m_nSeed = nSeed;
    }
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::BuildBranchLods definition
//
//  BuildBranchLods() will start to remove branches from the tree in order
//  of their relative volume.  Smaller branches go first because they have
//  less of a visual impact.  Larger branches remain because they define
//  the bulk of the tree.

void CTreeEngine::BuildBranchLods(void)
{
    // build a linear vector of branch tree
    vector<CIdvBranch*> vAllBranches;
    if (m_pBranch &&
        m_pBranchGeometry->GetVertexCount( ) > 0)
        m_pBranch->BuildBranchVector(vAllBranches);

    // compute total volume of all branches
    float fTotalVolume = 0.0f;
    float fLargestVolume = 0.0f;
    for (unsigned int i = 0; i < vAllBranches.size( ); ++i)
    {
        float fVolume = vAllBranches[i]->GetVolume( );
        fTotalVolume += fVolume;

        if (fVolume > fLargestVolume)
            fLargestVolume = fVolume;
    }

    // implement fuzziness
    CIdvRandom cRandom;
    float fSavePercent = 1.0f - m_fLargeBranchPercent;
    vector<CIdvBranch*> vSavedBranches;
    for (unsigned int i = 0; i < vAllBranches.size( ); ++i)
    {
        float fVolume = vAllBranches[i]->GetVolume( );

        if (fVolume > fLargestVolume * fSavePercent)
        {
            vSavedBranches.push_back(vAllBranches[i]);
            (void) vAllBranches.erase(vAllBranches.begin( ) + i--);
        }
        else
        {
            // compute a new, fuzzy volume
            float fFuzziness = cRandom.GetUniform(0.0f, m_fBranchReductionFuzziness), fOneMinusFuzziness = 1.0f - fFuzziness;
            fVolume = (fOneMinusFuzziness * fVolume) + (fFuzziness * fLargestVolume);
            // make sure we don't get a negative volume
            fVolume = __max(fVolume, 0.0f);
            vAllBranches[i]->SetFuzzyVolume(fVolume);
        }
    }

    // sort branches in descending order of volume and add insert save branches at the beginning
    CIdvBranch::SortBranchVector(vAllBranches);
    CIdvBranch::SortBranchVector(vSavedBranches);
    for (unsigned int i = 0; i < vSavedBranches.size( ); ++i)
        (void) vAllBranches.insert(vAllBranches.begin( ), vSavedBranches[i]);

    // for each LOD level, compute a target volume and use enough
    // branches to reach that target
    int nNumBranchLodLevels = GetNumBranchLodLevels( );
    for (unsigned int uiLod = 0; uiLod < (unsigned int) nNumBranchLodLevels; ++uiLod)
    {
        if (uiLod == 0)
            m_pBranchGeometry->DeleteLodStrip(0);

        // figure what part of the branch structure stays for lod level i, computed by volume
        float fPercent = (nNumBranchLodLevels < 2) ? 1.0f : VecInterpolate(m_fMaxBranchVolumePercent, m_fMinBranchVolumePercent, uiLod / float(nNumBranchLodLevels - 1));
        float fTargetVolume = fPercent * fTotalVolume;

        // which branches will contribute to lod level uiLod
        int nBranchCount = 0;
        float fLodVolume = 0.0f;
        for (unsigned int j = 0; j < vAllBranches.size( ) && fLodVolume < fTargetVolume; ++j)
        {
            fLodVolume += vAllBranches[j]->GetVolume( );
            nBranchCount++;
        }

        // setup stripping container
        m_pBranchGeometry->ResetStripCounter(static_cast<unsigned short>(uiLod));

        // compute the indexes for each branch in this lod
        if (uiLod == 0 && nBranchCount == 0)
        {
            // need to zero out the original LOD strip created in CIdvBranch::Compute()
            m_pBranchGeometry->AddStrip(0, NULL, 0);
        }
        else
            for (int k = 0; k < nBranchCount; ++k)
                vAllBranches[k]->ComputeLod(uiLod, m_pBranchGeometry);
    }
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseBranchInfo definition

void CTreeEngine::ParseBranchInfo(CTreeFileAccess& cFile)
{
    ClearBranchInfo( );

    int nSize = cFile.ParseInt( );
    for (int i = 0; i < nSize; ++i)
    {
        // Parse allocates a new SIdvBranchInfo structure
        SIdvBranchInfo* pInfo = SIdvBranchInfo::Parse(cFile);
        m_vBranchInfo.push_back(pInfo);
    }

    int nToken = cFile.ParseToken( );
    if (nToken != File_EndBranchInfo)
        throw(IdvFileError("malformed branch data"));
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveBranchInfo definition

void CTreeEngine::SaveBranchInfo(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginBranchInfo);

    int nSize = m_vBranchInfo.size( );
    cFile.SaveInt(nSize);
    for (int i = 0; i < nSize; ++i)
    {
        cFile.SaveToken(File_BeginBranchLevel);
        m_vBranchInfo[i]->Save(cFile);
        cFile.SaveToken(File_EndBranchLevel);
    }

    cFile.SaveToken(File_EndBranchInfo);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::BuildLeafLods definition
//
//  Wrapper invocation of CLeafLodEngine - see LeafLod.h & .cpp for details

void CTreeEngine::BuildLeafLods(float fLeafSizeIncreaseFactor)
{
    // find the largest leaf used in the tree
    float fLargestLeaf = -1.0f;
    for (unsigned int i = 0; i < m_sLeafInfo.m_vLeafTextures.size( ); ++i)
    {
        const SIdvLeafTexture& sTexture = m_sLeafInfo.m_vLeafTextures[i];
        float fLargestSide = sTexture.m_cSizeUsed[0] > sTexture.m_cSizeUsed[1] ? sTexture.m_cSizeUsed[0] : sTexture.m_cSizeUsed[1];
        if (fLargestSide * m_sLeafInfo.m_fSpacingTolerance > fLargestLeaf)
            fLargestLeaf = fLargestSide * m_sLeafInfo.m_fSpacingTolerance;
    }
    // if no leaves, use an arbitrary value
    if (fLargestLeaf == -1.0f)
        fLargestLeaf = 10.0f;

    // m_vBillboardLeaves contains all of the generated leaves, it will be the highest LOD
    if (m_vLeafLods)
    {
        m_vLeafLods[0] = m_vBillboardLeaves;
        for (int j = 1; j < m_sLeafInfo.m_nNumLeafLodLevels; ++j)
        {
            float fSizeMultiplier = 1.0f + fLeafSizeIncreaseFactor * float(j);
            CLeafLodEngine cLod(m_sLeafInfo, fLargestLeaf * 10.0f, m_fLeafReductionPercentage, fSizeMultiplier);
            m_vLeafLods[j] = cLod.ComputeNextLevel(m_vLeafLods[j - 1]);
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseLeafCluster definition
//
//  SPT files may or may not have all of the leaf LODs stored - this function
//  will retrieve the leaves is they're there.

void CTreeEngine::ParseLeafCluster(CTreeFileAccess& cFile)
{
    int nLeafLod = 0;

    int nNumLeafLodLevels = cFile.ParseInt( );
    m_sLeafInfo.m_nNumLeafLodLevels = nNumLeafLodLevels;
    delete[] m_vLeafLods;
    m_vLeafLods = new vector<CBillboardLeaf*>[m_sLeafInfo.m_nNumLeafLodLevels];

    int nToken = cFile.ParseToken( );
    while (nToken != File_EndLeafCluster)
    {
        if (nLeafLod >= m_sLeafInfo.m_nNumLeafLodLevels)
            throw(IdvFileError("too many leaf lod levels"));

        if (nToken != File_BeginLeafLodLevel)
            throw(IdvFileError("malformed leaf lod data"));

        nToken = cFile.ParseToken( );
        while (nToken != File_EndLeafLodLevel)
        {
            if (nToken != File_BeginBillboardLeaf)
                throw(IdvFileError("malformed leaf lod data"));

            m_vLeafLods[nLeafLod].push_back(CBillboardLeaf::Parse(cFile));

            nToken = cFile.ParseToken( );
        }
        ++nLeafLod;

        nToken = cFile.ParseToken( );
    }
    m_bParsedLeafLods = true;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveLeafCluster
//
//  SPT files may or may not have all of the leaf LODs stored - this function
//  will save the leaves LODs if invoked.

void CTreeEngine::SaveLeafCluster(CTreeFileAccess& cFile) const
{
    if (m_vLeafLods)
    {
        cFile.SaveToken(File_BeginLeafCluster);
        cFile.SaveInt(m_sLeafInfo.m_nNumLeafLodLevels);
        for (int i = 0; i < m_sLeafInfo.m_nNumLeafLodLevels; ++i)
        {
            cFile.SaveToken(File_BeginLeafLodLevel);
            for (unsigned int j = 0; j < m_vLeafLods[i].size( ); ++j)
                m_vLeafLods[i][j]->Save(cFile);
            cFile.SaveToken(File_EndLeafLodLevel);
        }
        cFile.SaveToken(File_EndLeafCluster);
    }
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveLodInfo
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::SaveLodInfo(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginEngineLodInfo);

    cFile.SaveToken(File_BranchNumLods);
    cFile.SaveInt(m_nNumBranchLodLevels);

    cFile.SaveToken(File_BranchMinVolumePercent);
    cFile.SaveFloat(m_fMinBranchVolumePercent);

    cFile.SaveToken(File_LeafReductionFactor);
    cFile.SaveFloat(m_fLeafReductionPercentage);

    cFile.SaveToken(File_LeafNumLods);
    cFile.SaveInt(m_sLeafInfo.m_nNumLeafLodLevels);

    cFile.SaveToken(File_BranchMaxVolumePercent);
    cFile.SaveFloat(m_fMaxBranchVolumePercent);

    cFile.SaveToken(File_BranchReductionFuzziness);
    cFile.SaveFloat(m_fBranchReductionFuzziness);

    cFile.SaveToken(File_LargeBranchPercent);
    cFile.SaveFloat(m_fLargeBranchPercent);

    cFile.SaveToken(File_EndEngineLodInfo);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseLodInfo
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::ParseLodInfo(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_BranchNumLods:
            m_nNumBranchLodLevels = cFile.ParseInt( );
            break;
        case File_BranchMinVolumePercent:
            m_fMinBranchVolumePercent = cFile.ParseFloat( );
            break;
        case File_LeafReductionFactor:
            m_fLeafReductionPercentage = cFile.ParseFloat( );
            break;
        case File_LeafNumLods:
            m_sLeafInfo.m_nNumLeafLodLevels = cFile.ParseInt( );
            break;
        case File_BranchMaxVolumePercent:
            m_fMaxBranchVolumePercent = cFile.ParseFloat( );
            break;
        case File_BranchReductionFuzziness:
            m_fBranchReductionFuzziness = cFile.ParseFloat( );
            break;
        case File_LargeBranchPercent:
            m_fLargeBranchPercent = cFile.ParseFloat( );
            break;
        default:
            throw(IdvFileError("malformed engine lod data"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndEngineLodInfo);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetBranchTextureFilename

void CTreeEngine::SetBranchTextureFilename(const char* pFilename)
{
    m_sTreeInfo.m_strBranchTextureFilename = pFilename;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SetLeafTextureFilename

void CTreeEngine::SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename)
{
    st_assert(nLeafMapIndex < m_sLeafInfo.m_vLeafTextures.size( ));
    m_sLeafInfo.m_vLeafTextures[nLeafMapIndex].m_strFilename = pFilename;
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveTextureControls
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::SaveTextureControls(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginTextureControls);

    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
    {
        cFile.SaveToken(File_TextureOffset);
        cFile.SaveBool(m_vBranchInfo[i]->m_bRandomTCoordOffset);

        cFile.SaveToken(File_TextureTwist);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fTwist);
    }

    cFile.SaveToken(File_EndTextureControls);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseTextureControls
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::ParseTextureControls(CTreeFileAccess& cFile)
{
    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
    {
        if (cFile.ParseToken( ) != File_TextureOffset)
            throw(IdvFileError("malformed texture controls"));
        m_vBranchInfo[i]->m_bRandomTCoordOffset = cFile.ParseBool( );

        if (cFile.ParseToken( ) != File_TextureTwist)
            throw(IdvFileError("malformed texture controls"));
        m_vBranchInfo[i]->m_fTwist = cFile.ParseFloat( );
    }

    if (cFile.ParseToken( ) != File_EndTextureControls)
        throw(IdvFileError("malformed texture controls"));
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveFlareInfo
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::SaveFlareInfo(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginFlareInfo);

    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
    {
        cFile.SaveToken(File_FlareSegmentPackingExponent);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fSegmentPackingExponent);

        cFile.SaveToken(File_FlareNumFlares);
        cFile.SaveInt(m_vBranchInfo[i]->m_nNumFlares);

        cFile.SaveToken(File_FlareBalance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fFlareBalance);

        cFile.SaveToken(File_FlareRadialInfluence);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fRadialInfluence);

        cFile.SaveToken(File_FlareRadialInfluenceVariance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fRadialInfluenceVariance);

        cFile.SaveToken(File_FlareRadialExponent);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fRadialExponent);

        cFile.SaveToken(File_FlareRadialDistance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fRadialDistance);

        cFile.SaveToken(File_FlareRadialDistanceVariance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fRadialVariance);

        cFile.SaveToken(File_FlareLengthDistance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fLengthDistance);

        cFile.SaveToken(File_FlareLengthDistanceVariance);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fLengthVariance);

        cFile.SaveToken(File_LengthExponent);
        cFile.SaveFloat(m_vBranchInfo[i]->m_fLengthExponent);
    }

    cFile.SaveToken(File_EndFlareInfo);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseFlareInfo
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::ParseFlareInfo(CTreeFileAccess& cFile)
{
    for (unsigned int i = 0; i < m_vBranchInfo.size( ); ++i)
    {
        if (cFile.ParseToken( ) != File_FlareSegmentPackingExponent)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fSegmentPackingExponent = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareNumFlares)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_nNumFlares = cFile.ParseInt( );

        if (cFile.ParseToken( ) != File_FlareBalance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fFlareBalance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareRadialInfluence)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fRadialInfluence = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareRadialInfluenceVariance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fRadialInfluenceVariance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareRadialExponent)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fRadialExponent = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareRadialDistance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fRadialDistance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareRadialDistanceVariance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fRadialVariance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareLengthDistance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fLengthDistance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_FlareLengthDistanceVariance)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fLengthVariance = cFile.ParseFloat( );

        if (cFile.ParseToken( ) != File_LengthExponent)
            throw(IdvFileError("malformed flare info"));
        m_vBranchInfo[i]->m_fLengthExponent = cFile.ParseFloat( );
    }

    if (cFile.ParseToken( ) != File_EndFlareInfo)
        throw(IdvFileError("malformed flare info"));
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::SaveFlareSeed
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::SaveFlareSeed(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_FlareSeed);
    cFile.SaveInt(m_sTreeInfo.m_nFlareSeed);
}


///////////////////////////////////////////////////////////////////////
//  CTreeEngine::ParseFlareSeed
//
//  This data must be saved separately at the end of the file to
//  maintain backward compatibility.

void CTreeEngine::ParseFlareSeed(CTreeFileAccess& cFile)
{
    m_sTreeInfo.m_nFlareSeed = cFile.ParseInt( );
}
