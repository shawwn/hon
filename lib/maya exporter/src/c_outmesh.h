// (C)2008 S2 Games
// c_outmesh.h
//
//=============================================================================
#ifndef __C_OUTMESH_H__
#define __C_OUTMESH_H__

//=============================================================================
// Definitions
//=============================================================================
typedef uint singleLink_t;

//a mesh is like a "sub object" of a model
//meshes may use any of the 3 following "modes":
//
//MESH_SKINNED_BLENDED
//	each vertex on the mesh is linked to one or more bones
//  if any one vertex in the mesh is linked to more than one bone, this is the mode that gets set
//MESH_SKINNED_NONBLENDED
//  one bone link per vertex
//  this mode gets set for all other meshes, including meshes that did not have physique applied
//  (a bone will be generated for this mesh that all vertices are linked to)
//  even meshes which use keyframe data will have their geometry set (though it may not get used)

typedef enum
{
	MESH_SKINNED_BLENDED = 1,
	MESH_SKINNED_NONBLENDED
}
meshModes_enum;

enum EMeshFlags
{
	MESH_GPU_DEFORM = BIT(0),
	MESH_INVIS = BIT(1)
};

const int MAX_UV_CHANNELS(8);
const int MAX_VERTEX_COLOR_CHANNELS(2);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class COutModel;
//=============================================================================

//=============================================================================
// COutMesh
//=============================================================================
class COutMesh
{
private:
	tstring		m_sName;
	tstring		m_sDefaultShaderName;

	COutModel*	m_pModel;		// Owner of this mesh

	uint		m_uiNumVerts;	//number of vertices

public:
	~COutMesh();
	COutMesh();

	void			Free();

	const tstring&	GetName() const								{ return m_sName; }
	void			SetName(const tstring &sName)				{ m_sName = sName; }

	COutModel*		GetModel() const							{ return m_pModel; }
	void			SetModel(COutModel *pModel)					{ m_pModel = pModel; }

	tstring			GetDefaultShaderName() const				{ return m_sDefaultShaderName; }
	void			SetDefaultShaderName(const tstring &sName)	{ m_sDefaultShaderName = sName; }

	void			SetVertexCount(uint uiCount)				{ m_uiNumVerts = uiCount; }
	uint			GetVertexCount() const						{ return m_uiNumVerts; }

	int				mode;		//see MESH_* defines above

	int				flags;
	int				surfflags;
	int				renderflags;

	CBBoxf			bbBounds;	// bounding box (in MESH coordinates)

	int				numFaces;
	CVec3ui			*faceList;

	int				num_uv_channels;
	int				num_color_channels;

	CVec3f*			verts;		// Vertex coords (always in MODEL space)
	CVec3f*			normals;	// Vertex normals
	CVec4b*			colors[MAX_VERTEX_COLOR_CHANNELS];	// Vertex colors
	CVec2f*			tverts[MAX_UV_CHANNELS];	// Texture coords
	CVec3f*			tangents[MAX_UV_CHANNELS];	// Vertex tangents
	byte*			signs[MAX_UV_CHANNELS];	// Sign of the texture matrix

	byte*			linkPool;		//blendedLinks sets pointers here, rather than allocating its own memory
	SBlendedLink*	blendedLinks;	//vertBlend only allocated if mode == MESH_SKINNED_BLENDED

	singleLink_t*	singleLinks;	//allocated for nonblended meshes.  can also be used as an LOD fallback for blended meshes.

	int				bonelink;		//if > -1, ALL vertices of this mesh are linked to this bone only, in which case both
									//singleLinks and blendedLinks will be NULL.
};
//=============================================================================

#endif // __C_OUTMESH_H__
