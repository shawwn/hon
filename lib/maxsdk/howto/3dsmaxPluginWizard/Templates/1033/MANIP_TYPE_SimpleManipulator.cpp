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
		// Parameter block handled by parent

		void OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
		void OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData);
		void OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData); 
		//from BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack(void);

		void UpdateShapes(TimeValue t, TSTR& toolTip);
		void GenerateShapes(TimeValue t);

		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return HELPER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return mpPblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (mpPblock->ID() == id) ? mpPblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		[!output CLASS_NAME]();
		[!output CLASS_NAME](INode *pnode);
		~[!output CLASS_NAME]();

};


[!output TEMPLATESTRING_CLASSDESC]

/*********************************************************************************************************
*
	If a manipulator applies to a node, this call will create an instance of it and add it to the 
	attached objects of the node.  It will then initialize the manipualtor
*
\*****************************************************************************************************/
Manipulator * [!output CLASS_NAME]ClassDesc::CreateManipulator(RefTargetHandle hTarget, INode* pNode)
{
    [!output CLASS_NAME]* pManip = new [!output CLASS_NAME]();
    return pManip;
}


[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


/**************************************************************************************************
*
	Define a Mouse Callback to implement the creation of the gizmo.  If the Manipulator does not
	have any user interface or creation method, then this is not need and ClassDesc2::Ispublic() 
	can return FALSE.
*
\**************************************************************************************************/

class [!output CLASS_NAME]CreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;			//First point in screen coordinates
	[!output CLASS_NAME] *ob;		//Pointer to the object 
	Point3 p0;				//First point in world coordinates
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




[!output CLASS_NAME]::[!output CLASS_NAME]():SimpleManipulator(NULL)
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}

[!output CLASS_NAME]::[!output CLASS_NAME](INode *pnode):SimpleManipulator(pnode)
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}


[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

void [!output CLASS_NAME]::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
{

	SimpleManipulator::BeginEditParams(ip,flags,prev);
	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);


}
		
void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	SimpleManipulator::EndEditParams(ip,flags,next);
	[!output CLASS_NAME]Desc.EndEditParams(ip, this, flags, next);

}


void [!output CLASS_NAME]::UpdateShapes(TimeValue t, TSTR& toolTip)
{
	GenerateShapes(t);
}

void [!output CLASS_NAME]::GenerateShapes(TimeValue t)
{

}

void [!output CLASS_NAME]::OnMouseMove(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
	SimpleManipulator::OnMouseMove(t, pVpt, m, flags, pHitData);
}

void [!output CLASS_NAME]::OnButtonDown(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
	SimpleManipulator::OnButtonDown(t, pVpt, m, flags, pHitData);

}
	
void [!output CLASS_NAME]::OnButtonUp(TimeValue t, ViewExp* pVpt, IPoint2& m, DWORD flags, ManipHitData* pHitData)
{
	SimpleManipulator::OnButtonUp(t, pVpt, m, flags, pHitData);

}


CreateMouseCallBack* [!output CLASS_NAME]::GetCreateMouseCallBack(void) 
{ 
	[!output CLASS_NAME]CreateCB.SetObj(this);
	return(&[!output CLASS_NAME]CreateCB);
}




RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir& remap) 
{
	[!output CLASS_NAME]* newob = new [!output CLASS_NAME]();	
	//TODO: Make a copy all the data and also clone all the references
	newob->ReplaceReference(0,remap.CloneRef(mpPblock));

	newob->mValid.SetEmpty();
	BaseClone(this, newob, remap);
	return(newob);
}