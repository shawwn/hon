//         Name: IdvRandom.h
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

#define BUILD_IDVRANDOM_SET
#include "../LibGlobals_Source/IdvGlobals.h"

struct SIdvRandomImpl;

///////////////////////////////////////////////////////////////////////  
//  class CIdvRandom declaration
//

class BUILD_IDVRANDOM_SET CIdvRandom 
{
public:
        CIdvRandom( );
        ~CIdvRandom( );

        double              GetNormal(void);
        double              GetNormal(double dMin, double dMax);
        float               GetUniform(float fMin, float fMax);
        void                Reseed(long lSeed = -1L);

        long                GetSeed(void) const;
};
