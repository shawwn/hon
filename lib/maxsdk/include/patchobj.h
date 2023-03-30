/**********************************************************************
 *<
	FILE: patchobj.h

	DESCRIPTION:  Defines Patch Mesh Object

	CREATED BY: Tom Hudson

	HISTORY: created 21 June 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __PATCHOBJ__ 

#define __PATCHOBJ__

#include "Max.h"
#include "namesel.h"
#include "meshlib.h"
#include "patchlib.h"
#include "snap.h"
#include "istdplug.h"
#include "sbmtlapi.h"

extern CoreExport Class_ID patchObjectClassID;

extern HINSTANCE hInstance;

// Named selection set list types for PatchObject
#define NS_PO_VERT 0
#define NS_PO_EDGE 1
#define NS_PO_PATCH 2

// set the default pick box size
#define DEF_PICKBOX_SIZE	4

// These are values for selLevel.
#define PO_OBJECT	0
#define PO_VERTEX	1
#define PO_EDGE		2
#define PO_PATCH	3
#define PO_ELEMENT	4
#define PO_HANDLE	5
#define PO_LEVELS	6

#define CID_EP_BIND	CID_USER + 203
#define CID_EP_EXTRUDE	CID_USER + 204
#define CID_EP_BEVEL	CID_USER + 205
#define CID_EP_NORMAL_FLIP	CID_USER + 206
#define CID_CREATE_VERT  CID_USER + 207
#define CID_CREATE_PATCH CID_USER + 208
#define CID_VERT_WELD CID_USER + 209
// CAL-06/02/03: copy/paste tangent. (FID #827)
#define CID_COPY_TANGENT CID_USER + 210
#define CID_PASTE_TANGENT CID_USER + 211

// Flags:
// Disp Result keeps track of "Show End Result" button for this Editable Patch
#define EP_DISP_RESULT 0x0100

// References
#define EP_MASTER_CONTROL_REF 0
#define EP_VERT_BASE_REF 1

class PatchObject;


class EP_BindMouseProc : public MouseCallBack {
	private:
		PatchObject *pobj;
		IObjParam *ip;
		IPoint2 om;
		BitArray knotList;
	
	protected:
		HCURSOR GetTransformCursor();
		BOOL HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert);
		BOOL HitASegment(ViewExp *vpt, IPoint2 *p, int *Seg);

		BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
		BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }		

	public:
		EP_BindMouseProc(PatchObject* spl, IObjParam *i) { pobj=spl; ip=i; }
		int proc( 
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};



class EP_BindCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_BindMouseProc eproc;
		PatchObject* pobj;
//		int type; // See above

	public:
		EP_BindCMode(PatchObject* spl, IObjParam *i) :
			fgProc((ReferenceTarget*)spl), eproc(spl,i) {pobj=spl;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_EP_BIND; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
//		void SetType(int type) { this->type = type; eproc.SetType(type); }
	};


class EP_ExtrudeMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EP_ExtrudeMouseProc(PatchObject* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EP_ExtrudeSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EP_ExtrudeSelectionProcessor(EP_ExtrudeMouseProc *mc, PatchObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EP_ExtrudeCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_ExtrudeSelectionProcessor mouseProc;
	EP_ExtrudeMouseProc eproc;
	PatchObject* po;

public:
	EP_ExtrudeCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_EXTRUDE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};

class EP_NormalFlipMouseProc : public MouseCallBack {
private:
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	Point3 ndir;
public:
	EP_NormalFlipMouseProc(PatchObject* o, IObjParam *i) {po=o;ip=i;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAPatch(ViewExp *vpt, IPoint2 *p, int *pix);
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EP_NormalFlipCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_NormalFlipMouseProc eproc;
	PatchObject* po;

public:
	EP_NormalFlipCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_NORMAL_FLIP; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};


class EP_BevelMouseProc : public MouseCallBack {
private:
	MoveTransformer moveTrans;
	PatchObject *po;
	Interface *ip;
	IPoint2 om;
	
public:
	EP_BevelMouseProc(PatchObject* o, IObjParam *i) : moveTrans(i) {po=o;ip=i;}
	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m);
};


class EP_BevelSelectionProcessor : public GenModSelectionProcessor {
protected:
	HCURSOR GetTransformCursor();
public:
	EP_BevelSelectionProcessor(EP_BevelMouseProc *mc, PatchObject *o, IObjParam *i) 
		: GenModSelectionProcessor(mc,(BaseObject*) o,i) {}
};


class EP_BevelCMode : public CommandMode {
private:
	ChangeFGObject fgProc;
	EP_BevelSelectionProcessor mouseProc;
	EP_BevelMouseProc eproc;
	PatchObject* po;

public:
	EP_BevelCMode(PatchObject* o, IObjParam *i) :
		fgProc((ReferenceTarget *)o), mouseProc(&eproc,o,i), eproc(o,i) {po=o;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_EP_BEVEL; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints=3; return &mouseProc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
};


class EP_CreateVertMouseProc : public MouseCallBack {
private:		
	PatchObject *po;
	IObjParam *mpIP;		
public:
	EP_CreateVertMouseProc(PatchObject* mod, IObjParam *i) {po=mod;mpIP=i;}
	int proc (HWND hwnd, int msg, int point, int flags, IPoint2 m);
};

class EP_CreateVertCMode : public CommandMode {
private:
	ChangeFGObject fgProc;		
	EP_CreateVertMouseProc proc;
	PatchObject *po;

public:
	EP_CreateVertCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_VERT; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=1; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};

class EP_CreatePatchMouseProc : public MouseCallBack {
public:
	PatchObject *po;
	IObjParam *mpIP;		
	int verts[4];
	IPoint2 anchor, lastPoint, startPoint;

	EP_CreatePatchMouseProc(PatchObject* mod, IObjParam *i);
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EP_CreatePatchCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	PatchObject *po;
	EP_CreatePatchMouseProc proc;

	EP_CreatePatchCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_CREATE_PATCH; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=5; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};


class EP_VertWeldMouseProc : public MouseCallBack {
public:
	PatchObject *po;
	IObjParam *mpIP;		
	int fromVert, toVert;
	IPoint2 anchor, lastPoint;

	EP_VertWeldMouseProc(PatchObject* mod, IObjParam *i){po=mod;mpIP=i;}
	BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, int subType );
	BOOL HitAVert(ViewExp *vpt, IPoint2 *p, int& vert);

	int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m );
};

class EP_VertWeldCMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	PatchObject *po;
	EP_VertWeldMouseProc proc;

	EP_VertWeldCMode(PatchObject* mod, IObjParam *i) : 
	  fgProc((ReferenceTarget *)mod), proc(mod,i) {po=mod;}
	int Class() { return MODIFY_COMMAND; }
	int ID() { return CID_VERT_WELD; }
	MouseCallBack *MouseProc(int *numPoints) {*numPoints=2; return &proc;}
	ChangeForegroundCallback *ChangeFGProc() {return &fgProc;}
	BOOL ChangeFG(CommandMode *oldMode) {return oldMode->ChangeFGProc()!= &fgProc;}
	void EnterMode();
	void ExitMode();
};


/*-------------------------------------------------------------------*/
// CAL-06/02/03: copy/paste tangent modes. (FID #827)

class EP_CopyTangentMouseProc : public MouseCallBack {
	private:
		PatchObject *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EP_CopyTangentMouseProc(PatchObject* obj, IObjParam *i) { po=obj; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EP_CopyTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_CopyTangentMouseProc eproc;
		PatchObject* po;

	public:
		EP_CopyTangentCMode(PatchObject* obj, IObjParam *i) :
			fgProc((ReferenceTarget*)obj), eproc(obj,i) {po=obj;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_COPY_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};

class EP_PasteTangentMouseProc : public MouseCallBack {
	private:
		PatchObject *po;
		IObjParam *ip;

	protected:
		HCURSOR GetTransformCursor();
		HitRecord* HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );

	public:
		EP_PasteTangentMouseProc(PatchObject* obj, IObjParam *i) { po=obj; ip=i; }
		int proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m );
	};

class EP_PasteTangentCMode : public CommandMode {
	private:
		ChangeFGObject fgProc;
		EP_PasteTangentMouseProc eproc;
		PatchObject* po;

	public:
		EP_PasteTangentCMode(PatchObject* obj, IObjParam *i) :
			fgProc((ReferenceTarget*)obj), eproc(obj,i) {po=obj;}

		int Class() { return MODIFY_COMMAND; }
		int ID() { return CID_PASTE_TANGENT; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
		ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
		BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
		void EnterMode();
		void ExitMode();
	};


/*-------------------------------------------------------------------*/

class POPickPatchAttach : 
		public PickModeCallback,
		public PickNodeCallback {
	public:		
		PatchObject *po;
		
		POPickPatchAttach() {po=NULL;}

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);

		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);

		HCURSOR GetHitCursor(IObjParam *ip);

		BOOL Filter(INode *node);
		
		PickNodeCallback *GetFilter() {return this;}

		BOOL RightClick(IObjParam *ip,ViewExp *vpt)	{return TRUE;}
	};


class SingleRefMakerPatchNode;
class SingleRefMakerPatchMtl;

// The Base Patch class
class PatchObject: public GeomObject, IPatchOps, IPatchSelect, IPatchSelectData, ISubMtlAPI, AttachMatDlgUser {
	friend class PatchObjectRestore;
	friend class POXFormProc;
	friend class POPickPatchAttach;
	public:
		static HWND hSelectPanel, hOpsPanel, hSurfPanel, hSoftSelPanel;
		static BOOL rsSel, rsOps, rsSurf;	// rollup states (FALSE = rolled up)
		static MoveModBoxCMode *moveMode;
		static RotateModBoxCMode *rotMode;
		static UScaleModBoxCMode *uscaleMode;
		static NUScaleModBoxCMode *nuscaleMode;
		static SquashModBoxCMode *squashMode;
		static SelectModBoxCMode *selectMode;
//watje command mode for the bind 		
		static EP_BindCMode *bindMode;
//watje command mode for the extrude 		
		static EP_ExtrudeCMode *extrudeMode;
		static EP_NormalFlipCMode *normalFlipMode;
		static EP_BevelCMode *bevelMode;
		static EP_CreateVertCMode *createVertMode;
		static EP_CreatePatchCMode *createPatchMode;
		static EP_VertWeldCMode *vertWeldMode;
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		static EP_CopyTangentCMode *copyTangentMode;
		static EP_PasteTangentCMode *pasteTangentMode;

		// for the tessellation controls
		static BOOL settingViewportTess;  // are we doing viewport or renderer
		static BOOL settingDisp;  // are we doing viewport or renderer
		static ISpinnerControl *uSpin;
		static ISpinnerControl *vSpin;
		static ISpinnerControl *edgeSpin;
		static ISpinnerControl *angSpin;
		static ISpinnerControl *distSpin;
		static ISpinnerControl *mergeSpin;
		static ISpinnerControl *matSpin;
		static ISpinnerControl *matSpinSel;
		// General rollup controls
		static ISpinnerControl *weldSpin;
		static ISpinnerControl *targetWeldSpin;
		static ISpinnerControl *stepsSpin;
#ifndef NO_OUTPUTRENDERER
		//3-18-99 watje to support render steps
		static ISpinnerControl *stepsRenderSpin;
#endif // NO_OUTPUTRENDERER
// 7/20/00 TH -- Relax controls
		static ISpinnerControl *relaxSpin;
		static ISpinnerControl *relaxIterSpin;

		static POPickPatchAttach pickCB;
		static BOOL patchUIValid;
		static BOOL opsUIValid;
		static int attachMat;
		static BOOL condenseMat;
		// selection box size 
		static int pickBoxSize;
		// selection box size for target weld 
		static int weldBoxSize;

		// Load reference version
		int loadRefVersion;

		Interval geomValid;
		Interval topoValid;
		Interval texmapValid;
		Interval selectValid;
		DWORD_PTR validBits; // for the remaining constant channels
			// WIN64 Cleanup: Shuler
		void CopyValidity(PatchObject *fromOb, ChannelMask channels);

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );

		static IObjParam *ip;		
		static PatchObject *editObj;

		PatchMesh patch;
		int patchSelSubType;		// PO_PATCH or PO_ELEMENT

		MasterPointControl	*masterCont;		// Master track controller
		Tab<Control*> vertCont;
		Tab<Control*> vecCont;

		// Remembered info
		PatchMesh *rememberedPatch;	// NULL if using all selected patches
		int rememberedIndex;
		int rememberedData;

		// Editing stuff:
		BOOL doingHandles;

		BOOL showMesh;
		BOOL propagate;

		BOOL inExtrude,inBevel, inPatchCreate;

		DWORD epFlags;

		// CAL-06/02/03: copy/paste tangent info. (FID #827)
		BOOL copyTanLength;
		BOOL tangentCopied;
		Point3 copiedTangent;

		// Named selection sets:
		GenericNamedSelSetList hselSet;  // Handle // CAL-06/10/03: (FID #1914)
		GenericNamedSelSetList vselSet;  // Vertex
		GenericNamedSelSetList eselSet;  // Edge
		GenericNamedSelSetList pselSet;  // Patch

		// additonal references
		SingleRefMakerPatchNode* noderef;                  
		SingleRefMakerPatchMtl* mtlref; 

		CoreExport PatchObject();
		CoreExport PatchObject(PatchObject &from);

		CoreExport void PatchObjectInit();	// Constructor helper

		CoreExport ~PatchObject();

		CoreExport PatchObject &operator=(PatchObject &from);

		// Flag methods.
		void SetFlag (DWORD fl, BOOL val=TRUE) { if (val) epFlags |= fl; else epFlags &= ~fl; }
		void ClearFlag (DWORD fl) { epFlags &= (~fl); }
		bool GetFlag (DWORD fl) { return (epFlags&fl) ? TRUE : FALSE; }

		//  inherited virtual methods:

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport RefTargetHandle Clone(RemapDir& remap = NoRemap());
		// Gizmo versions:
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		CoreExport void GetWorldBoundBox (TimeValue t, INode * inode, ViewExp* vp, Box3& box, ModContext *mc);

		// NS: New SubObjType API
		int NumSubObjTypes();
		ISubObjType *GetSubObjType(int i);

		// From Object			 
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport BOOL GetExtendedProperties(TimeValue t, TSTR &prop1Label, TSTR &prop1Data, TSTR &prop2Label, TSTR &prop2Data);
        CoreExport BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);

		// get and set the validity interval for the nth channel
	   	CoreExport Interval ChannelValidity(TimeValue t, int nchan);
		CoreExport void SetChannelValidity(int i, Interval v);
		CoreExport void InvalidateChannels(ChannelMask channels);
		CoreExport void TopologyChanged(); // mjm - 5.6.99

		// Convert-to-type validity
		CoreExport Interval ConvertValidity(TimeValue t);

		// Deformable object procs	
		virtual int IsDeformable() { return 1; }  
		CoreExport int NumPoints();
		CoreExport Point3 GetPoint(int i);
		CoreExport void SetPoint(int i, const Point3& p);
		CoreExport BOOL IsPointSelected (int i);
		
		CoreExport void PointsWereChanged();
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );
		CoreExport void Deform(Deformer *defProc, int useSel);

		virtual BOOL IsParamSurface() {return TRUE;}
		CoreExport  Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);

		// Mappable object procs
		virtual int IsMappable() { return 1; }
		virtual int NumMapChannels () { return patch.NumMapChannels (); }
		virtual int NumMapsUsed () { return patch.getNumMaps(); }
		virtual void ApplyUVWMap(int type, float utile, float vtile, float wtile,
			int uflip, int vflip, int wflip, int cap,const Matrix3 &tm,int channel=1) {
				patch.ApplyUVWMap(type,utile,vtile,wtile,uflip,vflip,wflip,cap,tm,channel); }

		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		CoreExport void FreeChannels(ChannelMask chan);
		CoreExport Object *MakeShallowCopy(ChannelMask channels);
		CoreExport void ShallowCopy(Object* fromOb, ChannelMask channels);
		CoreExport void NewAndCopyChannels(ChannelMask channels);

		CoreExport DWORD GetSubselState();

		// From GeomObject
		CoreExport int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		CoreExport void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport int IsInstanceDependent();	// Not view-dependent (yet)
		CoreExport Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

		CoreExport PatchMesh& GetPatchMesh(TimeValue t);
		CoreExport Mesh& GetMesh(TimeValue t);

		// Animatable methods

		virtual void DeleteThis() { delete this; }
		virtual void FreeCaches() {patch.InvalidateGeomCache(); }
		virtual Class_ID ClassID() { return Class_ID(PATCHOBJ_CLASS_ID,0); }
		CoreExport BOOL IsSubClassOf(Class_ID classID);
		CoreExport virtual void GetClassName(TSTR& s);
        
        using GeomObject::GetInterface;
		CoreExport void* GetInterface(ULONG id);
		
		// This is the name that will appear in the history browser.
		CoreExport virtual TCHAR *GetObjectName();

		// Controller stuff for animatable points
		int NumSubs()  { return 1; }	// Just tell it about the master point controller
		CoreExport Animatable* SubAnim(int i);
		CoreExport TSTR SubAnimName(int i);
		CoreExport BOOL AssignController(Animatable *control,int subAnim);
		int SubNumToRefNum(int subNum) {return subNum;}
		CoreExport BOOL SelectSubAnim(int subNum);
		CoreExport BOOL HasControllers(BOOL assertCheck=TRUE);

		// Reference methods
		CoreExport void RescaleWorldUnits(float f);
		int NumRefs() {return vertCont.Count() + vecCont.Count() + 1;}	// vert conts + vec conts + master
		CoreExport RefTargetHandle GetReference(int i);
		CoreExport void SetReference(int i, RefTargetHandle rtarg);
		CoreExport int RemapRefOnLoad(int iref);
		CoreExport void PlugControllersSel(TimeValue t);
		CoreExport void AllocVertContArray(int count);
		CoreExport void AllocVecContArray(int count);
		CoreExport void AllocContArrays(int vertCount, int vecCount);
		CoreExport void ReplaceVertContArray(Tab<Control *> &nc);
		CoreExport void ReplaceVecContArray(Tab<Control *> &nc);
		CoreExport void ReplaceContArrays(Tab<Control *> &vertnc, Tab<Control *> &vecnc);
		CoreExport BOOL PlugVertControl(TimeValue t,int i);
		CoreExport BOOL PlugVecControl(TimeValue t,int i);
		CoreExport void SetVertAnim(TimeValue t, int point, Point3 pt);
		CoreExport void SetVecAnim(TimeValue t, int point, Point3 pt);
		CoreExport void SetVertCont(int i, Control *c);
		CoreExport void SetVecCont(int i, Control *c);

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		// PatchObject-specific methods
		CoreExport virtual void UpdatePatchMesh(TimeValue t);
		CoreExport void PrepareMesh(TimeValue t);
		CoreExport Mesh& GetUnrelaxedMesh(TimeValue t);	// Ignore relax!
		CoreExport BOOL ShowLattice() { return patch.GetDispFlag(DISP_LATTICE) ? TRUE : FALSE; }
		CoreExport BOOL ShowVerts() { return patch.GetDispFlag(DISP_VERTS) ? TRUE : FALSE; }
		CoreExport void SetShowLattice(BOOL sw) { if(sw) patch.SetDispFlag(DISP_LATTICE); else patch.ClearDispFlag(DISP_LATTICE); }
		CoreExport void SetShowVerts(BOOL sw) { if(sw) patch.SetDispFlag(DISP_VERTS); else patch.ClearDispFlag(DISP_VERTS); }
		CoreExport void SetMeshSteps(int steps);
		CoreExport int GetMeshSteps();
#ifndef NO_OUTPUTRENDERER
		//3-18-99 watje to support render steps
		CoreExport void SetMeshStepsRender(int steps);
		CoreExport int GetMeshStepsRender();
#endif // NO_OUTPUTRENDERER
		CoreExport BOOL Relaxing();		// returns TRUE if Relax && RelaxValue != 0 && RelaxIter != 0
		CoreExport void SetRelax(BOOL v);
		CoreExport void SetRelaxViewports(BOOL v);
		CoreExport void SetRelaxValue(float v);
		CoreExport void SetRelaxIter(int v);
		CoreExport void SetRelaxBoundary(BOOL v);
		CoreExport void SetRelaxSaddle(BOOL v);
		BOOL GetRelax() { return patch.GetRelax(); }
		BOOL GetRelaxViewports() { return patch.GetRelaxViewports(); }
		float GetRelaxValue() { return patch.GetRelaxValue(); }
		int GetRelaxIter() { return patch.GetRelaxIter(); }
		BOOL GetRelaxBoundary() { return patch.GetRelaxBoundary(); }
		BOOL GetRelaxSaddle() { return patch.GetRelaxSaddle(); }

		CoreExport void SetShowInterior(BOOL si);
		CoreExport BOOL GetShowInterior();

		CoreExport void SetUsePatchNormals(BOOL usePatchNorm);
		CoreExport BOOL GetUsePatchNormals();

		CoreExport void SetAdaptive(BOOL sw);
		CoreExport BOOL GetAdaptive();
		CoreExport void SetViewTess(TessApprox tess);
		CoreExport TessApprox GetViewTess();
		CoreExport void SetProdTess(TessApprox tess);
		CoreExport TessApprox GetProdTess();
		CoreExport void SetDispTess(TessApprox tess);
		CoreExport TessApprox GetDispTess();
		CoreExport BOOL GetViewTessNormals();
		CoreExport void SetViewTessNormals(BOOL use);
		CoreExport BOOL GetProdTessNormals();
		CoreExport void SetProdTessNormals(BOOL use);
		CoreExport BOOL GetViewTessWeld();
		CoreExport void SetViewTessWeld(BOOL weld);
		CoreExport BOOL GetProdTessWeld();
		CoreExport void SetProdTessWeld(BOOL weld);
		CoreExport void InvalidateMesh();
		CoreExport void InvalidateRelaxedMesh();

		// Editable patch stuff follows...
		CoreExport virtual void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		CoreExport virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport int GetSubobjectLevel();
		CoreExport int GetSubobjectType();	// returns PO_PATCH for PO_ELEMENT level
		CoreExport void SetSubobjectLevel(int level);
		CoreExport void ActivateSubobjSel(int level, XFormModes& modes );
		CoreExport int SubObjectIndex(HitRecord *hitRec);
		CoreExport void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		CoreExport void ShowEndResultChanged (BOOL showEndResult);
		CoreExport void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		int NeedUseSubselButton() { return 0; }
		CoreExport void SelectSubComponent( HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert );
		CoreExport void ClearSelection(int selLevel);
		CoreExport void SelectAll(int selLevel);
		CoreExport void InvertSelection(int selLevel);
		
		CoreExport void PatchSelChanged();
		CoreExport void InvalidateSurfaceUI();
		CoreExport void InvalidateOpsUI();

		CoreExport void ChangeRememberedPatch(int type);
		CoreExport void ChangeSelPatches(int type);
		CoreExport int RememberPatchThere(HWND hWnd, IPoint2 m);
		CoreExport void SetRememberedPatchType(int type);
		CoreExport void ChangeRememberedVert(int type);
		CoreExport void ChangeSelVerts(int type);
		CoreExport int RememberVertThere(HWND hWnd, IPoint2 m);
		CoreExport void SetRememberedVertType(int type);

		// Generic xform procedure.
		CoreExport void XFormVerts( POXFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis );

		// Specialized xform for bezier handles
		CoreExport void XFormHandles( POXFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis, int handleIndex );

		// Affine transform methods		
		CoreExport void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
		CoreExport void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE );
		CoreExport void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );

		// The following is called before the first Move(), Rotate() or Scale() call
		CoreExport void TransformStart(TimeValue t);

		// The following is called after the user has completed the Move, Rotate or Scale operation and
		// the undo object has been accepted.
		CoreExport void TransformFinish(TimeValue t);		

		// The following is called when the transform operation is cancelled by a right-click and the undo
		// has been cancelled.
		CoreExport void TransformCancel(TimeValue t);		

		CoreExport ObjectState Eval(TimeValue t);

		BOOL SupportsNamedSubSels() {return TRUE;}
		CoreExport void ActivateSubSelSet(TSTR &setName);
		CoreExport void NewSetFromCurSel(TSTR &setName);
		CoreExport void RemoveSubSelSet(TSTR &setName);
		CoreExport void SetupNamedSelDropDown();
		CoreExport int NumNamedSelSets();
		CoreExport TSTR GetNamedSelSetName(int i);
		CoreExport void SetNamedSelSetName(int i,TSTR &newName);
		CoreExport void NewSetByOperator(TSTR &newName,Tab<int> &sets,int op);
		CoreExport BOOL GetUniqueSetName(TSTR &name);
		CoreExport int SelectNamedSet();
		CoreExport void NSCopy();
		CoreExport void NSPaste();
		CoreExport GenericNamedSelSetList &GetSelSet();

		CoreExport void RefreshSelType();
		CoreExport void SetNumSelLabel();
		CoreExport void SetSelDlgEnables();
		CoreExport void SetOpsDlgEnables();
		CoreExport void SetSurfDlgEnables();
		CoreExport void SetSoftSelDlgEnables( HWND hSoftSel = NULL );

		// from AttachMatDlgUser
		int GetAttachMat() { return attachMat; }
		void SetAttachMat(int value) { attachMat = value; }
		BOOL GetCondenseMat() { return condenseMat; }
		void SetCondenseMat(BOOL sw) { condenseMat = sw; }

		CoreExport void DoAttach(INode *node, PatchMesh *attPatch, bool & canUndo);
		CoreExport void SetTessUI(HWND hDlg, TessApprox *tess);
		// 'interactive' in the following records Undo information, displays prompts, notifies
		// dependents and redraws when TRUE
		CoreExport void DoDeleteSelected(BOOL interactive=TRUE);
		CoreExport void DoBreak(BOOL interactive=TRUE);
		CoreExport void ResolveTopoChanges();
		CoreExport void DeletePatchParts(BitArray &delVerts, BitArray &delPatches);
		// Animated point stuff:
		CoreExport void CreateContArrays();
		CoreExport void SynchContArrays();
		// Materials
		CoreExport int GetSelMatIndex();
		CoreExport void SetSelMatIndex(int index);
		CoreExport void SelectByMat(int index,BOOL clear);
		// Smoothing
		CoreExport DWORD GetSelSmoothBits(DWORD &invalid);
		CoreExport DWORD GetUsedSmoothBits();
		CoreExport void SelectBySmoothGroup(DWORD bits,BOOL clear);
		CoreExport void SetSelSmoothBits(DWORD bits,DWORD which);
		// Subdivision / addition
		CoreExport void SetPropagate(BOOL sw);
		CoreExport BOOL GetPropagate();
		CoreExport void DoPatchAdd(int type);
		CoreExport void DoSubdivide(int level);
		CoreExport void DoPatchDetach(int copy, int reorient);
		// Welding
		CoreExport void DoVertWeld();
		CoreExport void DoVertWeld(int fromVert, int toVert);
		CoreExport void DoEdgeWeld();
		// CAL-06/02/03: copy/paste tangent. (FID #827)
		CoreExport void StartCopyTangentMode();
		CoreExport void StartPasteTangentMode();
		CoreExport void StartPasteTangent();
		CoreExport void EndPasteTangent();

		CoreExport BOOL CopyTangent(int vec);
		CoreExport BOOL PasteTangent(int vec);
//watje
		//hide and unhide stuff
		CoreExport void DoHide(int type) ;
		CoreExport void DoUnHide();
		CoreExport void DoPatchHide();
		CoreExport void DoVertHide();
		CoreExport void DoEdgeHide();
//watje hook stuff
		CoreExport void DoAddHook(int vert1, int seg1) ;
		CoreExport void DoRemoveHook() ;
//watje bevel and extrusion stuff
		CoreExport void DoExtrude(TimeValue t) ;
		CoreExport void BeginExtrude(TimeValue t); 	
		CoreExport void EndExtrude (TimeValue t, BOOL accept=TRUE);		
		CoreExport void Extrude( TimeValue t, float amount, BOOL useLocalNorm );

		CoreExport void DoFlipNormals(int patchIndex = -1);
		CoreExport void DoUnifyNormals();
		
		CoreExport void DoBevel(TimeValue t) ;
		CoreExport void BeginBevel(TimeValue t); 	
		CoreExport void EndBevel (TimeValue t, BOOL accept=TRUE);		
		CoreExport void Bevel( TimeValue t, float amount, BOOL smoothStart, BOOL smoothEnd );

		// CAL-04/23/03: patch smooth (FID #1914)
		CoreExport void DoPatchSmooth(int type);

		// methods for creating new vertices and patches
		CoreExport void CreateVertex(Point3 pt, int& newIndex);
		CoreExport void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3);
		CoreExport void CreatePatch(int vertIndx1,int vertIndx2,int vertIndx3,int vertIndx4);

		// patch select and operations interfaces, JBW 2/2/99
		CoreExport void StartCommandMode(patchCommandMode mode);
		CoreExport void ButtonOp(patchButtonOp opcode);

// LAM: added 9/3/00
	// UI controls access
		CoreExport void GetUIParam (patchUIParam uiCode, int & ret);
		CoreExport void SetUIParam (patchUIParam uiCode, int val);
		CoreExport void GetUIParam (patchUIParam uiCode, float & ret);
		CoreExport void SetUIParam (patchUIParam uiCode, float val);
		bool Editing () { return (ip && (editObj==this)) ? TRUE : FALSE; }

		CoreExport DWORD GetSelLevel();
		CoreExport void SetSelLevel(DWORD level);
		CoreExport void LocalDataChanged();

		CoreExport BitArray GetVecSel();
		CoreExport BitArray GetVertSel();
		CoreExport BitArray GetEdgeSel();
		CoreExport BitArray GetPatchSel();
		
		CoreExport void SetVecSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetVertSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetEdgeSel(BitArray &set, IPatchSelect *imod, TimeValue t);
		CoreExport void SetPatchSel(BitArray &set, IPatchSelect *imod, TimeValue t);

		CoreExport GenericNamedSelSetList& GetNamedVecSelList();
		CoreExport GenericNamedSelSetList& GetNamedVertSelList();
		CoreExport GenericNamedSelSetList& GetNamedEdgeSelList();
		CoreExport GenericNamedSelSetList& GetNamedPatchSelList();

		// Vertex Color Support...
		CoreExport Color GetVertColor(int mp=0, bool *differs=NULL);
		CoreExport void SetVertColor(Color clr, int mp=0);
		CoreExport void SelectVertByColor(VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp=0);
		// and patch-level vertex color support
		CoreExport Color GetPatchColor(int mp=0, bool *differs=NULL);
		CoreExport void SetPatchColor(Color clr, int mp=0);

		// ISubMtlAPI methods:
		CoreExport MtlID GetNextAvailMtlID(ModContext* mc);
		CoreExport BOOL HasFaceSelection(ModContext* mc);
		CoreExport void SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE);
		CoreExport int GetSelFaceUniqueMtlID(ModContext* mc);
		CoreExport int GetSelFaceAnyMtlID(ModContext* mc);
		CoreExport int GetMaxMtlID(ModContext* mc);
//watje new patch mapping
		CoreExport void ChangeMappingTypeLinear(BOOL linear);

		// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1914)
		CoreExport void ShrinkSelection(int type);
		CoreExport void GrowSelection(int type);
		CoreExport void SelectEdgeRing();
		CoreExport void SelectEdgeLoop();

		CoreExport void SelectOpenEdges();
		CoreExport void DoCreateShape();

// soft selection support
//
		CoreExport int  UseEdgeDists();
		CoreExport void SetUseEdgeDists( int useSoftSelections );
		CoreExport int  UseSoftSelections();
		CoreExport void SetUseSoftSelections( int useSoftSelections );
		CoreExport void InvalidateVertexWeights();
		// CAL-05/06/03: toggle shaded faces display for soft selection. (FID #1914)
		CoreExport void ToggleShadedFaces();

		CoreExport void UpdateVertexDists();
		CoreExport void UpdateEdgeDists( );
		CoreExport void UpdateVertexWeights();

		CoreExport Point3 VertexNormal( int vIndex ); 

};

CoreExport ClassDesc* GetPatchObjDescriptor();

// Helper classes used internally for undo operations

class POModRecord {
	public:
		virtual ~POModRecord(){}
		virtual BOOL Restore(PatchObject *po, BOOL isUndo)=0;
		virtual BOOL Redo(PatchObject *po)=0;
		virtual DWORD Parts()=0;				// Flags of parts modified by this
	};

class PatchObjectRestore : public RestoreObj {
	public:
		POModRecord *rec;		// Modification record
		PatchObject	 *po;

		PatchObjectRestore(PatchObject *po, POModRecord *rec);
		virtual ~PatchObjectRestore();

		void Restore(int isUndo);
		void Redo();
		int Size() { return 1; }
		void EndHold() {po->ClearAFlag(A_HELD);}
		TSTR Description() { return TSTR(_T("PatchObject restore")); }
	};

#define  SRMPN_CLASS_ID 0x74c4969
class SingleRefMakerPatchNode : public SingleRefMaker{
	public:
		HWND hwnd;
		PatchObject *po;
		SingleRefMakerPatchNode() {hwnd = NULL; po = NULL;}
		~SingleRefMakerPatchNode() { 
			theHold.Suspend();         
			DeleteAllRefsFromMe(); 
			theHold.Resume();
		}
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
		SClass_ID  SuperClassID() { return SRMPN_CLASS_ID; }
	};

#define  SRMPM_CLASS_ID 0x64391d45
class SingleRefMakerPatchMtl : public SingleRefMaker{
	public:	
		HWND hwnd;
		PatchObject *po;
		SingleRefMakerPatchMtl() {hwnd = NULL; po = NULL;}
		~SingleRefMakerPatchMtl() { 
			theHold.Suspend();         
			DeleteAllRefsFromMe(); 
			theHold.Resume();
		}
		RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message );
		SClass_ID  SuperClassID() { return SRMPM_CLASS_ID; }
};

// Command ID for the dynamic spline quad menu entry
#define ID_PATCH_MENU 1333

const DWORD kPatchActions = 0x34fe4c77;
const DWORD kPatchActionsContext = 0x34fe4c77;

#endif // __PATCHOBJ__
