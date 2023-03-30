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

class [!output CLASS_NAME];
class [!output CLASS_NAME]Gen: public ShadowGenerator 
{ 
public:
	[!output CLASS_NAME] *theShad;

	Matrix3 lightToWorld;
	Matrix3 worldToLight;
	Matrix3 camToLight;

	LightObject *light;
	ObjLightDesc *ltDesc;


	[!output CLASS_NAME]Gen([!output CLASS_NAME] *shad, LightObject *l,  ObjLightDesc *ld, ULONG flags) { 
		theShad = shad;
		light = l;
		ltDesc = ld;
		} 

	
	[!output CLASS_NAME]Gen() {} 
	~[!output CLASS_NAME]Gen() { FreeBuffer();  }

	// Update the shadow generator per frame
	int Update(TimeValue t,const RendContext& rendCntxt,RenderGlobalContext *rgc,
		Matrix3& lightToWorld,float aspect,float param,float clipDist);

	int UpdateViewDepParams(const Matrix3& worldToCam) { 
		camToLight = Inverse(lightToWorld * worldToCam);
		return 1;
		}

	// Clean up methods
	void FreeBuffer() {}
	void DeleteThis() {delete this; }

	// Perform the shadow sampling
	float Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope);
	float Sample(ShadeContext &sc, Point3 &norm, Color& color);
};

class [!output CLASS_NAME]Dlg: public ShadowParamDlg 
{
public:
	[!output CLASS_NAME] *theShad;
	Interface *ip;
	IParamMap2 *pmap;

	// Constructor
	[!output CLASS_NAME]Dlg([!output CLASS_NAME] *shad, Interface *iface);
	~[!output CLASS_NAME]Dlg();

	void DeleteThis() { delete this; }
};

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:


		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		[!output CLASS_NAME]Dlg *theParam;
		// Create the shadow system's UI
		ShadowParamDlg *CreateShadowParamDlg(Interface *ip) { 
			theParam = new [!output CLASS_NAME]Dlg(this, ip); return theParam;
			}

		// Create a shadow generator instance - only exists during a render
		ShadowGenerator* CreateShadowGenerator(LightObject *l,  ObjLightDesc *ld, ULONG flags) {
			return new [!output CLASS_NAME]Gen(this,l,ld,flags);
			}

		BOOL SupportStdMapInterface() { return FALSE; }


		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return SHADOW_TYPE_CLASS_ID; }
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
	theParam = NULL;
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
	assert(pblock);
	}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{
	if (theParam) theParam->theShad = NULL;
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

IOResult [!output CLASS_NAME]::Save(ISave *isave) 
{
	//TODO: Add code to allow plugin to save its data
	return IO_OK;
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload) 
{ 
	//TODO: Add code to allow plugin to load its data
	return IO_OK;
}

RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* newObj = new [!output CLASS_NAME]();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}

int [!output CLASS_NAME]Gen::Update(TimeValue t,const RendContext& rendCntxt,RenderGlobalContext *rgc,
		Matrix3& ltToWorld,float aspect,float param,float clipDist)
{
	lightToWorld  = ltToWorld;
	worldToLight = Inverse(lightToWorld);

	return 1;
}

float [!output CLASS_NAME]Gen::Sample(ShadeContext &sc, Point3 &norm, Color& color)
{ 
	return 1.0f;
}

float [!output CLASS_NAME]Gen::Sample(ShadeContext &sc, float x, float y, float z, float xslope, float yslope)
{
	return 1.0f;
}

/*===========================================================================*\
 |	Support the Parammap UI
\*===========================================================================*/

[!output CLASS_NAME]Dlg::[!output CLASS_NAME]Dlg([!output CLASS_NAME] *shad, Interface *intface) {
	theShad = shad;
	ip = intface;

	pmap = CreateCPParamMap2(
		theShad->pblock,
		ip,
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		GetString(IDS_PARAMS),
		0);
	}

[!output CLASS_NAME]Dlg::~[!output CLASS_NAME]Dlg(){
	if(theShad) theShad->theParam = NULL;
	DestroyCPParamMap2(pmap);
	}

