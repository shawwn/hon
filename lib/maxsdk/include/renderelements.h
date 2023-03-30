/////////////////////////////////////////////////////////////////////////
//
//
//	Render Element Plug-Ins
//
//	Created 4/15/2000	Kells Elmquist
//

#ifndef	RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

// includes
#include "sfx.h"

// predeclarations
class IllumParams;


#define BEAUTY_RENDER_ELEMENT_CLASS_ID 0x00000001

// Returned by a RenderElement when it is asked to put up its rollup page.
typedef SFXParamDlg IRenderElementParamDlg;


//////////////////////////////////////////////////////////////
//
//		RenderElement base interface
//
//		class SpecialFX declared in maxsdk/include/render.h
//
class IRenderElement : public SpecialFX
{
public:
	// set/get element's enabled state
	virtual void SetEnabled(BOOL enabled)=0;
	virtual BOOL IsEnabled() const = 0;

	// set/get element's filter enabled state
	virtual void SetFilterEnabled(BOOL filterEnabled)=0;
	virtual BOOL IsFilterEnabled() const =0;

	// is this element to be blended for multipass effects?
	virtual BOOL BlendOnMultipass() const =0;

	// set/get whether to apply atmosphere
//	virtual void SetApplyAtmosphere(BOOL applyAtmosphere) =0;
	virtual BOOL AtmosphereApplied() const =0;

//	virtual void SetApplyShadows(BOOL applyShadows) =0;
	virtual BOOL ShadowsApplied() const =0;

	// set/get element's name (as it will appear in render dialog)
	virtual void SetElementName( TCHAR* newName )=0; 
	virtual const TCHAR* ElementName() const =0;

/*
	// set/get path name for file output
	virtual void SetPathName( TCHAR* newPathName)=0;
	virtual const TCHAR* PathName() const =0;

	virtual void SetBitmap( Bitmap* bitmap)=0;
	virtual Bitmap* GetBitmap() const =0;
*/
	virtual void SetPBBitmap(PBBitmap* &pPBBitmap) const =0;
	virtual void GetPBBitmap(PBBitmap* &pPBBitmap) const =0;

	// this is the element specific optional UI, which is a rollup in the render dialog
	virtual IRenderElementParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }

	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// IRenderElement has secondary dialogs that don't have the IRenderElement as their 'thing'.
	// Called once for each secondary dialog, for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	virtual BOOL SetDlgThing(IRenderElementParamDlg* dlg) { return FALSE; }

	// ---------------------
	// from class RefMaker
	// ---------------------
	// it is critical for merging that this code is called at the start of a plug-in's save and load methods.
	// SpecialFX's base implementation saves/loads SpecialFX::name, which is used to populate the 'Merge Render Elements'
	// dialog box. if a plugin re-implements this function, it should first call IRenderElement::Save(iSave)
	// or IRenderElement::Load(iLoad)
	IOResult Save(ISave *iSave)
	{
		name = ElementName();
		return SpecialFX::Save(iSave);
	}
	IOResult Load(ILoad *iLoad)
	{
		return SpecialFX::Load(iLoad);
	}

	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// ---------------------
	// from class Animatable
	// ---------------------
	SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }

	// renderer will call this method to see if IRenderElement is compatible with it
    using SpecialFX::GetInterface;
	virtual void* GetInterface(ULONG id) =0;
    
	virtual void ReleaseInterface(ULONG id, void *i) =0;
};

//==============================================================================
// class IRenderElementRequirements
//
// This interface is used to query special requirement flags from render elements.
//==============================================================================
//! \brief This interface is used to query special requirement flags from render elements.
#define IRENDERELEMENTREQUIREMENTS_INTERFACE_ID Interface_ID(0x1804343c, 0x614c2fdf)
class IRenderElementRequirements : public BaseInterface {
public:

	//! \brief Types of requirement
	enum Requirement {
		kRequirement_MotionData = 0 //!< Indicates that the render element requires the renderer to produce motion data
	};

	//! \brief This method is used to know if the renderer has the given requirement.
	//! \param[in] requirement - The requirement to be looked for in the renderer.
	//! \return - true if the renderer has the given requirement, false otherwise.
	virtual bool HasRequirement(Requirement requirement) = 0;

	// -- from BaseInterface
	//! \brief Reimplemented from BaseInterface.
	virtual Interface_ID GetID();
};

inline Interface_ID IRenderElementRequirements::GetID() {

	return IRENDERELEMENTREQUIREMENTS_INTERFACE_ID;
}

//! \brief This returns the IRenderElementRequirements interface for a given render element.
/*!
This method checks for element being NULL and return directly NULL if its the case.
\param[in] element - The render element for which the IRenderElementRequirements interface is wanted.
\return - the IRenderElementRequirements interface for this element or NULL if element is NULL.
*/
inline IRenderElementRequirements* Get_RenderElementRequirements(IRenderElement* element) {

	return (element != NULL) ? static_cast<IRenderElementRequirements*>(element->GetInterface(IRENDERELEMENTREQUIREMENTS_INTERFACE_ID)) : NULL;
}

//////////////////////////////////////////////////////////////
//
//		RenderElement base class for max's default scanline renderer.
//      RenderElement plugins that utilize ShadeContext and IllumParams
//      should sub-class from here
//

class MaxRenderElement : public IRenderElement
{
	friend class FilterComp;
protected:
	int mShadeOutputIndex;

public:
	// this interface's ID
	enum { IID = 0xeffeeffe };

	// set/get this element's index into the ShadeOutput's array of element channels
	void SetShadeOutputIndex(int shadeOutputIndex) { mShadeOutputIndex = shadeOutputIndex; }
	int ShadeOutputIndex() const { return mShadeOutputIndex; }

	// will be called on each element before call to PostIllum()
	virtual void Update(TimeValue timeValue) { }

	// compute the element and store in the ShadeContext's ShadeOutput.
	virtual void PostIllum(ShadeContext& sc, IllumParams& ip) =0;

	// called after atmospheres are computed, to allow elements to handle atmospheres
	virtual void PostAtmosphere(ShadeContext& sc, float z, float prevZ) =0;

	// ---------------------
	// from class Animatable
	// ---------------------
	// renderer will call this method to see if IRenderElement is compatible with it
    using IRenderElement::GetInterface;
	virtual void* GetInterface(ULONG id) { return (id == IID) ? this : NULL; }
	virtual void ReleaseInterface(ULONG id, void *i) { }
};

//////////////////////////////////////////////////////////////
//
//		RenderElement base class for texture baking elements using
//		max's default scanline renderer.
//
class MaxBakeElement : public MaxRenderElement
{

public:
	// this interface's ID
	enum { IID = 0xeffeefff };

	// these are the common attributes for texture bake elements
	// set/get element's output size, shd always be square
	virtual void SetOutputSz( int  xSz, int ySz ) = 0;
	virtual void GetOutputSz( int&  xSz, int& ySz ) const = 0;

	virtual void SetFileName( TCHAR* newName) = 0;
	virtual const TCHAR* GetFileName() const = 0;
	virtual void SetFileType( TCHAR* newType) = 0;
	virtual	const TCHAR* GetFileType() const = 0;

	virtual void SetFileNameUnique(BOOL on) = 0; 
	virtual BOOL IsFileNameUnique() const = 0;

	// set/get the renderingbitmap, max of sizes
	// no memory management of the bitmap!
	virtual void SetRenderBitmap( Bitmap* pBitmap ) = 0;
	virtual Bitmap*  GetRenderBitmap() const = 0;

	// set/get whether the output texture is shaded, or represents raw
	// texture colors
	virtual void SetLightApplied(BOOL on) = 0; 
	virtual BOOL IsLightApplied() const = 0;

	// set/get element's shadow applied state
	virtual void SetShadowApplied(BOOL on) = 0;
	virtual BOOL IsShadowApplied() const = 0;


	// set/get element's atmosphere applied state
	virtual void SetAtmosphereApplied(BOOL on) = 0;
	virtual BOOL IsAtmosphereApplied() const = 0;

		// these IRenderElement methods hooked to new functions
	virtual	BOOL AtmosphereApplied() const {
			return IsAtmosphereApplied();
		}
	virtual	BOOL ShadowsApplied() const {
			return IsShadowApplied();
		}

	// these allow us to build dynamic ui for the elements in maxscript
	// currently only checkboxes supported
	// these use & return 0-based indices
	virtual int GetNParams() const = 0;
	virtual	const TCHAR* GetParamName( int nParam ) = 0;
	virtual	const int FindParamByName( TCHAR*  name ) = 0;
	virtual	int  GetParamType( int nParam ) = 0;
	virtual	int  GetParamValue( int nParam ) = 0;
	virtual	void SetParamValue( int nParam, int newVal ) = 0;

	// ---------------------
	// from class Animatable
	// ---------------------
	SClass_ID SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	
	// renderer will call this method to see if IRenderElement is compatible with it
    using MaxRenderElement::GetInterface;
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : MaxRenderElement::GetInterface(id);}
	
	virtual void ReleaseInterface(ULONG id, void *i) { }
};

//////////////////////////////////////////////////////////////
//
//! \brief Extention to MaxBakeElement in 3ds Max 8
/*! Extends MaxBakeElement, allowing Texture Bake Elements to provide parameters
    of integer, float, and color types in addition to booleans.
    The RTT dialog is updated to display spinners and color swatches accordingly */
class MaxBakeElement8 : public MaxBakeElement
{
public:
	//! \brief MaxBakeElement8 interface ID
	enum { IID = 0x1f2203c5 };

	//! \brief Allows for getting the bake element parameter types
	/*! \param[in] nParam Parameter index, 0-based
	    \return Parameter type, 1 for TYPE_BOOL, 2 for TYPE_INT, 3 for TYPE_FLOAT, 4 for TYPE_FRGBA_BV, or 0 (TYPE_VOID) on error */
	virtual	int  GetParamType( int nParam ) = 0;

	//! \brief Allows for getting the bake element parameters as seen in the RTT dialog
	/*! \param[in] nParam Parameter index, 0-based
	    \return Parameter value of TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_FRGBA_BV, or TYPE_VOID on error */
	virtual	FPValue GetParamFPValue( int nParam ) = 0;

	//! \brief Allows for settings the bake element parameters as seen in the RTT dialog
	/*! \param[in] nParam Parameter index, 0-based
	    \param[in] newVal Parameter value of TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_FRGBA_BV */
	virtual	void SetParamFPValue( int nParam, FPValue newVal ) = 0;


	//! \brief Allows the bake element to specify a minimum value for int and float parameters
	/*! \param[in] nParam The parameter index, 0-based
	    \return Minimum parameter value, of TYPE_INT or TYPE_FLOAT */
	virtual	FPValue GetParamFPValueMin( int nParam ) = 0;

	//! \brief Allows the bake element to specify a maximum value for int and float parameters
	/*! \param[in] nParam The parameter index, 0-based
	    \return Maximum parameter value, of TYPE_INT or TYPE_FLOAT */
	virtual	FPValue GetParamFPValueMax( int nParam ) = 0;

	// ---------------------
	// from class Animatable
	// ---------------------
	using MaxBakeElement::GetInterface;
	//! \brief Renderer calls this method to see if IRenderElement is compatible with it
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : MaxBakeElement::GetInterface(id);}
	
};

//////////////////////////////////////////////////////////////
//
//		IRenderElement compatibility interface
//
//      The system will ask a Renderer for this interface by calling
//      Renderer::GetInterface(IRenderElementCompatible::ICompatibleID).
//      If the Renderer returns a pointer to this interface, it is implied that the Renderer
//      supports render elements. The system will then call IRenderElementCompatible::IsCompatible()
//      to determine if an IRenderElement instance is compatible with it.
//
//      To determine compatibility, the Renderer can call IRenderElement::GetInterface()
//      (inherited from class Animatable), passing in an interface ID that a compatible IRenderElement
//      would understand. If the Renderer receives a valid interface pointer, it can return TRUE to
//      IRenderElementCompatible::IsCompatible().
//
//      for an example, see class IScanRenderElement below
//
class IRenderElementCompatible
{
public:
	// this interface's ID
	enum { IID = 0xcafeface };

	virtual BOOL IsCompatible(IRenderElement *pIRenderElement) = 0;
};


//////////////////////////////////////////////////////////////
//
//		RenderElementMgr base interface
//
#define IREND_ELEM_MGR_INTERFACE Interface_ID(0x95791767, 0x17651746)


class IRenderElementMgr : public FPMixinInterface
{
public:

	// called by system to add an element merged from another file
	virtual BOOL AppendMergedRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL AppendMergedRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element

	// adds/removes an instance of IRenderElement to the manager's list
	virtual BOOL AddRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL AddRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	virtual BOOL RemoveRenderElement(IRenderElement *pRenderElement) = 0;
	virtual BOOL RemoveRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	virtual void RemoveAllRenderElements() = 0;

	// returns number of render elements in manager's list
	virtual int NumRenderElements() = 0;

	// returns pointer to a specific render element in manager's list -- NULL if invalid index
	virtual IRenderElement *GetRenderElement(int index) = 0;

	// sets/gets whether element list should be active during a render
	virtual void SetElementsActive(BOOL elementsActive) = 0;
	virtual BOOL GetElementsActive() const = 0;

	// sets/gets whether elements should be displayed in their own viewer
	virtual void SetDisplayElements(BOOL displayElements) = 0;
	virtual BOOL GetDisplayElements() const = 0;

	// sets/gets whether element list should be exported to Combustion file format
	virtual void SetCombustionOutputEnabled(BOOL combustionOutEnabled) = 0;
	virtual BOOL GetCombustionOutputEnabled() const = 0;

	// sets/gets Combustion output path
	virtual void SetCombustionOutputPath(const TCHAR *combustionOutputPath) = 0;
	virtual void SetCombustionOutputPath(const TSTR& combustionOutputPath) = 0;
	virtual const TSTR& GetCombustionOutputPath() const = 0;
	virtual const TCHAR* GetCombustionOutputPathPtr() const = 0;

#ifndef NO_RENDER_ELEMENTS
	// function publishing
	enum
	{
		fps_AddRenderElement, fps_RemoveRenderElement, fps_RemoveAllRenderElements,
		fps_NumRenderElements, fps_GetRenderElement,
		fps_SetElementsActive, fps_GetElementsActive,
		fps_SetDisplayElements,	fps_GetDisplayElements,
		fps_SetCombustionOutputEnabled, fps_GetCombustionOutputEnabled,
		fps_SetCombustionOutputPath, fps_GetCombustionOutputPath,
		fps_SetRenderElementFilename, fps_GetRenderElementFilename,
	};
#endif // NO_RENDER_ELEMENTS

};

#endif