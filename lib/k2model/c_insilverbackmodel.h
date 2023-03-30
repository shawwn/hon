// c_insilverbackmodel.h
//
//=============================================================================
#ifndef __C_INSILVERBACKMODEL_H__
#define __C_INSILVERBACKMODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include <c_filehandle.h>

#include "c_outbone.h"
#include "c_outmesh.h"

#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
class CXMLDoc;

const int	MAX_POLYHEDRON_PLANES(128);
const int	MAX_BONES(128);
const int	MAX_SKINS_PER_MODEL(8);
const int	BONE_NAME_LENGTH(32);
const int	MESH_NAME_LENGTH(32);
const int	SKIN_NAME_LENGTH(32);
const int	MAX_RENDER_DATA(8);
const int	SURF_LIMIT(256);
const int	SKIN_SHADERNAME_LENGTH(64);

typedef float		vec_t;
typedef vec_t		vec4_t[4];

//reference as axis and position
typedef struct
{
	vec4_t			axis[3];
	vec4_t			pos;
} _matrix44_t;

//reference as list of floats
typedef struct
{
	float f00;
	float f01;
	float f02;
	float f03;
	float f10;
	float f11;
	float f12;
	float f13;
	float f20;
	float f21;
	float f22;
	float f23;
	float f30;
	float f31;
	float f32;
	float f33;
} __matrix44_t;

//4x4 matrix, can be referred to either by 'axis' and 'pos' or through the 'matrix' array
typedef union
{
	_matrix44_t t;
	float		matrix[16];		//reference as array
	__matrix44_t f;
} matrix44_t;

#pragma pack(1)
struct SModelHeaderSilverback
{
	int version;
	int numMeshes;
	int num_sprites;
	int num_surfs;
	int numBones;
	CVec3f bmin;
	CVec3f bmax;
};

struct SBoneBlockSilverback
{
	int parentIndex;
	char name[BONE_NAME_LENGTH];
	matrix44_t invBase;				//invbase is stored as a 4x4 matrix in the model file
	matrix44_t base;				//base is stored as a 4x4 matrix in the model file
};

struct SMeshBlockSilverback
{
	int	mesh_index;
	char name[MESH_NAME_LENGTH];
	char defaultShader[SKIN_SHADERNAME_LENGTH];
	int mode;
	int num_verts;
	CVec3f bmin;
	CVec3f bmax;
	int bonelink;
};

struct SBlendedLinksBlockSilverback
{
	int mesh_index;
	int num_verts;
};

struct SSingleLinksBlockSilverback
{
	int mesh_index;
	int num_verts;
};

struct SFaceBlockSilverback
{
	int mesh_index;
	int num_faces;
};

struct STextureCoordsBlockSilverback
{
	int mesh_index;
};

struct SColorBlockSilverback
{
	int mesh_index;
};

struct SNormalBlockSilverback
{
	int mesh_index;
};

struct SSurfBlockSilverback
{
	int surf_index;
	int num_planes;
	CVec3f bmin;
	CVec3f bmax;
	int flags;
};

struct SVertexBlockSilverback
{
	int mesh_index;
};
#pragma pack()
//=============================================================================

//=============================================================================
// CInSilverbackModel
//=============================================================================
class CInSilverbackModel
{
private:
	COutModel				*m_pOutModel;

	static const char		s_ayHeader[4];

	int		m_iNumBones;

	bool	ParseHeader(const SFileBlock &cBlock);
	bool	ParseBoneBlock(const SFileBlock &cBlock);
	bool	ParseMeshBlock(const SFileBlock &cBlock);
	bool	ParseVertexBlock(const SFileBlock &cBlock);
	bool	ParseBlendedLinksBlock(const SFileBlock &cBlock);
	bool	ParseSingleLinksBlock(const SFileBlock &cBlock);
	bool	ParseTextureCoordBlock(const SFileBlock &cBlock);
	bool	ParseColorBlock(const SFileBlock &cBlock);
	bool	ParseNormalBlock(const SFileBlock &cBlock);
	bool	ParseFaceBlock(const SFileBlock &cBlock);
	bool	ParseSurfBlock(const SFileBlock &cBlock);

	bool	ReadBlocks(FileBlockList &vBlockList);

public:
	~CInSilverbackModel()	{}
	CInSilverbackModel();

	void	SetOutModel(COutModel *pOutModel)		{ m_pOutModel = pOutModel; }

	void	ReadFile(const tstring &sFilename);
};
//=============================================================================

#endif //__C_INSILVERBACKMODEL_H__
