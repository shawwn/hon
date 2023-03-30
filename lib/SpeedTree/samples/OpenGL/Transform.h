//         Name: IdvVector.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//      Copyright (c) 2001-2002 IDV, Inc.
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


///////////////////////////////////////////////////////////////////////  
//  class CTransform declaration
//
//
//  Description:  Summary of public interface
//
//      Construction:
//      -------------
//      CTransform( )                               Builds a transformation matrix.  LoadIdentity( ) is called.
//
//      Variables:
//      ----------
//      m_afData                                    Matrix data storage.  Public for OpenGL convenience and fast access.
//
//      Inspectors:
//      -----------
//      IsIdentity( )                               Returns true if *this represents the identity matrix.
//
//      Mutators:
//      ---------
//      Set(int nRow, int nCol, float fValue)       Sets the element at [nRow][nCol] to fValue.
//      Set(float afData[4][4])                     Sets the entire matrix.
//      Invert( )                                   Inverts *this using lu decomposition.
//      LoadIdentity( )                             Makes *this the identity matrix.
//      Scale(float fFactor)                        Scales *this on the x, y, and z axes by fFactor.
//      Scale(float fX, float fY, float fZ)         Scales *this.
//      Translate(float fX, float fY, float fZ)     Translates *this.
//      Rotate(float fAngle, char chAxis)           Rotates *this fAngle degrees around chAxis ('x', 'X', 'y', etc.)
//      RotateAxis(float fAngle, CVec cAxis)        Rotates *this fAngle degrees around the axis represented by cAxis.  cAxis is normalized.
//      LookAt(const CVec& cEye, const CVec& ...)   Mimics the gluLookAt routine.
//
//      Operators:
//      ----------
//      operator float*( )                          Converts a CTransform to a float*.
//      operator*(const CTransform& cTransform)     Returns a CTransform equal to *this * cTransform.
//      operator*(const CVec& cVec)                 Returns a CVec equal to *this * cVec.
//      operator*(const CRegion& cRegion)           Returns a CRegion where cRegion.m_cMin/m_cMax are multiplied by *this.
//      operator+(const CTransform& cTransform)     Returns a CTransform equal to *this + cTransform.
//      operator-(const CTransform& cTransform)     Returns a CTransform equal to *this - cTransform.
//      operator<<(ostream& cStream, ...)           Outputs *this.

class CTransform
{
public:
        CTransform( );

        // direct access
        void                    Set(int nRow, int nCol, float fValue)       { m_afData[nRow][nCol] = fValue; }
        void                    Set(float afData[4][4]);

        // inspectors
        bool                    IsIdentity( ) const;

        // mutators
        void                    LoadIdentity( );
        void                    Scale(float fFactor);
        void                    Scale(float fX, float fY, float fZ);
        void                    Translate(float fX, float fY, float fZ);
        void                    Rotate(float fAngle, char chAxis);
        CTransform              operator*(const CTransform& cRight) const;

        float                   m_afData[4][4];             // standard 4 x 4 transform matrix
};
