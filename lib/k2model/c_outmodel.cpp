// c_outmodel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_outmodel.h"

#include <s_blendedlink.h>
//=============================================================================

/*====================
  COutModel::COutModel
  ====================*/
COutModel::COutModel() :
m_bIsValid(false)
{
	Validate();
}


/*====================
  COutModel::GetNumBones
  ====================*/
uint	COutModel::GetNumBones()
{
	return uint(m_vBones.size());
}


/*====================
  COutModel::GetBone
  ====================*/
COutBone*	COutModel::GetBone(uint uiBone)
{
	if (uiBone >= m_vBones.size())
		return NULL;
	else
		return &m_vBones[uiBone];
}


/*====================
  COutModel::GetBone
  ====================*/
COutBone*	COutModel::GetBone(const tstring &sBone)
{
	map<tstring, uint>::iterator findit(m_mapBoneIndices.find(sBone));

	if (findit != m_mapBoneIndices.end())
		return &m_vBones[findit->second];
	else
		return NULL;
}


/*====================
  COutModel::GetBoneIndex
  ====================*/
uint	COutModel::GetBoneIndex(const tstring &sBone)
{
	map<tstring, uint>::iterator findit(m_mapBoneIndices.find(sBone));

	if (findit != m_mapBoneIndices.end())
		return findit->second;
	else
		return INVALID_BONE;
}


/*====================
  COutModel::AddBone
  ====================*/
uint	COutModel::AddBone(const COutBone &newBone)
{
	if (m_vBones.size() < newBone.GetIndex() + 1)
		m_vBones.resize(newBone.GetIndex() + 1, COutBone(INVALID_BONE, _T(""), INVALID_BONE));

	m_vBones[newBone.GetIndex()] = newBone;
	m_mapBoneIndices[newBone.GetName()] = newBone.GetIndex();

	return uint(m_vBones.size());
}


/*====================
  COutModel::AllocMesh
  ====================*/
COutMesh&	COutModel::AllocMesh()
{
	m_vMeshes.push_back(COutMesh());

	return m_vMeshes.back();
}


/*====================
  COutModel::AllocMeshes
  ====================*/
void	COutModel::AllocMeshes(uint uiNumMeshes)
{
	m_vMeshes.resize(uiNumMeshes);
}


/*====================
  COutModel::AddMesh
  ====================*/
void	COutModel::AddMesh(const COutMesh &newMesh)
{
	m_vMeshes.push_back(newMesh);
}


/*====================
  COutModel::GetNumMeshes
  ====================*/
uint	COutModel::GetNumMeshes() const
{
	return uint(m_vMeshes.size());
}


/*====================
  COutModel::GetMesh
  ====================*/
COutMesh*	COutModel::GetMesh(uint uiIndex)
{
	return &m_vMeshes[uiIndex];
}


/*====================
  COutModel::GetMesh
  ====================*/
COutMesh*	COutModel::GetMesh(const tstring &sName)
{
	for (vector<COutMesh>::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
	{
		if (it->GetName() == sName)
			return &(*it);
	}

	return NULL;
}


/*====================
  COutModel::GetMeshIndex
  ====================*/
uint	COutModel::GetMeshIndex(const tstring &sName)
{
	for (vector<COutMesh>::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
	{
		if (it->GetName() == sName)
			return uint(it - m_vMeshes.begin());
	}

	return uint(-1);
}


/*====================
  COutModel::AddSurf
  ====================*/
void	COutModel::AddSurf(const CConvexPolyhedron &newSurf)
{
	m_vSurfs.push_back(newSurf);
}

/*====================
  COutModel::GetNumSurfs
  ====================*/
uint	COutModel::GetNumSurfs() const
{
	return uint(m_vSurfs.size());
}


/*====================
  COutModel::GetSurf
  ====================*/
CConvexPolyhedron*	COutModel::GetSurf(uint uiIndex)
{
	return &m_vSurfs[uiIndex];
}


/*====================
  COutModel::BuildBoneHierarchy
  ====================*/
void	COutModel::BuildBoneHierarchy()
{
	for (vector<COutBone>::iterator it(m_vBones.begin()); it != m_vBones.end(); ++it)
	{
		uint uiParentIndex(it->GetParentIndex());
		if (uiParentIndex == INVALID_BONE)
			continue;

		m_vBones[uiParentIndex].AddChild(it->GetIndex());
	}
}


/*====================
  COutModel::BeginBlock
  ====================*/
void	COutModel::BeginBlock(const char *blockName, FILE *stream)
{
	int templength = 0;

	fwrite(blockName, 4, 1, stream);
	//we'll fill in the length here on the EndBlock call
	seekback = ftell(stream);
	fwrite(&templength, 4, 1, stream);
}


/*====================
  COutModel::WriteBlockData
  ====================*/
void	COutModel::WriteBlockData(void *data, int size, FILE *stream)
{
	fwrite(data, size, 1, stream);
}


/*====================
  COutModel::EndBlock
  ====================*/
void	COutModel::EndBlock(FILE *stream)
{
	fpos_t curpos = ftell(stream);
	int length = curpos - seekback - 4;
	fseek(stream, seekback, SEEK_SET);
	fwrite(&length, 4, 1, stream);
	fseek(stream, curpos, SEEK_SET);
}


/*====================
  COutModel::RebuildData
  ====================*/
void	COutModel::RebuildData()
{
	for (vector<COutMesh>::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
	{
		COutMesh *pMesh(&*it);

		pMesh->ExpandVertexes();
		pMesh->CalcMeshTangents();
		pMesh->Optimize();
	}
}


/*====================
  COutModel::WriteFile
  ====================*/
tstring	COutModel::WriteFile(const tstring &sFilename)
{
	FILE *stream;
	if (fopen_s(&stream, sFilename.c_str(), "wb") != 0 || stream == NULL)
	{
		cerr << "Couldn't create .model file" << endl;
		return _T("");
	}

	// Identifier
	fwrite("SMDL", 4, 1, stream);

	// Header
	BeginBlock("head", stream);
	modelHeader_t header;
	header.version = 3;
	header.numMeshes = int(m_vMeshes.size());
	header.num_sprites = 0;
	header.num_surfs = int(m_vSurfs.size());
	header.numBones = int(m_vBones.size());
	header.bmin = m_bbBounds.GetMin();
	header.bmax = m_bbBounds.GetMax();

	WriteBlockData(&header, sizeof(header), stream);
	EndBlock(stream);

	// Bones
	BeginBlock("bone", stream);
	for (vector<COutBone>::iterator boneit(m_vBones.begin()); boneit != m_vBones.end(); ++boneit)
	{
		boneBlock_t boneblock;

		boneblock.parentIndex = boneit->GetParentIndex();
		memcpy(&boneblock.invBase, &boneit->m_invBase, sizeof(matrix43_t));
		memcpy(&boneblock.base, &boneit->m_base, sizeof(matrix43_t));

		boneblock.cNameLen = byte(MIN(boneit->GetName().size(), size_t(255)));
		strncpy_s(boneblock.szBoneName, boneit->GetName().c_str(), boneblock.cNameLen);
		boneblock.szBoneName[boneblock.cNameLen] = 0;

		WriteBlockData(&boneblock, sizeof(boneblock) - 256 + boneblock.cNameLen + 1, stream);
	}
	EndBlock(stream);

	// Write data for each mesh
	for (vector<COutMesh>::iterator meshit(m_vMeshes.begin()); meshit != m_vMeshes.end(); ++meshit)
	{
		int iMeshIndex(meshit - m_vMeshes.begin());

		// Mesh header
		BeginBlock("mesh", stream);

		SMeshBlock meshblock;
		meshblock.mesh_index	= iMeshIndex;
		meshblock.mode			= meshit->mode;
		meshblock.num_verts		= meshit->num_verts;
		meshblock.bmin			= meshit->bbBounds.GetMin();
		meshblock.bmax			= meshit->bbBounds.GetMax();
		meshblock.bonelink		= meshit->bonelink;

		meshblock.cNameLen = byte(MIN(meshit->GetName().size(), size_t(255)));
		strncpy_s(meshblock.szNameBuffer, meshit->GetName().c_str(), meshblock.cNameLen);
		meshblock.szNameBuffer[meshblock.cNameLen] = 0;

		meshblock.cShaderNameLen = byte(MIN(meshit->GetDefaultShaderName().size(), size_t(255)));
		strncpy_s(&meshblock.szNameBuffer[meshblock.cNameLen + 1], sizeof(meshblock.szNameBuffer) - (meshblock.cNameLen + 1), meshit->GetDefaultShaderName().c_str(), meshblock.cShaderNameLen);
		meshblock.szNameBuffer[meshblock.cNameLen + 1 + meshblock.cShaderNameLen] = 0;

		WriteBlockData(&meshblock, sizeof(meshblock) - 512 + meshblock.cNameLen + meshblock.cShaderNameLen + 2, stream);

		EndBlock(stream);

		// Vertices
		if (meshit->verts != NULL)
		{
			BeginBlock("vrts", stream);

			vertexBlock_t vertblock;
			vertblock.mesh_index = iMeshIndex;
			WriteBlockData(&vertblock, sizeof(vertblock), stream);
			for (int v = 0; v < meshit->num_verts; ++v)
				WriteBlockData(meshit->verts[v], sizeof(CVec3f), stream);

			EndBlock(stream);
		}

		// Bone weights
		if (meshit->mode == MESH_SKINNED_BLENDED)
		{
			if (meshit->blendedLinks != NULL)
			{
#if 0
				BeginBlock("lnk1", stream);
#else
				BeginBlock("lnk3", stream);
#endif

				blendedLinksBlock_t blendblock;

				blendblock.mesh_index = iMeshIndex;
				blendblock.num_verts = meshit->num_verts;

				WriteBlockData(&blendblock, sizeof(blendblock), stream);

				for (int v = 0; v < meshit->num_verts; ++v)
				{
					fwrite(&meshit->blendedLinks[v].num_weights, 4, 1, stream);
					fwrite(meshit->blendedLinks[v].weights, 4 * meshit->blendedLinks[v].num_weights, 1, stream);
					fwrite(meshit->blendedLinks[v].indexes, 4 * meshit->blendedLinks[v].num_weights, 1, stream);
				}

				EndBlock(stream);
			}
		}
		else
		{
			if (meshit->bonelink != -1 && meshit->singleLinks != NULL)
			{
				BeginBlock("lnk2", stream);

				singleLinksBlock_t singleblock;
				singleblock.mesh_index = iMeshIndex;
				singleblock.num_verts = meshit->num_verts;
				WriteBlockData(&singleblock, sizeof(singleblock), stream);

				for (int v = 0; v < meshit->num_verts; ++v)
				{
					int bone = meshit->singleLinks[v];
					fwrite(&meshit->singleLinks[v], 4, 1, stream);
					fwrite(&bone, 4, 1, stream);
				}

				EndBlock(stream);
			}
		}

		// Face List
		if (meshit->faceList != NULL)
		{
			BeginBlock("face", stream);

			faceBlock_t faceblock;
			faceblock.mesh_index = iMeshIndex;
			faceblock.numFaces = meshit->numFaces;
			faceblock.faceIndexSize = (meshit->num_verts < 256) ? 1 : ((meshit->num_verts < 65536) ? 2 : 4);
			WriteBlockData(&faceblock, sizeof(faceblock), stream);

			for (int f = 0; f < meshit->numFaces; ++f)
			{
				fwrite(&meshit->faceList[f][0], faceblock.faceIndexSize, 1, stream);
				fwrite(&meshit->faceList[f][1], faceblock.faceIndexSize, 1, stream);
				fwrite(&meshit->faceList[f][2], faceblock.faceIndexSize, 1, stream);
			}

			EndBlock(stream);
		}

		// Texture coordinates
		for (int iChannel = 0; iChannel < MAX_UV_CHANNELS; ++iChannel)
		{
			// Check for data
			if (meshit->tverts[iChannel] == NULL)
				continue;

			BeginBlock("texc", stream);

			textureCoordsBlock_t texblock;
			texblock.mesh_index = iMeshIndex;
			texblock.channel = iChannel;

			WriteBlockData(&texblock, sizeof(texblock), stream);

			for (int f = 0; f < meshit->num_verts; ++f)
				fwrite(meshit->tverts[iChannel][f], sizeof(CVec2f), 1, stream);

			EndBlock(stream);
		}

		// Tangents
		for (int iChannel = 0; iChannel < MAX_UV_CHANNELS; ++iChannel)
		{
			// Check for data
			if (meshit->tangents[iChannel] == NULL)
				continue;

			BeginBlock("tang", stream);

			textureCoordsBlock_t tanblock;
			tanblock.mesh_index = iMeshIndex;
			tanblock.channel = iChannel;

			WriteBlockData(&tanblock, sizeof(tanblock), stream);

			for (int f = 0; f < meshit->num_verts; ++f)
				fwrite(meshit->tangents[iChannel][f], sizeof(CVec3f), 1, stream);

			EndBlock(stream);
		}

		// Signs
		for (int iChannel = 0; iChannel < MAX_UV_CHANNELS; ++iChannel)
		{
			// Check for data
			if (meshit->signs[iChannel] == NULL)
				continue;

			BeginBlock("sign", stream);

			textureCoordsBlock_t tanblock;
			tanblock.mesh_index = iMeshIndex;
			tanblock.channel = iChannel;

			WriteBlockData(&tanblock, sizeof(tanblock), stream);

			for (int f = 0; f < meshit->num_verts; ++f)
				fwrite(&meshit->signs[iChannel][f], sizeof(byte), 1, stream);

			EndBlock(stream);
		}

		// Vertex colors
		for (int iChannel = 0; iChannel < MAX_VERTEX_COLOR_CHANNELS; ++iChannel)
		{
			// Check for data
			if (meshit->colors[iChannel] == NULL)
				continue;

			BeginBlock("colr", stream);

			colorBlock_t colblock;

			colblock.mesh_index = iMeshIndex;
			colblock.channel = iChannel;
			WriteBlockData(&colblock, sizeof(colblock), stream);

			for (int f = 0; f < meshit->num_verts; ++f)
				fwrite(&meshit->colors[iChannel][f], sizeof(CVec4b), 1, stream);

			EndBlock(stream);
		}

		// Normals
		if (meshit->normals != NULL)
		{
			BeginBlock("nrml", stream);

			normalBlock_t nrmlblock;
			nrmlblock.mesh_index = iMeshIndex;
			WriteBlockData(&nrmlblock, sizeof(nrmlblock), stream);

			for (int f = 0; f < meshit->num_verts; ++f)
				fwrite(meshit->normals[f], sizeof(CVec3f), 1, stream);

			EndBlock(stream);
		}
	}

	// Collision surfaces
	for (vector<CConvexPolyhedron>::iterator surfit(m_vSurfs.begin()); surfit != m_vSurfs.end(); ++surfit)
	{
		int iSurfIndex = int(surfit - m_vSurfs.begin());
		const vector<uint> &vTriList(surfit->GetTriList());

		BeginBlock("surf", stream);

		surfBlock_t surfblock;

		surfblock.surf_index = iSurfIndex;
		surfblock.bmin = surfit->GetBounds().GetMin();
		surfblock.bmax = surfit->GetBounds().GetMax();
		surfblock.flags = 0;
		surfblock.num_planes = surfit->GetNumPlanes();
		surfblock.num_points = surfit->GetNumPoints();
		surfblock.num_edges = surfit->GetNumEdges();
		surfblock.num_tris = int(vTriList.size() / 3);

		WriteBlockData(&surfblock, sizeof(surfblock), stream);

		// Write planes
		for (uint p(0); p < surfit->GetNumPlanes(); ++p)
			fwrite(&surfit->GetPlane(p), sizeof(CPlane), 1, stream);

		// Write points
		for (uint p(0); p < surfit->GetNumPoints(); ++p)
			fwrite(&surfit->GetPoint(p), sizeof(CVec3f), 1, stream);

		// Write edges
		for (uint p(0); p < surfit->GetNumEdges(); ++p)
			fwrite(&surfit->GetEdge(p), sizeof(CEdge), 1, stream);

		// Write tris
		for (vector<uint>::const_iterator it(vTriList.begin()); it != vTriList.end(); ++it)
			fwrite(&*it, sizeof(uint), 1, stream);

		EndBlock(stream);
	}

	fclose(stream);

	return _T("Model written successfully to ") + sFilename;
}
