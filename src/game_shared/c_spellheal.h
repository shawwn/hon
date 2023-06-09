// (C)2006 S2 Games
// c_spellheal.h
//
//=============================================================================
#ifndef __C_SPELLHEAL_H__
#define __C_SPELLHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellHeal
//=============================================================================
class CSpellHeal : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, Heal);

public:
	~CSpellHeal()	{}
	CSpellHeal() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLHEAL_H__
