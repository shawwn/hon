//         Name: IdvGlobals.h
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
#ifdef WIN32
#pragma warning (disable : 4786 4018 4275 4251)
#endif

#include <new>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#if defined(linux) || defined(__APPLE__)
#include "errno.h"
#endif

//
// IDV exception classes, derived from standard C++ exception classes
//

///////////////////////////////////////////////////////////////////////
//  class IdvFileError definition
//
//  Used when file reads or writes fail.  It differs from a normal
//  runtime_error in that it will append the current system error to
//  the library error passed in.

class IdvFileError : public std::runtime_error
{
public:
        IdvFileError(const std::string& strDetails, bool bAppendSystemError = false) :
            runtime_error(strDetails + " [" + (bAppendSystemError ? (std::string(strerror(errno))) : "") + "]") { }
};


///////////////////////////////////////////////////////////////////////
//  class IdvSystemError definition
//
//  Used whenever a system call fails.  It differs from a normal
//  runtime_error in that it will append the current system error to
//  the library error passed in.

class IdvSystemError : public std::runtime_error
{
public:
        IdvSystemError(const std::string& strDetails, bool bAppendSystemError = false) :
            runtime_error(strDetails + " [" + (bAppendSystemError ? (std::string(strerror(errno))) : "") + "]") { }
};


///////////////////////////////////////////////////////////////////////
//  IDV type definitions

const int c_nIdvMaxStringSize = 1024;
typedef char IdvTmpString[c_nIdvMaxStringSize];



///////////////////////////////////////////////////////////////////////
//  IDV global function definitions

static inline std::string IdvFormatString(const char* pField, ...)
{
    va_list vlArgs;
    IdvTmpString szBuffer;

    va_start(vlArgs, pField);
    (void) vsprintf(szBuffer, pField, vlArgs);

    return szBuffer;
}



