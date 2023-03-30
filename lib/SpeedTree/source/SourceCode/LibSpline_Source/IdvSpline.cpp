//         Name: IdvSpline.cpp
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
#include "../Debug.h"
#include "IdvSpline.h"
#include "../LibRandom_Source/IdvRandom.h"
#include <map>
#include <algorithm>

#if defined(linux) || defined (__APPLE__)
template <class T>
inline T __max(T a, T b)
{
	return max(a, b);
}

template <class T>
inline T __min(T a, T b)
{
	return min(a, b);
}
#endif

using namespace std;
static map<string, CIdvBezierSpline*> g_mSplineCache;


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::CIdvBezierSpline

CIdvBezierSpline::CIdvBezierSpline( ) :
    m_fMin(0.0f),
    m_fMax(1.0f),
    m_fVariance(0.0f)
{
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::CIdvBezierSpline

CIdvBezierSpline::CIdvBezierSpline(const string& strInput)
{
    CIdvBezierSpline* pLookup = g_mSplineCache[strInput];
    if (!pLookup)
    {
        Parse(strInput);
        CreateEvenlySpacedPoints(c_nNumEvalPoints);

        CIdvBezierSpline* pCachedCopy = new CIdvBezierSpline(*this);
        g_mSplineCache[strInput] = pCachedCopy;
    }
    else
        *this = *pLookup;
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::CIdvBezierSpline

CIdvBezierSpline::CIdvBezierSpline(const CIdvBezierSpline& cRight)
{
    *this = cRight;
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::CIdvBezierSpline

CIdvBezierSpline::~CIdvBezierSpline( )
{
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::ClearCache

void CIdvBezierSpline::ClearCache(void)
{
    for (map<string, CIdvBezierSpline*>::iterator i = g_mSplineCache.begin( ); i != g_mSplineCache.end( ); ++i)
    {
        delete i->second;
        i->second = NULL;
    }

    g_mSplineCache.clear( );
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::Evaluate

float CIdvBezierSpline::Evaluate(float fPercent) const
{
    float fValue = 0.0f;

    if (m_vEvenlySpacedPoints.size( ) == c_nNumEvalPoints)
    {
        // new way, interpolated
        int nClosest = int(fPercent * float(c_nNumEvalPoints - 1));
        if (nClosest == c_nNumEvalPoints - 1)
        {
            fValue = m_vEvenlySpacedPoints[nClosest][1];
        }
        else
        {
            float fInterpPercent = (fPercent - (nClosest * c_fEvalPointSpacing)) / c_fEvalPointSpacing;
            fValue = VecInterpolate(m_vEvenlySpacedPoints[nClosest][1], m_vEvenlySpacedPoints[nClosest + 1][1], fInterpPercent);
        }

        fValue = fValue * (m_fMax - m_fMin) + m_fMin;

        static CIdvRandom cRandom;
        fValue += static_cast<float>(cRandom.GetUniform(-m_fVariance, m_fVariance));
    }

    return fValue;
}


///////////////////////////////////////////////////////////////////////
//  NextToken

static const char* NextToken(const char* pInput, char* pToken)
{
    while (isspace(*pInput))
        pInput++;
    sscanf(pInput, "%s", pToken);

    return pInput + strlen(pToken);
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::Parse

void CIdvBezierSpline::Parse(const string& strInput)
{
    const char* pInput = strInput.c_str( );

    char szToken[256];
    pInput = NextToken(pInput, szToken);

    if (strcmp(szToken, "BezierSpline") == 0)
    {
        // get min, max, variance
        pInput = NextToken(pInput, szToken);
        m_fMin = static_cast<float>(atof(szToken));
        pInput = NextToken(pInput, szToken);
        m_fMax = static_cast<float>(atof(szToken));
        pInput = NextToken(pInput, szToken);
        m_fVariance = static_cast<float>(atof(szToken));

        // match '{'
        pInput = NextToken(pInput, szToken);
        if (szToken[0] == '{')
        {
            // for each control point, get x,y control point, x,y tangent, and
            // tangent length
            pInput = NextToken(pInput, szToken);
            int nNumControlPoints = atoi(szToken);
            for (int i = 0; i < nNumControlPoints; ++i)
            {
                float afControlPoint[2];
                pInput = NextToken(pInput, szToken);
                afControlPoint[0] = static_cast<float>(atof(szToken));
                pInput = NextToken(pInput, szToken);
                afControlPoint[1] = static_cast<float>(atof(szToken));

                float afTangent[2];
                pInput = NextToken(pInput, szToken);
                afTangent[0] = static_cast<float>(atof(szToken));
                pInput = NextToken(pInput, szToken);
                afTangent[1] = static_cast<float>(atof(szToken));

                float fTangentLength;
                pInput = NextToken(pInput, szToken);
                fTangentLength = static_cast<float>(atof(szToken));

                AddControlPoint(afControlPoint, afTangent, fTangentLength);
            }
            // the last token should be '}', but we ignore it
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::Save

string CIdvBezierSpline::Save(void) const
{
    string strOutput;

    strOutput = "BezierSpline ";
    strOutput += IdvFormatString("%g %g %g\n", m_fMin, m_fMax, m_fVariance);
    strOutput += "{\n";
        strOutput += IdvFormatString("\t%d\n", m_vControlPoints.size( ));
        for (unsigned int i = 0; i < m_vControlPoints.size( ); ++i)
            strOutput += IdvFormatString("\t%g %g %g %g %g\n", m_vControlPoints[i][0], m_vControlPoints[i][1], m_vControlPointTangents[i][0], m_vControlPointTangents[i][1], m_vControlPointTangentLengths[i]);
        strOutput += "\n";
    strOutput += "}\n";

    return strOutput;
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::ScaledVariance

float CIdvBezierSpline::ScaledVariance(float fPercent) const
{
    float fValue = 0.0f;

    if (m_vEvenlySpacedPoints.size( ) == c_nNumEvalPoints)
    {
        int nClosest = int(fPercent * float(c_nNumEvalPoints - 1) + 0.5f);
        fValue = m_vEvenlySpacedPoints[nClosest][1];

        static CIdvRandom cRandom;
        fValue = static_cast<float>(cRandom.GetUniform(-m_fVariance * fValue, m_fVariance * fValue));
    }

    return fValue;
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::AddControlPoint

void CIdvBezierSpline::AddControlPoint(float afPoint[2], float afTangent[2], float fTangentLength)
{
    CVec cPoint(afPoint[0], afPoint[1]);
    CVec cTangent(afTangent[0], afTangent[1]);
    cTangent.Normalize( );

    if (!m_vControlPoints.empty( ))
    {
        unsigned int nLastEntry = m_vControlPoints.size( ) - 1;
        m_vSplinePoints.push_back(m_vControlPoints[nLastEntry] + (m_vControlPointTangents[nLastEntry] * m_vControlPointTangentLengths[nLastEntry]));
        m_vSplinePoints.push_back(cPoint - (cTangent * fTangentLength));
    }

    m_vControlPoints.push_back(cPoint);
    m_vSplinePoints.push_back(cPoint);
    m_vControlPointTangents.push_back(cTangent);
    m_vControlPointTangentLengths.push_back(fTangentLength);
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::CreateEvenlySpacedPoints

void CIdvBezierSpline::CreateEvenlySpacedPoints(unsigned int nNumPoints)
{
    vector<CVec> vRawPoints;
    vRawPoints.resize(nNumPoints);

    for (unsigned int i = 0; i < nNumPoints; ++i)
        vRawPoints[i] = EvaluateRawPoint(float(i) / float(nNumPoints));

    m_vEvenlySpacedPoints.clear( );
    m_vEvenlySpacedPoints.resize(nNumPoints);
    unsigned int nIndex = 0;
    m_vEvenlySpacedPoints[0] = m_vControlPoints[0];
    //m_vEvenlySpacedPoints.push_back(m_vControlPoints[0]);
    for (unsigned int i = 1; i < nNumPoints - 1; ++i)
    {
        float fPercent = float(i) / float(nNumPoints);
        for (unsigned int j = nIndex; j < nNumPoints - 1; ++j)
        {
            if (fPercent >= vRawPoints[j][0] &&
                fPercent < vRawPoints[j + 1][0])
            {
                nIndex = j;
                break;
            }
        }
        CVec cOut(fPercent, 0.0f, 0.0f, 0.0f, 0.0f);
        fPercent = (fPercent - vRawPoints[nIndex][0]) / (vRawPoints[nIndex + 1][0] - vRawPoints[nIndex][0]);
        cOut[1] = VecInterpolate(vRawPoints[nIndex][1], vRawPoints[nIndex + 1][1], fPercent);
        //m_vEvenlySpacedPoints.push_back(cOut);
        m_vEvenlySpacedPoints[i] = cOut;
    }
    //m_vEvenlySpacedPoints.push_back(m_vControlPoints[m_vControlPoints.size( ) - 1]);
    m_vEvenlySpacedPoints[nNumPoints - 1] = m_vControlPoints[m_vControlPoints.size( ) - 1];
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::EvaluateRawPoint

inline CVec CIdvBezierSpline::EvaluateRawPoint(float fPercent)
{
    CVec cValue(2);

    int nSize = m_vControlPoints.size( );
    if (nSize > 1)
    {
        // clip domain
        fPercent = __max(0.0f, fPercent);
        fPercent = __min(1.0f, fPercent);

        // scale percent to span number of control points
        fPercent *= nSize - 1;
        // fractional amount between control points
        float fFraction = fPercent - float(int(fPercent));

        bool bDetermined = false;
        for (int i = 0; i < nSize && !bDetermined; ++i)
        {
            if (i == nSize - 1)
            {
                cValue = m_vControlPoints[i];
                bDetermined = true;
            }
            else if (fPercent >= float(i) && fPercent < float(i + 1))
            {
                cValue = SplineInterpolate(m_vSplinePoints[i * 3],
                                           m_vSplinePoints[i * 3 + 1],
                                           m_vSplinePoints[i * 3 + 2],
                                           m_vSplinePoints[i * 3 + 3],
                                           fFraction);
                bDetermined = true;
            }
        }
    }

    return cValue;
}


///////////////////////////////////////////////////////////////////////
//  CIdvBezierSpline::SplineInterpolate

inline CVec CIdvBezierSpline::SplineInterpolate(const CVec& cVec1, const CVec& cVec2, const CVec& cVec3, const CVec& cVec4, float x)
{
    float c12[2] = { VecInterpolate(cVec1[0], cVec2[0], x), VecInterpolate(cVec1[1], cVec2[1], x) };
    float c23[2] = { VecInterpolate(cVec2[0], cVec3[0], x), VecInterpolate(cVec2[1], cVec3[1], x) };
    float c34[2] = { VecInterpolate(cVec3[0], cVec4[0], x), VecInterpolate(cVec3[1], cVec4[1], x) };
    float c1223[2] = { VecInterpolate(c12[0], c23[0], x), VecInterpolate(c12[1], c23[1], x) };
    float c2334[2] = { VecInterpolate(c23[0], c34[0], x), VecInterpolate(c23[1], c34[1], x) };

    return CVec(VecInterpolate(c1223[0], c2334[0], x), VecInterpolate(c1223[1], c2334[1], x));
}
