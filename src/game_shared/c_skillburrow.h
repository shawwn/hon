// (C)2007 S2 Games
// c_skillburrow.h
//
//=============================================================================
#ifndef __C_SKILLBURROW_H__
#define __C_SKILLBURROW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilltoggle.h"
//=============================================================================

//=============================================================================
// CSkillBurrow
//=============================================================================
class CSkillBurrow : public ISkillToggle
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, Burrow)

public:
	~CSkillBurrow()	{}
	CSkillBurrow() :
	ISkillToggle(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLBURROW_H__
