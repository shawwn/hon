///////////////////////////////////////////////////////////////////////  
//         Name: Endian.h
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


//  Endian-awareness was added in version 1.6.0 in order to port to the Macintosh

#ifdef ST_BIG_ENDIAN
#define EndianSwap32(a) ((((a) & 0xff000000) >> 24) | ((a) & 0x00ff0000) >> 8 | ((a) & 0x0000ff00) << 8 | ((a) & 0x000000ff) << 24)
#define EndianSwap16(a) ((((a) & 0xff00) >> 8) | (((a) & 0x00ff) << 8))
#else
#define EndianSwap32(a) a
#define EndianSwap16(a) a
#endif
