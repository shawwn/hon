// (C)2007 S2 Games
// c_gunfireball.h
//
//=============================================================================
#ifndef __C_GUNFIREBALL_H__
#define __C_GUNFIREBALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// CGunFireball
//=============================================================================
class CGunFireball : public IGunItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Gun, Fireball);

public:
	~CGunFireball()	{}
	CGunFireball() :
	IGunItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_GUNFIREBALL_H__
