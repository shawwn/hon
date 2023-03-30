// c_outmesh.h
//
//=============================================================================
#ifndef __C_OUTMESH_H__
#define __C_OUTMESH_H__

//=============================================================================
// Definitions
//=============================================================================
typedef uint singleLink_t;

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

	COutModel	*m_pModel;		// Owner of this mesh

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

	void			ReverseWinding();
	void			ReverseNormals();
	void			ExpandVertexes();
	void			CalcMeshTangents();
	void			Optimize();

	int				mode;		//see MESH_* defines above

	CBBoxf			bbBounds;	// bounding box (in MESH coordinates)

	int				numFaces;
	CVec3ui			*faceList;

	int				num_verts;	//number of vertices

	CVec3f			*verts;		// Vertex coords (always in MODEL space)
	CVec3f			*normals;	// Vertex normals
	CVec4b			*colors[MAX_VERTEX_COLOR_CHANNELS];	// Vertex colors
	CVec2f			*tverts[MAX_UV_CHANNELS];	// Texture coords
	CVec3f			*tangents[MAX_UV_CHANNELS];	// Vertex tangents
	byte			*signs[MAX_UV_CHANNELS];	// Sign of texture space matrix

	byte			*linkPool;		//blendedLinks sets pointers here, rather than allocating its own memory
	struct SBlendedLink	*blendedLinks;	//vertBlend only allocated if mode == MESH_SKINNED_BLENDED

	singleLink_t	*singleLinks;	//allocated for nonblended meshes.  can also be used as an LOD fallback for blended meshes.

	int				bonelink;		//if > -1, ALL vertices of this mesh are linked to this bone only, in which case both
									//singleLinks and blendedLinks will be NULL.
};
//=============================================================================

#endif // __C_OUTMESH_H__
