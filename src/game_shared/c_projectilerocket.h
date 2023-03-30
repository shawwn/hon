// (C)2006 S2 Games
// c_projectilerocket.h
//
//=============================================================================
#ifndef __C_PROJECTILEROCKET_H__
#define __C_PROJECTILEROCKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileRocket
//=============================================================================
class CProjectileRocket : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, Rocket);

public:
	~CProjectileRocket()	{}
	CProjectileRocket();
};
//=============================================================================

#endif //__C_PROJECTILEROCKET_H__
