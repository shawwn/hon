//***************************************************************************
//* Asciiexp.h	- Ascii File Importer
//* 
//* NOTE: This is unsupported material - not supported by Kinetix.
//*
//* By Christer Janson
//* Kinetix Development
//*
//* January 20, 1997 CCJ Initial coding
//*
//* Class definition and file format tokens
//*
//* Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************


/*********************************************************************************
REVISION LOG ENTRY
Revision By: rjc
Revised on 4/16/2001 7:19:52 PM
Comments: 16 apr 2001  - rjc : Making changes and upgrades for R4
*********************************************************************************/


/*********************************************************************************
REVISION LOG ENTRY
Revision By: rjc
Revised on 06/29/2000 11:18:37 AM
Comments:  rjc 29 jun 2000 :  Making changes and upgrades for v3.1
*********************************************************************************/
#ifndef __ASCIIIMP__H
#define __ASCIIIMP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "splshape.h"
#include "dummy.h"

// Token definitions (get from asciiexp project)
#include "asciitok.h"

#define USE_IMPNODES 1

extern ClassDesc* GetAsciiImpDesc();

extern HINSTANCE hInstance;

typedef Tab<INodeTab*> NodeTabTab;

class GroupManager 
{
public:
	BOOL	BeginGroup(TCHAR* groupName);
	BOOL	EndGroup(Interface* i);
	BOOL	IsRecording();
	BOOL	AddNode(INode* node);

	BOOL	ProcessGroups(Interface* i);

private:
	NodeTabTab	nodeTabTab;
	NameTab		groupNameTab;
};

class AsciiImp : public SceneImport 
{
public:
	AsciiImp();
	~AsciiImp();
	int		ExtCount();     // Number of extensions supported 
	const TCHAR * Ext(int n);     // Extension #n (i.e. "ASC")
	const TCHAR * LongDesc();     // Long ASCII description (i.e. "Ascii Export") 
	const TCHAR * ShortDesc();    // Short ASCII description (i.e. "Ascii")
	const TCHAR * AuthorName();    // ASCII Author name
	const TCHAR * CopyrightMessage();   // ASCII Copyright message 
	const TCHAR * OtherMessage1();   // Other message #1
	const TCHAR * OtherMessage2();   // Other message #2
	unsigned int Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void	ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	int		DoImport(const TCHAR *name,ImpInterface *ei,Interface *i, BOOL suppressPrompts); // Import file
	
	BOOL	ImportGeomObject();
	BOOL	ImportCamera();
	BOOL	ImportLight();
	BOOL	ImportShape();
	BOOL	ImportHelper();
	BOOL	ImportMaterialList();

	// Misc
	BOOL	IsWhite(TCHAR c);
	BOOL	Compare(TCHAR* token, TCHAR* id);
	TCHAR*	GetToken();
	BOOL	SkipBlock();
	Point3	GetPoint3();
	int		GetInt();
	float	GetFloat();
	TSTR	GetString();
	Matrix3	GetNodeTM(TSTR& name, DWORD& iFlags);
	INode*	GetNodeByName(TCHAR* name);
	BOOL	RecordNode(INode* node);


	// Scene params
	BOOL	ImportSceneParams();

	// GeomObject
	BOOL	GetVertexList(TriObject* tri);
	DWORD	GetSmoothingGroups(TCHAR* smStr);
	BOOL	GetFaceList(TriObject* tri);
	BOOL	GetTVertexList(TriObject* tri);
	BOOL	GetTFaceList(TriObject* tri);
	BOOL	GetCVertexList(TriObject* tri);
	BOOL	GetCFaceList(TriObject* tri);
	BOOL	GetMeshNormals(TriObject* tri);
	BOOL	GetMesh(TriObject* tri);

	// Camera object
	TimeValue	GetCameraSettings(GenCamera* cam);

	// LightObject
	BOOL	GetLightExclusions(GenLight* light);
	TimeValue	GetLightSettings(GenLight* light);

	// Materials
	Mtl*	GetMaterial();
	BOOL	GetStandardMtl(Mtl*& mtl);
	BOOL	GetMultiSubObjMtl(Mtl*& mtl);

	// Textures
	Texmap*	GetTexture(float& amount);
	BOOL	GetBitmapTexture(Texmap*& tex, float& amount);
	BOOL	GetMaskTexture(Texmap*& tex, float& amount);
	BOOL	GetRGBTintTexture(Texmap*& tex, float& amount);
	BOOL	GetCheckerTexture(Texmap*& tex, float& amount);
	BOOL	GetMixTexture(Texmap*& tex, float& amount);
	BOOL	GetMarbleTexture(Texmap*& tex, float& amount);
	BOOL	GetNoiseTexture(Texmap*& tex, float& amount);
	BOOL	GetReflectRefractTexture(Texmap*& tex, float& amount);
	BOOL	GetFlatMirrorTexture(Texmap*& tex, float& amount);
	BOOL	GetGradientTexture(Texmap*& tex, float& amount);
	BOOL	GetCompositeTexture(Texmap*& tex, float& amount);
	BOOL	GetWoodTexture(Texmap*& tex, float& amount);
	BOOL	GetDentTexture(Texmap*& tex, float& amount);
	BOOL	GetCellularTexture(Texmap*& tex, float& amount);
	BOOL	GetFalloffTexture(Texmap*& tex, float& amount);
	BOOL	GetOutputTexture(Texmap*& tex, float& amount);
	BOOL	GetParticleAgeTexture(Texmap*& tex, float& amount);
	BOOL	GetParticleMBlurTexture(Texmap*& tex, float& amount);
	BOOL	GetPerlinMarbleTexture(Texmap*& tex, float& amount);
	BOOL	GetPlanetTexture(Texmap*& tex, float& amount);
	BOOL	GetRaytraceTexture(Texmap*& tex, float& amount);
	BOOL	GetRGBMultTexture(Texmap*& tex, float& amount);
	BOOL	GetSmokeTexture(Texmap*& tex, float& amount);
	BOOL	GetSpeckleTexture(Texmap*& tex, float& amount);
	BOOL	GetSplatTexture(Texmap*& tex, float& amount);
	BOOL	GetStuccoTexture(Texmap*& tex, float& amount);
	BOOL	GetThinWallRefractTexture(Texmap*& tex, float& amount);
	BOOL	GetVertexColorTexture(Texmap*& tex, float& amount);
	BOOL	GetWaterTexture(Texmap*& tex, float& amount);

	// TM Animation
	BOOL	ImportTMAnimation();


public:
	static BOOL resetScene;
	
private:
	Interface*		ip;
	ImpInterface*	impip;
	FILE*			pStream;
	INodeTab		nodeTab;
	Tab<Mtl*>		mtlTab;
	int				fileVersion;
	GroupManager	groupMgr;
};

#endif // __ASCIIIMP__H
