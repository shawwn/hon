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

#define WARPOBJ_CLASS_ID	Class_ID([!output WARPID1], [!output WARPID2])


#define PBLOCK_REF SIMPWSMMOD_PBLOCKREF


//Definition of the Object Representation of the Deformer
class [!output CLASS_NAME]Object : public SimpleWSMObject2
{	
	public:		
		IObjParam *ip;
		HWND hSot;
					
		[!output CLASS_NAME]Object();		

		void DeleteThis() {delete this;}		
		Class_ID ClassID() {return WARPOBJ_CLASS_ID;}		
		TCHAR *GetObjectName() {return GetString(IDS_CLASS_NAME);}		

		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		
		void InvalidateUI();

		int DoOwnSelectHilite() {return TRUE;}
		CreateMouseCallBack* GetCreateMouseCallBack();

		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);

		Modifier *CreateWSMMod(INode *node);
		void BuildMesh(TimeValue t);
		
		// Direct paramblock access - Use the Paramblock maintained by SimpleWSMObject2

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock2; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
};

class [!output CLASS_NAME]ObjCreateCallBack : public CreateMouseCallBack 
{
	[!output CLASS_NAME]Object *swo;
	IPoint2 sp0;
	Point3 p0;

	public:
	
	int proc( ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj([!output CLASS_NAME]Object *obj) {swo = obj;}
};

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		IObjParam *ip;

		[!output CLASS_NAME](INode *node,[!output CLASS_NAME]Object *obj);		

		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		

		Interval GetValidity(TimeValue t);
		void InvalidateUI();

		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		
		Animatable* SubAnim(int i);

		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return WSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );

		int NumSubs() { return 1; }

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


/*===========================================================================*\
 |	[!output CLASS_NAME]Deformer class defn
 |  This is the system that gets applied to each deformable point
 |
\*===========================================================================*/

class [!output CLASS_NAME]Deformer: public Deformer 
{
	public:
		
		[!output CLASS_NAME]Deformer() {}
		Point3 Map(int i, Point3 p); 
};

/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

[!output CLASS_NAME]Object::[!output CLASS_NAME]Object()
{
	[!output CLASS_NAME]ObjDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
}



/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void [!output CLASS_NAME]Object::InvalidateUI()
{
	[!output CLASS_NAME_LOWER]obj_param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}




/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void [!output CLASS_NAME]Object::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	SimpleWSMObject::BeginEditParams(ip,flags,prev);
	[!output CLASS_NAME]ObjDesc.BeginEditParams(ip, this, flags, prev);


}
		
void [!output CLASS_NAME]Object::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleWSMObject::EndEditParams(ip,flags,next);
	[!output CLASS_NAME]ObjDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}




/*===========================================================================*\
 |	Standard Load and clone
\*===========================================================================*/

IOResult [!output CLASS_NAME]Object::Load(ILoad *iload)
{
	SimpleWSMObject::Load(iload);
	return IO_OK;
}

RefTargetHandle [!output CLASS_NAME]Object::Clone(RemapDir& remap) 
{	
	[!output CLASS_NAME]Object* newob = new [!output CLASS_NAME]Object();	
	newob->ReplaceReference(0,remap.CloneRef(pblock2));
	BaseClone(this, newob, remap);
	return(newob);
}



/*===========================================================================*\
 |	Create the WSM modifier
\*===========================================================================*/

Modifier *[!output CLASS_NAME]Object::CreateWSMMod(INode *node)
{
	return new [!output CLASS_NAME](node,this);
}

// Default constructor/Destructor

[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
	assert(pblock);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}
/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

[!output CLASS_NAME]::[!output CLASS_NAME](INode *node,[!output CLASS_NAME]Object *obj)
{	
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
	assert(pblock);

	MakeRefByID(FOREVER,SIMPWSMMOD_NODEREF,node);
	
	obRef = NULL;
}


/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void [!output CLASS_NAME]::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;

	SimpleWSMMod::BeginEditParams(ip,flags,prev);
	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);

}
		
void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleWSMMod::EndEditParams(ip,flags,next);
	[!output CLASS_NAME]Desc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;	
}



/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void [!output CLASS_NAME]::InvalidateUI()
{
	[!output CLASS_NAME_LOWER]_param_blk.InvalidateUI(pblock->LastNotifyParamID());
}



/*===========================================================================*\
 |	Standard Load Save and clone
\*===========================================================================*/

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	Modifier::Load(iload);
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	Modifier::Save(isave);
	return IO_OK;
}

RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir& remap) 
{
	[!output CLASS_NAME] *newob = new [!output CLASS_NAME](nodeRef,([!output CLASS_NAME]Object*)obRef);
	newob->ReplaceReference(SIMPWSMMOD_PBLOCKREF,remap.CloneRef(pblock));
	newob->ReplaceReference(SIMPWSMMOD_OBREF,this->obRef);
	newob->ReplaceReference(SIMPWSMMOD_NODEREF,this->nodeRef);
	BaseClone(this, newob, remap);
	return newob;
}



/*===========================================================================*\
 |	The validity of our parameters
 |	Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/

Interval [!output CLASS_NAME]::GetValidity(TimeValue t) 
{
	if (nodeRef) {
		Interval valid = FOREVER;
		Matrix3 tm;
		float f;

		[!output CLASS_NAME]Object *obj = ([!output CLASS_NAME]Object*)GetWSMObject(t);
		pblock->GetValue(pb_spin, t, f, valid);
		obj->pblock2->GetValue(pb_spin_obj, t, f, valid);

		tm = nodeRef->GetObjectTM(t,&valid);
		return valid;
	}
	else 
	{
		return FOREVER;
	}
}




/*===========================================================================*\
 |	Get the actual deformer to modify the object
\*===========================================================================*/

Deformer& [!output CLASS_NAME]::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	Interval valid;
	static [!output CLASS_NAME]Deformer wsmd;

//TODO: Define the Deformer
	return wsmd;
}

/*===========================================================================*\
 |	Deform the actual point
\*===========================================================================*/


Point3 [!output CLASS_NAME]Deformer::Map(int i, Point3 p)
{
	//TODO: Deform the actual Point
	return p;
}

int [!output CLASS_NAME]ObjCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1, center;

	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
	}


	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {

		case 0:
			sp0 = m;
			p0 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			break;


		case 1:
			mat.IdentityMatrix();
			
			p1 = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
			r = Length(p1-p0);
			mat.SetTrans(p0);

			if (msg==MOUSE_POINT) {
				return (Length(m-sp0)<3)?CREATE_ABORT:CREATE_STOP;
			}
			break;					   
		}
	} else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}

	return TRUE;
}

static [!output CLASS_NAME]ObjCreateCallBack WSMCreateCB;

CreateMouseCallBack* [!output CLASS_NAME]Object::GetCreateMouseCallBack() 
{
	WSMCreateCB.SetObj(this);
	return(&WSMCreateCB);
}




/*===========================================================================*\
 |	Mesh creation - create the actual object
\*===========================================================================*/

void [!output CLASS_NAME]Object::BuildMesh(TimeValue t)
{
	//TODO: Build the Space Warp Gizmo
}


/*===========================================================================*\
 |	SimpleWSMMod2 overloads
\*===========================================================================*/

RefTargetHandle [!output CLASS_NAME]::GetReference(int i) 
{ 
	switch (i) {
		case SIMPWSMMOD_OBREF: return obRef;
		case SIMPWSMMOD_NODEREF: return nodeRef;
		case SIMPWSMMOD_PBLOCKREF: return pblock;
		default: return NULL;
		}
}

void [!output CLASS_NAME]::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch (i) {
		case SIMPWSMMOD_OBREF: obRef = (WSMObject*)rtarg; break;
		case SIMPWSMMOD_NODEREF: nodeRef = (INode*)rtarg; break;
		case SIMPWSMMOD_PBLOCKREF: pblock = (IParamBlock2*)rtarg; SimpleWSMMod::SetReference(i, rtarg);break;
		}
}

Animatable* [!output CLASS_NAME]::SubAnim(int i) 
{ 
	switch (i) {
		case 0: return pblock;		
		default: return NULL;
		}
}


