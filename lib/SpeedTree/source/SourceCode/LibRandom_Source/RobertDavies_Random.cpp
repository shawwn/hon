//         Name: RobertDavies_Random.cpp
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
//  Original random library code was supplied by Robert Davies of Statistics
//  Research Associates, Ltd. of Wellington, New Zealand.  Their corporate
//  URL is http://www.statsresearch.co.nz/
//
//  All modifications to their software is supplied under the terms of a
//  license agreement or nondisclosure agreement with Interactive Data
//  Visualization and may not be copied or disclosed except in accordance with
//  the terms of that agreement.

#include "RobertDavies_Random.h"
#include <cmath>
#if defined(linux) || defined (__APPLE__)
#include <stdexcept>
#endif
using namespace std;


// static variables

double Random::seed;
Real Random::Buffer[128];
Real Normal::Nxi;
Real* Normal::Nsx;
Real* Normal::Nsfx;
long Normal::count = 0;


// utility functions

inline Real     square(Real x)              { return x * x; }
inline ExtReal  square(const ExtReal& x)    { return x * x; }


///////////////////////////////////////////////////////////////////////
//  Random::Raw definition
//

Real Random::Raw( )
{
   long iseed = (long) seed;
   long hi = iseed / 127773L;                 // integer division
   long lo = iseed - hi * 127773L;            // modulo
   iseed = 16807 * lo - 2836 * hi;
   if (iseed <= 0)
       iseed += 2147483647L;
   seed = (double)iseed;

   return float(seed) * 4.656612875e-10f;
}


///////////////////////////////////////////////////////////////////////
//  Random::Density definition
//

Real Random::Density(Real) const
{
    return 0.0;
}


///////////////////////////////////////////////////////////////////////
//  Random::DoNothing definition
//

#ifdef _MSC_VER
static void DoNothing(int)
{
}
#endif


///////////////////////////////////////////////////////////////////////
//  Random::Next definition
//

Real Random::Next( )
{
   if (!seed)
      throw(logic_error("Random number generator not initialised"));

   int i = (int) (Raw( ) * 128);               // 0 <= i < 128
#ifdef _MSC_VER
   DoNothing(i);
   DoNothing(i);
#endif
   Real f = Buffer[i]; Buffer[i] = Raw();  // Microsoft release gets this wrong

   return f;
}


///////////////////////////////////////////////////////////////////////
//  Random::Get definition
//

double Random::Get( )
{
    return seed/2147483648UL;
}


///////////////////////////////////////////////////////////////////////
//  Random::GetSeed definition
//

long Random::GetSeed( )
{
    return static_cast<long>(seed);
}


///////////////////////////////////////////////////////////////////////
//  Random::load definition
//

void Random::load(int*, Real*, Random**)
{
    throw(logic_error("Newran: illegal combination"));
}


///////////////////////////////////////////////////////////////////////
//  Random::Set definition
//

void Random::Set(double s)
{
   if (s >= 1.0 || s <= 0.0)
      throw(logic_error("Newran: seed out of range"));

   seed = (long)(s*2147483648UL);
   for (int i = 0; i < 128; i++)
       Buffer[i] = Raw();
}


///////////////////////////////////////////////////////////////////////
//  Random::SetLong definition
//

void Random::SetLong(long s)
{
    seed = s;
    for (int i = 0; i < 128; i++)
        Buffer[i] = Raw();
}


///////////////////////////////////////////////////////////////////////
//  PosGen::PosGen definition
//

PosGen::PosGen( )
{
   NotReady = true;
   xi = 0.0f;
   sx = NULL;
   sfx = NULL;
   //lint -esym(613,PosGen::sx)
   //lint -esym(613,PosGen::sfx)
}


///////////////////////////////////////////////////////////////////////
//  PosGen::~PosGen definition

PosGen::~PosGen()
{
   if (!NotReady)
   {
        delete[] sx;
        delete[] sfx;
   }
}


///////////////////////////////////////////////////////////////////////
//  PosGen::Build definition

void PosGen::Build(bool sym)
{
    NotReady = false;
    sx = new Real[60];
    sfx = new Real[60];

    Real sxi = 0.0f;
    Real inc = sym ? 0.01f : 0.02f;
    int i(0);
	for (; i < 60; i++)
    {
      sx[i] = sxi;
      Real f1 = Density(sxi);
      sfx[i] = f1;
      if (f1 <= 0.0)
          goto L20;
      sxi += inc / f1;
    }
    throw(runtime_error("Newran: area too large"));

L20:
    if (i < 50)
       throw(runtime_error("Newran: area too small"));
    xi = sym ? 2.0f * i : i;

    return;
}


///////////////////////////////////////////////////////////////////////
//  PosGen::Next definition
//

Real PosGen::Next( )
{
    Real ak,y;
    int ir;

    if (NotReady)
        Build(false);
    do
    {
        Real r1 = Random::Next( );
        ir = (int)(r1 * xi);
        Real sxi = sx[ir];

        ak = sxi + (sx[ir + 1] - sxi) * Random::Next( );
        y = sfx[ir] * Random::Next();
    }
    while (y >= sfx[ir + 1] && y >= Density(ak));

    return ak;
}


///////////////////////////////////////////////////////////////////////
//  SymGen::Next definition

Real SymGen::Next()
{
    Real s,ak,y;
    int ir;

    if (NotReady)
        Build(true);
    do
    {
        s = 1.0;
        Real r1 = Random::Next();
        if (r1 > 0.5)
        {
            s = -1.0f;
            r1 = 1.0f - r1;
        }
        ir = (int)(r1 * xi);
        Real sxi = sx[ir];
        ak = sxi + (sx[ir + 1] - sxi) * Random::Next();
        y = sfx[ir] * Random::Next();
    }
    while (y >= sfx[ir + 1] && y >= Density(ak));

    return s * ak;
}


///////////////////////////////////////////////////////////////////////
//  Normal::Normal definition

Normal::Normal()
{
   //lint -save -e423
   if (count)
   {
       NotReady = false;
       xi = Nxi;
       sx = Nsx;
       sfx = Nsfx;
   }
   else
   {
       Build(true);
       Nxi = xi;
       Nsx = sx;
       Nsfx = sfx;
   }
   count++;
   //lint -restore
}


///////////////////////////////////////////////////////////////////////
//  Normal::~Normal definition
//

Normal::~Normal()
{
   count--;
   if (count)
       NotReady = true;
}

///////////////////////////////////////////////////////////////////////
//  Normal::Density definition
//

Real Normal::Density(Real x) const
{
    return (fabs(x) > 8.0f) ? 0.0f : 0.398942280f * float(exp(-x * x / 2.0f));
}


///////////////////////////////////////////////////////////////////////
//  ln_gamma definition
//

Real ln_gamma(Real xx)
{
   // log gamma function adapted from numerical recipes in C

   if (xx < 1.0)                           // Use reflection formula
   {
      float piz = 3.14159265359f * (1.0f - xx);
      return float(log(piz / float(sin(float(piz)))) - ln_gamma(2.0f - xx));
   }
   else
   {
        static double cof[6] =
        {
            76.18009173,
            -86.50532033,
            24.01409822,
            -1.231739516,
            0.120858003e-2,
            -0.536382e-5
        };

        double x = xx-1.0;
        double tmp = x + 5.5;

        tmp -= (x + 0.5) * log(tmp);
        double ser=1.0;

        for (int j = 0; j <= 5; j++)
        {
            x += 1.0; ser += cof[j] / x;
        }

        return float(-tmp + log(2.50662827465f * ser));
   }
}



// Identification routines for each class - may not work on all compilers?

char* Random::Name()            { return "Random"; }
char* Uniform::Name()           { return "Uniform"; }
char* Normal::Name()            { return "Normal"; }
char* PosGen::Name()            { return "PosGen"; }
char* SymGen::Name()            { return "SymGen"; }


