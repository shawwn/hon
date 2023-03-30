//         Name: IdvSpline.h
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

#pragma warning (push,3)
#include <vector>
#pragma warning (pop)
#include "../LibVector_Source/IdvVector.h"

const unsigned int  c_nNumEvalPoints = 500;
const float         c_fEvalPointSpacing = 1.0f / (c_nNumEvalPoints - 1.0f);

///////////////////////////////////////////////////////////////////////  
//  class CIdvBezierSpline declaration

class CIdvBezierSpline
{
public:
        CIdvBezierSpline( );
        CIdvBezierSpline(const std::string& strInput);
        CIdvBezierSpline(const CIdvBezierSpline& cRight);
        ~CIdvBezierSpline( );

        float                   Evaluate(float fPercent) const;
        void                    Parse(const std::string& strInput);
        std::string             Save(void) const;
        float                   ScaledVariance(float fPercent) const;

static  void                    ClearCache(void);

private:
        void                    AddControlPoint(float afPoint[2], float afTangent[2], float fTangentLength);
        void                    CreateEvenlySpacedPoints(unsigned int nNumPoints);
        CVec                    EvaluateRawPoint(float fPercent);
        CVec                    SplineInterpolate(const CVec& c1, const CVec& c2, const CVec& c3, const CVec& c4, float fFraction);

        float                   m_fMin;                         // minimum value returned when spline is evaluated
        float                   m_fMax;                         // maximum value returned when spline is evaluated
        float                   m_fVariance;                    // random amount of variance around true value

        // control points
        std::vector<CVec>       m_vControlPoints;               // x,y pairs of control points
        std::vector<CVec>       m_vControlPointTangents;        // x,y pairs of control point tangents
        std::vector<float>      m_vControlPointTangentLengths;  // control point tangent lengths

        std::vector<CVec>       m_vEvenlySpacedPoints;
        std::vector<CVec>       m_vSplinePoints;
};
