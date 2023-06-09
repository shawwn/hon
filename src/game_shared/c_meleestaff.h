// (C)2006 S2 Games
// c_meleestaff.h
//
//=============================================================================
#ifndef __C_MELEESTAFF_H__
#define __C_MELEESTAFF_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeStaff
//=============================================================================
class CMeleeStaff : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, Staff);

public:
	~CMeleeStaff()	{}
	CMeleeStaff() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEESTAFF_H__
