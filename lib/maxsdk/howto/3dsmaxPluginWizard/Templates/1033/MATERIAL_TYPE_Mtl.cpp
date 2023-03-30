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


#define NSUBMTL		1 // TODO: number of sub-materials supported by this plugin 

#define PBLOCK_REF	NSUBMTL

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		Mtl				*submtl[NSUBMTL];  //array of sub-materials
		BOOL			mapOn[NSUBMTL];
		float			spin;
		Interval		ivalid;
		
		ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		Interval Validity(TimeValue t);
		void Reset();

		void NotifyChanged();

		// From MtlBase and Mtl
		void SetAmbient(Color c, TimeValue t);		
		void SetDiffuse(Color c, TimeValue t);		
		void SetSpecular(Color c, TimeValue t);
		void SetShininess(float v, TimeValue t);
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);		
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
		float WireSize(int mtlNum=0, BOOL backFace=FALSE);
				

		// Shade and displacement calculation
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 	

		// SubMaterial access methods
		int NumSubMtls() {return NSUBMTL;}
		Mtl* GetSubMtl(int i);
		void SetSubMtl(int i, Mtl *m);
		TSTR GetSubMtlSlotName(int i);
		TSTR GetSubMtlTVName(int i);

		// SubTexmap access methods
		int NumSubTexmaps() {return 0;}
		Texmap* GetSubTexmap(int i);
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		TSTR GetSubTexmapTVName(int i);
		
		BOOL SetDlgThing(ParamDlg* dlg);
		[!output CLASS_NAME](BOOL loading);

		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1+NSUBMTL; }
		Animatable* SubAnim(int i); 
		TSTR SubAnimName(int i);

		// TODO: Maintain the number or references here 
		int NumRefs() { return 1+NSUBMTL; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);



		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }
};


[!output TEMPLATESTRING_CLASSDESC]


[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]



[!output CLASS_NAME]::[!output CLASS_NAME](BOOL loading) 
{
	for (int i=0; i<NSUBMTL; i++) submtl[i] = NULL;
	pblock = NULL;

	if (!loading) 
		Reset();
}


void [!output CLASS_NAME]::Reset() 
{
	ivalid.SetEmpty();
	for (int i=0; i<NSUBMTL; i++) {
		if( submtl[i] ){ 
			DeleteReference(i);
			submtl[i] = NULL;
		}

	}

	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}



ParamDlg* [!output CLASS_NAME]::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* masterDlg = [!output CLASS_NAME]Desc.CreateParamDlgs(hwMtlEdit, imp, this);
	
	// TODO: Set param block user dialog if necessary
	return masterDlg;
}

BOOL [!output CLASS_NAME]::SetDlgThing(ParamDlg* dlg)
{
	return FALSE;
}

Interval [!output CLASS_NAME]::Validity(TimeValue t)
{
	Interval valid = FOREVER;		

	for (int i=0; i<NSUBMTL; i++) 
	{
		if (submtl[i]) 
			valid &= submtl[i]->Validity(t);
	}
	
	float u;
	pblock->GetValue(pb_spin,t,u,valid);
	return valid;
}

/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

RefTargetHandle [!output CLASS_NAME]::GetReference(int i) 
{
	if (i < NSUBMTL )
		return submtl[i];
	else return pblock;
}

void [!output CLASS_NAME]::SetReference(int i, RefTargetHandle rtarg) 
{
	if (i < NSUBMTL)
		submtl[i] = (Mtl *)rtarg; 
	else pblock = (IParamBlock2 *)rtarg; 
}

TSTR [!output CLASS_NAME]::SubAnimName(int i) 
{
	if (i < NSUBMTL)
		return GetSubMtlTVName(i);
	else return TSTR(_T(""));
}

Animatable* [!output CLASS_NAME]::SubAnim(int i) {
	if (i < NSUBMTL)
		return submtl[i]; 
	else return pblock;
	}

RefResult [!output CLASS_NAME]::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
				[!output CLASS_NAME_LOWER]_param_blk.InvalidateUI(changing_param);
				}
			break;

		}
	return REF_SUCCEED;
}

/*===========================================================================*\
 |	SubMtl get and set
\*===========================================================================*/

Mtl* [!output CLASS_NAME]::GetSubMtl(int i)
{
	if (i < NSUBMTL )
		return submtl[i];
	return NULL;
}

void [!output CLASS_NAME]::SetSubMtl(int i, Mtl *m)
{
	ReplaceReference(i,m);
	// TODO: Set the material and update the UI	
}

TSTR [!output CLASS_NAME]::GetSubMtlSlotName(int i)
{
	// Return i'th sub-material name 
	return _T(""); 
}

TSTR [!output CLASS_NAME]::GetSubMtlTVName(int i)
{
	return GetSubMtlSlotName(i);
}

/*===========================================================================*\
 |	Texmap get and set
 |  By default, we support none
\*===========================================================================*/

Texmap* [!output CLASS_NAME]::GetSubTexmap(int i)
{
	return NULL;
}

void [!output CLASS_NAME]::SetSubTexmap(int i, Texmap *m)
{
}

TSTR [!output CLASS_NAME]::GetSubTexmapSlotName(int i)
{
	return _T("");
}

TSTR [!output CLASS_NAME]::GetSubTexmapTVName(int i)
{
	// Return i'th sub-texture name 
	return GetSubTexmapSlotName(i);
}



/*===========================================================================*\
 |	Standard IO
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000

IOResult [!output CLASS_NAME]::Save(ISave *isave) { 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
	}	

IOResult [!output CLASS_NAME]::Load(ILoad *iload) { 
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	return IO_OK;
	}


/*===========================================================================*\
 |	Updating and cloning
\*===========================================================================*/

RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir &remap) {
	[!output CLASS_NAME] *mnew = new [!output CLASS_NAME](FALSE);
	*((MtlBase*)mnew) = *((MtlBase*)this); 
	mnew->ReplaceReference(NSUBMTL,remap.CloneRef(pblock));

	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBMTL; i++) {
		mnew->submtl[i] = NULL;
		if (submtl[i])
			mnew->ReplaceReference(i,remap.CloneRef(submtl[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

void [!output CLASS_NAME]::NotifyChanged() 
{
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void [!output CLASS_NAME]::Update(TimeValue t, Interval& valid) 
{	
	if (!ivalid.InInterval(t)) {

		ivalid.SetInfinite();
		pblock->GetValue( mtl_mat1_on, t, mapOn[0], ivalid);
		pblock->GetValue( pb_spin, t, spin, ivalid);

		for (int i=0; i<NSUBMTL; i++) {
			if (submtl[i]) 
				submtl[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
}

/*===========================================================================*\
 |	Determine the characteristics of the material
\*===========================================================================*/

void [!output CLASS_NAME]::SetAmbient(Color c, TimeValue t) {}		
void [!output CLASS_NAME]::SetDiffuse(Color c, TimeValue t) {}		
void [!output CLASS_NAME]::SetSpecular(Color c, TimeValue t) {}
void [!output CLASS_NAME]::SetShininess(float v, TimeValue t) {}
				
Color [!output CLASS_NAME]::GetAmbient(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetAmbient(mtlNum,backFace):Color(0,0,0);
}

Color [!output CLASS_NAME]::GetDiffuse(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetDiffuse(mtlNum,backFace):Color(0,0,0);
}

Color [!output CLASS_NAME]::GetSpecular(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetSpecular(mtlNum,backFace):Color(0,0,0);
}

float [!output CLASS_NAME]::GetXParency(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetXParency(mtlNum,backFace):0.0f;
}

float [!output CLASS_NAME]::GetShininess(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetShininess(mtlNum,backFace):0.0f;
}

float [!output CLASS_NAME]::GetShinStr(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->GetShinStr(mtlNum,backFace):0.0f;
}

float [!output CLASS_NAME]::WireSize(int mtlNum, BOOL backFace)
{
	return submtl[0]?submtl[0]->WireSize(mtlNum,backFace):0.0f;
}

		
/*===========================================================================*\
 |	Actual shading takes place
\*===========================================================================*/

void [!output CLASS_NAME]::Shade(ShadeContext& sc) 
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;
	if (gbufID) sc.SetGBufferID(gbufID);

	if(sm1) sm1->Shade(sc);
	
	// TODO: compute the color and transparency output returned in sc.out.
}

float [!output CLASS_NAME]::EvalDisplacement(ShadeContext& sc)
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;
	return (sm1)?sm1->EvalDisplacement(sc):0.0f;
}

Interval [!output CLASS_NAME]::DisplacementValidity(TimeValue t)
{
	Mtl *sm1 = mapOn[0]?submtl[0]:NULL;

	Interval iv; iv.SetInfinite();
	if(sm1) iv &= sm1->DisplacementValidity(t);

	return iv;	
}
	

