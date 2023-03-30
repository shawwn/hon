// c_outbone.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_outbone.h"
//=============================================================================

/*====================
  COutBone::COutBone
  ====================*/
COutBone::COutBone(uint uiIndex, const tstring &sName, uint uiParentIndex) :
m_uiIndex(uiIndex),
m_sName(sName),
m_uiParent(uiParentIndex)
{
	m_base.axis.Set(0.0f, 0.0f, 0.0f);
	m_base.pos.Set(0.0f, 0.0f, 0.0f);

	m_invBase.axis.Set(0.0f, 0.0f, 0.0f);
	m_invBase.pos.Set(0.0f, 0.0f, 0.0f);
}
