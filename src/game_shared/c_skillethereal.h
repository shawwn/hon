// (C)2007 S2 Games
// c_skillethereal.h
//
//=============================================================================
#ifndef __C_SKILLETHEREAL_H__
#define __C_SKILLETHEREAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilltoggle.h"
//=============================================================================

//=============================================================================
// CSkillEthereal
//=============================================================================
class CSkillEthereal : public ISkillToggle
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, Ethereal)

public:
	~CSkillEthereal()	{}
	CSkillEthereal() :
	ISkillToggle(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLETHEREAL_H__
