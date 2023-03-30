// (C)2008 S2 Games
// k0_constants.h
//
//=============================================================================
#ifndef __K0_CONSTANTS_H__
#define __K0_CONSTANTS_H__

//=============================================================================
// Definitions
//=============================================================================
#if 0
// Common vectors
const CVec3f V_ZERO		(0.0f, 0.0f, 0.0f);

const CVec2f V2_ZERO	(0.0f, 0.0f);
const CVec2f V3_ZERO	(0.0f, 0.0f);
const CVec4f V4_ZERO	(0.0f, 0.0f, 0.0f, 0.0f);

const CVec3f V_UP		(0.0f, 0.0f, 1.0f);

// Colors
const CVec4f WHITE		(1.0f, 1.0f, 1.0f, 1.0f);
const CVec4f CLEAR		(1.0f, 1.0f, 1.0f, 0.0f);
const CVec4f GRAY		(0.5f, 0.5f, 0.5f, 1.0f);
const CVec4f BLACK		(0.0f, 0.0f, 0.0f, 1.0f);
const CVec4f RED		(1.0f, 0.0f, 0.0f, 1.0f);
const CVec4f GREEN		(0.0f, 1.0f, 0.0f, 1.0f);
const CVec4f BLUE		(0.0f, 0.0f, 1.0f, 1.0f);
const CVec4f CYAN		(0.0f, 1.0f, 1.0f, 1.0f);
const CVec4f MAGENTA	(1.0f, 0.0f, 1.0f, 1.0f);
const CVec4f YELLOW		(1.0f, 1.0f, 0.0f, 1.0f);

#else // Temp fix for construction ordering issues with the constants

// Colors
#define CYAN		CVec4f(0.00f, 1.00f, 1.00f, 1.00f)
#define GRAY		CVec4f(0.50f, 0.50f, 0.50f, 1.00f)
#define NAVY		CVec4f(0.00f, 0.00f, 0.50f, 1.00f)
#define SILVER		CVec4f(0.75f, 0.75f, 0.75f, 1.00f)
#define BLACK		CVec4f(0.00f, 0.00f, 0.00f, 1.00f)
#define GREEN		CVec4f(0.00f, 0.50f, 0.00f, 1.00f)
#define OLIVE		CVec4f(0.50f, 0.50f, 0.00f, 1.00f)
#define TEAL		CVec4f(0.00f, 0.50f, 0.50f, 1.00f)
#define BLUE		CVec4f(0.00f, 0.00f, 1.00f, 1.00f)
#define LIME		CVec4f(0.00f, 1.00f, 0.00f, 1.00f)
#define PURPLE		CVec4f(0.50f, 0.00f, 0.50f, 1.00f)
#define WHITE		CVec4f(1.00f, 1.00f, 1.00f, 1.00f)
#define MAGENTA		CVec4f(1.00f, 0.00f, 1.00f, 1.00f)
#define MAROON		CVec4f(0.50f, 0.00f, 0.00f, 1.00f)
#define RED			CVec4f(1.00f, 0.00f, 0.00f, 1.00f)
#define YELLOW		CVec4f(1.00f, 1.00f, 0.00f, 1.00f)
#define ORANGE		CVec4f(1.00f, 0.65f, 0.00f, 1.00f)

#define CLEAR		CVec4f(1.00f, 1.00f, 1.00f, 0.00f)
#endif

const CVec4f g_v4Colors[] =
{
	CYAN,
	MAGENTA,
	YELLOW,
	BLUE,
	LIME,
	RED,
	TEAL,
	PURPLE,
	OLIVE,
	NAVY,
	GREEN,
	MAROON,
	ORANGE,
	SILVER,
	GRAY,
	WHITE,
	BLACK
};

const int NUM_COLORS(sizeof(g_v4Colors) / sizeof(CVec4f));

// Time
const uint MS_PER_SEC(1000);
const uint SEC_PER_MIN(60);
const uint MS_PER_MIN(MS_PER_SEC * SEC_PER_MIN);
const uint MIN_PER_HR(60);
const uint SEC_PER_HR(MIN_PER_HR * SEC_PER_MIN);
const uint MS_PER_HR(MIN_PER_HR * MS_PER_MIN);
const uint HR_PER_DAY(24);
const uint MIN_PER_DAY(HR_PER_DAY * MIN_PER_HR);
const uint SEC_PER_DAY(HR_PER_DAY * SEC_PER_HR);
const uint MS_PER_DAY(HR_PER_DAY * MS_PER_HR);

const float SEC_PER_MS(1.0f / MS_PER_SEC);
const float MIN_PER_MS(1.0f / MS_PER_MIN);
const float HR_PER_MS(1.0f / MS_PER_HR);
const float DAY_PER_MS(1.0f / MS_PER_DAY);
const float MIN_PER_SEC(1.0f / SEC_PER_MIN);
const float HR_PER_SEC(1.0f / SEC_PER_HR);
const float DAY_PER_SEC(1.0f / SEC_PER_DAY);
const float HR_PER_MIN(1.0f / MIN_PER_HR);
const float DAY_PER_MIN(1.0f / MIN_PER_DAY);
const float DAY_PER_HR(1.0f / HR_PER_DAY);

inline float	MsToSec(uint ms)	{ return ms * SEC_PER_MS; }
inline float	MsToMin(uint ms)	{ return ms * MIN_PER_MS; }
inline float	MsToHr(uint ms)		{ return ms * HR_PER_MS; }
inline float	MsToDay(uint ms)	{ return ms * DAY_PER_MS; }
inline uint		SecToMs(uint s)		{ return uint(s * MS_PER_SEC); }
inline float	SecToMin(uint s)	{ return s * MIN_PER_SEC; }
inline float	SecToHr(uint s)		{ return s * HR_PER_SEC; }
inline float	SecToDay(uint s)	{ return s * DAY_PER_SEC; }
inline uint		MinToMs(uint min)	{ return uint(min * MS_PER_MIN); }
inline uint		MinToSec(uint min)	{ return uint(min * SEC_PER_MIN); }
inline float	MinToHr(uint min)	{ return min * HR_PER_MIN; }
inline float	MinToDay(uint min)	{ return min * DAY_PER_MIN; }
inline uint		HrToMs(uint hr)		{ return uint(hr * MS_PER_HR); }
inline uint		HrToSec(uint hr)	{ return uint(hr * SEC_PER_HR); }
inline uint		HrToMin(uint hr)	{ return uint(hr * MIN_PER_HR); }
inline float	HrToDay(uint hr)	{ return hr * DAY_PER_HR; }
inline uint		DayToMs(uint day)	{ return uint(day * MS_PER_DAY); }
inline uint		DayToSec(uint day)	{ return uint(day * SEC_PER_DAY); }
inline uint		DayToMin(uint day)	{ return uint(day * MIN_PER_DAY); }
inline uint		DayToHr(uint day)	{ return uint(day * HR_PER_DAY); }

inline float	MsToSec(float ms)	{ return ms * SEC_PER_MS; }
inline float	MsToMin(float ms)	{ return ms * MIN_PER_MS; }
inline float	MsToHr(float ms)	{ return ms * HR_PER_MS; }
inline float	MsToDay(float ms)	{ return ms * DAY_PER_MS; }
inline uint		SecToMs(float s)	{ return uint(s * MS_PER_SEC); }
inline float	SecToMin(float s)	{ return s * MIN_PER_SEC; }
inline float	SecToHr(float s)	{ return s * HR_PER_SEC; }
inline float	SecToDay(float s)	{ return s * DAY_PER_SEC; }
inline uint		MinToMs(float min)	{ return uint(min * MS_PER_MIN); }
inline uint		MinToSec(float min)	{ return uint(min * SEC_PER_MIN); }
inline float	MinToHr(float min)	{ return min * HR_PER_MIN; }
inline float	MinToDay(float min)	{ return min * DAY_PER_MIN; }
inline uint		HrToMs(float hr)	{ return uint(hr * MS_PER_HR); }
inline uint		HrToSec(float hr)	{ return uint(hr * SEC_PER_HR); }
inline uint		HrToMin(float hr)	{ return uint(hr * MIN_PER_HR); }
inline float	HrToDay(float hr)	{ return hr * DAY_PER_HR; }
inline uint		DayToMs(float day)	{ return uint(day * MS_PER_DAY); }
inline uint		DayToSec(float day)	{ return uint(day * SEC_PER_DAY); }
inline uint		DayToMin(float day)	{ return uint(day * MIN_PER_DAY); }
inline uint		DayToHr(float day)	{ return uint(day * HR_PER_DAY); }

const uint	INVALID_INDEX(-1);
const uint	INVALID_TIME(-1);
const float	FAR_AWAY(10e9f);
const float CONTACT_EPSILON(0.03125f);
const float	DIAG(0.70710678118654752440084436210485f);
const float	LN2(0.69314718055994530941723212145818f);
//=============================================================================

#endif //__K0_CONSTANTS_H__
