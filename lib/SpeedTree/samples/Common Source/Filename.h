///////////////////////////////////////////////////////////////////////  
//  CFilename Class
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
#include <string>
using namespace std;

///////////////////////////////////////////////////////////////////////  
//  CFilename Class

class CFilename : public string
{
public:
        CFilename( )                                            { }
        CFilename(const char* pFilename) : string(pFilename)    { }
        CFilename(std::string strFilename) :string(strFilename) { }

        CFilename GetExtension(void);      // if filename is "/idv/code/file.cpp", it returns "cpp"
        CFilename GetPath(void);           // if filename is "/idv/code/file.cpp", it returns "/idv/code"
        CFilename NoExtension(void);       // if filename is "/idv/code/file.cpp", it returns "/idv/code/file"
        CFilename NoPath(void);            // if filename is "/idv/code/file.cpp", it returns "file.cpp"
};


///////////////////////////////////////////////////////////////////////  
//  CFilename::GetExtension

inline CFilename CFilename::GetExtension(void)
{
    string strExtension;

    int nLength = (int)length( );
    for (int i = nLength - 1; i > 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
            strExtension = string(c_str( ) + i + 1);
            break;
        }

    return strExtension;
}


///////////////////////////////////////////////////////////////////////  
//  CFilename::GetPath

inline CFilename CFilename::GetPath(void)
{
    char szPath[1024];

    szPath[0] = '\0';
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' ||
            (*this)[i] == '\\')
        {
			int j;
            for (j = 0; j < i + 1; j++)
                szPath[j] = (*this)[j];
            szPath[j] = '\0';
            break;
        }

    return CFilename(szPath);
}


///////////////////////////////////////////////////////////////////////  
//  CFilename::NoExtension

inline CFilename CFilename::NoExtension(void)
{
    char szExtension[1024];

    strcpy(szExtension, c_str( ));
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0 && (*this)[i] != '/'; i--)
        if ((*this)[i] == '.')
        {
			int j;
            for (j = 0; j < i; j++)
                szExtension[j] = (*this)[j];
            szExtension[j] = '\0';
            break;
        }

    return szExtension;
}


///////////////////////////////////////////////////////////////////////  
//  CFilename::NoPath

inline CFilename CFilename::NoPath(void)
{
    char szPath[1024];

    strcpy(szPath, c_str( ));
    int nLength = (int)length( );
    for (int i = nLength - 1; i >= 0; i--)
        if ((*this)[i] == '/' || (*this)[i] == '\\')
        {
			int j, k;
            for (j = i + 1, k = 0; j < nLength; j++, k++)
                szPath[k] = (*this)[j];
            szPath[k] = '\0';
            break;
        }

    return string(szPath);
}


