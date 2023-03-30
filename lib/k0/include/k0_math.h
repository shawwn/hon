// (C)2008 S2 Games
// k0_math.h
//
//=============================================================================
#ifndef __K0_MATH_H__
#define __K0_MATH_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include <algorithm>

#include "k0_types.h"

#if defined(linux) || defined(__APPLE__) || defined(__FreeBSD__)
#define _isnan isnan
#define _finite isfinite
#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifndef M_PI
#define M_PI	3.14159265358979323846f
#endif

#ifndef M_E
#define M_E		2.71828182845904523536f
#endif

#ifndef DEG2RAD
#define DEG2RAD(a) ((a) * (M_PI / 180.0f))
#endif
#ifndef RAD2DEG
#define RAD2DEG(a) ((a) * (180.0f / M_PI))
#endif

#define DEGSIN(a) (sin(DEG2RAD(a)))
#define DEGCOS(a) (cos(DEG2RAD(a)))

#define WORD2ANGLE(a) (((float)(a) / 65536) * 360)
#define ANGLE2WORD(a) (word)(((int)((a) * 65536.0 / 360.0)) & 65535)
#define BYTE2ANGLE(a) (((float)(a) / 256) * 360)
#define ANGLE2BYTE(a) (byte)(((int)((a) * 256.0 / 360.0)) & 255)

const float	EPSILON(1.19209e-007f);
const float	QUAT_EPSILON(1.19209e-007f);
//=============================================================================

//=============================================================================
// Templates
//=============================================================================

/*====================
  MIN
  ====================*/
template<class T>
inline
const T& MIN(const T& _Left, const T& _Right)
{
	// return smaller of _Left and _Right
	return (_Right < _Left ? _Right : _Left);
}


/*====================
  MAX
  ====================*/
template<class T>
inline
const T& MAX(const T& _Left, const T& _Right)
{
	// return larger of _Left and _Right
	return (_Left < _Right ? _Right : _Left);
}


/*====================
  SIGN
  ====================*/
template<class T>
inline
int SIGN(T a)
{
	return a < 0 ? -1 : 1;
}


/*====================
  LERP
  ====================*/
template<class T>
inline
T LERP(float f, T low, T hi)
{
	return static_cast<T>(low + (hi - low) * f);
}


/*====================
  ILERP

  Inverse of the lerp function
  ====================*/
template<class T>
inline
float ILERP(T f, T low, T hi)
{
	return float(f - low)/(hi - low);
}


/*====================
  CLAMP
  ====================*/
template<class T>
inline
T CLAMP(T v, T low, T hi)
{
	return v < low ? low : v > hi ? hi : v;
}


/*====================
  CLAMP_LERP
  ====================*/
template<class T>
inline
T CLAMP_LERP(float f, T low, T hi)
{
	return CLAMP(static_cast<T>(low + (hi - low) * f), low, hi);
}


/*====================
  ABS
  ====================*/
template<class T>
inline
T ABS(T x)
{
	return x < 0 ? -x : x;
}


/*====================
  SWAP
  ====================*/
template<class T>
inline
void SWAP(T &a, T &b)
{
	T temp = a;
	a = b;
	b = temp;
}


/*====================
  ROUND
  ====================*/
inline
float ROUND(float f)
{
	return floor(f + 0.5f);
}


/*====================
  SQR
  ====================*/
template<class T>
inline T SQR(T x)
{
	return x * x;
}


/*====================
  FRAC
  ====================*/
inline
float FRAC(float f)
{
	return f - floor(f);
}


/*====================
  PCF
  ====================*/
template<class T>
inline
T PCF(const float *lerps, const T *values)
{
	return LERP(lerps[1], LERP(lerps[0], values[0], values[1]), LERP(lerps[0], values[2], values[3]));
}


/*====================
  POPCOUNT
  
  Bit population count
  http://en.wikipedia.org/wiki/Hamming_weight
  ====================*/
inline uint POPCOUNT(uint x)
{
	x -= (x >> 1) & 0x55555555;						// Put count of each 2 bits into those 2 bits
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);	// Put count of each 4 bits into those 4 bits 
	x = (x + (x >> 4)) & 0x0f0f0f0f;				// Put count of each 8 bits into those 8 bits 
	return (x * 0x01010101) >> 24;					// Returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


/*====================
  LZC
  ====================*/
inline uint LZC(uint x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);

	return ((sizeof(uint) * 8) - POPCOUNT(x));
}


/*====================
  TZC
  ====================*/
inline uint TZC(uint x)
{
	return POPCOUNT((x & uint(-int(x))) - 1);
}


/*====================
  TOGGLE
  ====================*/
template<class T>
inline
T TOGGLE(T a)
{
	return ~a & 0x1;
}

inline int INT_ROUND(float f)				{ return static_cast<int>(f + 0.5f); }
inline int INT_FLOOR(float f)				{ return static_cast<int>(f); }
inline int INT_CEIL(float f)				{ return static_cast<int>(ceil(f)); }
inline bool FLOAT_EQUALS(float a, float b)	{ return fabs(a - b) <= 0.00001f; }
inline byte BYTE_ROUND(float f)				{ return byte(CLAMP(int(floor(f + 0.5f)), 0, 255)); }

inline	void	M_SinCos(float a, float &s, float &c);
inline	int		M_FtoI(float f);

//=============================================================================

#include "c_vec.h"
#include "k0_constants.h"

/*====================
  DECAY
  ====================*/
template<class T>
inline
T DECAY(T start, T target, float fHalfLife, float fTime)
{
	if (fTime == 0.0f)
		return start;
	else if (fHalfLife == 0.0f)
		return target;
	else
		return T(target + (start - target) * exp(-(LN2 / fHalfLife) * fTime));
}


#include "c_boundingbox.h"
#include "c_plane.h"
#include "c_axis.h"
#include "c_rect.h"
#include "c_matrix4x3.h"

//=============================================================================
// Functions
//=============================================================================
EPlaneTest	M_AABBOnPlaneSide(const CBBoxf &bbBox, const CPlane &p);
EPlaneTest	M_OBBOnPlaneSide(const CBBoxf &bbBox, const CVec3f &v3Pos, const CAxis &aAxis, const CPlane &p);
void	M_CalcBoxExtents(const CVec3f &bmin, const CVec3f &bmax, CVec3f &pos, CVec3f &ext);
float	M_RayPlaneIntersect(const CVec3f &v3Origin, const CVec3f &v3Dir, const CPlane &plPlane, CVec3f &v3Result);

void	M_TransformBounds(const CVec3f &bmin, const CVec3f &bmax, const CVec3f &pos, const CAxis &axis, CVec3f &bmin_out, CVec3f &bmax_out);
float	M_GetYawFromForwardVec2(const CVec2f &v2Forward);
void	M_GetAnglesFromForwardVec(const CVec3f &vForward, CVec3f &vAngles);
CVec3f	M_GetAnglesFromForwardVec(const CVec3f &vForward);
CVec3f	M_GetForwardVecFromAngles(const CVec3f &v3Angles);
CVec2f	M_GetForwardVec2FromYaw(float fYaw);

float	M_Random(float fMin, float fMax);
int		M_Random(int iMin, int iMax);
uint	M_Random(uint iMin, uint iMax);

float	M_ClampLerp(float amount, float low, float high);
float	M_LerpAngle(float a, float low, float high);
CVec3f	M_LerpAngles(float a, const CVec3f &v3Low, const CVec3f &v3High);
float	M_ChangeAngle(float fAngleStep, float fStartAngle, float fEndAngle);
float	M_DiffAngle(float fAngle1, float fAngle2);

bool	M_LineBoxIntersect(const CVec3f &vecStart, const CVec3f &vecEnd, const CBBoxf &bbBounds, float &fFraction);

CAxis	M_QuatToAxis(const CVec4f &v4);
CVec4f	M_AxisToQuat(const CAxis &in);
CVec4f	M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To);
void	M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To, CVec4f &v4Result);

CVec3f	M_SlerpDirection(float fLerp, const CVec3f &v3A, const CVec3f &v3B);
float	M_ArcLengthFactor(float fAngle1, float fAngle2);

int		M_NextPowerOfTwo(int x);

CVec3f	M_RandomDirection();
CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle);
CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle0, float fAngle1);
CVec3f	M_RandomPointInSphere();
CVec2f	M_RandomPointInCircle();

float	M_AreaOfTriangle(const CVec3f &v3A, const CVec3f &v3B, const CVec3f &v3C);

uint	M_BlueNoise(const CRectf &rec, int iDepth, vector<CVec2f> &v2Points);

void	M_CalcAxisExtents(const CVec3f &v3Axis, const vector<CVec3f> &v3Points, float &fMin, float &fMax);
void	M_CalcAxisExtents(const CVec3f &v3Axis, const CBBoxf &bbBox, float &fMin, float &fMax);

float	M_LinePointDistance(const CVec3f &v3Origin, const CVec3f &v3Dir, const CVec3f &v3Point);

CVec3f	M_RotatePointAroundAxis(const CVec3f &v3In, const CVec3f &v3Dir, float fAngle);
CVec3f	M_RotatePointAroundLine(const CVec3f &v3In, const CVec3f &v3Start, const CVec3f &v3End, float fAngle);

bool	M_ClipLine(const CPlane &plPlane, CVec3f &p1, CVec3f &p2);

CVec3f	M_SurfaceNormal(const CVec3f &a, const CVec3f &b, const CVec3f &c);

uint	M_GetCRC32(const byte *pBuffer, uint uiSize);

byte	M_GetAngle8(float fAngle);
float	M_GetAngle(byte yAngle8);

CVec3f	M_GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2);

void    M_Init();
//=============================================================================

//=============================================================================
// Inlines
//=============================================================================
#include "math_inlines.h"
//=============================================================================

#endif //__k0_math_H__
