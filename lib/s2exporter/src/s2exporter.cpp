// (C)2008 S2 Games
// ModelExporterR8.cpp
//
// 3dsmax .model exporter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "s2exporter.h"
#include "notify.h"

#include "decomp.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "misc.h"
#include "c_outmaterial.h"
#include "c_outmodel.h"
#include "c_outclip.h"
#include "c_outmdf.h"
#include "s_blendedlink.h"

#include "NvTriStrip.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
static float one = 1.0;
static float *pOne = &one;

typedef matrix44_t transform_t;

#define	FACEVERTEX(mesh, face, idx) mesh->verts[mesh->faces[face].v[idx]]

char *nodeTypeNames[] =
{
	"Sprite",
	"Mesh",
	"Reference Bone",
	"Collision Surface"
};

char *editorCategories[] =
{
	"Nomad_Chars",
	"Nomad_Structs",
	"Nomad_Props",
	"Nomad_Weapons",
	"Nomad_Artillery",

	"Beast_Chars",
	"Beast_Structs",
	"Beast_Props",
	"Beast_Weapons",
	"Beast_Artillery",

	"NPC_Chars",
	"NPC_Structs",
	"NPC_Props",

	"Trees",
	"Foliage",
	"General_Nature",
	"Rocks",
	""
};

ISpinnerControl *scaleRangeLow = NULL;
ISpinnerControl *scaleRangeHigh = NULL;
//=============================================================================

ModelExporterR8 g_S2ModelExporter;

/*====================
  SelectionChanged
  ====================*/
static void	SelectionChanged(void *pParam, NotifyInfo *pInfo)
{
	g_S2ModelExporter.Mesh_ApplyCurrentSettings();
	g_S2ModelExporter.UpdateMeshPanel();
}


/*====================
  ModelExporterR8DlgProc
  ====================*/
static BOOL CALLBACK ModelExporterR8DlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			RegisterNotification(SelectionChanged, NULL, NOTIFY_SELECTIONSET_CHANGED);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_FORCE_STATIC, 0);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_REMOVE_HIERARCHY, 0);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_COPY_TEXTURES, 1);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_GEOMETRY, 1);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_MDF, 1);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_MATERIALS, 1);
			CheckDlgButton(hWndDlg, IDC_EXPOPT_ANIMATION, 0);
			break;

		case WM_DESTROY:
			UnRegisterNotification(SelectionChanged, NULL, NOTIFY_SELECTIONSET_CHANGED);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EXPORT_BUTTON:
					g_S2ModelExporter.Export(false);
					break;

				case IDC_EXPORT_CLIP:
					g_S2ModelExporter.Export(true);
					break;

				case IDC_CLOSE:
					g_S2ModelExporter.Close();
					break;

				case IDC_SCENE_INFO:
					g_S2ModelExporter.SceneInfo();
					break;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


/*====================
  MeshPanelDlgProc
  ====================*/
static BOOL CALLBACK MeshPanelDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		break;

	case WM_DESTROY:
		g_S2ModelExporter.UpdateMeshPanel();
		break;

	case WM_PAINT:
		g_S2ModelExporter.UpdateMeshPanel();
		return FALSE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_MESH_NAME:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				DisableAccelerators();
				break;

			case EN_KILLFOCUS:
				{
                    EnableAccelerators();

					char buffer[256];
					GetDlgItemText(hWndDlg, IDC_MESH_NAME, buffer, 255);
					if (buffer[0] != 0)
						g_S2ModelExporter.Mesh_SetName(buffer);
				}
				break;

			default:
				break;
			}
			break;

		case IDC_MESH_MATERIAL_VSH:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				DisableAccelerators();
				break;

			case EN_KILLFOCUS:
				{
                    EnableAccelerators();

					char buffer[256];
					GetDlgItemText(hWndDlg, IDC_MESH_MATERIAL_VSH, buffer, 255);
					if (buffer[0] != 0)
						g_S2ModelExporter.Mesh_SetVSHname(buffer);
				}
				break;

			default:
				break;
			}
			break;

		case IDC_MESH_MATERIAL_PSH:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				DisableAccelerators();
				break;

			case EN_KILLFOCUS:
				{
                    EnableAccelerators();

					char buffer[256];
					GetDlgItemText(hWndDlg, IDC_MESH_MATERIAL_PSH, buffer, 255);
					if (buffer[0] != 0)
						g_S2ModelExporter.Mesh_SetPSHname(buffer);
				}
				break;

			default:
				break;
			}
			break;

		case IDC_MESH_EXCLUDENRMLS:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				g_S2ModelExporter.Mesh_SetProp("exclude_normals", IsDlgButtonChecked(hWndDlg, IDC_MESH_EXCLUDENRMLS) == TRUE);
				break;

			default:
				break;
			}
			break;

		case IDC_MESH_ROOF:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				g_S2ModelExporter.Mesh_SetProp("roof", IsDlgButtonChecked(hWndDlg, IDC_MESH_ROOF) == TRUE);
				break;

			default:
				break;
			}
			break;

		case IDC_MESH_R_NORMAL:
			g_S2ModelExporter.Mesh_SetProp("obj_type", "mesh");
			break;

		case IDC_MESH_R_SURF:
			g_S2ModelExporter.Mesh_SetProp("obj_type", "surf");
			break;

		case IDC_MESH_R_BONE:
			g_S2ModelExporter.Mesh_SetProp("obj_type", "bone");
			break;

		case IDC_MESH_R_SPRITE:
			g_S2ModelExporter.Mesh_SetProp("obj_type", "sprite");
			break;

		case IDC_MESH_R_GROUND:
			g_S2ModelExporter.Mesh_SetProp("obj_type", "ground_plane");
			break;

		default:
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}


/*====================
  ObjectSettingsDlgProc
  ====================*/
static BOOL CALLBACK ObjectSettingsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int n = 0;

	switch(msg)
	{
		case WM_INITDIALOG:
			scaleRangeLow = SetupFloatSpinner(hwndDlg, IDC_SCALE_RANGE_LOW_SPIN, IDC_SCALE_RANGE_LOW, 0.1f, 10, g_S2ModelExporter.scale_range_low, 0.01f);
			scaleRangeHigh = SetupFloatSpinner(hwndDlg, IDC_SCALE_RANGE_HIGH_SPIN, IDC_SCALE_RANGE_HIGH, 0.1f, 10, g_S2ModelExporter.scale_range_high, 0.01f);
			SetDlgItemText(hwndDlg, IDC_OBJECT_NAME, g_S2ModelExporter.object_name);
			SetDlgItemText(hwndDlg, IDC_CATEGORY, g_S2ModelExporter.object_category);
			//set up editor category dropdown
			while(editorCategories[n][0])
			{
				SendDlgItemMessage(hwndDlg, IDC_CATEGORY, CB_ADDSTRING, 0, ((LPARAM)(LPCSTR)editorCategories[n]));
				++n;
			}

			CenterWindow(hwndDlg,GetParent(hwndDlg));
			break;

		case WM_COMMAND:
			switch LOWORD(wParam)
			{
				case IDOK:
					g_S2ModelExporter.scale_range_low = scaleRangeLow->GetFVal();
					g_S2ModelExporter.scale_range_high = scaleRangeHigh->GetFVal();
					GetDlgItemText(hwndDlg, IDC_OBJECT_NAME, g_S2ModelExporter.object_name, OBJECT_NAME_LENGTH-1);
					GetDlgItemText(hwndDlg, IDC_CATEGORY, g_S2ModelExporter.object_category, OBJECT_CATEGORY_LENGTH-1);
					EndDialog(hwndDlg, TRUE);
					break;

	        	case IDCANCEL:
					EndDialog(hwndDlg, FALSE);
					break;

				default:
					break;
			}
		default:
			return FALSE;
	}

	return TRUE;
}


/*====================
  SceneInfoDlgProc
  ====================*/
static BOOL CALLBACK SceneInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_DETAILS, g_S2ModelExporter.GetOutptBuffer());
		return TRUE;

	case WM_COMMAND:
		switch LOWORD(wParam)
		{
		case IDOK:
			EndDialog(hwndDlg, TRUE);
			return TRUE;

	    default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}


/*====================
  ErrorInfoDlgProc
  ====================*/
static BOOL CALLBACK ErrorInfoDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg, IDC_DETAILS, g_S2ModelExporter.error_message);
		return TRUE;

	case WM_COMMAND:
		switch LOWORD(wParam)
		{
		case IDOK:
			EndDialog(hwndDlg, TRUE);
			return TRUE;

	    default:
			return FALSE;
		}

	default:
		return FALSE;
	}
}


/*====================
  StatusBarDlgProc
  ====================*/
static BOOL CALLBACK StatusBarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	default:
		return FALSE;
	}
}


/*====================
  ModelExporterR8::ModelExporterR8
  ====================*/
ModelExporterR8::ModelExporterR8() :
m_pMaxInterface(NULL),
m_pMaxUtil(NULL),

m_hPanel(NULL),
m_hMeshPanel(NULL),
hStatus(NULL),

m_pOutMDF(NULL),
m_pOutModel(NULL),
m_pOutClip(NULL),

scale_range_low(1.0),
scale_range_high(1.0)
{
	object_name[0] = '\0';
	object_category[0] = '\0';
}


/*====================
  ModelExporterR8::~ModelExporterR8
  ====================*/
ModelExporterR8::~ModelExporterR8()
{
}


/*====================
  ModelExporterR8::Mesh_ApplyCurrentSettings
  ====================*/
void	ModelExporterR8::Mesh_ApplyCurrentSettings()
{
}


/*====================
  ModelExporterR8::UpdateMeshPanel
  ====================*/
void	ModelExporterR8::UpdateMeshPanel()
{
	size_t zCount = m_pMaxInterface->GetSelNodeCount();

	// Clear stuff out
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_NAME, EM_SETREADONLY, TRUE, 0);

	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_EXCLUDENRMLS), FALSE);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_EXCLUDENRMLS, BM_SETCHECK, FALSE, 0);
	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_ROOF), FALSE);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_ROOF, BM_SETCHECK, FALSE, 0);

	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_NORMAL), FALSE);
	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_SURF), FALSE);
	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_BONE), FALSE);
	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_SPRITE), FALSE);
	EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_GROUND), FALSE);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_SURF, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_BONE, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_SPRITE, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_GROUND, BM_SETCHECK, FALSE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_NORMAL, BM_SETCHECK, FALSE, 0);

	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_MATERIAL_NAME, EM_SETREADONLY, TRUE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_MATERIAL_VSH, EM_SETREADONLY, TRUE, 0);
	SendDlgItemMessage(m_hMeshPanel, IDC_MESH_MATERIAL_PSH, EM_SETREADONLY, TRUE, 0);

	if (zCount == 0)
	{
		SetDlgItemText(m_hMeshPanel, IDC_MESH_NAME, "");
	}
	else if (zCount == 1)
	{
		INode *pNode = m_pMaxInterface->GetSelNode(0);

		// Name
		SetDlgItemText(m_hMeshPanel, IDC_MESH_NAME, pNode->GetName());
		SendDlgItemMessage(m_hMeshPanel, IDC_MESH_NAME, EM_SETREADONLY, FALSE, 0);

		// Exclude normals
		BOOL bExNrml = 0;
		pNode->GetUserPropBool("exclude_normals", bExNrml);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_EXCLUDENRMLS), TRUE);
		SendDlgItemMessage(m_hMeshPanel, IDC_MESH_EXCLUDENRMLS, BM_SETCHECK, bExNrml, 0);

		// Roof pieces
		BOOL bRoof = 0;
		pNode->GetUserPropBool("roof", bRoof);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_ROOF), TRUE);
		SendDlgItemMessage(m_hMeshPanel, IDC_MESH_ROOF, BM_SETCHECK, bRoof, 0);

		// Type
		TSTR sType;
		pNode->GetUserPropString("obj_type", sType);

		if (sType == TSTR("surf"))
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_SURF, BM_SETCHECK, TRUE, 0);
		else if (sType == TSTR("bone"))
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_BONE, BM_SETCHECK, TRUE, 0);
		else if (sType == TSTR("sprite"))
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_SPRITE, BM_SETCHECK, TRUE, 0);
		else if (sType == TSTR("ground_plane"))
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_GROUND, BM_SETCHECK, TRUE, 0);
		else
		{
			pNode->SetUserPropString("obj_type", "mesh");
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_R_NORMAL, BM_SETCHECK, TRUE, 0);
		}

		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_NORMAL), TRUE);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_SURF), TRUE);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_BONE), TRUE);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_SPRITE), TRUE);
		EnableWindow(GetDlgItem(m_hMeshPanel, IDC_MESH_R_GROUND), TRUE);

		// Material
		tstring sMatName;

		Mtl	*pBaseMaterial = pNode->GetMtl();
		if (pBaseMaterial &&  pBaseMaterial->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
		{
			StdMat2 *pMaterial = static_cast<StdMat2*>(pBaseMaterial);
			sMatName = pMaterial->GetName();
		}

		SetDlgItemText(m_hMeshPanel, IDC_MESH_MATERIAL_NAME, sMatName.c_str());

		map<tstring, COutMaterial>::iterator mat = m_mapMaterials.find(sMatName);
		if (mat != m_mapMaterials.end())
		{
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_MATERIAL_VSH, EM_SETREADONLY, FALSE, 0);
			SendDlgItemMessage(m_hMeshPanel, IDC_MESH_MATERIAL_PSH, EM_SETREADONLY, FALSE, 0);
			SetDlgItemText(m_hMeshPanel, IDC_MESH_MATERIAL_VSH, mat->second.GetVSH().c_str());
			SetDlgItemText(m_hMeshPanel, IDC_MESH_MATERIAL_PSH, mat->second.GetPSH().c_str());
		}
	}
	else
	{
		SetDlgItemText(m_hMeshPanel, IDC_MESH_NAME, "<-multiple->");
	}
}


/*====================
  ModelExporterR8::Mesh_SetProp
  ====================*/
void	ModelExporterR8::Mesh_SetProp(const tstring &sTag, bool b)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetUserPropBool(sTag.c_str(), b);
}

void	ModelExporterR8::Mesh_SetProp(const tstring &sTag, int i)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetUserPropInt(sTag.c_str(), i);
}

void	ModelExporterR8::Mesh_SetProp(const tstring &sTag, float f)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetUserPropFloat(sTag.c_str(), f);
}

void	ModelExporterR8::Mesh_SetProp(const tstring &sTag, const tstring &s)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetUserPropString(sTag.c_str(), s.c_str());
}

void	ModelExporterR8::Mesh_SetProp(const tstring &sTag, const char *sz)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetUserPropString(sTag.c_str(), sz);
}


/*====================
  ModelExporterR8::Mesh_SetName
  ====================*/
void	ModelExporterR8::Mesh_SetName(const tstring &sName)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	pNode->SetName((TCHAR*)sName.c_str());
	pNode->NotifyDependents(FOREVER, REFMSG_NODE_NAMECHANGE, REFMSG_NODE_NAMECHANGE);
}


/*====================
  ModelExporterR8::Mesh_SetVSHname
  ====================*/
void	ModelExporterR8::Mesh_SetVSHname(const tstring &sName)
{
#if 0
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	tstring sMatName = AddTextureReference(NULL, pNode);
	map<tstring, COutMaterial>::iterator mat = m_mapMaterials.find(sMatName);
	if (mat != m_mapMaterials.end())
		mat->second.SetVSH(sName);
#endif
}


/*====================
  ModelExporterR8::Mesh_SetPSHname
  ====================*/
void	ModelExporterR8::Mesh_SetPSHname(const tstring &sName)
{
#if 0
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	tstring sMatName = AddTextureReference(NULL, pNode);
	map<tstring, COutMaterial>::iterator mat = m_mapMaterials.find(sMatName);
	if (mat != m_mapMaterials.end())
		mat->second.SetPSH(sName);
#endif
}


/*====================
  ModelExporterR8::TagSlection
  ====================*/
void	ModelExporterR8::TagSelection(const tstring &sTag)
{
	INode *pNode = m_pMaxInterface->GetSelNode(0);
	tstring sName = pNode->GetName();
	if (sName.find(sTag) == 0)
		return;

	sName = sTag + sName;
	pNode->SetName((TCHAR*)sName.c_str());
	pNode->SetUserPropBool(sTag.c_str(), true);
	pNode->SetWireColor(RGB(192, 240, 255));

	pNode->NotifyDependents(FOREVER, REFMSG_CHANGE, REFMSG_CHANGE);
	pNode->NotifyDependents(FOREVER, REFMSG_NODE_NAMECHANGE, REFMSG_NODE_NAMECHANGE);
	pNode->NotifyDependents(FOREVER, REFMSG_NODE_WIRECOLOR_CHANGED, REFMSG_NODE_WIRECOLOR_CHANGED);
	m_pMaxInterface->ForceCompleteRedraw();
}


/*====================
  ModelExporterR8::BeginEditParams
  ====================*/
void	ModelExporterR8::BeginEditParams(Interface *pMaxInterface, IUtil *pMaxUtil)
{
	m_pMaxInterface = pMaxInterface;
	m_pMaxUtil = pMaxUtil;

	m_hPanel = pMaxInterface->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_PANEL),
		ModelExporterR8DlgProc, GetString(IDS_PANEL_TITLE), 0);
	m_hMeshPanel = pMaxInterface->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_OBJ_PANEL),
		MeshPanelDlgProc, GetString(IDS_OBJ_PANEL_TITLE), 0);
}


/*====================
  ModelExporterR8::EndEditParams
  ====================*/
void	ModelExporterR8::EndEditParams(Interface *pMaxInterface, IUtil *pMaxUtil)
{
	m_pMaxInterface->DeleteRollupPage(m_hPanel);
	m_pMaxInterface->DeleteRollupPage(m_hMeshPanel);
	m_hPanel = NULL;
	m_hMeshPanel = NULL;

	m_pMaxInterface = NULL;
	m_pMaxUtil = NULL;
}


/*====================
  ModelExporterR8::ObjectSettings
  ====================*/
void	ModelExporterR8::ObjectSettings()
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_OBJECT_SETTINGS),
		m_hPanel, ObjectSettingsDlgProc,(LPARAM)this);
}


/*====================
  ModelExporterR8::GetBoneParentNode
  ====================*/
INode*	ModelExporterR8::GetBoneParentNode(INode *node)
{
	if (node->GetParentNode())
	{
		if (m_bExpOptNoHierarchy)
			return m_pMaxInterface->GetRootNode();
		else
			return node->GetParentNode();
	}

	return NULL;
}


/*====================
  ModelExporterR8::IsNodePhysiqued
  ====================*/
Modifier*	ModelExporterR8::IsNodePhysiqued(INode *pNode)
{
	return FindModifier(pNode, Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));
}


/*====================
  ModelExporterR8::IsNodeSkinned
  ====================*/
Modifier*	ModelExporterR8::IsNodeSkinned(INode *pNode)
{
	return FindModifier(pNode, SKIN_CLASSID);
}


/*====================
  ModelExporterR8::AddMessage
  ====================*/
void	ModelExporterR8::AddMessage(const string &sMessage)
{
	m_sOutput += m_sIndent + sMessage + "\r\n";
}


/*====================
  ModelExporterR8::Error
  ====================*/
void	ModelExporterR8::Error(const string &sMessage)
{
	_snprintf(error_message, ERROR_LOG_LENGTH - 1, "%s%s\r\n", error_message, sMessage.c_str());
	model_error = true;
}


/*====================
  ModelExporterR8::BeginNode
  ====================*/
void	ModelExporterR8::BeginNode(INode *node, ENodeType nodetype)
{
	AddMessage(_TS("-------======= ") + node->GetName() + " ======-------");
	AddMessage(_TS("Type: ") + nodeTypeNames[nodetype]);
	AddMessage("Details:");
	Indent();
}


/*====================
  ModelExporterR8::EndNode
  ====================*/
void	ModelExporterR8::EndNode()
{
	AddMessage("\r\n\r\n");
	Unindent();
}


/*====================
  ScaleBiped

  from PhyExportSample.cpp:
  This function can be used to set the non-uniform scale of a biped.
  The node argument should be a biped node.
  If the scale argument is non-zero the non-uniform scale will be removed from the biped.
  Remove the non-uniform scale before exporting biped nodes and animation data
  If the scale argument is zero the non-uniform scaling will be reapplied to the biped.
  Add the non-uniform scaling back on the biped before exporting skin data
  ====================*/
void ScaleBiped(INode* node, int scale)
{
	if (node->IsRootNode())
		return;

	// Use the class ID to check to see if we have a biped node
	Control* c = node->GetTMController();
    if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
         (c->ClassID() == FOOTPRINT_CLASS_ID))
    {

        // Get the Biped Export Interface from the controller
        IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);

		if (BipIface)
		{
			// Either remove the non-uniform scale from the biped,
			// or add it back in depending on the BOOLean scale value
			BipIface->RemoveNonUniformScale(scale);
			Control* iMaster = (Control *) c->GetInterface(I_MASTER);
			iMaster->NotifyDependents(FOREVER, PART_TM, REFMSG_CHANGE);

			// Release the interfaces
			c->ReleaseInterface(I_MASTER,iMaster);
			c->ReleaseInterface(I_BIPINTERFACE,BipIface);
		}
	}
}


/*====================
  ModelExporterR8::AddPhysiqueBones
  ====================*/
BOOL	ModelExporterR8::AddPhysiqueBones(IPhyContextExport *mcExport, int *num_blended_verts, int *num_nonblended_verts)
{
	INodeTab bones;
	INode* bone;

	SecondaryStatusText("Adding physique-referenced bones");
	SecondaryStatusProgress(0);

	//These are the different types of vertex classes
	IPhyBlendedRigidVertex *rb_vtx;
	IPhyRigidVertex *r_vtx;
	//IPhyFloatingVertex *f_vtx;

	//get the vertex count from the export interface
	int numverts = mcExport->GetNumberVertices();

	*num_blended_verts = 0;
	*num_nonblended_verts = 0;

	//iterate through all vertices and gather the bone list
	for (int i = 0; i < numverts; ++i)
	{
		BOOL exists = FALSE;

		//get the hierarchial vertex interface
		IPhyVertexExport* vi = mcExport->GetVertexInterface(i);
		if (vi)
		{
			//check the vertex type and process accordingly
			int type = vi->GetVertexType();
			switch (type)
			{
				//The vertex is rigid, blended vertex.  It's assigned to multiple links
				case RIGID_BLENDED_TYPE:
					//type-cast the node to the proper class
					rb_vtx = (IPhyBlendedRigidVertex*)vi;

					//iterate through the bones assigned to this vertex
					for (int x = 0; x < rb_vtx->GetNumberNodes(); ++x)
					{
						exists = FALSE;
						//get the node by index
						bone = rb_vtx->GetNode(x);

						//If the bone is a biped bone, scale needs to be
						//restored before exporting skin data
						//ScaleBiped(bone, 0);

						//check to see if we already have this bone
						for (int z = 0; z < bones.Count(); ++z)
							if (bone == bones[z]) exists = TRUE;

						//if you didn't find a match add it to the list
						if (!exists) bones.Append(1, &bone);
					}

					++(*num_blended_verts);

					break;
				//The vertex is a rigid vertex and only assigned to one link
				case RIGID_TYPE:
					//type-cast the node to the proper class
					r_vtx = (IPhyRigidVertex*)vi;

					//get the node
					bone = r_vtx->GetNode();

					//If the bone is a biped bone, scale needs to be
					//restored before exporting skin data
					//ScaleBiped(bone, 0);

					//check to see if the bone is already in the list
					for (int x = 0; x < bones.Count(); ++x)
						if (bone == bones[x]) exists = TRUE;

					//if you didn't find a match add it to the list
					if (!exists) bones.Append(1, &bone);

					++(*num_nonblended_verts);
					break;

				// Shouldn't make it here because we converted to rigid earlier.
				// It should be one of the above two types
				default:
					Error("Vertex type was not RIGID_TYPE or RIGID_BLENDED_TYPE!");
					return FALSE;
			}

			SecondaryStatusProgress(((float)i / numverts)*100.0);
		}
		/*
		// After gathering the bones from the rigid vertex interface
		// gather all floating bones if there are any
		f_vtx = (IPhyFloatingVertex*)mcExport->GetVertexInterface(i, FLOATING_VERTEX);
		if (f_vtx) {	//We have a vertex assigned to a floating bone
			// iterate through the links assigned to this vertex
			for (x = 0; x<f_vtx->GetNumberNodes(); x++)
			{
				exists = FALSE;

				 //get the node by index
				bone = f_vtx->GetNode(x);

				//If the bone is a biped bone, scale needs to be
				//restored before exporting skin data
				ScaleBiped(bone, 0);

				//check to see if we already have this bone
				for (int z=0;z<bones.Count();z++)
					if (bone == bones[z]) exists = TRUE;

				//if you didn't find a match add it to the list
				if (!exists) bones.Append(1, &bone);
			}
		}*/
	}

	for (int i = 0; i < bones.Count(); ++i)
		AddBone(bones[i]);

	return TRUE;
}


/*====================
  ModelExporterR8::AddPhysiquedMesh
  ====================*/
bool	ModelExporterR8::AddPhysiquedMesh(COutMesh *pMesh, INode *pNode, Modifier *pMod)
{
	IPhysiqueExport *phyExport = (IPhysiqueExport *)pMod->GetInterface(I_PHYEXPORT);

	if (!phyExport)
	{
		Error(fmt("Error with physiqued mesh %s", pNode->GetName()));
		return false;
	}

	IPhyContextExport *mcExport = (IPhyContextExport *)phyExport->GetContextInterface(pNode);

	if (!mcExport)
	{
		Error(fmt("Error with physiqued mesh %s (couldn't get context interface)", pNode->GetName()));
		return false;
	}

	int num_blended_verts = 0;
	int num_nonblended_verts = 0;

	mcExport->ConvertToRigid(TRUE);

	if (!AddPhysiqueBones(mcExport, &num_blended_verts, &num_nonblended_verts))
	{
		Error(fmt("Failed to add physique-referenced bones for mesh %s", pMesh->GetName().c_str()));
		return false;
	}

	if (!m_bExpOptGeometry)
		return true;				//if we're only exporting animation we don't need vertex info

	int numverts = mcExport->GetNumberVertices();
	int max_links = 1;

	//allocate space for vertex data
	if (num_blended_verts)
	{
		pMesh->mode = MESH_SKINNED_BLENDED;
		pMesh->blendedLinks = new SBlendedLink[numverts];
		if (!pMesh->blendedLinks)
		{
			Error(fmt("Failed to allocate %i blended verts", numverts));
			return false;
		}
	}
	else	//all verts are nonblended
	{
		pMesh->mode = MESH_SKINNED_NONBLENDED;
		pMesh->singleLinks = new singleLink_t[numverts];
		if (!pMesh->singleLinks)
		{
			Error(fmt("Failed to allocate %i nonblended verts", numverts));
			return false;
		}
	}

	pMesh->bonelink = -1;

	//copy physique weights
	SecondaryStatusText("Copying physique weights");
	SecondaryStatusProgress(0);

	for (int v = 0; v < numverts; ++v)
	{
		IPhyVertexExport *vtxExport = mcExport->GetVertexInterface(v);

		if (vtxExport)
		{
			if (num_blended_verts)
			{
				if (vtxExport->GetVertexType() == RIGID_BLENDED_TYPE)
				{
					// Vertex is attached to multiple bones with different weights
					int numnodes;

					IPhyBlendedRigidVertex *vtxBlend = (IPhyBlendedRigidVertex *)vtxExport;

					numnodes = vtxBlend->GetNumberNodes();
					if (numnodes > max_links)
						max_links = numnodes;

					// Allocate space for this vertex
					pMesh->blendedLinks[v].indexes = new uint[numnodes];
					pMesh->blendedLinks[v].weights = new float[numnodes];
					pMesh->blendedLinks[v].num_weights = numnodes;

					// For each bone...
					for (int n = 0; n < numnodes; ++n)
					{
						INode *Bone = vtxBlend->GetNode(n);
						uint uiIndex(m_pOutModel->GetBoneIndex(Bone->GetName()));
						if (uiIndex != INVALID_BONE)
						{
							// this shouldn't happen
							Error("ERROR: Found a vertex linked to a bone that's not in the bones list!");
							break;
						}

						pMesh->blendedLinks[v].indexes[n] = uiIndex;
						pMesh->blendedLinks[v].weights[n] = vtxBlend->GetWeight(n);
					}

					//contribute to bounding box (this only takes into account the first frame of the animation)
					//M_AddPointToBounds((float *)(BlendP), model.bmin, model.bmax);
				}
				else
				{
					// Vertex is attached to only one bone
					IPhyRigidVertex *vtx = (IPhyRigidVertex *)vtxExport;

					uint uiIndex(m_pOutModel->GetBoneIndex(vtx->GetNode()->GetName()));
					if (uiIndex != INVALID_BONE)
					{
						//this shouldn't happen
						Error("ERROR: Found a vertex linked to a bone that's not in the bones list!");
						break;
					}

					// Allocate space for this vertex
					pMesh->blendedLinks[v].num_weights = 1;
					pMesh->blendedLinks[v].weights = new float[1];
					pMesh->blendedLinks[v].weights[0] = 1.0f;
					pMesh->blendedLinks[v].indexes = new uint[1];
					pMesh->blendedLinks[v].indexes[0] = uiIndex;
				}
			}
			else
			{
				IPhyRigidVertex *vtx = (IPhyRigidVertex *)vtxExport;
				INode *node = vtx->GetNode();

				if (!node)
				{
					Error(fmt("Unknown problem exporting physique data for %s", pMesh->GetName().c_str()));
					break;
				}

				uint uiIndex(m_pOutModel->GetBoneIndex(node->GetName()));
				if (uiIndex != INVALID_BONE)
				{
					//this shouldn't happen
					Error("ERROR: Found a vertex linked to a bone that's not in the bones list!");
					break;
				}

				// allocate space for this vertex
				pMesh->singleLinks[v] = uiIndex;
			}
	   }

	   mcExport->ReleaseVertexInterface(vtxExport);
	   SecondaryStatusProgress(((float)v / numverts) * 100.0);
	}

	AddMessage("Physique info:");
	Indent();
	AddMessage(fmt("Blended vertices: %i", num_blended_verts));
	AddMessage(fmt("Nonblended vertices: %i", num_nonblended_verts));
	AddMessage(fmt("Max links: %i", max_links));
	Unindent();

	phyExport->ReleaseContextInterface(mcExport);
	pMod->ReleaseInterface(I_PHYINTERFACE, phyExport);

	++m_iPhysiquedMeshes;
	return true;
}


/*====================
  ModelExporterR8::AddSkinBones
  ====================*/
bool	ModelExporterR8::AddSkinBones(ISkin *pSkinInterface, ISkinContextData *pSkinCD, int *piNumBlended, int *piNumNonBlended)
{
	SecondaryStatusText("Adding skin-referenced bones");
	SecondaryStatusProgress(0);

	// Get the vertex count from the export interface
	int numverts = pSkinCD->GetNumPoints();

	// Reset vertex counts
	*piNumBlended = 0;
	*piNumNonBlended = 0;

	// Bone list
	set<int>	setBones;

	// Step through each vertex of the mesh
	AddMessage(fmt("Skinned mesh with %i verts:", numverts));
	for (int i = 0; i < numverts; ++i)
	{
		// Step through each bone associated with this vert
		int numbones = pSkinCD->GetNumAssignedBones(i);
		//AddMessage(fmt("Vert #%i, %i bones:", i, numbones));
		Indent();

		if (numbones >= 2)
			++(*piNumBlended);
		else if (numbones == 1)
			++(*piNumNonBlended);

		for (int b = 0; b < numbones; ++b)
		{
			int boneid = pSkinCD->GetAssignedBone(i, b);
			setBones.insert(boneid);
			//AddMessage(fmt("Bone #%i, '%s': %f", b, pSkinInterface->GetBone(boneid)->GetName(), weight));
		}
		Unindent();

		SecondaryStatusProgress((i / (float)numverts) * 100);
	}

	for (set<int>::iterator it = setBones.begin(); it != setBones.end(); ++it)
	{
		AddBone(pSkinInterface->GetBone(*it));
		//AddMessage(fmt("Added bone #%i, %s", zIndex, pSkinInterface->GetBone(*it)->GetName()));
	}
	return true;
}


/*====================
  ModelExporterR8::AddSkinnedMesh
  ====================*/
bool	ModelExporterR8::AddSkinnedMesh(COutMesh *pMesh, INode *pNode, Modifier *pMod)
{
	try
	{
		ISkin *pSkinInterface = (ISkin*)pMod->GetInterface(I_SKIN);
		if (pSkinInterface == NULL)
			throw _T("Could not get Skin interface");

		ISkinContextData *pSkinCD = pSkinInterface->GetContextInterface(pNode);
		if (pSkinCD == NULL)
			throw _T("Could not get Skin context data");

		int num_blended_verts = 0;
		int num_nonblended_verts = 0;

		if (!AddSkinBones(pSkinInterface, pSkinCD, &num_blended_verts, &num_nonblended_verts))
			throw _T("Error adding bones");

		if (!m_bExpOptGeometry)
			return true;				//if we're only exporting animation we don't need vertex info

		int numverts = pSkinCD->GetNumPoints();
		int max_links = 1;

		// Allocate space for vertex data
		if (num_blended_verts)
		{
			pMesh->mode = MESH_SKINNED_BLENDED;
			pMesh->blendedLinks = new SBlendedLink[numverts];
			if (pMesh->blendedLinks == NULL)
			{
				Error(fmt("Failed to allocate %i blended verts", numverts));
				return false;
			}
		}
		else	//all verts are nonblended
		{
			pMesh->mode = MESH_SKINNED_NONBLENDED;
			pMesh->singleLinks = new singleLink_t[numverts];
			if (pMesh->singleLinks == NULL)
				throw _T(fmt("Failed to allocate %i nonblended verts", numverts));
		}

		pMesh->bonelink = -1;

		// Copy skin weights
		SecondaryStatusText("Copying skin weights");
		SecondaryStatusProgress(0);

		for (int v = 0; v < numverts; ++v)
		{
			int numbones = pSkinCD->GetNumAssignedBones(v);
			if (numbones > max_links)
				max_links = numbones;

			if (numbones == 0)
				Error("Unlinked vertex");

			pMesh->blendedLinks[v].indexes = new uint[numbones];
			pMesh->blendedLinks[v].weights = new float[numbones];
			pMesh->blendedLinks[v].num_weights = 0;

			for (int b = 0; b < numbones; ++b)
			{
				const char *szBoneName = pSkinInterface->GetBone(pSkinCD->GetAssignedBone(v, b))->GetName();
				uint uiIndex = m_pOutModel->GetBoneIndex(szBoneName);
				if (uiIndex == INVALID_BONE)
					throw _T(fmt("Missing bone '%s'", szBoneName));

				float fWeight = pSkinCD->GetBoneWeight(v, b);
				if (fWeight < 0.01f)
					continue;

				pMesh->blendedLinks[v].indexes[pMesh->blendedLinks[v].num_weights] = uiIndex;
				pMesh->blendedLinks[v].weights[pMesh->blendedLinks[v].num_weights] = pSkinCD->GetBoneWeight(v, b);
				++pMesh->blendedLinks[v].num_weights;
			}

			int iNumWeights(pMesh->blendedLinks[v].num_weights);
			for (int b(0); b < iNumWeights; ++b)
			{
				int iMax(b);

				for (int n(b + 1); n < iNumWeights; ++n)
				{
					if (pMesh->blendedLinks[v].weights[n] > pMesh->blendedLinks[v].weights[iMax])
						iMax = n;
				}

				if (iMax != b)
				{
					SWAP(pMesh->blendedLinks[v].weights[b], pMesh->blendedLinks[v].weights[iMax]);
					SWAP(pMesh->blendedLinks[v].indexes[b], pMesh->blendedLinks[v].indexes[iMax]);
				}
			}

			SecondaryStatusProgress((v / (float)numverts) * 100);
		}

		AddMessage("Skin info:");
		Indent();
		AddMessage(fmt("Blended vertices: %i", num_blended_verts));
		AddMessage(fmt("Nonblended vertices: %i", num_nonblended_verts));
		AddMessage(fmt("Max links: %i", max_links));
		Unindent();

		// I guess this is not necessary?
		//pSkinInterface->ReleaseContextInterface(pSkinCD);
		pMod->ReleaseInterface(I_SKIN, pSkinInterface);
	}
	catch (const string &sReason)
	{
		Error(_T("AddSkinnedMesh(): ") + sReason);
		return false;
	}

	++m_iSkinnedMeshes;
	return true;
}


/*====================
  ModelExporterR8::AddSprite
  ====================*/
void	ModelExporterR8::AddSprite(INode *node, int spritetype)
{
	Object *obj;

	obj = node->GetObjectRef();
	if (!obj)
		return;

	if (obj->ClassID() != Class_ID(BOXOBJ_CLASS_ID, 0))
	{
		Error(fmt("%s must be a BOX object to export (use length and width)", node->GetName()));
		return;
	}

	IParamArray *pa = obj->GetParamBlock();
	if (pa == NULL)
		return;

	Interval valid;
	float width, height;

	pa->GetValue(obj->GetParamBlockIndex(BOXOBJ_WIDTH), 0, width, valid);
	pa->GetValue(obj->GetParamBlockIndex(BOXOBJ_LENGTH), 0, height, valid);

	BeginNode(node, NODETYPE_SPRITE);
	AddMessage(fmt("Width = %f, Height = %f, Type = %i", width, height, spritetype));
	EndNode();
	spritecount++;
}


/*====================
  ModelExporterR8::GenerateVertexData

  Generates a set of 3 vertices for each face and fills in their information.
  Duplicate vertices will be culled out later.
  ====================*/
bool	ModelExporterR8::GenerateVertexData(COutMesh *pMesh, INode *pNode, vector< vector<size_t> > &comparisonList)
{
	int deleteIt;
	TriObject *triobj = GetTriObject(m_pMaxInterface, FRAME(GetStartFrame()), pNode, deleteIt);
	if (!triobj)
	{
		Error("GenerateVertexData: Failed to retrieve TriObject");
		return false;
	}

	Matrix3 tm = pNode->GetObjectTM(FRAME(GetStartFrame()));
	comparisonList = CreateFaceAndVertexData(&triobj->mesh, pMesh, pNode, tm);

	if (deleteIt)
		triobj->DeleteMe();

	return true;
}


/*====================
  ModelExporterR8::AddTextureReference
  ====================*/
tstring	ModelExporterR8::AddTextureReference(COutMesh *pMesh, INode *pNode)
{
	// Get the material
	COutMaterial outmat(pMesh, pNode, GetStartFrame());
	m_mapMaterials[outmat.GetName()] = outmat;

	if (pMesh != NULL)
		pMesh->SetDefaultShaderName(outmat.GetName());

	return outmat.GetName();
}


/*====================
  ModelExporterR8::FinishMesh
  ====================*/
bool	ModelExporterR8::FinishMesh(COutMesh *pMesh, INode *pNode)
{
	if (!m_bExpOptGeometry)
		return true;

	vector< vector<size_t> > comparisonList;
	try
	{
		if (!GenerateVertexData(pMesh, pNode, comparisonList))
			throw "FinishMesh: failed to generate vertex data";
	}
	catch (const char *szMsg)
	{
		Error(szMsg);
		return false;
	}

	// Adjust the model's bounds to contain this mesh's bounds
	m_pOutModel->AddBounds(pMesh->bbBounds);

	// Check for redundant vertices and remap faces to exclude them
	map<int, int> mapVerts;
	SecondaryStatusText(fmt("Comparing %i vertices", pMesh->num_verts));
	SecondaryStatusProgress(0);
	int count = 0;
	float total = pMesh->num_verts;
	for (int l = 0; l < comparisonList.size(); ++l)
	{
		for (int v = 0; v < comparisonList[l].size(); ++v)
		{
			++count;
			for (int w = v + 1; w < comparisonList[l].size(); ++w)
			{
				bool bMatch = true;

				// Check normals
				if (pMesh->normals != NULL)
				{
					if (memcmp(pMesh->normals[comparisonList[l][v]], pMesh->normals[comparisonList[l][w]], sizeof(vec3_t)) != 0)
						continue;
				}

				// Check mapping coordinates
				for (int c = 0; c < MAX_UV_CHANNELS; ++c)
				{
					if (pMesh->tverts[c] == NULL)
						continue;

					if (memcmp(pMesh->tverts[c][comparisonList[l][v]], pMesh->tverts[c][comparisonList[l][w]], sizeof(vec2_t)) != 0)
					{
						bMatch = false;
						break;
					}
				}
				if (!bMatch)
					continue;

				// Check tangents
				for (int c = 0; c < MAX_UV_CHANNELS; ++c)
				{
					if (pMesh->tangents[c] == NULL)
						continue;

					if (!Compare(pMesh->tangents[c][comparisonList[l][v]], pMesh->tangents[c][comparisonList[l][w]], 0.0001f))
					{
						bMatch = false;
						break;
					}
				}
				if (!bMatch)
					continue;

				// Check signs
				for (int c = 0; c < MAX_UV_CHANNELS; ++c)
				{
					if (pMesh->signs[c] == NULL)
						continue;

					if (!pMesh->signs[c][comparisonList[l][v]] != pMesh->signs[c][comparisonList[l][w]])
					{
						bMatch = false;
						break;
					}
				}
				if (!bMatch)
					continue;


				// Check colors
				for (int c = 0; c < MAX_VERTEX_COLOR_CHANNELS; ++c)
				{
					if (pMesh->colors[c] == NULL)
						continue;

					if (memcmp(pMesh->colors[c][comparisonList[l][v]], pMesh->colors[c][comparisonList[l][w]], sizeof(bvec4_t)) != 0)
					{
						bMatch = false;
						break;
					}
				}
				if (!bMatch)
					continue;

				// Add to remapping list
				mapVerts[comparisonList[l][w]] = comparisonList[l][v];

				SecondaryStatusProgress((count / total) * 100);
			}
		}
	}

	AddMessage(fmt("Removed %i duplicate vertices.", mapVerts.size()));

	// Finish the map, accounting for a condensed vert list
	count = 0;
	for (int v = 0; v < pMesh->num_verts; ++v)
	{
		if (mapVerts.find(v) != mapVerts.end())
		{
			mapVerts[v] = mapVerts[mapVerts[v]];
			continue;
		}

		mapVerts[v] = count;
		if (v != count)
		{
			memcpy(pMesh->verts[count], pMesh->verts[v], sizeof(vec3_t));

			if (pMesh->normals != NULL)
				memcpy(pMesh->normals[count], pMesh->normals[v], sizeof(vec3_t));

			for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
			{
				if (pMesh->colors[n] == NULL)
					continue;

				memcpy(pMesh->colors[n][count], pMesh->colors[n][v], sizeof(bvec4_t));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (pMesh->tverts[n] == NULL)
					continue;

				memcpy(pMesh->tverts[n][count], pMesh->tverts[n][v], sizeof(vec2_t));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (pMesh->tangents[n] == NULL)
					continue;

				memcpy(pMesh->tangents[n][count], pMesh->tangents[n][v], sizeof(vec3_t));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (pMesh->signs[n] == NULL)
					continue;

				pMesh->signs[n][count] =pMesh->signs[n][v];
			}

			switch(pMesh->mode)
			{
			//case MESH_KEYFRAMED:

			case MESH_SKINNED_BLENDED:
				pMesh->blendedLinks[count] = pMesh->blendedLinks[v];
				break;

			case MESH_SKINNED_NONBLENDED:
				if (pMesh->bonelink == -1)
					pMesh->singleLinks[count] = pMesh->singleLinks[v];
				break;
			}
		}
		++count;
	}
	pMesh->num_verts = count;

	// Fix the facelist
	for (int f = 0; f < pMesh->numFaces; ++f)
	{
		for (int n = 0; n < 3; ++n)
			pMesh->faceList[f][n] = mapVerts[pMesh->faceList[f][n]];
	}

	// Optimize facelist using NvTriStrip
	SetCacheSize(CACHESIZE_GEFORCE3);
	SetStitchStrips(true);
	SetMinStripSize(0);
	SetListsOnly(true);

	unsigned short	usNumSections;
	PrimitiveGroup	*pPrimitiveGroupsList;

	GenerateStrips((uint *)pMesh->faceList, pMesh->numFaces * 3, &pPrimitiveGroupsList, &usNumSections);

	unsigned int	*pRemappedIndices;

	RemapIndices(pPrimitiveGroupsList, usNumSections, pMesh->numFaces * 3, &pRemappedIndices);

	// Reorder primitive groups
	for (int g(0); g < usNumSections; ++g)
		for (int i(0); i < pPrimitiveGroupsList[g].numIndices; ++i)
			pPrimitiveGroupsList[g].indices[i] = pRemappedIndices[pPrimitiveGroupsList[g].indices[i]];

	// Reorder verticies
	vec3_t *verts(new vec3_t[pMesh->num_verts]);
	memcpy(verts, pMesh->verts, sizeof(vec3_t) * pMesh->num_verts);

	vec3_t *normals(NULL);
	if (pMesh->normals)
	{
		normals = new vec3_t[pMesh->num_verts];
		memcpy(normals, pMesh->normals, sizeof(vec3_t) * pMesh->num_verts);
	}

	bvec4_t *colors[MAX_VERTEX_COLOR_CHANNELS];
	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		if (pMesh->colors[n])
		{
			colors[n] = new bvec4_t[pMesh->num_verts];
			memcpy(colors[n], pMesh->colors[n], sizeof(bvec4_t) * pMesh->num_verts);
		}
		else
			colors[n] = NULL;
	}

	vec2_t *tverts[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->tverts[n])
		{
			tverts[n] = new vec2_t[pMesh->num_verts];
			memcpy(tverts[n], pMesh->tverts[n], sizeof(vec2_t) * pMesh->num_verts);
		}
		else
			tverts[n] = NULL;
	}

	vec3_t *tangents[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->tangents[n])
		{
			tangents[n] = new vec3_t[pMesh->num_verts];
			memcpy(tangents[n], pMesh->tangents[n], sizeof(vec3_t) * pMesh->num_verts);
		}
		else
			tangents[n] = NULL;
	}

	byte *signs[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->signs[n])
		{
			signs[n] = new byte[pMesh->num_verts];
			memcpy(signs[n], pMesh->signs[n], pMesh->num_verts);
		}
		else
			signs[n] = NULL;
	}

	SBlendedLink *blendedLinks(NULL);
	if (pMesh->mode == MESH_SKINNED_BLENDED && pMesh->blendedLinks)
	{
		blendedLinks = new SBlendedLink[pMesh->num_verts];
		memcpy(blendedLinks, pMesh->blendedLinks, sizeof(SBlendedLink) * pMesh->num_verts);
	}

	singleLink_t *singleLinks(NULL);
	if (pMesh->mode == MESH_SKINNED_NONBLENDED && pMesh->singleLinks)
	{
		singleLinks = new singleLink_t[pMesh->num_verts];
		memcpy(singleLinks, pMesh->singleLinks, sizeof(singleLinks) * pMesh->num_verts);
	}

	for (int v = 0; v < pMesh->num_verts; ++v)
	{
		unsigned int v2(pRemappedIndices[v]);

		memcpy(pMesh->verts[v2], verts[v], sizeof(vec3_t));

		if (pMesh->normals)
			memcpy(pMesh->normals[v2], normals[v], sizeof(vec3_t));

		for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
		{
			if (pMesh->colors[n])
				memcpy(pMesh->colors[n][v2], colors[n][v], sizeof(bvec4_t));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (pMesh->tverts[n])
				memcpy(pMesh->tverts[n][v2], tverts[n][v], sizeof(vec2_t));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (pMesh->tangents[n])
				memcpy(pMesh->tangents[n][v2], tangents[n][v], sizeof(vec3_t));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (pMesh->signs[n])
				pMesh->signs[n][v2] = signs[n][v];
		}

		if (pMesh->blendedLinks)
			pMesh->blendedLinks[v2] = blendedLinks[v];

		if (pMesh->singleLinks)
			pMesh->singleLinks[v2] = singleLinks[v];
	}

	memcpy(pMesh->faceList, pPrimitiveGroupsList[0].indices, sizeof(uint) * pPrimitiveGroupsList[0].numIndices);

	delete[] pPrimitiveGroupsList;
	delete[] pRemappedIndices;

	SAFE_DELETE_ARRAY(normals);

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
		SAFE_DELETE_ARRAY(colors[n]);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		SAFE_DELETE_ARRAY(tverts[n]);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		SAFE_DELETE_ARRAY(tangents[n]);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		SAFE_DELETE_ARRAY(signs[n]);

	SAFE_DELETE_ARRAY(blendedLinks);
	SAFE_DELETE_ARRAY(singleLinks);

	AddMessage("");
	AddMessage(fmt("Mesh has %i vertices and %i faces", pMesh->num_verts, pMesh->numFaces));
	return true;
}


/*====================
  ModelExporterR8::AddBone
  ====================*/
uint	ModelExporterR8::AddBone(INode *pNode)
{
	// Do a name check to make sure this bone doesn't already exist
	const COutBone *pBone = m_pOutModel->GetBone(pNode->GetName());
	if (pBone != NULL)
		return pBone->GetIndex();

	// Initialize the new bone
	uint	uiNewIndex(m_pOutModel->GetNumBones());
	COutBone	newBone(uiNewIndex, pNode->GetName(), INVALID_BONE);

	// Set up the bone transform matrices
	Matrix3 baseTM = GetBoneTM(pNode, FRAME(GetStartFrame()));
	CopyMatrix3(baseTM, newBone.m_base);
	Matrix3 invBaseTM = baseTM;
	invBaseTM.Invert();
	CopyMatrix3(invBaseTM, newBone.m_invBase);

	// Add the bone index to the clip in case we're exporting animation
	if (m_pOutClip)
		m_pOutClip->AddBone(newBone);

	// Keep track of which node each bone belongs to
	if (m_vpBoneNodes.size() < uiNewIndex + 1)
		m_vpBoneNodes.resize(uiNewIndex + 1, NULL);

	m_vpBoneNodes[uiNewIndex] = pNode;

	// Add the bone to the model
	m_pOutModel->AddBone(newBone);

	// Get parent information
	// Keep track of which node each bone belongs to
	if (m_vpBoneParentNodes.size() < uiNewIndex + 1)
		m_vpBoneParentNodes.resize(uiNewIndex + 1, NULL);

	INode *pParentNode(GetBoneParentNode(pNode));
	if (pParentNode)
	{
		m_vpBoneParentNodes[uiNewIndex] = pParentNode;
		AddMessage(fmt("Added bone \"%s\" (index %i) to model (parent: %s)", pNode->GetName(), uiNewIndex, pParentNode->GetName()));

		// Ensure all parent bones of this bone are added
		AddBone(pParentNode);
	}
	else
	{
		AddMessage(fmt("Added bone \"%s\" (index %i) to model (no parent)", pNode->GetName(), uiNewIndex));
	}

	return uiNewIndex;
}


/*====================
  ModelExporterR8::AddStaticMesh

  add a rigid mesh (a mesh that may transform but the vertices remain static)
  ====================*/
bool	ModelExporterR8::AddStaticMesh(COutMesh *pMesh, INode *pNode)
{
	// Make a bone which represents this mesh's transform
	size_t zBoneIndex = AddBone(pNode);
	if (zBoneIndex == INVALID_BONE)
	{
		Error("AddStaticMesh: failed to add bone");
		return false;
	}

	pMesh->bonelink = zBoneIndex;
	pMesh->mode = MESH_SKINNED_NONBLENDED;
	return true;
}


/*====================
  ModelExporterR8::AddKeyframedMesh
  ====================*/
bool	ModelExporterR8::AddKeyframedMesh(COutMesh *pMesh, INode *pNode)
{
	Error("Keyframes not currently supported");
	return false;
}


/*====================
  ModelExporterR8::AddKeyframedMesh

  add keyframed vertex info to a mesh
  ====================*/
/*
bool	ModelExporterR8::AddKeyframedMesh(COutMesh *s2mesh)
{
	int numframes = GetNumFrames();
	BOOL firstloop = TRUE;

	s2mesh->keyframes = (keyframe_t *)malloc(sizeof(keyframe_t) * numframes);
	if (!s2mesh->keyframes)
		return false;

	SecondaryStatusText("Adding keyframed mesh");

	int endframe = GetEndFrame();
	int startframe = GetStartFrame();
	for (int frame = startframe; frame < endframe; frame++)
	{
		int deleteIt;
		keyframe_t *keyframe = &s2mesh->keyframes[frame-startframe];
		TriObject *triobj = GetTriObject(m_pMaxInterface, FRAME(frame), node, deleteIt);

		if (!triobj)
			return false;

		SecondaryStatusProgress(((float)(frame - startframe) / GetNumFrames()) * 100.0);

		Mesh *mesh = &triobj->mesh;

		if (firstloop)
		{
			s2mesh->num_verts = mesh->numVerts;
			firstloop = FALSE;
		}

		keyframe->key_verts = (keyVert_t *)malloc(sizeof(keyVert_t) * s2mesh->num_verts);
		if (!keyframe->key_verts)
			return FALSE;

		vec3_t bmin, bmax;
		ComputeBounds(mesh, bmin, bmax);
		M_CalcBoxExtents(bmin, bmax, keyframe->bpos, keyframe->bext);

		for (int v=0; v<s2mesh->num_verts; v++)
		{
			Point3 vert;
			int compressed[3];

			//compress to -0.5 to 0.5 range
			vert.x = (mesh->verts[v].x - keyframe->bpos[0]) / keyframe->bext[0];
			vert.y = (mesh->verts[v].y - keyframe->bpos[1]) / keyframe->bext[1];
			vert.z = (mesh->verts[v].z - keyframe->bpos[2]) / keyframe->bext[2];

			//transform to model space
			vert = vert * node->GetObjectTM(FRAME(frame));

			//while we're at it, add to the model bounds
			M_AddPointToBounds((float *)(vert), model.bmin, model.bmax);


			compressed[0] = (char)(vert.x * 255);
			compressed[1] = (char)(vert.y * 255);
			compressed[2] = (char)(vert.z * 255);
			for (int i=0; i<3; i++)
			{
				if (compressed[i] > 127)
					compressed[i] = 127;
				else if (compressed[i] < -128)
					compressed[i] = -128;
			}

			keyframe->key_verts[v][0] = (char)(compressed[0]);
			keyframe->key_verts[v][1] = (char)(compressed[1]);
			keyframe->key_verts[v][2] = (char)(compressed[2]);
		}

		if (deleteIt)
			triobj->DeleteMe();
	}

	AddMessage(fmt("Copied %i keyframes", numframes));

	return true;
}/**/


/*====================
  ModelExporterR8::AddAnimatingMesh

  this is the general animated but non-physiqued mesh function
  first we must determine if this mesh has deforming vertices or not
  then we call the appropriate function
  ====================*/
#if 0
BOOL	ModelExporterR8::AddAnimatingMesh(COutMesh *s2mesh, BOOL checkDeform)
{
	int frame;
	int deleteIt;
	int numframes = GetNumFrames();
	BOOL deformed = FALSE;
	TriObject *tri1, *tri2;
	Mesh copy;
	int endframe = GetEndFrame();

	//figured i could test the validity interval to determine if the object was deforming or not
	//but this doesn't always seem to work (what a surprise)...right after a file open the validity does not
	//seem to get set to FOREVER, even if it should be...if i move the frame slider a little bit
	//in max and test it again, i get the correct validity interval of FOREVER.  screw it.

	SecondaryStatusText("Determining if mesh deforms");

	if (GetNumModifiers(node) && checkDeform)
	{
		for (frame = GetStartFrame()+1; frame <= endframe; frame++)
		{
			int v;

			SecondaryStatusProgress(((float)(frame - GetStartFrame()) / GetNumFrames()) * 100.0);

			tri1 = GetTriObject(m_pMaxInterface, FRAME(frame-1), node, deleteIt);
			if (!tri1)
				return FALSE;

			copy = tri1->mesh;

			if (deleteIt)
				tri1->DeleteMe();

			tri2 = GetTriObject(m_pMaxInterface, FRAME(frame), node, deleteIt);
			if (!tri2)
				return FALSE;

			/*if (tri1 == tri2)
			{
				if (delete1)
					tri1->DeleteMe();
				continue;
			}*/

			//compare the vertices of the two meshes
			if (tri2->mesh.numVerts != copy.numVerts)
			{
				if (deleteIt)
					tri2->DeleteMe();
				Error("Mesh did not keep the same number of verts");
				return FALSE;
			}

			for (v=0; v<tri2->mesh.numVerts; v++)
			{
				if (copy.verts[v] != tri2->mesh.verts[v])
				{
					deformed = TRUE;
					break;
				}
			}

			if (deleteIt)
				tri2->DeleteMe();

			if (deformed)
				break;
		}
	}

	if (deformed)
	{
		AddMessage("Mesh vertices are animated, mesh mode is KEYFRAMED");
		return AddKeyframedMesh(s2mesh);
	}
	else
	{
		AddMessage("Mesh vertices are not animated, mesh mode is RIGID");
		return AddRigidMesh(s2mesh);
	}
}
#endif


/*====================
  ModelExporterR8::AddMesh
  ====================*/
void	ModelExporterR8::AddMesh(INode *pNode)
{
	try
	{
		BeginNode(pNode, NODETYPE_MESH);

		COutMesh *pNewMesh = new COutMesh;
		if (pNewMesh == NULL)
			throw "AddMesh(): Failed allocation";

		pNewMesh->SetName(pNode->GetName());

		if (m_bExpOptForceStatic)
		{
			if (!AddStaticMesh(pNewMesh, pNode))
				throw "AddStaticMesh() failed";
		}
		else
		{
			Modifier *pPhysMod = IsNodePhysiqued(pNode);
			Modifier *pSkinMod = IsNodeSkinned(pNode);

			if (pSkinMod != NULL)
			{
				if(!AddSkinnedMesh(pNewMesh, pNode, pSkinMod))
					throw "AddSkinnedMesh() failed";
			}
			else if (pPhysMod != NULL)
			{
				if (!AddPhysiquedMesh(pNewMesh, pNode, pPhysMod))
					throw "AddPhysiquedMesh() failed";
			}
			else
			{
				if (!AddStaticMesh(pNewMesh, pNode))
					throw "AddStaticMesh() failed";
			}
		}

		if (!FinishMesh(pNewMesh, pNode))
			throw "FinishMesh() failed";

		// Mesh data stored successfully, add this mesh to the model
		AddTextureReference(pNewMesh, pNode);
		m_pOutModel->AddMesh(*pNewMesh);

		EndNode();
	}
	catch (const char *szMsg)
	{
		Error(szMsg);
		AddMessage("There was an error with this mesh");
		EndNode();
	}
}


/*====================
  ModelExporterR8::AddReferenceBone
  ====================*/
void	ModelExporterR8::AddReferenceBone(INode *node)
{
	BeginNode(node, NODETYPE_REFERENCE_BONE);

	AddBone(node);

	EndNode();
}


/*====================
  ModelExporterR8::AddSurf

  add a collision surface
  ====================*/
void	ModelExporterR8::AddSurf(INode *node)
{
	int deleteIt;
	TriObject *triobj;
	CConvexPolyhedron surf;
	CBBoxf bbBounds;

	BeginNode(node, NODETYPE_SURFACE);

	triobj = GetTriObject(m_pMaxInterface, FRAME(GetStartFrame()), node, deleteIt);

	if (!triobj)
	{
		Error("Couldn't get TriObject");
		return;
	}

	if (!TestMeshConvexity(triobj->mesh))
	{
		Error("The surface is not convex");
		return;
	}

	BOOL negParity = FALSE;
	Matrix3 tm = node->GetObjectTM(FRAME(GetStartFrame()));
	if (tm.Parity())
	{
		negParity = TRUE;
		AddMessage("Using reversed winding");
	}

	Mesh *mesh = &triobj->mesh;

	AddMessage(fmt("Generating collision surface from a mesh with %i faces", mesh->numFaces));

	for (int face = 0; face < mesh->numFaces; face++)
	{
		CVec3f v0, v1, v2;

		if (negParity)
		{
			v0 = CVec3f((float *)FACEVERTEX(mesh, face, 2) * tm);
			v1 = CVec3f((float *)FACEVERTEX(mesh, face, 1) * tm);
			v2 = CVec3f((float *)FACEVERTEX(mesh, face, 0) * tm);
		}
		else
		{
			v0 = CVec3f((float *)FACEVERTEX(mesh, face, 0) * tm);
			v1 = CVec3f((float *)FACEVERTEX(mesh, face, 1) * tm);
			v2 = CVec3f((float *)FACEVERTEX(mesh, face, 2) * tm);
		}

		// Add this face to the surface
		surf.AddFace(v0, v1, v2);
	}

	if (surf.GetNumPlanes() < 4)
	{
		Error(fmt("Not enough planes in surface (numplanes == %i)", surf.GetNumPlanes()));
		goto finishAddSurf;
	}

	// work out bounding box
	ComputeBounds(&triobj->mesh, &tm, bbBounds);
	surf.SetBounds(bbBounds);

	// Add the surface to the model
	m_pOutModel->AddSurf(surf);

	AddMessage(fmt("Surface contains %i planes", surf.GetNumPlanes()));
	AddMessage(fmt("Bounds: (%f, %f, %f)  to  (%f, %f, %f)",
		surf.GetBounds().GetMin().x, surf.GetBounds().GetMin().y, surf.GetBounds().GetMin().z,
		surf.GetBounds().GetMax().x, surf.GetBounds().GetMax().y, surf.GetBounds().GetMax().z)
	);

finishAddSurf:

	if (deleteIt)
		triobj->DeleteMe();

	EndNode();
}


/*====================
  ModelExporterR8::TraverseNode
  ====================*/
void	ModelExporterR8::TraverseNode(INode *pNode)
{
	if (!pNode->IsHidden())		//ignore this node if hidden
	{
		Status(fmt("Processing %s...", pNode->GetName()), ((float)n_zCurrentNode / m_zNumNodes) * 100);

		// Determine node type
		/*if (strncmp(pNode->GetName(), "_billboard", 10)==0)
			AddSprite(pNode, S2SPRITE_BILLBOARD);
		else if (strncmp(pNode->GetName(), "_groundplane", 11)==0)
			AddSprite(pNode, S2SPRITE_GROUNDPLANE);
		else */if (strncmp(pNode->GetName(), "_surf", 5)==0)
			AddSurf(pNode);
		else if (strncmp(pNode->GetName(), "_bone", 5)==0)
			AddReferenceBone(pNode);
		else if (IsMesh(pNode, m_pMaxInterface))
			AddMesh(pNode);

		++n_zCurrentNode;
	}

	for (int n = 0; n < pNode->NumberOfChildren(); ++n)
		TraverseNode(pNode->GetChildNode(n));
}


/*====================
  ModelExporterR8::CountNodes
  ====================*/
void	ModelExporterR8::CountNodes(INode *pNode)
{
	if (!pNode->IsHidden())
		++m_zNumNodes;

	for (int n = 0; n < pNode->NumberOfChildren(); ++n)
		CountNodes(pNode->GetChildNode(n));
}


/*====================
  ModelExporterR8::ClearSceneInfo
  ====================*/
void	ModelExporterR8::ClearSceneInfo()
{
	// Clear working variables
	ClearIndent();
	m_sOutput.clear();

	m_iPhysiquedMeshes = 0;
	m_iSkinnedMeshes = 0;
	spritecount = 0;
	keyframed_mesh_count = 0;
	num_meshes = 0;
	error_message[0] = '\0';
	model_error = false;

	// Panel options
	m_bExpOptForceStatic = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_FORCE_STATIC);
	m_bExpOptNoHierarchy = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_REMOVE_HIERARCHY);
	m_bExpOptCopyTextures = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_COPY_TEXTURES);
	m_bExpOptGeometry = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_GEOMETRY);
	m_bExpOptMaterials = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_MATERIALS);
	m_bExpOptMDF = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_MDF);
	m_bExpOptAnimation = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_ANIMATION);

	export_clip = 0;
	export_objdef = 0;

	m_vpBoneNodes.clear();
	m_mapMaterials.clear();

	// Clear working mdf
	if (m_pOutMDF != NULL)
	{
		delete m_pOutMDF;
	}
	m_pOutMDF = new COutMDF;

	// Clear the working model
	if (m_pOutModel != NULL)
	{
		for (uint ui(0); ui < m_pOutModel->GetNumMeshes(); ++ui)
			m_pOutModel->GetMesh(ui)->Free();
		delete m_pOutModel;
	}
	m_pOutModel = new COutModel;

	// Clear theworking clip
	if (m_pOutClip != NULL)
	{
		delete m_pOutClip;
	}
	m_pOutClip = new COutClip;

	// Count active nodes in the scene
	n_zCurrentNode = 0;
	m_zNumNodes = 0;
	CountNodes(m_pMaxInterface->GetRootNode());

	// Add origin bone
	AddBone(m_pMaxInterface->GetRootNode());
}


/*====================
  ModelExporterR8::OpenStatusBar
  ====================*/
void	ModelExporterR8::OpenStatusBar()
{
	if (hStatus)
		return;

	hStatus = CreateDialog(
		hInstance,
		MAKEINTRESOURCE(IDD_STATUS_BAR),
		m_hPanel,
		StatusBarDlgProc);
	ShowWindow(hStatus, SW_SHOWNORMAL);
}


/*====================
  ModelExporterR8::Status
  ====================*/
void	ModelExporterR8::Status(const char *msg, int progress)
{
	if (!hStatus)
		return;

	SetDlgItemText(hStatus, IDC_STATUS_MESSAGE, (LPCSTR)msg);
	SendDlgItemMessage(hStatus, IDC_PROGRESS, PBM_SETPOS, (WPARAM)progress, 0);
	SetDlgItemText(hStatus, IDC_SECONDARY_STATUS_MESSAGE, "");
	SendDlgItemMessage(hStatus, IDC_PROGRESS2, PBM_SETPOS, 0, 0);
}


/*====================
  ModelExporterR8::SecondaryStatusText
  ====================*/
void	ModelExporterR8::SecondaryStatusText(const char *msg)
{
	if (!hStatus)
		return;

	SetDlgItemText(hStatus, IDC_SECONDARY_STATUS_MESSAGE, (LPCSTR)msg);
}


/*====================
  ModelExporterR8::SecondaryStatusProgress
  ====================*/
void	ModelExporterR8::SecondaryStatusProgress(int progress)
{
	if (!hStatus)
		return;

	SendDlgItemMessage(hStatus, IDC_PROGRESS2, PBM_SETPOS, (WPARAM)progress, 0);
}


/*====================
  ModelExporterR8::CloseStatusBar
  ====================*/
void	ModelExporterR8::CloseStatusBar()
{
	if (hStatus)
	{
		DestroyWindow(hStatus);
		hStatus = NULL;
	}
}


/*====================
  ModelExporterR8::CountMeshType
  ====================*/
int ModelExporterR8::CountMeshType(int meshtype)
{
	int count = 0;
	for (uint ui(0); ui < m_pOutModel->GetNumMeshes(); ++ui)
	{
		if (m_pOutModel->GetMesh(ui)->mode == meshtype)
			++count;
	}

	return count;
}


/*====================
  ModelExporterR8::CopyKeys
  ====================*/
void	ModelExporterR8::CopyKeys(INode *node, const COutBone *pBone, SBoneMotion *motion, int frame)
{
	Point3 axis[3];
	Point3 pos;
	int baseframe = frame - GetStartFrame();

	//store the transform info for this frame
	Matrix3 tm = GetBoneTM(node, FRAME(frame));
	INode *parent(GetBoneParentNode(node));

	if (parent)
	{
		//store the transform relative to the parent bone
		Matrix3 invParentTM;
		invParentTM = GetBoneTM(parent, FRAME(frame));
		invParentTM.Invert();
		tm = tm * invParentTM;
	}

	AffineParts parts;
	decomp_affine(tm, &parts);

	/*if (tm.Parity())
	{
	}
	else*/
	{
	#ifdef _DEBUG_BONE_KEYS
		AddMessage(fmt("CopyKeys(): %s[%i]", pBone->GetName().c_str(), frame));
		Indent();

		AddMessage(fmt("X: %f %f", tm.GetTrans().x, parts.t.x));
		AddMessage(fmt("Y: %f %f", tm.GetTrans().y, parts.t.y));
		AddMessage(fmt("Z: %f %f", tm.GetTrans().z, parts.t.z));
	#endif

		motion->keys_x.keys[baseframe] = tm.GetTrans().x;
		++motion->keys_x.num_keys;
		motion->keys_y.keys[baseframe] = tm.GetTrans().y;
		++motion->keys_y.num_keys;
		motion->keys_z.keys[baseframe] = tm.GetTrans().z;
		++motion->keys_z.num_keys;

		float eulers[3];
		MatrixToEuler(tm, eulers, EULERTYPE_YXZ);		//YXZ is the euler order used in M_GetAxis()

	#ifdef _DEBUG_BONE_KEYS
		AddMessage(fmt("Yaw: %f", RAD2DEG(eulers[0])));
		AddMessage(fmt("Pitch: %f", RAD2DEG(eulers[1])));
		AddMessage(fmt("Roll: %f", RAD2DEG(eulers[2])));
	#endif

		motion->keys_roll.keys[baseframe] = RAD2DEG(eulers[0]);
		++motion->keys_roll.num_keys;
		motion->keys_pitch.keys[baseframe] = RAD2DEG(eulers[1]);
		++motion->keys_pitch.num_keys;
		motion->keys_yaw.keys[baseframe] = RAD2DEG(eulers[2]);
		++motion->keys_yaw.num_keys;

	#ifdef _DEBUG_BONE_KEYS
		Point3 p;
		Quat q;
		SpectralDecomp(tm, p, q);
		AddMessage(fmt("sX: %f %f", parts.k.x, p.x));
		AddMessage(fmt("sY: %f %f", parts.k.y, p.y));
		AddMessage(fmt("sZ: %f %f", parts.k.z, p.z));
	#endif

		motion->keys_scalex.keys[baseframe] = parts.k.x;
		++motion->keys_scalex.num_keys;
		motion->keys_scaley.keys[baseframe] = parts.k.y;
		++motion->keys_scaley.num_keys;
		motion->keys_scalez.keys[baseframe] = parts.k.z;
		++motion->keys_scalez.num_keys;
	}

	#ifdef _DEBUG_BONE_KEYS
		AddMessage(fmt("Vis: %i", CLAMP(INT_FLOOR(node->GetVisibility(FRAME(frame)) * 255), 0, 255)));
	#endif

	motion->keys_visibility.keys[baseframe] = CLAMP(INT_FLOOR(node->GetVisibility(FRAME(frame)) * 255), 0, 255);
	++motion->keys_visibility.num_keys;

#ifdef _DEBUG_BONE_KEYS
	Unindent();
#endif
}


/*====================
  ModelExporterR8::StoreBoneMotions

  store animation data
  ====================*/
void	ModelExporterR8::StoreBoneMotions()
{
	if (!m_bExpOptAnimation)
		return;

	int iNumFrames = GetNumFrames();

	for (uint ui(0); ui < m_pOutClip->GetNumMotions(); ++ui)
	{
		SBoneMotion *pMotion = m_pOutClip->GetBoneMotion(ui);

		AllocateFloatKeys(&pMotion->keys_x, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_y, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_z, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_pitch, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_roll, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_yaw, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_scalex, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_scaley, iNumFrames);
		AllocateFloatKeys(&pMotion->keys_scalez, iNumFrames);
		AllocateByteKeys(&pMotion->keys_visibility, iNumFrames);
	}

	// Set up pointers to bone motions
	SBoneMotion		**ppMotions = new SBoneMotion *[m_pOutModel->GetNumBones()];

	for (uint n = 0; n < m_pOutModel->GetNumBones(); ++n)
	{
		ppMotions[n] = NULL;

		for (uint i = 0; i < m_pOutClip->GetNumMotions(); ++i)
		{
			if (m_pOutModel->GetBone(n)->GetName() == m_pOutClip->GetBoneMotion(i)->sBoneName)
			{
				// Assign the bone to this motion
				ppMotions[n] = m_pOutClip->GetBoneMotion(i);
				break;
			}
		}
	}

	for (int frame = GetStartFrame(); frame <= GetEndFrame(); ++frame)
	{
		for (uint i = 0; i < m_pOutClip->GetNumMotions(); ++i)
		{
			CopyKeys(m_vpBoneNodes[i], m_pOutModel->GetBone(i), ppMotions[i], frame);
		}
		//SecondaryStatusProgress(((float)(frame - GetStartFrame()) / GetNumFrames()) * 100.0);
	}

	delete[] ppMotions;

	// Do a compare on all keys to see which ones we can compress down
	for (uint ui(0); ui < m_pOutClip->GetNumMotions(); ++ui)
	{
		SBoneMotion *pMotion = m_pOutClip->GetBoneMotion(ui);

		CompressFloatKeys(&pMotion->keys_x);
		CompressFloatKeys(&pMotion->keys_y);
		CompressFloatKeys(&pMotion->keys_z);
		CompressFloatKeys(&pMotion->keys_pitch);
		CompressFloatKeys(&pMotion->keys_roll);
		CompressFloatKeys(&pMotion->keys_yaw);
		CompressFloatKeys(&pMotion->keys_scalex);
		CompressFloatKeys(&pMotion->keys_scaley);
		CompressFloatKeys(&pMotion->keys_scalez);
		CompressByteKeys(&pMotion->keys_visibility);
	}
}


/*====================
  ModelExporterR8::FinishModel
  ====================*/
void	ModelExporterR8::FinishModel()
{
	int totalverts = 0;
	int totalfaces = 0;

	if (!m_bExpOptGeometry)
		return;

	AddMessage("******************************************");
	AddMessage("**********     MODEL SUMMARY    **********");
	AddMessage("******************************************");
	Indent();

	for (size_t n = 0; n < m_pOutModel->GetNumMeshes(); ++n)
	{
		totalverts += m_pOutModel->GetMesh(n)->num_verts;
		totalfaces += m_pOutModel->GetMesh(n)->numFaces;

		for (size_t i = n + 1; i < m_pOutModel->GetNumMeshes(); ++i)
		{
			if (m_pOutModel->GetMesh(n)->GetName() == m_pOutModel->GetMesh(i)->GetName())
				Error(fmt("Duplicate mesh name: \"%s\"", m_pOutModel->GetMesh(i)->GetName().c_str()));
		}
	}

	for (int n = 0; n < m_pOutModel->GetNumBones(); ++n)
	{
		// Set parent
		COutBone *pBone(m_pOutModel->GetBone(n));

		vector<INode *>::iterator findit(find(m_vpBoneNodes.begin(), m_vpBoneNodes.end(), m_vpBoneParentNodes[n]));

		if (findit != m_vpBoneNodes.end())
			pBone->SetParent(uint(findit - m_vpBoneNodes.begin()));

		for (int i = n + 1; i < m_pOutModel->GetNumBones(); ++i)
		{
			if (m_pOutModel->GetBone(n)->GetName() == m_pOutModel->GetBone(i)->GetName())
				Error(fmt("Duplicate bone name: \"%s\"", m_pOutModel->GetBone(i)->GetName().c_str()));
		}
	}

	AddMessage("");
	AddMessage("");
	AddMessage(fmt("%i vertices", totalverts));
	AddMessage(fmt("%i faces", totalfaces));
	AddMessage("");
	AddMessage(fmt("%i meshes", m_pOutModel->GetNumMeshes()));
	AddMessage(fmt("%i bones", m_pOutModel->GetNumBones()));
	//AddMessage(fmt("%i sprites", m_pOutModel->num_sprites));
	AddMessage(fmt("%i collision surfaces", m_pOutModel->GetNumSurfs()));
	const CBBoxf &bbBounds(m_pOutModel->GetBounds());
	AddMessage(fmt("Bounding box: (%f, %f, %f) to (%f, %f, %f)", bbBounds.GetMin().x, bbBounds.GetMin().y, bbBounds.GetMin().z, bbBounds.GetMax().x, bbBounds.GetMax().y, bbBounds.GetMax().z));
	AddMessage("");
	AddMessage("Mesh breakdown:");
	AddMessage(fmt("%i non-blended / rigid meshes", CountMeshType(MESH_SKINNED_NONBLENDED)));
	AddMessage(fmt("%i blended meshes", CountMeshType(MESH_SKINNED_BLENDED)));

	if (model_error)
		::MessageBox(m_hPanel, "There were one or more errors with this model.  Check error log for details.", "Error(s) with model", MB_OK);
}


/*====================
  ModelExporterR8::Cleanup
  ====================*/
void	ModelExporterR8::Cleanup()
{
	// Fixme: need a solution to this
	//Tag_FreeAll(MEM_SCENE);		//free any memory allocated during scene traversal
}


/*====================
  ModelExporterR8::SceneInfo
  ====================*/
void	ModelExporterR8::SceneInfo()
{
	ClearSceneInfo();

	m_bExpOptGeometry = true;
	m_bExpOptMDF = false;
	m_bExpOptAnimation = false;
	m_bExpOptMaterials = false;

	OpenStatusBar();
	TraverseNode(m_pMaxInterface->GetRootNode());
	FinishModel();
	CloseStatusBar();

	if (model_error)
		ShowErrorInfo();
	ShowSceneInfo();

	Cleanup();
}


/*====================
  ModelExporterR8::ShowSceneInfo
  ====================*/
void	ModelExporterR8::ShowSceneInfo()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_SCENE_INFO),
		m_hPanel,
		SceneInfoDlgProc, (LPARAM)this);
}


/*====================
  ModelExporterR8::ShowErrorInfo
  ====================*/
void	ModelExporterR8::ShowErrorInfo()
{
	DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_MODEL_ERRORS),
		m_hPanel,
		ErrorInfoDlgProc, (LPARAM)this);
}


/*====================
  ModelExporterR8::Export
  ====================*/
void	ModelExporterR8::Export(bool bAnimationOnly)
{
	try
	{
		ClearSceneInfo();

		if (bAnimationOnly)
		{
			m_bExpOptGeometry = false;
			m_bExpOptMDF = false;
			m_bExpOptMaterials = false;
			m_bExpOptAnimation = true;
		}

		OpenStatusBar();
		TraverseNode(m_pMaxInterface->GetRootNode());
		StoreBoneMotions();
		FinishModel();
		CloseStatusBar();

		if (model_error)
			throw _T("Model processing failed");

		// Geometry
		if (m_bExpOptGeometry)
		{
			if (strcmp(filename, ""))
			{
				tstring sFilename(Filename_StripExtension(filename) + ".model");
				strncpy(filename, sFilename.c_str(), sizeof(filename) - 1);
			}

			if (GetFilename("K2 Model Files (*.model)", "*.model", "model"))
				m_pOutModel->WriteFile(filename);
			if (model_error)
				throw _T("Model writing failed");
		}

		// MDF
		if (m_bExpOptMDF)
		{
			if (m_bExpOptGeometry || GetFilename("K2 Model Files (*.model)", "*.model", "model"))
			{
				m_pOutMDF->SetName(static_cast<TCHAR *>(m_pMaxInterface->GetCurFileName()));
				m_pOutMDF->SetModelFile(Filename_StripPath(Filename_SanitizePath(filename)));

				tstring sFilename(Filename_StripExtension(filename) + ".mdf");

				strncpy(filename, sFilename.c_str(), sizeof(filename) - 1);

				if (GetFilename("K2 Model Definition Files (*.mdf)", "*.mdf", "mdf"))
					m_pOutMDF->WriteFile(filename);
				if (model_error)
					throw _T("Model writing failed");
			}
		}

		// Materials
		if (m_bExpOptMaterials)
			WriteMaterials();

		// Clip
		if (m_bExpOptAnimation)
		{
			tstring sFilename(Filename_StripExtension(filename) + ".clip");

			strncpy(filename, sFilename.c_str(), sizeof(filename) - 1);

			if (GetFilename("K2 Animation Files (*.clip)", "*.clip", "clip"))
				m_pOutClip->WriteFile(filename);
			if (model_error)
				throw _T("Error writing model");
		}

		ShowSceneInfo();
	}
	catch (const tstring sMsg)
	{
		Error(sMsg);
		ShowErrorInfo();
	}

	Cleanup();
}


/*====================
  ModelExporterR8::GetFilename
  ====================*/
BOOL	ModelExporterR8::GetFilename(char *fileTypeDesc, char *wildcard, char *extension)
{
	OPENFILENAME ofn;
	FilterList fl;

	fl.Append(fileTypeDesc);
	fl.Append(wildcard);

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hPanel;
	ofn.hInstance = (HINSTANCE)GetWindowLongPtr(ofn.hwndOwner, GWLP_HINSTANCE);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = fl;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = extension;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}


/*====================
  ModelExporterR8::WriteMaterials
  ====================*/
void	ModelExporterR8::WriteMaterials()
{
	tstring sPath(Filename_GetPath(Filename_SanitizePath(filename)));
	for (map<tstring, COutMaterial>::iterator it(m_mapMaterials.begin()); it != m_mapMaterials.end(); ++it)
		AddMessage(it->second.WriteFile(sPath, m_bExpOptCopyTextures));
}


/*====================
  ModelExporterR8::MessageBox
  ====================*/
void	ModelExporterR8::MessageBox(const char *szMsg, const char *szTitle)
{
	::MessageBox(m_hPanel, szMsg, szTitle, MB_OK);
}
