// (C)2005 S2 Games
// c_edge.h
//
// An edge
//=============================================================================
#ifndef __C_EDGE_H__
#define __C_EDGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_matrix3x3.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// CEdge
//=============================================================================
class CEdge
{
public:
	CEdge() {}
	K2_API CEdge(const CVec3f &a, const CVec3f &b);

	K2_API void	Transform(const CAxis &aAxis, const CVec3f &v3Pos, float fScale);

	CVec3f	v3Normal;
	CVec3f	v3Point;
};
//=============================================================================
#endif	//__C_EDGE_H__
