/**********************************************************************
 *<
	FILE: iparamb2.h

	DESCRIPTION: Interface to Parameter blocks, 2nd edition

	CREATED BY: Rolf Berteig,
				John Wainwright, 2nd Ed.

	HISTORY: created 1/25/95
			 2nd Ed. 9/2/98

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __IPARAMB2__
#define __IPARAMB2__

#include <iparamb.h>
#include <iparamm.h>

#ifdef BLD_PARAMBLK2
#	define PB2Export __declspec( dllexport )
#else
#	define PB2Export __declspec( dllimport )
#endif

// parameters & blocks have permanent, position independent IDs
typedef short ParamID;
typedef short BlockID;
typedef short MapID;

// WIN64 Cleanup: Shuler
// Need a String ResourcId Type
// Correct this if it doesn't fit with design!
typedef INT_PTR StringResID;
typedef INT_PTR ResID;

// per descriptor flags
#define P_CLASS_PARAMS		0x0001		// this block holds class-level parameters, attached to ClassDesc
#define P_AUTO_CONSTRUCT	0x0002		// instructs ClassDesc2 to autoconstuct this block & wire it in, requires pblock refno
#define	P_AUTO_UI			0x0004		// this block support automatic UI rollout managements, requires rollout template res ID, etc.
#define	P_USE_PARAMS		0x0008		// this block shares (exactly) the paramdefs from another descriptor, requires addr of source descriptor
#define	P_INCLUDE_PARAMS	0x0010		// this block loads in a copy the paramdefs from another descriptor, requires addr of source descriptor
#define P_MULTIMAP			0x0020		// indicates this block as mulitple parameter maps   ### move me
#define P_CALLSETS_ON_LOAD	0x0040		// causes CallSets() to be called during load PLCB for this block
#define P_HASCATEGORY		0x0080		// indicates, that category field is defined for rollup (after rollupproc)
#define P_TEMPLATE_UI		0x0100		// indicates that dialog templates will be provided or constructed
//! \brief Indicates a parameter block version number is specified
/*! When used as a ParamBlockDesc2 flag, a version number is expected immediately after the flags field, before the reference number field.
    This version number will be stored as ParamBlockDesc2::version and returned as IParamBlock2::GetVersion() */
#define P_VERSION			0x0200		// indicates a version number is specified (integer value, before pblock refno)

// per descriptor internal flags
#define	P_SCRIPTED_CLASS	0x1000		// belongs to a scripted plug-in class
#define	P_TEMPORARY			0x2000		// temporary descriptor built during scene load to support schema migration

// per param constructor-specifiable flags
#define P_ANIMATABLE		0x000001	// animatable param
#define P_TRANSIENT			0x000002	// do not store actual value, PBAccessor-derived
#define P_NO_INIT			0x000004	// do not initialize
#define P_COMPUTED_NAME		0x000008	// call compute name fn to get name
#define P_INVISIBLE			0x000010	// not visible in track view (if an animatable)
#define P_RESET_DEFAULT		0x000020	// do not make create params sticky, reset to defaults always
#define P_SUBANIM			0x000040	// non-animatable reference param is still a subanim (makes it visible in TV)
#define P_TV_SHOW_ALL		0x000080	// for Tab<> animatables, show all entries even if no controller assigned
#define P_NO_REF			0x000100	// for reftarg params do not maintain Reference automatically
#define P_OWNERS_REF		0x000200	// reference param maintained by owner, specify owner's refno in a p_refno =>P_NO_REF
#define P_CAN_CONVERT		0x000400	// indicates the p_classid validator is is in a CanConvertoTo() call, rather than as exact class
#define P_SUBTEX			0x000800	// indicates texmap param is kept by owner using MtlBase::xSubTexmap protocol, give subtex # in p_subtexno
#define P_VARIABLE_SIZE		0x001000	// Tab<> param is variable size allowing scripted changes
#define P_NO_AUTO_LABELS	0x002000	// don't auto-set map & mtl names for associated button UI controls
#define P_SHORT_LABELS		0x004000	// use short auto names for associated button UI controls
#define P_READ_ONLY			0x008000	// this parameter is not assignable through MAXScript (allows try-and-buy 3rd-party plugins)

// per param internal flags
#define P_IS_REF			0x010000	// is a reftarget param
#define P_HAS_DEFAULT		0x020000	// has accessor function => a virtual param
#define P_HAS_CUR_DEFAULT	0x040000	// has a snapshotted current default value
#define P_HAS_MS_DEFAULT	0x080000	// has a MAXScript default
#define P_HAS_RANGE			0x100000	// has a range specified
#define P_HAS_CLASS_ID		0x200000	// a classID validator was given
#define P_HAS_SCLASS_ID		0x400000	// an SClassID validator was given
#define P_UI_ENABLED		0x800000	// indicates whether UI controls are initially enabled or diabled
#define P_HAS_PROMPT	   0x1000000	// has status line prompt string res ID for various picker buttons
#define P_HAS_CAPTION	   0x2000000	// has caption string res ID for open/save file dlgs
#define P_HAS_FILETYPES	   0x4000000	// has file types string res ID for open/save file dlgs (in MAXScript type: form)
#define P_HAS_REFNO		   0x8000000	// has refno supplied
#define P_HAS_SUBTEXNO	  0x10000000	// has subtexno supplied
#define P_INCLUDED		  0x20000000	// included from another descriptor, don't double free
//! \brief Indicates parameter is obsolete
/* When used as a ParamBlockDesc2 param flag, the param will not be saved to disk or maintain referecnes or animation keys.
   When loading older files, the parameter will be reset after the load is completed, but will be valid during a post-load callback */
#define P_OBSOLETE		  0x40000000	// obsolete param, don't maintain refs, a controller or a tab, and don't save to file

// Parameter types
#include "paramtype.h"  // parameter type codes

#define base_type(t)	((ParamType2)((t) & ~(TYPE_TAB)))	// get base type ignoring Tab flag
#define root_type(t)	((ParamType2)((t) & ~(TYPE_TAB | TYPE_BY_VAL | TYPE_BY_REF | TYPE_BY_PTR)))	// get base type ignoring all flags
#define is_tab(t)		((t) & TYPE_TAB)					// is this param a table?
#define is_by_val(t)	((t) & TYPE_BY_VAL)					// is this param passed by value?  (only for FnPub)
#define is_by_ref(t)	((t) & TYPE_BY_REF)					// is this param passed by reference?  (only for FnPub)
#define is_by_ptr(t)	((t) & TYPE_BY_PTR)					// is this param passed by pointer?  (only for FnPub)
#define is_ref(d)		(((d).flags & (P_IS_REF | P_NO_REF | P_OWNERS_REF)) == P_IS_REF) // is this param a true local refmaker?
#define has_ui(d)		((d).ctrl_count > 0)				// this param has UI info defined
#define animatable_type(t) (base_type(t) == TYPE_INT || base_type(t) == TYPE_RGBA ||  base_type(t) == TYPE_HSV || base_type(t) == TYPE_POINT3 || \
	                        base_type(t) == TYPE_FLOAT || base_type(t) == TYPE_ANGLE || base_type(t) == TYPE_BOOL || base_type(t) == TYPE_PCNT_FRAC || \
	                        base_type(t) == TYPE_WORLD || base_type(t) == TYPE_COLOR_CHANNEL || base_type(t) == TYPE_TIMEVALUE  || \
							base_type(t) == TYPE_INDEX  || base_type(t) == TYPE_RADIOBTN_INDEX || base_type(t) == TYPE_FRGBA || base_type(t) == TYPE_POINT4)
#define reftarg_type(t) (base_type(t) == TYPE_MTL || base_type(t) == TYPE_TEXMAP || base_type(t) == TYPE_INODE || \
	                     base_type(t) == TYPE_REFTARG || base_type(t) == TYPE_PBLOCK2 || base_type(t) == TYPE_OBJECT || base_type(t) == TYPE_CONTROL)
        
class ParamBlockDesc2;
class ClassDesc;
class PBBitmap;
class ParamMap2UserDlgProc;
class MSPluginClass;
class Value;
class Rollout;
class FPInterface;

// parameter value
#pragma pack(push,parameter_entry)
//#pragma pack(1)	// this messes up Win64 builds & the GreatCircle memory debugger

class Texmap;
class Mtl;
class INode;
class IAutoMParamDlg;
class IAutoSParamDlg;
class IAutoEParamDlg;
struct ParamDef;

typedef struct 
{
	union 
	{
		int					i;
		float				f;
		Point3*				p;
		Point4*				p4;
		TimeValue			t;
		TCHAR*				s;
		PBBitmap*			bm;
		ReferenceTarget*	r;
		// new for R4
		Matrix3*			m;
		Control*			control;  // replaces i,f,p or t values if animated
	};
	BYTE flags;
	PB2Export BOOL is_constant();
	PB2Export void Free(ParamType2 type);
} PB2Value;

#pragma pack(pop,parameter_entry)

// defines a parameter alias
typedef struct 
{
	TCHAR*	alias;
	ParamID	ID;
	int		tabIndex;
} ParamAlias;

// the interface to a ParamBlock2
class IParamBlock2 : public ReferenceTarget 
{
	public:
		virtual DWORD		GetVersion()=0;
		virtual int			NumParams()=0;
		virtual TCHAR*		GetLocalName()=0;
		// acquire & release the descriptor for this paramblock, get individual paramdefs
		virtual ParamBlockDesc2* GetDesc()=0;
		virtual void		ReleaseDesc()=0;
		virtual void		SetDesc(ParamBlockDesc2* desc)=0;
		virtual ParamDef&	GetParamDef(ParamID id)=0;
		// access block ID
		virtual BlockID		ID()=0;
		// index-to/from-ID conversion
		virtual int			IDtoIndex(ParamID id)=0;
		virtual ParamID		IndextoID(int i)=0;
		// get object that owns this block
		virtual ReferenceMaker* GetOwner()=0;

		// Get's the super class of a parameters controller
		virtual SClass_ID GetAnimParamControlType(int anim)=0;
		virtual SClass_ID GetParamControlType(ParamID id)=0;
		// Get the param type & name
		virtual ParamType2 GetParameterType(ParamID id)=0;
		virtual TSTR GetLocalName(ParamID id, int tabIndex = -1)=0; 

		// parameter accessors, one for each known type
		virtual BOOL SetValue(ParamID id, TimeValue t, float v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, int v, int tabIndex=0)=0;		
		virtual BOOL SetValue(ParamID id, TimeValue t, Point3& v, int tabIndex=0)=0;		
		virtual BOOL SetValue(ParamID id, TimeValue t, Point4& v, int tabIndex=0)=0;		
		virtual BOOL SetValue(ParamID id, TimeValue t, Color& v, int tabIndex=0)=0;  // uses Point3 controller
		virtual BOOL SetValue(ParamID id, TimeValue t, AColor& v, int tabIndex=0)=0;  // uses Point4 controller
		virtual BOOL SetValue(ParamID id, TimeValue t, TCHAR* v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, Mtl*	v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, Texmap* v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, PBBitmap* v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, INode* v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, ReferenceTarget*	v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, IParamBlock2* v, int tabIndex=0)=0;
		virtual BOOL SetValue(ParamID id, TimeValue t, Matrix3& v, int tabIndex=0)=0;		

		virtual BOOL GetValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Point3& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Point4& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Color& v, Interval &ivalid, int tabIndex=0)=0; // uses Point3 controller
		virtual BOOL GetValue(ParamID id, TimeValue t, AColor& v, Interval &ivalid, int tabIndex=0)=0; // uses Point4 controller
		virtual BOOL GetValue(ParamID id, TimeValue t, TCHAR*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Mtl*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Texmap*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, PBBitmap*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, INode*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, ReferenceTarget*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, IParamBlock2*& v, Interval &ivalid, int tabIndex=0)=0;
		virtual BOOL GetValue(ParamID id, TimeValue t, Matrix3& v, Interval &ivalid, int tabIndex=0)=0;

		// short cut getters for each type
		virtual Color		GetColor(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual AColor		GetAColor(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual Point3		GetPoint3(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual Point4		GetPoint4(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual int			GetInt(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual float		GetFloat(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual TimeValue	GetTimeValue(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual TCHAR*		GetStr(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual Mtl*		GetMtl(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual Texmap*		GetTexmap(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual PBBitmap*	GetBitmap(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual INode*		GetINode(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual ReferenceTarget* GetReferenceTarget(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual IParamBlock2* GetParamBlock2(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		virtual Matrix3		GetMatrix3(ParamID id, TimeValue t=0, int tabIndex=0)=0;
		// and one to get the value in a PB2Value 
		virtual PB2Value& GetPB2Value(ParamID id, int tabIndex=0)=0;


		// parameter Tab management
		virtual int		Count(ParamID id)=0;
		virtual void	ZeroCount(ParamID id)=0;
		virtual void	SetCount(ParamID id, int n)=0;
		virtual int		Delete(ParamID id, int start,int num)=0; 
		virtual int		Resize(ParamID id, int num)=0;
		virtual void	Shrink(ParamID id)=0;
		virtual void	Sort(ParamID id, CompareFnc cmp)=0;
		// Tab Insert for each type
		virtual int		Insert(ParamID id, int at, int num, float* el)=0;
		virtual int		Insert(ParamID id, int at, int num, Point3** el)=0;
		virtual int		Insert(ParamID id, int at, int num, Point4** el)=0;
		virtual int		Insert(ParamID id, int at, int num, Color** el)=0;
		virtual int		Insert(ParamID id, int at, int num, AColor** el)=0;
		virtual int		Insert(ParamID id, int at, int num, TimeValue* el)=0;
		virtual int		Insert(ParamID id, int at, int num, TCHAR** vel)=0;
		virtual int		Insert(ParamID id, int at, int num, Mtl** el)=0;
		virtual int		Insert(ParamID id, int at, int num, Texmap** el)=0;
		virtual int		Insert(ParamID id, int at, int num, PBBitmap** el)=0;
		virtual int		Insert(ParamID id, int at, int num, INode** v)=0;
		virtual int		Insert(ParamID id, int at, int num, ReferenceTarget** el)=0;
		virtual int		Insert(ParamID id, int at, int num, IParamBlock2** el)=0;
		virtual int		Insert(ParamID id, int at, int num, Matrix3** el)=0;
		// Tab Insert for each type
		virtual int		Append(ParamID id, int num, float* el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Point3** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Point4** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Color** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, AColor** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, TimeValue* el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, TCHAR** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Mtl** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Texmap** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, PBBitmap** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, INode** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, ReferenceTarget** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, IParamBlock2** el, int allocExtra=0)=0;
		virtual int		Append(ParamID id, int num, Matrix3** el, int allocExtra=0)=0;

		// Checks to see if a keyframe exists for the given parameter at the given time
		virtual BOOL KeyFrameAtTime(int i,      TimeValue t, int tabIndex=0) { return FALSE; }
		virtual BOOL KeyFrameAtTime(ParamID id, TimeValue t, int tabIndex=0) { return KeyFrameAtTime(IDtoIndex(id), t, tabIndex); }

		virtual void		RemoveController(int i, int tabIndex)=0;
		virtual Control*	GetController(ParamID id, int tabIndex=0)=0;
		virtual Control*	GetController(int i, int tabIndex=0)=0;
		virtual void		SetController(int i, int tabIndex, Control *c, BOOL preserveFrame0Value=TRUE)=0;
		virtual void		SetController(ParamID id, int tabIndex, Control *c, BOOL preserveFrame0Value=TRUE) { SetController(IDtoIndex(id), tabIndex, c, preserveFrame0Value); }
		virtual	void		SwapControllers(int i1, int tabIndex1, int i2, int tabIndex2)=0;

		// Given the param num & optional Tab<> index, what is the refNum?
		virtual	int GetRefNum(int i, int tabIndex=0)=0;
		// Given the param num & optional Tab<> index, what is the animated param controller refnum?
		virtual	int GetControllerRefNum(int i, int tabIndex=0)=0;

		// Given the parameter ID what is the animNum?
		virtual	int GetAnimNum(ParamID id, int tabIndex=0)=0;

		// Given the animNum what is the parameter index?
		virtual	int AnimNumToParamNum(int animNum, int& tabIndex)=0;

		virtual	ParamDimension* GetParamDimension(int subAnim)=0;

		// This is only for use in a RescaleWorldUnits() implementation:
		// The param block implementation of RescaleWorldUnits scales only tracks
		// that have dimension type = stdWorldDim. If letting the param block handle 
		// the rescaling is not sufficient, call this on just the parameters you need to rescale.
		virtual void RescaleParam(int paramNum, int tabIndex, float f)=0;

		// When a NotifyRefChanged is received from a param block, you 
		// can call this method to find out which parameter generated the notify.
		virtual ParamID LastNotifyParamID()=0;
		virtual ParamID LastNotifyParamID(int& tabIndex)=0;  // variant also returns changing element index
		
		// control notifications, enable/disable send NotifyRefChanged messages when parameters change (via SetValue(), eg)
		virtual void EnableNotifications(BOOL onOff)=0;

		// allows owner to signal pblock when P_OWNER_REF params are deleted
		virtual void RefDeleted(ParamID id, int tabIndex=0)=0;

		// ParamMap2 access, 
		virtual void SetMap(IParamMap2* m, MapID map_id = 0)=0;
		virtual IParamMap2* GetMap(MapID map_id = 0)=0;
		// rollout state, normally used by ParamMap2 to automatically save & restore state 
		virtual void SetRolloutOpen(BOOL open, MapID map_id = 0)=0;
		virtual BOOL GetRolloutOpen(MapID map_id = 0)=0;
		virtual void SetRolloutScrollPos(int pos, MapID map_id = 0)=0;
		virtual int GetRolloutScrollPos(MapID map_id = 0)=0;

		// ParamDlg access, 
		virtual IAutoMParamDlg* GetMParamDlg()=0;
		virtual IAutoEParamDlg* GetEParamDlg()=0;

		// init parameters with MAXScript defaults
		virtual void InitMSParameters()=0;

		// alias maintenance
		virtual void DefineParamAlias(TCHAR* alias_name, ParamID id, int tabIndex=-1)=0;
		virtual ParamAlias* FindParamAlias(TCHAR* alias_name)=0;
		virtual TCHAR* FindParamAlias(ParamID id, int tabIndex=-1)=0;
		virtual void ClearParamAliases()=0;
		virtual int ParamAliasCount()=0;
		virtual ParamAlias* GetParamAlias(int i)=0;

		// set subanim number for given param
		virtual void SetSubAnimNum(ParamID id, int subAnimNum, int tabIndex=0)=0;
		virtual void ClearSubAnimMap()=0;

		// parameter value copying, copy src_id param from src block to id param in this block
		virtual void Assign(ParamID id, IParamBlock2* src, ParamID src_id)=0;

		// find the param ID & tabIndex for given ReferenceTarget(either as a subanim or reftarg parameter)
		virtual ParamID FindRefParam(ReferenceTarget* ref, int& tabIndex)=0;

		// reset params to default values
		virtual void ResetAll(BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE)=0;
		virtual void Reset(ParamID id, int tabIndex=-1, BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE)=0;
		// force a call to the PBAccessor Get()/Set() functions for a param or all params
		virtual void CallSet(ParamID id, int tabIndex=-1)=0;
		virtual void CallGet(ParamID id, int tabIndex=-1)=0;
		virtual void CallSets()=0;
		virtual void CallGets()=0;
		// get validity of all params in all paramblock
		virtual void GetValidity(TimeValue t, Interval &valid)=0;
};

// specialize this class to provide a custom SetValue() validator
// the Validate() function should return falsi if the given PB2Value is
// not valid
class PBValidator : public InterfaceServer
{
public:
	virtual BOOL Validate(PB2Value& v) = 0;
	virtual BOOL Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex) { return this->Validate(v); }
	virtual void DeleteThis() { };
};

// specialize this class to provide 'virtual' parameter value accessor functions
class PBAccessor : public InterfaceServer
{
public:
	// get into v
	virtual void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) { }    
	// set from v
	virtual void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) { }						 
	// computed keyframe presence
	virtual BOOL KeyFrameAtTime(ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) { return FALSE; }
	// computed parameter localized (subanim) name, only called if P_COMPUTED_NAME is flagged
	virtual TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex) { return _T(""); }
	// called when a Tab<> parameter has a change made to its table structure
	enum tab_changes { tab_insert, tab_append, tab_delete, tab_ref_deleted, tab_setcount, tab_sort };
	virtual void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count) { }
	// implement this if your PBAccessors are dynamically allocated
	virtual void DeleteThis() { };
};

// Parameter Block Descriptors
//    in PB2s, there is one ParamBlockDesc2 per entire PB, containing
//    the metadata for all the parameters in the PB.  All the PBs
//    mapped by this structure contain pointers back to it and the 
//    owning class's ClassDesc contains all the PB2Descs for PBs in
//    its objects

// HEY!  for the moment, all the possible optional parameters are in extensis.  If
//       this proves a big memory hog, we can institute some kind of streaming
//       scheme that packs used optionals into a single mem buffer.  The GetDesc() 
//       function should be used to access the PBD in all cases to allow this caching.
//       sizeof(ParmDef) ~ 70 bytes, so for 2500 params =~ 175K bytes
 
#pragma pack(push, parameter_def)
// #pragma pack(1)   // this messes up Win64 builds & the GreatCircle memory debugger

struct ParamDef 
{
	DWORD size;
	//public:
	ParamID		ID;				// pos independent ID
	TCHAR*		int_name;		// fixed internal name
	ParamType2	type;			// parameter type
	int			flags;			// status flags
	// optional
	StringResID	local_name;		// localized (subabim) name (string res id)
	ParamDimension* dim;		// parameter dimension
	PB2Value	def;			// default value
	PB2Value	ms_def;			// default value for MAXScript & MacroRecorder
	PB2Value	cur_def;		// current 'sticky' default value, used to maintain creation defaults within a session
	int			description;	// one sentence description (string res id)
	PB2Value	range_low;		// range values
	PB2Value	range_high;
	PBValidator* validator;		// validator object
	PBAccessor*	accessor;		// virtual param accessor object
	short		tab_size;		// initial table size
	short		ref_no;			// block-owner's refno for non-hosted ReferenceTargets
	short		subobj_no;		// block-owner's SubTex/SubMtl index for Texmap/Mtl parameters in Mtl owners
	Class_ID	class_ID;		// validator for reftargs
	SClass_ID	sclass_ID;		//    "       "     "
	// UI optional
	ControlType2 ctrl_type;		// type of UI control
	EditSpinnerType spin_type;	// spinner type if spinner
	int*		ctrl_IDs;		// array of control IDs for this control  (or ui element names if for scripted plugin)
	short		ctrl_count;		// number of controls
	int*		val_bits;		// radiobutton vals or bit numbers for int bits controlled by multiple checkboxes
	float		scale;			// display scale
// begin - mjm 12.19.98
	int			numSegs;		// slider segments
// end - mjm 12.19.98
	ParamID*	enable_ctrls;	// array of which other params ahave their UI ctrls automatically enabled by this param
	short		enable_count;	// count of enable control params
	int			prompt;			// status line prompt string res ID for various picker buttons
	int			caption;		// caption string res ID for open/save file dlgs
	TCHAR*		init_file;		// initial filename for open/save file dlgs
	int			file_types;		// file types string res ID for open/save file dlgs (in MAXScript type: form)
	// new for R4
	Tab<MapID>	maps;			// maps IDs if in a multi-map block (block flag P_MULTIMAP)


	PB2Export void DeleteThis();
	ParamDef() { size = sizeof(ParamDef);}
};
		
#pragma pack(pop, parameter_def)

/* ----------------------- ClassDesc2 ------------------------------------*/

class MtlBase;
class SpecialFX; // mjm - 07.06.00

// 2nd Edition of ClassDesc with necessary extra stuff for ParamBlock2 support
class ClassDesc2 : public ClassDesc 
{
	private:
		Tab<ParamBlockDesc2*>	pbDescs;		// parameter block descriptors
		Tab<IParamMap2*>		paramMaps;		// any current param maps
		IAutoMParamDlg*			masterMDlg;		// master material/mapParamDlg if any
		IAutoEParamDlg*			masterEDlg;		// master EffectParamDlg if any

	protected:
		
		// [dl | 27feb2003] These methods may be used by derived classes which want
		// to handle the ParamDlg creation themselves (instead of using CreateParamDlg() et al.)
		void SetMParamDlg(IAutoMParamDlg* dlg) { masterMDlg = dlg; }
		void SetEParamDlg(IAutoEParamDlg* dlg) { masterEDlg = dlg; }
		Tab<IParamMap2*>& GetParamMaps() { return paramMaps; }

	public:
		PB2Export				ClassDesc2();
		PB2Export			   ~ClassDesc2();
		PB2Export void			ResetClassParams(BOOL fileReset);
		// ParamBlock2-related metadata
		// access parameter block descriptors for this class
		PB2Export int			NumParamBlockDescs() { return pbDescs.Count(); }
		PB2Export ParamBlockDesc2*	GetParamBlockDesc(int i) { return pbDescs[i]; }
		PB2Export ParamBlockDesc2*	GetParamBlockDescByID(BlockID id);
		PB2Export ParamBlockDesc2*	GetParamBlockDescByName(TCHAR* name);
		PB2Export void			AddParamBlockDesc(ParamBlockDesc2* pbd);
		PB2Export void			ClearParamBlockDescs() { pbDescs.ZeroCount(); }
		// automatic command panel UI management
		PB2Export void			BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
		PB2Export void			EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
		PB2Export void			InvalidateUI();
		PB2Export void			InvalidateUI(ParamBlockDesc2* pbd);
		PB2Export void			InvalidateUI(ParamBlockDesc2* pbd, ParamID id, int tabIndex=-1); // nominated param
		// automatic ParamBlock construction
		PB2Export void			MakeAutoParamBlocks(ReferenceMaker* owner);
		// access automatically-maintained ParamMaps, by simple index or by associated ParamBlockDesc
		PB2Export int			NumParamMaps() { return paramMaps.Count(); }
		PB2Export IParamMap2*	GetParamMap(int i) { return paramMaps[i]; }
		PB2Export IParamMap2*	GetParamMap(ParamBlockDesc2* pbd, MapID map_id = 0);
		// maintain user dialog procs on automatically-maintained ParamMaps
		PB2Export void			SetUserDlgProc(ParamBlockDesc2* pbd, MapID map_id, ParamMap2UserDlgProc* proc=NULL);
		inline void				SetUserDlgProc(ParamBlockDesc2* pbd, ParamMap2UserDlgProc* proc=NULL) { SetUserDlgProc(pbd, 0, proc); }
		PB2Export ParamMap2UserDlgProc*	GetUserDlgProc(ParamBlockDesc2* pbd, MapID map_id = 0);
		// 	automatic UI management 
		PB2Export IAutoMParamDlg* CreateParamDlgs(HWND hwMtlEdit, IMtlParams *imp, ReferenceTarget* obj);
		PB2Export IAutoMParamDlg* CreateParamDlg(BlockID id, HWND hwMtlEdit, IMtlParams *imp, ReferenceTarget* obj, MapID mapID=0);
		PB2Export IAutoEParamDlg* CreateParamDialogs(IRendParams *ip, SpecialFX* obj); // mjm - 07.06.00
		PB2Export IAutoEParamDlg* CreateParamDialog(BlockID id, IRendParams *ip, SpecialFX* obj, MapID mapID=0); // mjm - 07.06.00
		PB2Export void			MasterDlgDeleted(IAutoMParamDlg* dlg);
		PB2Export void			MasterDlgDeleted(IAutoEParamDlg* dlg);
		PB2Export IAutoMParamDlg* GetMParamDlg() { return masterMDlg; }
		PB2Export IAutoEParamDlg* GetEParamDlg() { return masterEDlg; }
		// restore any saved rollout state
		PB2Export void			RestoreRolloutState();
		// find last modified param in all blocks, same as on IParamBlock2, but scans all pb's in the object
		PB2Export ParamID		LastNotifyParamID(ReferenceMaker* owner, IParamBlock2*& pb);
		// reset all params of all known paramblocks to default values, update any UI
		PB2Export void			Reset(ReferenceMaker* owner, BOOL updateUI = TRUE, BOOL callSetHandlers = TRUE);
		// get validity of all params in all owner's paramblocks
		PB2Export void			GetValidity(ReferenceMaker* owner, TimeValue t, Interval &valid);
};

// use the constructors to build both static & dynamic descriptors
class ParamBlockDesc2 : public BaseInterfaceServer
{
private:
	va_list		check_param(va_list ap, int id);
	va_list		scan_param(va_list ap, int id, ParamDef* p);
	va_list		scan_option(va_list ap, int tag, ParamDef* p, TCHAR* parm_name, int& optionnum);

public:
	ParamDef*	paramdefs;		// parameter definitions
	ClassDesc2*	cd;				// owning class
	TCHAR*		int_name;		// fixed internal name
	StringResID	local_name;		// localized (subanim) name string resource ID
	BlockID		ID;				// ID
	USHORT		count;			// number of params in block
	ULONG		version;		// paramblock version
	USHORT		flags;			// block type flags
	// auto-construct optional
	int			ref_no;			// reference number for auto-constructed pb
	// auto-ui optional
	int			dlg_template;	// rollout dialog template resource
	int			title;			// rollout title
	int			test_flags;		// BeginEditParams test flags
	int			rollup_flags;	// add rollup page flags
	ParamMap2UserDlgProc* dlgProc;  // IParamMap2 dialog proc
	int			category;		// category for rollup

	// rollout specs if multi-map block (R4 extension) (flags & P_MULTIMAP)
	typedef struct 
	{
		MapID		map_id;			// ID of the map associated with this rollout
		int			dlg_template;	// rollout dialog template resource
		int			title;			// rollout title
		int			test_flags;		// BeginEditParams test flags
		int			rollup_flags;	// add rollup page flags
		ParamMap2UserDlgProc* dlgProc;  // IParamMap2 dialog proc
		int			category;		// category for rollup
	} map_spec;
	Tab<map_spec> map_specs;	
	// scripted plug-in stuff if this belongs to a scripted plug-in class
	MSPluginClass* pc;			// the scripted class if non-NULL (gc-protected by the scripted plugin class)
	Rollout*	rollout;		// rollout if specified (gc-protected by the scripted plugin class)
	// class param optional
	IParamBlock2* class_params;	// pointer to class paramblock if CLASS_PARAM descriptor
		
	// constructors
			    ParamBlockDesc2();
	PB2Export   ParamBlockDesc2(BlockID ID, TCHAR* int_name, StringResID local_name, ClassDesc2* cd, USHORT flags, ...); 
	PB2Export  ~ParamBlockDesc2();

	// building descriptors incrementally
	PB2Export void AddParam(ParamID id, ...);
	PB2Export void ReplaceParam(ParamID id, ...);
	PB2Export void DeleteParam(ParamID id);
	PB2Export void ParamOption(ParamID id, int option_tag, ...);
	// for delayed setting of ClassDesc2
	PB2Export void SetClassDesc(ClassDesc2* cd);

	// param metrics
	USHORT		Count() { return count; }
	DWORD		Version() { return version; }
	PB2Export int IDtoIndex(ParamID id);
	PB2Export int NameToIndex(TCHAR* name);
	ParamID		IndextoID(int i) { return (((i >= 0) && (i < Count())) ? paramdefs[i].ID : -1); }
	ParamDef&	GetParamDef(ParamID id) { int i = IDtoIndex(id); DbgAssert(i >= 0); return paramdefs[i]; }

	// parameter accessors for static class param blocks, these bounce off to the class paramblock
	BOOL SetValue(ParamID id, TimeValue t, float v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, int v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	BOOL SetValue(ParamID id, TimeValue t, Point3& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	BOOL SetValue(ParamID id, TimeValue t, Point4& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }		
	BOOL SetValue(ParamID id, TimeValue t, Color& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }  // uses Point3 controller
	BOOL SetValue(ParamID id, TimeValue t, AColor& v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }  // uses Point4 controller
	BOOL SetValue(ParamID id, TimeValue t, TCHAR* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, Mtl*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, Texmap* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, PBBitmap* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, INode* v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, ReferenceTarget*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, IParamBlock2*	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }
	BOOL SetValue(ParamID id, TimeValue t, Matrix3&	v, int tabIndex=0) { return class_params->SetValue(id, t, v, tabIndex); }

	BOOL GetValue(ParamID id, TimeValue t, float& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, int& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Point3& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Point4& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Color& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, AColor& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, TCHAR*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Mtl*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Texmap*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, PBBitmap*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, INode*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, ReferenceTarget*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, IParamBlock2*& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }
	BOOL GetValue(ParamID id, TimeValue t, Matrix3& v, Interval &ivalid, int tabIndex=0) { return class_params->GetValue(id, t, v, ivalid, tabIndex); }

	PB2Export Color			GetColor(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export AColor		GetAColor(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Point3		GetPoint3(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Point4		GetPoint4(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export int			GetInt(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export float			GetFloat(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export TimeValue		GetTimeValue(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export TCHAR*		GetStr(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Mtl*			GetMtl(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Texmap*		GetTexmap(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export PBBitmap*		GetBitmap(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export INode*		GetINode(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export ReferenceTarget* GetReferenceTarget(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export IParamBlock2* GetParamBlock2(ParamID id, TimeValue t=0, int tabIndex=0);
	PB2Export Matrix3		GetMatrix3(ParamID id, TimeValue t=0, int tabIndex=0);

	// get a string resource from plug-in module's resource
	PB2Export TCHAR* GetString(StringResID id) { return cd->GetRsrcString(id); }
	// invalidate any current UI (parammap2) currently open for this descriptor
	PB2Export void InvalidateUI() { cd->InvalidateUI(this); }
	PB2Export void InvalidateUI(ParamID id, int tabIndex=-1) { cd->InvalidateUI(this, id, tabIndex); } // nominated param
	// get/set user dialog proc for the param map currently open this descriptor
	PB2Export void SetUserDlgProc(MapID map_id, ParamMap2UserDlgProc* proc=NULL);
	inline void		SetUserDlgProc(ParamMap2UserDlgProc* proc=NULL) { SetUserDlgProc(0, proc); }
	PB2Export ParamMap2UserDlgProc* GetUserDlgProc(MapID id = 0);
	// dynamically access the P_OWNERS_REF refno for given RefTarg parameter 
	PB2Export void SetOwnerRefNo(ParamID id, int refno);
	PB2Export int  GetOwnerRefNo(ParamID id);
	// dynamically access the p_subtexno/p_submtlno number for given map/mtl parameter 
	PB2Export void SetSubTexNo(ParamID id, int texno);
	PB2Export void SetSubMtlNo(ParamID id, int mtlno);
	PB2Export int  GetSubTexNo(ParamID id);
	PB2Export int  GetSubMtlNo(ParamID id);
	// dynamically access the TYPE_OPEN/SAVEFILEBUTTON p_init_file field 
	PB2Export void SetInitFile(ParamID id, TCHAR* s);
	PB2Export TCHAR* GetInitFile(ParamID id);

	// Interface support
	//! \brief Adds an interface to the descriptor's BaseInterfaceServer interfaces list.
	/*! The interface can later be obtained from the list using GetInterface()
	    \param[in] iface The interface to add */
	PB2Export void AddInterface(BaseInterface* iface);
	//! \brief Removes an interface from the descriptor's BaseInterfaceServer interfaces list.
	/*! \param[in] iface The interface to remove */
	PB2Export void RemoveInterface(BaseInterface* iface);
};

PB2Export IParamBlock2 *CreateParameterBlock2(ParamBlockDesc2 *pdesc, ReferenceMaker* iowner);
class MacroRecorder;
PB2Export void SetPB2MacroRecorderInterface(MacroRecorder* mri);
						

// This updates or creates a new ParamBlock2, based on an existing ParamBlock of
// an earlier version. The new/updated parameter block inherits any parameters from
// the old parameter block whose parameter IDs match.
PB2Export IParamBlock2* UpdateParameterBlock2(
	ParamBlockDescID *pdescOld, int oldCount, IParamBlock *oldPB,
	ParamBlockDesc2* pdescNew,
	IParamBlock2* newPB=NULL);

// Thsi post-load callback handles conversion of pre-ParamBlock2 versions
// of an object to a ParamBlock2 version.
// NOTE: this thing deletes itself when it's done.
class ParamBlock2PLCB : public PostLoadCallback 
{
public:
	ParamVersionDesc* versions;
	int				  count;
	ParamBlockDesc2*  curdesc;
	ReferenceTarget*  targ;
	int				  pbRefNum;
	
	ParamBlock2PLCB(ParamVersionDesc *v, int cnt, ParamBlockDesc2* pd, ReferenceTarget *t, int refNum)
	{ 
		versions = v; count = cnt; curdesc = pd; targ = t; pbRefNum = refNum;
	}

	PB2Export void proc(ILoad *iload);
	int Priority() { return 0; }
	PB2Export INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);
};


// This post-load info about a param block as it was loaded from disk.
// The info can be accessed from an IParamBlock2 or ParamBlockDesc2 using GetInterface
#define IPARAMBLOCK2POSTLOADINFO_ID Interface_ID(0x6c7b290a, 0x7c56423c)

//! \brief Informational class about ParamBlock2 objects loaded from file.
/*! This allows a plugin to detect when an obsolete version of itself has been loaded,
    so the plugin can migrate obsolete parameters to the newer version.
    The information is available only under the following conditions, indicating that an
    obsolete file was loaded:
    1) The current version of the ParamBlock2 uses the P_VERSION tag
    2) The loaded version either did not use P_VERSION, or the version numbers don't match
    3) The information is available only during a PostLoadCallback; it is deleted after load.
    To obtain a pointer to this class, query the plugin's ParamBlockDesc2 as follows:
    postLoadInfo = (IParamBlock2PostLoadInfo*)desc->GetInterface(IPARAMBLOCK2POSTLOADINFO_ID); */
class IParamBlock2PostLoadInfo : public BaseInterface
{
public:
	//! \brief Returns the interface ID of this class, IPARAMBLOCK2POSTLOADINFO_ID
	virtual Interface_ID	GetID() {return IPARAMBLOCK2POSTLOADINFO_ID;}
	//! \brief This method allows for future extention.
	/*! Returns itself if the parameter is IPARAMBLOCK2POSTLOADINFO_ID.
		Otherwise returns BaseInterface::GetInterface(id)
		\param[in] id The ID of the interface */
	virtual BaseInterface*	GetInterface(Interface_ID id)=0;

	//! \brief Returns the current ParamBlockDesc2 which this information is associated with
	virtual ParamBlockDesc2* GetParent()=0;
	//! \brief Returns the version number of the ParamBlock2 loaded from file.
	/*! This is either the P_VERSION number from the obsolete plugin, or if the plugin
	    did not use P_VERSION it is equal to the value 3DSMAX_VERSION when the file was saved */
	virtual DWORD			GetVersion()=0;
	//! \brief Returns the ID numbers of the parameters in the ParamBlock2 loaded from file.
	/*! The numbers are in their original order used by the obsolete plugin. */
	virtual IntTab&			GetParamLoaded()=0;  // Loaded param ID numbers
	//! \brief Returns a mapping from old parameter indices to current parameter indices.
	/*! Each entry is an index into the current ParamBlock2; the number of entries equals
	    the current number of parameters.  The parameters are listed in an order that matches
	    the order from the obsolete plugin.
	    This is used internally during load.  The reference numbering of the current ParamBlock2
	    is temporarily reordered, allowing objects and controllers referenced by the ParamBlock2
	    to load correctly even when the parameter order changes between plugin versions. */
	virtual IntTab&			GetParamReorder()=0; // Current param index numbers; ordered to match loaded params
};

#endif


