// (C)2008 S2 Games
// s2exporter.h
//
//=============================================================================
#ifndef __MAXEXPORTER_H__
#define __MAXEXPORTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "CS/phyexp.h"
#include "CS/bipexp.h"
#include "iskin.h"

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

#include "stdmat.h"

#include "utilapi.h"

#include "meshtools.h"
//=============================================================================

//=============================================================================
//=============================================================================
extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

// Export flags
const int EXPORT_GEOMETRY	= BIT(0);
const int EXPORT_ANIMS		= BIT(1);
const int EXPORT_MATERIALS	= BIT(2);
const int EXPORT_MDF		= BIT(3);

enum ENodeType
{
	NODETYPE_SPRITE,
	NODETYPE_MESH,
	NODETYPE_REFERENCE_BONE,
	NODETYPE_SURFACE
};

#define OBJECT_NAME_LENGTH 64
#define OBJECT_CATEGORY_LENGTH 64
#define ERROR_LOG_LENGTH	8192

typedef struct texref_s
{
	BitmapTex	*bmptex;
	int			meshidx;
}
texref_t;

// Forward declared classes
class COutBone;
class COutMesh;
class COutMDF;
class COutModel;
class COutClip;
class COutMaterial;
struct SBoneMotion;
//=============================================================================

//=============================================================================
// ModelExporterR8
//=============================================================================
class ModelExporterR8 : public UtilityObj
{
private:
	Interface	*m_pMaxInterface;
	IUtil		*m_pMaxUtil;

	// Working MDF
	COutMDF					*m_pOutMDF;

	// Working model
	COutModel				*m_pOutModel;

	// Working clip
	COutClip				*m_pOutClip;

	// Message output
	string		m_sIndent;
	string		m_sOutput;

	// Nodes
	size_t			m_zNumNodes, n_zCurrentNode;
	vector<INode*>	m_vpBoneNodes;
	vector<INode*>	m_vpBoneParentNodes;

	void	BeginNode(INode *node, ENodeType nodetype);
	void	EndNode();

	// Materials
	map<tstring, COutMaterial>	m_mapMaterials;

	// Mesh
	void	AddMesh(INode *pNode);

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

public:
	HWND			m_hPanel;
	HWND			m_hMeshPanel;
	HWND			hStatus;

	// Mesh Panel
	void	UpdateMeshPanel();
	void	Mesh_ApplyCurrentSettings();
	void	Mesh_SetName(const tstring &sName);
	void	Mesh_SetVSHname(const tstring &sName);
	void	Mesh_SetPSHname(const tstring &sName);
	void	Mesh_SetProp(const tstring &sTag, bool b);
	void	Mesh_SetProp(const tstring &sTag, int i);
	void	Mesh_SetProp(const tstring &sTag, float f);
	void	Mesh_SetProp(const tstring &sTag, const tstring &s);
	void	Mesh_SetProp(const tstring &sTag, const char *sz);
	void	TagSelection(const tstring &sTag);

	// Message output
	void	AddMessage(const string &sMessage);
	void	Error(const string &sMessage);
	void	Indent()		{ if (m_sIndent.length() < 32) m_sIndent += "    "; }
	void	Unindent()		{ if (!m_sIndent.empty()) m_sIndent = m_sIndent.substr(0, m_sIndent.length() - 4); }
	void	ClearIndent()	{ m_sIndent.clear(); }

	// Objdef settings
	char			object_name[OBJECT_NAME_LENGTH];
	char			object_category[OBJECT_CATEGORY_LENGTH];
	float			scale_range_low;
	float			scale_range_high;

	// File options
	BOOL			export_objdef;
	BOOL			export_clip;
	char			filename[_MAX_PATH];

	char			error_message[ERROR_LOG_LENGTH];

	int				spritecount;
	int				keyframed_mesh_count;
	int				num_meshes;

	bool			model_error;

	// Message output
	const char*	GetOutptBuffer()	{ return m_sOutput.c_str(); }

	// Main panel functions
	void	Export(bool bAnimationOnly);
	void	WriteMaterials();
	void	Close()					{ m_pMaxUtil->CloseUtility(); }
	void	SceneInfo();

	void	ObjectSettings();			// Display object settings dialog

	void TraverseNode(INode *pNode);
	void CountNodes(INode *pNode);

	Modifier*	IsNodePhysiqued(INode *pNode);
	Modifier*	IsNodeSkinned(INode *pNode);

	BOOL	AddPhysiqueBones(IPhyContextExport *mcExport, int *num_blended_verts, int *num_nonblended_verts);
	bool	AddSkinBones(ISkin *pSkinInterface, ISkinContextData *pSkinCD, int *piNumBlended, int *piNumNonBlended);
	void	StoreBoneMotions();

	void	AddSprite(INode *node, int spritetype);
	void	AddSurf(INode *node);
	bool	AddSurfToModel(INode *node, struct SConvexPolyhedron *surf);
	INode*	GetBoneParentNode(INode *node);

	// Meshes
	bool	AddKeyframedMesh(COutMesh *pMesh, INode *pNode);
	bool	AddStaticMesh(COutMesh *pMesh, INode *pNode);
	bool	AddPhysiquedMesh(COutMesh *pMesh, INode *pNode, Modifier *pMod);
	bool	AddSkinnedMesh(COutMesh *pMesh, INode *pNode, Modifier *pMod);

	int	GetStartFrame()	{ return (m_pMaxInterface->GetAnimRange().Start() / GetTicksPerFrame()); }
	int GetEndFrame()	{ return (m_pMaxInterface->GetAnimRange().End() / GetTicksPerFrame()); }
	int GetNumFrames()	{ return (GetEndFrame() - GetStartFrame()) + 1; }

	void ClearSceneInfo();
	void Status(const char *msg, int progress);
	void SecondaryStatusText(const char *msg);
	void SecondaryStatusProgress(int progress);
	void OpenStatusBar();
	void CloseStatusBar();

	bool	GenerateVertexData(COutMesh *pMesh, INode *pNode, vector< vector<size_t> > &comparisonList);

	bool	FinishMesh(COutMesh *pMesh, INode *pNode);
	void	AddReferenceBone(INode *node);
	void	WriteClipFile();

	void	CopyKeys(INode *node, const COutBone *s2bone, SBoneMotion *motion, int frame);
	BOOL	GetFilename(char *fileTypeDesc, char *wildcard, char *extension);
	tstring	AddTextureReference(COutMesh *pMesh, INode *pNode);

	void ShowSceneInfo();
	void ShowErrorInfo();

	// Functions that manipulate the 'model' structure
	size_t			AddBone(INode *pNode);
	const COutBone*	GetBone(uint uiIndex);
	const COutBone*	GetBone(const tstring &sName);
	uint			GetBoneIndex(const tstring &sName);
	int				CountMeshType(int meshtype);
	void			FinishModel();
	void			Cleanup();

	void	MessageBox(const char *szMsg, const char *szTitle);

	// Required by UtilityObj
	void	BeginEditParams(Interface *pMaxInterface, IUtil *pMaxUtil);
	void	EndEditParams(Interface *pMaxInterface, IUtil *pMaxUtil);
	void	DeleteThis()	{}

	~ModelExporterR8();
	ModelExporterR8();
};
//=============================================================================

extern ModelExporterR8 g_S2ModelExporter;

#endif //__MAXEXPORTER_H__
