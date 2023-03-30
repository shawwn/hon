// (C)2007 S2 Games
// c_meleepush.h
//
//=============================================================================
#ifndef __C_MELEEPUSH_H__
#define __C_MELEEPUSH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleePush
//=============================================================================
class CMeleePush : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, Push)

public:
	~CMeleePush()	{}
	CMeleePush() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEEPUSH_H__
