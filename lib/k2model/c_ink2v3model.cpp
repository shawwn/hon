// c_ink2v3model.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_ink2v3model.h"

#include "c_outmodel.h"

#include <s_blendedlink.h>
#include <c_filehandle.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
const char	CInK2v3Model::s_ayHeader[4] = {'S', 'M', 'D', 'L'};
//=============================================================================


/*====================
  CInK2v3Model::CInK2v3Model
  ====================*/
CInK2v3Model::CInK2v3Model()
{
}


/*====================
  CInK2v3Model::ParseHeader
  ====================*/
bool	CInK2v3Model::ParseHeader(const SFileBlock &cBlock)
{
	SModelHeaderK2v3 &cIn(*(SModelHeaderK2v3 *)cBlock.pData);

	if (cBlock.uiLength < sizeof(SModelHeaderK2v3))
	{
		cerr << _T("Bad model header") << endl;
		return false;
	}

	int iVersion(LittleInt(cIn.version));
	if (iVersion != 3)
	{
		if (iVersion != 3)
			cerr << _T("Invalid model version: ") << iVersion;

		cerr << _T(", expected version is: ") << 3 << endl;
		return false;
	}

	CVec3f v3Min(LittleFloat(cIn.bmin[X]), LittleFloat(cIn.bmin[Y]), LittleFloat(cIn.bmin[Z]));
	CVec3f v3Max(LittleFloat(cIn.bmax[X]), LittleFloat(cIn.bmax[Y]), LittleFloat(cIn.bmax[Z]));
	//FixModelCoord(v3Min);
	//FixModelCoord(v3Max);
	m_pOutModel->SetBounds(CBBoxf(v3Min, v3Max));
	
	m_pOutModel->AllocMeshes(LittleInt(cIn.numMeshes));

	return true;
}


/*====================
  CInK2v3Model::ParseBoneBlock
  ====================*/
bool	CInK2v3Model::ParseBoneBlock(const SFileBlock &cBlock)
{
	SBoneBlockK2v3 *pIn((SBoneBlockK2v3 *)cBlock.pData);

	// Read each bone
	for (uint uiIndex(0); (byte *)pIn - cBlock.pData < int(cBlock.uiLength); ++uiIndex)
	{
		// Initialize new bone
		tstring sBoneName;
		StrToTString(sBoneName, pIn->szBoneName);
		COutBone cOutBone(uiIndex, sBoneName, pIn->parentIndex);

		for (int i(0); i < 3; ++i)
		{
			cOutBone.m_invBase.pos[i] = LittleFloat(pIn->invBase.pos[i]);
			cOutBone.m_invBase.axis[0][i] = LittleFloat(pIn->invBase.axis[0][i]);
			cOutBone.m_invBase.axis[1][i] = LittleFloat(pIn->invBase.axis[1][i]);
			cOutBone.m_invBase.axis[2][i] = LittleFloat(pIn->invBase.axis[2][i]);
		}

		for (int i(0); i < 3; ++i)
		{
			cOutBone.m_base.pos[i] = LittleFloat(pIn->base.pos[i]);
			cOutBone.m_base.axis[0][i] = LittleFloat(pIn->base.axis[0][i]);
			cOutBone.m_base.axis[1][i] = LittleFloat(pIn->base.axis[1][i]);
			cOutBone.m_base.axis[2][i] = LittleFloat(pIn->base.axis[2][i]);
		}
		//FixModelMatrix(&newBone.m_invBase);

		m_pOutModel->AddBone(cOutBone);

		// Advance the pointer
		pIn = (SBoneBlockK2v3 *)((byte *)pIn + (sizeof(SBoneBlockK2v3) - 256 + pIn->cNameLen + 1));
	}

	return true;
}


/*====================
  CInK2v3Model::ParseMeshBlock
  ====================*/
bool	CInK2v3Model::ParseMeshBlock(const SFileBlock &cBlock)
{
	SMeshBlockK2v3 &cIn(*(SMeshBlockK2v3 *)cBlock.pData);

	// Validate the block
	if (cBlock.uiLength != sizeof(SMeshBlock) - sizeof(char) * 512 + cIn.cNameLen + cIn.cShaderNameLen + 2)
	{
		cerr << _T("Bad mesh block") << endl;
		return false;
	}

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in single link block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));

	tstring sMeshName;
	StrToTString(sMeshName, cIn.szNameBuffer);
	pOutMesh->SetName(sMeshName);

	tstring sDefaultShaderName;
	StrToTString(sDefaultShaderName, &cIn.szNameBuffer[cIn.cNameLen + 1]);
	pOutMesh->SetDefaultShaderName(sDefaultShaderName);

	pOutMesh->mode = LittleInt(cIn.mode);
	pOutMesh->num_verts = LittleInt(cIn.num_verts);

	CVec3f v3Min(LittleFloat(cIn.bmin[X]), LittleFloat(cIn.bmin[Y]), LittleFloat(cIn.bmin[Z]));
	CVec3f v3Max(LittleFloat(cIn.bmax[X]), LittleFloat(cIn.bmax[Y]), LittleFloat(cIn.bmax[Z]));
	//FixModelCoord(v3Min);
	//FixModelCoord(v3Max);
	pOutMesh->bbBounds = CBBoxf(v3Min, v3Max);

	pOutMesh->bonelink = LittleInt(cIn.bonelink);
	pOutMesh->SetModel(m_pOutModel);

	return true;
}


/*====================
  CInK2v3Model::ParseVertexBlock
  ====================*/
bool	CInK2v3Model::ParseVertexBlock(const SFileBlock &cBlock)
{
	SVertexBlockK2v3 &cIn(*(SVertexBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in single link block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	int iNumVerts(pOutMesh->num_verts);
	
	pOutMesh->verts = new CVec3f[iNumVerts];

	CVec3f *pVerts((CVec3f *)(&cIn + 1));
	for (int n(0); n < iNumVerts; ++n)
	{
 		pOutMesh->verts[n][0] = LittleFloat(pVerts[n][0]);
		pOutMesh->verts[n][1] = LittleFloat(pVerts[n][1]);
		pOutMesh->verts[n][2] = LittleFloat(pVerts[n][2]);
	}

	return true;
}


typedef pair<int, float> BlendedLinkPair;

static bool LinkPredIndex(const BlendedLinkPair &elem1, const BlendedLinkPair &elem2)
{
	return elem1.first < elem2.first;
}

static bool LinkPredWeight(const BlendedLinkPair &elem1, const BlendedLinkPair &elem2)
{
	return elem1.second > elem2.second;
}


/*====================
  CInK2v3Model::ParseBlendedLinksBlock
  ====================*/
bool	CInK2v3Model::ParseBlendedLinksBlock(const SFileBlock &cBlock)
{
	SBlendedLinksBlockK2v3 &cIn(*(SBlendedLinksBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in blended link block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	if (pOutMesh->mode != MESH_SKINNED_BLENDED)
	{
		cerr << _T("Invalid blended link block") << endl;
		return false;
	}

	if (LittleInt(cIn.num_verts) != pOutMesh->num_verts)
	{
		cerr << _T("Invalid blended link block (num_verts didn't match)") << endl;
		return false;
	}

	// Alloc a pool we can use for keeping the weight data so memory doesn't get fragmented into little pieces
	int poolsize(cBlock.uiLength - (4 * pOutMesh->num_verts) - sizeof(blendedLinksBlock_t));
	byte *pool = pOutMesh->linkPool = new byte[poolsize];  // Allocate enough memory to store weights and indexes
	int poolpos = 0;

	pOutMesh->blendedLinks = new SBlendedLink[pOutMesh->num_verts];
	// also allocated the single links block, as we can use this as an LOD fallback
	pOutMesh->singleLinks = new singleLink_t[pOutMesh->num_verts];

	int pos(sizeof(SBlendedLinksBlockK2v3));

#if 1
	int iMaxBoneWeights(4);
	
	for (int n(0); n < pOutMesh->num_verts; ++n)
	{
		float fWeight(0.0f);
		
		int iNumWeights(LittleInt(*(int *)&cBlock.pData[pos]));

		pos += 4;

		float *pfWeights((float *)&cBlock.pData[pos]);
		int *piIndexes((int *)&cBlock.pData[pos + (iNumWeights << 2)]);

		int iClippedWeights(MIN(iMaxBoneWeights, iNumWeights));

		pOutMesh->blendedLinks[n].num_weights = 0;
		pOutMesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
		poolpos += iClippedWeights * sizeof(float);
		pOutMesh->blendedLinks[n].indexes = (uint *)(&pool[poolpos]);
		poolpos += iClippedWeights * sizeof(uint);

		// Combine duplicate weights
		for (int c(0); c < iNumWeights; ++c)
		{
			int iIndexC(piIndexes[c]);

			for (int d(c + 1); d < iNumWeights; ++d)
			{
				if (pfWeights[d] > 0.0f && piIndexes[d] == iIndexC)
				{
					pfWeights[c] += pfWeights[d];
					pfWeights[d] = 0.0f;
				}
			}
		}

		// Pick the biggest weights first
		for (int c(0); c < iClippedWeights; ++c)
		{
			float fMaxWeight(0.0f);
			int iMax(-1);

			for (int w(c); w < iNumWeights; ++w)
			{
				float fTestWeight(LittleFloat(pfWeights[w]));

				if (fTestWeight > fMaxWeight)
				{
					fMaxWeight = fTestWeight;
					iMax = w;
				}
			}

			if (iMax == -1)
				break;

			pOutMesh->blendedLinks[n].weights[c] = fMaxWeight;
			pOutMesh->blendedLinks[n].indexes[c] = LittleInt(piIndexes[iMax]);
			pOutMesh->blendedLinks[n].num_weights++;

			fWeight += fMaxWeight;

			//if (fWeight >= 1.0f)
			//	break;

			// Shift active values up the list
			if (iMax != c)
			{
				pfWeights[iMax] = pfWeights[c];
				piIndexes[iMax] = piIndexes[c];
			}
		}

		for (int w(0); w < pOutMesh->blendedLinks[n].num_weights; ++w)
			pOutMesh->blendedLinks[n].weights[w] /= fWeight;

		pOutMesh->singleLinks[n] = pOutMesh->blendedLinks[n].indexes[0];

		pos += iNumWeights * 4 * 2;

		//if (poolpos > poolsize)
		//	K2System.Error(_TS("CInK2v3Model::ParseBlendedLinksBlock(") + GetName() + _TS("): overflowed link pool"));
	}
#else
	for (int n(0); n < pOutMesh->num_verts; ++n)
	{
		int iNumWeights(LittleInt(*(int *)&cBlock.pData[pos]));

		pos += 4;

		float *pfWeights((float *)&cBlock.pData[pos]);
		int *piIndexes((int *)&cBlock.pData[pos + (iNumWeights << 2)]);

		pOutMesh->blendedLinks[n].num_weights = 0;
		pOutMesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
		poolpos += iNumWeights * sizeof(float);
		pOutMesh->blendedLinks[n].indexes = (uint *)(&pool[poolpos]);
		poolpos += iNumWeights * sizeof(uint);

		for (int c(0); c < iNumWeights; ++c)
		{
			pOutMesh->blendedLinks[n].weights[c] = pfWeights[c];
			pOutMesh->blendedLinks[n].indexes[c] = piIndexes[c];
			pOutMesh->blendedLinks[n].num_weights++;
		}

		pOutMesh->singleLinks[n] = pOutMesh->blendedLinks[n].indexes[0];

		pos += iNumWeights * 4 * 2;
	}
#endif

	return true;
}


/*====================
  CInK2v3Model::ParseSingleLinksBlock
  ====================*/
bool	CInK2v3Model::ParseSingleLinksBlock(const SFileBlock &cBlock)
{
	SSingleLinksBlockK2v3 &cIn(*(SSingleLinksBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in single links block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	if (pOutMesh->mode != MESH_SKINNED_NONBLENDED)
	{
		cerr << _T("Invalid single links block") << endl;
		return false;
	}

	if (LittleInt(cIn.num_verts) != pOutMesh->num_verts)
	{
		cerr << _T("Invalid single links block (num_verts didn't match)") << endl;
		return false;
	}

	singleLink_t *pLinks((singleLink_t *)(&cIn + 1));
	pOutMesh->singleLinks = new singleLink_t[pOutMesh->num_verts];
	for (int n = 0; n < pOutMesh->num_verts; ++n)
		pOutMesh->singleLinks[n] = LittleInt(pLinks[n]);

	return true;
}


/*====================
  CInK2v3Model::ParseTextureCoordBlock
  ====================*/
bool	CInK2v3Model::ParseTextureCoordBlock(const SFileBlock &cBlock)
{
	STextureCoordsBlockK2v3 &cIn(*(STextureCoordsBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in texture coord block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->tverts[cIn.channel] = new CVec2f[pOutMesh->num_verts];

	CVec2f *pTVerts((CVec2f *)(&cIn + 1));
	for (int n = 0; n < pOutMesh->num_verts; ++n)
	{
		pOutMesh->tverts[cIn.channel][n][0] = LittleFloat(pTVerts[n][0]);
		pOutMesh->tverts[cIn.channel][n][1] = LittleFloat(pTVerts[n][1]);
	}

	return true;
}


/*====================
  CInK2v3Model::ParseTangentBlock
  ====================*/
bool	CInK2v3Model::ParseTangentBlock(const SFileBlock &cBlock)
{
	STangentBlockK2v3 &cIn(*(STangentBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in tangent block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->tangents[cIn.channel] = new CVec3f[pOutMesh->num_verts];

	CVec3f *pTangets((CVec3f *)(&cIn + 1));
	for (int n(0); n < pOutMesh->num_verts; ++n)
	{
		pOutMesh->tangents[cIn.channel][n][0] = LittleFloat(pTangets[n][0]);
		pOutMesh->tangents[cIn.channel][n][1] = LittleFloat(pTangets[n][1]);
		pOutMesh->tangents[cIn.channel][n][2] = LittleFloat(pTangets[n][2]);
	}

	return true;
}


/*====================
  CInK2v3Model::ParseColorBlock
  ====================*/
bool	CInK2v3Model::ParseColorBlock(const SFileBlock &cBlock)
{
	SColorBlockK2v3 &cIn(*(SColorBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in color block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->colors[cIn.channel] = new CVec4b[pOutMesh->num_verts];

	CVec4b *pColors((CVec4b *)(&cIn + 1));

	// Don't do any endian conversion since these are byte arrays
	memcpy(pOutMesh->colors[cIn.channel], pColors, sizeof(CVec4b) * pOutMesh->num_verts);

	return true;
}


/*====================
  CInK2v3Model::ParseNormalBlock
  ====================*/
bool	CInK2v3Model::ParseNormalBlock(const SFileBlock &cBlock)
{
	SNormalBlockK2v3 &cIn(*(SNormalBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in normal block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->normals = new CVec3f[pOutMesh->num_verts];

	CVec3f *pNormals((CVec3f *)(&cIn + 1));
	for (int n(0); n < pOutMesh->num_verts; ++n)
	{
		pOutMesh->normals[n][0] = LittleFloat(pNormals[n][0]);
		pOutMesh->normals[n][1] = LittleFloat(pNormals[n][1]);
		pOutMesh->normals[n][2] = LittleFloat(pNormals[n][2]);
	}

	return true;
}


/*====================
  CInK2v3Model::ParseFaceBlock
  ====================*/
bool	CInK2v3Model::ParseFaceBlock(const SFileBlock &cBlock)
{
	SFaceBlockK2v3 &cIn(*(SFaceBlockK2v3 *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in face block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->numFaces = LittleInt(cIn.numFaces);
	pOutMesh->faceList = new CVec3ui[pOutMesh->numFaces];

	int iIndexSize(cIn.faceIndexSize);

	switch (iIndexSize)
	{
	case 1:
		{
			byte *pFaceList((byte *)(&cIn + 1));
			for (int n(0); n < pOutMesh->numFaces; ++n)
			{
				for (int v(0); v < 3; ++v)
				{
					pOutMesh->faceList[n][v] = *pFaceList;
					++pFaceList;
				}
			}
		}
		break;
	case 2:
		{
			unsigned short *pFaceList((unsigned short *)(&cIn + 1));
			for (int n(0); n < pOutMesh->numFaces; ++n)
			{
				for (int v(0); v < 3; ++v)
				{
					pOutMesh->faceList[n][v] = LittleShort(*pFaceList);
					++pFaceList;
				}
			}
		}
		break;
	case 4:
		{
			uint *pFaceList((uint *)(&cIn + 1));
			for (int n(0); n < pOutMesh->numFaces; ++n)
			{
				for (int v(0); v < 3; ++v)
				{
					pOutMesh->faceList[n][v] = LittleInt(*pFaceList);
					++pFaceList;
				}
			}
		}
		break;
	}

	return true;
}


/*====================
  CInK2v3Model::ParseSurfBlock
  ====================*/
bool	CInK2v3Model::ParseSurfBlock(const SFileBlock &cBlock)
{
	SSurfBlockK2v3 &cIn(*(SSurfBlockK2v3 *)cBlock.pData);

	if (cIn.num_points < 0 || cIn.num_edges < 0 || cIn.num_tris < 0 ||
		cIn.num_points * sizeof(CVec3f) > cBlock.uiLength ||
		cIn.num_edges * sizeof(CEdge) > cBlock.uiLength ||
		cIn.num_tris * sizeof(uint) > cBlock.uiLength)
		return true;

	CVec3f v3Min, v3Max;
	for (int n(0); n < 3; ++n)
	{
		v3Min[n] = LittleFloat(cIn.bmin[n]);
		v3Max[n] = LittleFloat(cIn.bmax[n]);
	}

	CConvexPolyhedron &cSurf(m_pOutModel->AllocCollisionSurf());
	cSurf.SetBounds(CBBoxf(v3Min, v3Max));

	CPlane *pPlanes((CPlane *)(&cIn + 1));
	cSurf.ReservePlanes(cIn.num_planes);
	for (int n(0); n < cIn.num_planes; ++n)
	{
		cSurf.AddPlane(CPlane(LittleFloat(pPlanes[n].v3Normal.x), LittleFloat(pPlanes[n].v3Normal.y),
			LittleFloat(pPlanes[n].v3Normal.z), LittleFloat(pPlanes[n].fDist)));
	}

	CVec3f *pPoints((CVec3f *)(pPlanes + cIn.num_planes));
	cSurf.ReservePoints(cIn.num_points);
	for (int n(0); n < cIn.num_points; ++n)
	{
		cSurf.AddPoint(CVec3f(LittleFloat(pPoints[n].x), LittleFloat(pPoints[n].y), LittleFloat(pPoints[n].z)));
	}

	CEdge *pEdges((CEdge *)(pPoints + cIn.num_points));
	cSurf.ReserveEdges(cIn.num_edges);
	for (int n(0); n < cIn.num_edges; ++n)
	{
		CEdge cNewEdge;

		cNewEdge.v3Normal.x = LittleFloat(pEdges[n].v3Normal.x);
		cNewEdge.v3Normal.y = LittleFloat(pEdges[n].v3Normal.y);
		cNewEdge.v3Normal.z = LittleFloat(pEdges[n].v3Normal.z);

		cNewEdge.v3Point.x = LittleFloat(pEdges[n].v3Point.x);
		cNewEdge.v3Point.y = LittleFloat(pEdges[n].v3Point.y);
		cNewEdge.v3Point.z = LittleFloat(pEdges[n].v3Point.z);

		cSurf.AddEdge(cNewEdge);
	}

	uint *pTris((uint *)(pEdges + cIn.num_edges));
	cSurf.ReserveTris(cIn.num_tris);
	for (int n(0); n < cIn.num_tris; ++n)
	{
		cSurf.AddTri
		(
			LittleInt(pTris[n * 3 + 0]),
			LittleInt(pTris[n * 3 + 1]),
			LittleInt(pTris[n * 3 + 2])
		);
	}

	cSurf.CalcExtents();

	return true;
}


/*====================
  CInK2v3Model::ReadBlocks
  ====================*/
bool	CInK2v3Model::ReadBlocks(FileBlockList &vBlockList)
{
	if (vBlockList.empty() || strcmp(vBlockList.front().szName, "head") != 0)
	{
		cerr << _T("CInK2v3Model::ReadBlocks: No header") << endl;
		return false;
	}

	if (!ParseHeader(vBlockList[0]))
		return false;

	for (uint ui(1); ui < vBlockList.size(); ++ui)
	{
		SFileBlock &cBlock(vBlockList[ui]);

		if (strcmp(cBlock.szName, "bone") == 0)
		{
			if (!ParseBoneBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "mesh") == 0)
		{
			if (!ParseMeshBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "vrts") == 0)
		{
			if (!ParseVertexBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "lnk1") == 0)
		{
			if (!ParseBlendedLinksBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "lnk2") == 0)
		{
			if (!ParseSingleLinksBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "texc") == 0)
		{
			if (!ParseTextureCoordBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "tang") == 0)
		{
			if (!ParseTangentBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "colr") == 0)
		{
			if (!ParseColorBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "nrml") == 0)
		{
			if (!ParseNormalBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "face") == 0)
		{
			if (!ParseFaceBlock(cBlock))
				return false;
		}
		else if (strcmp(cBlock.szName, "surf") == 0)
		{
			if (!ParseSurfBlock(cBlock))
				return false;
		}
	}

	return true;
}


/*====================
  CInK2v3Model::ReadFile
  ====================*/
void	CInK2v3Model::ReadFile(const tstring &sFilename)
{
	CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

	vector<SFileBlock> vBlockList;
	if (!hFile.BuildBlockList(s_ayHeader, sizeof(s_ayHeader), vBlockList))
		return;

	if (!ReadBlocks(vBlockList))
		return;

	m_pOutModel->RebuildData();
}