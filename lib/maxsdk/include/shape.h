/**********************************************************************
 *<
	FILE: shape.h

	DESCRIPTION:  Defines Basic BezierShape Object

	CREATED BY: Tom Hudson

	HISTORY: created 23 February 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __SHAPE__ 

#define __SHAPE__

#include "shphier.h"
#include "spline3d.h"
#include "shpsels.h"	// Shape selection classes

class BezierShape;

// Parameters for BezierShape::PerformTrimOrExtend
#define SHAPE_TRIM 0
#define SHAPE_EXTEND 1

//watje class contains the data for the bind points
class bindShape
{
public:
	int pointSplineIndex;      // spline that is to be bound
	int segSplineIndex;        // spline that contains the segment to bind to
	int seg;					// seg that the point is bound to
	Point3 bindPoint,segPoint;  // posiiton in space of the start seg point and the bind point
	BOOL isEnd;                 // is the bound point the end or beginning of the spline
};


class ShapeSubHitRec {
	private:		
		ShapeSubHitRec *next;
	public:
		DWORD	dist;
		BezierShape*	shape;
		int		poly;
		int		index;
		ShapeSubHitRec( DWORD dist, BezierShape *shape, int poly, int index, ShapeSubHitRec *next ) 
			{ this->dist = dist; this->shape = shape; this->poly = poly; this->index = index; this->next = next; }

		ShapeSubHitRec *Next() { return next; }		
	};

class SubShapeHitList {
	private:
		ShapeSubHitRec *first;
	public:
		SubShapeHitList() { first = NULL; }
		~SubShapeHitList() {
			ShapeSubHitRec *ptr = first, *fptr;
			while ( ptr ) {
				fptr = ptr;
				ptr = ptr->Next();
				delete fptr;
				}
			first = NULL;
			}	

		ShapeSubHitRec *First() { return first; }
		void AddHit( DWORD dist, BezierShape *shape, int poly, int index ) {
			first = new ShapeSubHitRec(dist,shape,poly,index,first);
			}
		int Count() {
			int count = 0;
			ShapeSubHitRec *ptr = first;
			while(ptr) {
				count++;
				ptr = ptr->Next();
				}
			return count;
			}
	};

// Special storage class for hit records so we can know which object was hit
class ShapeHitData : public HitData {
	public:
		BezierShape *shape;
		int poly;
		int index;
		ShapeHitData(BezierShape *shape, int poly, int index)
			{ this->shape = shape; this->poly = poly; this->index = index; }
		~ShapeHitData() {}
	};

// Display flags
#define DISP_VERTTICKS		(1<<0)
#define DISP_BEZHANDLES		(1<<1)
#define DISP_SELVERTS		(1<<10)
#define DISP_SELSEGMENTS	(1<<11)
#define DISP_SELPOLYS		(1<<13)
#define DISP_UNSELECTED		(1<<14)		// Used by loft -- Shape unselected
#define DISP_SELECTED		(1<<15)		// Used by loft -- Shape selected
#define DISP_ATSHAPELEVEL	(1<<16)		// Used by loft -- Shape at current level
#define DISP_VERT_NUMBERS	(1<<17)
#define DISP_VERT_NUMBERS_SELONLY	(1<<18)
#define DISP_SPLINES_ORTHOG (1<<25)     // Not display, but splines to be created orthog
// Selection level bits.
#define SHAPE_OBJECT		(1<<0)
#define SHAPE_SPLINE		(1<<1)
#define SHAPE_SEGMENT		(1<<2)
#define SHAPE_VERTEX		(1<<3)

// Flags for sub object hit test

// NOTE: these are the same bits used for object level.
#define SUBHIT_SHAPE_SELONLY	(1<<0)
#define SUBHIT_SHAPE_UNSELONLY	(1<<2)
#define SUBHIT_SHAPE_ABORTONHIT	(1<<3)
#define SUBHIT_SHAPE_SELSOLID	(1<<4)

#define SUBHIT_SHAPE_VERTS		(1<<24)
#define SUBHIT_SHAPE_SEGMENTS	(1<<25)
#define SUBHIT_SHAPE_POLYS		(1<<26)
#define SUBHIT_SHAPE_TYPEMASK	(SUBHIT_SHAPE_VERTS|SUBHIT_SHAPE_SEGMENTS|SUBHIT_SHAPE_POLYS)

// Callback used for retrieving other shapes in the current editing context
class ShapeContextCallback {
	public:
		virtual BezierShape *GetShapeContext(ModContext *context) = 0;
	};

class ShapeObject;

class BezierShapeTopology {
	public:
		BOOL ready;
		IntTab kcount;
		BitArray closed;
		BezierShapeTopology() { ready = FALSE; }
		CoreExport void Build(BezierShape &shape);
		CoreExport int operator==(const BezierShapeTopology& t);
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);
	};

// The following {class, member, macro, flag} has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

// CAL-04/12/01: Use area selection to jump between splines in soft selection
// To be merged into BezierShape in the future.
// Two public data members, mUseAreaSelect & mAreaSelect, in the interface extension class.
// Set mUseAreaSelect to TRUE to enable the use of area selection to jump between splines
// when doing soft selection with edge distance enabled. mAreaSelect is the distance
// allowed to jump from one node in a spline to another node in another spline.
#define BEZIER_SHAPE_INTERFACE_EX41 Interface_ID(0x23526cd1, 0x695d7e4e)
#define GET_BEZIER_SHAPE_INTERFACE_EX41() \
	(BezierShapeInterface_Ex41*) GetInterface(BEZIER_SHAPE_INTERFACE_EX41)

class BezierShapeInterface_Ex41 : public BaseInterface
{
public:
	BOOL mUseAreaSelect;		// TRUE if use area selection
	float mAreaSelect;			// Radius of area selection

	BaseInterface*	GetInterface(Interface_ID id) { if (id == BEZIER_SHAPE_INTERFACE_EX41) return (BaseInterface*)this; else return NULL; }
	Interface_ID	GetID() { return BEZIER_SHAPE_INTERFACE_EX41; }
	void			DeleteInterface() { delete this; }
};
// End of 3ds max 4.2 Extension

class BezierShape : public BaseInterfaceServer {
 		Box3			bdgBox;			// object space--depends on geom+topo
		static int shapeCount;			// Number of shape objects in the system!
		PolyShape pShape;				// PolyShape cache
		int pShapeSteps;				// Number of steps in the cache
		BOOL pShapeOptimize;			// TRUE if cache is optimized
		BOOL pShapeCacheValid;			// TRUE if the cache is current
		int *vertBase;					// Cache giving vert start index for each spline
		int totalShapeVerts;			// Total number of verts in the shape (cached)
		int *knotBase;					// Cache giving knot start index for each spline
		int totalShapeKnots;			// Total number of knots in the shape (cached)
		BOOL topoCacheValid;			// TRUE if topology cache is valid
		BezierShapeTopology topology;	// Topology cache
		
		// CAL-03/25/01: use area selection to jump between splines in soft selection
		//				 Make new APIs to access these in the future.
		// BOOL mUseAreaSelect;			// TRUE if use area selection
		// float mAreaSelect;				// Radius of area selection

		// CAL-04/12/01: move the above two private data members into interface extension
		//				 and use the following private methods to access them
		BOOL UseAreaSelect();
		void SetUseAreaSelect( BOOL useAreaSelect );

		float AreaSelect();
		void SetAreaSelect( float aresSelect );

		// CAL-03/25/01: Remove these friend declaration once their access to area selection
		//				 variables are replaced by the new APIs.
		friend INT_PTR CALLBACK SplineSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

	public:
		
		// Patch capping cache (mesh capping and hierarchy caches stored in PolyShape cache)
		PatchCapInfo patchCap;
		BOOL patchCapCacheValid;

		// The list of splines
		Spline3D **splines;
		int splineCount;

		int steps;						// Number of steps (-1 = adaptive)
		BOOL optimize;					// TRUE optimizes linear segments

		// Selection
		ShapeVSel	vertSel;  		// selected vertices
		ShapeSSel	segSel;  		// selected segments
		ShapePSel	polySel;  		// selected polygons

		// If hit bezier vector, this is its info:
		int bezVecPoly;
		int bezVecVert;

		// Selection level
		DWORD		selLevel;

		// Display attribute flags
		DWORD		dispFlags;

		CoreExport BezierShape();
		CoreExport BezierShape(BezierShape& fromShape);

		CoreExport void		Init();

		CoreExport ~BezierShape();

		CoreExport BezierShape& 		operator=(BezierShape& fromShape);
		CoreExport BezierShape& 		operator=(PolyShape& fromShape);
		
		CoreExport Point3	GetVert(int poly, int i);
		CoreExport void		SetVert(int poly, int i, const Point3 &xyz);
		
		CoreExport void		Render(GraphicsWindow *gw, Material *ma, RECT *rp, int compFlags, int numMat);
		CoreExport void		RenderGizmo(GraphicsWindow *gw);
		CoreExport BOOL		Select(GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit = FALSE);
		CoreExport void		Snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm);
		// See polyshp.h for snap flags
		CoreExport void		Snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm, DWORD flags);
		CoreExport BOOL 	SubObjectHitTest(GraphicsWindow *gw, Material *ma, HitRegion *hr,
								DWORD flags, SubShapeHitList& hitList );

		CoreExport void		BuildBoundingBox(void);
		CoreExport Box3		GetBoundingBox(Matrix3 *tm=NULL); // RB: optional TM allows the box to be calculated in any space.
				                                              // NOTE: this will be slower becuase all the points must be transformed.
		
		CoreExport void		InvalidateGeomCache();
		CoreExport void		InvalidateCapCache();
		CoreExport void		FreeAll(); //DS
				
		// functions for use in data flow evaluation
		CoreExport void 	ShallowCopy(BezierShape *ashape, ULONG_PTR channels);
			// WIN64 Cleanup: Shuler
		CoreExport void 	DeepCopy(BezierShape *ashape, ULONG_PTR channels);
			// WIN64 Cleanup: Shuler
		CoreExport void		NewAndCopyChannels(ULONG_PTR channels);
			// WIN64 Cleanup: Shuler
		CoreExport void 	FreeChannels( ULONG_PTR channels, int zeroOthers=1);
			// WIN64 Cleanup: Shuler

		// Display flags
		CoreExport void		SetDispFlag(DWORD f);
		CoreExport DWORD	GetDispFlag(DWORD f);
		CoreExport void		ClearDispFlag(DWORD f);

		// Constructs a vertex selection list based on the current selection level.
		// Can specify selection level optionally
		CoreExport BitArray VertexTempSel(int poly, int level = -1);
		// Constructs a vertex selection list for all polys based on the current selection level
		// This is a bitarray with a bit for each vertex in each poly in the shape.  If
		// includeVecs is set, it will set the bits for vectors associated with selected knots.
		// Specify a poly number or -1 for all.  Can specify selection level optionally
		CoreExport BitArray	VertexTempSelAll(int poly = -1, BOOL includeVecs = FALSE, int level = 0, BOOL forceSel = FALSE);

		// CAL-05/23/03: Constructs a vertex selection list based on the given flag set on the vertex.
		CoreExport BitArray VertexFlagSel(int poly, DWORD flag);

		CoreExport IOResult Save(ISave* isave);
		CoreExport IOResult Load(ILoad* iload);

		// BezierShape-specific methods

		inline int SplineCount() { return splineCount; }
		CoreExport Spline3D* GetSpline(int index);
		CoreExport Spline3D* NewSpline(int itype = KTYPE_CORNER,int dtype = KTYPE_BEZIER,int ptype = PARM_UNIFORM);
		CoreExport Spline3D* AddSpline(Spline3D* spline);
		CoreExport int DeleteSpline(int index);
		CoreExport int InsertSpline(Spline3D* spline, int index);
		CoreExport void NewShape();
		CoreExport int GetNumVerts();
		CoreExport int GetNumSegs();
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		CoreExport void UpdateSels(BOOL save=FALSE);
		CoreExport void GetClosures(BitArray& array);
		CoreExport void SetClosures(BitArray& array);
		CoreExport float FindSegmentPoint(int poly, int segment, GraphicsWindow *gw, Material *ma, HitRegion *hr, int ptype = PARAM_SIMPLE);
		CoreExport void Reverse(int poly, BOOL keepZero = FALSE);
		CoreExport void Reverse(BitArray &reverse, BOOL keepZero = FALSE);
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier = NULL);
		CoreExport void MakePolyShape(PolyShape &pshp, int steps = -1, BOOL optimize = FALSE);
		CoreExport void MakeFirst(int poly, int vertex);
		CoreExport void Transform(Matrix3 &tm);
		CoreExport BezierShape& operator+=(BezierShape& from);
		CoreExport void AddAndWeld(BezierShape &from, float weldThreshold);
		CoreExport void ReadyCachedPolyShape();
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);
		CoreExport int ReadyPatchCap();
		
		// The following copies the shapes, selection sets and any caches from the source object.
		// It does NOT copy selection level info or display info.
		CoreExport void CopyShapeDataFrom(BezierShape &fromShape);

		// The following methods provide an easy way to derive a simple index for any
		// vert in any spline in the shape, and turn that index back into a poly/vert pair
		CoreExport void PrepVertBaseIndex();	// Used internally automatically
		CoreExport int GetVertIndex(int poly, int vert);
		CoreExport void GetPolyAndVert(int index, int &polyOut, int &vertOut);
		CoreExport int GetTotalVerts();	// Total number of verts in the shape
		CoreExport Point3	GetVert( int index ); // returns a point using global indexing scheme
		CoreExport void		SetVert( int index , const Point3 &xyz); // sets a point using global indexing scheme

		// The following methods provide an easy way to derive a simple index for any
		// knot in any spline in the shape, and turn that index back into a poly/knot pair
		CoreExport void PrepKnotBaseIndex();	// Used internally automatically
		CoreExport int GetKnotIndex(int poly, int knot);
		CoreExport void GetPolyAndKnot(int index, int &polyOut, int &knotOut);
		CoreExport int GetTotalKnots();	// Total number of knots in the shape

		// The following functions delete the selected items, returning TRUE if any were
		// deleted, or FALSE if none were deleted.
		CoreExport BOOL DeleteSelVerts(int poly);	// For single poly
		CoreExport BOOL DeleteSelSegs(int poly);	// For single poly
		CoreExport BOOL DeleteSelectedVerts();	// For all polys
		CoreExport BOOL DeleteSelectedSegs();
		CoreExport BOOL DeleteSelectedPolys();

		// Copy the selected geometry (segments or polys), reversing if needed.
		// Returns TRUE if anything was copied
		CoreExport BOOL CloneSelectedParts(BOOL reverse=FALSE);

		// Tag the points in the spline components to record our topology (This stores
		// identifying values in the Spline3D's Knot::aux fields for each control point)
		// This info can be used after topology-changing operations to remap information
		// tied to control points.
		// Returns FALSE if > 32767 knots or polys (can't record that many)
		// 
		// MAXr3: Optional channel added.  0=aux2, 1=aux3
		CoreExport BOOL RecordTopologyTags(int channel=0);

		// Support for interpolating along the shape's splines
		CoreExport Point3 InterpCurve3D(int poly, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentCurve3D(int poly, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 InterpPiece3D(int poly, int piece, float param, int ptype=PARAM_SIMPLE);
		CoreExport Point3 TangentPiece3D(int poly, int piece, float param, int ptype=PARAM_SIMPLE);
		CoreExport MtlID GetMatID(int poly, int piece);
		CoreExport float LengthOfCurve(int poly);

		// Get information on shape topology
		CoreExport void GetTopology(BezierShapeTopology &topo);

		// Perform a trim or extend
		CoreExport BOOL PerformTrimOrExtend(IObjParam *ip, ViewExp *vpt, ShapeHitData *hit, IPoint2 &m, ShapeContextCallback &cb, int trimType, int trimInfinite);

		CoreExport BOOL SelVertsSameType();	// Are all selected vertices the same type?
		CoreExport BOOL SelSegsSameType();	// Are all selected segments the same type?
		CoreExport BOOL SelSplinesSameType();	// Are all segments in selected splines the same type?

//watje 1-10-98 additonal changes

		Tab<bindShape> bindList;		//list of bind points
		CoreExport void BindKnot(BOOL isEnd, int segIndex, int splineSegID, int splinePointID); // binds a knot to a seg ment
		CoreExport BOOL UnbindKnot(int splineID, BOOL isEnd);  //unbind a knot 
		CoreExport void UpdateBindList(BOOL useGeometricTracking = FALSE);	//when topology changes this needs to be called to update the bind list
		CoreExport BOOL HideSelectedSegs();  // hide selected segs
		CoreExport BOOL HideSelectedVerts(); // hide segs attached to selected verts
		CoreExport BOOL HideSelectedSplines(); // hide segs attached to selected splines
		CoreExport BOOL UnhideSegs();          //unhide all segs

		CoreExport int  UnselectHiddenVerts();
		CoreExport int  UnselectHiddenSegs();
		CoreExport int  UnselectHiddenSplines();

		float * mpVertexWeights;
		int numVertexWeights;
		int   * mpVertexEdgeDists;
		float * mpVertexDists;

		public:
		CoreExport void  SetVertexWeightCount( int i ); // destroys existing weights, sets all weights = 0.0.
		CoreExport void  SetVertexWeight( int i, float w ) { assert( mpVertexWeights ); if ( i >= numVertexWeights ) return; mpVertexWeights[i] = w; }
		CoreExport float VertexWeight( int i ) { if ( !mpVertexWeights ) return 0.0f; if ( i >= numVertexWeights ) return 0.0f; return mpVertexWeights[i]; }
		CoreExport bool  VertexWeightSupport() { if ( mpVertexWeights ) return true; return false; }
		CoreExport float *GetVSelectionWeights() { return mpVertexWeights; }
		CoreExport void SupportVSelectionWeights();		// Allocate a weight table if none 

        float mFalloff, mPinch, mBubble;
		int   mEdgeDist, mUseEdgeDist, mAffectBackface, mUseSoftSelections;

		CoreExport int  UseEdgeDists( );
		CoreExport void SetUseEdgeDists( int edgeDist );

		CoreExport int  EdgeDist( );
		CoreExport void SetEdgeDist( int edgeDist );

		CoreExport int  UseSoftSelections();
		CoreExport void SetUseSoftSelections( int useSoftSelections );

		CoreExport int AffectBackface( );
		CoreExport void SetAffectBackface( int affectBackface );

		CoreExport float Falloff( );
		CoreExport void SetFalloff( float falloff );

		CoreExport float Pinch( );
		CoreExport void SetPinch( float pinch );

		CoreExport float Bubble( );
		CoreExport void SetBubble( float bubble );

		CoreExport void InvalidateVertexWeights();

		CoreExport void UpdateVertexDists();
		CoreExport void UpdateEdgeDists( );
		CoreExport void UpdateVertexWeights();

		CoreExport Point3 VertexNormal( int vIndex ); 

	};

#endif // __SHAPE__
