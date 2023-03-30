///////////////////////////////////////////////////////////////////////  
//  Random functions
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

#include <cstdlib>


///////////////////////////////////////////////////////////////////////  
//  GetRandom
//
//  nMin <= (return value) <= nMax

static inline int GetRandom(int nMin, int nMax)
{
    int nDiff = nMax - nMin + 1;

    return nMin + (rand( ) % nDiff);
}


///////////////////////////////////////////////////////////////////////  
//  GetRandom
//
//  fMin <= (return value) <= fMax

static inline float GetRandom(float fMin, float fMax)
{
    float fUnit = float(rand( )) / RAND_MAX;
    float fDiff = fMax - fMin;

    return fMin + fUnit * fDiff;
}

