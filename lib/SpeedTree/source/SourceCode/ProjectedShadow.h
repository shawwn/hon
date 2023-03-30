///////////////////////////////////////////////////////////////////////  
//         Name: ProjectedShadow.h
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
#include "LibVector_Source/IdvFastMath.h"
#include "FileAccess.h"
#include <string>
#include "Debug.h"

// forward references
class CIndexedGeometry;

///////////////////////////////////////////////////////////////////////  
//  class CProjectedShadow declaration

class CProjectedShadow
{
public:
        CProjectedShadow( );

        void            Save(CTreeFileAccess& cFile) const;
        void            Parse(CTreeFileAccess& cFile);

        void            ComputeTexCoords(CIndexedGeometry* pGeometry, CVec3 cCenter, float fRadius, float* pShadowCoords = NULL) const;
        const char*     GetSelfShadowFilename(void)             { return m_strSelfShadowMap.c_str( ); }

private:
        CVec3           ClosestPoint(const CVec3& cStart, const CVec3& cEnd, const CVec3& cVertex) const;

        CVec3           m_cRight;               // the "right" direction of the camera when the shadow image was taken by SpeedTreeCAD
        CVec3           m_cUp;                  // the "up" direction of the camera when the shadow image was taken by SpeedTreeCAD
        CVec3           m_cOut;                 // the "out" direction of the camera when the shadow image was taken by SpeedTreeCAD
        std::string     m_strSelfShadowMap;     // the name of the self-shadow texture map
};
