// c_insilverbackmodel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2model_common.h"

#include "c_insilverbackmodel.h"

#include "c_outmodel.h"

#include <s_blendedlink.h>
#include <c_filehandle.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
const char	CInSilverbackModel::s_ayHeader[4] = {'S', 'M', 'D', 'L'};
//=============================================================================


/*====================
  CInSilverbackModel::CInSilverbackModel
  ====================*/
CInSilverbackModel::CInSilverbackModel() :
m_iNumBones(0)
{
}


/*====================
  CInSilverbackModel::ParseHeader
  ====================*/
bool	CInSilverbackModel::ParseHeader(const SFileBlock &cBlock)
{
	SModelHeaderSilverback &cIn(*(SModelHeaderSilverback *)cBlock.pData);

	if (cBlock.uiLength < sizeof(SModelHeaderSilverback))
	{
		cerr << _T("Bad model header") << endl;
		return false;
	}

	int iVersion(LittleInt(cIn.version));
	if (iVersion != 1 && iVersion != 2)
	{
		if (iVersion != 2)
			cerr << _T("Invalid model version: ") << iVersion;

		cerr << _T(", expected version is: ") << 2 << endl;
		return false;
	}

	CVec3f v3Min(LittleFloat(cIn.bmin[X]), LittleFloat(cIn.bmin[Y]), LittleFloat(cIn.bmin[Z]));
	CVec3f v3Max(LittleFloat(cIn.bmax[X]), LittleFloat(cIn.bmax[Y]), LittleFloat(cIn.bmax[Z]));
	//FixModelCoord(v3Min);
	//FixModelCoord(v3Max);
	m_pOutModel->SetBounds(CBBoxf(v3Min, v3Max));

	m_pOutModel->AllocMeshes(LittleInt(cIn.numMeshes));

	m_iNumBones = LittleInt(cIn.numBones);

	return true;
}


/*====================
  CInSilverbackModel::ParseBoneBlock
  ====================*/
bool	CInSilverbackModel::ParseBoneBlock(const SFileBlock &cBlock)
{
	SBoneBlockSilverback *pIn((SBoneBlockSilverback *)cBlock.pData);

	// Read each bone
	for (int n(0); n < m_iNumBones; ++n)
	{
		int parentIndex = LittleInt(pIn->parentIndex);

		tstring sBoneName;
		StrToTString(sBoneName, pIn->name);

		COutBone cBone(n, sBoneName, parentIndex);

		for (int i(0); i < 3; ++i)
		{
			cBone.m_invBase.axis[0][i] = LittleFloat(pIn->invBase.t.axis[0][i]);
			cBone.m_invBase.axis[1][i] = LittleFloat(pIn->invBase.t.axis[1][i]);
			cBone.m_invBase.axis[2][i] = LittleFloat(pIn->invBase.t.axis[2][i]);
			cBone.m_invBase.pos[i] = LittleFloat(pIn->invBase.t.pos[i]);
		}

		//Geom_FixModelMatrix(&cBone.invBase);
		
		for (int i(0); i < 3; ++i)
		{
			cBone.m_base.axis[0][i] = LittleFloat(pIn->base.t.axis[0][i]);
			cBone.m_base.axis[1][i] = LittleFloat(pIn->base.t.axis[1][i]);
			cBone.m_base.axis[2][i] = LittleFloat(pIn->base.t.axis[2][i]);
			cBone.m_base.pos[i] = LittleFloat(pIn->base.t.pos[i]);
		}		

		//Geom_FixModelMatrix(&cBone.localBase);

		m_pOutModel->AddBone(cBone);
		
		++pIn;
	}

	return true;
}


/*====================
  CInSilverbackModel::ParseMeshBlock
  ====================*/
bool	CInSilverbackModel::ParseMeshBlock(const SFileBlock &cBlock)
{
	SMeshBlockSilverback &cIn(*(SMeshBlockSilverback *)cBlock.pData);

	// Validate the block
	if (cBlock.uiLength < sizeof(SMeshBlockSilverback))
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
	StrToTString(sMeshName, cIn.name);
	pOutMesh->SetName(sMeshName);

	tstring sDefaultShaderName;
	StrToTString(sDefaultShaderName, cIn.defaultShader);
	pOutMesh->SetDefaultShaderName(Filename_GetName(sDefaultShaderName));

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
  CInSilverbackModel::ParseVertexBlock
  ====================*/
bool	CInSilverbackModel::ParseVertexBlock(const SFileBlock &cBlock)
{
	SVertexBlockSilverback &cIn(*(SVertexBlockSilverback *)cBlock.pData);

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


/*====================
  CInSilverbackModel::ParseBlendedLinksBlock
  ====================*/
bool	CInSilverbackModel::ParseBlendedLinksBlock(const SFileBlock &cBlock)
{
	SBlendedLinksBlockSilverback &cIn(*(SBlendedLinksBlockSilverback *)cBlock.pData);

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
	int poolsize(cBlock.uiLength - (4 * pOutMesh->num_verts) - sizeof(SBlendedLinksBlockSilverback));
	byte *pool = pOutMesh->linkPool = new byte[poolsize];  // Allocate enough memory to store weights and indexes
	int poolpos = 0;

	pOutMesh->blendedLinks = new SBlendedLink[pOutMesh->num_verts];
	// also allocated the single links block, as we can use this as an LOD fallback
	pOutMesh->singleLinks = new singleLink_t[pOutMesh->num_verts];

	int pos(sizeof(SBlendedLinksBlockSilverback));
	int iMaxBoneWeights(8);
	
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

		if (poolpos > poolsize)
			cerr << _T("overflowed link pool") << endl;
	}

	return true;
}


/*====================
  CInSilverbackModel::ParseSingleLinksBlock
  ====================*/
bool	CInSilverbackModel::ParseSingleLinksBlock(const SFileBlock &cBlock)
{
	cerr << _T("Single link block") << endl;

#if 0
	SSingleLinksBlockSilverback &cIn(*(SSingleLinksBlockSilverback *)cBlock.pData);

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
#endif

	return true;
}


/*====================
  CInSilverbackModel::ParseTextureCoordBlock
  ====================*/
bool	CInSilverbackModel::ParseTextureCoordBlock(const SFileBlock &cBlock)
{
	STextureCoordsBlockSilverback &cIn(*(STextureCoordsBlockSilverback *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in texture coord block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->tverts[0] = new CVec2f[pOutMesh->num_verts];

	CVec2f *pTVerts((CVec2f *)(&cIn + 1));
	for (int n = 0; n < pOutMesh->num_verts; ++n)
	{
		pOutMesh->tverts[0][n][0] = LittleFloat(pTVerts[n][0]);
		pOutMesh->tverts[0][n][1] = 1.0f - LittleFloat(pTVerts[n][1]);
	}

	return true;
}


/*====================
  CInSilverbackModel::ParseColorBlock
  ====================*/
bool	CInSilverbackModel::ParseColorBlock(const SFileBlock &cBlock)
{
	SColorBlockSilverback &cIn(*(SColorBlockSilverback *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in color block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->colors[0] = new CVec4b[pOutMesh->num_verts];

	CVec4b *pColors((CVec4b *)(&cIn + 1));

	// Don't do any endian conversion since these are byte arrays
	memcpy(pOutMesh->colors[0], pColors, sizeof(CVec4b) * pOutMesh->num_verts);

	return true;
}


/*====================
  CInSilverbackModel::ParseNormalBlock
  ====================*/
bool	CInSilverbackModel::ParseNormalBlock(const SFileBlock &cBlock)
{
	SNormalBlockSilverback &cIn(*(SNormalBlockSilverback *)cBlock.pData);

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
  CInSilverbackModel::ParseFaceBlock
  ====================*/
bool	CInSilverbackModel::ParseFaceBlock(const SFileBlock &cBlock)
{
	SFaceBlockSilverback &cIn(*(SFaceBlockSilverback *)cBlock.pData);

	int iMeshIndex(LittleInt(cIn.mesh_index));
	if (iMeshIndex >= int(m_pOutModel->GetNumMeshes()) || iMeshIndex < 0)
	{
		cerr << _T("Bad mesh index in face block") << endl;
		return false;
	}

	COutMesh *pOutMesh(m_pOutModel->GetMesh(iMeshIndex));
	pOutMesh->numFaces = LittleInt(cIn.num_faces);
	pOutMesh->faceList = new CVec3ui[pOutMesh->numFaces];

	uint *pFaceList((uint *)(&cIn + 1));
	for (int n(0); n < pOutMesh->numFaces; ++n)
	{
		for (int v(0); v < 3; ++v)
		{
			pOutMesh->faceList[n][v] = LittleInt(*pFaceList);
			++pFaceList;
		}
	}

	return true;
}


/*====================
  CInSilverbackModel::ParseSurfBlock
  ====================*/
bool	CInSilverbackModel::ParseSurfBlock(const SFileBlock &cBlock)
{
#if 0
	SSurfBlockSilverback &cIn(*(SSurfBlockSilverback *)cBlock.pData);

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
#endif

	return true;
}


/*====================
  CInSilverbackModel::ReadBlocks
  ====================*/
bool	CInSilverbackModel::ReadBlocks(FileBlockList &vBlockList)
{
	if (vBlockList.empty() || strcmp(vBlockList.front().szName, "head") != 0)
	{
		cerr << _T("CInSilverbackModel::ReadBlocks: No header") << endl;
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


#ifndef MATRIX_DEFINED
typedef struct _MATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} MATRIX;
#define MATRIX_DEFINED
#endif

typedef struct XMATRIX : public MATRIX
{
public:
    XMATRIX() {};
    XMATRIX( const float * );
    XMATRIX( const MATRIX& );
    XMATRIX( float _11, float _12, float _13, float _14,
                float _21, float _22, float _23, float _24,
                float _31, float _32, float _33, float _34,
                float _41, float _42, float _43, float _44 );


    // access grants
    float& operator () ( uint Row, uint Col );
    float  operator () ( uint Row, uint Col ) const;

    // casting operators
    operator float* ();
    operator const float* () const;

    // assignment operators
    XMATRIX& operator *= ( const XMATRIX& );
    XMATRIX& operator += ( const XMATRIX& );
    XMATRIX& operator -= ( const XMATRIX& );
    XMATRIX& operator *= ( float );
    XMATRIX& operator /= ( float );

    // unary operators
    XMATRIX operator + () const;
    XMATRIX operator - () const;

    // binary operators
    XMATRIX operator * ( const XMATRIX& ) const;
    XMATRIX operator + ( const XMATRIX& ) const;
    XMATRIX operator - ( const XMATRIX& ) const;
    XMATRIX operator * ( float ) const;
    XMATRIX operator / ( float ) const;

    friend XMATRIX operator * ( float, const XMATRIX& );

    bool operator == ( const XMATRIX& ) const;
    bool operator != ( const XMATRIX& ) const;

} XMATRIX, *LPXMATRIX;


/*====================
  XMatrixInverse
  ====================*/
MATRIX* XMatrixInverse(XMATRIX *pOut, float *pDeterminant, const XMATRIX *pM)
{
	XMATRIX M = *pM;

	pOut->_11 = M._23*M._34*M._42 - M._24*M._33*M._42 + M._24*M._32*M._43 - M._22*M._34*M._43 - M._23*M._32*M._44 + M._22*M._33*M._44;
	pOut->_12 = M._14*M._33*M._42 - M._13*M._34*M._42 - M._14*M._32*M._43 + M._12*M._34*M._43 + M._13*M._32*M._44 - M._12*M._33*M._44;
	pOut->_13 = M._13*M._24*M._42 - M._14*M._23*M._42 + M._14*M._22*M._43 - M._12*M._24*M._43 - M._13*M._22*M._44 + M._12*M._23*M._44;
	pOut->_14 = M._14*M._23*M._32 - M._13*M._24*M._32 - M._14*M._22*M._33 + M._12*M._24*M._33 + M._13*M._22*M._34 - M._12*M._23*M._34;
	pOut->_21 = M._24*M._33*M._41 - M._23*M._34*M._41 - M._24*M._31*M._43 + M._21*M._34*M._43 + M._23*M._31*M._44 - M._21*M._33*M._44;
	pOut->_22 = M._13*M._34*M._41 - M._14*M._33*M._41 + M._14*M._31*M._43 - M._11*M._34*M._43 - M._13*M._31*M._44 + M._11*M._33*M._44;
	pOut->_23 = M._14*M._23*M._41 - M._13*M._24*M._41 - M._14*M._21*M._43 + M._11*M._24*M._43 + M._13*M._21*M._44 - M._11*M._23*M._44;
	pOut->_24 = M._13*M._24*M._31 - M._14*M._23*M._31 + M._14*M._21*M._33 - M._11*M._24*M._33 - M._13*M._21*M._34 + M._11*M._23*M._34;
	pOut->_31 = M._22*M._34*M._41 - M._24*M._32*M._41 + M._24*M._31*M._42 - M._21*M._34*M._42 - M._22*M._31*M._44 + M._21*M._32*M._44;
	pOut->_32 = M._14*M._32*M._41 - M._12*M._34*M._41 - M._14*M._31*M._42 + M._11*M._34*M._42 + M._12*M._31*M._44 - M._11*M._32*M._44;
	pOut->_33 = M._12*M._24*M._41 - M._14*M._22*M._41 + M._14*M._21*M._42 - M._11*M._24*M._42 - M._12*M._21*M._44 + M._11*M._22*M._44;
	pOut->_34 = M._14*M._22*M._31 - M._12*M._24*M._31 - M._14*M._21*M._32 + M._11*M._24*M._32 + M._12*M._21*M._34 - M._11*M._22*M._34;
	pOut->_41 = M._23*M._32*M._41 - M._22*M._33*M._41 - M._23*M._31*M._42 + M._21*M._33*M._42 + M._22*M._31*M._43 - M._21*M._32*M._43;
	pOut->_42 = M._12*M._33*M._41 - M._13*M._32*M._41 + M._13*M._31*M._42 - M._11*M._33*M._42 - M._12*M._31*M._43 + M._11*M._32*M._43;
	pOut->_43 = M._13*M._22*M._41 - M._12*M._23*M._41 - M._13*M._21*M._42 + M._11*M._23*M._42 + M._12*M._21*M._43 - M._11*M._22*M._43;
	pOut->_44 = M._12*M._23*M._31 - M._13*M._22*M._31 + M._13*M._21*M._32 - M._11*M._23*M._32 - M._12*M._21*M._33 + M._11*M._22*M._33;

	float determinant =
	M._14 * M._23 * M._32 * M._41-M._13 * M._24 * M._32 * M._41-M._14 * M._22 * M._33 * M._41+M._12 * M._24    * M._33 * M._41+
	M._13 * M._22 * M._34 * M._41-M._12 * M._23 * M._34 * M._41-M._14 * M._23 * M._31 * M._42+M._13 * M._24    * M._31 * M._42+
	M._14 * M._21 * M._33 * M._42-M._11 * M._24 * M._33 * M._42-M._13 * M._21 * M._34 * M._42+M._11 * M._23    * M._34 * M._42+
	M._14 * M._22 * M._31 * M._43-M._12 * M._24 * M._31 * M._43-M._14 * M._21 * M._32 * M._43+M._11 * M._24    * M._32 * M._43+
	M._12 * M._21 * M._34 * M._43-M._11 * M._22 * M._34 * M._43-M._13 * M._22 * M._31 * M._44+M._12 * M._23    * M._31 * M._44+
	M._13 * M._21 * M._32 * M._44-M._11 * M._23 * M._32 * M._44-M._12 * M._21 * M._33 * M._44+M._11 * M._22    * M._33 * M._44;
	
	for(int x = 0; x < 4; x++)
		for(int y = 0; y < 4; y++)
			pOut->m[x][y] /= determinant;

	if(pDeterminant)
		*pDeterminant = determinant;
    return pOut;
}


/*====================
  CInSilverbackModel::ReadFile
  ====================*/
void	CInSilverbackModel::ReadFile(const tstring &sFilename)
{
	CFileHandle hFile(sFilename, FILE_READ | FILE_BINARY);

	vector<SFileBlock> vBlockList;
	if (!hFile.BuildBlockList(s_ayHeader, sizeof(s_ayHeader), vBlockList))
		return;

	if (!ReadBlocks(vBlockList))
		return;

	// Mesh post-processing (optimizations, tangents, etc)
	for (uint uiMesh(0); uiMesh < m_pOutModel->GetNumMeshes(); ++uiMesh)
	{
		COutMesh *pMesh(m_pOutModel->GetMesh(uiMesh));
		if (pMesh == NULL)
			continue;

		COutBone *pBone(m_pOutModel->GetBone(pMesh->bonelink));
		if (pBone != NULL)
		{
			// sometimes matrices are given to us in the wrong 
			// coordinate system, so we correct for it here
			CVec3f v3Cross(CrossProduct(pBone->m_base.axis[0], pBone->m_base.axis[1]));
			if (DotProduct(v3Cross, pBone->m_base.axis[2]) < 0.0f)
			{
				cout << pBone->GetName() << _T(" is left handed (flipped)") << endl;

				pBone->m_base.axis[0] *= -1.0f;
				pBone->m_base.axis[1] *= -1.0f;
				pBone->m_base.axis[2] *= -1.0f;

				XMATRIX mat;
				mat.m[0][0] = pBone->m_base.axis[0][0];
				mat.m[0][1] = pBone->m_base.axis[0][1];
				mat.m[0][2] = pBone->m_base.axis[0][2];
				mat.m[0][3] = 0.0f;

				mat.m[1][0] = pBone->m_base.axis[1][0];
				mat.m[1][1] = pBone->m_base.axis[1][1];
				mat.m[1][2] = pBone->m_base.axis[1][2];
				mat.m[1][3] = 0.0f;

				mat.m[2][0] = pBone->m_base.axis[2][0];
				mat.m[2][1] = pBone->m_base.axis[2][1];
				mat.m[2][2] = pBone->m_base.axis[2][2];
				mat.m[2][3] = 0.0f;

				mat.m[3][0] = pBone->m_base.pos[0];
				mat.m[3][1] = pBone->m_base.pos[1];
				mat.m[3][2] = pBone->m_base.pos[2];
				mat.m[3][3] = 1.0f;

				XMATRIX matInv;
				XMatrixInverse(&matInv, NULL, &mat);

				pBone->m_invBase.axis[0][0] = matInv.m[0][0];
				pBone->m_invBase.axis[0][1] = matInv.m[0][1];
				pBone->m_invBase.axis[0][2] = matInv.m[0][2];

				pBone->m_invBase.axis[1][0] = matInv.m[1][0];
				pBone->m_invBase.axis[1][1] = matInv.m[1][1];
				pBone->m_invBase.axis[1][2] = matInv.m[1][2];

				pBone->m_invBase.axis[2][0] = matInv.m[2][0];
				pBone->m_invBase.axis[2][1] = matInv.m[2][1];
				pBone->m_invBase.axis[2][2] = matInv.m[2][2];

				pBone->m_invBase.pos[0] = matInv.m[3][0];
				pBone->m_invBase.pos[1] = matInv.m[3][1];
				pBone->m_invBase.pos[2] = matInv.m[3][2];

				pMesh->ReverseWinding();
				//pMesh->ReverseNormals();
			}
		}

		pMesh->ExpandVertexes();
		pMesh->CalcMeshTangents();
		pMesh->Optimize();
	}
}