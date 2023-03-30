/*==============================================================================

  file:     mrTwoSidedShader.cpp

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

#include "mrTwoSidedShader.h"
#include "resource.h"

#include <mentalray\imrShader.h>
#include <mentalray\imrShaderClassDesc.h>
#include <mentalray\mentalrayInMax.h>

extern TCHAR *GetString(int id);

namespace {

	// Utility function which returns the ID of a parameter, within a given parameter block, based
	// on a name. The search is _not_ case sensitive.
	bool GetParamIDByName(ParamID& paramID, const TCHAR* name, IParamBlock2* pBlock, bool errorOnMissing = true) {

		DbgAssert(pBlock != NULL);
		int count = pBlock->NumParams();
		for(int i = 0; i < count; ++i) {
			ParamID id = pBlock->IndextoID(i);
			ParamDef& paramDef = pBlock->GetParamDef(id);
			if(_tcsicmp(name, paramDef.int_name) == 0) {
				paramID = id;
				return true;
			}
		}

		if(errorOnMissing) {
			DbgAssert(false);
		}

		return false;
	}
}

//==============================================================================
// class mrTwoSidedShader
//==============================================================================

mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc& mrTwoSidedShader::GetClassDesc() {

	static mrShaderTwoSidedShader_ClassDesc theClassDesc;
	return theClassDesc;
}

mrTwoSidedShader::mrTwoSidedShader(bool loading) 
: m_mainPB(NULL)
{

	if(!loading) {
		GetClassDesc().MakeAutoParamBlocks(this);
	}
}

mrTwoSidedShader::~mrTwoSidedShader() {

	// Cleanup
	DeleteAllRefs();
}

AColor mrTwoSidedShader::EvalColor(ShadeContext& sc) {

	// Provide a good default for this (for the material editor peview)... 
	// Use the front color for the top half of the screen the the back color 
	// for the bottom half.
	if(m_mainPB != NULL) {
		Point2 screenUV;
		Point2 screenDUV;
		sc.ScreenUV(screenUV, screenDUV);

		// Front map is used for top part of the image
		bool useFront = (screenUV.y > 0.5f);

		TimeValue t = sc.CurTime();
		BOOL mapOn = m_mainPB->GetInt(useFront ? kMainPID_FrontMapOn : kMainPID_BackMapOn, t);
		if(mapOn) {
			Texmap* map = m_mainPB->GetTexmap(useFront ? kMainPID_FrontMap : kMainPID_BackMap, t);
			if(map != NULL) {
				return map->EvalColor(sc);
			}
		}

		// Return the color only
		AColor col = m_mainPB->GetAColor(useFront ? kMainPID_FrontColor : kMainPID_BackColor, t);
		return col;
	}
	
	return AColor(0,0,0);
}

Point3 mrTwoSidedShader::EvalNormalPerturb(ShadeContext& sc) {

	// Unsupported: we don't support the max scanline renderer
	return Point3(0,0,0);
}

imrShader* mrTwoSidedShader::GetMRShader(imrShaderCreation& shaderCreation) {

	// Create an instance of the "two-sided" shader, from the mental ray base library
	imrShader* twoSidedShader = shaderCreation.CreateShader(_T("mib_twosided"), GetFullName());
	return twoSidedShader;
}

void mrTwoSidedShader::ReleaseMRShader() {

	// Nothing to do here
}

bool mrTwoSidedShader::NeedsAutomaticParamBlock2Translation() {

	// No automatic translation - the translation is done by hand.
	return false;
}

void mrTwoSidedShader::TranslateParameters(imrShader* shader, TimeValue t, Interval& valid) {

	// The "map" parameters are translated using the shader connections.
	// The color parameters have already been translated by the automatic system
	if((shader != NULL) && (m_mainPB != NULL)) {

		BOOL frontMapOn;
		BOOL backMapOn;
		Texmap* frontMap;
		Texmap* backMap;
		AColor frontColor;
		AColor backColor;

		// Get the map values
		m_mainPB->GetValue(kMainPID_FrontColor, t, frontColor, valid);
		m_mainPB->GetValue(kMainPID_BackColor, t, backColor, valid);
		m_mainPB->GetValue(kMainPID_FrontMapOn, t, frontMapOn, valid);
		m_mainPB->GetValue(kMainPID_BackMapOn, t, backMapOn, valid);
		m_mainPB->GetValue(kMainPID_FrontMap, t, frontMap, valid);
		m_mainPB->GetValue(kMainPID_BackMap, t, backMap, valid);

		// Get the parameter blocks used to store the parameters
		IParamBlock2* parametersPB = shader->GetParametersParamBlock();
		IParamBlock2* connectionsPB = shader->GetConnectionsParamBlock();

		if((parametersPB != NULL) && (connectionsPB != NULL)) {
			ParamID paramID;

			// Translate the colors
			if(GetParamIDByName(paramID, _T("front"), parametersPB)) {
				parametersPB->SetValue(paramID, 0, frontColor);	// Value must be set at time 0, this is a restriction of the system.
			}
			if(GetParamIDByName(paramID, _T("back"), parametersPB)) {
				parametersPB->SetValue(paramID, 0, backColor);	// Value must be set at time 0, this is a restriction of the system.
			}

			// Set the shader connections. The shaders must be set before the "shader ON" flags
			// since setting a shader automatically turns the "ON" flag ON.
			if(GetParamIDByName(paramID, _T("front.shader"), connectionsPB)) {
				connectionsPB->SetValue(paramID, 0, frontMap);	// Value must be set at time 0, this is a restriction of the system.
			}
			if(GetParamIDByName(paramID, _T("back.shader"), connectionsPB)) {
				connectionsPB->SetValue(paramID, 0, backMap);	// Value must be set at time 0, this is a restriction of the system.
			}
			if(GetParamIDByName(paramID, _T("front.connected"), connectionsPB)) {
				connectionsPB->SetValue(paramID, 0, frontMapOn);	// Value must be set at time 0, this is a restriction of the system.
			}
			if(GetParamIDByName(paramID, _T("back.connected"), connectionsPB)) {
				connectionsPB->SetValue(paramID, 0, backMapOn);	// Value must be set at time 0, this is a restriction of the system.
			}
		}
		else {
			DbgAssert(false);
		}
	}
	else {
		DbgAssert(false);
	}
}

void mrTwoSidedShader::GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) {

	// No additional dependencies
}

ParamDlg* mrTwoSidedShader::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {

	return GetClassDesc().CreateParamDlgs(hwMtlEdit, imp, this);
}

void mrTwoSidedShader::Update(TimeValue t, Interval& valid) {

	// Update the sub textures
	int count = NumSubTexmaps();
	for(int i = 0; i < count; ++i) {
		Texmap* subMap = GetSubTexmap(i);
		if(subMap != NULL)
			subMap->Update(t, valid);
	}
}

void mrTwoSidedShader::Reset() {

	// Reset all PB2's
	GetClassDesc().Reset(this);
}

Interval mrTwoSidedShader::Validity(TimeValue t) {

	// Get the validity of all the parameter blocks and sub-textures
	Interval valid = FOREVER;

	int count = NumParamBlocks();
	int i;
	for(i = 0; i < count; ++i) {
		IParamBlock2* pBlock = GetParamBlock(i);
		if(pBlock != NULL)
			pBlock->GetValidity(t, valid);
	}

	count = NumSubTexmaps();
	for(i = 0; i < count; ++i) {
		Texmap* subMap = GetSubTexmap(i);
		if(subMap != NULL)
			valid &= subMap->Validity(t);
	}

	return valid;
}

int mrTwoSidedShader::NumSubTexmaps() {

	return 2;
}

Texmap* mrTwoSidedShader::GetSubTexmap(int i) {

	if(m_mainPB != NULL) {
		switch(i) {
		case 0:
			return m_mainPB->GetTexmap(kMainPID_FrontMap);
		case 1:
			return m_mainPB->GetTexmap(kMainPID_BackMap);
		default:
			break;
		}
	}

	return NULL;
}

void mrTwoSidedShader::SetSubTexmap(int i, Texmap *m) {

	if(m_mainPB != NULL) {
		switch(i) {
		case 0:
			m_mainPB->SetValue(kMainPID_FrontMap, 0, m);
			break;
		case 1:
			m_mainPB->SetValue(kMainPID_BackMap, 0, m);
			break;
		default:
			break;
		}
	}
}

int mrTwoSidedShader::SubTexmapOn(int i) {

	if(m_mainPB != NULL) {
		switch(i) {
		case 0:
			return m_mainPB->GetInt(kMainPID_FrontMapOn);
		case 1:
			return m_mainPB->GetInt(kMainPID_BackMapOn);
		default:
			break;
		}
	}

	return FALSE;
}

TSTR mrTwoSidedShader::GetSubTexmapSlotName(int i) {

	switch(i) {
	case 0:
		return GetString(IDS_FRONT);
	case 1:
		return GetString(IDS_BACK);
	default:
		return _T("");
	}
}

RefTargetHandle mrTwoSidedShader::Clone(RemapDir &remap) {

	mrTwoSidedShader* pNew = new mrTwoSidedShader(true);
    
	int count = NumRefs();
	for(int i = 0; i < count; ++i) {
		ReferenceTarget* refTarg = GetReference(i);
		pNew->ReplaceReference(i, ((refTarg != NULL) ? remap.CloneRef(refTarg) : NULL));
	}

	pNew->MtlBase::operator=(*this);

	BaseClone(this, pNew, remap);

	return pNew;
}

int mrTwoSidedShader::NumRefs() {

	return kRef_Count;
}

RefTargetHandle mrTwoSidedShader::GetReference(int i) {

	switch(i) {
	case kRef_MainPB:
		return m_mainPB;
	default:
		return NULL;
	}
}

void mrTwoSidedShader::SetReference(int i, RefTargetHandle rtarg) {

	switch(i) {
	case kRef_MainPB:
		DbgAssert((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID));
		if((rtarg == NULL) || (rtarg->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID))
			m_mainPB = static_cast<IParamBlock2*>(rtarg);
		break;
	default:
		break;
	}
}

RefResult mrTwoSidedShader::NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message ) {

	// No need to do anything
	return REF_SUCCEED;
}

IOResult mrTwoSidedShader::Save(ISave *isave) {

	// Nothing to save
	return IO_OK;
}

IOResult mrTwoSidedShader::Load(ILoad *iload) {

	// Nothing to load
	return IO_OK;
}

Class_ID mrTwoSidedShader::ClassID() {

	return GetClassDesc().ClassID();
}

SClass_ID mrTwoSidedShader::SuperClassID() {

	return GetClassDesc().SuperClassID();
}

void mrTwoSidedShader::GetClassName(TSTR& s) {

	s = GetClassDesc().ClassName();
}

void mrTwoSidedShader::DeleteThis() {

	delete this;
}

int mrTwoSidedShader::NumSubs() {

	return 1;
}

Animatable* mrTwoSidedShader::SubAnim(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	default:
		return NULL;
	}
}

TSTR mrTwoSidedShader::SubAnimName(int i) {

	switch(i) {
	case 0:
		return GetString(IDS_PARAMETERS);
	default:
		return _T("");
	}
}

void mrTwoSidedShader::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {

	GetClassDesc().BeginEditParams(ip, this, flags, prev);
}

void mrTwoSidedShader::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {

	GetClassDesc().EndEditParams(ip, this, flags, next);
}

int	mrTwoSidedShader::NumParamBlocks() {

	return 1;
}

IParamBlock2* mrTwoSidedShader::GetParamBlock(int i) {

	switch(i) {
	case 0:
		return m_mainPB;
	default:
		return NULL;
	}
}

IParamBlock2* mrTwoSidedShader::GetParamBlockByID(BlockID id) {

	switch(id) {
	case kMainPBID:
		return m_mainPB;
	default:
		return NULL;
	}
}

BaseInterface* mrTwoSidedShader::GetInterface(Interface_ID id) {

	// Return the custom translation interface when it is requested
	if(id == IMRSHADERTRANSLATION_INTERFACE_ID) {
		imrShaderTranslation* r = this;
		return r;
	}
	else {
		return Texmap::GetInterface(id);
	}
}

//==============================================================================
// class mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc
//==============================================================================

mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::mrShaderTwoSidedShader_ClassDesc() 
: m_mainPBDesc(
	
	// The internal name of the parameter block must be "Parameters" since we want it to be translated automatically
	kMainPBID, _T("Parameters"), IDS_PARAMETERS, NULL, (P_AUTO_CONSTRUCT | P_AUTO_UI), kRef_MainPB,

	// Rollup
	IDD_TWOSIDEDSHADER, IDS_TWOSIDEDSHADER_PARAMETERS, 0, 0, NULL,

	// Parameters
	// The internal name of this parameter must match that of the .mi declaration, because we want it to be translated automatically
	kMainPID_FrontColor, _T("Front"), TYPE_FRGBA, P_ANIMATABLE, IDS_FRONTCOLOR,	
		p_default, AColor(0.5f, 0.5f, 0.5f),
		p_ui, TYPE_COLORSWATCH, IDC_COLOR_FRONT,
		end,
	// The internal name of this parameter must match that of the .mi declaration, because we want it to be translated automatically
	kMainPID_BackColor, _T("Back"), TYPE_FRGBA, P_ANIMATABLE, IDS_BACKCOLOR,	
		p_default, AColor(0.5f, 0.5f, 0.5f),
		p_ui, TYPE_COLORSWATCH, IDC_COLOR_BACK,
		end,
	kMainPID_FrontMapOn, _T("FrontMapOn"), TYPE_BOOL, P_ANIMATABLE, IDS_FRONTMAPON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_FRONTMAPON,
		end,
	kMainPID_BackMapOn, _T("BackMapOn"), TYPE_BOOL, P_ANIMATABLE, IDS_BACKMAPON,
		p_default, TRUE,
		p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK_BACKMAPON,
		end,
	kMainPID_FrontMap, _T("FrontMap"), TYPE_TEXMAP, 0, IDS_FRONTMAP,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_FRONTMAP,
		end,
	kMainPID_BackMap, _T("BackMap"), TYPE_TEXMAP, 0, IDS_BACKMAP,
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_BACKMAP,
		end,
	end
  )
{
	m_mainPBDesc.SetClassDesc(this);
	imrShaderTranslation_ClassInfo::Init(*this);
	IMtlRender_Compatibility_MtlBase::Init(*this);
}

mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::~mrShaderTwoSidedShader_ClassDesc() {

}

int mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::IsPublic() {

	return TRUE;
}

void* mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::Create(BOOL loading) {

	return new mrTwoSidedShader(loading != 0);
}

const TCHAR* mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::ClassName() {

	return GetString(IDS_TWOSIDEDSHADER_CLASSNAME);
}

SClass_ID mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::SuperClassID() {

	return TEXMAP_CLASS_ID;
}

Class_ID mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::ClassID() {

	return MRTWOSIDEDSHADER_CLASS_ID;
}

const TCHAR* mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::Category() {

	return GetString(IDS_CATEGORY);
}

const TCHAR* mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::InternalName() {

	return _T("mrTwoSidedShader");
}

HINSTANCE mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::HInstance() {

	extern HINSTANCE hInstance;
	return hInstance;
}

unsigned int mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::GetApplyTypes() {

	return imrShaderClassDesc::kApplyFlag_Default;
}

bool mrTwoSidedShader::mrShaderTwoSidedShader_ClassDesc::IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {

	// Compatible only with the mental ray renderer
	return ((rendererClassDesc.ClassID() == MRRENDERER_CLASSID) != 0);
}







