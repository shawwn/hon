//         Name: ExtendedReal.cpp
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

#include "ExtendedReal.h"
using namespace std;

///////////////////////////////////////////////////////////////////////  
//  ExtReal::operator+ definition
//

ExtReal ExtReal::operator+(const ExtReal& er) const
{
   if (c == Finite && er.c == Finite)
       return ExtReal(value+er.value);
   if (c == Missing || er.c == Missing)
       return ExtReal(Missing);
   if (c == Indefinite || er.c == Indefinite)
       return ExtReal(Indefinite);
   if (c == PlusInfinity)
   {
      if (er.c == MinusInfinity)
          return ExtReal(Indefinite);
      return *this;
   }
   if (c == MinusInfinity)
   {
      if (er.c == PlusInfinity)
          return ExtReal(Indefinite);
      return *this;
   }

   return er;
}


///////////////////////////////////////////////////////////////////////  
//  ExtReal::operator- definition
//

ExtReal ExtReal::operator-(const ExtReal& er) const
{
   if (c == Finite && er.c == Finite)
       return ExtReal(value-er.value);
   if (c == Missing || er.c == Missing)
       return ExtReal(Missing);
   if (c == Indefinite || er.c == Indefinite)
       return ExtReal(Indefinite);
   if (c == PlusInfinity)
   {
      if (er.c == PlusInfinity)
          return ExtReal(Indefinite);
      return *this;
   }
   if (c == MinusInfinity)
   {
      if (er.c == MinusInfinity)
          return ExtReal(Indefinite);
      return *this;
   }

   return er;
}


///////////////////////////////////////////////////////////////////////  
//  ExtReal::operator* definition
//

ExtReal ExtReal::operator*(const ExtReal& er) const
{
   if (c == Finite && er.c == Finite)
       return ExtReal(value*er.value);
   if (c == Missing || er.c == Missing)
       return ExtReal(Missing);
   if (c == Indefinite || er.c == Indefinite)
       return ExtReal(Indefinite);
   if (c == Finite)
   {
      if (value == 0.0)
          return ExtReal(Indefinite);
      if (value>0.0)
          return er;

      return (-er);
   }
   if (er.c == Finite)
   {
      if (er.value == 0.0)
          return ExtReal(Indefinite);
      if (er.value>0.0)
          return *this;

      return -(*this);
   }
   if (c == PlusInfinity)
       return er;

   return (-er);
}


///////////////////////////////////////////////////////////////////////  
//  ExtReal::operator- definition
//

ExtReal ExtReal::operator-() const
{
   switch (c)
   {
      case Finite:        return ExtReal(-value);
      case PlusInfinity:  return ExtReal(MinusInfinity);
      case MinusInfinity: return ExtReal(PlusInfinity);
      case Indefinite:    return ExtReal(Indefinite);
      case Missing:       return ExtReal(Missing);
   }

   return 0.0;
}


///////////////////////////////////////////////////////////////////////  
//  ExtReal::operator<< definition
//

ostream& operator<<(ostream& os, const ExtReal& er)
{
   switch (er.c)
   {
      case Finite:        os << er.value;         break;
      case PlusInfinity:  os << "plus-infinity";  break;
      case MinusInfinity: os << "minus-infinity"; break;
      case Indefinite:    os << "indefinite";     break;
      case Missing:       os << "missing";        break;
   }

   return os;
}

