// (C)2008 S2 Games
// k0_endian.h
//
//=============================================================================
#ifndef __K0_ENDIAN_H__
#define __K0_ENDIAN_H__

/*====================
  SwapIntEndian
 ====================*/
inline int SwapIntEndian(int in)
{
	return ((in >> 24) & 255) +
		   (((in >> 16) & 255) << 8) +
		   (((in >> 8) & 255) << 16) +
		   ((in & 255) << 24);
}


/*====================
  SwapFloatEndian
 ====================*/
inline float SwapFloatEndian(float in)
{
	union
	{
		float f;
		byte b[4];
	}
	u1, u2;

	u1.f = in;
	u2.b[0] = u1.b[3];
	u2.b[1] = u1.b[2];
	u2.b[2] = u1.b[1];
	u2.b[3] = u1.b[0];
	return u2.f;
}


/*====================
  SwapShortEndian
 ====================*/
inline int SwapShortEndian(int in)
{
	return ((in >> 8) & 255) + ((in & 255) << 8);
}


/*====================
  SwapInt64Endian
 ====================*/
inline LONGLONG SwapInt64Endian(LONGLONG in)
{
	return ((in >> 56) & 255) +
		   (((in >> 48) & 255) << 8) +
		   (((in >> 40) & 255) << 16) +
		   (((in >> 32) & 255) << 24) +
		   (((in >> 24) & 255) << 32) +
		   (((in >> 16) & 255) << 40) +
		   (((in >> 8) & 255) << 48) +
		   ((in & 255) << 56);
}


//=============================================================================
#if BYTE_ORDER == LITTLE_ENDIAN
#define LittleInt(in)	(in)
#define LittleShort(in)	(in)
#define LittleFloat(in)	(in)
#define LittleInt64(in)	(in)
#else //BYTE_ORDER != LITTLE_ENDIAN
inline int	LittleInt(int in)		{ return SwapIntEndian(in); }
inline short LittleShort(short in)	{ return SwapShortEndian(in); }
inline float LittleFloat(float in)	{ return SwapFloatEndian(in); }
inline LONGLONG LittleInt64(LONGLONG in)	{ return SwapInt64Endian(in); }
#endif //BYTE_ORDER
//=============================================================================

#endif //__K0_ENDIAN_H__
