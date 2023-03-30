/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])

[!if EXTENSION]
#define XTC[!output CLASS_NAME]_CLASS_ID	Class_ID([!output EXTCLASSID1], [!output EXTCLASSID2])
[!endif]

#define PBLOCK_REF  SIMPMOD_PBLOCKREF

[!if EXTENSION]
class XTC[!output CLASS_NAME] : public XTCObject
{
public:
	XTC[!output CLASS_NAME]();
	~XTC[!output CLASS_NAME]();

	Class_ID ExtensionID(){return XTC[!output CLASS_NAME]_CLASS_ID;}

	XTCObject *Clone();

	void DeleteThis(){delete this;}
	int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj);

	ChannelMask DependsOn(){return GEOM_CHANNEL|TOPO_CHANNEL;}
	ChannelMask ChannelsChanged(){return GEOM_CHANNEL;}

	void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod);
	void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod);
	
	BOOL SuspendObjectDisplay();
	
};
[!endif]



class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:
		// Parameter block handled by parent

		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier

		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		// From SimpleMod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);

		Interval GetValidity(TimeValue t);

		// Automatic texture support
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};

//This is the callback object used by modifiers to deform "Deformable" objects.
class [!output CLASS_NAME]Deformer: public Deformer {
	public:
		[!output CLASS_NAME]Deformer();
		//TODO: Add other plugin specific constructors and member functions
		Point3 Map(int i, Point3 p); 
	};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]


IObjParam *[!output CLASS_NAME]::ip			= NULL;


[!if EXTENSION]

// TODO: Perform any Class Setup here.
XTC[!output CLASS_NAME]::XTC[!output CLASS_NAME]()
{

}

XTC[!output CLASS_NAME]::~XTC[!output CLASS_NAME]()
{

}

XTCObject * XTC[!output CLASS_NAME]::Clone()
{
	//TODO: Perform any class initialisation ready for a clone.

	return new XTC[!output CLASS_NAME]();
}	

int XTC[!output CLASS_NAME]::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj)
{
	//TODO: Add Extension Objects drawing routine here
	return 0;
}


/******************************************************************************************************************
*
	This method will be called before a modifier in the pipleine changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/

void XTC[!output CLASS_NAME]::PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod)
{

}

/******************************************************************************************************************
*
	This method will be called after a modifier in the pipleine changes any channels that this XTCObject depends on
	The channels the XTCObect will react on are determine by the call to DependsOn() 
*
\******************************************************************************************************************/

void XTC[!output CLASS_NAME]::PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod)
{

}
	
BOOL XTC[!output CLASS_NAME]::SuspendObjectDisplay()
{
	//TODO:  Tell the system to display the Object or not
	return FALSE;
}
	
[!endif]


[!output CLASS_NAME]Deformer::[!output CLASS_NAME]Deformer() 
{ 
	
}

/*************************************************************************************************
*
 	Map is called for every deformable point in the object
*
\*************************************************************************************************/

Point3 [!output CLASS_NAME]Deformer::Map(int i, Point3 p)
{
	//TODO: Add code to deform or alter a single point
	return p;
}

//--- [!output CLASS_NAME] -------------------------------------------------------
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
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	SimpleMod2::BeginEditParams(ip,flags,prev);
	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);
}

void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	SimpleMod2::EndEditParams(ip,flags,next);
	[!output CLASS_NAME]Desc.EndEditParams(ip, this, flags, next);

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	this->ip = NULL;
}



Interval [!output CLASS_NAME]::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	//TODO: Return the validity interval of the modifier
	return valid;
}

Deformer& [!output CLASS_NAME]::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	static [!output CLASS_NAME]Deformer deformer;
	//TODO: Add code to modify the deformer object
	return deformer;
}



RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir& remap)
{
	[!output CLASS_NAME]* newmod = new [!output CLASS_NAME]();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock2));
	newmod->SimpleModClone(this, remap);
	BaseClone(this, newmod, remap);
	return(newmod);
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

/*===========================================================================*\
 |	SimpleMod2 overloads
\*===========================================================================*/

RefTargetHandle SimpleMod2::GetReference(int i) 
	{ 
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock2;
		default: return NULL;
		}
	}

void SimpleMod2::SetReference(int i, RefTargetHandle rtarg) 
	{ 
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock2 = (IParamBlock2*)rtarg; break;
		}
	}

Animatable* SimpleMod2::SubAnim(int i) 
	{ 
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock2;		
		default: return NULL;
		}
	}
