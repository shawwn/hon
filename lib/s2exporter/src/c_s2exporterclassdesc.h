// (C)2005 S2 Games
// c_s2exporterclassdesc.h
//
// 3ds max class description for our exporter
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
#ifdef _DEBUG
#define S2EXPORTER_CLASS_ID	Class_ID(0x75874e2c, 0x7b912a59)
#else
#define S2EXPORTER_CLASS_ID Class_ID(0x3dd80fc8, 0x36bb49ce)
#endif

extern class ModelExporterR8 g_S2ModelExporter;
//=============================================================================

//=============================================================================
// CS2ExporterClassDesc
//=============================================================================
class CS2ExporterClassDesc : public ClassDesc2
{
public:
	int 			IsPublic()						{ return TRUE; }
	void*			Create(BOOL loading = FALSE)	{ return &g_S2ModelExporter; }
	SClass_ID		SuperClassID()					{ return UTILITY_CLASS_ID; }
	Class_ID		ClassID()						{ return S2EXPORTER_CLASS_ID; }
	const TCHAR* 	Category()						{ return GetString(IDS_CATEGORY); }

#ifdef _DEBUG
	const TCHAR*	ClassName()						{ return GetString(IDS_CLASS_NAME_D); }
	const TCHAR*	InternalName()					{ return _T("S2Exporter_debug"); }	// returns fixed parsable name (scripter-visible name)
#else
	const TCHAR*	ClassName()						{ return GetString(IDS_CLASS_NAME); }
	const TCHAR*	InternalName()					{ return _T("S2Exporter"); }	// returns fixed parsable name (scripter-visible name)
#endif
	HINSTANCE		HInstance()						{ return hInstance; }				// returns owning module handle

};
//=============================================================================
