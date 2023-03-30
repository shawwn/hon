// (C)2008 S2 Games
// k0_intersection.h
//
// Geometry intersection routines
//=============================================================================
#ifndef __K0_INTERSECTION_H__
#define __K0_INTERSECTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_boundingbox.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSphere;
class CConvexPolyhedron;
//=============================================================================

//=============================================================================
// Functions
//=============================================================================
bool	I_LineMeshIntersect(const CVec3f &start, const CVec3f &end, const class CMesh *mesh,
					float &fFraction, int &face_hit);
bool	I_LineBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_IntersectBoxWithSurface(const CVec3f &start, const CVec3f &end, struct linkedSurface_s *surf, float &fraction);
bool	I_RayTriIntersect(const CVec3f &orig, const CVec3f &dir, const CVec3f &vert0, const CVec3f &vert1, const CVec3f &vert2, float &fDistance);
bool	I_SphereBoundsIntersect(const CSphere &sSphere, const CBBoxf &bbBox);
bool	I_LineTriangleIntersect(const CVec3f &start, const CVec3f &end, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction);
bool	I_LineQuadIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, const CVec3f &v4, float &fFraction);
bool	I_LineSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_LineTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_MovingBoundsBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_MovingBoundsSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_MovingBoundsTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CPlane aPlanes[], const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_LinePlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction);
bool	I_LineDoubleSidedPlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction);
bool	I_BoundsSurfaceIntersect(const CBBoxf &bbBounds, const CConvexPolyhedron &surface);
bool	I_SurfaceSurfaceIntersect(const CConvexPolyhedron &cSurface1, const CConvexPolyhedron &cSurface2);
bool	I_LineBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fEnter, float &fExit);
bool	I_MovingBoundsBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fEnter, float &fExit);
bool	I_LineBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
bool	I_MovingBoundsBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CVec3f &v3Diag, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
//=============================================================================
#endif //__K0_INTERSECTION_H__
