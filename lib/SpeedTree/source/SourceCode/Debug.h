///////////////////////////////////////////////////////////////////////  
//         Name: Debug.h
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

//  k2 specific
#ifndef _DEBUG
#ifndef _SECURE_SCL
#define _SECURE_SCL 0
#endif
#endif

#include <iostream>

#pragma warning (disable : 4702)

//  The "st" in st_assert stands for SpeedTree.  If an assertion fails, it is mostly likely an internal
//  error, although user errors can easilty cause assertion failures (passing NULL pointers).

#ifdef _DEBUG
    // assertion macro
    #define st_assert(a) \
        if (!(a))       \
        {                   \
            std::cerr << "Assertion failed\nLine " << __LINE__ << ", file: " << __FILE__ << std::endl; \
            abort( ); \
        }

    // memory leak tool
    #include <crtdbg.h>

    #define track_mem_start \
        _CrtMemState sMemStateStart, sMemStateEnd, sMemStateDiff; \
        _CrtMemCheckpoint(&sMemStateStart)

    #define track_mem_end(id) \
        _CrtMemCheckpoint(&sMemStateEnd); \
        if (_CrtMemDifference(&sMemStateDiff, &sMemStateStart, &sMemStateEnd)) \
        { \
            _RPT1(_CRT_WARN, "\n%s BEGIN memory report\n", id); \
            _CrtMemDumpStatistics(&sMemStateDiff); \
            _RPT1(_CRT_WARN, "%s END memory report\n\n", id); \
        } \
        else \
            _RPT1(_CRT_WARN, "\n%s - no memory leaks\n\n", id)

#else
    //  If in release mode, st_assert is defined as a no op.
    #define st_assert(x)
#endif


//  Most exposed CSpeedTreeRT functions have a try and catch around the code as a precautionary
//  mesure.  This macro is used to catch both standard C++ exceptions (those derived from the
//  expception class) and non-standard exceptions, caught by the catch all (...).

#define SpeedTreeCatch(FuncName) \
    catch (exception& cException) \
    { \
        SetError(IdvFormatString("%s - failed [%s]", FuncName, cException.what( )).c_str( )); \
    }

#define SpeedTreeCatchAll(FuncName) \
    catch (...) \
    { \
        SetError(IdvFormatString("%s - threw an unknown system exception", FuncName).c_str( )); \
    }
