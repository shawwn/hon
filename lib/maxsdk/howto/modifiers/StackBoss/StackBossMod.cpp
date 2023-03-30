/**********************************************************************
 *<
	FILE: StackBossMod.cpp

	DESCRIPTION:	The actual StackBoss Modifier.  It is used to maintain the
					the Custom Attributes of an object.

	CREATED BY:		Neil Hazzard		

	HISTORY:		October 2003

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "StackBossMod.h"
#include "modstack.h"




class StackBossModClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new StackBossMod(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return STACKBOSSMOD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("StackBossMod"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static StackBossModClassDesc StackBossModDesc;
ClassDesc2* GetStackBossModDesc() { return &StackBossModDesc; }


enum { stackbossmod_params };


//TODO: Add enums for various parameters


static ParamBlockDesc2 stackbossmod_param_blk ( stackbossmod_params, _T("params"),  0, &StackBossModDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,
	// params
	StackBossMod::pb_delete, 			_T("allowDelete"), TYPE_BOOL, 	0, 	IDS_DELETE_MOD, 
		p_default, 		FALSE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DELETE_MOD,
		end,
	StackBossMod::pb_showMedit, 		_T("showMeditCA"), TYPE_BOOL, 	0, 	IDS_SHOW_MEDIT, 
		p_default, 		TRUE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_SHOW_MEDIT,
		end,
	StackBossMod::pb_showMod, 			_T("showModCA"), TYPE_BOOL, 	0, 	IDS_SHOW_MOD, 
		p_default, 		TRUE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_SHOW_MOD,
		end,
	StackBossMod::pb_showBaseObject, 	_T("showBaseObjectCA"), TYPE_BOOL, 	0, 	IDS_SHOW_BASEOBJ, 
		p_default, 		TRUE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_SHOW_BASEOBJ,
		end,
//No longer used
	StackBossMod::pb_copyCA, 	_T("copyCA"), TYPE_BOOL, 	0, 	IDS_COPY_CA,
		p_default, 		TRUE, 
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_COPY_CA,
		end,
//No longer used
	StackBossMod::pb_copyNormals, 	_T("copyNormals"), TYPE_BOOL, 	0, 	IDS_COPY_NORMALS, 
		p_default, 		TRUE, 
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_COPY_NORMALS,
		end,
	StackBossMod::pb_globalHighlight, 	_T("globalHighlight"), TYPE_BOOL, 	0, 	IDS_GLOBAL_HIGHLIGHT, 
		p_default, 		TRUE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_GLOBAL_HIGHLIGHT,
		end,
	end
);


BOOL SBMDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);

	switch (msg) 
	{
		case WM_PAINT:
			PAINTSTRUCT ps;
			BeginPaint(hWnd,&ps);

			int q;
			for( q=0;q<4;q++ )
			{
				// Get next CI
				HWND item = GetDlgItem(hWnd,(IDC_IC1+q));
				Rect rect;
				GetClientRect(item,&rect);
				HDC hdc = GetDC(item);

				// choose the indicator colour
				COLORREF FilCol;
				FilCol = GetCustSysColor(COLOR_3DFACE);
				if(q==0) FilCol = RGB(0,0,255);
				if(q==1) FilCol = RGB(255,0,0);
				if(q==2) FilCol = RGB(0,255,0);
				if(q==3) FilCol = RGB(255,255,0);
				
				// Draw the button-y outline around the colour indicator
				Rect tR = rect;
				tR.right--;
				tR.bottom--;
				Rect3D(hdc,tR,FALSE);

				HPEN Pen = CreatePen( PS_SOLID , 1 , FilCol );
				SelectObject(hdc,Pen);
				LOGBRUSH BR;
				BR.lbStyle = BS_SOLID;
				BR.lbColor = FilCol;

				HBRUSH Brush = CreateBrushIndirect(&BR);
				SelectObject(hdc,Brush);


				Rectangle( hdc, 1,1,rect.w()-2,rect.h()-2);

				DeleteObject(Pen);
				DeleteObject(Brush);

				ReleaseDC(item,hdc);
			}
			EndPaint(hWnd,&ps);
			return TRUE;

		case WM_INITDIALOG:
		{

			for(int i=0;i<4;i++)
			{
				LONG style = GetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE);
				style &= ~(SS_BLACKFRAME);
				style |= SS_OWNERDRAW;
				SetWindowLongPtr(GetDlgItem(hWnd,IDC_IC1+i),GWL_STYLE,style);
			}
			return TRUE;
		}
	}
	return FALSE;	
}


XTCStackBossMod::XTCStackBossMod()
{
	drawBase = true;
	drawMedit = true;
	drawMod = true;
	hasBaseCA = false;
	hasModCA = false;
}

XTCStackBossMod::XTCStackBossMod(bool medit, bool mod, bool base, bool global)
{
	drawBase = base;
	drawMedit = medit;
	drawMod = mod;
	drawGlobal = global;
	hasBaseCA = false;
	hasModCA = false;

}

XTCStackBossMod::XTCStackBossMod(XTCStackBossMod * mFrom)
{
	drawBase = mFrom->drawBase;
	drawMedit = mFrom->drawMedit;
	drawMod = mFrom->drawMod;
	drawGlobal = mFrom->drawGlobal;
	hasBaseCA = mFrom->hasBaseCA;
	hasModCA = mFrom->hasModCA;
}

XTCStackBossMod::~XTCStackBossMod()
{

}

#define STRFACT ((float)1.02)

static void DrLine(GraphicsWindow *gw, const Point3& p, const Point3 &q) {
	Point3 v[3];
	v[0] = p;
	v[1] = p+(q-p)/(float)4.0;
	gw->polyline( 2, v, NULL, NULL, FALSE, NULL );	
	v[0] = q;
	v[1] = q+(p-q)/(float)4.0;
	gw->polyline( 2, v, NULL, NULL, FALSE, NULL );	
}

static void DisplayBox(GraphicsWindow *gw, Box3 bs)
{
	DrLine(gw, bs[0],bs[1]);		
	DrLine(gw, bs[1],bs[3]);		
	DrLine(gw, bs[3],bs[2]);		
	DrLine(gw, bs[2],bs[0]);		
	DrLine(gw, bs[4],bs[5]);		
	DrLine(gw, bs[5],bs[7]);		
	DrLine(gw, bs[7],bs[6]);		
	DrLine(gw, bs[6],bs[4]);		
	DrLine(gw, bs[0],bs[4]);		
	DrLine(gw, bs[1],bs[5]);		
	DrLine(gw, bs[2],bs[6]);		
	DrLine(gw, bs[3],bs[7]);	

}

int XTCStackBossMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj)
{
	//TODO: Add Extension Objects drawing routine here
	Object * obj = inode->EvalWorldState(t).obj;
	Box3 bs, actualBS;
	bs.Init();

	obj->GetDeformBBox(t,bs);
	actualBS = bs;

	Interval ivalid = FOREVER;
	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(inode->GetObjectTM(t));
	DWORD oldMode = gw->getRndLimits();
	gw->setRndLimits(oldMode|GW_ILLUM);

	if(drawMod && hasModCA)
	{
		gw->setColor( LINE_COLOR, 1.0f, 0.0f, 0.0f);
		actualBS.Scale(.95f); // stretch the box a teeny bit
		DisplayBox(gw,actualBS);
	}
	
	actualBS = bs;
	if(drawBase && hasBaseCA)
	{
		gw->setColor( LINE_COLOR, 0.0f, 1.0f, 0.0f);
		actualBS.Scale(0.93f); // stretch the box a teeny bit
		DisplayBox(gw,actualBS);
	}
	actualBS = bs;
	Mtl * mtl = inode->GetMtl();
	bool hasMeditCA = false;
	if(mtl)
	{
		ICustAttribContainer * cont = mtl->GetCustAttribContainer();
		if(cont && cont->GetNumCustAttribs()>0)
			hasMeditCA = true;
	}
	if(drawMedit && hasMeditCA)
	{
		gw->setColor( LINE_COLOR, 0.0f, 0.0f, 1.0f);
		actualBS.Scale(0.9f); // stretch the box a teeny bit
		DisplayBox(gw,actualBS);
	}
	actualBS = bs;
	if(drawGlobal)
	{
		gw->setColor( LINE_COLOR, 1.0f, 1.0f, 0.0f);
		actualBS.Scale(0.98f); // stretch the box a teeny bit
		DisplayBox(gw,actualBS);
	}

	gw->setRndLimits(oldMode);
	return 0;
}


/******************************************************************************************************************
*
	This method will be called before a modifier in the pipeline changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/

void XTCStackBossMod::PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline)
{

	ICustAttribContainer * cont = NULL;
	cont = mod->GetCustAttribContainer();
	if(cont && cont->GetNumCustAttribs()>0 )
	{
		hasModCA = true;
	}
}

/******************************************************************************************************************
*
	This method will be called after a modifier in the pipeline changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/

void XTCStackBossMod::PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline)
{

	ICustAttribContainer * cont = NULL;

	IDerivedObject * dObj;
	int modIndex;
	mod->GetIDerivedObject(&mc,dObj,modIndex);

	Object * pObj = dObj->GetObjRef();

	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject*) pObj;

		// Iterate over all entries of the modifier stack.
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get current modifier.
			Modifier* mod = pDerObj->GetModifier(Idx);

			cont = mod->GetCustAttribContainer();
			if(cont && cont->GetNumCustAttribs()>0)
			{
				hasModCA = true;
			}
			// Next modifier stack entry.
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	//this will be the base object.

	cont = pObj->GetCustAttribContainer();

	if(cont && cont->GetNumCustAttribs()>0)
	{
		hasBaseCA = true;
	}
	cont = mod->GetCustAttribContainer();

	if(cont && cont->GetNumCustAttribs()>0)
		hasModCA = true;

	DebugPrint("PostChanChangedNotify\n");
}
	
BOOL XTCStackBossMod::SuspendObjectDisplay()
{
	//TODO:  Tell the system to display the Object or not
	return FALSE;
}





//--- StackBossMod -------------------------------------------------------
StackBossMod::StackBossMod()
{
	pblock2 = NULL;
	StackBossModDesc.MakeAutoParamBlocks(this);
	custAttribList.SetCount(0);
}

StackBossMod::~StackBossMod()
{
	pblock2 = NULL;
}


void StackBossMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	
	StackBossModDesc.BeginEditParams(ip, this, flags, prev);
	stackbossmod_param_blk.SetUserDlgProc(new SBMDlgProc(this));
}

void StackBossMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	StackBossModDesc.EndEditParams(ip, this, flags, next);
	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

}



Interval StackBossMod::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}


RefTargetHandle StackBossMod::Clone(RemapDir& remap)
{
	StackBossMod* newmod = new StackBossMod();	
	newmod->ReplaceReference(0,remap.CloneRef(pblock2));
	BaseClone(this, newmod, remap);
	return(newmod);
}


IOResult StackBossMod::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult StackBossMod::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

RefTargetHandle StackBossMod::GetReference(int i) 
{ 
	DbgAssert(i==0);
	return pblock2;
}

void StackBossMod::SetReference(int i, RefTargetHandle rtarg) 
{ 
	DbgAssert(i==0);
	pblock2 = (IParamBlock2*)rtarg;
}

Animatable* StackBossMod::SubAnim(int i) 
{
	return NULL;
}

void StackBossMod::CheckForCustomAttributes(ReferenceTarget *object,IDerivedObject *derObj)
{
	ICustAttribContainer * cont = NULL;
	if(derObj){
		cont = derObj->GetCustAttribContainer();
		if(cont)
		{
			for(int i=0; i<cont->GetNumCustAttribs(); i++)
			{
				CustAttrib * ca = cont->GetCustAttrib(i);
				custAttribList.Append(1,&ca);
			}
		}
	}
	if(object)
	{
		cont = object->GetCustAttribContainer();
		if(cont)
		{
			for(int i=0; i<cont->GetNumCustAttribs(); i++)
			{
				CustAttrib * ca = cont->GetCustAttrib(i);
				custAttribList.Append(1,&ca);
			}
		}
		for(int j=0; j<object->NumRefs();j++)
		{
			CheckForCustomAttributes((ReferenceTarget*)object->GetReference(j),NULL);
		}
	}

}

void StackBossMod::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	int i=0;
	TimeValue t = GetCOREInterface()->GetTime();
	Object * wObj = node->EvalWorldState(t).obj;
	ICustAttribContainer *  cont = NULL;
	Object* pObj = node->GetObjectRef();

	if (!pObj) return ;

	// Is derived object ?
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject*) pObj;

		CheckForCustomAttributes(NULL,pDerObj);

		// Iterate over all entries of the modifier stack.
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get current modifier.
			Modifier* mod = pDerObj->GetModifier(Idx);
			CheckForCustomAttributes((ReferenceTarget*)mod,NULL);
			// Next modifier stack entry.
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}
	//this will be the base object.
	CheckForCustomAttributes((ReferenceTarget*)pObj,NULL);
	
	NotifyDependents(Interval(t,t),PART_ALL,REFMSG_CHANGE);
}

void StackBossMod::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{

	TimeValue t = GetCOREInterface()->GetTime();

	Object *bo = node->GetObjectRef();
	BOOL copyCA, copyNorms;
	pblock2->GetValue(StackBossMod::pb_copyCA,t,copyCA,FOREVER);
	pblock2->GetValue(StackBossMod::pb_copyNormals,t,copyNorms,FOREVER);

	if(copyCA && custAttribList.Count()>0)
	{
	
		ICustAttribContainer * cont = obj->GetCustAttribContainer();

		if(!cont){
			obj->AllocCustAttribContainer();
			cont = obj->GetCustAttribContainer();
		}

		for(int i=0; i<custAttribList.Count();i++)
		{
			cont->AppendCustAttrib(custAttribList[i]);
		}
	}
	// We don't allow a stack collapse, to delete us. 
	// We're going to apply ourselves to the collapsed object

	IDerivedObject *derob = NULL;
	if(obj->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) obj;

	custAttribList.SetCount(0);
	derob->AddModifier(this,NULL,derob->NumModifiers());
	
}

RefResult StackBossMod::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

void StackBossMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	BOOL medit,base,mod,global;

	pblock2->GetValue(StackBossMod::pb_showMedit,t,medit,FOREVER);
	pblock2->GetValue(StackBossMod::pb_showBaseObject,t,base,FOREVER);
	pblock2->GetValue(StackBossMod::pb_showMod,t,mod,FOREVER);
	pblock2->GetValue(StackBossMod::pb_globalHighlight,t,global,FOREVER);
	XTCStackBossMod *pObj = NULL;
	pObj = new XTCStackBossMod(medit,mod,base,global);
	os->obj->AddXTCObject(pObj);
	os->obj->SetChannelValidity(EXTENSION_CHAN_NUM, FOREVER);
}
