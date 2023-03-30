//         Name: RotTransform.cpp
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

#include "IdvVector.h"
using namespace std;
    

///////////////////////////////////////////////////////////////////////  
//  CRotTransform::operator<< definition

ostream &operator<<(ostream& cStream, const CRotTransform& cRotTransform)
{
    cStream << '[';
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            cStream << cRotTransform.m_afData[i][j];
            if (i == 2 && j == 2)
                continue;
            cStream << ' ';
        }
    cStream << ']';

    return cStream;
}
    
