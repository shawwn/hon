/**********************************************************************
 *<
	FILE: Widget.cpp

	DESCRIPTION:	This plugin shows how to:
					
					Create a simple procedural object for 3ds Max.

					This Visual Studio project accompanies an article
					in the 3ds Max SDK Help file that walks a developer, 
					who is new to the 3ds Max SDK, through the process of
					writing a basic procedural object.

	OVERVIEW:		The widget object is a simple procedural object that can be
					created in the 3ds Max viewport. It is located in the create
					command panel under the category "Max SDK tutorials". 

					A widget looks like a collapsed oblique box with two attached
					'boxes' on each side.
					
					A widget has three parameters:

					Size: A float value that governs it's overall size.

					Left: A float value that determines the extension of the box
						  on the left side of the widget

				    Right: A float value that determines the extension of the box
						  on the right side of the widget.

				    This project was created using the Visual Studio Application
					wizard for 3ds Max. Hence many TODO comments are still in the
					body of the plugin.

					The most important functions that were implemented were:

					int WidgetCreateCallBack::proc(... ...)
					void Widget::BuildMesh(TimeValue t)

					The parameter block implemented was:

					static ParamBlockDesc2 widget_param_blk(... ...)
					
	CREATED BY:		Chris Johnson
		
	HISTORY:		Code started in April 2005

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "Widget.h"

#define Widget_CLASS_ID	Class_ID(0x986df9b4, 0x792ce6d7)

#define PBLOCK_REF	0

class Widget : public SimpleObject2 {
	public:
		// Parameter block handled by parent

		//Class vars
		static IObjParam *ip;			//Access to the interface
		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack();
		
		// From Object
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		//TODO: Evaluate the object and return the ObjectState
		ObjectState Eval(TimeValue t) { return ObjectState(this); };		
		//TODO: Return the validity interval of the object as a whole
		Interval ObjectValidity(TimeValue t) { return FOREVER; }

		// From Animatable
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// From SimpleObject
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return Widget_CLASS_ID;}		
		SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );

		//This returns a name that will be used to automaticly name the object
		//when it gets created
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock2; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		
		//Constructor/Destructor
		Widget();
		~Widget();
};



class WidgetClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new Widget(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	Class_ID		ClassID() { return Widget_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("Widget"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	

};

static WidgetClassDesc WidgetDesc;
ClassDesc2* GetWidgetDesc() { return &WidgetDesc; }


//TODO: Add enumerations for each parameter block
enum { widget_params };


//TODO: Add enumerations for various parameters
enum { 
	widget_size,
	widget_left,
	widget_right
};


static ParamBlockDesc2 widget_param_blk ( 
	//Required arguments ----------------------------
	widget_params, _T("params"),  0, &WidgetDesc,
	//flags
	P_AUTO_CONSTRUCT + P_AUTO_UI,

	//Dependent arguments ---------------------------
	//required because P_AUTO_CONSTRUCT was flagged
	//This declares the number of rollouts
	PBLOCK_REF,
	//required because P_AUTO_UI was flagged. 
	//This is the Rollout description
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	//Parameter Specifications ----------------------
	// For each control create a parameter:
	widget_size, 		_T("size"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN, 
		//Zero or more optional tags
		p_default, 		1.0f, 
		p_range, 		0.0f,1000.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_FLOAT, IDC_SIZE_EDIT,	IDC_SIZE_SPIN, 0.50f, 
		end,
	widget_left,		_T("Left"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN, 
		//Zero or more optional tags	
		p_default, 		0.0f, 
		p_range, 		0.0f,100.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_LEFT_EDIT,	IDC_LEFT_SPIN, 0.50f, 
		end,
	widget_right,		_T("Right"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_SPIN, 
		//Zero or more optional tags	
		p_default, 		0.0f, 
		p_range, 		0.0f,100.0f, 
		p_ui, 			TYPE_SPINNER,		EDITTYPE_UNIVERSE, IDC_RIGHT_EDIT,	IDC_RIGHT_SPIN, 0.50f, 
		end,
	end
	);


IObjParam *Widget::ip			= NULL;

//--- Widget -------------------------------------------------------

Widget::Widget()
{
	WidgetDesc.MakeAutoParamBlocks(this);

}

Widget::~Widget()
{
}

IOResult 
Widget::Load(ILoad *iload)
{
	//TODO: Add code to allow plugin to load its data
	
	return IO_OK;
}

IOResult 
Widget::Save(ISave *isave)
{
	//TODO: Add code to allow plugin to save its data
	
	return IO_OK;
}

void 
Widget::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	this->ip = ip;

	SimpleObject2::BeginEditParams(ip,flags,prev);
	WidgetDesc.BeginEditParams(ip, this, flags, prev);
}

void 
Widget::EndEditParams( IObjParam *ip, ULONG flags, Animatable *next )
{
	//TODO: Save plugin parameter values into class variables, if they are not hosted in ParamBlocks. 
	
	SimpleObject2::EndEditParams(ip,flags,next);
	WidgetDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}

//From Object
BOOL 
Widget::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

void 
Widget::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin's internal value to sw				
}

//Class for interactive creation of the object using the mouse
class WidgetCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;	//First point in screen coordinates
	Widget *ob;		//Pointer to the object 
	Point3 p0;		//First point in world coordinates
	Point3 p1;		//We added this point. Second point in world coordinates.
public:	
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(Widget *obj) {ob = obj;}
};

int 
WidgetCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) 
		{
			case 0: // only happens with MOUSE_POINT msg
				ob->suspendSnap = TRUE;
				sp0 = m;
				p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				mat.SetTrans(p0);

				//Set a default overall size in the parameter block
				ob->pblock2->SetValue(widget_size, ob->ip->GetTime(), 0.0f);
				break;
			case 1:
			{
				ob->suspendSnap = TRUE; 
				p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				
				float speedFactor = 24.0f;
				float theSize = (Length(p1 - p0) / speedFactor);
				
				//Set the overall size in parameter block
				ob->pblock2->SetValue(widget_size, ob->ip->GetTime(), theSize);

				//Invalidate and display the mesh in the viewport
				widget_param_blk.InvalidateUI();
				break;
			}
			case 2:
			{
				return CREATE_STOP;
			}
		}
	} 
	else {
		if (msg == MOUSE_ABORT) return CREATE_ABORT;
	}
	return TRUE;
}

static WidgetCreateCallBack WidgetCreateCB;

//From BaseObject
CreateMouseCallBack* Widget::GetCreateMouseCallBack() 
{
	WidgetCreateCB.SetObj(this);
	return(&WidgetCreateCB);
}

//From SimpleObject
void 
Widget::BuildMesh(TimeValue t)
{
	float size = pblock2->GetFloat(widget_size, t);
	float sizeLeft = pblock2->GetFloat(widget_left, t);
	float sizeRight= pblock2->GetFloat(widget_right, t);
	#include "widgetmesh.h"
	mesh.InvalidateGeomCache();
}
BOOL 
Widget::OKtoDisplay(TimeValue t) 
{
	//TODO: Check whether all the parameters have valid values and return the state
	return TRUE;
}

void 
Widget::InvalidateUI() 
{
	// Hey! Update the param blocks
}

Object* 
Widget::ConvertToType(TimeValue t, Class_ID obtype)
{
	//TODO: If the plugin can convert to a nurbs surface then check to see 
	//		whether obtype == EDITABLE_SURF_CLASS_ID and convert the object
	//		to nurbs surface and return the object
	
	return SimpleObject::ConvertToType(t,obtype);
	return NULL;
}

int 
Widget::CanConvertToType(Class_ID obtype)
{
	//TODO: Check for any additional types the plugin supports
	if (obtype==defObjectClassID ||
		obtype==triObjectClassID) {
		return 1;
	} else {		
	return SimpleObject::CanConvertToType(obtype);
		}
}

// From Object
int 
Widget::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
{
	//TODO: Return TRUE after you implement this method
	return FALSE;
}

void 
Widget::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
    Object::GetCollapseTypes(clist, nlist);
	//TODO: Append any any other collapse type the plugin supports
}

// From ReferenceTarget
RefTargetHandle Widget::Clone(RemapDir& remap) 
{
	Widget* newob = new Widget();	
	// Make a copy all the data and also clone all the references
	newob->ReplaceReference(0,remap.CloneRef(pblock2));
	newob->ivalid.SetEmpty();
	BaseClone(this, newob, remap);
	return(newob);
}
