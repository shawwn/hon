// (C)2008 S2 Games
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
//=============================================================================

//=============================================================================
// CEdge
//=============================================================================
class CEdge
{
public:
	CEdge() {}
	CEdge(const CVec3f &a, const CVec3f &b);

	void	Transform(const CAxis &aAxis, const CVec3f &v3Pos, float fScale);

	CVec3f	v3Normal;
	CVec3f	v3Point;
};
//=============================================================================

//=============================================================================
// Member functions
//=============================================================================

/*====================
  CEdge::CEdge
  ====================*/
inline
CEdge::CEdge(const CVec3f &a, const CVec3f &b) :
v3Normal(Normalize(b - a)),
v3Point(a)
{
}


/*====================
  CEdge::Transform
  ====================*/
inline
void	CEdge::Transform(const CAxis &aAxis, const CVec3f &v3Pos, float fScale)
{
	v3Point = TransformPoint(v3Point, aAxis, v3Pos, fScale);	// Transform point
	v3Normal = TransformPoint(v3Normal, aAxis);					// Rotate normal
	v3Normal.Normalize();										// Renormalized
}
//=============================================================================

#endif	//__C_EDGE_H__
