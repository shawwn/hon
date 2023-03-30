// (C)2006 S2 Games
// c_outmodel.h
//
//=============================================================================
#ifndef __C_OUTMODEL_H__
#define __C_OUTMODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_outbone.h"
#include "c_outmesh.h"

#include "c_convexpolyhedron.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
class INode;
class Mtl;
class CXMLDoc;

#pragma pack(1)
typedef struct modelHeader_s
{
	int version;
	int numMeshes;
	int num_sprites;
	int num_surfs;
	int numBones;
	CVec3f bmin;
	CVec3f bmax;
} modelHeader_t;

typedef struct boneBlock_s
{
	int	 parentIndex;
	matrix43_t invBase;				//invbase is stored as a 4x4 matrix in the model file
	matrix43_t base;				//base is stored as a 4x4 matrix in the model file

	// At the end, so we can save space
	byte		cNameLen;
	char		szBoneName[256];
} boneBlock_t;

struct SMeshBlock
{
	int	mesh_index;
	int mode;
	int num_verts;
	CVec3f bmin;
	CVec3f bmax;
	int bonelink;

	// At the end, so we can save space
	byte cNameLen;
	byte cShaderNameLen;
	char szNameBuffer[512];
};

typedef struct blendedLinksBlock_s
{
	int mesh_index;
	int num_verts;
} blendedLinksBlock_t;

typedef struct singleLinksBlock_s
{
	int mesh_index;
	int num_verts;
} singleLinksBlock_t;

typedef struct faceBlock_s
{
	int mesh_index;
	int numFaces;
	char faceIndexSize;
} faceBlock_t;

typedef struct textureCoordsBlock_s
{
	int		mesh_index;
	int		channel;
} textureCoordsBlock_t;

typedef struct tangentBlock_s
{
	int		mesh_index;
	int		channel;
} tangentBlock_t;

typedef struct colorBlock_s
{
	int		mesh_index;
	int		channel;
} colorBlock_t;

typedef struct normalBlock_s
{
	int		mesh_index;
} normalBlock_t;

typedef struct surfBlock_s
{
	int surf_index;
	int num_planes;
	int num_points;
	int num_edges;
	int num_tris;
	CVec3f bmin;
	CVec3f bmax;
	int flags;
} surfBlock_t;

typedef struct vertexBlock_s
{
	int mesh_index;
}
vertexBlock_t;
#pragma pack()
//=============================================================================

//=============================================================================
// COutModel
//=============================================================================
class COutModel
{
private:
	bool	m_bIsValid;
	tstring	m_sStatus;

	CBBoxf	m_bbBounds;

	vector<COutBone>		m_vBones;
	map<tstring, uint>	m_mapBoneIndices;

	vector<COutMesh>		m_vMeshes;

	vector<CConvexPolyhedron>	m_vSurfs;

	int		seekback;		// position in the file to fill in the block length

	void	BeginBlock(const char *blockname, FILE *stream);
	void	WriteBlockData(void *data, int size, FILE *stream);
	void	EndBlock(FILE *stream);

public:
	~COutModel()						{}
	COutModel();
	COutModel(INode *pNode, TimeValue time);

	void			Validate()			{ m_bIsValid = true; }
	void			Invalidate()		{ m_bIsValid = false; }
	bool			IsValid() const		{ return m_bIsValid; }

	void	AddPointToBounds(const CVec3f &v3Point)	{ m_bbBounds.AddPoint(v3Point); }
	void	AddBounds(const CBBoxf &bbBounds)		{ m_bbBounds.AddBox(bbBounds); }
	const CBBoxf&	GetBounds()						{ return m_bbBounds; }

	// Bones
	uint	GetNumBones();
	COutBone*	GetBone(uint uiBone);
	COutBone*	GetBone(const tstring &sBone);
	uint	GetBoneIndex(const tstring &sBone);
	uint	AddBone(const COutBone &newBone);

	// Meshes
	void	AddMesh(const COutMesh &newMesh);
	uint	GetNumMeshes() const;
	COutMesh*	GetMesh(uint uiIndex);
	COutMesh*	GetMesh(const tstring &sName);
	uint	GetMeshIndex(const tstring &sName);

	// Surfs
	void				AddSurf(const CConvexPolyhedron &newSurf);
	uint				GetNumSurfs() const;
	CConvexPolyhedron*	GetSurf(uint uiIndex);

	tstring	WriteFile(const tstring &sFilename);
};
//=============================================================================
#endif //__C_OUTMODEL_H__
