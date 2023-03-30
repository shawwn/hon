// (C)2008 S2 Games
// c_outbone.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

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
}
