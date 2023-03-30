/*==============================================================================

  file:     mrGeomShaderObject.cpp

  author:   Daniel Levesque

  created:  16 June 2003

  description:

    Simple object plugin used for instancing geometry using geometry shaders.
    

  modified:	


© 2004 Autodesk
==============================================================================*/
#include "resource.h"

#include "mrGeomShaderObject.h"
#include "Utility.h"
#include "iparamb2.h"

#include <mentalray\imrShader.h>
#include <mentalray\imrPreferences.h>
#include <mentalray\imrShaderClassDesc.h>
#include <mentalray\shared_src\mrShaderFilter.h>

#define DD (.01f)

namespace {

	mrShaderFilter geomShaderFilter(imrShaderClassDesc::kApplyFlag_Geometry, static_cast<ParamType2>(TYPE_MAX_TYPE), false);

	void MakeQuad(Face *f, int a, int b , int c , int d, int sg) {
		f[0].setVerts( a, b, c);
		f[0].setSmGroup(sg);
		f[0].setEdgeVisFlags(1,1,0);
		f[1].setVerts( c, d, a);
		f[1].setSmGroup(sg);
		f[1].setEdgeVisFlags(1,1,0);
	}

}

//==============================================================================
// class mrGeomShaderObject_ClassDesc
//==============================================================================

class mrGeomShaderObject_ClassDesc : public ClassDesc2 {

public:

	mrGeomShaderObject_ClassDesc();

	int IsPublic();
	void* Create(BOOL loading);
	const TCHAR* ClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();
	const TCHAR* InternalName();
	HINSTANCE HInstance();    

private:
    
    // Prefernces change callback
    static void PreferencesChangeCallback(void* param);

    bool m_preferenceCallbackRegistered;
};

mrGeomShaderObject_ClassDesc::mrGeomShaderObject_ClassDesc()
: m_preferenceCallbackRegistered(false)
{

}

void mrGeomShaderObject_ClassDesc::PreferencesChangeCallback(void* param) {

    DbgAssert(param != NULL);
    mrGeomShaderObject_ClassDesc* classDesc = static_cast<mrGeomShaderObject_ClassDesc*>(param);
    
    ClassEntry* classEntry = GetCOREInterface()->GetDllDir().ClassDir().FindClassEntry(classDesc->SuperClassID(), classDesc->ClassID());
    DbgAssert(classEntry != NULL);
    if(classEntry != NULL) {
		// Delete the class and add it back. This updates the IsPublic state and
		// also updates the class list in the create panel
		GetCOREInterface()->DeleteClass(classDesc);
		GetCOREInterface()->AddClass(classDesc);

        // Update the 'isPublic' state of the class entry. Do this by completely
        // updating the class entry
        //classEntry->Set(classDesc, classEntry->DllNumber(), classEntry->ClassNumber(), classEntry->IsLoaded());
    }
}

int mrGeomShaderObject_ClassDesc::IsPublic() {

    imrPreferences* preferences = GetMRPreferences();
    DbgAssert(preferences != NULL);
    if(preferences != NULL) {

        // Register the preference change callback once
        if(!m_preferenceCallbackRegistered) {
            preferences->RegisterChangeCallback(PreferencesChangeCallback, this);
            m_preferenceCallbackRegistered = true;
        }
        
        // Public when the 'enable mental ray extensions' preference is ON
        return preferences->GetMRExtensionsActive();
    }
    else {
        return TRUE;
    }
}

void* mrGeomShaderObject_ClassDesc::Create(BOOL loading) {

    return new mrGeomShaderObject(loading != 0);
}

const TCHAR* mrGeomShaderObject_ClassDesc::ClassName() {

    return GetString(IDS_MRGEOMETRYOBJECT_CLASSNAME);
}

SClass_ID mrGeomShaderObject_ClassDesc::SuperClassID() {

    return GEOMOBJECT_CLASS_ID;
}

Class_ID mrGeomShaderObject_ClassDesc::ClassID() {

    return MRGEOMETRYOBJECT_CLASS_ID;
}

const TCHAR* mrGeomShaderObject_ClassDesc::Category() {

    return _T("mental ray");
}

const TCHAR* mrGeomShaderObject_ClassDesc::InternalName() {

    return _T("mental ray Geometry Shader");
}

HINSTANCE mrGeomShaderObject_ClassDesc::HInstance() {

    extern HINSTANCE hInstance;
    return hInstance;
}
 
//==============================================================================
// class mrGeomShaderObject_CreateCallback
//
// The mouse creation callback for the mental ray geometry object.
//==============================================================================

class mrGeomShaderObject_CreateCallback : public CreateMouseCallBack {

public:

    static mrGeomShaderObject_CreateCallback& GetInstance() { return m_theInstance; };

    void SetObject(mrGeomShaderObject* object) { m_object = object; }

    // -- from CreateMouseCallBack
    int proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);

private:

    static mrGeomShaderObject_CreateCallback m_theInstance;

    mrGeomShaderObject_CreateCallback();

    // The current object
    mrGeomShaderObject* m_object;

    // The initial point & snap point
    Point3 m_point;
    IPoint2 m_snapPoint;  
};

mrGeomShaderObject_CreateCallback mrGeomShaderObject_CreateCallback::m_theInstance;

mrGeomShaderObject_CreateCallback::mrGeomShaderObject_CreateCallback() 
: m_object(NULL)
{

}

int mrGeomShaderObject_CreateCallback::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat) {

    DbgAssert(m_object != NULL);

    // Stole this code from DummyObjectCreateCallBack::proc(), see src\core\dummy.cpp

    switch(msg) {
    case MOUSE_FREEMOVE:
#ifdef _OSNAP
        vpt->SnapPreview(m, m, NULL, SNAP_IN_3D);
#endif
        break;

    case MOUSE_POINT:
    case MOUSE_MOVE:
        switch(point) {
        case 0:
            m_object->SetBox(Box3(Point3(-DD, -DD, -DD), Point3(DD, DD, DD)));
            m_object->SetCreating(true);
#ifdef _3D_CREATE
            m_point = vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
#else
            m_point = vpt->SnapPoint(m, m, NULL, SNAP_IN_PLANE);
#endif
            mat.SetTrans(m_point);
            m_snapPoint = m;
            m_object->InvalidateMesh();
            break;

        case 1:
            {
                float l;
#ifdef _OSNAP
                l = fabsf(vpt->SnapLength(vpt->GetCPDisp(m_point, Point3(0,0,1), m_snapPoint, m, TRUE)));
#else
                l = fabsf(vpt->SnapLength(vpt->GetCPDisp(m_point, Point3(0,0,1), m_snapPoint, m)));
#endif
                m_object->SetBox(Box3(Point3(-l, -l, -l), Point3(l, l, l)));
                m_object->InvalidateMesh();
                if(msg == MOUSE_POINT) {
                    m_object->SetCreating(false);
                    if(Length(m - m_snapPoint) < 4)
                        return CREATE_ABORT;
                    else
                        return CREATE_STOP;
                }
            }
            break;
        }
        break;

    case MOUSE_ABORT:
        m_object->SetCreating(false);
        return CREATE_ABORT;
    }

    return CREATE_CONTINUE;
}


//==============================================================================
// class mrGeomShaderObject
//==============================================================================

// The param block descriptor
ParamBlockDesc2 mrGeomShaderObject::m_mainPBDesc(
    kMainPBID, _T("Parameters"), IDS_PARAMETERS, &mrGeomShaderObject::GetClassDesc(), P_AUTO_CONSTRUCT | P_AUTO_UI, kRef_MainPB,

    // Rollup
	IDD_MRGEOMETRYOBJECT, IDS_PARAMETERS, 0, 0, &UserDlgProc::GetInstance(),
    
    // Parameters
    kMainPBParam_Shader, _T("Shader"), TYPE_TEXMAP, P_SUBANIM, IDS_SHADER,
        //p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_SHADER,
        p_validator, static_cast<PBValidator*>(&geomShaderFilter),
        end,

    kMainPBParam_Wireframe, _T("Wireframe"), TYPE_BOOL, 0, IDS_WIREFRAME,
        p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_WIREFRAME,
        p_default, TRUE,
        end,

    end      
);


ClassDesc2& mrGeomShaderObject::GetClassDesc() {

    static mrGeomShaderObject_ClassDesc classDesc;
    return classDesc;
}

mrGeomShaderObject::mrGeomShaderObject(bool loading) 
: m_mainPB(NULL),
  m_creating(false)
{

    if(!loading)
        GetClassDesc().MakeAutoParamBlocks(this);
}

mrGeomShaderObject::~mrGeomShaderObject() {

    DeleteAllRefs();
}

void mrGeomShaderObject::DeleteThis() {

    delete this;
}

void mrGeomShaderObject::UpdateMesh(TimeValue t) {

    if(!m_meshValid.InInterval(t)) {
        BuildMesh(t);
    }
}

bool mrGeomShaderObject::DisplayAsWireframe() {

    if(m_mainPB != NULL) {
        return (m_mainPB->GetInt(kMainPBParam_Wireframe) != 0);
    }

    return false;
}

void mrGeomShaderObject::SetGeomShader(Texmap* shader) {

	DbgAssert(m_mainPB != NULL);
    if(m_mainPB != NULL) {
		m_mainPB->SetValue(kMainPBParam_Shader, 0, shader);
    }
}

Texmap* mrGeomShaderObject::GetGeomShaderAsTexmap() {

    if(m_mainPB != NULL) {
        Texmap* texmap = m_mainPB->GetTexmap(kMainPBParam_Shader);
		return texmap;
    }
	else {
		DbgAssert(false);
		return NULL;
	}
}

void mrGeomShaderObject::BuildMesh(TimeValue t) {

    // Stole this from DummyObject::BuildMesh()
	int nverts = 8;
	int nfaces = 12;
	Point3 va = m_box.pmin;
	Point3 vb = m_box.pmax;
	if(m_box.IsEmpty()) {
    	va = Point3(-DD, -DD, -DD);
		vb = Point3( DD,  DD,  DD);
    }

	m_mesh.setNumVerts(nverts);
	m_mesh.setNumFaces(nfaces);

	m_mesh.setVert(0, Point3( va.x, va.y, va.z));
	m_mesh.setVert(1, Point3( vb.x, va.y, va.z));
	m_mesh.setVert(2, Point3( va.x, vb.y, va.z));
	m_mesh.setVert(3, Point3( vb.x, vb.y, va.z));
	m_mesh.setVert(4, Point3( va.x, va.y, vb.z));
	m_mesh.setVert(5, Point3( vb.x, va.y, vb.z));
	m_mesh.setVert(6, Point3( va.x, vb.y, vb.z));
	m_mesh.setVert(7, Point3( vb.x, vb.y, vb.z));

	MakeQuad(&(m_mesh.faces[ 0]), 0,2,3,1,  1);
	MakeQuad(&(m_mesh.faces[ 2]), 2,0,4,6,  2);
	MakeQuad(&(m_mesh.faces[ 4]), 3,2,6,7,  4);
	MakeQuad(&(m_mesh.faces[ 6]), 1,3,7,5,  8);
	MakeQuad(&(m_mesh.faces[ 8]), 0,1,5,4, 16);
	MakeQuad(&(m_mesh.faces[10]), 4,5,7,6, 32);
	m_mesh.InvalidateGeomCache();
	m_mesh.EnableEdgeList(1);

    m_meshValid.SetInfinite();
}

int mrGeomShaderObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) {

    GraphicsWindow *gw = vpt->getGW();
    DWORD rlim = gw->getRndLimits();

    Matrix3 mat = inode->GetObjectTM(t);
    gw->setTransform(mat);

    UpdateMesh(t);

    if(DisplayAsWireframe()) {
        gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);

        if (inode->Selected())  {
		    gw->setColor( LINE_COLOR, GetSelColor());
        }
        else if(!inode->IsFrozen() && !inode->Dependent()) {
            Color color(inode->GetWireColor());
		    gw->setColor( LINE_COLOR, color.r, color.g, color.b);
        }
    }
    
    m_mesh.render(gw, inode->Mtls(), NULL, COMP_ALL, inode->NumMtls());

    gw->setRndLimits(rlim);

    return 0;
}

CreateMouseCallBack* mrGeomShaderObject::GetCreateMouseCallBack() {

    mrGeomShaderObject_CreateCallback& callback = mrGeomShaderObject_CreateCallback::GetInstance();
    callback.SetObject(this);
    return &callback;
}

void mrGeomShaderObject::RescaleWorldUnits(float f) {

	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);   

    m_box.pmin *= f;
    m_box.pmax *= f;

    NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
}

IOResult mrGeomShaderObject::Save(ISave *isave) {

    ULONG nBytes;
    IOResult res;

    isave->BeginChunk(kChunk_Box);
    res = isave->Write(&m_box, sizeof(m_box), &nBytes);
    isave->EndChunk();

    return res;
}

IOResult mrGeomShaderObject::Load(ILoad *iload) {

    ULONG nBytes;
    IOResult res = IO_OK;

    while(iload->OpenChunk() == IO_OK) {
        switch(iload->CurChunkID()) {
        case kChunk_Box:
            res = iload->Read(&m_box, sizeof(m_box), &nBytes);
            break;
        }

        iload->CloseChunk();
        if(res != IO_OK)
            return res;
    }

    return IO_OK;
}

ReferenceTarget* mrGeomShaderObject::GetGeomShader() {

    return GetGeomShaderAsTexmap();
}

void mrGeomShaderObject::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) {

    ClassDesc2& classDesc = GetClassDesc();

    classDesc.BeginEditParams(ip, this, flags, prev);
}

void mrGeomShaderObject::EndEditParams(IObjParam *ip, ULONG flags,Animatable *nex) {

    ClassDesc2& classDesc = GetClassDesc();

    classDesc.EndEditParams(ip, this, flags, nex);
}

int mrGeomShaderObject::IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) {

    UpdateMesh(t);
    return m_mesh.IntersectRay(ray, at, norm);
}

void mrGeomShaderObject::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) {

	Matrix3 mat = inode->GetObjectTM(t);
	UpdateMesh(t);
	box = m_mesh.getBoundingBox();
	box = box * mat;
}

void mrGeomShaderObject::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box ) {

    UpdateMesh(t);
    box = m_mesh.getBoundingBox();
}

void mrGeomShaderObject::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel ) {

    UpdateMesh(t);
    box = m_mesh.getBoundingBox(tm);
}

Mesh* mrGeomShaderObject::GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) {

    needDelete = FALSE;

    UpdateMesh(t);
    return &m_mesh;
}

Interval mrGeomShaderObject::ObjectValidity(TimeValue t) {

    return FOREVER;
}

int mrGeomShaderObject::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {

    GraphicsWindow* gw = vpt->getGW();
    DWORD rlim = gw->getRndLimits();

    if(DisplayAsWireframe()) {
        gw->setRndLimits(GW_WIREFRAME|GW_EDGES_ONLY|GW_BACKCULL);
    }

    HitRegion hitRegion;
    MakeHitRegion(hitRegion, type, crossing, 4, p);
    Matrix3 tm = inode->GetObjectTM(t);

    gw->setTransform(tm);
    UpdateMesh(t);

    int res = m_mesh.select(gw, inode->Mtls(), &hitRegion, (flags & HIT_ABORTONHIT), inode->NumMtls());

    gw->setRndLimits(rlim);

    return res;
}

void mrGeomShaderObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {

    // Don't snap while creating
    if(m_creating)
        return;

	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);

	UpdateMesh(t);
	m_mesh.snap( gw, snap, p, tm );    
}

TCHAR* mrGeomShaderObject::GetObjectName() {

    return GetString(IDS_MRGEOMETRY_OBJECTNAME);
}

RefResult mrGeomShaderObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message) {

    return REF_SUCCEED;
}

int mrGeomShaderObject::NumRefs() {

    return kNumRefs;
}

RefTargetHandle mrGeomShaderObject::GetReference(int i) {

    switch(i) {
    case kRef_MainPB:
        return m_mainPB;
    default:
        return NULL;
    }
}

void mrGeomShaderObject::SetReference(int i, RefTargetHandle rtarg) {

    switch(i) {
    case kRef_MainPB:
        DbgAssert((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID));
        m_mainPB = static_cast<IParamBlock2*>(rtarg);
        break;
    }
}

RefTargetHandle mrGeomShaderObject::Clone(RemapDir &remap) {

    mrGeomShaderObject* newObject = new mrGeomShaderObject(true);

    int count = NumRefs();
    for(int i = 0; i < count; ++i) {
        ReferenceTarget* refTarg = GetReference(i);
        newObject->ReplaceReference(i, remap.CloneRef(refTarg));
    }

    newObject->m_box = m_box;
    newObject->m_mesh = m_mesh;
    newObject->m_meshValid = m_meshValid;
    newObject->m_creating = m_creating;

	BaseClone(this, newObject, remap);
    return newObject;
}

int mrGeomShaderObject::NumParamBlocks() {

    return 1;
}

IParamBlock2* mrGeomShaderObject::GetParamBlock(int i) {

    return m_mainPB;
}

IParamBlock2* mrGeomShaderObject::GetParamBlockByID(short id) {

    return m_mainPB;
}

void mrGeomShaderObject::FreeCaches() {

    m_meshValid.SetEmpty();
    m_mesh.FreeAll();
}

void mrGeomShaderObject::GetClassName(TSTR& s) {

    s = GetString(IDS_MRGEOMETRYOBJECT_CLASSNAME);
}

SClass_ID mrGeomShaderObject::SuperClassID() {

    return GEOMOBJECT_CLASS_ID;
}

Class_ID mrGeomShaderObject::ClassID() {

    return MRGEOMETRYOBJECT_CLASS_ID;
}

ObjectState mrGeomShaderObject::Eval(TimeValue t) {

    UpdateMesh(t);
    return ObjectState(this);
}

BaseInterface* mrGeomShaderObject::GetInterface(Interface_ID id) {

	if(id == IMRGEOMSHADERTRANSLATION_INTERFACE_ID) {
		return static_cast<imrGeomShaderTranslation*>(this);
	}
	else {
		return GeomObject::GetInterface(id);
	}
}

//==============================================================================
// class mrGeomShaderObject::UserDlgProc
//==============================================================================

mrGeomShaderObject::UserDlgProc::UserDlgProc() 
: m_theObject(NULL)
{
	m_geomShaderButtonHandler.SetUserDlgProc(*this);
}

mrGeomShaderObject::UserDlgProc::~UserDlgProc() {

}

mrGeomShaderObject::UserDlgProc& mrGeomShaderObject::UserDlgProc::GetInstance() {

	static UserDlgProc theInstance;
	return theInstance;
}

void mrGeomShaderObject::UserDlgProc::SetGeomShader(Texmap* shader) {

	if(m_theObject != NULL) {
		m_theObject->SetGeomShader(shader);
	}
	else {
		DbgAssert(false);
	}
}

Texmap* mrGeomShaderObject::UserDlgProc::GetGeomShader() {

	if(m_theObject != NULL) {
		return m_theObject->GetGeomShaderAsTexmap();
	}
	else {
		DbgAssert(false);
		return NULL;
	}
}

void mrGeomShaderObject::GetScale(Point3& scale) {

	// Scale by the size of the bounding box
	scale = Point3((m_box.pmax.x - m_box.pmin.x), (m_box.pmax.y - m_box.pmin.y), (m_box.pmax.z - m_box.pmin.z));
}

BOOL mrGeomShaderObject::UserDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	SetThingToOwner(map);
	m_geomShaderButtonHandler.OnInitDialog(hWnd);

	switch(msg) {
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BUTTON_SHADER:
			if(HIWORD(wParam) == BN_CLICKED) {
				m_geomShaderButtonHandler.OnCommand();
			}
			else {
				return FALSE;
			}
			break;
		default:
			return FALSE;
		}
		break;

	case WM_CLOSE:
		m_geomShaderButtonHandler.OnClose();
		SetThing(NULL);
		break;

	default:
		return FALSE;
	}

	return TRUE;

}

void mrGeomShaderObject::UserDlgProc::DeleteThis() {

	// Do nothing - a single static instance ever exists
}

void mrGeomShaderObject::UserDlgProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap) {

	SetThingToOwner(pmap);

	HWND hWnd = pmap->GetHWnd();
	if(hWnd != NULL) {
		m_geomShaderButtonHandler.OnInitDialog(hWnd);
		m_geomShaderButtonHandler.Update();
	}
}

void mrGeomShaderObject::UserDlgProc::SetThing(ReferenceTarget *m) {

	ReferenceMaker* owner = m;
	if((owner == NULL) || ((owner->SuperClassID() == GEOMOBJECT_CLASS_ID) && (owner->ClassID() == MRGEOMETRYOBJECT_CLASS_ID))) {
		m_theObject = static_cast<mrGeomShaderObject*>(owner);
	}
	else {
		DbgAssert(false);
		m_theObject = NULL;
	}
}

void mrGeomShaderObject::UserDlgProc::SetThingToOwner(IParamMap2 *map) {

	if(map != NULL) {
		ReferenceMaker* owner = map->GetParamBlock()->GetOwner();
		if(owner->IsRefTarget()) {
			SetThing(static_cast<ReferenceTarget*>(owner));
		}
		else {
			DbgAssert(false);
			SetThing(NULL);
		}
	}
	else {
		SetThing(NULL);
	}
}

//==============================================================================
// class mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler
//==============================================================================

mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::GeomShaderButtonHandler() 
: mrShaderButtonHandler(imrShaderClassDesc::kApplyFlag_Geometry, IDC_BUTTON_SHADER),
  m_userDlgProc(NULL)
{

}

mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::~GeomShaderButtonHandler() {

}

void mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::SetUserDlgProc(UserDlgProc& userDlgProc) {

	m_userDlgProc = &userDlgProc;
}

void mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::SetShader(Texmap* shader) {

	if(m_userDlgProc != NULL) {
		m_userDlgProc->SetGeomShader(shader);
	}
	else {
		DbgAssert(false);
	}
}

Texmap* mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::GetShader() {

	if(m_userDlgProc != NULL) {
		return m_userDlgProc->GetGeomShader();
	}
	else {
		DbgAssert(false);
		return NULL;
	}
}

const TCHAR* mrGeomShaderObject::UserDlgProc::GeomShaderButtonHandler::GetNoneString() {

	return GetString(IDS_NONE);
}



