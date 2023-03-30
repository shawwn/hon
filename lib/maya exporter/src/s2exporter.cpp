// (C)2008 S2 Games
// CModelExporter.cpp
//
// 3dsmax .model exporter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

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

#include "NvTriStrip.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
typedef matrix44_t transform_t;

#define	FACEVERTEX(mesh, face, idx) mesh->verts[mesh->faces[face].v[idx]]

char *nodeTypeNames[] =
{
	"Sprite",
	"Mesh",
	"Reference Bone",
	"Collision Surface"
};
//=============================================================================

/*====================
  CModelExporter::~CModelExporter
  ====================*/
CModelExporter::~CModelExporter()
{
}


/*====================
  CModelExporter::CModelExporter
  ====================*/
CModelExporter::CModelExporter() :
m_zNumNodes(0),
m_zCurrentNode(0),

m_hPanel(NULL),
m_hMeshPanel(NULL),

m_bExpOptForceStatic(false),
m_bExpOptNoHierarchy(false),
m_bExpOptCopyTextures(false),
m_bExpOptGeometry(true),
m_bExpOptMDF(true),
m_bExpOptMaterials(true),
m_bExpOptAnimation(false),

m_pOutMDF(NULL),
m_pOutModel(NULL),
m_pOutClip(NULL),

m_bModelError(false)
{
}


/*====================
  CModelExporter::Export
  ====================*/
void	CModelExporter::Export(bool bAnimationOnly)
{
	try
	{
		cout << endl << "Starting export..." << endl;

		ClearSceneInfo();

		if (bAnimationOnly)
		{
			m_bExpOptGeometry = false;
			m_bExpOptMDF = false;
			m_bExpOptMaterials = false;
			m_bExpOptAnimation = true;
		}

		TraverseNode();
		StoreBoneMotions();
		FinishModel();

		if (m_bModelError)
			throw _T("Model processing failed");

		// Geometry
		if (m_bExpOptGeometry)
		{
			if (strcmp(filename, ""))
			{
				tstring sFilename(Filename_StripExtension(filename) + ".model");
				strncpy_s(filename, _MAX_PATH, sFilename.c_str(), sizeof(filename) - 1);
			}

			if (GetFilename("K2 Model Files (*.model)", "*.model", "model"))
				m_pOutModel->WriteFile(filename);
			if (m_bModelError)
				throw _T("Model writing failed");
		}

		// MDF
		if (m_bExpOptMDF)
		{
			if (m_bExpOptGeometry || GetFilename("K2 Model Files (*.model)", "*.model", "model"))
			{
				m_pOutMDF->SetName(""/*static_cast<TCHAR *>(m_pMaxInterface->GetCurFileName())*/);
				m_pOutMDF->SetModelFile(Filename_StripPath(Filename_SanitizePath(filename)));

				tstring sFilename(Filename_StripExtension(filename) + ".mdf");

				strncpy_s(filename, _MAX_PATH, sFilename.c_str(), sizeof(filename) - 1);

				if (GetFilename("K2 Model Definition Files (*.mdf)", "*.mdf", "mdf"))
					m_pOutMDF->WriteFile(filename);
				if (m_bModelError)
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

			strncpy_s(filename, _MAX_PATH, sFilename.c_str(), sizeof(filename) - 1);

			if (GetFilename("K2 Animation Files (*.clip)", "*.clip", "clip"))
				m_pOutClip->WriteFile(filename);
			if (m_bModelError)
				throw _T("Error writing model");
		}
	}
	catch (const tstring sMsg)
	{
		Error(sMsg);
		::MessageBox(m_hPanel, sMsg.c_str(), "Export error", MB_OK);
	}

	Cleanup();
}


#if 0
/*====================
  SelectionChanged
  ====================*/
static void	SelectionChanged(void *pParam, NotifyInfo *pInfo)
{
	g_ModelExporter.Mesh_ApplyCurrentSettings();
	g_ModelExporter.UpdateMeshPanel();
}


/*====================
  CModelExporterDlgProc
  ====================*/
static BOOL CALLBACK CModelExporterDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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
					g_ModelExporter.Export(false);
					break;

				case IDC_EXPORT_CLIP:
					g_ModelExporter.Export(true);
					break;

				case IDC_CLOSE:
					g_ModelExporter.Close();
					break;

				case IDC_SCENE_INFO:
					g_ModelExporter.SceneInfo();
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
  CModelExporter::GetBoneParentNode
  ====================*/
INode*	CModelExporter::GetBoneParentNode(INode *node)
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
#endif


/*====================
  CModelExporter::IsMeshSkinned
  ====================*/
bool	CModelExporter::IsMeshSkinned(MItDag &itDag)
{
	MDagPath pathMesh;
	if (itDag.getPath(pathMesh) != MS::kSuccess)
	{
		cerr << "Could not get path" << endl;
		return false;
	}

	cout << "Looking for a skin cluster for: " << pathMesh.fullPathName() << endl;

	// Search dependency graph for skin clusters
	for (MItDependencyNodes itNode(MFn::kInvalid); !itNode.isDone(); itNode.next())
	{
		MObject object(itNode.item());
		if (object.apiType() != MFn::kSkinClusterFilter)
			continue;

		// Check to see if this object is affected by this skin cluster
		MFnSkinCluster fnSkin(object);
		
		uint uiOutputCount(fnSkin.numOutputConnections());
		for (uint uiOutput(0); uiOutput < uiOutputCount; ++uiOutput)
		{
			uint uiIndex(fnSkin.indexForOutputConnection(uiOutput));
			MDagPath pathTest;
			fnSkin.getPathAtIndex(uiIndex, pathTest);
			cout << "  " << pathTest.fullPathName() << endl;
			if (pathTest == pathMesh)
				return true;
		}
	}

	return false;
}


/*====================
  CModelExporter::Error
  ====================*/
void	CModelExporter::Error(const string &sMessage)
{
	cerr << sMessage << endl;
	m_bModelError = true;
}


#if 0
/*====================
  CModelExporter::AddSkinBones
  ====================*/
bool	CModelExporter::AddSkinBones(ISkin *pSkinInterface, ISkinContextData *pSkinCD, int *piNumBlended, int *piNumNonBlended)
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
	cout << "Skinned mesh with " << numverts << " verts:" << endl;
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
#endif

/*====================
  CModelExporter::AddSkinnedMesh
  ====================*/
bool	CModelExporter::AddSkinnedMesh(COutMesh *pMesh, MItDag &itDag)
{
	cout << "Skinned mesh!" << endl;
#if 0
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
#endif
	return true;
}


#if 0
/*====================
  CModelExporter::AddSprite
  ====================*/
void	CModelExporter::AddSprite(INode *node, int spritetype)
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

	cout << "Type: " << nodeTypeNames[NODETYPE_SPRITE] << endl;
	cout << "Details:" << endl;
	AddMessage(fmt("Width = %f, Height = %f, Type = %i", width, height, spritetype));
	cout << endl;
}
#endif


/*====================
  CModelExporter::AddTextureReference
  ====================*/
tstring	CModelExporter::AddTextureReference(COutMesh *pMesh, MItDag &itDag)
{
	// Get the material
	COutMaterial outmat(pMesh, itDag);
	m_mapMaterials[outmat.GetName()] = outmat;

	if (pMesh != NULL)
		pMesh->SetDefaultShaderName(outmat.GetName());

	return outmat.GetName();
}


/*====================
  CModelExporter::FinishMesh
  ====================*/
bool	CModelExporter::FinishMesh(COutMesh *pMesh, MItDag &itDag)
{
	if (!m_bExpOptGeometry)
		return true;

	IndexVectorVector comparisonList;
	if (!CreateFaceAndVertexData(pMesh, itDag, comparisonList))
	{
		Error("FinishMesh: failed to generate vertex data");
		return false;
	}

	// Adjust the model's bounds to contain this mesh's bounds
	m_pOutModel->AddBounds(pMesh->bbBounds);

	// Check for redundant vertices and remap faces to exclude them
	map<int, int> mapVerts;
	int count(0);

	cout << "Comparing " << pMesh->GetVertexCount() << " vertices" << endl;

	for (uint l = 0; l < comparisonList.size(); ++l)
	{
		for (uint v = 0; v < comparisonList[l].size(); ++v)
		{
			++count;
			for (uint w = v + 1; w < comparisonList[l].size(); ++w)
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
			}
		}
	}

	cout << "Removed " << mapVerts.size() << " duplicate vertices." << endl;

	// Finish the map, accounting for a condensed vert list
	count = 0;
	for (uint v(0); v < pMesh->GetVertexCount(); ++v)
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
	pMesh->SetVertexCount(count);

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
	for (uint g(0); g < usNumSections; ++g)
	{
		for (uint i(0); i < pPrimitiveGroupsList[g].numIndices; ++i)
			pPrimitiveGroupsList[g].indices[i] = pRemappedIndices[pPrimitiveGroupsList[g].indices[i]];
	}

	// Reorder verticies
	vec3_t *verts(new vec3_t[pMesh->GetVertexCount()]);
	memcpy(verts, pMesh->verts, sizeof(vec3_t) * pMesh->GetVertexCount());

	vec3_t *normals(NULL);
	if (pMesh->normals)
	{
		normals = new vec3_t[pMesh->GetVertexCount()];
		memcpy(normals, pMesh->normals, sizeof(vec3_t) * pMesh->GetVertexCount());
	}

	bvec4_t *colors[MAX_VERTEX_COLOR_CHANNELS];
	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		if (pMesh->colors[n])
		{
			colors[n] = new bvec4_t[pMesh->GetVertexCount()];
			memcpy(colors[n], pMesh->colors[n], sizeof(bvec4_t) * pMesh->GetVertexCount());
		}
		else
			colors[n] = NULL;
	}

	vec2_t *tverts[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->tverts[n])
		{
			tverts[n] = new vec2_t[pMesh->GetVertexCount()];
			memcpy(tverts[n], pMesh->tverts[n], sizeof(vec2_t) * pMesh->GetVertexCount());
		}
		else
			tverts[n] = NULL;
	}

	vec3_t *tangents[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->tangents[n])
		{
			tangents[n] = new vec3_t[pMesh->GetVertexCount()];
			memcpy(tangents[n], pMesh->tangents[n], sizeof(vec3_t) * pMesh->GetVertexCount());
		}
		else
			tangents[n] = NULL;
	}

	byte *signs[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (pMesh->signs[n])
		{
			signs[n] = new byte[pMesh->GetVertexCount()];
			memcpy(signs[n], pMesh->signs[n], pMesh->GetVertexCount());
		}
		else
			signs[n] = NULL;
	}

	SBlendedLink *blendedLinks(NULL);
	if (pMesh->mode == MESH_SKINNED_BLENDED && pMesh->blendedLinks)
	{
		blendedLinks = new SBlendedLink[pMesh->GetVertexCount()];
		memcpy(blendedLinks, pMesh->blendedLinks, sizeof(SBlendedLink) * pMesh->GetVertexCount());
	}

	singleLink_t *singleLinks(NULL);
	if (pMesh->mode == MESH_SKINNED_NONBLENDED && pMesh->singleLinks)
	{
		singleLinks = new singleLink_t[pMesh->GetVertexCount()];
		memcpy(singleLinks, pMesh->singleLinks, sizeof(singleLinks) * pMesh->GetVertexCount());
	}

	for (uint v(0); v < pMesh->GetVertexCount(); ++v)
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

	cout << endl;
	cout << "Mesh has " << pMesh->GetVertexCount() << " vertices and " << pMesh->numFaces << " faces" << endl;
	return true;
}


/*====================
  CModelExporter::AddBone
  ====================*/
uint	CModelExporter::AddBone(MItDag &itDag)
{
	string sName(itDag.partialPathName().asChar());
	cout << "Adding bone for node: " << sName << " (" << itDag.currentItem().apiTypeStr() << ")" << endl;
	return 0;

	//if (itDag.currentItem().hasFn(MFn::kMesh))
		MFnMesh mesh(itDag.currentItem());
		MMatrix matrix(mesh.transformationMatrix());


	// Do a name check to make sure this bone doesn't already exist
	const COutBone *pBone(m_pOutModel->GetBone(sName));
	if (pBone != NULL)
		return pBone->GetIndex();

	// Initialize the new bone
	uint uiNewIndex(m_pOutModel->GetNumBones());
	COutBone newBone(uiNewIndex, sName, INVALID_BONE);

	// Set up the bone transform matrices
	CopyMatrix3(matrix, newBone.m_base);
	MMatrix matrixInverse(matrix.inverse());
	CopyMatrix3(matrixInverse, newBone.m_base);

	// Add the bone index to the clip in case we're exporting animation
	if (m_pOutClip)
		m_pOutClip->AddBone(newBone);

	// Keep track of which node each bone belongs to
#if 0
	if (m_vBoneNodes.size() < uiNewIndex + 1)
		m_vBoneNodes.resize(uiNewIndex + 1, MItDag());

	m_vBoneNodes[uiNewIndex] = itDag;
#endif

	// Add the bone to the model
	m_pOutModel->AddBone(newBone);

	// Get parent information
	// Keep track of which node each bone belongs to
#if 0
	if (m_vpBoneParentNodes.size() < uiNewIndex + 1)
		m_vpBoneParentNodes.resize(uiNewIndex + 1, NULL);

	INode *pParentNode(GetBoneParentNode(pNode));
	if (pParentNode)
	{
		m_vpBoneParentNodes[uiNewIndex] = pParentNode;
		cout << "Added bone \"" << sName << "\" (index " << uiNewIndex << ") to model (parent: " << pParentNode->GetName() << ")" << endl;

		// Ensure all parent bones of this bone are added
		AddBone(pParentNode);
	}
	else
#endif
	{
		cout << "Added bone \"" << sName << "\" (index " << uiNewIndex << ") to model (no parent)" << endl;
	}

	return uiNewIndex;
}


/*====================
  CModelExporter::AddStaticMesh

  add a rigid mesh (a mesh that may transform but the vertices remain static)
  ====================*/
bool	CModelExporter::AddStaticMesh(COutMesh *pMesh, MItDag &itDag)
{
	MFnMesh fnMesh(itDag.currentItem());

	// Make a bone which represents this mesh's transform
	size_t zBoneIndex(AddBone(itDag));
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
  CModelExporter::AddMesh
  ====================*/
void	CModelExporter::AddMesh(MItDag &itDag)
{
	try
	{
		COutMesh *pNewMesh(new COutMesh);
		pNewMesh->SetName(itDag.partialPathName().asChar());

		if (m_bExpOptForceStatic)
		{
			if (!AddStaticMesh(pNewMesh, itDag))
				throw "AddStaticMesh() failed";
		}
		else
		{
			if (IsMeshSkinned(itDag))
			{
				if(!AddSkinnedMesh(pNewMesh, itDag))
					throw "AddSkinnedMesh() failed";
			}
			else
			{
				if (!AddStaticMesh(pNewMesh, itDag))
					throw "AddStaticMesh() failed";
			}
		}

		if (!FinishMesh(pNewMesh, itDag))
			throw "FinishMesh() failed";

		// Mesh data stored successfully, add this mesh to the model
		AddTextureReference(pNewMesh, itDag);
		m_pOutModel->AddMesh(*pNewMesh);
		cout << endl;
	}
	catch (const string &sMsg)
	{
		Error(sMsg);
		cerr << "There was an error with this mesh" << endl;
		cout << endl;
	}
}


#if 0
/*====================
  CModelExporter::AddReferenceBone
  ====================*/
void	CModelExporter::AddReferenceBone(INode *node)
{
	cout << "Type: " << nodeTypeNames[NODETYPE_REFERENCE_BONE] << endl;
	cout << "Details:" << endl;

	AddBone(node);

	cout << endl;
}


/*====================
  CModelExporter::AddSurf

  add a collision surface
  ====================*/
void	CModelExporter::AddSurf(INode *node)
{
	int deleteIt;
	TriObject *triobj;
	CConvexPolyhedron surf;
	CBBoxf bbBounds;

	cout << "Type: " << nodeTypeNames[NODETYPE_SURFACE] << endl;
	cout << "Details:" << endl;

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

	cout << endl;
}
#endif


/*====================
  CModelExporter::TraverseNode
  ====================*/
void	CModelExporter::TraverseNode()
{
	// Initialize iterator
	MStatus status;
	MItDag itDag(MItDag::kBreadthFirst, MFn::kInvalid, &status);
	if (status != MS::kSuccess)
	{
		cerr << "Failed to intialize iterator" << endl;
		return;
	}

	// Iterate over all objects
	for ( ; !itDag.isDone(); itDag.next())
	{
		// TODO: Ignore hidden?

		//cout << "Node: \"" << itDag.partialPathName().asChar() << "\"" << endl;

		// Mesh
		if (itDag.currentItem().hasFn(MFn::kMesh))
		{
			MFnDagNode node(itDag.currentItem());
			if (node.isIntermediateObject())
				continue;

			++m_zCurrentNode;
			cout << "Mesh: \"" << itDag.partialPathName().asChar() << "\" [" << m_zCurrentNode << "/" << m_zNumNodes << "]" << endl;
			AddMesh(itDag);
			continue;
		}

		// TODO: Surf
		// TODO: Reference Bone
	}
}


/*====================
  CModelExporter::CountNodes
  ====================*/
void	CModelExporter::CountNodes()
{
	m_zNumNodes = 0;
	MStatus stat;
	MItDag itDag(MItDag::kBreadthFirst, MFn::kInvalid, &stat);
	for ( ; !itDag.isDone(); itDag.next())
	{
		MDagPath dagPath;
		stat = itDag.getPath(dagPath);
		if (!stat)
			continue;

		if (dagPath.hasFn(MFn::kTransform) || !dagPath.hasFn(MFn::kMesh))
			continue;

		MFnDagNode node(dagPath, &stat);
		if (node.isIntermediateObject())
			continue;

		++m_zNumNodes;
	}
}


/*====================
  CModelExporter::ClearSceneInfo
  ====================*/
void	CModelExporter::ClearSceneInfo()
{
	// Clear working variables
	m_iPhysiquedMeshes = 0;
	m_iSkinnedMeshes = 0;
	m_bModelError = false;

#if 0
	// Panel options
	m_bExpOptForceStatic = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_FORCE_STATIC);
	m_bExpOptNoHierarchy = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_REMOVE_HIERARCHY);
	m_bExpOptCopyTextures = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_COPY_TEXTURES);
	m_bExpOptGeometry = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_GEOMETRY);
	m_bExpOptMaterials = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_MATERIALS);
	m_bExpOptMDF = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_MDF);
	m_bExpOptAnimation = !!IsDlgButtonChecked(m_hPanel, IDC_EXPOPT_ANIMATION);

	m_vBoneNodes.clear();
	m_vBoneParentNodes.clear();
#endif
	m_mapMaterials.clear();

	// Clear working mdf
	delete m_pOutMDF;
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
	delete m_pOutClip;
	m_pOutClip = new COutClip;

	// Count active nodes in the scene
	m_zCurrentNode = 0;
	CountNodes();

	// Add origin bone
	MItDag itDag;
	AddBone(itDag);
}


/*====================
  CModelExporter::CountMeshType
  ====================*/
int CModelExporter::CountMeshType(int meshtype)
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
  CModelExporter::CopyKeys
  ====================*/
void	CModelExporter::CopyKeys(MItDag &itDag, const COutBone *pBone, SBoneMotion *motion, int frame)
{
#if 0
	int baseframe(frame - GetStartFrame());

	// Store the transform info for this frame
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

#ifdef _DEBUG_BONE_KEYS
	AddMessage(fmt("CopyKeys(): %s[%i]", pBone->GetName().c_str(), frame));
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

#ifdef _DEBUG_BONE_KEYS
	AddMessage(fmt("Vis: %i", CLAMP(INT_FLOOR(node->GetVisibility(FRAME(frame)) * 255), 0, 255)));
#endif

	motion->keys_visibility.keys[baseframe] = CLAMP(INT_FLOOR(node->GetVisibility(FRAME(frame)) * 255), 0, 255);
	++motion->keys_visibility.num_keys;
#endif
}


/*====================
  CModelExporter::StoreBoneMotions

  store animation data
  ====================*/
void	CModelExporter::StoreBoneMotions()
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
	SBoneMotion **ppMotions = new SBoneMotion *[m_pOutModel->GetNumBones()];

	for (uint n(0); n < m_pOutModel->GetNumBones(); ++n)
	{
		ppMotions[n] = NULL;

		for (uint i(0); i < m_pOutClip->GetNumMotions(); ++i)
		{
			if (m_pOutModel->GetBone(n)->GetName() == m_pOutClip->GetBoneMotion(i)->sBoneName)
			{
				// Assign the bone to this motion
				ppMotions[n] = m_pOutClip->GetBoneMotion(i);
				break;
			}
		}
	}

	for (int frame(GetStartFrame()); frame <= GetEndFrame(); ++frame)
	{
#if 0
		for (uint i(0); i < m_pOutClip->GetNumMotions(); ++i)
			CopyKeys(m_vBoneNodes[i], m_pOutModel->GetBone(i), ppMotions[i], frame);
#endif
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
  CModelExporter::FinishModel
  ====================*/
void	CModelExporter::FinishModel()
{
	int totalverts = 0;
	int totalfaces = 0;

	if (!m_bExpOptGeometry)
		return;

	cout << "******************************************" << endl;
	cout << "**********     MODEL SUMMARY    **********" << endl;
	cout << "******************************************" << endl;

	for (size_t n = 0; n < m_pOutModel->GetNumMeshes(); ++n)
	{
		totalverts += m_pOutModel->GetMesh(n)->GetVertexCount();
		totalfaces += m_pOutModel->GetMesh(n)->numFaces;

		for (size_t i = n + 1; i < m_pOutModel->GetNumMeshes(); ++i)
		{
			if (m_pOutModel->GetMesh(n)->GetName() == m_pOutModel->GetMesh(i)->GetName())
				Error("Duplicate mesh name: \"" + m_pOutModel->GetMesh(i)->GetName() + "\"");
		}
	}

	for (uint n(0); n < m_pOutModel->GetNumBones(); ++n)
	{
#if 0
		// Set parent
		COutBone *pBone(m_pOutModel->GetBone(n));
		DagIteratorVector::iterator itBone(find(m_vBoneNodes.begin(), m_vBoneNodes.end(), m_vBoneParentNodes[n]));
		if (itBone != m_vBoneNodes.end())
			pBone->SetParent(uint(itBone - m_vBoneNodes.begin()));

		for (uint i(n + 1); i < m_pOutModel->GetNumBones(); ++i)
		{
			if (m_pOutModel->GetBone(n)->GetName() == m_pOutModel->GetBone(i)->GetName())
				Error("Duplicate bone name: \"" + m_pOutModel->GetBone(i)->GetName() + "\"");
		}
#endif
	}

	cout << endl;
	cout << endl;
	cout << totalverts << " vertices" << endl;
	cout << totalfaces << " faces" << endl;
	cout << endl;
	cout << m_pOutModel->GetNumMeshes() << " meshes" << endl;
	cout << m_pOutModel->GetNumBones() << " bones" << endl;
	//cout << m_pOutModel->num_sprites << " sprites" << endl;
	cout << m_pOutModel->GetNumSurfs() << " collision surfaces" << endl;
	const CBBoxf &bbBounds(m_pOutModel->GetBounds());
	cout << "Bounding box: (" << bbBounds.GetMin().x << ", " << bbBounds.GetMin().y << ", " << bbBounds.GetMin().z << ") to ("
		<< bbBounds.GetMax().x << ", " << bbBounds.GetMax().y << ", " << bbBounds.GetMax().z << ")" << endl;
	cout << endl;
	cout << "Mesh breakdown:" << endl;
	cout << CountMeshType(MESH_SKINNED_NONBLENDED) << " non-blended / rigid meshes" << endl;
	cout << CountMeshType(MESH_SKINNED_BLENDED) << " blended meshes" << endl;

	if (m_bModelError)
		::MessageBox(m_hPanel, "There were one or more errors with this model.  Check error log for details.", "Error(s) with model", MB_OK);
}


/*====================
  CModelExporter::Cleanup
  ====================*/
void	CModelExporter::Cleanup()
{
	// FIXME: Free any memory allocated during scene traversal
}


/*====================
  CModelExporter::SceneInfo
  ====================*/
void	CModelExporter::SceneInfo()
{
	ClearSceneInfo();

	m_bExpOptGeometry = true;
	m_bExpOptMDF = false;
	m_bExpOptAnimation = false;
	m_bExpOptMaterials = false;

	TraverseNode();
	FinishModel();
	Cleanup();
}


/*====================
  CModelExporter::GetFilename
  ====================*/
BOOL	CModelExporter::GetFilename(char *szFileTypeDesc, char *szWildcard, char *extension)
{
	OPENFILENAME ofn;
	size_t zTypeLen(strlen(szFileTypeDesc));
	size_t zWildcardLen(strlen(szWildcard));
	size_t zSize(zTypeLen + zWildcardLen + 3);
	char *szFilter(new char[zSize]);

	memset(szFilter, 0, zSize);
	memcpy_s(szFilter, zTypeLen, szFileTypeDesc, zTypeLen);
	memcpy_s(&szFilter[zTypeLen + 1], zWildcardLen, szWildcard, zWildcardLen);

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hPanel;
	ofn.hInstance = (HINSTANCE)GetWindowLongPtr(ofn.hwndOwner, GWLP_HINSTANCE);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = extension;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}


/*====================
  CModelExporter::WriteMaterials
  ====================*/
void	CModelExporter::WriteMaterials()
{
	tstring sPath(Filename_GetPath(Filename_SanitizePath(filename)));
	for (map<tstring, COutMaterial>::iterator it(m_mapMaterials.begin()); it != m_mapMaterials.end(); ++it)
		cout << it->second.WriteFile(sPath, m_bExpOptCopyTextures) << endl;
}
