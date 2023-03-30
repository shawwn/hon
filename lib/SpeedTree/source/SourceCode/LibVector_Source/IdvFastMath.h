///////////////////////////////////////////////////////////////////////  
//         Name: IdvFastMath.h
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
#include <cmath>


///////////////////////////////////////////////////////////////////////  
//  _idv_sqrt1 definition

inline float _idv_sqrt1(float fVal)
{
    int i = ((*reinterpret_cast<int*>(&fVal)) >> 1 ) + 0x1fc00000; 

    return *reinterpret_cast<float*>(&i);
}

