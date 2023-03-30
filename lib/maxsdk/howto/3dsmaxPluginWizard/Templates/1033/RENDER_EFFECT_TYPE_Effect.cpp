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

		// Effect class methods
		TSTR GetName() { return GetString(IDS_CLASS_NAME); }
		EffectParamDlg *CreateParamDialog( IRendParams *pParams );
		int RenderBegin( TimeValue t, ULONG flags );
		int RenderEnd( TimeValue t );
		void Update( TimeValue t, Interval& valid );
		void Apply( TimeValue t, Bitmap *pBM, RenderGlobalContext *pGC, CheckAbortCallback *checkAbort);
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return RENDER_EFFECT_CLASS_ID; }
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


[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
	assert(pblock);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{
}


/*===========================================================================*\
 |	Standard Load
\*===========================================================================*/

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	Effect::Load(iload);
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	Effect::Save(isave);
	return IO_OK;
}

RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* newObj = new [!output CLASS_NAME]();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}
/*===========================================================================*\
 |	Ask the ClassDesc2 to make the AUTO_UI [!output CLASS_NAME]DlgProc
\*===========================================================================*/

EffectParamDlg *[!output CLASS_NAME]::CreateParamDialog(IRendParams *ip)
{		
	return [!output CLASS_NAME]Desc.CreateParamDialogs(ip, this);
}

RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			[!output CLASS_NAME_LOWER]_param_blk.InvalidateUI();
			break;
	}
	return REF_SUCCEED;
}


/*===========================================================================*\
 |	This method is called once per frame when the renderer begins.  
\*===========================================================================*/

void [!output CLASS_NAME]::Update(TimeValue t, Interval& valid)
{
	//TODO: Implement the actual effect
}


/*===========================================================================*\
 |	Called to initialize or clear up before and after rendering  
\*===========================================================================*/

int [!output CLASS_NAME]::RenderBegin(TimeValue t, ULONG flags)
{
	return 0;
}

int [!output CLASS_NAME]::RenderEnd(TimeValue t)
{
	return 0;
}

/*===========================================================================*\
 |	Apply the actual changes to the rendered bitmap at time 't'
 *===========================================================================*/

void [!output CLASS_NAME]::Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc,CheckAbortCallback *checkAbort) 
{

}
