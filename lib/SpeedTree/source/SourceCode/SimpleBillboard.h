///////////////////////////////////////////////////////////////////////  
//         Name: SimpleBillboard.h
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
#include "Debug.h"
#include "LibVector_Source/IdvVector.h"
#include "UpVector.h"


///////////////////////////////////////////////////////////////////////  
//  class CSimpleBillboard declaration
//

class CSimpleBillboard
{
public:
        CSimpleBillboard( );

static  void            ComputeUnitBillboard(const float* pCameraDir);
        const float*    GetBillboardCoords(float fWidth, float fHeight)
        {
            if (!IsValid( ))
            {
#ifdef UPVECTOR_POS_Y
                float afScale[3] = { fWidth, fHeight, fWidth };
#else
                float afScale[3] = { fWidth, fWidth, fHeight };
#endif

                m_acTreeBillboard[0] = afScale[0] * m_acUnitBillboard[0][0];
                m_acTreeBillboard[1] = afScale[1] * m_acUnitBillboard[0][1];
                m_acTreeBillboard[2] = afScale[2] * m_acUnitBillboard[0][2];
                m_acTreeBillboard[3] = afScale[0] * m_acUnitBillboard[1][0];
                m_acTreeBillboard[4] = afScale[1] * m_acUnitBillboard[1][1];
                m_acTreeBillboard[5] = afScale[2] * m_acUnitBillboard[1][2];
                m_acTreeBillboard[6] = afScale[0] * m_acUnitBillboard[2][0];
                m_acTreeBillboard[7] = afScale[1] * m_acUnitBillboard[2][1];
                m_acTreeBillboard[8] = afScale[2] * m_acUnitBillboard[2][2];
                m_acTreeBillboard[9] = afScale[0] * m_acUnitBillboard[3][0];
                m_acTreeBillboard[10] = afScale[1] * m_acUnitBillboard[3][1];
                m_acTreeBillboard[11] = afScale[2] * m_acUnitBillboard[3][2];

                Validate( );
            }
            
            return m_acTreeBillboard;
        }

        void            Invalidate(void)                            { m_bValid = false; }
        bool            IsValid(void) const                         { return m_bValid; }
        void            Validate(void)                              { m_bValid = true; }

private:
        float           m_acTreeBillboard[12];      // used as a buffer for holding the scaled billboard coordinates
        //lint esym(1401, m_acTreeBillboard)  
static  CVec3           m_acUnitBillboard[4];       // unitized quad computed to face the camera - all various sized
                                                    // billboards are scaled from this one billboard
        bool            m_bValid;                   // used billboarding computations to eliminate redundant computations
};
