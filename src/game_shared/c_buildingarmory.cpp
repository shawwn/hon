// (C)2006 S2 Games
// c_buildingarmory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_buildingarmory.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Building, Armory);
//=============================================================================

/*====================
  CBuildingArmory::CBuildingArmory
  ====================*/
CBuildingArmory::CBuildingArmory() :
IBuildingEntity(GetEntityConfig())
{
}
