// (C)2006 S2 Games
// c_projectilebolt.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectilebolt.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Projectile, Bolt);
//=============================================================================

/*====================
  CProjectileBolt::CProjectileBolt
  ====================*/
CProjectileBolt::CProjectileBolt() :
IProjectile(GetEntityConfig())
{
}
