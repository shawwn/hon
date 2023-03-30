// (C)2008 S2 Games
// c_sphere.h
//
//=============================================================================
#ifndef __C_SPHERE_H__
#define __C_SPHERE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
//=============================================================================

//=============================================================================
// CSphere
//=============================================================================
class CSphere
{
private:
	CVec3f	m_v3Center;
	float	m_fRadius;

public:
	~CSphere() {}

	// Constructors
	CSphere() {}

	CSphere(const CVec3f &v3Center, float fRadius) :
	m_v3Center(v3Center),
	m_fRadius(fRadius)
	{
	}

	// Accessors
	const CVec3f&	GetCenter() const	{ return m_v3Center; }
	float			GetRadius() const	{ return m_fRadius; }

	// Mutators
	void			SetCenter(const CVec3f &v3Center) { m_v3Center = v3Center; }
	void			SetRadius(float fRadius) { m_fRadius = fRadius; }
};
//=============================================================================

#endif //__C_SPHERE_H__
