// c_outmesh.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_outmesh.h"

#include "s_blendedlink.h"
#include "nvtristrip/NvTriStrip.h"
//=============================================================================

/*====================
  COutMesh::COutMesh
  ====================*/
COutMesh::COutMesh() :
m_pModel(NULL),

mode(0),
numFaces(0),
faceList(NULL),
num_verts(0),
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


/*====================
  COutMesh::ReverseWinding
  ====================*/
void	COutMesh::ReverseWinding()
{
	if (faceList == NULL)
		return;

	for (int f(0); f < numFaces; ++f)
		SWAP(faceList[f][1], faceList[f][2]);
}


/*====================
  COutMesh::ReverseNormals
  ====================*/
void	COutMesh::ReverseNormals()
{
	if (normals == NULL)
		return;

	for (int n(0); n < num_verts; ++n)
		normals[n] *= -1.0f;
}


/*====================
  COutMesh::ExpandVertexes

  Puts the mesh vertexes into face order (faceNum * 3 + vertexIndex)
  ====================*/
void	COutMesh::ExpandVertexes()
{
	//
	// Alloc new data
	//

	int iNewNumVerts(numFaces * 3);

	CVec3ui *pNewFaceList(new CVec3ui[numFaces]);
	CVec3f *pNewNormals(new CVec3f[iNewNumVerts]);
	CVec3f *pNewVerts(new CVec3f[iNewNumVerts]);
	
	byte *pNewLinkPool(new byte[iNewNumVerts * 32]);
	int iPoolPos(0);
	SBlendedLink *pNewBlendedLinks(blendedLinks != NULL ? new SBlendedLink[iNewNumVerts] : NULL);

	singleLink_t *pNewSingleLinks(singleLinks != NULL ? new singleLink_t[iNewNumVerts] : NULL);
	
	CVec2f *pNewTVerts[MAX_UV_CHANNELS];
	CVec3f *pNewTangents[MAX_UV_CHANNELS];
	for (int n(0); n < MAX_UV_CHANNELS; ++n)
	{
		pNewTVerts[n] = tverts[n] != NULL ? new CVec2f[iNewNumVerts] : NULL;
		pNewTangents[n] = tangents[n] != NULL ? new CVec3f[iNewNumVerts] : NULL;
	}

	CVec4b *pNewColors[MAX_VERTEX_COLOR_CHANNELS];
	for (int n(0); n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		pNewColors[n] = colors[n] != NULL ? new CVec4b[iNewNumVerts] : NULL;
	}

	//
	// Copy data
	//

	uint *pFaceOrderMap(new uint[iNewNumVerts]);
	for (int v(0); v < iNewNumVerts; v += 3)
	{
		int iOwner(v / 3);

		pFaceOrderMap[v + 0] = faceList[iOwner][0];
		pFaceOrderMap[v + 1] = faceList[iOwner][1];
		pFaceOrderMap[v + 2] = faceList[iOwner][2];
	}

	for (int f(0); f < numFaces; ++f)
	{
		pNewFaceList[f][0] = f * 3 + 0;
		pNewFaceList[f][1] = f * 3 + 1;
		pNewFaceList[f][2] = f * 3 + 2;
	}

	for (int v(0); v < iNewNumVerts; ++v)
		pNewNormals[v] = normals[pFaceOrderMap[v]];

	for (int v(0); v < iNewNumVerts; ++v)
		pNewVerts[v] = verts[pFaceOrderMap[v]];

	if (blendedLinks != NULL)
	{
		for (int v(0); v < iNewNumVerts; ++v)
		{
			SBlendedLink &cBlendLink(blendedLinks[pFaceOrderMap[v]]);

			pNewBlendedLinks[v].num_weights = cBlendLink.num_weights;
			
			pNewBlendedLinks[v].weights = (float *)(&pNewLinkPool[iPoolPos]);
			iPoolPos += cBlendLink.num_weights * sizeof(float);
			memcpy(pNewBlendedLinks[v].weights, cBlendLink.weights, cBlendLink.num_weights * sizeof(float));

			pNewBlendedLinks[v].indexes = (uint *)(&pNewLinkPool[iPoolPos]);
			iPoolPos += cBlendLink.num_weights * sizeof(uint);
			memcpy(pNewBlendedLinks[v].indexes, cBlendLink.indexes, cBlendLink.num_weights * sizeof(uint));
		}
	}

	if (singleLinks != NULL)
	{
		for (int v(0); v < iNewNumVerts; ++v)
			pNewSingleLinks[v] = singleLinks[pFaceOrderMap[v]];
	}

	for (int n(0); n < MAX_UV_CHANNELS; ++n)
	{
		if (tverts[n] != NULL)
		{
			for (int v(0); v < iNewNumVerts; ++v)
				pNewTVerts[n][v] = tverts[n][pFaceOrderMap[v]];
		}

		if (tangents[n] != NULL)
		{
			for (int v(0); v < iNewNumVerts; ++v)
				pNewTangents[n][v] = tangents[n][pFaceOrderMap[v]];
		}
	}

	for (int n(0); n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		if (colors[n] != NULL)
		{
			for (int v(0); v < iNewNumVerts; ++v)
				pNewColors[n][v] = colors[n][pFaceOrderMap[v]];
		}
	}

	//
	// Delete old data
	//

	SAFE_DELETE_ARRAY(faceList);
	SAFE_DELETE_ARRAY(normals);
	SAFE_DELETE_ARRAY(verts);
	SAFE_DELETE_ARRAY(linkPool);
	SAFE_DELETE_ARRAY(blendedLinks);
	SAFE_DELETE_ARRAY(singleLinks);

	for (int n(0); n < MAX_UV_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(tverts[n]);
		SAFE_DELETE_ARRAY(tangents[n]);
	}

	for (int n(0); n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		SAFE_DELETE_ARRAY(colors[n]);
	}

	//
	// Assign new data
	//

	num_verts = iNewNumVerts;
	faceList = pNewFaceList;
	normals = pNewNormals;
	verts = pNewVerts;
	linkPool = pNewLinkPool;
	blendedLinks = pNewBlendedLinks;
	singleLinks = pNewSingleLinks;

	for (int n(0); n < MAX_UV_CHANNELS; ++n)
	{
		tverts[n] = pNewTVerts[n];
		tangents[n] = pNewTangents[n];
	}

	for (int n(0); n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		colors[n] = pNewColors[n];
	}
}


/*====================
  COutMesh::CalcMeshTangents

  This assumes the vertexes are in face order (faceNum * 3 + vertexIndex)
  ====================*/
void	COutMesh::CalcMeshTangents()
{
	const float MAX_TANGENT_SMOOTH_ANGLE = 135.0f;

	if (normals == NULL || num_verts != numFaces * 3)
		return;

	for (int t(0); t < MAX_UV_CHANNELS; ++t)
	{
		if (!tverts[t])
			continue;

		tangents[t] = new CVec3f[num_verts];
		signs[t] = new byte[num_verts];

		for (int v(0); v < num_verts; ++v)
		{
			int iOwner(v / 3);
			CVec3f ov0(verts[faceList[iOwner][0]][0], verts[faceList[iOwner][0]][1], verts[faceList[iOwner][0]][2]);
			CVec3f ov1(verts[faceList[iOwner][1]][0], verts[faceList[iOwner][1]][1], verts[faceList[iOwner][1]][2]);
			CVec3f ov2(verts[faceList[iOwner][2]][0], verts[faceList[iOwner][2]][1], verts[faceList[iOwner][2]][2]);

			CVec2f ot0(tverts[t][faceList[iOwner][0]][0], tverts[t][faceList[iOwner][0]][1]);
			CVec2f ot1(tverts[t][faceList[iOwner][1]][0], tverts[t][faceList[iOwner][1]][1]);
			CVec2f ot2(tverts[t][faceList[iOwner][2]][0], tverts[t][faceList[iOwner][2]][1]);

			bool bOwnerSign(((ot1.x - ot0.x) * (ot2.y - ot0.y) - (ot1.y - ot0.y) * (ot2.x - ot0.x)) > 0.0f);

			signs[t][v] = bOwnerSign ? 255 : 0;

			CVec3f vOwnerTangent(M_GetFaceTangent(ov0, ov1, ov2, ot0, ot1, ot2));

			// Figure out which faces this vertex belongs to
			ivector vFaceList;

			for (int f(0); f < numFaces; ++f)
			{
				// make a list of any face that contains a vert that matches our position and normal
				if ((memcmp(&verts[faceList[f][0]], &verts[v], sizeof(CVec3f)) == 0 ||
					memcmp(&verts[faceList[f][1]], &verts[v], sizeof(CVec3f)) == 0 ||
					memcmp(&verts[faceList[f][2]], &verts[v], sizeof(CVec3f)) == 0) &&
					(memcmp(&normals[faceList[f][0]], &normals[v], sizeof(CVec3f)) == 0 ||
					memcmp(&normals[faceList[f][1]], &normals[v], sizeof(CVec3f)) == 0 ||
					memcmp(&normals[faceList[f][2]], &normals[v], sizeof(CVec3f)) == 0))
				{
					vFaceList.push_back(f);
				}
			}

			// Calculate a tangent for each face then smooth the tangents together
			CVec3f vFinalTangent(0.0f, 0.0f, 0.0f);

			for (ivector::iterator it(vFaceList.begin()); it != vFaceList.end(); ++it)
			{
				int iFace(*it);

				CVec3f v0(verts[faceList[iFace][0]][0], verts[faceList[iFace][0]][1], verts[faceList[iFace][0]][2]);
				CVec3f v1(verts[faceList[iFace][1]][0], verts[faceList[iFace][1]][1], verts[faceList[iFace][1]][2]);
				CVec3f v2(verts[faceList[iFace][2]][0], verts[faceList[iFace][2]][1], verts[faceList[iFace][2]][2]);

				CVec2f t0(tverts[t][faceList[iFace][0]][0], tverts[t][faceList[iFace][0]][1]);
				CVec2f t1(tverts[t][faceList[iFace][1]][0], tverts[t][faceList[iFace][1]][1]);
				CVec2f t2(tverts[t][faceList[iFace][2]][0], tverts[t][faceList[iFace][2]][1]);

				CVec3f vTangent(M_GetFaceTangent(v0, v1, v2, t0, t1, t2));

				bool bSign(((t1.x - t0.x) * (t2.y - t0.y) - (t1.y - t0.y) * (t2.x - t0.x)) > 0.0f);
				
				if (bSign != bOwnerSign)
					vTangent *= -1.0f;

				if (DotProduct(vTangent, vOwnerTangent) >= cos(DEG2RAD(MAX_TANGENT_SMOOTH_ANGLE)))
					vFinalTangent += vTangent;
			}

			vFinalTangent.Normalize();

			// do a quick Gram-Schmidt orthogonalize
			vFinalTangent -= normals[v] * DotProduct(normals[v], vFinalTangent);
			vFinalTangent.Normalize();

			tangents[t][v] = vFinalTangent;
		}
	}
}


/*====================
  COutMesh::Optimize

  Removes duplicate vertexes and reorders using NvTriStrip
  ====================*/
void	COutMesh::Optimize()
{
	map<int, int> mapVerts;
	for (int v = 0; v < num_verts; ++v)
	{
		for (int w = v + 1; w < num_verts; ++w)
		{
			bool bMatch = true;

			// Check normals
			if (normals != NULL)
			{
				if (memcmp(normals[v], normals[w], sizeof(CVec3f)) != 0)
					continue;
			}

			// Check normals
			if (verts != NULL)
			{
				if (memcmp(verts[v], verts[w], sizeof(CVec3f)) != 0)
					continue;
			}

			// Check mapping coordinates
			for (int c = 0; c < MAX_UV_CHANNELS; ++c)
			{
				if (tverts[c] == NULL)
					continue;

				if (memcmp(tverts[c][v], tverts[c][w], sizeof(CVec2f)) != 0)
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
				if (tangents[c] == NULL)
					continue;

				if (!Compare(tangents[c][v], tangents[c][w], 0.0001f))
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
				if (signs[c] == NULL)
					continue;

				if (signs[c][v] != signs[c][w])
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
				if (colors[c] == NULL)
					continue;

				if (memcmp(colors[c][v], colors[c][w], sizeof(CVec4b)) != 0)
				{
					bMatch = false;
					break;
				}
			}
			if (!bMatch)
				continue;

			// Check blended links
			if (blendedLinks != NULL)
			{
				if (blendedLinks[v].num_weights != blendedLinks[w].num_weights)
					continue;

				if (memcmp(blendedLinks[v].weights, blendedLinks[w].weights, blendedLinks[v].num_weights * sizeof(float)) != 0)
					continue;
				if (memcmp(blendedLinks[v].indexes, blendedLinks[w].indexes, blendedLinks[v].num_weights * sizeof(uint)) != 0)
					continue;
			}

			// Check single links
			if (singleLinks != NULL)
			{
				if (memcmp(&singleLinks[v], &singleLinks[w], sizeof(singleLink_t)) != 0)
					continue;
			}

			// Add to remapping list
			mapVerts[w] = v;
		}
	}

	// Finish the map, accounting for a condensed vert list
	int count(0);
	for (int v = 0; v < num_verts; ++v)
	{
		if (mapVerts.find(v) != mapVerts.end())
		{
			mapVerts[v] = mapVerts[mapVerts[v]];
			continue;
		}

		mapVerts[v] = count;
		if (v != count)
		{
			memcpy(verts[count], verts[v], sizeof(CVec3f));

			if (normals != NULL)
				memcpy(normals[count], normals[v], sizeof(CVec3f));

			for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
			{
				if (colors[n] == NULL)
					continue;

				memcpy(colors[n][count], colors[n][v], sizeof(CVec4b));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (tverts[n] == NULL)
					continue;

				memcpy(tverts[n][count], tverts[n][v], sizeof(CVec2f));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (tangents[n] == NULL)
					continue;

				memcpy(tangents[n][count], tangents[n][v], sizeof(CVec3f));
			}

			for (int n = 0; n < MAX_UV_CHANNELS; ++n)
			{
				if (signs[n] == NULL)
					continue;

				signs[n][count] = signs[n][v];
			}

			if (blendedLinks != NULL)
				blendedLinks[count] = blendedLinks[v];

			if (singleLinks != NULL)
				singleLinks[count] = singleLinks[v];
		}
		++count;
	}
	num_verts = count;

	// Fix the facelist
	for (int f = 0; f < numFaces; ++f)
	{
		for (int n = 0; n < 3; ++n)
			faceList[f][n] = mapVerts[faceList[f][n]];
	}

	// Optimize using NvTriStrip
	SetCacheSize(CACHESIZE_GEFORCE3);
	SetStitchStrips(true);
	SetMinStripSize(0);
	SetListsOnly(true);

	unsigned short	usNumSections;
	PrimitiveGroup	*pPrimitiveGroupsList;

	GenerateStrips((uint *)this->faceList, this->numFaces * 3, &pPrimitiveGroupsList, &usNumSections);

	unsigned int	*pRemappedIndices;

	RemapIndices(pPrimitiveGroupsList, usNumSections, this->numFaces * 3, &pRemappedIndices);

	// Reorder primitive groups
	for (int g(0); g < usNumSections; ++g)
		for (int i(0); i < int(pPrimitiveGroupsList[g].numIndices); ++i)
			pPrimitiveGroupsList[g].indices[i] = pRemappedIndices[pPrimitiveGroupsList[g].indices[i]];

	// Reorder verticies
	CVec3f *verts(new CVec3f[this->num_verts]);
	memcpy(verts, this->verts, sizeof(CVec3f) * this->num_verts);

	CVec3f *normals(NULL);
	if (this->normals)
	{
		normals = new CVec3f[this->num_verts];
		memcpy(normals, this->normals, sizeof(CVec3f) * this->num_verts);
	}

	CVec4b *colors[MAX_VERTEX_COLOR_CHANNELS];
	for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
	{
		if (this->colors[n])
		{
			colors[n] = new CVec4b[this->num_verts];
			memcpy(colors[n], this->colors[n], sizeof(CVec4b) * this->num_verts);
		}
		else
			colors[n] = NULL;
	}

	CVec2f *tverts[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (this->tverts[n])
		{
			tverts[n] = new CVec2f[this->num_verts];
			memcpy(tverts[n], this->tverts[n], sizeof(CVec2f) * this->num_verts);
		}
		else
			tverts[n] = NULL;
	}

	CVec3f *tangents[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (this->tangents[n])
		{
			tangents[n] = new CVec3f[this->num_verts];
			memcpy(tangents[n], this->tangents[n], sizeof(CVec3f) * this->num_verts);
		}
		else
			tangents[n] = NULL;
	}

	byte *signs[MAX_UV_CHANNELS];
	for (int n = 0; n < MAX_UV_CHANNELS; ++n)
	{
		if (this->signs[n])
		{
			signs[n] = new byte[this->num_verts];
			memcpy(signs[n], this->signs[n], sizeof(byte) * this->num_verts);
		}
		else
			signs[n] = NULL;
	}

	SBlendedLink *blendedLinks(NULL);
	if (this->blendedLinks)
	{
		blendedLinks = new SBlendedLink[this->num_verts];
		memcpy(blendedLinks, this->blendedLinks, sizeof(SBlendedLink) * this->num_verts);
	}

	singleLink_t *singleLinks(NULL);
	if (this->singleLinks)
	{
		singleLinks = new singleLink_t[this->num_verts];
		memcpy(singleLinks, this->singleLinks, sizeof(singleLinks) * this->num_verts);
	}

	for (int v = 0; v < this->num_verts; ++v)
	{
		unsigned int v2(pRemappedIndices[v]);

		memcpy(this->verts[v2], verts[v], sizeof(CVec3f));

		if (this->normals)
			memcpy(this->normals[v2], normals[v], sizeof(CVec3f));

		for (int n = 0; n < MAX_VERTEX_COLOR_CHANNELS; ++n)
		{
			if (this->colors[n])
				memcpy(this->colors[n][v2], colors[n][v], sizeof(CVec4b));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (this->tverts[n])
				memcpy(this->tverts[n][v2], tverts[n][v], sizeof(CVec2f));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (this->tangents[n])
				memcpy(this->tangents[n][v2], tangents[n][v], sizeof(CVec3f));
		}

		for (int n = 0; n < MAX_UV_CHANNELS; ++n)
		{
			if (this->signs[n])
				this->signs[n][v2] = signs[n][v];
		}

		if (this->blendedLinks)
			this->blendedLinks[v2] = blendedLinks[v];

		if (this->singleLinks)
			this->singleLinks[v2] = singleLinks[v];
	}

	memcpy(this->faceList, pPrimitiveGroupsList[0].indices, sizeof(uint) * pPrimitiveGroupsList[0].numIndices);

	delete[] pPrimitiveGroupsList;
	delete[] pRemappedIndices;
}