/*==============================================================================

  file:     mrTwoSidedShader.h

  author:   Daniel Levesque

  created:  04 November 2003

  description:

      Implementation of a sample texture map proxy for the two-sided mental ray
	  shader. This sample is meant to show how one can use the mental ray SDK
	  to supply a good-looking UI for a mental ray shader.

	  3rd party developers may use this sample as the base building block for their
	  mental ray shaders.

  modified:	


© 2003 Autodesk
==============================================================================*/

#ifndef _MRTWOSIDEDSHADER_H_
#define _MRTWOSIDEDSHADER_H_

#include <max.h>
#include <imtl.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <IMtlRender_Compatibility.h>

#include <mentalray\imrShaderTranslation.h>

#define MRTWOSIDEDSHADER_CLASS_ID	Class_ID(0x79d22c8f, 0x682730a7)	// The class ID for the two-sided shader

//==============================================================================
// class mrTwoSidedShader
//
// Implementation of a Texmap plugin which really acts as a UI proxy for the
// mental ray two-sided shader. This plugin does two things:
// 1. It exposes a UI to control the parameters of the shader.
// 2. It handles custom translation of the parameters to the mental ray shader.
//==============================================================================
class mrTwoSidedShader : 
	public Texmap,					// This is the base class for all MAX texture map plugins
	public imrShaderTranslation		// This is the "custom translation interface" for mental ray shaders
{
public:

	// -- Class Definitions --

	// Parameter block IDs (we only have 1 parameter block here)
	enum ParamBlockID {
		kMainPBID = 0
	};

	// Main PB param IDs (IDs of parameters that are part of the "main" parameter block)
	enum MainPBParamID {
		kMainPID_FrontColor = 0,
		kMainPID_BackColor = 1,
		kMainPID_FrontMapOn = 2,
		kMainPID_BackMapOn = 3,
		kMainPID_FrontMap = 4,
		kMainPID_BackMap = 5
	};

	// Reference numbers (references that are handles through the SetReference()/GetReference() system)
	enum RefNum {
		kRef_MainPB = 0,		// Main parameter block

		kRef_Count
	};

	class mrShaderTwoSidedShader_ClassDesc;

	// -- Methods --

	// Get the class descriptor for this class (uses the singleton pattern)
	static mrShaderTwoSidedShader_ClassDesc& GetClassDesc();

	mrTwoSidedShader(bool loading);
	virtual ~mrTwoSidedShader();

	// -- from Texmap
	virtual	AColor EvalColor(ShadeContext& sc);
	virtual	Point3 EvalNormalPerturb(ShadeContext& sc);

	// -- from imrShaderTranslation
	virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation);
	virtual void ReleaseMRShader();
	virtual bool NeedsAutomaticParamBlock2Translation();
	virtual void TranslateParameters(imrShader* shader, TimeValue t, Interval& valid);
	virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets);

	// -- from MtlBase
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	virtual void Update(TimeValue t, Interval& valid);
	virtual void Reset();
	virtual Interval Validity(TimeValue t);

	// -- from ISubMap
	virtual int NumSubTexmaps();
	virtual Texmap* GetSubTexmap(int i);
	virtual void SetSubTexmap(int i, Texmap *m);
	virtual int SubTexmapOn(int i);
	virtual TSTR GetSubTexmapSlotName(int i);

	// -- from ReferenceTarget
	virtual RefTargetHandle Clone(RemapDir &remap = NoRemap());

	// -- from ReferenceMaker
 	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);
	virtual RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
	virtual IOResult Save(ISave *isave);
	virtual IOResult Load(ILoad *iload);
	
	// -- from Animatable
	virtual Class_ID ClassID();
	virtual SClass_ID SuperClassID();
	virtual void GetClassName(TSTR& s);
	virtual void DeleteThis();
	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);
	virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev=NULL);
	virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next=NULL);
	virtual int	NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(BlockID id);

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);

private:

	// -- Data Members --

	// Main parameter block
	IParamBlock2* m_mainPB;
};

//==============================================================================
// class mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc
//
// Class Descriptor for the two-sided shader
//==============================================================================
class mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc : 
	public ClassDesc2,							// This is the base class for class descriptors
	public imrShaderTranslation_ClassInfo,		// This is to provide 3ds max with information about this mental ray shader
	public IMtlRender_Compatibility_MtlBase		// This interface is used to indicate which renders are compatible
{

public:

	mrShaderTwoSidedShader_ClassDesc();
	virtual ~mrShaderTwoSidedShader_ClassDesc();

	// -- from ClassDesc
	virtual int IsPublic();
	virtual void* Create(BOOL loading);
	virtual const TCHAR* ClassName();
	virtual SClass_ID SuperClassID();
	virtual Class_ID ClassID();
	virtual const TCHAR* Category();
	virtual const TCHAR* InternalName();
	virtual HINSTANCE HInstance();

	// -- from imrShaderTranslation_ClassInfo
	virtual unsigned int GetApplyTypes();

	// -- from IMtlRender_Compatibility_MtlBase
	virtual bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc);

private:

	// The parameter block descriptor for the "main" parameter block
	ParamBlockDesc2 m_mainPBDesc;
};


#endif



