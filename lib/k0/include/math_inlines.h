// (C)2008 S2 Games
// math_inlines.h
//
//=============================================================================
#ifndef __MATH_INLINES_H__
#define __MATH_INLINES_H__

/*====================
  M_EulerToQuat

  uses YXZ euler angle ordering convention
  this corresponds to roll, then pitch, then yaw
  this is a RIGHT HANDED coordinate system
  ====================*/
inline
CVec4f	M_EulerToQuat(float fPitch, float fRoll, float fYaw)
{
	fRoll = DEG2RAD(fRoll) * 0.5f;
	float sr(sin(fRoll));
	float cr(cos(fRoll));
	
	fPitch = DEG2RAD(fPitch) * 0.5f;
	float sp(sin(fPitch));
	float cp(cos(fPitch));
	
	fYaw = DEG2RAD(fYaw) * 0.5f;
	float sy(sin(fYaw));
	float cy(cos(fYaw));

	return CVec4f
	(
		cy * sp * cr - sy * cp * sr,
		cy * cp * sr + sy * sp * cr,
		cy * sp * sr + sy * cp * cr,
		cy * cp * cr - sy * sp * sr
	);
}

inline
CVec4f	M_EulerToQuat(const CVec3f &v3)
{
	float fRoll(DEG2RAD(v3[ROLL]) * 0.5f);
	float sr(sin(fRoll));
	float cr(cos(fRoll));
	
	float fPitch(DEG2RAD(v3[PITCH]) * 0.5f);
	float sp(sin(fPitch));
	float cp(cos(fPitch));
	
	float fYaw(DEG2RAD(v3[YAW]) * 0.5f);
	float sy(sin(fYaw));
	float cy(cos(fYaw));

	return CVec4f
	(
		cy * sp * cr - sy * cp * sr,
		cy * cp * sr + sy * sp * cr,
		cy * sp * sr + sy * cp * cr,
		cy * cp * cr - sy * sp * sr
	);
}


/*====================
  M_SurfaceNormal
  ====================*/
inline
CVec3f	M_SurfaceNormal(const CVec3f &a, const CVec3f &b, const CVec3f &c)
{
	return Normalize(CrossProduct(b - a, c - a));
}


/*====================
  M_PointOnLine
  ====================*/
inline
CVec3f	M_PointOnLine(const CVec3f &origin, const CVec3f &dir, float t)
{
	return origin + dir * t;
}


/*====================
  M_DirectionFromSphericalCoords
  ====================*/
inline
CVec3f	M_DirectionFromSphericalCoords(float fPhi, float fTheta)
{
	// Convert to directional vector
	return CVec3f(cos(fTheta) * sin(fPhi), sin(fTheta) * sin(fPhi), cos(fPhi));
}


/*====================
  M_IsPow2
  ====================*/
inline
bool	M_IsPow2(int iNum)
{
	for (int i = 0; i < 32; ++i)
		if (iNum == BIT(i)) return true;

	return false;
}


/*====================
  M_CeilPow2
  ====================*/
inline
int		M_CeilPow2(int iNum)
{
	int i = 1;

	while (i < iNum)
		i <<= 1;
	return i;
}


/*====================
  M_FloorPow2
  ====================*/
inline
int		M_FloorPow2(int iNum)
{
	int i = 1;

	while (i <= iNum && i)
		i <<= 1;

	return i >> 1;
}


/*====================
  M_Power
  ====================*/
inline
int		M_Power(int iBase, int iExp)
{
	int x = 1;

	for (int i = 0; i < iExp; ++i)
		x *= iBase;

	return x;
}


/*====================
  M_Log2
  ====================*/
inline
int		M_Log2(int iX)
{
	int iLog(0);

	while (iX > 1)
	{
		iX >>= 1;
		++iLog;
	}
	
	return iLog;
}


/*====================
  M_TransformPlaneInverse
  ====================*/
inline
void	M_TransformPlaneInverse(const CPlane &plane, const CVec3f &pos, const CAxis &axis, CPlane &out)
{
	// get a point on the plane and transform it
	CVec3f point = TransformPointInverse(plane.v3Normal * plane.fDist, pos, axis);

	// rotate normal
	out.v3Normal = TransformPointInverse(plane.v3Normal, V_ZERO, axis);

	// recalc plane distance using new normal and point
	out.fDist = DotProduct(point, out.v3Normal);
}


/*====================
  M_PolarToCartesian

  x = theta (counterclockwise angle on xy plane from x axis)
  y = phi (counterclockwise angle on yz plane from y axis)
  z = r
  ====================*/
inline
CVec3f	M_PolarToCartesian(const CVec3f &p)
{
	return CVec3f(cos(p.x) * sin(p.y) * p.z, sin(p.x) * sin(p.y) * p.z, cos(p.y) * p.z);
}


/*====================
  M_CartesianToPolar
  ====================*/
inline
CVec3f	M_CartesianToPolar(const CVec3f &p)
{
	float fR(p.Length());
	float fTheta(atan2(p.y, p.x));
	float fPhi(acos(p.z/fR));

	return CVec3f(fTheta, fPhi, fR);
}


/*====================
  M_CylindricalToCartesian
  ====================*/
inline
CVec3f	M_CylindricalToCartesian(const CVec3f &p)
{
	return CVec3f(cos(p.x) * p.z, sin(p.x) * p.z, p.y);
}


/*====================
  M_SmoothStep
  ====================*/
inline
float	M_SmoothStep(float fValue, float fLow, float fHigh)
{
   if (fValue <= fLow)
      return 0.0f;

   if (fValue >= fHigh)
      return 1.0f;

   // Scale / bias into [0..1] range
   fValue = (fValue - fLow) / (fHigh - fLow);

   return fValue * fValue * (3.0f - 2.0f * fValue);
}


/*====================
  M_SmoothStepN

  Smoothstep between [0..1]
  ====================*/
inline
float	M_SmoothStepN(float fValue)
{
   if (fValue <= 0.0f)
      return 0.0f;

   if (fValue >= 1.0f)
      return 1.0f;

   return fValue * fValue * (3.0f - 2.0f * fValue);
}


/*====================
  M_SinCos
  ====================*/
inline
void	M_SinCos(float a, float &s, float &c)
{
#ifdef _WIN32
	_asm
	{
		fld		a
		fsincos
		mov		ecx, c
		mov		edx, s
		fstp	dword ptr [ecx]
		fstp	dword ptr [edx]
	}
#else
	s = sin(a);
	c = cos(a);
#endif
}


/*====================
  M_FtoI
  ====================*/
inline
int		M_FtoI(float f)
{
#ifdef _WIN32
	int tmp;
	_asm
	{
		fld		f
		fistp	tmp
		mov		eax, tmp
	}
#else
	return int(f);
#endif	
}
//=============================================================================
#endif //__MATH_INLINES_H__
