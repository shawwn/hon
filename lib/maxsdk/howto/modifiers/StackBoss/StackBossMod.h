/**********************************************************************
 *<
	FILE: StackBossMod.h

	DESCRIPTION:	Includes for Plugins

	CREATED BY:		Neil Hazzard	

	HISTORY:		October 2003

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __STACKBOSSMOD__H
#define __STACKBOSSMOD__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "Simpmod.h"
#include "Simpobj.h"

#include "meshadj.h"
#include "XTCObject.h"

#include "ICustAttribContainer.h"
#include "custattrib.h"
#include "MeshNormalSpec.h"
#include "MNNormalSpec.h"
#include "polyobj.h"


extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define STACKBOSSMOD_CLASS_ID	Class_ID(0x5521ef2d, 0x38a2d032)
#define XTCSTACKBOSSMOD_CLASS_ID	Class_ID(0x458b0ba1, 0x1ef090d8)

#define STACKBOSS_CHAN_CHANGED PART_GEOM|PART_TOPO|PART_SELECT|PART_VERTCOLOR|TEXMAP_CHANNEL

class XTCStackBossMod : public XTCObject
{
private:
	bool drawMedit;
	bool drawMod;
	bool drawBase;
	bool drawGlobal;
	bool hasModCA;
	bool hasBaseCA;
public:
	XTCStackBossMod();
	XTCStackBossMod(bool medit, bool mod, bool base, bool global);
	XTCStackBossMod(XTCStackBossMod * mFrom);
	~XTCStackBossMod();

	Class_ID ExtensionID(){return XTCSTACKBOSSMOD_CLASS_ID;}

	XTCObject *Clone(){return new XTCStackBossMod(this);}

	void DeleteThis(){delete this;}
	int  Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, Object *pObj);

	ChannelMask DependsOn(){return ALL_CHANNELS;}
	ChannelMask ChannelsChanged(){return ALL_CHANNELS;}

	void PreChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline);
	void PostChanChangedNotify(TimeValue t, ModContext &mc, ObjectState* os, INode *node,Modifier *mod, bool bEndOfPipeline);

	BOOL SuspendObjectDisplay();

};


class StackBossMod : public Modifier {

private:

	Tab < CustAttrib *> custAttribList;

public:

	enum { 
		pb_delete,
		pb_showMedit,
		pb_showMod,
		pb_showBaseObject,
		pb_copyCA,
		pb_copyNormals,
		pb_globalHighlight,
	};

	// Parameter block handled by parent
	IParamBlock2 * pblock2;

	// From Animatable
	TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

	//From Modifier
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);

	Class_ID InputType() {return defObjectClassID;}
	ChannelMask ChannelsUsed()  { return GEOM_CHANNEL | TOPO_CHANNEL | EXTENSION_CHANNEL; }
	ChannelMask ChannelsChanged() { return EXTENSION_CHANNEL|STACKBOSS_CHAN_CHANGED; }
	BOOL ChangeTopology() {return FALSE;}	

	Interval GetValidity(TimeValue t);

	// Automatic texture support

	// Loading/Saving
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	//From Animatable
	Class_ID ClassID() {return STACKBOSSMOD_CLASS_ID;}		
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
	void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

	int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

	int NumRefs(){return 1;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i,RefTargetHandle rtarg);

	int NumSubs(){return 0;}
	Animatable * SubAnim(int i);

	void DeleteThis() { delete this; }		
	//Constructor/Destructor

	StackBossMod();
	~StackBossMod();		
	int NumSubObjTypes() {return 0;}
	ISubObjType *GetSubObjType(int i){return NULL;}

	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc) {return 1;}

	void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index);
	void NotifyPostCollapse(INode *node, Object *obj, IDerivedObject *derObj, int index);

	void CheckForCustomAttributes(ReferenceTarget *object,IDerivedObject *derObj);



};


class SBMDlgProc : public ParamMap2UserDlgProc 
{
public:
	StackBossMod *sbm;
	SBMDlgProc(){}
	SBMDlgProc(StackBossMod *sbm_in) { sbm = sbm_in; }
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
};

#pragma warning(disable: 4800)	//forcing to bool warning

#endif // __STACKBOSSMOD__H
