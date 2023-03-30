//         Name: IdvVector.h
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
#define BUILD_VECTOR_SET


#include <iostream>
#include "IdvFastMath.h"


#define VectorSinD(x)       sinf((x) / 57.29578f)
#define VectorCosD(x)       cosf((x) / 57.29578f)
#define VectorSinR(x)       sinf(x)
#define VectorCosR(x)       cosf(x)


///////////////////////////////////////////////////////////////////////  
//  Vector constants
//

const float c_fPi = 3.1415926535897932384626433832795f;
const float c_fHalfPi = c_fPi * 0.5f;
const float c_fQuarterPi = c_fPi * 0.25f;
const float c_fTwoPi = 2.0f * c_fPi;


///////////////////////////////////////////////////////////////////////  
//  Forward references
//

class CTransform;
class CRotTransform;
class CRegion;
class CVec;


///////////////////////////////////////////////////////////////////////  
//  class CVec3 declaration
//
//
//  Description:
//
//      Construction:
//      -------------
//      CVec3( )                                    Builds a vector equal to (0.0f, 0.0f, 0.0f).
//      CVec3(bool bSkipInitialization)             Builds a vector within uninitialized member variables
//      CVec3(float x, y, z)                        Builds a vector with values (x, y, z)
//      CVec3(float x, y)                           Builds a vector with values (x, y, 0.0f);
//
//      Variables:
//      ----------
//      m_afData[3]                                 Vector data storage.  Public for OpenGL convenience and fast access.
//
//      Inspectors:
//      -----------
//      AbsoluteValue( )                            Returns a CVec3 where each m_afData[i] = fabs((*this)[i]).
//      AngleBetween(const CVec3& CVec3)            Returns the angle (in radians) between *this and CVec3 (assumes normalized vectors)
//      Distance(const CVec3& cPoint)               Returns the distance between *this and cPoint.
//      GetSize( )                                  Returns the size of the CVec3.
//      GetTolerance( )                             Returns the current tolerance setting (used in RoughlyEqual( )) for all CVec3s.
//      Magnitude( )                                Computes the magnitude of the CVec3.
//      MaxAxis( )                                  Returns the largest value in the CVec3.
//      Midpoint(const CVec3& cVec)                 Computes the point half way between cVec and *this.
//      MinAxis( )                                  Returns the smallest value in the CVec3.
//      RoughlyEqual(const CVec3& CVec3)            Determines if *this CVec3 is equivalent to CVec3 within the specified tolerance.
//
//      Mutators:
//      ---------
//      Set(...)                                    All forms set the specified data points and leave the others unchanged.
//      SetTolerance(float fTolerance)              Sets the RoughlyEqual tolerance for ALL CVec3s.
//      SetSize(int nSize)                          Sets the size (number of data points considered valid) of *this.
//      Normalize( )                                Makes the CVec3 a unit vector.
//
//      Operators:
//      ----------
//      operator float*( )                          Converts a CVec3 to a float*.
//      operator[](int nIndex)                      Returns a reference to the specified data point (const version allows access for other const member functions).
//      operator<(const CVec3 &CVec3)               Returns true if the x coord of *this is less than that of CVec3.  If they are equal, the test moves to the y coord and then the z.
//      operator<=(const CVec3 &CVec3)              Returns true if the x coord of *this is less than or equal to that of CVec3.
//      operator>(const CVec3 &CVec3)               Returns true if the x coord of *this is greater than that of CVec3.  If they are equal, the test moves to the y coord and then the z.
//      operator>=(const CVec3 &CVec3)              Returns true if the x coord of *this is greater than or equal to that of CVec3.
//      operator+(const CVec3& CVec3)               Returns *this + CVec3.
//      operator+=(const CVec3& CVec3)              Adds CVec3 to *this.
//      operator-(const CVec3& CVec3)               Returns *this - CVec3.
//      operator-=(const CVec3& CVec3)              Subtracts CVec3 from *this.
//      operator-( )                                Returns -(*this).
//      operator^(const CVec3& CVec3)               Computes the DOT product of *this and CVec3.
//      operator*(const CVec3& CVec3)               Computes the CROSS product of *this and CVec3.
//      operator*=(const CVec3& CVec3)              *this equals the cross product of *this and CVec3.
//      operator*(float fValue)                     Returns a CVec3 equal to *this with each data point multiplied by fValue.
//      operator*=(float fValue)                    Each data point of *this is multiplied by fValue.
//      operator/(float fValue)                     Returns a CVec3 equal to *this with each data point divided by fValue.
//      operator/=(float fValue)                    Each data point of *this is divided by fValue.
//      operator==(const CVec3& CVec3)              Returns true when *this and CVec3 are identical.
//      operator!=(const CVec3& CVec3)              Returns true when *this and CVec3 are not identical.
//      operator*(const CTransform& CTrans)         Returns a CVec3 that is equal to *this * CTransform3 (the vector is on the left).
//      operator*(const CRotTransform3& CTrans)     Returns a CVec3 that is equal to *this * CTransform3 (the vector is on the left).
//      operator<<(ostream& cStream, ...)           Outputs *this.

class BUILD_VECTOR_SET CVec3
{
public:
        CVec3( );
        CVec3(float fX, float fY, float fZ);
        CVec3(float fX, float fY);

        //  inspectors
        CVec3       AbsoluteValue( ) const;
        float       AngleBetween(const CVec3& cVec) const;
        CVec3       Cross(const CVec3& cVec) const;
        float       Distance(const CVec3& cPoint) const;
        float       Dot(const CVec3& cVec) const;
        float       GetTolerance( ) const                                   { return m_fTolerance; }
        float       Magnitude( ) const;
        float       MaxAxis( ) const;
        CVec3       Midpoint(const CVec3& cVec) const;
        float       MinAxis( ) const;
        bool        RoughlyEqual(const CVec3& cVec) const;
        CVec        ToVec(void) const;

        //  mutators
        void        Normalize( );
        void        Set(float fX, float fY);
        void        Set(float fX, float fY, float fZ);
        void        SetTolerance(float fTolerance)                          { m_fTolerance = fTolerance; }

        //  operators
                    operator float*( )                                      { return &m_afData[0]; }
                    operator const float*( ) const                          { return &m_afData[0]; }
        float&      operator[](int nIndex)                                  { return m_afData[nIndex]; }
        bool        operator<(const CVec3& cVec)    const;  
        bool        operator<=(const CVec3& cVec) const;
        bool        operator>(const CVec3& cVec)    const;
        bool        operator>=(const CVec3& cVec) const;
        CVec3       operator+(const CVec3& cVec) const;
        CVec3&      operator+=(const CVec3& cVec);
        CVec3       operator-(const CVec3& cVec) const;
        CVec3&      operator-=(const CVec3& cVec);
        CVec3       operator-( ) const;
        float       operator^(const CVec3& cVec) const;
        CVec3       operator*(const CVec3& cVec) const;
        CVec3&      operator*=(const CVec3& cVec);
        CVec3       operator*(float fValue) const;
        CVec3&      operator*=(float fValue);
        CVec3       operator/(float fValue) const;
        CVec3&      operator/=(float fValue);
        bool        operator==(const CVec3& cVec) const;
        bool        operator!=(const CVec3& cVec) const;
        CVec3       operator*(const CTransform& cTransform) const;
        CVec3       operator*(const CRotTransform& cTransform) const;
friend  BUILD_VECTOR_SET std::ostream&  operator<<(std::ostream& cStream, const CVec3& cVec);

        float       m_afData[3];            // store the x, y, & z components of the CVec3

private:
static  float       m_fTolerance;
};



///////////////////////////////////////////////////////////////////////  
//  class CVec declaration
//
//  Description:  Summary of public interface
//
//
//      Construction:
//      -------------
//      CVec( )                                     Builds a vector equal to (0.0f, 0.0f, 0.0f).
//      CVec(int nSize)                             Builds a vector of size nSize with each member set to zero.
//      CVec(float fX, ...)                         All forms build a vector whose size is equal to the number of arguments.  The values are set to the corresponding parameters.
//
//      Variables:
//      ----------
//      m_afData                                    Vector data storage.  Public for OpenGL convenience and fast access.
//
//      Inspectors:
//      -----------
//      GetTolerance( )                             Returns the current tolerance setting (used in RoughlyEqual( )) for all CVecs.
//      GetSize( )                                  Returns the size of the CVec.
//      Magnitude( )                                Computes the magnitude of the CVec.
//      MaxAxis( )                                  Returns the largest value in the CVec.
//      MinAxis( )                                  Returns the smallest value in the CVec.
//      AbsoluteValue( )                            Returns a CVec where each m_afData[i] = fabs((*this)[i]).
//      RoughlyEqual(const CVec& cVec)              Determines if *this CVec is equivalent to cVec within the specified tolerance.
//      Distance(const CVec& cPoint)                Returns the distance between *this and cPoint.
//      AngleBetween(const CVec& cVec)              Returns the angle (in radians) between *this and cVec.
//
//      Mutators:
//      ---------
//      Set(...)                                    All forms set the specified data points and leave the others unchanged.
//      SetTolerance(float fTolerance)              Sets the RoughlyEqual tolerance for ALL CVecs.
//      SetSize(int nSize)                          Sets the size (number of data points considered valid) of *this.
//      Normalize( )                                Makes the CVec a unit vector.
//
//      Operators:
//      ----------
//      operator float*( )                          Converts a CVec to a float*.
//      operator[](int nIndex)                      Returns a reference to the specified data point (const version allows access for other const member functions).
//      operator<(const CVec &cVec)                 Returns true if the x coord of *this is less than that of cVec.  If they are equal, the test moves to the y coord and then the z.
//      operator<=(const CVec &cVec)                Returns true if the x coord of *this is less than or equal to that of cVec.
//      operator>(const CVec &cVec)                 Returns true if the x coord of *this is greater than that of cVec.  If they are equal, the test moves to the y coord and then the z.
//      operator>=(const CVec &cVec)                Returns true if the x coord of *this is greater than or equal to that of cVec.
//      operator+(const CVec& cVec)                 Returns *this + cVec.
//      operator+=(const CVec& cVec)                Adds cVec to *this.
//      operator-(const CVec& cVec)                 Returns *this - cVec.
//      operator-=(const CVec& cVec)                Subtracts cVec from *this.
//      operator-( )                                Returns -(*this).
//      operator^(const CVec& cVec)                 Computes the DOT product of *this and cVec.
//      operator*(const CVec& cVec)                 Computes the CROSS product of *this and cVec.
//      operator*=(const CVec& cVec)                *this equals the cross product of *this and cVec.
//      operator*(float fValue)                     Returns a CVec equal to *this with each data point multiplied by fValue.
//      operator*=(float fValue)                    Each data point of *this is multiplied by fValue.
//      operator/(float fValue)                     Returns a CVec equal to *this with each data point divided by fValue.
//      operator/=(float fValue)                    Each data point of *this is divided by fValue.
//      operator==(const CVec& cVec)                Returns true when *this and cVec are identical.
//      operator!=(const CVec& cVec)                Returns true when *this and cVec are not identical.
//      operator*(const CTransform& cTransform)     Returns a CVec that is equal to *this * cTransform (the vector is on the left).
//      operator<<(ostream& cStream, ...)           Outputs *this.

class BUILD_VECTOR_SET CVec
{
public:
        CVec( );
        CVec(int nSize);
        CVec(float fX, float fY);
        CVec(float fX, float fY, float fZ);
        CVec(float fX, float fY, float fZ, float fW);
        CVec(float fX, float fY, float fZ, float fW, float fV);
        ~CVec( );

        //  inspectors
        float           GetTolerance( ) const                                   { return m_fTolerance; }
        int             GetSize( ) const                                        { return m_nSize; }
        CVec            Midpoint(const CVec& cVec) const                        { return (*this + cVec) * 0.5f; }

        void            SetTolerance(float fTolerance)                          { m_fTolerance = fTolerance; }
        void            SetSize(int nSize)                                      { m_nSize = nSize; }

        float           Magnitude( ) const;
        float           MaxAxis( ) const;
        float           MinAxis( ) const;
        bool            RoughlyEqual(const CVec& cVec) const;
        CVec            AbsoluteValue( ) const;
        float           Distance(const CVec& cPoint) const;
        float           AngleBetween(const CVec& cVec) const;

        //  mutators
        void            Normalize( );
        void            Set(float fX, float fY)                                 { m_afData[0] = fX; m_afData[1] = fY; }
        void            Set(float fX, float fY, float fZ)                       { m_afData[0] = fX; m_afData[1] = fY; m_afData[2] = fZ; }
        void            Set(float fX, float fY, float fZ, float fW)             { m_afData[0] = fX; m_afData[1] = fY; m_afData[2] = fZ; m_afData[3] = fW; }
        void            Set(float fX, float fY, float fZ, float fW, float fV)   { m_afData[0] = fX; m_afData[1] = fY; m_afData[2] = fZ; m_afData[3] = fW; m_afData[4] = fV; }

        //  operators
                        operator float*( )                                      { return &m_afData[0]; }
                        operator const float*( ) const                          { return &m_afData[0]; }
        
        float&          operator[](int nIndex)                                  { return m_afData[nIndex]; }

        bool            operator<(const CVec& cVec) const;  
        bool            operator<=(const CVec& cVec) const;
        bool            operator>(const CVec& cVec) const;
        bool            operator>=(const CVec& cVec) const;
        CVec            operator+(const CVec& cVec) const;
        CVec&           operator+=(const CVec& cVec);
        CVec            operator-(const CVec& cVec) const;
        CVec&           operator-=(const CVec& cVec);
        CVec            operator-( ) const;
        float           operator^(const CVec& cVec) const;
        CVec            operator*(const CVec& cVec) const;
        CVec&           operator*=(const CVec& cVec);
        CVec            operator*(float fValue) const;
        CVec&           operator*=(float fValue);
        CVec            operator/(float fValue) const;
        CVec&           operator/=(float fValue);
        bool            operator==(const CVec& cVec) const;
        bool            operator!=(const CVec& cVec) const;
        CVec            operator*(const CTransform& cTrans) const;
        bool            operator<=(const CRegion& cRegion) const;
        bool            operator<(const CRegion& cRegion) const;
friend  BUILD_VECTOR_SET std::ostream&  operator<<(std::ostream& cStream, const CVec& cVec);

        float           m_afData[5];            // store each component of the vector
private:
        int             m_nSize;    
        static float    m_fTolerance;
};



///////////////////////////////////////////////////////////////////////  
//  class CRotTransform declaration
//
//
//  Description:  Optimized class used for matrix rotations & scales only (3 x 3 instead of 4 x 4)
//
//      Construction:
//      -------------
//      CRotTransform( )                            Builds a transformation matrix.  LoadIdentity( ) is called.
//      CRotTransform(bool)                         Builds a matrix without initialization.
//
//      Variables:
//      ----------
//      m_afData[3][3]                              Matrix data storage.  Public for OpenGL convenience and fast access.
//
//      Mutators:
//      ---------
//      Set(int nRow, int nCol, float fValue)       Sets the element at [nRow][nCol] to fValue.
//      Set(float afData[3][3])                     Sets the entire matrix.
//      LoadIdentity( )                             Makes *this the identity matrix.
//      Scale(float fFactor)                        Scales *this on the x, y, and z axes by fFactor.
//      Scale(float fX, float fY, float fZ)         Scales *this.
//      RotateX(float fAngle)                       Rotates *this fAngle degrees around the X axis (faster than general rotate)
//      RotateY(float fAngle)                       Rotates *this fAngle degrees around the Y axis (faster than general rotate)
//      RotateZ(float fAngle)                       Rotates *this fAngle degrees around the Z axis (faster than general rotate)
//      RotateYZ(float fAngle)                      Rotates *this fAngle degrees around the Y then Z axes (faster than separate calls)
//      RotateAxis(float fAngle, CVec3 cAxis)       Rotates *this fAngle degrees around the axis represented by cAxis.  *cAxis is assumed normalized.
//      RotateAxis(float fAngle, CVec cAxis)        Rotates *this fAngle degrees around the axis represented by cAxis.  *cAxis is assumed normalized.
//
//      Operators:
//      ----------
//      operator float*( )                          Converts a CTransform3 to a float*.
//      operator*(const CRotTransform3& )           Returns a CTransform3 equal to *this * CTransform3.
//      operator*(const CVec3& CVec3)               Returns a CVec3 equal to *this * CVec3.
//      operator*(const CVec& CVec)                 Returns a CVec equal to *this * CVec.
//      operator+(const CRotTransform3& )           Returns a CTransform3 equal to *this + CTransform3.
//      operator-(const CRotTransform3& )           Returns a CTransform3 equal to *this - CTransform3.
//      operator<<(ostream& cStream, ...)           Outputs *this.

class BUILD_VECTOR_SET CRotTransform
{
public:
        // constructors
        CRotTransform( );

        // data gathering/setting
        void                    Set(int nRow, int nCol, float fValue)       { m_afData[nRow][nCol] = fValue; }
        void                    Set(float afData[3][3]);
        void                    LoadIdentity( );

        // standard affine transform functions
        void                    RotateX(float fAngle);
        void                    RotateY(float fAngle);
        void                    RotateZ(float fAngle);
        void                    RotateYZ(float fYangle, float fZangle);
        void                    RotateAxis(float fAngle, CVec3 cAxis);
        void                    RotateAxisFromIdentity(float fAngle, CVec3 cAxis);

        // C++ operator overloads
                                operator float*( )                          { return &m_afData[0][0]; }
                                operator const float*( ) const              { return &m_afData[0][0]; }
        CRotTransform           operator*(const CRotTransform& cTrans) const;
        CVec3                   operator*(const CVec3& cVec) const;
friend  BUILD_VECTOR_SET std::ostream&          operator<<(std::ostream& cStream, const CRotTransform& cTrans);

        float                   m_afData[3][3];             // 3 x 3 rotation matrix
};



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

class BUILD_VECTOR_SET CTransform
{
public:
        enum EInversionCodeType
        {
            OK, SINGULAR
        };

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
        void                    RotateAxis(float fAngle, CVec cAxis);
        void                    LookAt(const CVec& cEye, const CVec& cCenter, const CVec& cUp);

        // operators
                                operator float*( )                          { return &m_afData[0][0]; }
                                operator const float*( ) const              { return &m_afData[0][0]; }
        CTransform              operator*(const CTransform& cTransform) const;
        CVec                    operator*(const CVec& cVec) const;
        CVec3                   operator*(const CVec3& cVec3) const;
        CRegion                 operator*(const CRegion& cRegion) const;
        CTransform              operator+(const CTransform& cTransform) const;
        CTransform              operator-(const CTransform& cTransform) const;
friend  BUILD_VECTOR_SET std::ostream&  operator<<(std::ostream& cStream, const CTransform& cTransform);

        float                   m_afData[4][4];             // standard 4 x 4 transform matrix
};



///////////////////////////////////////////////////////////////////////  
//  class CRegion declaration
//
//
//  Description:  Summary of public interface
//
//
//      Construction:
//      -------------
//      CRegion( )                              Builds a CBox and calls Initialize( ).
//
//      Variables:
//      ----------
//      m_cMin                                  Minimum extents vector.  Public for OpenGL convenience and fast access.
//      m_cMax                                  Maximum extents vector.  Public for OpenGL convenience and fast access.
//
//      Inspectors:
//      -----------
//      Dimension(int nAxis)                    Returns the length of the specified axis (0 = x, 1 = y, 2 = z).
//      MidValue(int nAxis)                     Returns the middle of the specified axis.
//      Midpoint( )                             Returns the midpoint of the extents.
//      LongestExtent( )                        Returns the length of the longest axis.
//      ShortestExtent( )                       Returns the length of the shortest axis.
//      AverageExtent( )                        Returns the average length of all axes.
//      IsSet( )                                Returns true when any member of *this does not match the initialized state.
//
//
//      Mutators:
//      ---------
//      Initialize( )                           Sets m_cMin to (FLT_MAX, FLT_MAX, FLT_MAX) and m_cMax to -m_cMin.  Called automatically by the constructor.
//      Scale(float fScale)                     Scales *this by fScale relative to the origin (NOT the midpoint).
//
//      Operators:
//      ----------
//      operator+(const CRegion& cRegion)       Returns a CRegion exactly large enough to include *this and cRegion.
//      operator+(const CVec& cVec)             Returns a CRegion where cVec is added to m_cMin/m_cMax.
//      operator^(const CVec3& cVec)            Returns a CRegion where min and max include those of cVec
//      operator*(float fScale)                 Returns a CRegion scaled by fScale relative to the midpoint of *this (NOT the origin).
//      operator*(const CTransform& cTransform) Returns a CRegion where m_cMin/m_cMax of *this are multiplied by cTransform.
//      operator<(const CRegion& cRegion)       Returns true when *this is contained within cRegion (using <= and >=).
//      operator<<(ostream& cStream, ...)       Outputs *this.

class BUILD_VECTOR_SET CRegion
{
public:
        CRegion( );

        // inspectors
        float                   Dimension(int nAxis) const          { return m_cMax.m_afData[nAxis] - m_cMin.m_afData[nAxis]; }
        float                   MidValue(int nAxis) const           { return (m_cMax.m_afData[nAxis] + m_cMin.m_afData[nAxis]) * 0.5f; }
        CVec                    Midpoint( ) const                   { return m_cMin.Midpoint(m_cMax); }
        float                   LongestExtent( ) const              { return (((m_cMax - m_cMin).AbsoluteValue( )).MaxAxis( )); }
        float                   ShortestExtent( ) const             { return (((m_cMax - m_cMin).AbsoluteValue( )).MinAxis( )); }
        float                   AverageExtent( ) const;
        bool                    IsSet( ) const;

        // mutators
        void                    Initialize( );
        void                    Scale(float fScale);

        // operators
        CRegion                 operator+(const CRegion& cRegion) const;
        CRegion                 operator+(const CVec& cVec) const;
        CRegion                 operator^(const CVec3& cVec) const;
        CRegion                 operator*(float fScale) const;
        CRegion                 operator*(const CTransform& cTransform) const;
        bool                    operator<(const CRegion& cRegion) const;
friend  BUILD_VECTOR_SET std::ostream&  operator<<(std::ostream& cStream, const CRegion& cRegion);

        CVec                    m_cMin;             // minimum corner of the region (x, y, z)
        CVec                    m_cMax;             // maximum corner of the region (x, y, z)
};


///////////////////////////////////////////////////////////////////////  
//  class CVec3 inlined function definitions
//

// CVec3( )
inline CVec3::CVec3( )
{
    m_afData[0] = m_afData[1] = m_afData[2] = 0.0f;
}

// CVec3(float fX, float fY, float fZ)
inline CVec3::CVec3(float fX, float fY, float fZ)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = fZ;
}

// CVec3(float fX, float fY)
inline CVec3::CVec3(float fX, float fY)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = 0.0f;
}

// AbsoluteValue
inline CVec3 CVec3::AbsoluteValue( ) const
{
    return CVec3(fabsf(m_afData[0]),
                 fabsf(m_afData[1]),
                 fabsf(m_afData[2]));
}

// AngleBetween
inline float CVec3::AngleBetween(const CVec3& cVec) const
{
    float fDot = *this ^ cVec;
    if (fDot < -1.0f)
        fDot = -1.0f;
    if (fDot > 1.0f)
        fDot = 1.0f;
    return acosf(fDot);
}

// Dot
inline float CVec3::Dot(const CVec3& cVec) const
{
    return m_afData[0] * cVec.m_afData[0] +
           m_afData[1] * cVec.m_afData[1] +
           m_afData[2] * cVec.m_afData[2];
}

// Cross
inline CVec3 CVec3::Cross(const CVec3& cVec) const
{
    return CVec3(m_afData[1] * cVec.m_afData[2] - m_afData[2] * cVec.m_afData[1],
                 m_afData[2] * cVec.m_afData[0] - m_afData[0] * cVec.m_afData[2],
                 m_afData[0] * cVec.m_afData[1] - m_afData[1] * cVec.m_afData[0]);
}

// Distance
inline float CVec3::Distance(const CVec3& cVec) const
{
    return _idv_sqrt1(
                ((cVec.m_afData[0] - m_afData[0]) * (cVec.m_afData[0] - m_afData[0])) +
                ((cVec.m_afData[1] - m_afData[1]) * (cVec.m_afData[1] - m_afData[1])) +
                ((cVec.m_afData[2] - m_afData[2]) * (cVec.m_afData[2] - m_afData[2]))
                );
}

// Magnitude
inline float CVec3::Magnitude( ) const
{
    return _idv_sqrt1(m_afData[0] * m_afData[0] + 
                      m_afData[1] * m_afData[1] + 
                      m_afData[2] * m_afData[2]);
}

// MaxAxis
inline float CVec3::MaxAxis( ) const
{
    float fMax(m_afData[0]);

    if (m_afData[1] > fMax)
        fMax = m_afData[1];
    if (m_afData[2] > fMax)
        fMax = m_afData[2];

    return fMax;
}

// Midpoint
inline CVec3 CVec3::Midpoint(const CVec3& cVec) const
{
    return CVec3(0.5f * (cVec.m_afData[0] + m_afData[0]),
                 0.5f * (cVec.m_afData[1] + m_afData[1]),
                 0.5f * (cVec.m_afData[2] + m_afData[2]));
}

// MinAxis
inline float CVec3::MinAxis( ) const
{
    float fMin(m_afData[0]);

    if (m_afData[1] < fMin)
        fMin = m_afData[1];
    if (m_afData[2] < fMin)
        fMin = m_afData[2];

    return fMin;
}

// RoughlyEqual
inline bool CVec3::RoughlyEqual(const CVec3& cVec) const
{
    return ((fabs(m_afData[0] - cVec.m_afData[0]) > m_fTolerance) &&
            (fabs(m_afData[1] - cVec.m_afData[1]) > m_fTolerance) &&
            (fabs(m_afData[2] - cVec.m_afData[2]) > m_fTolerance));
}

// ToVec
inline CVec CVec3::ToVec(void) const
{
    return CVec(m_afData[0], m_afData[1], m_afData[2]);
}

// Normalize
inline void CVec3::Normalize( )
{
    float fSumOfSquares = (m_afData[0] * m_afData[0]) +
                          (m_afData[1] * m_afData[1]) +
                          (m_afData[2] * m_afData[2]);
    float fInvSquareRoot = 1.0f / sqrtf(fSumOfSquares);

    m_afData[0] *= fInvSquareRoot;
    m_afData[1] *= fInvSquareRoot;
    m_afData[2] *= fInvSquareRoot;
}

// Set(float fX, float fY)
inline void CVec3::Set(float fX, float fY)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
}

// Set(float fX, float fY. float fZ)
inline void CVec3::Set(float fX, float fY, float fZ)
{
    m_afData[0] = fX;
    m_afData[1] = fY;
    m_afData[2] = fZ;
}

// operator <
inline bool CVec3::operator<(const CVec3& cVec) const
{
    if (m_afData[0] < cVec.m_afData[0])
    {
        return true;
    }
    else if (m_afData[0] > cVec.m_afData[0])
    {
        return false;
    }
    else
    {
        if (m_afData[1] < cVec.m_afData[1])
            return true;
        else if (m_afData[1] > cVec.m_afData[1])
            return false;
        else
            return m_afData[2] < cVec.m_afData[2];
    }
}

// operator <=
inline bool CVec3::operator<=(const CVec3& cVec) const
{
    return (m_afData[0] <= cVec.m_afData[0]);
}

// operator >
inline bool CVec3::operator>(const CVec3& cVec) const
{
    if (m_afData[0] > cVec.m_afData[0])
    {
        return true;
    }
    else if (m_afData[0] < cVec.m_afData[0])
    {
        return false;
    }
    else
    {
        if (m_afData[1] > cVec.m_afData[1])
            return true;
        else if (m_afData[1] < cVec.m_afData[1])
            return false;
        else
            return m_afData[2] > cVec.m_afData[2];
    }
}

// operator >=
inline bool CVec3::operator>=(const CVec3& cVec) const
{
    return (m_afData[0] >= cVec.m_afData[0]);
}

// operator +
inline CVec3 CVec3::operator+(const CVec3& cVec) const
{
    return CVec3(m_afData[0] + cVec.m_afData[0],
                 m_afData[1] + cVec.m_afData[1],
                 m_afData[2] + cVec.m_afData[2]);
}

// operator +=
inline CVec3& CVec3::operator+=(const CVec3& cVec) 
{
    m_afData[0] += cVec.m_afData[0];
    m_afData[1] += cVec.m_afData[1];
    m_afData[2] += cVec.m_afData[2];

    return *this;
}

// operator -
inline CVec3 CVec3::operator-(const CVec3& cVec) const
{
    return CVec3(m_afData[0] - cVec.m_afData[0],
                 m_afData[1] - cVec.m_afData[1],
                 m_afData[2] - cVec.m_afData[2]);
}

// operator +=
inline CVec3& CVec3::operator-=(const CVec3& cVec) 
{
    m_afData[0] -= cVec.m_afData[0];
    m_afData[1] -= cVec.m_afData[1];
    m_afData[2] -= cVec.m_afData[2];

    return *this;
}

// operator-
inline CVec3 CVec3::operator-( ) const
{
    return CVec3(-m_afData[0],
                 -m_afData[1],
                 -m_afData[2]);
}

// operator^
inline float CVec3::operator^(const CVec3& cVec) const
{
    return m_afData[0] * cVec.m_afData[0] +
           m_afData[1] * cVec.m_afData[1] +
           m_afData[2] * cVec.m_afData[2];
}

// operator*
inline CVec3 CVec3::operator*(const CVec3& cVec) const
{
    return CVec3(m_afData[1] * cVec.m_afData[2] - m_afData[2] * cVec.m_afData[1],
                 m_afData[2] * cVec.m_afData[0] - m_afData[0] * cVec.m_afData[2],
                 m_afData[0] * cVec.m_afData[1] - m_afData[1] * cVec.m_afData[0]);
}

// operator*=
inline CVec3& CVec3::operator*=(const CVec3& cVec)
{
    float x(m_afData[0]), y(m_afData[1]), z(m_afData[2]);

    m_afData[0] = y * cVec.m_afData[2] - z * cVec.m_afData[1];
    m_afData[1] = z * cVec.m_afData[0] - x * cVec.m_afData[2];
    m_afData[2] = x * cVec.m_afData[1] - y * cVec.m_afData[0];

    return *this;
}

// operator*
inline CVec3 CVec3::operator*(float fScale) const
{
    return CVec3(m_afData[0] * fScale,
                 m_afData[1] * fScale,
                 m_afData[2] * fScale);
}

// operator*=
inline CVec3& CVec3::operator*=(float fScale) 
{
    m_afData[0] *= fScale;
    m_afData[1] *= fScale;
    m_afData[2] *= fScale;

    return *this;
}

// operator/
inline CVec3 CVec3::operator/(float fScale) const
{
    return CVec3(m_afData[0] / fScale,
                 m_afData[1] / fScale,
                 m_afData[2] / fScale);
}

// operator/=
inline CVec3& CVec3::operator/=(float fScale) 
{
    m_afData[0] /= fScale;
    m_afData[1] /= fScale;
    m_afData[2] /= fScale;

    return *this;
}

// operator==
inline bool CVec3::operator==(const CVec3& cVec) const
{
    return m_afData[0] == cVec.m_afData[0] &&
           m_afData[1] == cVec.m_afData[1] &&
           m_afData[2] == cVec.m_afData[2];
}

// operator!=
inline bool CVec3::operator!=(const CVec3& cVec) const
{
    return m_afData[0] != cVec.m_afData[0] ||
           m_afData[1] != cVec.m_afData[1] ||
           m_afData[2] != cVec.m_afData[2];
}

// operator*
inline CVec3 CVec3::operator*(const CTransform& cTransform) const
{
    return CVec3(m_afData[0] * cTransform.m_afData[0][0] +
                 m_afData[1] * cTransform.m_afData[1][0] +
                 m_afData[2] * cTransform.m_afData[2][0] +
                               cTransform.m_afData[3][0],
                 m_afData[0] * cTransform.m_afData[0][1] +
                 m_afData[1] * cTransform.m_afData[1][1] +
                 m_afData[2] * cTransform.m_afData[2][1] +
                               cTransform.m_afData[3][1],
                 m_afData[0] * cTransform.m_afData[0][2] +
                 m_afData[1] * cTransform.m_afData[1][2] +
                 m_afData[2] * cTransform.m_afData[2][2] +
                               cTransform.m_afData[3][2]);
}

// operator*
inline CVec3 CVec3::operator*(const CRotTransform& cTransform) const
{
    return CVec3(m_afData[0] * cTransform.m_afData[0][0] +
                 m_afData[1] * cTransform.m_afData[1][0] +
                 m_afData[2] * cTransform.m_afData[2][0],
                 m_afData[0] * cTransform.m_afData[0][1] +
                 m_afData[1] * cTransform.m_afData[1][1] +
                 m_afData[2] * cTransform.m_afData[2][1],
                 m_afData[0] * cTransform.m_afData[0][2] +
                 m_afData[1] * cTransform.m_afData[1][2] +
                 m_afData[2] * cTransform.m_afData[2][2]);
}

// operator<<
inline std::ostream& operator<<(std::ostream& cStream, const CVec3& cVec)
{
    cStream << "(" << cVec.m_afData[0] << " " << cVec.m_afData[1] << " " << cVec.m_afData[2] << ")\n";

    return cStream;
}



///////////////////////////////////////////////////////////////////////  
//  class CRotTransform inlined function definitions
//

// CRotTransform
inline CRotTransform::CRotTransform( )
{
    m_afData[0][0] = 1.0f;
    m_afData[0][1] = 0.0f;
    m_afData[0][2] = 0.0f;
    m_afData[1][0] = 0.0f;
    m_afData[1][1] = 1.0f;
    m_afData[1][2] = 0.0f;
    m_afData[2][0] = 0.0f;
    m_afData[2][1] = 0.0f;
    m_afData[2][2] = 1.0f;
}


// Set
inline void CRotTransform::Set(float afData[3][3])
{
    m_afData[0][0] = afData[0][0]; 
    m_afData[0][1] = afData[0][1];
    m_afData[0][2] = afData[0][2];
    m_afData[1][0] = afData[1][0];
    m_afData[1][1] = afData[1][1];
    m_afData[1][2] = afData[1][2];
    m_afData[2][0] = afData[2][0];
    m_afData[2][1] = afData[2][1];
    m_afData[2][2] = afData[2][2];
}

// LoadIdentity
inline void CRotTransform::LoadIdentity( )
{
    m_afData[0][0] = 1.0f;
    m_afData[0][1] = 0.0f;
    m_afData[0][2] = 0.0f;
    m_afData[1][0] = 0.0f;
    m_afData[1][1] = 1.0f;
    m_afData[1][2] = 0.0f;
    m_afData[2][0] = 0.0f;
    m_afData[2][1] = 0.0f;
    m_afData[2][2] = 1.0f;
}
    

// operator*
inline CRotTransform CRotTransform::operator*(const CRotTransform& cTrans) const
{
    CRotTransform cTemp;

    cTemp.m_afData[0][0] = m_afData[0][0] * cTrans.m_afData[0][0] +
                           m_afData[0][1] * cTrans.m_afData[1][0] +
                           m_afData[0][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[0][1] = m_afData[0][0] * cTrans.m_afData[0][1] +
                           m_afData[0][1] * cTrans.m_afData[1][1] +
                           m_afData[0][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[0][2] = m_afData[0][0] * cTrans.m_afData[0][2] +
                           m_afData[0][1] * cTrans.m_afData[1][2] +
                           m_afData[0][2] * cTrans.m_afData[2][2];
    cTemp.m_afData[1][0] = m_afData[1][0] * cTrans.m_afData[0][0] +
                           m_afData[1][1] * cTrans.m_afData[1][0] +
                           m_afData[1][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[1][1] = m_afData[1][0] * cTrans.m_afData[0][1] +
                           m_afData[1][1] * cTrans.m_afData[1][1] +
                           m_afData[1][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[1][2] = m_afData[1][0] * cTrans.m_afData[0][2] +
                           m_afData[1][1] * cTrans.m_afData[1][2] +
                           m_afData[1][2] * cTrans.m_afData[2][2];
    cTemp.m_afData[2][0] = m_afData[2][0] * cTrans.m_afData[0][0] +
                           m_afData[2][1] * cTrans.m_afData[1][0] +
                           m_afData[2][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[2][1] = m_afData[2][0] * cTrans.m_afData[0][1] +
                           m_afData[2][1] * cTrans.m_afData[1][1] +
                           m_afData[2][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[2][2] = m_afData[2][0] * cTrans.m_afData[0][2] +
                           m_afData[2][1] * cTrans.m_afData[1][2] +
                           m_afData[2][2] * cTrans.m_afData[2][2];

    return cTemp;
}



// operator*
inline CVec3 CRotTransform::operator*(const CVec3& cVec3) const
{
    return CVec3(m_afData[0][0] * cVec3.m_afData[0] +
                 m_afData[0][1] * cVec3.m_afData[1] +
                 m_afData[0][2] * cVec3.m_afData[2],
                 m_afData[1][0] * cVec3.m_afData[0] +
                 m_afData[1][1] * cVec3.m_afData[1] +
                 m_afData[1][2] * cVec3.m_afData[2],
                 m_afData[2][0] * cVec3.m_afData[0] +
                 m_afData[2][1] * cVec3.m_afData[1] +
                 m_afData[2][2] * cVec3.m_afData[2]);
}


// RotateX
inline void CRotTransform::RotateX(float fAngle)
{
    CRotTransform cRotMatrix;

    float fCosine = VectorCosD(fAngle);
    float fSine = VectorSinD(fAngle);

    cRotMatrix.m_afData[0][0] = 1.0f;
    cRotMatrix.m_afData[0][1] = 0.0f;
    cRotMatrix.m_afData[0][2] = 0.0f;
    cRotMatrix.m_afData[1][0] = 0.0f;
    cRotMatrix.m_afData[1][1] = fCosine;
    cRotMatrix.m_afData[1][2] = fSine;
    cRotMatrix.m_afData[2][0] = 0.0f;
    cRotMatrix.m_afData[2][1] = -fSine;
    cRotMatrix.m_afData[2][2] = fCosine;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}


// RotateY
inline void CRotTransform::RotateY(float fAngle)
{
    CRotTransform cRotMatrix;

    float fCosine = VectorCosD(fAngle);
    float fSine = VectorSinD(fAngle);

    cRotMatrix.m_afData[0][0] = fCosine;
    cRotMatrix.m_afData[0][1] = 0.0f;
    cRotMatrix.m_afData[0][2] = -fSine;
    cRotMatrix.m_afData[1][0] = 0.0f;
    cRotMatrix.m_afData[1][1] = 1.0f;
    cRotMatrix.m_afData[1][2] = 0.0f;
    cRotMatrix.m_afData[2][0] = fSine;
    cRotMatrix.m_afData[2][1] = 0.0f;
    cRotMatrix.m_afData[2][2] = fCosine;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}



// RotateZ
inline void CRotTransform::RotateZ(float fAngle)
{
    CRotTransform cRotMatrix;

    float fCosine = VectorCosD(fAngle);
    float fSine = VectorSinD(fAngle);

    cRotMatrix.m_afData[0][0] = fCosine;
    cRotMatrix.m_afData[0][1] = fSine;
    cRotMatrix.m_afData[0][2] = 0.0f;
    cRotMatrix.m_afData[1][0] = -fSine;
    cRotMatrix.m_afData[1][1] = fCosine;
    cRotMatrix.m_afData[1][2] = 0.0f;
    cRotMatrix.m_afData[2][0] = 0.0f;
    cRotMatrix.m_afData[2][1] = 0.0f;
    cRotMatrix.m_afData[2][2] = 1.0f;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}



// RotateYZ
inline void CRotTransform::RotateYZ(float fYangle, float fZangle)
{
    CRotTransform cRotMatrix;

    float fCosineY = VectorCosD(fYangle);
    float fSineY = VectorSinD(fYangle);
    float fCosineZ = VectorCosD(fZangle);
    float fSineZ = VectorSinD(fZangle);

    cRotMatrix.m_afData[0][0] = fCosineY * fCosineZ;
    cRotMatrix.m_afData[0][1] = fCosineY * fSineZ;
    cRotMatrix.m_afData[0][2] = -fSineY;
    cRotMatrix.m_afData[1][0] = -fSineZ;
    cRotMatrix.m_afData[1][1] = fCosineZ;
    cRotMatrix.m_afData[1][2] = 0.0f;
    cRotMatrix.m_afData[2][0] = fSineY * fCosineZ;
    cRotMatrix.m_afData[2][1] = fSineY * fSineZ;
    cRotMatrix.m_afData[2][2] = fCosineY;

    *this = cRotMatrix * *this;
}


// RotateAxis
inline void CRotTransform::RotateAxis(float fAngle, CVec3 cAxis)
{
    CRotTransform cRotMatrix;

    float s = VectorSinD(fAngle);
    float c = VectorCosD(fAngle);
    float t = 1.0f - c;

    float x = cAxis.m_afData[0];
    float y = cAxis.m_afData[1];
    float z = cAxis.m_afData[2];

    cRotMatrix.m_afData[0][0] = t * x * x + c;
    cRotMatrix.m_afData[0][1] = t * x * y + s * z;
    cRotMatrix.m_afData[0][2] = t * x * z - s * y;
    cRotMatrix.m_afData[1][0] = t * x * y - s * z;
    cRotMatrix.m_afData[1][1] = t * y * y + c;
    cRotMatrix.m_afData[1][2] = t * y * z + s * x;
    cRotMatrix.m_afData[2][0] = t * x * z + s * y;
    cRotMatrix.m_afData[2][1] = t * y * z - s * x;
    cRotMatrix.m_afData[2][2] = t * z * z + c;
        
    *this = cRotMatrix * *this;
}   

// RotateAxisFromIdentity
inline void CRotTransform::RotateAxisFromIdentity(float fAngle, CVec3 cAxis)
{
    float s = VectorSinD(fAngle);
    float c = VectorCosD(fAngle);
    float t = 1.0f - c;

    float x = cAxis.m_afData[0];
    float y = cAxis.m_afData[1];
    float z = cAxis.m_afData[2];

    m_afData[0][0] = t * x * x + c;
    m_afData[0][1] = t * x * y + s * z;
    m_afData[0][2] = t * x * z - s * y;
    m_afData[1][0] = t * x * y - s * z;
    m_afData[1][1] = t * y * y + c;
    m_afData[1][2] = t * y * z + s * x;
    m_afData[2][0] = t * x * z + s * y;
    m_afData[2][1] = t * y * z - s * x;
    m_afData[2][2] = t * z * z + c;
}


///////////////////////////////////////////////////////////////////////  
//  Global utility function definitions
//

inline float VecRad2Deg(float fRad)
{
    return fRad * 57.295779513082320876798154814105f;
}

inline float VecDeg2Rad(float fDeg)
{
    return fDeg / 57.295779513082320876798154814105f;
}

inline float VecInterpolate(float fStart, float fEnd, float fPercent)
{
    return fStart + (fEnd - fStart) * fPercent;
}

inline CVec VecInterpolate(const CVec& cStart, const CVec& cEnd, float fPercent)
{
    return cStart + (cEnd - cStart) * fPercent;
}

inline CVec3 VecInterpolate(const CVec3& cStart, const CVec3& cEnd, float fPercent)
{
    return cStart + (cEnd - cStart) * fPercent;
}

inline bool VecRouglyEqual(float fA, float fB, float fTolerance)
{
    return fabs(fA - fB) < fTolerance;
}

