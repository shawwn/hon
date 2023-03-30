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

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] , public ResourceMakerCallback {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		IObjParam *ip;
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		void SetInitialName();
		TCHAR *GetName();
		void SetName(TCHAR *name);
		BOOL IsEnabled();
		BOOL IsVolumeBased();
		BOOL IsInVolume(Point3 p, Matrix3 tm);

		BOOL IsEditing();
		void Enable(BOOL enable);
		void EndEditing();
		void EnableEditing(BOOL enable);

		IGizmoBuffer *CopyToBuffer(); 
		void PasteFromBuffer(IGizmoBuffer *buffer); 
		
		BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable); 
		void PostDeformSetup(TimeValue t);
		void PreDeformSetup(TimeValue t);
		HWND hGizmoParams;
		Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm);

		// From Base Object
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               
		int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm );
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm);
		void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
		void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE );
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm);
		void ClearSelection(int selLevel);
		void SelectAll(int selLevel);
		void InvertSelection(int selLevel);

		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return REF_TARGET_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		void DeleteThis() { delete this; }		
		
		//Constructor/Destructor
		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};


[!output TEMPLATESTRING_CLASSDESC]


[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]

[!output CLASS_NAME]::[!output CLASS_NAME]()
{
[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

void [!output CLASS_NAME]::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);
}

void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	[!output CLASS_NAME]Desc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;

}

/*******************************************************************************************
*
	This is called by the Skin Modifier to receive the name at creation of the gizmo
	It will appear in Skin's Gizmo list box
*
********************************************************************************************/

void [!output CLASS_NAME]::SetInitialName()
{
	//TODO: Set the initial name to appear in the gizmo list
	pblock_gizmo_data->SetValue(pb_gizmoparam_name,0,_T("Wizard Gizmo"));

}

/********************************************************************************************
*
	The following functions give access to the name of the Gizmo by accessing the
	Parameter block storing the name
*
*********************************************************************************************/

TCHAR * [!output CLASS_NAME]::GetName()
{
	TCHAR *name;
	pblock_gizmo_data->GetValue(pb_gizmoparam_name,0,name,FOREVER);
	return name;
}

void [!output CLASS_NAME]::SetName(TCHAR *name)
{
	pblock_gizmo_data->SetValue(pb_gizmoparam_name,0,name);
	
}


BOOL [!output CLASS_NAME]::IsEnabled()
{
	//TODO:  Tell the system whether the gizmo is Enabled or not
	return TRUE;
}
		
BOOL [!output CLASS_NAME]::IsVolumeBased()
{
	//TODO: Tell the system whether the gizmo works on a volume basis
	return FALSE;
}

BOOL [!output CLASS_NAME]::IsInVolume(Point3 p, Matrix3 tm)
{
	//TODO:  Check whether the past in point lies in the volume
	return TRUE;
}

BOOL [!output CLASS_NAME]::IsEditing()
{
	return TRUE;
}
		
void [!output CLASS_NAME]::Enable(BOOL enable)
{
	//TODO:  Enable or disable the Gizmo
}

void [!output CLASS_NAME]::EndEditing()
{

}

void [!output CLASS_NAME]::EnableEditing(BOOL enable)
{

}

		
IGizmoBuffer * [!output CLASS_NAME]::CopyToBuffer() 
{
	return 0;
}
		
void [!output CLASS_NAME]::PasteFromBuffer(IGizmoBuffer *buffer)
{

}

/************************************************************************************************
*
	This is called when the gizmo is initially created it is passed to the current selected 
	verts in the world space.

	count is the number of vertice in *p

	*p is the list of point being affected in world space

	numberOfInstances is the number of times this modifier has been instanced

	mapTable is an index list into the original vertex table for *p
*
**************************************************************************************************/


BOOL [!output CLASS_NAME]::InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable)
{

	return TRUE;
}

/*************************************************************************************************
*
	This is called before the deformation on a frame to allow the gizmo to do some
	initial setup
*
**************************************************************************************************/

void [!output CLASS_NAME]::PreDeformSetup(TimeValue t)
{
	//TODO:Perform any pre deform initialization

}

void [!output CLASS_NAME]::PostDeformSetup(TimeValue t)
{
	//TODO: Perform any clean up

}	

/*************************************************************************************************
*
	This is what deforms the point
	It is passed in from the Map call from the Skin modifier
*
**************************************************************************************************/	

Point3 [!output CLASS_NAME]::DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)
{
	//TODO: Defrom the Point
	return Point3(0,0,0);
}

/**************************************************************************************************
*
  This retrieves the boudng box of the gizmo in world space
*
**************************************************************************************************/


void [!output CLASS_NAME]::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{
	//TODO: Calculate the bounding box of the Gizmo
}

/*************************************************************************************************
*
	This is called in the skin modifier's display code to show the actual gizmo
*
***************************************************************************************************/


int [!output CLASS_NAME]::Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm )
{
	//TODO: Draw the physical representation of the Gizmo
	return 1;
}


/*************************************************************************************************
*
	The following functions are standard subobject selection and manipulation routines
*
**************************************************************************************************/


int [!output CLASS_NAME]::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm)
{
	//TODO:  Perform the gizmo's hit testing and return the result
	return 0;
}

void [!output CLASS_NAME]::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
{
	//TODO: Perform the actual selection based on the HitRecord
	
}

void [!output CLASS_NAME]::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin ) 
{
	//TODO: Perform the transformation of the selected object
}

void [!output CLASS_NAME]::GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm)
{
	//TODO: Update the centres of the selected Objected
}

void [!output CLASS_NAME]::GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) 
{
	//TODO: Update the Tranformation matrices of the selected objects
}

void [!output CLASS_NAME]::ClearSelection(int selLevel)
{
	//TODO: Clear the current selection 
}

void [!output CLASS_NAME]::SelectAll(int selLevel)
{
	//TODO: Select all the subobject components at the currect sub object level
}

void [!output CLASS_NAME]::InvertSelection(int selLevel)
{
	//TODO: Invert the current selection at the currect sub object level
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	//TODO: Standard loading of data
	return IO_OK;
}


IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	//TODO: standard saving of data
	return IO_OK;
}

RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}


