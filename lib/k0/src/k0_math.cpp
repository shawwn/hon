// (C)2008 S2 Games
// k0_math.cpp
//
// Math library
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include "k0_math.h"

#include "c_plane.h"
#include "c_axis.h"
#include "k0_intersection.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================

// From "Texturing and Modeling, a Procedural Approach", chapter 6
const int BB = 0x100;
const int BM = 0xff;

const int N = 0x1000;
const int NP = 12;   // 2^N
const int NM = 0xfff;

static int p[BB + BB + 2];
static float g3[BB + BB + 2][3];
static float g2[BB + BB + 2][2];
static float g1[BB + BB + 2];

#define s_curve(t) ( t * t * (3.0f - 2.0f * t) )

#define setup(i, b0, b1, r0, r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0 + 1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.0f;

float lattice[128][128];

static uint s_auiCRC32[256];
//=============================================================================

/*====================
  M_NextPowerOfTwo
  ====================*/
int		M_NextPowerOfTwo(int x)
{
	// Check that it isn't already a power of two
	for (int n = 0; n < 32; ++n)
	{
		if (x == (1 << n))
			return x;
	}

	// Find the highest bit
	int c = 0;
	while (x > 0)
	{
		x >>= 1;
		++c;
	}
	
	return (1 << c);
}


/*====================
  M_AABBOnPlaneSide
  ====================*/
EPlaneTest	M_AABBOnPlaneSide(const CBBoxf &bbBox, const CPlane &p)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Neg, v3Pos; // negative and positive vertices
	
	if (p.v3Normal.x > 00.0f)
	{
		v3Pos.x = v3Max.x;
		v3Neg.x = v3Min.x;
	}
	else
	{
		v3Pos.x = v3Min.x;
		v3Neg.x = v3Max.x;
	}

	if (p.v3Normal.y > 0.0f)
	{
		v3Pos.y = v3Max.y;
		v3Neg.y = v3Min.y;
	}
	else
	{
		v3Pos.y = v3Min.y;
		v3Neg.y = v3Max.y;
	}

	if (p.v3Normal.z > 0.0f)
	{
		v3Pos.z = v3Max.z;
		v3Neg.z = v3Min.z;
	}
	else
	{
		v3Pos.z = v3Min.z;
		v3Neg.z = v3Max.z;
	}

	if (DotProduct(p.v3Normal, v3Pos) - p.fDist < 0)
		return PLANE_NEGATIVE;
	else if (DotProduct(p.v3Normal, v3Neg) - p.fDist > 0)
		return PLANE_POSITIVE;
	else
		return PLANE_INTERSECTS;
}


/*====================
  M_OBBOnPlaneSide
  ====================*/
EPlaneTest		M_OBBOnPlaneSide(const CBBoxf &bbBox, const CVec3f &v3Pos, const CAxis &aAxis, const CPlane &p)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Negative, v3Positive; // Negative and positive vertices
	CVec3f	v3Normal;

	// Transform the plane normal into the space of the box so we can work out the P and N vertices
	v3Normal.x = DotProduct(p.v3Normal, aAxis[0]);
	v3Normal.y = DotProduct(p.v3Normal, aAxis[1]);
	v3Normal.z = DotProduct(p.v3Normal, aAxis[2]);

	if (v3Normal.x > 0.0f)
	{
		v3Positive.x = v3Max.x;
		v3Negative.x = v3Min.x;
	}
	else
	{
		v3Positive.x = v3Min.x;
		v3Negative.x = v3Max.x;
	}

	if (v3Normal.y > 0.0f)
	{
		v3Positive.y = v3Max.y;
		v3Negative.y = v3Min.y;
	}
	else
	{
		v3Positive.y = v3Min.y;
		v3Negative.y = v3Max.y;
	}

	if (v3Normal.z > 0.0f)
	{
		v3Positive.z = v3Max.z;
		v3Negative.z = v3Min.z;
	}
	else
	{
		v3Positive.z = v3Min.z;
		v3Negative.z = v3Max.z;
	}
	
	// we need the N and P vertices in world space
	v3Positive = TransformPoint(v3Positive, aAxis, v3Pos);
	
	if (DotProduct(p.v3Normal, v3Positive) - p.fDist < 0.0f)
	{
		return PLANE_NEGATIVE;
	}
	else
	{
		v3Negative = TransformPoint(v3Negative, aAxis, v3Pos);
		if (DotProduct(p.v3Normal, v3Negative) - p.fDist > 0.0f)
			return PLANE_POSITIVE;
		else
			return PLANE_INTERSECTS;
	}
}


/*====================
  M_QuatToAxis
  ====================*/
CAxis	M_QuatToAxis(const CVec4f &v4)
{
	// calculate coefficients
	float x2(v4[X] + v4[X]), y2(v4[Y] + v4[Y]), z2(v4[Z] + v4[Z]);
	float xx(v4[X] * x2), xy(v4[X] * y2), xz(v4[X] * z2);
	float yy(v4[Y] * y2), yz(v4[Y] * z2), zz(v4[Z] * z2);
	float wx(v4[W] * x2), wy(v4[W] * y2), wz(v4[W] * z2);

	return CAxis(
		CVec3f(1.0f - (yy + zz), xy + wz, xz - wy),
		CVec3f(xy - wz, 1.0f - (xx + zz), yz + wx),
		CVec3f(xz + wy, yz - wx, 1.0f - (xx + yy))
		);
}


/*====================
  M_AxisToQuat
  ====================*/
CVec4f	M_AxisToQuat(const CAxis &m)
{
	CVec4f quat;
	float	tr, s, q[4];
	int		i, j, k;
	int		nxt[3] = {1, 2, 0};
	
	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0)
	{
		s = sqrt(tr + 1.0f);
		quat[W] = s / 2.0f;
		s = 0.5f / s;
		quat[X] = (m[1][2] - m[2][1]) * s;
		quat[Y] = (m[2][0] - m[0][2]) * s;
		quat[Z] = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;
		if (m[1][1] > m[0][0])
			i = 1;
		if (m[2][2] > m[i][i])
			i = 2;

		j = nxt[i];
		k = nxt[j];
		s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0)
			s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;

		quat[X] = q[0];
		quat[Y] = q[1];
		quat[Z] = q[2];
		quat[W] = q[3];
	}

	return quat;
}


/*====================
  M_GetYawFromForwardVec2
  ====================*/
float	M_GetYawFromForwardVec2(const CVec2f &v2Forward)
{
	float fResult;

	if (v2Forward.x == 0.f && v2Forward.y == 0.f)
	{
		fResult = 0.f;
	}
	else if (v2Forward.x == 0.f)
	{
		if (v2Forward.y > 0.f)
			fResult = 0.f;
		else
			fResult = 180.f;
	}
	else
	{
		fResult = RAD2DEG(atan2(-v2Forward.x, v2Forward.y));
	}

	if (fResult > 180.f)
		fResult -= 360.f;

	return fResult;
}


/*====================
  M_GetAnglesFromForwardVec
  ====================*/
void	M_GetAnglesFromForwardVec(const CVec3f &vForward, CVec3f &vAngles)
{
	if (vForward.x == 0.0f && vForward.y == 0.0f) // straight up
	{
		// +Z will go up, -z down
		if (vForward.z > 0.0f)
			vAngles[PITCH] = 90.0f;
		else
			vAngles[PITCH] = -90.0f;
		
		vAngles[YAW] = 0.0f;
	}
	else
	{
		if (vForward.x == 0.0f) // north - south
		{
			// +y will go north, -y south
			if (vForward.y > 0.0f)
				vAngles[YAW]  = 0.0f;
			else
				vAngles[YAW]  = 180.0f;
		}
		else
		{
			vAngles[YAW] = RAD2DEG(atan2(-vForward.x, vForward.y));
		}

		if (vForward.z == 0.0f)
		{
			vAngles[PITCH] = 0.0f;
		}
		else
		{
			vAngles[PITCH] = RAD2DEG(atan2(vForward.z, sqrt(vForward.x * vForward.x + vForward.y * vForward.y)));
		}
	}

	vAngles[ROLL] = 0.0f;
}


/*====================
  M_GetAnglesFromForwardVec
  ====================*/
CVec3f	M_GetAnglesFromForwardVec(const CVec3f &vForward)
{
	CVec3f v3Angles;

	if (vForward.x == 0.0f && vForward.y == 0.0f) // straight up
	{
		// +Z will go up, -z down
		if (vForward.z > 0.0f)
			v3Angles[PITCH] = 90.0f;
		else
			v3Angles[PITCH] = -90.0f;
		
		v3Angles[YAW] = 0.0f;
	}
	else
	{
		if (vForward.x == 0.0f) // north - south
		{
			// +y will go north, -y south
			if (vForward.y > 0.0f)
				v3Angles[YAW]  = 0.0f;
			else
				v3Angles[YAW]  = 180.0f;
		}
		else
		{
			v3Angles[YAW] = RAD2DEG(atan2(-vForward.x, vForward.y));
		}

		if (vForward.z == 0.0f)
		{
			v3Angles[PITCH] = 0.0f;
		}
		else
		{
			v3Angles[PITCH] = RAD2DEG(atan2(vForward.z, sqrt(vForward.x * vForward.x + vForward.y * vForward.y)));
		}
	}

	v3Angles[ROLL] = 0.0f;

	return v3Angles;
}


/*====================
  M_GetForwardVecFromAngles
  ====================*/
CVec3f	M_GetForwardVecFromAngles(const CVec3f &v3Angles)
{
	float sp = DEGSIN(v3Angles[PITCH]);
	float cp = DEGCOS(v3Angles[PITCH]);
	float sy = DEGSIN(v3Angles[YAW]);
	float cy = DEGCOS(v3Angles[YAW]);

	return CVec3f(cp * -sy, cp * cy, sp);
}


/*====================
  M_GetForwardVec2FromYaw
  ====================*/
CVec2f	M_GetForwardVec2FromYaw(float fYaw)
{
	float sy = DEGSIN(fYaw);
	float cy = DEGCOS(fYaw);

	return CVec2f(-sy, cy);
}


/*====================
  M_Random

  simple random number generator
  32,767 values are possible for any given range.
  ====================*/
float	M_Random(float fMin, float fMax)
{
	if (fMax <= fMin)
		return fMin;

	return (rand() & 0x7fff) / (float)0x7fff * (fMax - fMin) + fMin;
}

int		M_Random(int iMin, int iMax)
{
	if (iMax <= iMin)
		return iMin;

	return rand() % (iMax - iMin + 1) + iMin;
}

uint	M_Random(uint uiMin, uint uiMax)
{
	if (uiMax <= uiMin)
		return uiMin;

	return rand() % (uiMax - uiMin + 1) + uiMin;
}


/*====================
  M_RayPlaneIntersect

  intersection test for ray with plane
  ====================*/
float	M_RayPlaneIntersect(const CVec3f &v3Origin, const CVec3f &v3Dir, const CPlane &plPlane, CVec3f &v3Result)
{
	v3Result.Clear();

	float fDenom(DotProduct(plPlane.v3Normal, v3Dir));

	float fTime((plPlane.fDist - DotProduct(plPlane.v3Normal, v3Origin)) / fDenom);
	if (fTime < 0.0f)
		return 0.0f;

	v3Result = v3Origin + (v3Dir * fTime);
	return fTime;
}


/*====================
  M_LerpQuat

  From "Slerping Clock Cycles"
  ====================*/
CVec4f	M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To)
{
	float fScale0, fScale1;

	// calc cosine
	float fCosOmega(v4From.x * v4To.x + v4From.y * v4To.y + v4From.z * v4To.z + v4From.w * v4To.w);

	float fAbsCosOmega(fabs(fCosOmega));
	
	if ((1.0f - fAbsCosOmega) > QUAT_EPSILON)
	{
		float fSinSqr(1.0f - fAbsCosOmega * fAbsCosOmega);
		float fSinOmega(1.0f / sqrt(fSinSqr));
		float fOmega(atan2(fSinSqr * fSinOmega, fAbsCosOmega));

		fScale0 = sin((1.0f - fLerp) * fOmega ) * fSinOmega;
		fScale1 = sin(fLerp * fOmega) * fSinOmega;
	}
	else
	{
		fScale0 = 1.0f - fLerp;
		fScale1 = fLerp;
	}
	
	// Adjust sign
	fScale1 = (fCosOmega >= 0.0f) ? fScale1 : -fScale1;

	return CVec4f
	(
		fScale0 * v4From.x + fScale1 * v4To.x,
		fScale0 * v4From.y + fScale1 * v4To.y,
		fScale0 * v4From.z + fScale1 * v4To.z,
		fScale0 * v4From.w + fScale1 * v4To.w
	);
}


/*====================
  M_LineBoxIntersect
  ====================*/
bool	M_LineBoxIntersect(const CVec3f &vecStart, const CVec3f &vecEnd, const CBBoxf &bbBounds, float &fFraction)
{
	CVec3f v;
	CVec3f t0(0.0f, 0.0f, 0.0f);
	CVec3f t1(1.0f, 1.0f, 1.0f);
	float enter, exit;
	int i;

	const CVec3f &vecBMin(bbBounds.GetMin());
	const CVec3f &vecBMax(bbBounds.GetMax());

	if (vecStart.InBounds(vecBMin, vecBMax))
	{
		fFraction = 0.0f;
		return true;
	}

	v = vecEnd - vecStart;

	if (v == V_ZERO)
	{
		//line is a point outside the box
		return false;
	}

	// Get first times of overlapping axes	
	for (i = 0; i < 3; ++i)
	{
		bool cont = false;
		
		if (v[i] == 0.0f)
		{
			if (vecStart[i] > vecBMax[i] || vecStart[i] < vecBMin[i])
				return false;		//this axis is outside the box
			else
				continue;
		}

		if (v[i] < 0.0f)
		{
			t0[i] = (vecBMax[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMin[i]-vecStart[i]) / v[i];
			cont = true;
		}
		else
		{
			t0[i] = (vecBMin[i]-vecStart[i]) / v[i];
			t1[i] = (vecBMax[i]-vecStart[i]) / v[i];
			cont = true;
		}
		
		if (!cont)
			return false;
	}

	enter = MAX(t0.x, MAX(t0.y, t0.z));
	exit = MIN(t1.x, MIN(t1.y, t1.z));

	if (enter <= exit && enter < 1)
	{
		fFraction = enter;
		return true;
	}

	return false;
}


/*====================
  M_LerpAngle

  lerp across the smallest arc
  ====================*/
float	M_LerpAngle(float a, float low, float high) 
{
	float ret;

	if (high - low > 180.0f)
	{
		low += 360.0f;
	}
	if (high - low < -180.0f)
	{
		high += 360.0f;
	}

	ret = LERP(a, low, high);
	if (ret < 0.0f)
		ret += 360.0f;
	if (ret > 360.0f)
		ret -= 360.0f;

	return ret;
}


/*====================
  M_LerpAngles
  ====================*/
CVec3f	M_LerpAngles(float a, const CVec3f &v3Low, const CVec3f &v3High)
{
	return CVec3f
	(
		M_LerpAngle(a, v3Low.x, v3High.x),
		M_LerpAngle(a, v3Low.y, v3High.y),
		M_LerpAngle(a, v3Low.z, v3High.z)
	);
}


/*====================
  M_ChangeAngle
  ====================*/
float	M_ChangeAngle(float fAngleStep, float fStartAngle, float fEndAngle)
{
	float fResult(fEndAngle);
	
	if (fStartAngle - fEndAngle > 180.0f)
		fEndAngle += 360.0f;
	if (fStartAngle - fEndAngle < -180.0f)
		fStartAngle += 360.0f;

	if (fStartAngle > fEndAngle)
	{
		if (fStartAngle - fEndAngle <= fAngleStep)
			fResult = fEndAngle;
		else
			fResult = fStartAngle - fAngleStep;
	}
	else if (fEndAngle > fStartAngle)
	{
		if (fEndAngle - fStartAngle <= fAngleStep)
			fResult = fEndAngle;
		else
			fResult = fStartAngle + fAngleStep;
	}

	if (fResult >= 180.0f)
		fResult -= 360.0f;

	return fResult;
}


/*====================
  M_DiffAngle
  ====================*/
float	M_DiffAngle(float fAngle1, float fAngle2)
{
	if (fAngle1 - fAngle2 > 180.f)
		fAngle2 += 360.f;
	if (fAngle1 - fAngle2 < -180.f)
		fAngle1 += 360.f;

	if (fAngle1 > fAngle2)
		return fAngle1 - fAngle2;
	else
		return fAngle2 - fAngle1;
}


/*====================
  M_SlerpDirection
  ====================*/
CVec3f	M_SlerpDirection(float fLerp, const CVec3f &v3A, const CVec3f &v3B)
{
	float fAngle(acos(DotProduct(v3A, v3B)));

	if (fAngle > 0.001f)
		return CVec3f(v3A * ((sin((1.0f - fLerp) * fAngle)) / sin(fAngle)) + v3B * ((sin(fLerp * fAngle)) / sin(fAngle)));
	else
		return v3A;
}


/*====================
  M_ArcLengthFactor

  returns the scale between the length of a chord at
  angle1 and angle2 and the arc-length
  ====================*/
float	M_ArcLengthFactor(float fAngle1, float fAngle2)
{
	float fCoordLength(sqrt(cos(fAngle1) * cos(fAngle1) + sin(fAngle2) * sin(fAngle2)));
	float fArcLength(2.0f * M_PI * fabs(fAngle1 - fAngle2));

	return fArcLength / fCoordLength;
}


/*====================
  M_RandomDirection
  ====================*/
CVec3f	M_RandomDirection()
{
	// Generate a random point in the unit square
	CVec2f	r(M_Random(0.0f, 1.0f), M_Random(0.0f, 1.0f));

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)), 2.0f * M_PI * r.y);

	// Convert to directional vector
	return CVec3f(cos(s.y) * sin(s.x), sin(s.y) * sin(s.x), cos(s.x));
}

CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle)
{
	// Generate a random point in the unit square
	CVec2f	r(M_Random(0.0f, 1.0f), M_Random(0.0f, 1.0f));

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)) * (fAngle/180.0f), 2.0f * M_PI * r.y);

	// Convert to directional vector
	CVec3f	v3Ret(cos(s.y) * sin(s.x), sin(s.y) * sin(s.x), cos(s.x));

	// Rotate toward v3Dir
	CAxis	aAxis(GetAxisFromUpVec(v3Dir));
	
	return TransformPoint(v3Ret, aAxis);
}

CVec3f	M_RandomDirection(const CVec3f &v3Dir, float fAngle0, float fAngle1)
{
	// Generate a random point in the unit square
	CVec2f	r(M_Random(0.0f, 1.0f), M_Random(0.0f, 1.0f));

	float fA0(fAngle0/180.0f), fA1(fAngle1/180.0f);

	// Remap to spherical coords (phi, theta) distributed evenly over the sphere
	CVec2f	s(2.0f * acos(sqrt(1.0f - r.x)) * (fA1 - fA0) + DEG2RAD(fAngle0), 2.0f * M_PI * r.y);

	// Convert to directional vector
	CVec3f	v3Ret(cos(s.y) * sin(s.x), sin(s.y) * sin(s.x), cos(s.x));

	// Rotate toward v3Dir
	CAxis	aAxis(GetAxisFromUpVec(v3Dir));
	
	return TransformPoint(v3Ret, aAxis);
}


/*====================
  M_RandomPointInSphere
  ====================*/
CVec3f	M_RandomPointInSphere()
{
	for (;;)
	{
		CVec3f p(M_Random(-1.0f, 1.0f), M_Random(-1.0f, 1.0f), M_Random(-1.0f, 1.0f));

		if (p.LengthSq() <= 1.0f)
			return p;
	}
}


/*====================
  M_RandomPointInCircle
  ====================*/
CVec2f	M_RandomPointInCircle()
{
	for (;;)
	{
		CVec2f p(M_Random(-1.0f, 1.0f), M_Random(-1.0f, 1.0f));

		if (p.LengthSq() <= 1.0f)
			return p;
	}
}


/*====================
  M_AreaOfTriangle

  The area of a triangle in 3-space is one half the magnitude of the
  cross-product of two sides of the triangle.
  ====================*/
float	M_AreaOfTriangle(const CVec3f &v3A, const CVec3f &v3B, const CVec3f &v3C)
{
	return Length(CrossProduct(v3B - v3A, v3C - v3A)) / 2.0f;
}


/*====================
  M_CalcAxisExtents
  ====================*/
void	M_CalcAxisExtents(const CVec3f &v3Axis, const vector<CVec3f> &v3Points, float &fMin, float &fMax)
{
	fMin = FAR_AWAY;
	fMax = -FAR_AWAY;

	for (vector<CVec3f>::const_iterator it(v3Points.begin()); it != v3Points.end(); ++it)
	{
		float fDist(DotProduct(v3Axis, *it));

		if (fDist < fMin)
			fMin = fDist;
		if (fDist > fMax)
			fMax = fDist;
	}
}


/*====================
  M_CalcAxisExtents
  ====================*/
void	M_CalcAxisExtents(const CVec3f &v3Axis, const CBBoxf &bbBox, float &fMin, float &fMax)
{
	const CVec3f &v3Min(bbBox.GetMin());
	const CVec3f &v3Max(bbBox.GetMax());

	CVec3f	v3Neg, v3Pos; // negative and positive vertices
	
	if (v3Axis.x > 0)
	{
		v3Pos.x = v3Max.x;
		v3Neg.x = v3Min.x;
	}
	else
	{
		v3Pos.x = v3Min.x;
		v3Neg.x = v3Max.x;
	}

	if (v3Axis.y > 0)
	{
		v3Pos.y = v3Max.y;
		v3Neg.y = v3Min.y;
	}
	else
	{
		v3Pos.y = v3Min.y;
		v3Neg.y = v3Max.y;
	}

	if (v3Axis.z > 0)
	{
		v3Pos.z = v3Max.z;
		v3Neg.z = v3Min.z;
	}
	else
	{
		v3Pos.z = v3Min.z;
		v3Neg.z = v3Max.z;
	}

	fMin = DotProduct(v3Axis, v3Neg);
	fMax = DotProduct(v3Axis, v3Pos);
}


/*====================
  M_LinePointDistance

  The distance between a line and a point is the cross product of the
  difference between the origin of a ray on the line and the point and the direction
  of that ray.
  ====================*/
float	M_LinePointDistance(const CVec3f &v3Origin, const CVec3f &v3Dir, const CVec3f &v3Point)
{
	return Length(CrossProduct(v3Point - v3Origin, v3Dir));
}


/*====================
  M_RotatePointAroundAxis
  ====================*/
CVec3f	M_RotatePointAroundAxis(const CVec3f &v3In, const CVec3f &v3Axis, float fAngle)
{
	CVec3f v3Fulcrum(NormalProject(v3Axis, v3In));

	CVec3f p0(v3In - v3Fulcrum);
	CVec3f p1(CrossProduct(v3Axis, p0));
 
	float fRad(DEG2RAD(fAngle));

	return v3Fulcrum + p0 * cos(fRad) + p1 * sin(fRad);
}


/*====================
  M_RotatePointAroundLine
  ====================*/
CVec3f	M_RotatePointAroundLine(const CVec3f &v3In, const CVec3f &v3Start, const CVec3f &v3End, float fAngle)
{
	CVec3f v3Axis(Normalize(v3End - v3Start));

	CVec3f v3Fulcrum(NormalProject(v3Axis, v3In - v3Start));

	CVec3f p0(v3In - v3Fulcrum - v3Start);
	CVec3f p1(CrossProduct(v3Axis, p0));

	float fRad(DEG2RAD(fAngle));

	return v3Start + v3Fulcrum + p0 * cos(fRad) + p1 * sin(fRad);
}


/*====================
  M_ClipLine

  Clip everything on the positive side of the plane
  ====================*/
bool	M_ClipLine(const CPlane &plPlane, CVec3f &p1, CVec3f &p2)
{
	if (plPlane.Distance(p1) > 0.0f)
	{
		if (plPlane.Distance(p2) > 0.0f)
		{
			// Both positive
			return false;
		}
		else
		{
			// Crossing positive into negative
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(p1, p2, plPlane, fFraction))
			{
				p1 = LERP(fFraction, p1, p2);
				return true;
			}

			return false;
		}
	}
	else
	{
		if (plPlane.Distance(p2) > 0.0f)
		{
			// Crossing negative into positive
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(p1, p2, plPlane, fFraction))
			{
				p2 = LERP(fFraction, p1, p2);
				return true;
			}

			return false;
		}
		else
		{
			// Both negative
			return true;
		}
	}
}


/*====================
  M_InitCRC32
  ====================*/
void	M_InitCRC32()
{
	// This is the official polynomial defined in IEEE 802.3.
	// The polynomial is occasionally reversed to 0x04C11DB7.
	uint uiPolynomial(0xEDB88320);

	uint uiCRC;
	for(int i(0); i < 256; i++)
	{
		uiCRC = i;
		for(int j(8); j > 0; j--)
		{
			if(uiCRC & 1)
				uiCRC = (uiCRC >> 1) ^ uiPolynomial;
			else
				uiCRC >>= 1;
		}

		s_auiCRC32[i] = uiCRC;
	}
}


/*====================
  M_GetCRC32
  ====================*/
uint	M_GetCRC32(const byte *pBuffer, uint uiSize)
{
	uint uiCRC32(0xFFFFFFFF);
	
#ifdef __GNUC__
#ifdef __x86_64__
	asm("crc32loop:\n\t"
		"xorq %%rax, %%rax\n\t"
		"mov (%%rsi), %%bl\n\t"
		"mov %%cl, %%al\n\t"
		"incq %%rsi\n\t"
		"xor %%bl, %%al\n\t"
		"shrl $8, %%ecx\n\t"
		"movl (%%rdi, %%rax, 4), %%ebx\n\t"
		"xorl %%ebx, %%ecx\n\t"
		"decl %%edx\n\t"
		"jnz crc32loop\n\t"
		: "=c" (uiCRC32)
		: "c" (uiCRC32), "d" (uiSize), "S" (pBuffer), "D" (s_auiCRC32)
		: "rax", "rbx");
#else
	asm("crc32loop:\n\t"
		"xorl %%eax, %%eax\n\t"
		"mov (%%esi), %%bl\n\t"
		"mov %%cl, %%al\n\t"
		"incl %%esi\n\t"
		"xor %%bl, %%al\n\t"
		"shrl $8, %%ecx\n\t"
		"movl (%%edi, %%eax, 4), %%ebx\n\t"
		"xorl %%ebx, %%ecx\n\t"
		"decl %%edx\n\t"
		"jnz crc32loop\n\t"
		: "=c" (uiCRC32)
		: "c" (uiCRC32), "d" (uiSize), "S" (pBuffer), "D" (s_auiCRC32)
		: "eax", "ebx");
#endif
#else	
	// Register use:
	//		ebx - a lot of things
	//		ecx - CRC32 value
	//		edx - address of end of buffer
	//		esi - address of start of buffer
	//		edi - CRC32 table
	__asm
	{
		// Save the esi and edi registers
		push esi
		push edi

		mov ecx, uiCRC32			// Initialize ecx
		lea edi, s_auiCRC32			// Load the crc table
		mov esi, pBuffer			// Load buffer
		mov edx, uiSize				// Load size

	crc32loop:
		xor eax, eax				// Clear the eax register
		mov bl, byte ptr [esi]
		
		mov al, cl					// Copy crc value into eax
		inc esi						// Advance the source pointer

		xor al, bl					// Create the index into the CRC32 table
		shr ecx, 8

		mov ebx, [edi + (eax * 4)]	// Get the value out of the table
		xor ecx, ebx				// xor with the current byte

		dec edx
		jnz crc32loop

		// Restore the edi and esi registers
		pop edi
		pop esi

		mov uiCRC32, ecx			// Write the result
	}
#endif

	return uiCRC32;
}


/*====================
  M_GetAngle8
  ====================*/
byte	M_GetAngle8(float fAngle)
{
	while (fAngle < 0.0f) fAngle += 360.0f;
	
	return byte(INT_ROUND(fAngle / 360.0f * 255.0f));
}


/*====================
  M_GetAngle
  ====================*/
float	M_GetAngle(byte yAngle8)
{
	return yAngle8 / 255.0f * 360.0f;
}


/*====================
  M_GetFaceTangent
  ====================*/
CVec3f	M_GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2)
{
	CVec3f A(CrossProduct(CVec3f(v0.x, t0.x, t0.y) - CVec3f(v1.x, t1.x, t1.y), CVec3f(v0.x, t0.x, t0.y) - CVec3f(v2.x, t2.x, t2.y)));
	CVec3f B(CrossProduct(CVec3f(v0.y, t0.x, t0.y) - CVec3f(v1.y, t1.x, t1.y), CVec3f(v0.y, t0.x, t0.y) - CVec3f(v2.y, t2.x, t2.y)));
	CVec3f C(CrossProduct(CVec3f(v0.z, t0.x, t0.y) - CVec3f(v1.z, t1.x, t1.y), CVec3f(v0.z, t0.x, t0.y) - CVec3f(v2.z, t2.x, t2.y)));

	// return the tangent
	return CVec3f(A.x == 0.0f ? FAR_AWAY : -A.y/A.x, B.x == 0.0f ? FAR_AWAY : -B.y/B.x, C.x == 0.0f ? FAR_AWAY : -C.y/C.x).Direction();
}


/*====================
  M_Init
  ====================*/
void    M_Init()
{
	M_InitCRC32();
}
