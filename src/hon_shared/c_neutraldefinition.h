// (C)2008 S2 Games
// c_neutraldefinition.h
//
//=============================================================================
#ifndef __C_NEUTRALDEFINITION_H__
#define __C_NEUTRALDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(INeutralEntity, Neutral, neutral)
//=============================================================================

//=============================================================================
// CNeutralDefinition
//=============================================================================
class CNeutralDefinition : public IUnitDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

public:
	~CNeutralDefinition()	{}
	CNeutralDefinition() :
	IUnitDefinition(&g_allocatorNeutral)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(ctx_Game,    CNeutralDefinition)(*this); }
};
//=============================================================================

#endif //__C_NEUTRALDEFINITION_H__
