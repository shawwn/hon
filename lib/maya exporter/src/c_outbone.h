// (C)2008 S2 Games
// c_outbone.h
//
//=============================================================================
#ifndef __C_OUTBONE_H__
#define __C_OUTBONE_H__

//=============================================================================
// Headers
//=============================================================================
#include "misc.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint INVALID_BONE(-1);
//=============================================================================

//=============================================================================
// COutBone
//=============================================================================
class COutBone
{
private:
	uint		m_uiIndex;
	tstring		m_sName;

	uivector	m_vChildren;

	uint		m_uiParent;

	// Force new bones to be initialized
	COutBone();

public:
	~COutBone()	{}

	COutBone(uint uiIndex, const tstring &sName, uint uiParentIndex);

	void			SetIndex(uint uiIndex)				{ m_uiIndex = uiIndex; }
	uint			GetIndex() const					{ return m_uiIndex; }

	void			SetName(const tstring &sName)		{ m_sName = sName; }
	const tstring&	GetName() const						{ return m_sName; }

	uint			GetChildIndex(uint uiIndex) const	{ return m_vChildren[uiIndex]; }
	void			AddChild(uint uiChildIndex)			{ m_vChildren.push_back(uiChildIndex); }
	uint			NumChildren() const					{ return uint(m_vChildren.size()); }
	const uivector&	GetChildren() const					{ return m_vChildren; }

	void			SetParent(uint uiParent)			{ m_uiParent = uiParent; }
	uint			GetParentIndex() const				{ return m_uiParent; }

	matrix43_t		m_invBase;					// INVERSE of the base transform for the bone (to transform verts to bone space)
	matrix43_t		m_base;						// base transform for the bone (for static meshes)
};
//=============================================================================

#endif //__C_OUTBONE_H__
