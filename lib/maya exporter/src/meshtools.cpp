// (C)2008 S2 Games
// meshtools.cpp
//
// useful mesh functions for max plugins
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "meshtools.h"
#include "misc.h"
//=============================================================================

#define	_FACEVTX(mesh, face, idx) mesh.verts[mesh.faces[face].v[idx]]

/*====================
  BaryToXYZ
  ====================*/
CVec3d	BaryToXYZ(double u, double v, CVec3d &v3A, CVec3d &v3B, CVec3d &v3C)
{
	return (v3A * (1.0 - u - v)) + (v3B * u) + (v3C * v);
}


/*====================
  TestMeshConvexity

  determines if a mesh is convex (may fail if the mesh is degenerate?)
  probably a better way to do this...
  ====================*/
bool	TestMeshConvexity(MFnMesh &fnMesh)
{
#if 0
	// Get the adjacency information
	AdjEdgeList adjedge = AdjEdgeList(mesh);
	AdjFaceList adj = AdjFaceList(mesh, adjedge);

	for (int iFace(0); iFace < fnMesh.numPolygons(); ++iFace)
	{
		// Get a point on the center of the triangle (could be any point on the triangle that isn't on an edge
		Point3 a = _FACEVTX(mesh, iFace, 0);
		CVec3d v3A(a.x, a.y, a.z);
		Point3 b = _FACEVTX(mesh, iFace, 0);
		CVec3d v3B(b.x, b.y, b.z);
		Point3 c = _FACEVTX(mesh, iFace, 0);
		CVec3d v3C(c.x, c.y, c.z);
		CVec3d v3PointInTri(BaryToXYZ(0.5, 0.5, v3A, v3B, v3C));

		for (int iAdj(0); iAdj < 3; ++iAdj)
		{
			int faceidx = adj[iFace].f[iAdj];
			if (faceidx == iFace)
				continue;

			if (faceidx == UNDEFINED)
			{
				cout << "UNDEFINED adjacent face " << iAdj << " to face " << iFace << endl;
				return false;
			}

			Point3 i = _FACEVTX(mesh, faceidx, 0);
			CVec3d v3I(i.x, i.y, i.z);
			Point3 j = _FACEVTX(mesh, faceidx, 1);
			CVec3d v3J(j.x, j.y, j.z);
			Point3 k = _FACEVTX(mesh, faceidx, 2);
			CVec3d v3K(k.x, k.y, k.z);

			// Test the triangle center against adjacent planes
			struct SPlaneD
			{
				CVec3d	m_v3Nml;
				double	m_dDist;
			};
			SPlaneD plane;
			plane.m_v3Nml = CrossProduct((v3J - v3I), (v3K - v3I));
			plane.m_v3Nml.Normalize();
			plane.m_dDist = DotProduct(plane.m_v3Nml, v3I);

			double dTest(DotProduct(v3PointInTri, plane.m_v3Nml) - plane.m_dDist);

			// FIXME:
			// Using a more forgiving epsilon here since it was failing often...
			const double _LOCAL_EPSILON(0.00001);

			if (dTest > _LOCAL_EPSILON)
			{
				cout << "plane test failed:" << endl;
				cout << "iFace: " << (iFace + 1) << ", iAdj: " << iAdj << " (" << (faceidx + 1) << "), nml: " << (plane.m_v3Nml) + ") dist: " + plane.m_dDist << endl;
				cout << "pit: " << XtoA(v3PointInTri) << endl;
				cout << "p0: " << XtoA(v3I) << endl;
				cout << "p1: " << XtoA(v3J)) << endl;
				cout << "p2: " << XtoA(v3K)) << endl;
				cout << "EQ: (" << XtoA(DotProduct(v3PointInTri, plane.m_v3Nml)) << " - " << XtoA(plane.m_dDist) << ") = " << XtoA(dTest, 0, 0, 9) << " > " << XtoA(_LOCAL_EPSILON, 0, 0, 9)) << endl;
				return false;
			}
		}
	}
#endif
	return true;
}
