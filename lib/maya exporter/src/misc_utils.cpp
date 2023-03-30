// (C)2008 S2 Games
// misc_utils.cpp
//
// Various little functions (most taken from shared) used by the exporter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "exporter_common.h"

#include "misc.h"
#include "c_outmesh.h"
#include "c_outclip.h"
//=============================================================================

/*====================
  CreateFaceAndVertexData
  ====================*/
bool	CreateFaceAndVertexData(COutMesh *pOutMesh, MItDag &itDag, IndexVectorVector &comparisonGroups)
{
	MFnMesh fnMesh(itDag.currentItem());

	// Get face iterator
	MItMeshPolygon itPolygon(fnMesh.object());
	cout << "Faces: " << itPolygon.count() << endl;

	// Count triangles
	uint uiTriangleCount(0);
	for (itPolygon.reset(); !itPolygon.isDone(); itPolygon.next())
	{
		if (!itPolygon.hasValidTriangulation())
		{
			g_ModelExporter.Error("Face #" + XtoA(itPolygon.index()) + "has no valid triangulation");
			return false;
		}

		int iTriCount;
		if (itPolygon.numTriangles(iTriCount) == MS::kSuccess)
			uiTriangleCount += iTriCount;
	}

	pOutMesh->numFaces = uiTriangleCount;

	cout << "Tris: " << uiTriangleCount << endl;

	// Get vertex count
	uint uiVertexCount(uiTriangleCount * 3);
	pOutMesh->SetVertexCount(uiVertexCount);

	cout << "Tri verts: " << uiVertexCount << endl;

	// Allocate storage in the output model
	pOutMesh->verts = new CVec3f[uiVertexCount];
	pOutMesh->normals = new CVec3f[uiVertexCount];
	pOutMesh->faceList = new CVec3ui[uiTriangleCount];

	if (fnMesh.numColorSets() > 0)
		pOutMesh->colors[0] = new CVec4b[uiVertexCount];

	if (fnMesh.numUVSets() > 0)
		pOutMesh->tverts[0] = new CVec2f[uiVertexCount];

	// Allocate new space for the bone links, but
	// save the originals to read from
	SBlendedLink *pBaseBlendedLinks = pOutMesh->blendedLinks;
	singleLink_t *pBaseSingleLinks = pOutMesh->singleLinks;
	switch(pOutMesh->mode)
	{
	case MESH_SKINNED_BLENDED:
		pOutMesh->blendedLinks = new SBlendedLink[pOutMesh->GetVertexCount()];
		break;

	case MESH_SKINNED_NONBLENDED:
		if (pOutMesh->bonelink == -1)
			pOutMesh->singleLinks = new singleLink_t[pOutMesh->GetVertexCount()];
		break;
	}

	// Initialize bounding box
	pOutMesh->bbBounds.Clear();

	// Store vertex data
	uint uiOutVertexIndex(0);
	uint uiOutFaceIndex(0);
	for (itPolygon.reset(); !itPolygon.isDone(); itPolygon.next())
	{
		//cout << "face: " << itPolygon.index() << endl;

		// Retrieve arrays of vertices for all triangles in this face
		MPointArray aVerts;
		MIntArray aIndices;
		itPolygon.getTriangles(aVerts, aIndices);

		//cout << "tri-verts: " << aVerts.length() << endl;
	
		// Map object-relative vertex indices to face-relative indices
		imapi mapObjectToFace;
		for (uint uiIndex(0); uiIndex < itPolygon.polygonVertexCount(); ++uiIndex)
			mapObjectToFace[itPolygon.vertexIndex(uiIndex)] = uiIndex;
		
		//for (imapi_it it(mapObjectToFace.begin()), itEnd(mapObjectToFace.end()); it != itEnd; ++it)
		//	cout << "  " << it->first << " -> " << it->second << endl;

		// Process each triangle of the face
		for (uint uiFaceVert(0); uiFaceVert < aVerts.length(); )
		{
			// Process each vertex of the triangle
			for (uint uiTriVert(0) ; uiTriVert < 3; ++uiTriVert, ++uiFaceVert)
			{
				uint uiFaceIndex(mapObjectToFace[aIndices[uiFaceVert]]);

				//cout << "vertex: " << uiOutVertexIndex << " (" << uiFaceIndex << ")" << endl;

				// Position
				pOutMesh->verts[uiOutVertexIndex][X] = aVerts[uiFaceVert].x;
				pOutMesh->verts[uiOutVertexIndex][Y] = aVerts[uiFaceVert].y;
				pOutMesh->verts[uiOutVertexIndex][Z] = aVerts[uiFaceVert].z;
				pOutMesh->bbBounds.AddPoint(pOutMesh->verts[uiOutVertexIndex]);

				//cout << "  pos: (" << aVerts[uiFaceVert].x << ", " << aVerts[uiFaceVert].y << ", " << aVerts[uiFaceVert].z << ")" << endl;

				// Normal
				if (pOutMesh->normals != NULL)
				{
					MVector vecNormal;
					if (itPolygon.getNormal(uiFaceIndex, vecNormal) != MS::kSuccess)
					{
						pOutMesh->normals[uiOutVertexIndex].Clear();
						g_ModelExporter.Error("Failed retrieving normal for face #" + XtoA(itPolygon.index()) + ", vertex #" + XtoA(uiFaceIndex));
					}
					else
					{
						pOutMesh->normals[uiOutVertexIndex][X] = vecNormal.x;
						pOutMesh->normals[uiOutVertexIndex][Y] = vecNormal.y;
						pOutMesh->normals[uiOutVertexIndex][Z] = vecNormal.z;

						//cout << "  normal: (" << vecNormal.x << ", " << vecNormal.y << ", " << vecNormal.z << ")" << endl;
					}
				}

				// Color
				if (pOutMesh->colors[0] != NULL)
				{
					MColor color;
					if (itPolygon.getColor(color, uiFaceIndex) != MS::kSuccess)
					{
						pOutMesh->colors[uiOutVertexIndex]->Clear();
						g_ModelExporter.Error("Failed retrieving color for face #" + XtoA(itPolygon.index()) + ", vertex #" + XtoA(uiFaceIndex));
					}
					else
					{
						pOutMesh->colors[0][uiOutVertexIndex][R] = CLAMP(color.r, 0.0f, 1.0f) * 255;
						pOutMesh->colors[0][uiOutVertexIndex][G] = CLAMP(color.g, 0.0f, 1.0f) * 255;
						pOutMesh->colors[0][uiOutVertexIndex][B] = CLAMP(color.b, 0.0f, 1.0f) * 255;
						pOutMesh->colors[0][uiOutVertexIndex][A] = CLAMP(color.a, 0.0f, 1.0f) * 255;

						//cout << "  color: (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << endl;
					}
				}

				// Texture coords
				if (pOutMesh->tverts[0] != NULL)
				{
					float2 uvPoint;
					if (itPolygon.getUV(uiFaceIndex, uvPoint) != MS::kSuccess)
					{
						pOutMesh->colors[uiOutVertexIndex]->Clear();
						g_ModelExporter.Error("Failed retrieving UV for face #" + XtoA(itPolygon.index()) + ", vertex #" + XtoA(uiFaceIndex));
					}
					else
					{
						pOutMesh->tverts[0][uiOutVertexIndex][0] = uvPoint[0];
						pOutMesh->tverts[0][uiOutVertexIndex][1] = uvPoint[1];

						//cout << "  UV: (" << uvPoint[0] << ", " << uvPoint[1] << ")" << endl;
					}
				}

				// Store bone links
				if (pOutMesh->bonelink == -1)
				{
					switch(pOutMesh->mode)
					{
					case MESH_SKINNED_BLENDED:
						pOutMesh->blendedLinks[uiOutVertexIndex] = pBaseBlendedLinks[uiOutVertexIndex];
						break;

					case MESH_SKINNED_NONBLENDED:
						pOutMesh->singleLinks[uiOutVertexIndex] = pBaseSingleLinks[uiOutVertexIndex];
						break;
					}
				}

				++uiOutVertexIndex;
			}

			// Store face
			pOutMesh->faceList[uiOutFaceIndex][0] = uiOutFaceIndex * 3 + 0;
			pOutMesh->faceList[uiOutFaceIndex][1] = uiOutFaceIndex * 3 + 1;
			pOutMesh->faceList[uiOutFaceIndex][2] = uiOutFaceIndex * 3 + 2;

			++uiOutFaceIndex;
		}
	}

	CalcMeshTangents(pOutMesh);

	return true;
}


/*====================
  CopyMatrix3
  ====================*/
void	CopyMatrix3(const MMatrix &a, matrix43_t &b)
{
	b.axis[0][0] = a(0, 0);
	b.axis[0][1] = a(0, 1);
	b.axis[0][2] = a(0, 2);
	b.axis[1][0] = a(1, 0);
	b.axis[1][1] = a(1, 1);
	b.axis[1][2] = a(1, 2);
	b.axis[2][0] = a(2, 0);
	b.axis[2][1] = a(2, 1);
	b.axis[2][2] = a(2, 2);
	b.pos[0] = a(3, 0);
	b.pos[1] = a(3, 1);
	b.pos[2] = a(3, 2);
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


#if 0
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
#endif


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

		mesh->tangents[t] = new CVec3f[mesh->GetVertexCount()];
		mesh->signs[t] = new byte[mesh->GetVertexCount()];

		for (uint v(0); v < mesh->GetVertexCount(); ++v)
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
