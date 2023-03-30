// (C)2007 S2 Games
// c_petpredator.h
//
//=============================================================================
#ifndef __C_PETPREDATOR_H__
#define __C_PETPREDATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_petentity.h"
//=============================================================================

//=============================================================================
// CPetPredator
//=============================================================================
class CPetPredator : public IPetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Pet, Predator);

public:
	~CPetPredator()	{}
	CPetPredator() : IPetEntity(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_PETPREDATOR_H__
