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

#define PBLOCK_REF	0


inline float PcToFrac(int pc) { return (float)pc/100.0f; }

inline int FracToPc(float f) {
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
}

inline float Bound( float x, float min = 0.0f, float max = 1.0f ){ return x < min? min:( x > max? max : x); }
inline Color Bound( Color& c )
	{ return Color( Bound(c.r), Bound(c.g), Bound(c.b) ); }
/*===========================================================================*\
 |	Definition of our UI and map parameters for the shader
\*===========================================================================*/

// Number of Map Buttons on our UI and number of texmaps
#define NMBUTS 3
#define SHADER_NTEXMAPS	3
// Channels used by this shader
#define S_DI	0
#define S_BR	1
#define S_TR	2


// Texture Channel number --> IDC resource ID
static int texMButtonsIDC[] = {
	IDC_MAPON_CLR, IDC_MAPON_BR, IDC_MAPON_TR,
};
		
// Map Button --> Texture Map number
static int texmapFromMBut[] = { 0, 1, 2 };


// Channel Name array
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_MN_DIFFUSE,		IDS_MN_BRIGHTNESS,	IDS_MN_OPACITY,		IDS_MN_NONE, 
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
};	

// Channel Name array (INTERNAL NAMES)
static int intertexNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_MN_DIFFUSE_I,	IDS_MN_BRIGHTNESS_I,IDS_MN_OPACITY_I,		IDS_MN_NONE, 
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
	IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,		IDS_MN_NONE,
};	

// Internal channel names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("diffuseMap"),_T("BrightnessMap"), _T("opacityMap"), _T(""), 	
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
};	

// Type of map channels supported
static int chanType[STD2_NMAX_TEXMAPS] = {
	CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};	


// What channel in our shader maps to one of the StdMat channel IDs?
//
//
// The following are the StdMat channel IDs:
//
// 0   --	ambient
// 1   --	diffuse
// 2   --	specular
// 3   --	shininesNs
// 4   --	shininess strength
// 5   --	self-illumination
// 6   --	opacity
// 7   --	filter color
// 8   --	bump 
// 9   --	reflection
// 10  --	refraction 
// 11  --	displacement

static int stdIDToChannel[N_ID_CHANNELS] = { -1, 0, -1, -1, -1, -1, 2, -1, -1, -1, -1, -1 };



/*===========================================================================*\
 |	Class definition for the shader itself
\*===========================================================================*/

#define SHADER_PARAMS (STD_EXTRA)

class [!output CLASS_NAME]Dlg;





class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0
	
	private:
	friend class [!output CLASS_NAME]Dlg;
	protected:
		Interval		ivalid;
		TimeValue		curTime;

		// Pointer to the dialog handler
		[!output CLASS_NAME]Dlg*	paramDlg;

		// Storage for our parameters
		Color			diffuse;
		float			brightness;

	public:
		TSTR GetName() { return GetString( IDS_CLASS_NAME ); }

			// Tell MAX what standard parameters that we support
		ULONG SupportStdParams(){ return SHADER_PARAMS; }

		// copy std params, for switching shaders
		void CopyStdParams( Shader* pFrom );
		void ConvertParamBlk( ParamBlockDescID *oldPBDesc, int oldCount, IParamBlock *oldPB ){};

		// Texture map channel support
		long nTexChannelsSupported(){ return SHADER_NTEXMAPS; }
		TSTR GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
		TSTR GetTexChannelInternalName( long nTex ){ return GetString( intertexNameIDS[ nTex ] ); }
		long ChannelType( long nChan ) { return chanType[nChan]; }
		long StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

		// Find out if we have a key at time value t
		BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }

		// Get the requirements for this material (supersampling, etc)
		ULONG GetRequirements( int subMtlNum ){ return MTLREQ_PHONG; }

		// Support for the dialog UI
		ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen,int );
		ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
		void SetParamDlg( ShaderParamDlg* newDlg,int ){ paramDlg = ([!output CLASS_NAME]Dlg*)newDlg; }

			// Shader state methods
		void Update(TimeValue t, Interval& valid);
		void Reset();

		// Fill the IllumParams with our data
		void GetIllumParams( ShadeContext &sc, IllumParams &ip );

		// Shader specific implimentations
		void Illum(ShadeContext &sc, IllumParams &ip);
		void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol);

		void CombineComponents( ShadeContext &sc, IllumParams& ip );

		float EvalHiliteCurve(float x);

		// Metal support
		BOOL IsMetal();

		// This plugin specific
		void SetBrightness(float v, TimeValue t);
		float GetBrightness(int mtlNum=0, BOOL backFace=FALSE);
		float GetBrightness( TimeValue t);

		// DIFFUSE
		void SetDiffuseClr(Color c, TimeValue t);
		Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetDiffuseClr(TimeValue t);		

		// AMBIENT
		void SetAmbientClr(Color c, TimeValue t);
		Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetAmbientClr(TimeValue t);		

		// SPECULAR
		void SetSpecularClr(Color c, TimeValue t);
		void SetSpecularLevel(float v, TimeValue t);		
		Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE);
		float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE);
		Color GetSpecularClr(TimeValue t);
		float GetSpecularLevel(TimeValue t);

		// SELFILLUM
		void SetSelfIllum(float v, TimeValue t);
		float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE);
		void SetSelfIllumClrOn( BOOL on );
		BOOL IsSelfIllumClrOn();
		BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace);
		void SetSelfIllumClr(Color c, TimeValue t);
		Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE);
		float GetSelfIllum(TimeValue t);	
		Color GetSelfIllumClr(TimeValue t);		

		// SOFTEN
		void SetSoftenLevel(float v, TimeValue t);
		float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE);
		float GetSoftenLevel(TimeValue t);

		void SetGlossiness(float v, TimeValue t);
		float GetGlossiness(int mtlNum, BOOL backFace);
		float GetGlossiness( TimeValue t);

		// Standard locks not supported (Diffuse+Specular)/(Ambient+Diffuse)
		void SetLockDS(BOOL lock){ }
		BOOL GetLockDS(){ return FALSE; }
		void SetLockAD(BOOL lock){ }
		BOOL GetLockAD(){ return FALSE; }
		void SetLockADTex(BOOL lock){ }
		BOOL GetLockADTex(){ return FALSE; }

		void NotifyChanged(){ NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); }
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return SHADER_CLASS_ID ; }
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

#define SHADER_NCOLBOX 1

class [!output CLASS_NAME]Dlg : public ShaderParamDlg {
public:
	
	[!output CLASS_NAME]*	pShader;
	StdMat2*		pMtl;
	HPALETTE		hOldPal;
	HWND			hwmEdit;
	IMtlParams*		pMtlPar;
	HWND			hwHilite;
	HWND			hRollup;
	TimeValue		curTime;
	BOOL			valid;
	BOOL			isActive;


	IColorSwatch *cs[SHADER_NCOLBOX];
	ISpinnerControl *brSpin, *trSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	

	[!output CLASS_NAME]Dlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~[!output CLASS_NAME]Dlg(); 


	// DnD: Required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i<NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return texmapFromMBut[i];
		}	
		return -1;
	}


	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return [!output CLASS_NAME]_CLASS_ID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	pShader = ([!output CLASS_NAME]*)theShader; 
		if (pShader)pShader->SetParamDlg(this,0); 
		pMtl = theMtl; 
	}


	ReferenceTarget* GetThing(){ return (ReferenceTarget*)pMtl; }
	Shader* GetShader(){ return pShader; }
	
	void SetTime(TimeValue t) 
	{ 
		if (!pShader->ivalid.InInterval(t)) {
			Interval v;
			pShader->Update(t,v);
			LoadDialog(TRUE); 
		}
		curTime = t; 
	}		
	BOOL KeyAtCurTime(int id) { return pShader->KeyAtTime(id,curTime); } 

	void DeleteThis() { delete this; }
	void ActivateDlg( BOOL dlgOn ){ isActive = dlgOn; }
	HWND GetHWnd(){ return hRollup; }

	void NotifyChanged(){ pShader->NotifyChanged(); }
	void LoadDialog(BOOL draw);
	void ReloadDialog(){ Interval v; pShader->Update(pMtlPar->GetTime(), v); LoadDialog(FALSE);}
	void UpdateDialog( ParamID paramId ){ ReloadDialog(); }

	void UpdateMtlDisplay(){ pMtlPar->MtlChanged(); }
    void UpdateHilite( );
	void UpdateColSwatches();
	void UpdateMapButtons();
	void UpdateOpacity();

	void SelectEditColor(int i) { cs[ i ]->EditThis(FALSE); }
};

static BOOL CALLBACK  [!output CLASS_NAME]DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	[!output CLASS_NAME]Dlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = ([!output CLASS_NAME]Dlg*)lParam;
		SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
	} else {
	    if ( (theDlg = ([!output CLASS_NAME]Dlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* [!output CLASS_NAME]::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen,int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	[!output CLASS_NAME]Dlg *pDlg = new [!output CLASS_NAME]Dlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );


	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			[!output CLASS_NAME]DlgProc, 
			GetString(IDS_PARAMS),
			(LPARAM)pDlg , 
			rollupOpen
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			[!output CLASS_NAME]DlgProc, 
			GetString(IDS_PARAMS),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult [!output CLASS_NAME]::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									  PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock){
				// update UI if paramblock changed, possibly from scripter
				ParamID changingParam = pblock->LastNotifyParamID();
				// reload the dialog if present
				if (paramDlg){
					paramDlg->UpdateDialog(changingParam);
				}
				// notify our dependents that we've changed
				NotifyChanged();
			}
			break;
	}
	return(REF_SUCCEED);
}

static void VertLine(HDC hdc,int x, int ystart, int yend) 
{
	MoveToEx(hdc, x, ystart, NULL); 
	if (ystart <= yend)
		LineTo(hdc, x, yend+1);
	else 
		LineTo(hdc, x, yend-1);
}

void DrawHilite(HDC hdc, Rect& rect, Shader* pShader )
{
int w,h,npts,xcen,ybot,ytop,ylast,i,iy;

	HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
	HPEN fgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));
	HPEN bgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNSHADOW));

	w = rect.w();
	h = rect.h()-3;
	npts = (w-2)/2;
	xcen = rect.left+npts;
	ybot = rect.top+h;
	ytop = rect.top+2;
	ylast = -1;
	for (i=0; i<npts; i++) {
		float v = pShader->EvalHiliteCurve( (float)i/((float)npts*2.0f) );
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = ybot-(int)(v*((float)h-2.0f));

		if (iy<ytop) iy = ytop;

		SelectPen(hdc, fgPen);
		VertLine(hdc,xcen+i,ybot,iy);
		VertLine(hdc,xcen-i,ybot,iy);

		if (iy-1>ytop) {
			// Fill in above curve
			SelectPen(hdc,bgPen);
			VertLine(hdc,xcen+i, ytop, iy-1);
			VertLine(hdc,xcen-i, ytop, iy-1);
			}
		if (ylast>=0) {
			SelectPen(hdc,linePen);
			VertLine(hdc,xcen+i-1,iy-1,ylast);
			VertLine(hdc,xcen-i+1,iy-1,ylast);
			}

		ylast = iy;
	}

	SelectObject( hdc, linePen );
	DeleteObject(fgPen);
	DeleteObject(bgPen);
	WhiteRect3D(hdc, rect, 1);
}

LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLong(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	ShaderParamDlg *theDlg = (ShaderParamDlg *)GetWindowLong(hwParent, GWL_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_LBUTTONUP: 
		case WM_CREATE:
		case WM_DESTROY: 
		break;

		case WM_PAINT: 	
		{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				Shader* pShader = (Shader*)(theDlg->GetShader());
				DrawHilite(hdc, rect, pShader );
			}
			EndPaint( hwnd, &ps );
		}													
		break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 


[!output CLASS_NAME]Dlg::[!output CLASS_NAME]Dlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	brSpin = trSpin = NULL;
	hRollup = hwHilite = NULL;
	curTime = 0;
	isActive = valid = FALSE;

	for( long i = 0; i < SHADER_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

[!output CLASS_NAME]Dlg::~[!output CLASS_NAME]Dlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}
	
	ReleaseISpinner(brSpin);
	ReleaseISpinner(trSpin);

	SetWindowLong(hRollup, GWL_USERDATA, NULL);
	SetWindowLong(hwHilite, GWL_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}


void [!output CLASS_NAME]Dlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		brSpin->SetValue( pShader->GetBrightness() ,FALSE);
		brSpin->SetKeyBrackets(KeyAtCurTime(pb_brightness));
		
		trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		UpdateColSwatches();
		UpdateHilite();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void [!output CLASS_NAME]Dlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}


void [!output CLASS_NAME]Dlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}

void [!output CLASS_NAME]Dlg::UpdateColSwatches() 
{
	cs[0]->SetKeyBrackets( pShader->KeyAtTime(pb_diffuse,curTime) );
	cs[0]->SetColor( pShader->GetDiffuseClr() );
}


void [!output CLASS_NAME]Dlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}



static int ColorIDCToIndex(int idc) {
	switch (idc) {
		case IDC_COLOR: return 0;
		default: return 0;
	}
}


BOOL [!output CLASS_NAME]Dlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			HWND hwndCS = GetDlgItem(hwndDlg, IDC_COLOR);
			cs[0] = GetIColorSwatch( hwndCS, pShader->GetDiffuseClr(), GetString(IDS_COLOR) );

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLong( hwHilite, GWL_WNDPROC, (LONG)HiliteWndProc);

			brSpin = SetupFloatSpinner(hwndDlg, IDC_BR_SPIN, IDC_BR_EDIT, 0.0f,1.0f, 0);
			brSpin->SetScale(0.01f);

			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}
			LoadDialog(TRUE);
		}
		break;

		case WM_COMMAND: 
			{
			for ( int i=0; i<NMBUTS; i++) {
				if (id == texMButtonsIDC[i]) {
					PostMessage(hwmEdit,WM_TEXMAP_BUTTON, texmapFromMBut[i],(LPARAM)pMtl );
					UpdateMapButtons();
					goto exit;
					}
				}
			}

			break;

		case CC_COLOR_SEL: {
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
		}			
		break;
		case CC_COLOR_DROP:	{
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
		}
		break;
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
		 break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_PARAMCHG));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE: {			
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = ColorIDCToIndex(id);
			if (buttonUp) theHold.Begin();
			DWORD curRGB = cs[n]->GetColor();
			pShader->SetDiffuseClr((Color)curRGB, curTime); 
			if (buttonUp) {
				theHold.Accept(GetString(IDS_PARAMCHG));
				UpdateMtlDisplay();				
				}
		} break;
		case WM_PAINT: 
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_CLOSE:
		case WM_DESTROY: 
			break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_BR_SPIN: 
					pShader->SetBrightness( brSpin->GetFVal() , curTime); 
					UpdateHilite();
					break;
				case IDC_TR_SPIN: 
					pMtl->SetOpacity(PcToFrac( trSpin->GetIVal()),curTime); 
					break;
			}
			UpdateMtlDisplay();
		break;

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_PARAMCHG));
			else 
				theHold.Cancel();
			UpdateMtlDisplay();
			break;

    }
	exit:
	return FALSE;
	}

/*===========================================================================*\
 |	Constructor
\*===========================================================================*/

[!output CLASS_NAME]::~[!output CLASS_NAME]() 
{ 

}

[!output CLASS_NAME]::[!output CLASS_NAME]() 
{ 
	pblock = NULL; 
	paramDlg = NULL; 
	curTime = 0;
	ivalid.SetEmpty(); 
}



/*===========================================================================*\
 |	Cloning and coping standard parameters
\*===========================================================================*/

void [!output CLASS_NAME]::CopyStdParams( Shader* pFrom )
{
	// We don't want to see this parameter copying in macrorecorder
	macroRecorder->Disable(); 

		SetAmbientClr( pFrom->GetAmbientClr(0,0), curTime );
		SetDiffuseClr( pFrom->GetDiffuseClr(0,0), curTime );

	macroRecorder->Enable();
	ivalid.SetEmpty();	
}


RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* mnew = new [!output CLASS_NAME]();
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->diffuse = diffuse;
	mnew->brightness = brightness;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}



/*===========================================================================*\
 |	Shader state
\*===========================================================================*/

void [!output CLASS_NAME]::Update(TimeValue t, Interval &valid) {
	Point3 p;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		pblock->GetValue( pb_diffuse, t, p, ivalid );
		diffuse= Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( pb_brightness, t, brightness, ivalid );
		brightness = Bound(brightness );
		curTime = t;
	}
	valid &= ivalid;
}

void [!output CLASS_NAME]::Reset()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);	// Create and intialize paramblock2

	ivalid.SetEmpty();
	SetDiffuseClr( Color(0.8f,0.5f,0.5f), 0 );
	SetBrightness( 0.2f,0);   
}



/*===========================================================================*\
 |	Shader load/save
\*===========================================================================*/


#define SHADER_VERS_CHUNK 0x6500

IOResult [!output CLASS_NAME]::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = 1;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	return IO_OK;
}		



IOResult [!output CLASS_NAME]::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	return IO_OK;
}

/*===========================================================================*\
 |	Actually shade the surface
\*===========================================================================*/

void [!output CLASS_NAME]::GetIllumParams( ShadeContext &sc, IllumParams &ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[S_DI] = diffuse;
	ip.channels[S_BR].r = brightness;
}

void [!output CLASS_NAME]::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *l;
	Color lightCol;

	Color Cd		= ip.channels[ S_DI ];
	float bright	= ip.channels[ S_BR ].r;

	BOOL is_shiny	= (bright > 0.0f) ? 1:0; 
	double phExp	= pow(2.0, bright * 10.0) * 4.0; 

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (l->Illuminate(sc,sc.Normal(),lightCol,L,NL,diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			if (l->affectDiffuse)
				ip.diffIllumOut += diffCoef * lightCol;

			if (is_shiny&&l->affectSpecular) {
				Point3 H = Normalize(L-sc.V());
				float c = DotProd(sc.Normal(),H);	 
				if (c>0.0f) {
					c = (float)pow((double)c, phExp); 
					ip.specIllumOut += c * bright * lightCol;
					}
				}
 			}
		}



	// now we can multiply by the clrs
	ip.ambIllumOut *= Cd * 0.5f; 
	ip.diffIllumOut *= Cd;
	CombineComponents( sc, ip ); 

}


void [!output CLASS_NAME]::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) 
{ 
}


void [!output CLASS_NAME]::CombineComponents( ShadeContext &sc, IllumParams& ip )
{ 
	ip.finalC = ip.finalOpac * (ip.ambIllumOut + ip.diffIllumOut  + ip.selfIllumOut)
			+ ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut; 
}


float [!output CLASS_NAME]::EvalHiliteCurve(float x) 
{
	double phExp	= pow(2.0, brightness * 10.0) * 4.0; 
	return 1.0f*(float)pow((double)cos(x*PI), phExp );  
}


BOOL [!output CLASS_NAME]::IsMetal() 
{ 
	return FALSE; 
}

/*===========================================================================*\
 |	Shader specific transaction
\*===========================================================================*/

void [!output CLASS_NAME]::SetBrightness(float v, TimeValue t)
{ 
	brightness= v; 
	pblock->SetValue( pb_brightness, t, v); 
}

float [!output CLASS_NAME]::GetBrightness(int mtlNum, BOOL backFace)
{ 
	return brightness; 
}	

float [!output CLASS_NAME]::GetBrightness( TimeValue t)
{
	return pblock->GetFloat(pb_brightness,t);  
}		

void [!output CLASS_NAME]::SetGlossiness(float v, TimeValue t)
{ 
}

float [!output CLASS_NAME]::GetGlossiness(int mtlNum, BOOL backFace)
{ 
	return 40.0f; 
}	

float [!output CLASS_NAME]::GetGlossiness( TimeValue t)
{
	return 40.0f;  
}		


/*===========================================================================*\
 |	Diffuse Transactions
\*===========================================================================*/

void [!output CLASS_NAME]::SetDiffuseClr(Color c, TimeValue t)		
{ 
	diffuse = c; pblock->SetValue( pb_diffuse, t, c); 
}

Color [!output CLASS_NAME]::GetDiffuseClr(int mtlNum, BOOL backFace)
{ 
	return diffuse;
}

Color [!output CLASS_NAME]::GetDiffuseClr(TimeValue t)
{ 
	return pblock->GetColor(pb_diffuse,t); 
}


/*===========================================================================*\
 |	Ambient Transactions
\*===========================================================================*/

void [!output CLASS_NAME]::SetAmbientClr(Color c, TimeValue t)
{
}

Color [!output CLASS_NAME]::GetAmbientClr(int mtlNum, BOOL backFace)
{ 
	return diffuse * 0.5f;
}

Color [!output CLASS_NAME]::GetAmbientClr(TimeValue t)
{ 
	return diffuse * 0.5f; 
}


/*===========================================================================*\
 |	Specular Transactions
\*===========================================================================*/

void [!output CLASS_NAME]::SetSpecularClr(Color c, TimeValue t)
{
}

void [!output CLASS_NAME]::SetSpecularLevel(float v, TimeValue t)
{
}
		
Color [!output CLASS_NAME]::GetSpecularClr(int mtlNum, BOOL backFace)
{ 
	return Color(0.9f,0.9f,0.9f); 
}

float [!output CLASS_NAME]::GetSpecularLevel(int mtlNum, BOOL backFace)
{ 
	return 1.0f; 
}

Color [!output CLASS_NAME]::GetSpecularClr(TimeValue t)
{ 
	return Color(0.9f,0.9f,0.9f);
}

float [!output CLASS_NAME]::GetSpecularLevel(TimeValue t)
{ 
	return 1.0f; 
}


/*===========================================================================*\
 |	SelfIllum Transactions
\*===========================================================================*/

void [!output CLASS_NAME]::SetSelfIllum(float v, TimeValue t)	
{
}

float [!output CLASS_NAME]::GetSelfIllum(int mtlNum, BOOL backFace)
{ 
	return 0.0f; 
}

void [!output CLASS_NAME]::SetSelfIllumClrOn( BOOL on )
{
}

BOOL [!output CLASS_NAME]::IsSelfIllumClrOn()
{ 
	return FALSE; 
}

BOOL [!output CLASS_NAME]::IsSelfIllumClrOn(int mtlNum, BOOL backFace)
{ 
	return FALSE; 
}

void [!output CLASS_NAME]::SetSelfIllumClr(Color c, TimeValue t)
{
}

Color [!output CLASS_NAME]::GetSelfIllumClr(int mtlNum, BOOL backFace)
{ 
	return Color(0,0,0); 
}

float [!output CLASS_NAME]::GetSelfIllum(TimeValue t)
{ 
	return 0.0f;
}		

Color [!output CLASS_NAME]::GetSelfIllumClr(TimeValue t)
{ 
	return Color(0,0,0);
}		


/*===========================================================================*\
 |	Soften Transactions
\*===========================================================================*/

void [!output CLASS_NAME]::SetSoftenLevel(float v, TimeValue t)
{
}

float [!output CLASS_NAME]::GetSoftenLevel(int mtlNum, BOOL backFace)
{ 
	return 0.0f; 
}

float [!output CLASS_NAME]::GetSoftenLevel(TimeValue t)
{ 
	return  0.0f; 
}

