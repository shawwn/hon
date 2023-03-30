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

		// From Atmospheric
		TSTR GetName() {return GetString(IDS_CLASS_NAME);}
		AtmosParamDlg *CreateParamDialog(IRendParams *ip);
		int RenderBegin(TimeValue t, ULONG flags);
		int RenderEnd(TimeValue t);
		void Update(TimeValue t, Interval& valid);
		void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG);
		
		// Support for gizmos in our atmospheric
		int NumGizmos() ;
		INode *GetGizmo(int i);
		void DeleteGizmo(int i);
		void InsertGizmo(int i, INode *node);
		void AppendGizmo(INode *node);
		BOOL OKGizmo(INode *node); 
 		void EditGizmo(INode *node);
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return ATMOSPHERIC_CLASS_ID; }
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

class [!output CLASS_NAME]ParamDlg : public AtmosParamDlg {
	public:
		[!output CLASS_NAME] *atmos;
		IRendParams *ip;
		IParamMap2 *pmap;

		[!output CLASS_NAME]ParamDlg([!output CLASS_NAME] *a,IRendParams *i);
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}
		ReferenceTarget* GetThing() {return atmos;}
		void SetThing(ReferenceTarget *m);		
		void DeleteThis();
	};

[!output TEMPLATESTRING_CLASSDESC]


[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


//--- [!output CLASS_NAME]ParamDlg -------------------------------------------------------

[!output CLASS_NAME]ParamDlg::[!output CLASS_NAME]ParamDlg([!output CLASS_NAME] *a,IRendParams *i) 
{
	//TODO: Add the dialog and parameter map creation code here 
	atmos = a;
	ip    = i;
	pmap = CreateRParamMap2(
		atmos->pblock,
		i,
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		GetString(IDS_PARAMS),
		0);
}

void [!output CLASS_NAME]ParamDlg::SetThing(ReferenceTarget *m)
{
	
}

void [!output CLASS_NAME]ParamDlg::DeleteThis()
{
	//TODO: Add the dialog and parameter map destruction code here 
	delete this;
}



[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	Atmospheric::Load(iload);

	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	Atmospheric::Save(isave);

	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}


RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
	{
	switch (message) {
		case REFMSG_TARGET_DELETED:
			{
				for(int i=0;i<NumGizmos();i++)
				{
					if((INode*)hTarget==GetGizmo(i)) DeleteGizmo(i);
				}
			}
			break;

		case REFMSG_CHANGE:
			if (hTarget == pblock && pblock->GetMap() != NULL)
				pblock->GetMap()->Invalidate();
			break;
		}
	return REF_SUCCEED;
	}


RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* newObj = new [!output CLASS_NAME]();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}

AtmosParamDlg *[!output CLASS_NAME]::CreateParamDialog(IRendParams *ip)
{
	return new [!output CLASS_NAME]ParamDlg(this,ip);
}

int [!output CLASS_NAME]::RenderBegin(TimeValue t,ULONG flags)
{		
  	//TODO: Add intiliazing stuff required by plugin prior to rendering
	return 0;
}

int [!output CLASS_NAME]::RenderEnd(TimeValue t)
{	
	//TODO: Add stuff required after each render
	return 0;
}

void [!output CLASS_NAME]::Update(TimeValue t, Interval& valid)
{
	
}

void [!output CLASS_NAME]::Shade(
		ShadeContext& sc,const Point3& p0,const Point3& p1,
		Color& color, Color& trans, BOOL isBG)
{
	//TODO: Add the code that is called to apply the atmospheric effect
}

/*===========================================================================*\
 |	Support for getting/setting gizmos  
\*===========================================================================*/

int [!output CLASS_NAME]::NumGizmos() 
{
	return pblock->Count(pb_gizmos);
}

INode *[!output CLASS_NAME]::GetGizmo(int i) 
{
	INode *node = NULL;
	Interval iv;
	pblock->GetValue(pb_gizmos,0,node,iv, i);
	return node;
}

void [!output CLASS_NAME]::DeleteGizmo(int i)
{
	pblock->Delete(pb_gizmos, i,1); 
	[!output CLASS_NAME_LOWER]_param_blk.InvalidateUI();

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void [!output CLASS_NAME]::InsertGizmo(int i, INode *node)
{
	pblock->SetValue(pb_gizmos, 0, node, i);
}

void [!output CLASS_NAME]::AppendGizmo(INode *node)
{
	pblock->Append(pb_gizmos, 1, &node);
}

BOOL [!output CLASS_NAME]::OKGizmo(INode *node)
{
	// check for duplicates in the gizmo list
	for(int i=0;i<NumGizmos();i++) { if(node==GetGizmo(i)) return FALSE; }

	ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
	if (os.obj->ClassID()==SPHEREGIZMO_CLASSID) return TRUE;
	if (os.obj->ClassID()==CYLGIZMO_CLASSID) return TRUE;
	if (os.obj->ClassID()==BOXGIZMO_CLASSID) return TRUE;
	return FALSE;
}

void [!output CLASS_NAME]::EditGizmo(INode *node)
{
}

