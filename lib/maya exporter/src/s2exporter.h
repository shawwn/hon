// (C)2008 S2 Games
// s2exporter.h
//
//=============================================================================
#ifndef __S2EXPORTER_H__
#define __S2EXPORTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "resource.h"

#include <vector>
#include <map>
#include <list>
#include <set>
#include <stack>
using std::vector;
using std::map;
using std::list;
using std::set;
using std::stack;

#include "meshtools.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class COutBone;
class COutMesh;
class COutMDF;
class COutModel;
class COutClip;
class COutMaterial;
struct SBoneMotion;

extern class CModelExporter g_ModelExporter;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Export flags
const int EXPORT_GEOMETRY	(BIT(0));
const int EXPORT_ANIMS		(BIT(1));
const int EXPORT_MATERIALS	(BIT(2));
const int EXPORT_MDF		(BIT(3));

enum ENodeType
{
	NODETYPE_SPRITE,
	NODETYPE_MESH,
	NODETYPE_REFERENCE_BONE,
	NODETYPE_SURFACE
};

typedef vector<size_t>		IndexVector;
typedef vector<IndexVector>	IndexVectorVector;
typedef vector<MItDag>		DagIteratorVector;
//=============================================================================

//=============================================================================
// CModelExporter
//=============================================================================
class CModelExporter
{
private:
	char			filename[_MAX_PATH];

	// Working MDF
	COutMDF*		m_pOutMDF;

	// Working model
	COutModel*		m_pOutModel;

	// Working clip
	COutClip*		m_pOutClip;

	// Nodes
	size_t				m_zNumNodes;
	size_t				m_zCurrentNode;
#if 0
	DagIteratorVector	m_vBoneNodes;
	DagIteratorVector	m_vBoneParentNodes;
#endif

	// Materials
	map<tstring, COutMaterial>	m_mapMaterials;

	// Mesh
	void	AddMesh(MItDag &itDag);

	int		m_iPhysiquedMeshes;
	int		m_iSkinnedMeshes;

	// Exporting options
	bool	m_bExpOptForceStatic;
	bool	m_bExpOptNoHierarchy;
	bool	m_bExpOptCopyTextures;
	bool	m_bExpOptGeometry;
	bool	m_bExpOptMDF;
	bool	m_bExpOptMaterials;
	bool	m_bExpOptAnimation;

	bool	m_bModelError;

	HWND	m_hPanel;
	HWND	m_hMeshPanel;

public:
	~CModelExporter();
	CModelExporter();

	// Main panel functions
	void	Export(bool bAnimationOnly);
	void	SceneInfo();
	void	WriteMaterials();

	// Mesh Panel
#if 0
	void	UpdateMeshPanel();
	void	Mesh_SetName(const tstring &sName);
	void	Mesh_SetVSHname(const tstring &sName);
	void	Mesh_SetPSHname(const tstring &sName);
	void	Mesh_SetProp(const tstring &sTag, bool b);
	void	Mesh_SetProp(const tstring &sTag, int i);
	void	Mesh_SetProp(const tstring &sTag, float f);
	void	Mesh_SetProp(const tstring &sTag, const tstring &s);
	void	Mesh_SetProp(const tstring &sTag, const char *sz);
	void	TagSelection(const tstring &sTag);
#endif

	// Message output
	void	Error(const string &sMessage);

	void	TraverseNode();
	void	CountNodes();

	bool	IsMeshSkinned(MItDag &itDag);

#if 0
	bool	AddSkinBones(ISkin *pSkinInterface, ISkinContextData *pSkinCD, int *piNumBlended, int *piNumNonBlended);

	void	AddSprite(INode *node, int spritetype);
	void	AddSurf(INode *node);
	bool	AddSurfToModel(INode *node, struct SConvexPolyhedron *surf);
	INode*	GetBoneParentNode(INode *node);
#endif
	void	StoreBoneMotions();

	// Meshes
	bool	AddStaticMesh(COutMesh *pMesh, MItDag &itDag);
	bool	AddSkinnedMesh(COutMesh *pMesh, MItDag &itDag);

	int		GetStartFrame()	{ return 0;/*(m_pMaxInterface->GetAnimRange().Start() / GetTicksPerFrame());*/ }
	int		GetEndFrame()	{ return 0;/*(m_pMaxInterface->GetAnimRange().End() / GetTicksPerFrame());*/ }
	int		GetNumFrames()	{ return (GetEndFrame() - GetStartFrame()) + 1; }

	void	ClearSceneInfo();

	bool	FinishMesh(COutMesh *pMesh, MItDag &itDag);
	tstring	AddTextureReference(COutMesh *pMesh, MItDag &itDag);
	uint	AddBone(MItDag &itDag);

	BOOL	GetFilename(char *fileTypeDesc, char *wildcard, char *extension);

	void	CopyKeys(MItDag &itDag, const COutBone *s2bone, SBoneMotion *motion, int frame);

#if 0
	void	AddReferenceBone(INode *node);
	void	WriteClipFile();

	// Functions that manipulate the 'model' structure
	const COutBone*	GetBone(uint uiIndex);
	const COutBone*	GetBone(const tstring &sName);
	uint			GetBoneIndex(const tstring &sName);
#endif
	int				CountMeshType(int meshtype);
	void			FinishModel();
	void			Cleanup();
};
//=============================================================================
#endif //__S2EXPORTER_H__
