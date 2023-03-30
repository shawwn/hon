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

[!if EXTENSION]
#define XTC[!output CLASS_NAME_UPPER]_CLASS_ID	Class_ID([!output EXTCLASSID1], [!output EXTCLASSID2])
[!endif]

#define PBLOCK_REF	0

[!if EXTENSION]
class XTC[!output CLASS_NAME] : public XTCObject
{
public:
	XTC[!output CLASS_NAME](){};
	~XTC[!output CLASS_NAME](){};

	Class_ID ExtensionID(){return XTC[!output CLASS_NAME_UPPER]_CLASS_ID;}

	XTCObject *Clone(){return new XTC[!output CLASS_NAME]();};

	void DeleteThis(){delete this;}
	int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj){return 1;}

	ChannelMask DependsOn(){return GEOM_CHANNEL|TOPO_CHANNEL;}
	ChannelMask ChannelsChanged(){return GEOM_CHANNEL;}

	void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline){};
	void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline){};

	
	BOOL SuspendObjectDisplay(){return FALSE;}
	
};
[!endif]

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0


		static IObjParam *ip;			//Access to the interface
		
		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		[!if EXTENSION]
		//TODO: Add the channels that the modifier needs to perform its modification
		ChannelMask ChannelsUsed()  { return EXTENSION_CHANNEL; }
		
		// We have to include the channels, that the extension object changes, so the 
		// PostChanChangedNotify will be called after the modifier added the extension objects
		// to the object flowing up the stack.

		ChannelMask ChannelsChanged() { return EXTENSION_CHANNEL|GEOM_CHANNEL; }

		[!else]
		ChannelMask ChannelsUsed()  { return GEOM_CHANNEL|TOPO_CHANNEL; }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
		[!endif]
		
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return defObjectClassID;}

		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);

		void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
		void NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index);


		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return FALSE;}		
		
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

		BOOL HasUVW();
		void SetGenUVW(BOOL sw);


		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);


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

/*===========================================================================*\
 |	The validity of the parameters.  First a test for editing is performed
 |  then Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/
Interval [!output CLASS_NAME]::LocalValidity(TimeValue t)
{
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  
	//TODO: Return the validity interval of the modifier
	return NEVER;
}


/*************************************************************************************************
*
	Between NotifyPreCollapse and NotifyPostCollapse, Modify is
	called by the system.  NotifyPreCollapse can be used to save any plugin dependant data e.g.
	LocalModData
*
\*************************************************************************************************/

void [!output CLASS_NAME]::NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index)
{
	//TODO:  Perform any Pre Stack Collapse methods here
}



/*************************************************************************************************
*
	NotifyPostCollapse can be used to apply the modifier back onto to the stack, copying over the
	stored data from the temporary storage.  To reapply the modifier the following code can be 
	used

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(bo->SuperClassID() != GEN_DERIVOB_CLASS_ID)
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	// Add ourselves to the top of the stack
	derob->AddModifier(this,NULL,derob->NumModifiers());

*
\*************************************************************************************************/

void [!output CLASS_NAME]::NotifyPostCollapse(INode *node,Object *obj, IDerivedObject *derObj, int index)
{
	//TODO: Perform any Post Stack collapse methods here.

}


/*************************************************************************************************
*
	ModifyObject will do all the work in a full modifier
    This includes casting objects to their correct form, doing modifications
	changing their parameters, etc
*
\************************************************************************************************/


void [!output CLASS_NAME]::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{
	//TODO: Add the code for actually modifying the object
[!if EXTENSION]
	 XTC[!output CLASS_NAME] *pObj = NULL;
	pObj = new  XTC[!output CLASS_NAME]();
	os->obj->AddXTCObject(pObj);
	os->obj->SetChannelValidity(EXTENSION_CHAN_NUM, FOREVER);
[!endif]
}


void [!output CLASS_NAME]::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);	

	[!output CLASS_NAME]Desc.BeginEditParams(ip, this, flags, prev);
}

void [!output CLASS_NAME]::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
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




RefTargetHandle [!output CLASS_NAME]::Clone(RemapDir& remap)
{
	[!output CLASS_NAME]* newmod = new [!output CLASS_NAME]();	
	//TODO: Add the cloning code here
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	BaseClone(this, newmod, remap);
	return(newmod);
}


//From ReferenceMaker 
RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	//TODO: Add code to handle the various reference changed messages
	return REF_SUCCEED;
}

/****************************************************************************************
*
 	NotifyInputChanged is called each time the input object is changed in some way
 	We can find out how it was changed by checking partID and message
*
\****************************************************************************************/

void [!output CLASS_NAME]::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{

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
	//TODO: Set the plugin internal value to sw				
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

