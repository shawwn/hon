/**********************************************************************
*<
FILE:			SplitMeshes.cpp
DESCRIPTION:	Splits Editable Meshes up into separate nodes based on materials.
CREATED BY:		Steve Anderson
HISTORY:		Created in a hurry, March 15, 2004

  *>	Copyright (c) 2004 Kinetix, All Rights Reserved.
**********************************************************************/

#include "MeshSplit.h"
#include "MeshDLib.h"

class SplitMeshesUtil : public UtilityObj
{
public:
	IUtil		*iu;
	Interface	*ip;
	HWND		hPanel;
    
	SplitMeshesUtil();
	~SplitMeshesUtil();

	void	BeginEditParams(Interface *ip,IUtil *iu);
	void	EndEditParams(Interface *ip,IUtil *iu);
	void	DeleteThis() {}

	void	ApplySelected();
	BOOL	ApplyNode(INode* node);
};

static SplitMeshesUtil theMeshSplitter;

//==============================================================================
// class SplitMeshesUtilClassDesc
//==============================================================================
class SplitMeshesUtilClassDesc : public ClassDesc {
public:
    int 			IsPublic() {return 1;}
    void *			Create(BOOL loading = FALSE) {return &theMeshSplitter;}
    const TCHAR *	ClassName() {return GetString(IDS_SPLIT_MESHES);}
    SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
    Class_ID		ClassID() {return Class_ID(0x265f538f, 0x5de97d06);}
    const TCHAR* 	Category() {return _T("");}
};

static SplitMeshesUtilClassDesc theSplitMeshesUtilClassDesc;
ClassDesc* GetSplitMeshesUtilDesc() {return &theSplitMeshesUtilClassDesc;}

static INT_PTR CALLBACK ApplySplitMeshesDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_COMMAND:
		switch (LOWORD(wParam)) {
        case IDC_SPLIT_MESHES_APPLY:
            theMeshSplitter.ApplySelected();
            break;
		}
        break;
        
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
        theMeshSplitter.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
        break;
        
    default:
        return FALSE;
    }
    return TRUE;
}	

SplitMeshesUtil::SplitMeshesUtil()
{
    iu = NULL;
    ip = NULL;	
    hPanel = NULL;
}

SplitMeshesUtil::~SplitMeshesUtil()
{
}

void SplitMeshesUtil::BeginEditParams(Interface *ip,IUtil *iu) 
{
    this->iu = iu;
    this->ip = ip;
    hPanel = ip->AddRollupPage (hInstance,
        MAKEINTRESOURCE(IDD_SPLIT_MESHES), ApplySplitMeshesDlgProc,
        GetString(IDS_SPLIT_MESHES_DIALOG_TITLE), 0);
}

void SplitMeshesUtil::EndEditParams(Interface *ip,IUtil *iu) 
{
    this->iu = NULL;
    this->ip = NULL;
    ip->DeleteRollupPage(hPanel);
    hPanel = NULL;
}

void SplitMeshesUtil::ApplySelected()
{
    for (int i=0; i<ip->GetSelNodeCount(); i++)
    {
        if (!ApplyNode(ip->GetSelNode(i))) {
            break;
        }
    }

	ip->RedrawViews(ip->GetTime());
}

BOOL SplitMeshesUtil::ApplyNode(INode* node)
{
    if (!node) return TRUE;
	if( ip==NULL ) ip=GetCOREInterface(); //hack
	Mtl *material = node->GetMtl();
	if (material == NULL) return true;
	if (!material->IsMultiMtl()) return true;
	int numSubs = material->NumSubMtls();
	if (numSubs <= 0) return true; // Shouldn't happen.

	if (numSubs == 1)
	{
		theHold.Begin ();
		// We have a multi/sub material with only 1 submaterial.  Flatten it.
		node->SetMtl (material->GetSubMtl (0));

		theHold.Accept ("Split Meshes By Material");
		return true;
	}

	ObjectState os = node->EvalWorldState(ip->GetTime());
    if (!os.obj) return TRUE;
    if (!os.obj->CanConvertToType(triObjectClassID)) return TRUE;

	theHold.Begin ();

	TriObject *tobj = (TriObject *) os.obj->ConvertToType (ip->GetTime(), triObjectClassID);
	Mesh & mesh = tobj->GetMesh();

	for (int i=0; i<material->NumSubMtls(); i++)
	{
		// Find faces with this material in the mesh.
		BitArray fsel;
		fsel.SetSize (mesh.numFaces);
		for (int f=0; f<mesh.numFaces; f++)
		{
			if (mesh.faces[f].getMatID()%numSubs == i) fsel.Set(f, true);
		}
		if (fsel.NumberSet() == 0) continue;

		TriObject *newObj;
		newObj = CreateNewTriObject();

		MeshDelta tmd;
		tmd.Detach (mesh, &(newObj->GetMesh()), fsel, true, false, false);

		// Set the material:
		INode *newNode = ip->CreateObjectNode (newObj);
		newNode->SetMtl (material->GetSubMtl (i));

		// Set the name:
		TCHAR numstring[20];
		sprintf (numstring, "_%d", i);
		TSTR uname = node->GetName();
		uname += TSTR(numstring);
		ip->MakeNameUnique (uname);
		newNode->SetName (uname);

		// Set the transform:
		newNode->SetNodeTM(ip->GetTime(),node->GetNodeTM(ip->GetTime()));

		// Set the other node properties:
		newNode->CopyProperties (node);
		newNode->FlagForeground(ip->GetTime(),FALSE);
		newNode->SetObjOffsetPos (node->GetObjOffsetPos());
		newNode->SetObjOffsetRot (node->GetObjOffsetRot());
		newNode->SetObjOffsetScale (node->GetObjOffsetScale());
	}

	ip->DeleteNode (node);

	theHold.Accept ("Split Meshes By Material");

	return true;
}




















