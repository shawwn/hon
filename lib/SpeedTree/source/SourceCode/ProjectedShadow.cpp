///////////////////////////////////////////////////////////////////////  
//         Name: ProjectedShadow.cpp
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

#pragma warning (disable : 4786)
#include "Debug.h"
#include "ProjectedShadow.h"
#include "LibGlobals_Source/IdvGlobals.h"
#include "LibFilename_Source/IdvFilename.h"
#include "IndexedGeometry.h"
#include "UpVector.h"


///////////////////////////////////////////////////////////////////////  
//  CProjectedShadow::CProjectedShadow definition

CProjectedShadow::CProjectedShadow( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CProjectedShadow::Save definition
//

void CProjectedShadow::Save(CTreeFileAccess& cFile) const
{
    cFile.SaveToken(File_BeginShadowProjectionInfo);

    cFile.SaveToken(File_ShadowRightVector);
    cFile.SaveVector3(m_cRight);

    cFile.SaveToken(File_ShadowUpVector);
    cFile.SaveVector3(m_cUp);

    cFile.SaveToken(File_ShadowOutVector);
    cFile.SaveVector3(m_cOut);

    cFile.SaveToken(File_ShadowMap);
    cFile.SaveString(m_strSelfShadowMap);

    cFile.SaveToken(File_EndShadowProjectionInfo);
}


///////////////////////////////////////////////////////////////////////  
//  CProjectedShadow::Parse definition

void CProjectedShadow::Parse(CTreeFileAccess& cFile)
{
    int nToken = cFile.ParseToken( );
    do
    {
        switch (nToken)
        {
        case File_ShadowRightVector:
            Assign3d(m_cRight, cFile.ParseVector3( ));
            //m_cRight = cFile.ParseVector3( );
            break;
        case File_ShadowUpVector:
            Assign3d(m_cUp, cFile.ParseVector3( ));
            //m_cUp = cFile.ParseVector3( );
            break;
        case File_ShadowOutVector:
            Assign3d(m_cOut, cFile.ParseVector3( ));
            //m_cOut = cFile.ParseVector3( );
            break;
        case File_ShadowMap:
            {
                CIdvFilename cFilename = cFile.ParseString( );
                m_strSelfShadowMap = cFilename.NoPath( );
            }
            break;
        default:
            throw(IdvFileError(IdvFormatString("malformed frond info (token %d)", nToken)));
        }
        nToken = cFile.ParseToken( );
    } while (nToken != File_EndShadowProjectionInfo);
}


///////////////////////////////////////////////////////////////////////  
//  CProjectedShadow::ComputeTexCoords definition

void CProjectedShadow::ComputeTexCoords(CIndexedGeometry* pGeometry, CVec3 cCenter, float fRadius, float* pShadowCoords) const
{
    // compute axis end points
    CVec3 cStartS = cCenter - m_cRight * fRadius, cEndS = cCenter + m_cRight * fRadius;
    CVec3 cStartT = cCenter - m_cUp * fRadius, cEndT = cCenter + m_cUp * fRadius;

    // compute the tex coords
    pGeometry->ResetVertexCounter( );
    for (unsigned short i = 0; i < pGeometry->GetVertexCount( ); ++i)
    {
        float pTexCoords[2];

        // get the vertex
        //lint -save -e661 -e662
        const float* pVertex = pGeometry->GetVertexCoord(i);
        CVec3 cVertex(pVertex[0], pVertex[1], pVertex[2]);
        //line -restore

        // compute the s axis perpendicular bisector and s-coord
        CVec3 cBisectorS = ClosestPoint(cStartS, cEndS, cVertex);
        pTexCoords[0] = cStartS.Distance(cBisectorS) / (fRadius * 2.0f);
        if (pShadowCoords)
            pTexCoords[0] = VecInterpolate(pShadowCoords[2], pShadowCoords[0], pTexCoords[0]);

        // compute the t axis perpendicular bisector and t-coord
        CVec3 cBisectorT = ClosestPoint(cStartT, cEndT, cVertex);
        pTexCoords[1] = cStartT.Distance(cBisectorT) / (fRadius * 2.0f);
        if (pShadowCoords)
            pTexCoords[1] = VecInterpolate(pShadowCoords[5], pShadowCoords[1], pTexCoords[1]);

        // add the verex coord to the geometry
        pGeometry->AddVertexTexCoord1(pTexCoords);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CProjectedShadow::ClosestPoint definition

CVec3 CProjectedShadow::ClosestPoint(const CVec3& cStart, const CVec3& cEnd, const CVec3& cVertex) const
{
    if (cStart == cEnd)
        return cStart;

    CVec3 cDirection = cEnd - cStart;
    cDirection.Normalize( );
    CVec3 cTmp = cVertex - cStart;
    const float fT = cTmp.Dot(cDirection);

    return cStart + cDirection * fT;
}

