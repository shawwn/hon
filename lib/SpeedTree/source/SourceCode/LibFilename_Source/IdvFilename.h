//         Name: IdvFilename.h
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
#include "../LibGlobals_Source/IdvGlobals.h"
#include <string>
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  class CIdvFilename declaration

//lint -e1509  { basic_string's destructor is not virtual }
class CIdvFilename : public string
{
public:
        CIdvFilename( );
        CIdvFilename(const char* pFilename);
        CIdvFilename(char* pFilename);
        CIdvFilename(string strFilename);

        void            ConvertToBackSlashes(void);         // converts all '/' chars to '\'
        void            ConvertToForwardSlashes(void);      // converts all '\' chars to '/'
        bool            Exists(void) const;                 // does the filename physically exist?
        CIdvFilename    GetExtension(void) const;           // if filename is "/idv/code/file.cpp", it returns "cpp"
        CIdvFilename    GetPath(void) const;                // if filename is "/idv/code/file.cpp", it returns "/idv/code"
        CIdvFilename    NoExtension(void) const;            // if filename is "/idv/code/file.cpp", it returns "/idv/code/file"
        CIdvFilename    NoPath(void) const;                 // if filename is "/idv/code/file.cpp", it returns "file.cpp"
};


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::CIdvFilename definition
//

inline CIdvFilename::CIdvFilename( )
{

}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::CIdvFilename definition
//

inline CIdvFilename::CIdvFilename(const char* pFilename) : string(pFilename)
{

}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::CIdvFilename definition
//

inline CIdvFilename::CIdvFilename(char* pFilename) : string(pFilename)
{

}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::CIdvFilename definition
//

inline CIdvFilename::CIdvFilename(string strFilename) : string(strFilename)
{

}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::ConvertToBackSlashes definition
//

inline void CIdvFilename::ConvertToBackSlashes(void)
{
    int nSize = length( );
    for (int i = 0; i < nSize; ++i)
        if ((*this)[i] == '/')
            (*this)[i] = '\\';
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::ConvertToForwardSlashes definition
//

inline void CIdvFilename::ConvertToForwardSlashes(void)
{
    int nSize = length( );
    for (int i = 0; i < nSize; ++i)
        if ((*this)[i] == '\\')
            (*this)[i] = '/';
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::Exists definition
//

inline bool CIdvFilename::Exists(void) const
{
    FILE* pFile = fopen(c_str( ), "r");
    bool bExists = (pFile != NULL);
    if (pFile)
        fclose(pFile);

    return bExists;
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::GetExtension definition
//

inline CIdvFilename CIdvFilename::GetExtension(void) const
{
    string strExtension;

    int nLength = length( );
    for (int i = nLength - 1; i > 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
            strExtension = string(c_str( ) + i + 1);
            break;
        }

    return strExtension;
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::GetPath definition
//

inline CIdvFilename CIdvFilename::GetPath(void) const
{
    IdvTmpString szPath;

    szPath[0] = '\0';
    int nLength = length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' ||
            (*this)[i] == '\\')
        {
			int j(0);
            for (; j < i + 1; j++)
                szPath[j] = (*this)[j];
            szPath[j] = '\0';
            break;
        }

    return CIdvFilename(szPath);
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::NoExtension definition
//

inline CIdvFilename CIdvFilename::NoExtension(void) const
{
    IdvTmpString szExtension;

    strcpy(szExtension, c_str( ));
    int nLength = length( );
    for (int i = nLength - 1; i >= 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
			int j(0);
            for (; j < i; j++)
                szExtension[j] = (*this)[j];
            szExtension[j] = '\0';
            break;
        }

    return szExtension;
}


///////////////////////////////////////////////////////////////////////  
//  function CIdvFilename::NoExtension definition
//

inline CIdvFilename CIdvFilename::NoPath(void) const
{
    IdvTmpString szPath;

    strcpy(szPath, c_str( ));
    int nLength = length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' || (*this)[i] == '\\')
        {
			int k(0);
            for (int j = i + 1; j < nLength; j++, k++)
                szPath[k] = (*this)[j];
            szPath[k] = '\0';
            break;
        }

    return string(szPath);
}


