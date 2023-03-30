// (C)2006 S2 Games
// c_gunscorch.h
//
//=============================================================================
#ifndef __C_GUNSCORCH_H__
#define __C_GUNSCORCH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// CGunScorch
//=============================================================================
class CGunScorch : public IGunItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Gun, Scorch);

public:
	~CGunScorch()	{}
	CGunScorch() :
	IGunItem(GetEntityConfig())
	{}

};
//=============================================================================

#endif //__C_GUNSCORCH_H__
