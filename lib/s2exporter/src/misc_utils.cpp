// (C)2008 S2 Games
// misc_utils.cpp
//
// Various little functions (most taken from shared) used by the exporter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "s2exporter.h"

#include "misc.h"
#include "c_outmesh.h"
#include "s_blendedlink.h"
#include "c_outclip.h"
//=============================================================================

/*====================
  fmt

  for passing a formatted string into any function that accepts a char * parameter
  borrowing from a function i saw in the q3 game code
  ====================*/
char*		fmt(const char *s, ...)
{
	const int NUM_FMT_STRINGS = 8;
	static char buf[NUM_FMT_STRINGS][8192];
	static uint marker = 0;
	uint idx = marker;

	va_list argptr;

	va_start(argptr, s);

	if (_vsnprintf(buf[idx], 8191, s, argptr) == -1)
	{
		cerr << _T("Buffer overflow occured during fmt()") << endl;
		buf[idx][8191] = 0;
	}

	va_end(argptr);

	marker = (marker + 1) % NUM_FMT_STRINGS;

	return buf[idx];
}


/*====================
  FilenameGetDir
 ====================*/
const char*	FilenameGetDir(const char *filename)
{
	static char dir[1024];
	_splitpath(filename, NULL, dir, NULL, NULL);
	return dir;
}


/*====================
  IsNodeBone
  ====================*/
bool	IsNodeBone(INode *pNode)
{
	if(pNode == NULL)
		return false;
	if (pNode->IsRootNode())
		return false;

	ObjectState os = pNode->EvalWorldState(0);
	if(os.obj->ClassID() == Class_ID(BONE_CLASS_ID, 0))
		return true;
	// dct 10/18/01, r4 creates bones as objects rather than helpers
	if(os.obj->ClassID() == BONE_OBJ_CLASSID)
		return true;

	// we don't want biped end effectors
	if(os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0))
		return false;

	Control *cont = pNode->GetTMController();
	if(cont->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||
		cont->ClassID() == BIPBODY_CONTROL_CLASS_ID /*||
		cont->ClassID() == FOOTPRINT_CLASS_ID*/)
		return true;

	return false;
}


/*====================
  GetBoneTM

  if it's a true bone, return the node tm with scale removed
  otherwise return the object TM (to take into account object offsets)
  ====================*/
Matrix3	GetBoneTM(INode *pNode, TimeValue t)
{
	Matrix3 tm;

	tm = pNode->GetNodeTM(t);
	//tm.NoScale();

	return tm;
}


/*====================
  GetNumModifiers
  ====================*/
int		GetNumModifiers(INode *nodePtr)
{
	int numMods = 0;

	if (!nodePtr)
		return 0;

	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr)
		return NULL;

	while (ObjectPtr && ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);

		numMods += DerivedObjectPtr->NumModifiers();

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	return numMods;
}


/*====================
  FindModifier
  ====================*/
Modifier*	FindModifier(INode *nodePtr, const Class_ID &class_id)
{
	// Get object from node. Abort if no object.
	if (!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr ) return NULL;

	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		// Yes -> Cast.
		IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);

		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;

		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this the correct modifier?
			if (ModifierPtr->ClassID() == class_id)
			{
				//ModContext *mc = DerivedObjectPtr->GetModContext(ModStackIndex);
				return ModifierPtr;
			}

			++ModStackIndex;
		}

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return NULL;
}


/*====================
  IsMesh
  ====================*/
bool	IsMesh(INode *node, Interface *ip)
{
	Object *obj = node->EvalWorldState(ip->GetTime()).obj;
	if (!obj)
		return false;

	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		return true;

	return false;
}


/*====================
  GetVertexNormal

  modified from ascii exporter
  ====================*/
Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;

	// Is normal specified
	// SPECIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL)
	{
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup)
	{
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1)
		{
			vertexNormal = rv->rn.getNormal();
		}
		else
		{
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++)
			{
				if (rv->ern[i].getSmGroup() & smGroup)
					vertexNormal = rv->ern[i].getNormal();
			}
		}
	}
	else
	{
		vertexNormal = mesh->getFaceNormal(faceNo);
	}

	return vertexNormal;
}


/*====================
  CreateFaceAndVertexData
  ====================*/
vector< vector<size_t> >	CreateFaceAndVertexData(Mesh *pMaxMesh, COutMesh *pOutMesh, INode *pNode, Matrix3 &tm)
{
	vector< vector<size_t> >	comparisonGroups(pMaxMesh->getNumVerts());

	Matrix3 vertTM = tm;

	tm.NoScale();
	tm.NoTrans();
	pMaxMesh->buildNormals();

	int iNumMeshFaces = pMaxMesh->getNumFaces();
	int iNumMeshVerts = iNumMeshFaces * 3;

	pOutMesh->num_verts = iNumMeshVerts;

	// Allocate vertex position list
	pOutMesh->verts = new CVec3f[iNumMeshVerts];
	if (pOutMesh->verts == NULL)
		throw "FinishMesh: ReAllocVerts failed";

	// Allocate normals array, if it is requested
	BOOL excludeNormals = FALSE;
	pNode->GetUserPropBool("exclude_normals", excludeNormals);
	if (!excludeNormals)
	{
		pOutMesh->normals = new CVec3f[iNumMeshVerts];
		if (pOutMesh->normals == NULL)
			throw "FinishMesh: failed to allocate normals";
	}

	// Allocate face list
	pOutMesh->faceList = new CVec3ui[iNumMeshFaces];
	if (pOutMesh->faceList == NULL)
		throw "Failed to allocate face list";
	pOutMesh->numFaces = iNumMeshFaces;

	// Allocate new space for the bone links, but
	// save the originals to read from
	SBlendedLink *pBaseBlendedLinks = pOutMesh->blendedLinks;
	singleLink_t *pBaseSingleLinks = pOutMesh->singleLinks;
	switch(pOutMesh->mode)
	{
	/*case MESH_KEYFRAMED:
		for (int n = 0; n < GetNumFrames(); ++n)
		{
			MemManager.Reallocat(pOutMesh->keyframes[n].key_verts, pOutMesh->num_verts * sizeof(keyVert_t));
			if (pOutMesh->keyframes[n].key_verts == NULL)
				throw "Failed to reallocate keyframe links;
		}
		break;*/

	case MESH_SKINNED_BLENDED:
		pOutMesh->blendedLinks = new SBlendedLink[pOutMesh->num_verts];
		if (pOutMesh->blendedLinks == NULL)
			throw "Failed to reallocate links";
		break;

	case MESH_SKINNED_NONBLENDED:
		if (pOutMesh->bonelink != -1)
			break;

		pOutMesh->singleLinks = new singleLink_t[pOutMesh->num_verts];
		if (pOutMesh->singleLinks == NULL)
			throw "Failed to reallocate links";
		break;
	}

	// Initialize bounding box
	pOutMesh->bbBounds.Clear();

	// Iterate through each mapping channel
	int iNumChannels = MIN(pMaxMesh->getNumMaps(), MAX_UV_CHANNELS + MAX_VERTEX_COLOR_CHANNELS);
	for (int iChannel = -3; iChannel < iNumChannels; ++iChannel)
	{
		// Skip 'illumination' channel
		if (iChannel == -1)
			continue;

		if (!pMaxMesh->mapSupport(iChannel) && iChannel != -3)
			continue;

		// Sanity check the face list
		int iNumFaces;
		if (iChannel == -3)
			iNumFaces = pMaxMesh->getNumFaces();
		else
			iNumFaces = pMaxMesh->Map(iChannel).getNumFaces();
		if (iNumFaces != iNumMeshFaces)
		{
			g_S2ModelExporter.AddMessage(fmt("WARNING: skipping map with %i faces, because it does not match the mesh (%i faces)", iNumFaces, iNumMeshFaces));
			continue;
		}

		// Perform allocations
		switch (iChannel)
		{
		case -3:	// Geometry, this is always allocated
			break;

		case -2:	// Alpha
			pOutMesh->colors[0] = new CVec4b[iNumMeshVerts];
			if (pOutMesh->colors[0] == NULL)
				throw "FinishMesh: failed to allocate colors";
			memset(pOutMesh->colors[0], 0, iNumMeshVerts * sizeof(bvec4_t));
			break;

		case 0:		// Color
		case 9:
			// Check to see if alpha allocated this already
			if (pOutMesh->colors[iChannel / 9] != NULL)
				break;
			pOutMesh->colors[iChannel / 9] = new CVec4b[iNumMeshVerts];
			if (pOutMesh->colors[iChannel / 9] == NULL)
				throw "FinishMesh: failed to allocate colors";
			break;

		default:	// Texture map coords
			pOutMesh->tverts[iChannel - 1] = new CVec2f[iNumMeshVerts];
			if (pOutMesh->tverts[iChannel - 1] == NULL)
				throw "FinishMesh: failed to allocate tverts";
			break;
		}

		// Iterate through each face of the map
		for (int iFace = 0; iFace < iNumFaces; ++iFace)
		{
			// Get data for each vertex of the face
			for (int iVert = 0; iVert < 3; ++iVert)
			{
				size_t	zThisVert = iFace * 3 + iVert;

				// Geometry
				// Channel -3 doesn't actually exist in max, this is just a way to
				// handle meshes that don't export any mapping channels
				if (iChannel == -3)
				{
					// Build comparison map
					comparisonGroups[pMaxMesh->faces[iFace].v[iVert]].push_back(zThisVert);

					// Store the position
					Point3 pos = pMaxMesh->verts[pMaxMesh->faces[iFace].v[iVert]] * vertTM;
					pOutMesh->verts[zThisVert][0] = pos.x;
					pOutMesh->verts[zThisVert][1] = pos.y;
					pOutMesh->verts[zThisVert][2] = pos.z;
					pOutMesh->bbBounds.AddPoint(pOutMesh->verts[zThisVert]);

					// Store the normal
					if (pOutMesh->normals != NULL)
					{
						Point3 nml = GetVertexNormal(pMaxMesh, iFace, pMaxMesh->getRVertPtr(pMaxMesh->faces[iFace].v[iVert]));
						nml.Normalize();
						nml = tm * nml;
						nml.Normalize();
						pOutMesh->normals[zThisVert][0] = nml.x;
						pOutMesh->normals[zThisVert][1] = nml.y;
						pOutMesh->normals[zThisVert][2] = nml.z;
					}

					if (pOutMesh->bonelink != -1)
						continue;

					// Store bone links
					switch(pOutMesh->mode)
					{
					//case MESH_KEYFRAMED:

					case MESH_SKINNED_BLENDED:
						pOutMesh->blendedLinks[zThisVert] = pBaseBlendedLinks[pMaxMesh->faces[iFace].v[iVert]];
						break;

					case MESH_SKINNED_NONBLENDED:
						pOutMesh->singleLinks[zThisVert] = pBaseSingleLinks[pMaxMesh->faces[iFace].v[iVert]];
						break;
					}
					continue;
				}

				// Get the index
				TVFace thisFace = pMaxMesh->Map(iChannel).tf[iFace];
				int iVertIndex = thisFace.t[iVert];

				// Alpha
				if (iChannel == -2)
				{
					Point3 alpha = pMaxMesh->mapVerts(iChannel)[iVertIndex];
					pOutMesh->colors[0][zThisVert][3] = alpha.x * 255;
					continue;
				}

				// Vertex colors
				if (iChannel == 0 || iChannel == 9)
				{
					int index = iChannel / 9;
					Point3 col = pMaxMesh->mapVerts(iChannel)[iVertIndex];
					pOutMesh->colors[index][zThisVert][0] = col.x * 255;
					pOutMesh->colors[index][zThisVert][1] = col.y * 255;
					pOutMesh->colors[index][zThisVert][2] = col.z * 255;
					continue;
				}

				// Texture coords
				Point3 uv = pMaxMesh->mapVerts(iChannel)[iVertIndex];
				pOutMesh->tverts[iChannel - 1][zThisVert][0] = uv.x;
				pOutMesh->tverts[iChannel - 1][zThisVert][1] = uv.y;
			}

			if (tm.Parity())
			{
				pOutMesh->faceList[iFace][0] = iFace * 3 + 2;
				pOutMesh->faceList[iFace][1] = iFace * 3 + 1;
				pOutMesh->faceList[iFace][2] = iFace * 3 + 0;
			}
			else
			{
				pOutMesh->faceList[iFace][0] = iFace * 3 + 0;
				pOutMesh->faceList[iFace][1] = iFace * 3 + 1;
				pOutMesh->faceList[iFace][2] = iFace * 3 + 2;
			}
		}
	}

	CalcMeshTangents(pOutMesh);

	return comparisonGroups;
}


/*====================
  ComputeBounds

  compute bounding box in MESH SPACE
  ====================*/
void	ComputeBounds(Mesh *mesh, Matrix3 *tm, CBBoxf &bbBounds)
{
	bbBounds.Clear();

	for (int n = 0; n < mesh->numVerts; ++n)
	{
		Point3 p = mesh->verts[n] * (*tm);

		bbBounds.AddPoint(CVec3f(p.x, p.y, p.z));
	}

	// Expand the bounds by an epsilon to prevent divide by 0 errors
	bbBounds.Expand(0.0001f);
}


/*====================
  MatrixToTransform
  ====================*/
void	MatrixToTransform(Matrix3 &tm, transform_t *transform)
{
	Point3 axis[3];
	Point3 pos;

	axis[0] = tm.GetRow(0);
	axis[1] = tm.GetRow(1);
	axis[2] = tm.GetRow(2);
	pos = tm.GetRow(3);

	transform->t.axis[0][0] = axis[0].x;
	transform->t.axis[0][1] = axis[0].y;
	transform->t.axis[0][2] = axis[0].z;
	transform->t.axis[0][3] = 0;
	transform->t.axis[1][0] = axis[1].x;
	transform->t.axis[1][1] = axis[1].y;
	transform->t.axis[1][2] = axis[1].z;
	transform->t.axis[1][3] = 0;
	transform->t.axis[2][0] = axis[2].x;
	transform->t.axis[2][1] = axis[2].y;
	transform->t.axis[2][2] = axis[2].z;
	transform->t.axis[2][3] = 0;
	transform->t.pos[0] = pos.x;
	transform->t.pos[1] = pos.y;
	transform->t.pos[2] = pos.z;
	transform->t.pos[3] = 1;
}


/*====================
  CopyMatrix3
  ====================*/
void	CopyMatrix3(Matrix3 &a, matrix43_t &b)
{
	Point3 axis[3];
	Point3 pos;

	axis[0] = a.GetRow(0);
	axis[1] = a.GetRow(1);
	axis[2] = a.GetRow(2);
	pos = a.GetRow(3);

	b.axis[0][0] = axis[0].x;
	b.axis[0][1] = axis[0].y;
	b.axis[0][2] = axis[0].z;
	b.axis[1][0] = axis[1].x;
	b.axis[1][1] = axis[1].y;
	b.axis[1][2] = axis[1].z;
	b.axis[2][0] = axis[2].x;
	b.axis[2][1] = axis[2].y;
	b.axis[2][2] = axis[2].z;
	b.pos[0] = pos.x;
	b.pos[1] = pos.y;
	b.pos[2] = pos.z;
}


/*====================
  IsNodeSelected
  ====================*/
bool	IsNodeSelected(INode *node)
{
	for (int n = 0; n < GetCOREInterface()->GetSelNodeCount(); ++n)
	{
		if (GetCOREInterface()->GetSelNode(n) == node)
			return true;
	}

	return false;
}


/*====================
  AllocateFloatKeys
  ====================*/
void	AllocateFloatKeys(SFloatKeys *keys, int numframes)
{
	keys->keys = new float[numframes];
	keys->num_keys = 0;

	keys->keys[0] = 0.0f;
}


/*====================
  AllocateByteKeys
  ====================*/
void	AllocateByteKeys(SByteKeys *keys, int numframes)
{
	keys->keys = new byte[numframes];
	keys->num_keys = 0;

	keys->keys[0] = 0;
}


/*====================
  CompressFloatKeys
  ====================*/
void	CompressFloatKeys(SFloatKeys *keys)
{
	for (int n = 1; n < keys->num_keys; ++n)
	{
		if (abs(keys->keys[n] - keys->keys[0]) > 0.0001)
			return;
	}

	// all keys are the same, so only store one key
	keys->num_keys = 1;
}


/*====================
  CompressByteKeys
  ====================*/
void	CompressByteKeys(SByteKeys *keys)
{
	for (int n = 1; n < keys->num_keys; ++n)
	{
		if (keys->keys[n] != keys->keys[0])
			return;
	}

	//all keys are the same, so only store one key
	keys->num_keys = 1;
}


/*====================
  GetFaceTangent
  ====================*/
CVec3f	GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2)
{
	CVec3f A(CrossProduct(CVec3f(v0.x, t0.x, t0.y) - CVec3f(v1.x, t1.x, t1.y), CVec3f(v0.x, t0.x, t0.y) - CVec3f(v2.x, t2.x, t2.y)));
	CVec3f B(CrossProduct(CVec3f(v0.y, t0.x, t0.y) - CVec3f(v1.y, t1.x, t1.y), CVec3f(v0.y, t0.x, t0.y) - CVec3f(v2.y, t2.x, t2.y)));
	CVec3f C(CrossProduct(CVec3f(v0.z, t0.x, t0.y) - CVec3f(v1.z, t1.x, t1.y), CVec3f(v0.z, t0.x, t0.y) - CVec3f(v2.z, t2.x, t2.y)));

	// return the tangent
	return CVec3f(A.x == 0.0f ? FAR_AWAY : -A.y/A.x, B.x == 0.0f ? FAR_AWAY : -B.y/B.x, C.x == 0.0f ? FAR_AWAY : -C.y/C.x).Direction();
}


const float MAX_TANGENT_SMOOTH_ANGLE = 135.0f;

/*====================
  CalcMeshTangents

  This assumes the vertexes are in face order (faceNum * 3 + vertexIndex)
  ====================*/
void	CalcMeshTangents(COutMesh *mesh)
{
	if (!mesh->normals)
		return;

	for (int t = 0; t < MAX_UV_CHANNELS; ++t)
	{
		if (!mesh->tverts[t])
			continue;

		mesh->tangents[t] = new CVec3f[mesh->num_verts];
		mesh->signs[t] = new byte[mesh->num_verts];

		for (int v = 0; v < mesh->num_verts; ++v)
		{
			int iOwner = v/3;
			CVec3f ov0(mesh->verts[mesh->faceList[iOwner][0]][0], mesh->verts[mesh->faceList[iOwner][0]][1], mesh->verts[mesh->faceList[iOwner][0]][2]);
			CVec3f ov1(mesh->verts[mesh->faceList[iOwner][1]][0], mesh->verts[mesh->faceList[iOwner][1]][1], mesh->verts[mesh->faceList[iOwner][1]][2]);
			CVec3f ov2(mesh->verts[mesh->faceList[iOwner][2]][0], mesh->verts[mesh->faceList[iOwner][2]][1], mesh->verts[mesh->faceList[iOwner][2]][2]);

			CVec2f ot0(mesh->tverts[t][mesh->faceList[iOwner][0]][0], mesh->tverts[t][mesh->faceList[iOwner][0]][1]);
			CVec2f ot1(mesh->tverts[t][mesh->faceList[iOwner][1]][0], mesh->tverts[t][mesh->faceList[iOwner][1]][1]);
			CVec2f ot2(mesh->tverts[t][mesh->faceList[iOwner][2]][0], mesh->tverts[t][mesh->faceList[iOwner][2]][1]);

			bool bOwnerSign(((ot1.x - ot0.x) * (ot2.y - ot0.y) - (ot1.y - ot0.y) * (ot2.x - ot0.x)) > 0.0f);

			mesh->signs[t][v] = bOwnerSign ? 255 : 0;

			CVec3f vOwnerTangent = GetFaceTangent(ov0, ov1, ov2, ot0, ot1, ot2);

			// Figure out which faces this vertex belongs to
			ivector vFaceList;

			for (int f = 0; f < mesh->numFaces; ++f)
			{
				// make a list of any face that contains a vert that matches our position and normal
				if ((memcmp(&mesh->verts[mesh->faceList[f][0]], &mesh->verts[v], sizeof(vec3_t)) == 0 ||
					memcmp(&mesh->verts[mesh->faceList[f][1]], &mesh->verts[v], sizeof(vec3_t)) == 0 ||
					memcmp(&mesh->verts[mesh->faceList[f][2]], &mesh->verts[v], sizeof(vec3_t)) == 0) &&
					(memcmp(&mesh->normals[mesh->faceList[f][0]], &mesh->normals[v], sizeof(vec3_t)) == 0 ||
					memcmp(&mesh->normals[mesh->faceList[f][1]], &mesh->normals[v], sizeof(vec3_t)) == 0 ||
					memcmp(&mesh->normals[mesh->faceList[f][2]], &mesh->normals[v], sizeof(vec3_t)) == 0))
				{
					vFaceList.push_back(f);
				}
			}

			// Calculate a tangent for each face then smooth the tangents together
			CVec3f vFinalTangent(0.0f, 0.0f, 0.0f);

			for (ivector::iterator it = vFaceList.begin(); it != vFaceList.end(); ++it)
			{
				int iFace = *it;

				CVec3f v0(mesh->verts[mesh->faceList[iFace][0]][0], mesh->verts[mesh->faceList[iFace][0]][1], mesh->verts[mesh->faceList[iFace][0]][2]);
				CVec3f v1(mesh->verts[mesh->faceList[iFace][1]][0], mesh->verts[mesh->faceList[iFace][1]][1], mesh->verts[mesh->faceList[iFace][1]][2]);
				CVec3f v2(mesh->verts[mesh->faceList[iFace][2]][0], mesh->verts[mesh->faceList[iFace][2]][1], mesh->verts[mesh->faceList[iFace][2]][2]);

				CVec2f t0(mesh->tverts[t][mesh->faceList[iFace][0]][0], mesh->tverts[t][mesh->faceList[iFace][0]][1]);
				CVec2f t1(mesh->tverts[t][mesh->faceList[iFace][1]][0], mesh->tverts[t][mesh->faceList[iFace][1]][1]);
				CVec2f t2(mesh->tverts[t][mesh->faceList[iFace][2]][0], mesh->tverts[t][mesh->faceList[iFace][2]][1]);

				CVec3f vTangent = GetFaceTangent(v0, v1, v2, t0, t1, t2);
				
				bool bSign(((t1.x - t0.x) * (t2.y - t0.y) - (t1.y - t0.y) * (t2.x - t0.x)) > 0.0f);
				
				if (bSign != bOwnerSign)
					vTangent *= -1.0f;

				if (DotProduct(vTangent, vOwnerTangent) >= cos(DEG2RAD(MAX_TANGENT_SMOOTH_ANGLE)))
					vFinalTangent += vTangent;
			}

			vFinalTangent.Normalize();

			// do a quick Gram-Schmidt orthogonalize
			vFinalTangent -= mesh->normals[v] * DotProduct(mesh->normals[v], vFinalTangent);
			vFinalTangent.Normalize();

			mesh->tangents[t][v] = vFinalTangent;
		}
	}
}

