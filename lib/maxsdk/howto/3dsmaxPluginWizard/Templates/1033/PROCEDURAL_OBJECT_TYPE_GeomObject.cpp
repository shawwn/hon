/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])


#define PBLOCK_REF	0


class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		//Class vars
		static IObjParam *ip;			//Access to the interface
		BOOL suspendSnap;				//A flag for setting snapping on/off
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		//TODO: Return the name that will appear in the history browser (modifier stack)
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }
		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );

		void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		//TODO: Return the default name of the node when it is created.
		void InitNodeName(TSTR& s) { s = GetString(IDS_CLASS_NAME); }
		
		// From Object
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		//TODO: Evaluate the object and return the ObjectState
		ObjectState Eval(TimeValue t) { return ObjectState(this); };		
		//TODO: Return the validity interval of the object as a whole
		Interval ObjectValidity(TimeValue t) { return FOREVER; }

		// From Animatable
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From GeomObject
		Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};


[!output TEMPLATESTRING_CLASSDESC]


[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


IObjParam *[!output CLASS_NAME]::ip			= NULL;

//--- [!output CLASS_NAME] -------------------------------------------------------

[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

void [!output CLASS_NAME]::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	this->ip = ip;

	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);
}

void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	//TODO: Save plugin parameter values into class variables, if they are not hosted in ParamBlocks. 
	

	[!output CLASS_NAME]Desc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}

//From Object
BOOL [!output CLASS_NAME]::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void [!output CLASS_NAME]::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin's internal value to sw				
}

//Class for interactive creation of the object using the mouse
class [!output CLASS_NAME]CreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;		//First point in screen coordinates
	[!output CLASS_NAME] *ob;		//Pointer to the object 
	Point3 p0;			//First point in world coordinates
public:	
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj([!output CLASS_NAME] *obj) {ob = obj;}
};

int [!output CLASS_NAME]CreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	//TODO: Implement the mouse creation code here
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0: // only happens with MOUSE_POINT msg
			ob->suspendSnap = TRUE;
			sp0 = m;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
			break;
		//TODO: Add for the rest of points
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static [!output CLASS_NAME]CreateCallBack [!output CLASS_NAME]CreateCB;

//From BaseObject
CreateMouseCallBack* [!output CLASS_NAME]::GetCreateMouseCallBack() 
{
	[!output CLASS_NAME]CreateCB.SetObj(this);
	return(&[!output CLASS_NAME]CreateCB);
}

int [!output CLASS_NAME]::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	//TODO: Implement the displaying of the object here
	return 0;
}

int [!output CLASS_NAME]::HitTest(TimeValue t, INode* inode, int type, int crossing, 
	int flags, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Implement the hit testing here
	return 0;
}

void [!output CLASS_NAME]::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Check the point passed for a snap and update the SnapInfo structure
}

void [!output CLASS_NAME]::GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the world space bounding box of the object
}

void [!output CLASS_NAME]::GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the local space bounding box of the object
}

void [!output CLASS_NAME]::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	//TODO: Compute the bounding box in the objects local coordinates 
	//		or the optional space defined by tm.
}

//From ReferenceMaker
RefResult [!output CLASS_NAME]::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID, RefMessage message )
{
	//TODO: Implement, if the object makes references to other things
	return(REF_SUCCEED);
}

Mesh* [!output CLASS_NAME]::GetRenderMesh(TimeValue t, 
						INode *inode, View& view, BOOL& needDelete)
{
	//TODO: Return the mesh representation of the object used by the renderer
	return NULL;
}


Object* [!output CLASS_NAME]::ConvertToType(TimeValue t, Class_ID obtype)
{
	//TODO: If the plugin can convert to a nurbs surface then check to see 
	//		whether obtype == EDITABLE_SURF_CLASS_ID and convert the object
	//		to nurbs surface and return the object
	
	return NULL;
}

int [!output CLASS_NAME]::CanConvertToType(Class_ID obtype)
{
	//TODO: Check for any additional types the plugin supports
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID) {
		return 1;
	} else {		
	return 0;
		}
}

// From Object
int [!output CLASS_NAME]::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
{
	//TODO: Return TRUE after you implement this method
	return FALSE;
}

void [!output CLASS_NAME]::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
	//TODO: Append any any other collapse type the plugin supports
}

// From ReferenceTarget
RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir& remap) 
{
	[!output CLASS_NAME]* newob = new [!output CLASS_NAME]();	
	//TODO: Make a copy all the data and also clone all the references
	newob->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newob, remap);
	return(newob);
}

