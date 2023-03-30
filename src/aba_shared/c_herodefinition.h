// (C)2008 S2 Games
// c_herodefinition.h
//
//=============================================================================
#ifndef __C_HERODEFINITION_H__
#define __C_HERODEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IHeroEntity, Hero, hero)
//=============================================================================

//=============================================================================
// CHeroDefinition
//=============================================================================
class CHeroDefinition : public IUnitDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_PROPERTY(Strength, float)
	ENT_DEF_PROPERTY(Agility, float)
	ENT_DEF_PROPERTY(Intelligence, float)
	ENT_DEF_PROPERTY(StrengthPerLevel, float)
	ENT_DEF_PROPERTY(AgilityPerLevel, float)
	ENT_DEF_PROPERTY(IntelligencePerLevel, float)
	ENT_DEF_PROPERTY(PrimaryAttribute, EAttribute)
	ENT_DEF_STRING_PROPERTY(Team)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(RespawnEffect, Effect)
	ENT_DEF_RESOURCE_PROPERTY(AnnouncerSound, Sample)

	tsvector		m_vsGoodItems;
	tsvector		m_vsRecommendedItems;

public:
	~CHeroDefinition()	{}
	CHeroDefinition() :
	IUnitDefinition(&g_allocatorHero)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(g_heapResources,    CHeroDefinition)(*this); }

	virtual void	Precache(EPrecacheScheme eScheme)
	{
		IUnitDefinition::Precache(eScheme);

		PRECACHE_GUARD
			PrecacheRespawnEffect();
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		IUnitDefinition::GetPrecacheList(eScheme, deqPrecache);

		PRECACHE_GUARD
			deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
		PRECACHE_GUARD_END
	}

	void			AddGoodItem(const tstring &sItem)			{ m_vsGoodItems.push_back(sItem); }
	const tstring	GetGoodItem(uint uiSlot) const				{ return uiSlot >= 0 && uiSlot < m_vsGoodItems.size() ? m_vsGoodItems[uiSlot] : TSNULL; }
	const tsvector&	GetGoodItems() const						{ return m_vsGoodItems; }

	bool	HasGoodItem(const tstring &sItem) const
	{
		for (tsvector_cit it(m_vsGoodItems.begin()); it != m_vsGoodItems.end(); it++)
		{
			if (*it == sItem)
				return true;
		}

		return false;
	}

	void			AddRecommendedItem(const tstring &sItem)	{ m_vsRecommendedItems.push_back(sItem); }
	const tstring	GetRecommendedItem(uint uiSlot) const		{ return uiSlot >= 0 && uiSlot < m_vsRecommendedItems.size() ? m_vsRecommendedItems[uiSlot] : TSNULL; }
	const tsvector&	GetRecommendedItems() const					{ return m_vsRecommendedItems; }

	bool	HasRecommendedItem(const tstring &sItem) const
	{
		for (tsvector_cit it(m_vsRecommendedItems.begin()); it != m_vsRecommendedItems.end(); it++)
		{
			if (*it == sItem)
				return true;
		}

		return false;
	}
};
//=============================================================================

#endif //__C_HERODEFINITION_H__
