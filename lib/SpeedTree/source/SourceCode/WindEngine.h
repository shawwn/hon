///////////////////////////////////////////////////////////////////////  
//         Name: WindEngine.h
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
#include "SpeedTreeRT.h"
#include "StructsInfo.h"
#include "Debug.h"

// constants
const float c_fStrengthToFrequencyRatio = 8.0f;
const float c_fStrengthToThrowRatio = 180.0f;


///////////////////////////////////////////////////////////////////////  
//  class CWindMatrices declaration/definition
//
//  This class largely exists in order to encapsulate the matrix array,
//  which exists statically globally within the SpeedTree library.  It
//  is allocated globally, and needed to be destructed during the
//  application's exit.

class CWindMatrices
{
public:
        CWindMatrices( ) :
            m_usNumMatrices(0),
            m_pMatrices(new CTransform[4])
        {
            Resize(4);
        }
        ~CWindMatrices( )
        {
            m_usNumMatrices = 0;
            delete[] m_pMatrices;
            m_pMatrices = NULL;
        }

        CTransform&     GetMatrix(unsigned short usIndex) const { return m_pMatrices[usIndex]; }
        unsigned short  GetSize(void) const                     { return m_usNumMatrices; }
        void            Resize(unsigned short usCount)
        {
            delete[] m_pMatrices;
            m_usNumMatrices = usCount;
            m_pMatrices = new CTransform[usCount];

            for (int i = 0; i < usCount; ++i)
                m_pMatrices[i].LoadIdentity( );
        }
        void            SetMatrix(unsigned short usIndex, const float* pMatrix)
        {
            st_assert(usIndex < m_usNumMatrices);
            memcpy(m_pMatrices[usIndex].m_afData, pMatrix, 16 * sizeof(float));
        }

private:
        unsigned short  m_usNumMatrices;        // number of matrices affecting the trees
        CTransform*     m_pMatrices;            // table of wind matrices
};


///////////////////////////////////////////////////////////////////////  
//  class CWindEngine declaration

class CWindEngine
{
public:
        CWindEngine( );

        // general
        float                       GetWindStrength(void) const                 { return m_fWindStrength; }
        float                       SetWindStrength(float fNewStrength, float fOldStrength, float fOldTimeShift);
        void                        SetWindStrengthAndLeafAngles(float fNewStrength, const float* pRockAngles = NULL, const float* pRustleAngles = NULL, unsigned int uiNumLeafAngles = 0);

        void                        Init(const SIdvWindInfo& cWindInfo);
static  void                        SetTime(float fTime)                        { m_fTime = fTime; }

        float                       GetFrequencyTimeShift(void) const           { return m_fTimeFrequencyShift; }

        // branches
        CSpeedTreeRT::EWindMethod   GetBranchWindMethod(void) const             { return m_eBranchMethod; }
        void                        SetBranchWindMethod(CSpeedTreeRT::EWindMethod eMethod)  { m_eBranchMethod = eMethod; }

        // fronds
        CSpeedTreeRT::EWindMethod   GetFrondWindMethod(void) const             { return m_eFrondMethod; }
        void                        SetFrondWindMethod(CSpeedTreeRT::EWindMethod eMethod)  { m_eFrondMethod = eMethod; }

        // leaves
        CSpeedTreeRT::EWindMethod   GetLeafWindMethod(void) const               { return m_eLeafMethod; }
        void                        SetLeafWindMethod(CSpeedTreeRT::EWindMethod eMethod)    { m_eLeafMethod = eMethod; }
        bool                        GetLeafRockingState(void) const             { return m_bRockingLeaves; }
        void                        SetLeafRockingState(bool bFlag)             { m_bRockingLeaves = bFlag; }
        void                        RockLeaf(float fAzimuth, float fPitch, int nIndex, float fTimeOffset, CRotTransform& cTrans) const;

        // wind matrices
static  unsigned int                GetNumWindMatrices(void)                            { return m_cWindMatrices.GetSize( ); }
static  void                        SetNumWindMatrices(unsigned short usNumMatrices)    { m_cWindMatrices.Resize(usNumMatrices); }
static  CTransform&                 GetWindMatrix(unsigned short nMatrixIndex)          { return m_cWindMatrices.GetMatrix(nMatrixIndex); }
static  void                        SetWindMatrix(unsigned short nMatrixIndex, const float* pMatrix) { m_cWindMatrices.SetMatrix(nMatrixIndex, pMatrix); }
        void                        GetLocalMatrices(unsigned int& nStartingMatrix, unsigned int& nMatrixSpan) { nStartingMatrix = m_nStartingMatrix; nMatrixSpan = m_nMatrixSpan; }
        void                        SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan);

        void                        ResetLeafWindState( );

        // external rocking angles
        bool                        UsingExternalRockAngles(void) const                 { return m_uiNumLeafAngles != 0; }
        void                        RockLeaf(float fAzimuth, float fPitch, int nIndex, CRotTransform& cTrans) const;

        // speedwind support
        void                        SetSpeedWindRockScalar(float fRockScalar)           { m_fSpeedWindRockScalar = fRockScalar; }
        float                       GetSpeedWindRockScalar(void) const                  { return m_fSpeedWindRockScalar; }
        void                        SetSpeedWindRustleScalar(float fRustleScalar)       { m_fSpeedWindRustleScalar = fRustleScalar; }
        float                       GetSpeedWindRustleScalar(void) const                { return m_fSpeedWindRustleScalar; }

private:
        // general
static  float                       m_fTime;                    // global absolute time in seconds
        float                       m_fTimeFrequencyShift;      // changing wind strength causes undesired side effect in leaf rocking - this compensates
        float                       m_fWindStrength;            // strength of wind, 0.0 = no wind, 1.0 = hurricane strength
                                                            
        // branches                                         
        CSpeedTreeRT::EWindMethod   m_eBranchMethod;            // wind method applied to branches
                                                            
        // fronds                                           
        CSpeedTreeRT::EWindMethod   m_eFrondMethod;             // wind method applied to fronds

        // leaves
        CSpeedTreeRT::EWindMethod   m_eLeafMethod;              // wind method applied to leaves
        bool                        m_bRockingLeaves;           // flag for leaf rocking
        float                       m_fLeafFactors[2];          // set by SpeedTreeCAD, [0] = frequency multiplier, [1] = throw multiplier
        float                       m_fLeafFrequency;           // leaf rocking frequency set by SpeedTreeCAD
        float                       m_fLeafThrow;               // leaf throw set by SpeedTreeCAD
                                                            
        // wind matrices                                    
static  CWindMatrices               m_cWindMatrices;            // matrices shared by all SpeedTree instances
        unsigned int                m_nStartingMatrix;          // the start of the global matrix group that this instance of CWindEngine uses
        unsigned int                m_nMatrixSpan;              // the span of the global matrix group that this instance of CWindEngine uses
                                                            
        // externally supplied rocking angles               
        unsigned int                m_uiNumLeafAngles;          // number of entries in the leaf angles arrays
        const float*                m_pRockingAngles;           // an array of rocking angles supplied by the app
        const float*                m_pRustleAngles;            // an array of rustling angles supplied by the app
        
        // speedwind support
        float                       m_fSpeedWindRockScalar;     // percentage of speedwind rock angles used for leaf rocking
        float                       m_fSpeedWindRustleScalar;   // percentage of speedwind rustle angles used for leaf rocking
};
