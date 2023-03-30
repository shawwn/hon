/**********************************************************************
 *<
	FILE: meshadj.h

	DESCRIPTION: Adjacency list for meshes.

	CREATED BY: Rolf Berteig

	HISTORY:	Extended for Shiva by: Steve Anderson

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MESHADJ__
#define __MESHADJ__

#include "export.h"

#ifndef UNDEFINED
#define UNDEFINED	0xffffffff
#endif

class AdjFaceList;

class MEdge {
public:
	DWORD f[2];
	DWORD v[2];

	DllExport int EdgeIndex(Face *faces,int side);
	DllExport BOOL Selected (Face *faces,BitArray &esel);
	DllExport BOOL Visible (Face *faces);
	DllExport BOOL Hidden (Face *faces);
	DllExport Point3 Center(Point3 *verts);
	DllExport BOOL AFaceSelected(BitArray &fsel);
	DllExport Point3 ButterFlySubdivide (Mesh *mesh,AdjFaceList *af,float tens);
	DllExport UVVert ButterFlyMapSubdivide (Mesh *mesh,AdjFaceList *af,float tens, int mp, bool & seam, UVVert & side2);
	DWORD OtherVert (DWORD vv) { return (v[0] == vv) ? v[1] : v[0]; }
	DWORD OtherFace (DWORD ff) { return (f[0] == ff) ? f[1] : f[0]; }
};

class AdjEdgeList {
public:
	DWORDTab *list; 	// 1 DWORDTab per vertex. The Tab is a list of indices into the edge list, 1 for each edge adjacent to the vertex
	Tab<MEdge> edges;	// Table of edges
	int nverts;			// size of 'list'.

	DllExport AdjEdgeList(Mesh& amesh);
	DllExport ~AdjEdgeList();

	DllExport void AddEdge( DWORD fi, DWORD v1, DWORD v2 );
	DWORDTab& operator[](int i) { return list[i]; }
	DllExport int FindEdge(DWORD v0, DWORD v1);
	DllExport int FindEdge(DWORDTab& vmap,DWORD v0, DWORD v1);		
	DllExport void TransferEdges(DWORD from,DWORD to,DWORD except1,DWORD except2,DWORD del);
	DllExport void RemoveEdge(DWORD from,DWORD e);
	DllExport void OrderAllEdges (Face *faces);	// only affects order in each vert's list.
	DllExport void OrderVertEdges (DWORD v, Face *faces, Tab<DWORD> *flist=NULL);
	DllExport void GetFaceList (DWORD v, Tab<DWORD> & flist);

	DllExport void MyDebugPrint ();

	void AddEdgeToVertex(DWORD v,DWORD e) { list[v].Append (1, &e); }
};

class AdjFace {
public:
	DWORD f[3];
	AdjFace() {f[0] = f[1] = f[2] = UNDEFINED;}
};

class AdjFaceList {
public:
	Tab<AdjFace> list;

	AdjFace& operator[](int i) {return list[i];}
	DllExport AdjFaceList(Mesh& mesh,AdjEdgeList& el);
};

class FaceElementList {
public:
	// For each face, which element is it in
	DWORDTab elem;
	DWORD count;
	DllExport FaceElementList(Mesh &mesh,AdjFaceList& af);
	DWORD operator[](int i) {return elem[i];}
};

class FaceClusterList {
public:
	// Cluster #, one for each face - non-selected faces have UNDEFINED for their id.
	DWORDTab clust;
	DWORD count;

	// This version separates cluster also using a minimum angle and optionally the selection set
	DllExport FaceClusterList(Mesh *mesh, AdjFaceList& adj,float angle,BOOL useSel=TRUE);		
	// Uses selection set
	DllExport FaceClusterList (BitArray& fsel, AdjFaceList& adj);		
	DWORD operator[](int i) { return clust[i]; }
	DllExport void MakeVertCluster(Mesh &mesh, Tab<DWORD> & vclust);
	DllExport void GetNormalsCenters (Mesh &mesh, Tab<Point3> & norm, Tab<Point3> & ctr);
	DllExport void GetBorder (DWORD clustID, AdjFaceList & af, Tab<DWORD> & cbord);
	DllExport void GetOutlineVectors (Mesh & m, AdjFaceList & af, Tab<Point3> & cnorms, Tab<Point3> & odir);
};

class EdgeClusterList {
public:
	DWORDTab clust;
	DWORD count;

	DllExport EdgeClusterList(Mesh &mesh,BitArray &esel,AdjEdgeList &adj);
	DWORD ID(int f, int e) {return clust[f*3+e];}
	DWORD operator[](int i) {return clust[i];}
	DllExport void MakeVertCluster (Mesh &mesh, Tab<DWORD> & vclust);
	DllExport void GetNormalsCenters (Mesh &mesh, Tab<Point3> & norm, Tab<Point3> & ctr);
};

class MeshChamferData {
	Tab<UVVert> hmdir[NUM_HIDDENMAPS];
public:
	Tab<Point3> vdir;
	Tab<float> vmax;
	Tab<UVVert> *mdir;

	MeshChamferData () { mdir=NULL; }
	MeshChamferData (const Mesh & m) { mdir=NULL; InitToMesh(m); }
	DllExport ~MeshChamferData ();

	DllExport void InitToMesh (const Mesh & m);
	DllExport void setNumVerts (int nv, bool keep=TRUE, int resizer=0);
	Tab<UVVert> & MDir (int mp) { return (mp<0) ? hmdir[-1-mp] : mdir[mp]; }
};

// The following function has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
DllExport void MeshChamferDataDebugPrint (MeshChamferData & mcd, int mapNum);
// End of 3ds max 4.2 Extension

// following is never saved: it's a collection of all the temporary data you might want to cache about a mesh.
// Extrusion types:
#define MESH_EXTRUDE_CLUSTER 1
#define MESH_EXTRUDE_LOCAL 2

class MeshTempData : public BaseInterfaceServer {
private:
	AdjEdgeList *adjEList;
	AdjFaceList *adjFList;
	EdgeClusterList *edgeCluster;
	FaceClusterList *faceCluster;
	Tab<DWORD> *vertCluster;
	Tab<Point3> *normals;
	Tab<Point3> *centers;
	Tab<Point3> *vnormals;
	Tab<Tab<float> *> *clustDist;
	Tab<float> *selDist;
	Tab<float> *vsWeight;
	MeshChamferData *chamData;

	Tab<Point3> *extDir;
	Tab<Point3> *outlineDir;

	Mesh *mesh;

public:
	DllExport MeshTempData ();
	DllExport MeshTempData (Mesh *m);
	DllExport ~MeshTempData ();

	void SetMesh (Mesh *m) { mesh = m; }

	DllExport AdjEdgeList *AdjEList ();
	DllExport AdjFaceList *AdjFList ();
	DllExport FaceClusterList *FaceClusters ();
	DllExport EdgeClusterList *EdgeClusters ();
	DllExport Tab<DWORD> *VertexClusters (DWORD sl);
	DllExport Tab<Point3> *ClusterNormals (DWORD sl);
	DllExport Tab<Point3> *ClusterCenters (DWORD sl);
	DllExport Matrix3 ClusterTM (int clust);
	DllExport Tab<Point3> *VertexNormals ();
	DllExport Tab<float> *VSWeight (BOOL useEdgeDist, int edgeIts,
									BOOL ignoreBack, float falloff, float pinch, float bubble);
	DllExport Tab<float> *SelectionDist (BOOL useEdgeDist, int edgeIts);
	DllExport Tab<float> *ClusterDist (DWORD sl, int clustId, BOOL useEdgeDist, int edgeIts);
	DllExport Tab<Point3> *EdgeExtDir (Tab<Point3> *edir, int extrusionType);
	DllExport Tab<Point3> *FaceExtDir (int extrusionType);
	Tab<Point3> *CurrentExtDir () { return extDir; }
	DllExport Tab<Point3> *OutlineDir (int extrusionType);

	DllExport MeshChamferData *ChamferData();

	DllExport void Invalidate (DWORD part);
	DllExport void InvalidateDistances ();
	DllExport void InvalidateAffectRegion ();
	DllExport void freeClusterDist ();
	DllExport void freeBevelInfo ();
	DllExport void freeChamferData();
	DllExport void freeAll ();
};

// CAL-04/30/01: soft selection parameters.
#define SOFTSEL_MIN_FALLOFF		0.0f
#define SOFTSEL_MAX_FALLOFF		999999.0f
#define SOFTSEL_DEFAULT_FALLOFF	20.0f

#define SOFTSEL_MIN_PINCH		-1000.0f
#define SOFTSEL_MAX_PINCH		1000.0f
#define SOFTSEL_DEFAULT_PINCH	0.0f

#define SOFTSEL_MIN_BUBBLE		-1000.0f
#define SOFTSEL_MAX_BUBBLE		1000.0f
#define SOFTSEL_DEFAULT_BUBBLE	0.0f

DllExport float AffectRegionFunction (float dist, float falloff, float pinch, float bubble);
DllExport Point3 SoftSelectionColor (float selAmount);
DllExport void MatrixFromNormal (Point3& normal, Matrix3& mat);
DllExport void AverageVertexNormals (Mesh & mesh, Tab<Point3> & vnormals);
DllExport Point3 AverageSelVertNormal (Mesh& mesh);
DllExport Point3 AverageSelVertCenter (Mesh& mesh);
DllExport void DeselectHiddenFaces (Mesh &mesh);
DllExport void DeselectHiddenEdges (Mesh &mesh);
DllExport void HiddenFacesToVerts (Mesh &mesh, BitArray alsoHide);
DllExport void SelectionDistance (Mesh & mesh, float *selDist);
DllExport void SelectionDistance (Mesh & mesh, float *selDist, int iters, AdjEdgeList *ae=NULL);
DllExport void ClustDistances (Mesh & mesh, DWORD numClusts, DWORD *vclust,
							   Tab<float> **clustDist);
DllExport void ClustDistances (Mesh & mesh, DWORD numClusts, DWORD *vclust,
							   Tab<float> **clustDist, int iters, AdjEdgeList *ae=NULL);

// Mesh Selection Converter Interface - by Steve Anderson, October 2002
// For accurate selections of polygons and elements in Meshes in window region selections,
// (as well as correct by-vertex selections of edges and faces in window regions)
// we need to know not only which components were selected, but also which were culled,
// for instance by virtue of being back-facing.  We want to select only elements in which
// all faces were in the selection region, but we shouldn't eliminate elements just because 
// some of their faces were culled.

// So we provide this Mesh Selection Converter interface, which automatically stores culling
// information in a BitArray when Mesh::SubObjectHitTest is called.  Then this can be used
// to handle level conversions in Editable Mesh, Edit Mesh, and Mesh Select 
// SelectSubComponent routines.

#define MESHSELECTCONVERT_INTERFACE Interface_ID(0x3da7dd5, 0x7ecf0391)

// Selection converter flags
#define MESH_SELCONV_REQUIRE_ALL 0x01	// Set during Mesh hit-testing (with window region)

class MeshSelectionConverter : public BaseInterface {
public:
	DllExport MeshSelectionConverter ();

	// Our interesting methods:
	DllExport void FaceToElement (Mesh & mesh, AdjFaceList *af, BitArray & faceSel, BitArray & elementSel);
	DllExport void FaceToPolygon (Mesh & mesh, AdjFaceList *af, BitArray & faceSel, BitArray & polygonSel,
		float faceThresh, bool ignoreVisEdge=false);
	DllExport void VertexToEdge (Mesh & mesh, BitArray & vertexSel, BitArray & edgeSel);
	DllExport void VertexToFace (Mesh & mesh, BitArray & vertexSel, BitArray & faceSel);

	// Note - we can't just combine VertexToFace and FaceToPolygon, because we won't have accurate Face culling info.
	DllExport void VertexToPolygon (Mesh & mesh, AdjFaceList *af, BitArray & vertexSel, BitArray & polygonSel,
		float faceThresh, bool ignoreVisEdge=false);
	DllExport void VertexToElement (Mesh & mesh, AdjFaceList *af, BitArray & vertexSel, BitArray & elementSel);

	// Access which subobject components were culled in last Mesh hit-testing.
	BitArray & GetCull () { return mCull; }

	void SetFlag (DWORD f, bool value=true) { if (value) mFlags |= f; else mFlags &= ~f; }
	void ClearFlag(DWORD f) { mFlags &= ~f; }
	bool GetFlag (DWORD f) { return (mFlags&f) ? true : false; }

	// From BaseInterface:
	Interface_ID GetID() {return MESHSELECTCONVERT_INTERFACE;}
	DllExport void DeleteInterface();
	DllExport BaseInterface* GetInterface(Interface_ID id);
	DllExport BaseInterface* CloneInterface(void* remapDir = NULL);

private:
	DWORD mFlags;
	BitArray mCull;
	DWORD mCullLevel;
};


#endif //__MESHADJ__
