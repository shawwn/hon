// c_ink2v3model.h
//
//=============================================================================
#ifndef __C_INK2V3MODEL_H__
#define __C_INK2V3MODEL_H__

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

#pragma pack(1)
struct SModelHeaderK2v3
{
	int		version;
	int		numMeshes;
	int		num_sprites;
	int		num_surfs;
	int		numBones;
	CVec3f	bmin;
	CVec3f	bmax;
};

struct SBoneBlockK2v3
{
	int			parentIndex;
	matrix43_t	invBase;				//invbase is stored as a 4x4 matrix in the model file
	matrix43_t	base;				//base is stored as a 4x4 matrix in the model file

	// At the end, so we can save space
	byte		cNameLen;
	char		szBoneName[256];
};

struct SMeshBlockK2v3
{
	int		mesh_index;
	int		mode;
	int		num_verts;
	CVec3f	bmin;
	CVec3f	bmax;
	int		bonelink;

	// At the end, so we can save space
	byte	cNameLen;
	byte	cShaderNameLen;
	char	szNameBuffer[512];
};

struct SBlendedLinksBlockK2v3
{
	int		mesh_index;
	int		num_verts;
};

struct SSingleLinksBlockK2v3
{
	int		mesh_index;
	int		num_verts;
};

struct SFaceBlockK2v3
{
	int		mesh_index;
	int		numFaces;
	char	faceIndexSize;
};

struct STextureCoordsBlockK2v3
{
	int		mesh_index;
	int		channel;
};

struct STangentBlockK2v3
{
	int		mesh_index;
	int		channel;
};

struct SColorBlockK2v3
{
	int		mesh_index;
	int		channel;
};

struct SNormalBlockK2v3
{
	int		mesh_index;
};

struct SSurfBlockK2v3
{
	int		surf_index;
	int		num_planes;
	int		num_points;
	int		num_edges;
	int		num_tris;
	CVec3f	bmin;
	CVec3f	bmax;
	int		flags;
};

struct SVertexBlockK2v3
{
	int		mesh_index;
};
#pragma pack()
//=============================================================================

//=============================================================================
// CInK2v3Model
//=============================================================================
class CInK2v3Model
{
private:
	COutModel				*m_pOutModel;

	static const char		s_ayHeader[4];

	bool	ParseHeader(const SFileBlock &cBlock);
	bool	ParseBoneBlock(const SFileBlock &cBlock);
	bool	ParseMeshBlock(const SFileBlock &cBlock);
	bool	ParseVertexBlock(const SFileBlock &cBlock);
	bool	ParseBlendedLinksBlock(const SFileBlock &cBlock);
	bool	ParseSingleLinksBlock(const SFileBlock &cBlock);
	bool	ParseTextureCoordBlock(const SFileBlock &cBlock);
	bool	ParseTangentBlock(const SFileBlock &cBlock);
	bool	ParseColorBlock(const SFileBlock &cBlock);
	bool	ParseNormalBlock(const SFileBlock &cBlock);
	bool	ParseFaceBlock(const SFileBlock &cBlock);
	bool	ParseSurfBlock(const SFileBlock &cBlock);

	bool	ReadBlocks(FileBlockList &vBlockList);

public:
	~CInK2v3Model()						{}
	CInK2v3Model();

	void	SetOutModel(COutModel *pOutModel)		{ m_pOutModel = pOutModel; }

	void	ReadFile(const tstring &sFilename);
};
//=============================================================================

#endif //__C_INK2V3MODEL_H__
