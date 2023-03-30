///////////////////////////////////////////////////////////////////////  
//  Simple vector functions
//
//  (c) 2003 IDV, Inc.
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
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

#pragma once

///////////////////////////////////////////////////////////////////////  
//  Include Files

#include <cmath>


///////////////////////////////////////////////////////////////////////  
//  VectorNormalize

inline void VectorNormalize(float* pVector)
{
    float fMag = (float)sqrt(pVector[0] * pVector[0] + pVector[1] * pVector[1] + pVector[2] * pVector[2]);

    pVector[0] /= fMag;
    pVector[1] /= fMag;
    pVector[2] /= fMag;
}


///////////////////////////////////////////////////////////////////////  
//  VectorDistance

inline float VectorDistance(const float* pA, const float* pB)
{
    return sqrtf((pA[0] - pB[0]) * (pA[0] - pB[0]) + 
                 (pA[1] - pB[1]) * (pA[1] - pB[1]) + 
                 (pA[2] - pB[2]) * (pA[2] - pB[2]));
}


///////////////////////////////////////////////////////////////////////  
//  VectorCrossProduct

inline void VectorCrossProduct(float* pResult, const float* pVector1, const float* pVector2)
{
    pResult[0] = pVector1[1] * pVector2[2] - pVector1[2] * pVector2[1],
    pResult[1] = pVector1[2] * pVector2[0] - pVector1[0] * pVector2[2],
    pResult[2] = pVector1[0] * pVector2[1] - pVector1[1] * pVector2[0];
}


///////////////////////////////////////////////////////////////////////  
//  VectorDotProduct

inline float VectorDotProduct(const float* pVector1, const float* pVector2)
{
    return pVector1[0] * pVector2[0] + pVector1[1] * pVector2[1] + pVector1[2] * pVector2[2];
}
