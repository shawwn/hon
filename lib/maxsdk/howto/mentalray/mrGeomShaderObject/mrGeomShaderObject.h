/*==============================================================================

  file:     mrGeomShaderObject.h

  author:   Daniel Levesque

  created:  16 June 2003

  description:

    Simple object plugin used for instancing geometry using geometry shaders.
    

  modified:	


© 2004 Autodesk
==============================================================================*/
#ifndef _MRGEOMSHADEROBJECT_H_
#define _MRGEOMSHADEROBJECT_H_

#include <max.h>
#include <simpobj.h>
#include <iparamm2.h>
#include <mentalray\imrShaderTranslation.h>
#include <mentalray\shared_src\mrShaderButtonHandler.h>

#define MRGEOMETRYOBJECT_CLASS_ID Class_ID(0x711f0aad, 0x1c181b09)

class ClassDesc2;


//==============================================================================
// class mrGeomShaderObject
//==============================================================================
class mrGeomShaderObject : public GeomObject, public imrGeomShaderTranslation {

public:

    static ClassDesc2& GetClassDesc();

    explicit mrGeomShaderObject(bool loading);
    ~mrGeomShaderObject();

    void SetBox(const Box3& box);
    void InvalidateMesh();
    void SetCreating(bool creating);

    void UpdateMesh(TimeValue t);

    bool DisplayAsWireframe();

	void SetGeomShader(Texmap* shader);
	Texmap* GetGeomShaderAsTexmap();

	// -- from imrGeomShaderTranslation
	virtual ReferenceTarget* GetGeomShader();
	virtual void GetScale(Point3& scale);

	// -- from GeomObject
	virtual Mesh* GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete);

    // -- from Object
	virtual int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);		
	virtual ObjectState Eval(TimeValue time);
	//virtual void InitNodeName(TSTR& s) {s = GetObjectName();}
	virtual Interval ObjectValidity(TimeValue t);
	//virtual int CanConvertToType(Class_ID obtype);
	//virtual Object* ConvertToType(TimeValue t, Class_ID obtype);		
    //virtual BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);
    
    // -- from BaseObject
	virtual void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
	virtual void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box );
	virtual void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
    virtual CreateMouseCallBack* GetCreateMouseCallBack();
    virtual int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	virtual void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
    virtual TCHAR *GetObjectName();
	//virtual IParamArray *GetParamBlock();
	//virtual int GetParamBlockIndex(int id);

    // -- from ReferenceMaker
    virtual void RescaleWorldUnits(float f);
    virtual IOResult Save(ISave *isave);
    virtual IOResult Load(ILoad *iload);
    virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);
    virtual int NumRefs();
    virtual RefTargetHandle GetReference(int i);
    virtual void SetReference(int i, RefTargetHandle rtarg);

    // -- from ReferenceTarget
    virtual RefTargetHandle Clone(RemapDir &remap = NoRemap());
        
    // -- from Animatable
    virtual void DeleteThis();
    virtual void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
    virtual void EndEditParams(IObjParam *ip, ULONG flags,Animatable *nex);
    virtual int NumParamBlocks();
    virtual IParamBlock2* GetParamBlock(int i);
    virtual IParamBlock2* GetParamBlockByID(short id);
    virtual void FreeCaches();
    virtual void GetClassName(TSTR& s);
    virtual SClass_ID SuperClassID();
    virtual Class_ID ClassID();

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);
    
private:

	class UserDlgProc;

    enum RefID {
        kRef_MainPB = 0,

        kNumRefs
    };

    // Param block IDs
    enum ParamBlockID {
        kMainPBID = 0,
    };

    // Parameter IDs for main param block
    enum MainPBParamID {
        kMainPBParam_Shader = 0,
        kMainPBParam_Wireframe = 1,
    };

    enum ChunkID {
        kChunk_Box = 0x0011
    };

    void BuildMesh(TimeValue t);

    ////////////////////////////////////////////////////////////////////////////
    // Data members
    ////////////////////////////////////////////////////////////////////////////

    static ParamBlockDesc2 m_mainPBDesc;

    // The bounding box of the object (used to generate the mesh)
    Box3 m_box;

    IParamBlock2* m_mainPB;

    Mesh m_mesh;
    Interval m_meshValid;

    // In the creation process?
    bool m_creating;
};


//==============================================================================
// class mrGeomShaderObject::mrShaderListShader_UserDlgProc
//
// User dialog proc for the geom shader object
//==============================================================================
class mrGeomShaderObject::UserDlgProc : public ParamMap2UserDlgProc {

public:

	virtual ~UserDlgProc();

	static UserDlgProc& GetInstance();

	void SetGeomShader(Texmap* shader);
	Texmap* GetGeomShader();

	virtual void SetThingToOwner(IParamMap2 *map);

	// -- from ParamMap2UserDlgProc
	virtual BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void DeleteThis();
	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap);
	virtual void SetThing(ReferenceTarget *m);

private:


	//==========================================================================
	// class GeomShaderButtonHandler
	//
	// Handles the shader button.
	//==========================================================================
	class GeomShaderButtonHandler : public mrShaderButtonHandler {

	public:

		GeomShaderButtonHandler();
		virtual ~GeomShaderButtonHandler();

		void SetUserDlgProc(UserDlgProc& userDlgProc);

	protected:
		
		// -- from mrShaderButtonHandler
		virtual void SetShader(Texmap* shader);
		virtual Texmap* GetShader();
		virtual const TCHAR* GetNoneString();

	private:

		UserDlgProc* m_userDlgProc;

	};

	// A single private instance is ever created.
	UserDlgProc();

	// -- Data members --

	// The button handler for the shader button
	GeomShaderButtonHandler m_geomShaderButtonHandler;

	// The geomshader object plugin
	mrGeomShaderObject* m_theObject;

};

//==============================================================================
// class mrGeomShaderObject inlined methods
//==============================================================================

inline void mrGeomShaderObject::SetBox(const Box3& box) {

    m_box = box;
}

inline void mrGeomShaderObject::InvalidateMesh() {

    m_meshValid.SetEmpty();
}

inline void mrGeomShaderObject::SetCreating(bool creating) {

    m_creating = creating;
}

#endif