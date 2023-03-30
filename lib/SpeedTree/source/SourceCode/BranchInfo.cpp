///////////////////////////////////////////////////////////////////////  
//         Name: BranchInfo.cpp
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
#include "FileAccess.h"


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SIdvBranchInfo definition

SIdvBranchInfo::SIdvBranchInfo( ) :
    m_nCrossSectionSegments(6),
    m_nSegments(3),
    m_fFirstBranch(0.3f),
    m_fLastBranch(1.0f),
    m_fFrequency(0.3f),
    m_fSTile(1.0f),
    m_fTTile(1.0f),
    m_bSTileAbsolute(true),
    m_bTTileAbsolute(false),
    m_pDisturbance(NULL),
    m_pGravity(NULL),
    m_pFlexibility(NULL),
    m_pFlexibilityScale(NULL),
    m_pLength(NULL),
    m_pRadius(NULL),
    m_pRadiusScale(NULL),
    m_pStartAngle(NULL),
    m_pAngleProfile(NULL),
    m_bRandomTCoordOffset(false),
    m_fTwist(0.0f),
    m_fSegmentPackingExponent(1.0f),
    m_nNumFlares(0),
    m_fFlareBalance(1.0f),
    m_fRadialInfluence(30.0f),
    m_fRadialInfluenceVariance(10.0f),
    m_fRadialExponent(1.0f),
    m_fRadialDistance(0.5f),
    m_fRadialVariance(0.25f),
    m_fLengthDistance(0.3f),
    m_fLengthVariance(0.1f),
    m_fLengthExponent(1.0f)
{
    // use default splines
    m_pFlexibility = new CIdvBezierSpline;
    m_pGravity = new CIdvBezierSpline;
    m_pRadius = new CIdvBezierSpline;
    m_pStartAngle = new CIdvBezierSpline;
    m_pRadiusScale = new CIdvBezierSpline;
    m_pLength = new CIdvBezierSpline;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SIdvBranchInfo definition

SIdvBranchInfo::SIdvBranchInfo(const SIdvBranchInfo& sRight)
{
    *this = sRight;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::~SIdvBranchInfo definition

SIdvBranchInfo::~SIdvBranchInfo( )
{
    delete m_pFlexibility;
    delete m_pFlexibilityScale;
    delete m_pGravity;
    delete m_pDisturbance;
    delete m_pStartAngle;
    delete m_pRadius;
    delete m_pRadiusScale;
    delete m_pLength;
    delete m_pAngleProfile;

    // set to NULL to force run-time error is class is accessed
    // after deletion
    m_pFlexibility = NULL;
    m_pFlexibilityScale = NULL;
    m_pGravity = NULL;
    m_pDisturbance = NULL;
    m_pStartAngle = NULL;
    m_pRadius = NULL;
    m_pRadiusScale = NULL;
    m_pLength = NULL;
    m_pAngleProfile = NULL;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::operator= definition

SIdvBranchInfo& SIdvBranchInfo::operator=(const SIdvBranchInfo& sInfo)
{
    if (&sInfo != this)
    {
        delete m_pDisturbance;
        m_pDisturbance = new CIdvBezierSpline;
        if (sInfo.m_pDisturbance)
            *m_pDisturbance = *sInfo.m_pDisturbance;

        delete m_pGravity;
        m_pGravity = new CIdvBezierSpline;
        if (sInfo.m_pGravity)
            *m_pGravity = *sInfo.m_pGravity;

        delete m_pFlexibility;
        m_pFlexibility = new CIdvBezierSpline;
        if (sInfo.m_pFlexibility)
            *m_pFlexibility = *sInfo.m_pFlexibility;

        delete m_pFlexibilityScale;
        m_pFlexibilityScale = new CIdvBezierSpline;
        if (sInfo.m_pFlexibilityScale)
            *m_pFlexibilityScale = *sInfo.m_pFlexibilityScale;

        delete m_pLength;
        m_pLength = new CIdvBezierSpline;
        if (sInfo.m_pLength)
            *m_pLength = *sInfo.m_pLength;

        delete m_pRadius;
        m_pRadius = new CIdvBezierSpline;
        if (sInfo.m_pRadius)
            *m_pRadius = *sInfo.m_pRadius;

        delete m_pRadiusScale;
        m_pRadiusScale = new CIdvBezierSpline;
        if (sInfo.m_pRadiusScale)
            *m_pRadiusScale = *sInfo.m_pRadiusScale;

        delete m_pStartAngle;
        m_pStartAngle = new CIdvBezierSpline;
        if (sInfo.m_pStartAngle)
            *m_pStartAngle = *sInfo.m_pStartAngle;

        delete m_pAngleProfile;
        m_pAngleProfile = new CIdvBezierSpline;
        if (sInfo.m_pAngleProfile)
            *m_pAngleProfile = *sInfo.m_pAngleProfile;

        // resolution-specifiers
        m_nCrossSectionSegments = sInfo.m_nCrossSectionSegments;
        m_nSegments = sInfo.m_nSegments;

        // branch generation
        m_fFirstBranch = sInfo.m_fFirstBranch;
        m_fLastBranch = sInfo.m_fLastBranch;
        m_fFrequency = sInfo.m_fFrequency;

        // tiling/texture controls
        m_fSTile = sInfo.m_fSTile;
        m_fTTile = sInfo.m_fTTile;
        m_bSTileAbsolute = sInfo.m_bSTileAbsolute;
        m_bTTileAbsolute = sInfo.m_bTTileAbsolute;
        m_bRandomTCoordOffset = sInfo.m_bRandomTCoordOffset;
        m_fTwist = sInfo.m_fTwist;

        // base flaring controls
        m_fSegmentPackingExponent = sInfo.m_fSegmentPackingExponent;
        m_nNumFlares = sInfo.m_nNumFlares;
        m_fFlareBalance = sInfo.m_fFlareBalance;
        m_fRadialInfluence = sInfo.m_fRadialInfluence;
        m_fRadialInfluenceVariance = sInfo.m_fRadialInfluenceVariance;
        m_fRadialExponent = sInfo.m_fRadialExponent;
        m_fRadialDistance = sInfo.m_fRadialDistance;
        m_fRadialVariance = sInfo.m_fRadialVariance;
        m_fLengthDistance = sInfo.m_fLengthDistance;
        m_fLengthVariance = sInfo.m_fLengthVariance;
        m_fLengthExponent = sInfo.m_fLengthExponent;
    }

    return *this;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::Parse definition

SIdvBranchInfo* SIdvBranchInfo::Parse(CTreeFileAccess& cFile)
{
    SIdvBranchInfo* pInfo = new SIdvBranchInfo;

    int nToken = cFile.ParseToken( );
    if (nToken != File_BeginBranchLevel)
        throw(IdvFileError("malformed branch data"));

    nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
            case File_Branch_Disturbance:
                pInfo->SetDisturbance(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_EndAngle:
                pInfo->SetGravity(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_Flexibility:
                pInfo->SetFlexibility(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_FlexibilityScale:
                pInfo->SetFlexibilityScale(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_AngleProfile:
                pInfo->SetAngleProfile(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_Length:
                pInfo->SetLength(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_Radius:
                pInfo->SetRadius(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_RadiusScale:
                pInfo->SetRadiusScale(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_StartAngle:
                pInfo->SetStartAngle(cFile.ParseBranchParameter( ));
                break;
            case File_Branch_CrossSectionSegments:
                pInfo->m_nCrossSectionSegments = cFile.ParseInt( );
                break;
            case File_Branch_Segments:
                pInfo->m_nSegments = cFile.ParseInt( );
                break;
            case File_Branch_FirstBranch:
                pInfo->m_fFirstBranch = cFile.ParseFloat( );
                break;
            case File_Branch_LastBranch:
                pInfo->m_fLastBranch = cFile.ParseFloat( );
                break;
            case File_Branch_Frequency:
                pInfo->m_fFrequency = cFile.ParseFloat( );
                break;
            case File_Branch_STile:
                pInfo->m_fSTile = cFile.ParseFloat( );
                break;
            case File_Branch_TTile:
                pInfo->m_fTTile = cFile.ParseFloat( );
                break;
            case File_Branch_STileAbsolute:
                pInfo->m_bSTileAbsolute = cFile.ParseBool( );
                break;
            case File_Branch_TTileAbsolute:
                pInfo->m_bTTileAbsolute = cFile.ParseBool( );
                break;
            default:
                throw(IdvFileError("malformed general branch information"));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndBranchLevel);

    return pInfo;
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::Save definition

void SIdvBranchInfo::Save(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_Branch_Disturbance);
    cFile.SaveBranchParameter(m_pDisturbance);

    cFile.SaveToken(File_Branch_EndAngle);
    cFile.SaveBranchParameter(m_pGravity);

    cFile.SaveToken(File_Branch_Flexibility);
    cFile.SaveBranchParameter(m_pFlexibility);

    cFile.SaveToken(File_Branch_FlexibilityScale);
    cFile.SaveBranchParameter(m_pFlexibilityScale);

    cFile.SaveToken(File_Branch_Length);
    cFile.SaveBranchParameter(m_pLength);

    cFile.SaveToken(File_Branch_Radius);
    cFile.SaveBranchParameter(m_pRadius);

    cFile.SaveToken(File_Branch_RadiusScale);
    cFile.SaveBranchParameter(m_pRadiusScale);

    cFile.SaveToken(File_Branch_StartAngle);
    cFile.SaveBranchParameter(m_pStartAngle);

    cFile.SaveToken(File_Branch_CrossSectionSegments);
    cFile.SaveInt(m_nCrossSectionSegments);

    cFile.SaveToken(File_Branch_Segments);
    cFile.SaveInt(m_nSegments);

    cFile.SaveToken(File_Branch_FirstBranch);
    cFile.SaveFloat(m_fFirstBranch);

    cFile.SaveToken(File_Branch_LastBranch);
    cFile.SaveFloat(m_fLastBranch);

    cFile.SaveToken(File_Branch_Frequency);
    cFile.SaveFloat(m_fFrequency);

    cFile.SaveToken(File_Branch_STile);
    cFile.SaveFloat(m_fSTile);

    cFile.SaveToken(File_Branch_TTile);
    cFile.SaveFloat(m_fTTile);

    cFile.SaveToken(File_Branch_STileAbsolute);
    cFile.SaveBool(m_bSTileAbsolute);

    cFile.SaveToken(File_Branch_TTileAbsolute);
    cFile.SaveBool(m_bTTileAbsolute);

    cFile.SaveToken(File_Branch_AngleProfile);
    cFile.SaveBranchParameter(m_pAngleProfile);
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetDisturbance definition

void SIdvBranchInfo::SetDisturbance(CIdvBezierSpline* pSpline)
{
    if (m_pDisturbance != pSpline)
    {
        delete m_pDisturbance;
        m_pDisturbance = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetGravity definition

void SIdvBranchInfo::SetGravity(CIdvBezierSpline* pSpline)
{
    if (m_pGravity != pSpline)
    {
        delete m_pGravity;
        m_pGravity = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetFlexibility definition

void SIdvBranchInfo::SetFlexibility(CIdvBezierSpline* pSpline)
{
    if (m_pFlexibility != pSpline)
    {
        delete m_pFlexibility;
        m_pFlexibility = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetFlexibilityScale definition

void SIdvBranchInfo::SetFlexibilityScale(CIdvBezierSpline* pSpline)
{
    if (m_pFlexibilityScale != pSpline)
    {
        delete m_pFlexibilityScale;
        m_pFlexibilityScale = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetLength definition

void SIdvBranchInfo::SetLength(CIdvBezierSpline* pSpline)
{
    if (m_pLength != pSpline)
    {
        delete m_pLength;
        m_pLength = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetRadius definition

void SIdvBranchInfo::SetRadius(CIdvBezierSpline* pSpline)
{
    if (m_pRadius != pSpline)
    {
        delete m_pRadius;
        m_pRadius = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetRadiusScale definition

void SIdvBranchInfo::SetRadiusScale(CIdvBezierSpline* pSpline)
{
    if (m_pRadiusScale != pSpline)
    {
        delete m_pRadiusScale;
        m_pRadiusScale = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetStartAngle definition

void SIdvBranchInfo::SetStartAngle(CIdvBezierSpline* pSpline)
{
    if (m_pStartAngle != pSpline)
    {
        delete m_pStartAngle;
        m_pStartAngle = pSpline;
    }
}


///////////////////////////////////////////////////////////////////////  
//  SIdvBranchInfo::SetAngleProfile definition

void SIdvBranchInfo::SetAngleProfile(CIdvBezierSpline* pSpline)
{
    if (m_pAngleProfile != pSpline)
    {
        delete m_pAngleProfile;
        m_pAngleProfile = pSpline;
    }
}


