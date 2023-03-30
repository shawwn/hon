//         Name: RobertDavies_Random.h
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

#pragma once
#include "ExtendedReal.h"


//typedef Real (*PDF)(Real);                // probability density function
//Real ln_gamma(Real);                      // log gamma function

// forward references

class RepeatedRandom;
class SelectedRandom;


///////////////////////////////////////////////////////////////////////  
//  class Random declaration
//

class Random
{
public:
friend  class           RandomPermutation;

        Random() { }                            // do nothing
virtual ~Random() { }                           // make destructors virtual

static  void            Set(double s);          // set seed (0 < seed < 1)
static  double          Get( );                 // get seed
static  void            SetLong(long s);        // set seed (0 < seed < 1)
static  long            GetSeed( );             // get seed
virtual Real            Next( );                // get new value
virtual char*           Name( );                // identification
virtual Real            Density(Real) const;    // used by PosGen & Asymgen
virtual ExtReal         Mean( ) const       { return 0.5f; }            // mean of distribution
virtual ExtReal         Variance( ) const   { return 1.0f / 12.0f; }    // variance of distribution
virtual int             nelems( ) const     { return 1; }
virtual void            tDelete( ) { }          // delete components of sum
virtual void            load(int*, Real*, Random**);

private:
static  double          seed;                   // seed
static  Real            Buffer[128];            // for mixing random numbers
static  Real            Raw( );                 // unmixed random numbers
//        void            operator=(const Random&) { }                    // private so can't access
};


///////////////////////////////////////////////////////////////////////  
//  class Uniform declaration
//

class Uniform : public Random
{
public:
        Uniform( ) { }

        char*           Name( );                
        Real            Next( )                 { return Random::Next(); }
        ExtReal         Mean( ) const           { return 0.5f; }
        ExtReal         Variance( ) const       { return 1.0f / 12.0f; }
        Real            Density(Real x) const   { return (x < 0.0f || x > 1.0f) ? 0.0f : 1.0f; }

private:
//        void            operator=(const Uniform&) { }               // private so can't access
};


///////////////////////////////////////////////////////////////////////  
//  class PosGen declaration
//

class PosGen : public Random
{
public:
        PosGen( );                          // constructor
        ~PosGen( );                         // destructor

        char*           Name( );            // identification
        Real            Next( );            // to get a single new value
        ExtReal         Mean( ) const           { return (ExtReal)Missing; }
        ExtReal         Variance( ) const       { return (ExtReal)Missing; }

private:
//        void            operator=(const PosGen&){ }                 // private so can't access

protected:
        void            Build(bool);        // called on first call to Next
        Real            xi, *sx, *sfx;
        bool            NotReady;
};


///////////////////////////////////////////////////////////////////////  
//  class SymGen declaration
//

class SymGen : public PosGen
{
public:
        char*           Name( );            // identification
        Real            Next( );            // to get a single new value

private:
//        void            operator=(const SymGen&) { }                // private so can't access
};


///////////////////////////////////////////////////////////////////////  
//  class Normal declaration
//

class Normal : public SymGen
{
public:
        Normal( );
        ~Normal( );

        char*           Name( );                // identification
        Real            Density(Real) const;    // normal density function
        ExtReal         Mean() const            { return 0.0f; }
        ExtReal         Variance() const        { return 1.0f; }

private:
//        void            operator=(const Normal&) { }                // private so can't access
static  Real            Nxi, *Nsx, *Nsfx;       // so we need initialise only once
static  long            count;                  // assume initialised to 0

};


