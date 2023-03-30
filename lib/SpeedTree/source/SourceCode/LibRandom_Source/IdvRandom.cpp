//         Name: IdvRandom.cpp
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

#include "IdvRandom.h"
#include "RobertDavies_Random.h"
#include <ctime>
#if defined(linux)
#include <limits.h>
#endif
using namespace std;


///////////////////////////////////////////////////////////////////////  
//  struct SIdvRandomImpl declaration
//
//  This structure contains the member variables of CIdvRandom in order
//  to hide the implementation details (namely the header file) of the Robert
//  Davies random library.

struct SIdvRandomImpl
{
        SIdvRandomImpl( )
        {
        }

static  Uniform     m_cUniform;         // Davies' uniform number generator
static  Normal      m_cNormal;          // Davies' normal distribution number generator

static  bool        m_bInit;            // Flag for initializing Davies' library
};


// static variables

Uniform SIdvRandomImpl::m_cUniform;
Normal SIdvRandomImpl::m_cNormal;
bool SIdvRandomImpl::m_bInit = false;


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::CIdvRandom definition
//

CIdvRandom::CIdvRandom( )
{
    if (!SIdvRandomImpl::m_bInit)
        Reseed( );
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::~CIdvRandom definition
//

CIdvRandom::~CIdvRandom( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::GetUniform definition
//

float CIdvRandom::GetUniform(float fMin, float fMax)
{
    return fMin + (fMax - fMin) * static_cast<float>(SIdvRandomImpl::m_cUniform.Next( ));
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::GetNormal definition
//

double CIdvRandom::GetNormal(void)
{
    return SIdvRandomImpl::m_cNormal.Next( );
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::GetNormal definition
//

double CIdvRandom::GetNormal(double dMin, double dMax)
{
    double dValue = SIdvRandomImpl::m_cNormal.Next( );
    while (dValue < dMin ||
           dValue > dMax)
        dValue = SIdvRandomImpl::m_cNormal.Next( );

    return dValue;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::GetUniform definition

void CIdvRandom::Reseed(long lSeed)
{
    if (lSeed == -1L) // automatically calc a seed
    {
        long nNewSeed = 12345L;
        if (SIdvRandomImpl::m_bInit)
            nNewSeed = static_cast<long>(GetUniform(0.0f, float(INT_MAX)));
        long nTimeValue = static_cast<long>(time((time_t* ) 0));
//        srand(nNewSeed);
        if (nNewSeed > nTimeValue)
            Random::Set(double(nTimeValue) / nNewSeed);
        else
            Random::Set(double(nNewSeed) / nTimeValue);
    }
    else
    {
        if (lSeed <= 1L)
            lSeed = 1L;
        Random::SetLong(lSeed);
    }

    SIdvRandomImpl::m_bInit = true;
}


///////////////////////////////////////////////////////////////////////  
//  CIdvRandom::GetUniform definition
//

long CIdvRandom::GetSeed(void) const
{
    return Random::GetSeed( );
}

