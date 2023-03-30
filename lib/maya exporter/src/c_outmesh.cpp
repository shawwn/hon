// (C)2008 S2 Games
// c_outmesh.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "c_outmesh.h"
//=============================================================================

/*====================
  COutMesh::COutMesh
  ====================*/
COutMesh::COutMesh() :
m_pModel(NULL),
m_uiNumVerts(0),

mode(0),
flags(0),
surfflags(0),
renderflags(0),
numFaces(0),
faceList(NULL),
num_uv_channels(0),
num_color_channels(0),
verts(NULL),
normals(NULL),
linkPool(NULL),
blendedLinks(NULL),
singleLinks(NULL),
bonelink(0)
{
	bbBounds.Zero();

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		tverts[n] = NULL;
		tangents[n] = NULL;
		signs[n] = NULL;
	}

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
		colors[n] = NULL;
}


/*====================
  COutMesh::~COutMesh
  ====================*/
COutMesh::~COutMesh()
{
}


/*====================
  COutMesh::Free
  ====================*/
void	COutMesh::Free()
{
	SAFE_DELETE_ARRAY(faceList);
	SAFE_DELETE_ARRAY(normals);
	SAFE_DELETE_ARRAY(verts);
	SAFE_DELETE_ARRAY(linkPool);
	SAFE_DELETE_ARRAY(blendedLinks);
	SAFE_DELETE_ARRAY(singleLinks);

	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(tverts[n]);
		SAFE_DELETE_ARRAY(tangents[n]);
		SAFE_DELETE_ARRAY(signs[n]);
	}

	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(colors[n]);
	}
}
