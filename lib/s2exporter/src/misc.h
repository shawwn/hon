// (C)2008 S2 Games
// misc.h
//
//=============================================================================
#ifndef __MISC_H__
#define __MISC_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
//=============================================================================

// Math stuff
#define DEG2RAD(a) ((a) * (M_PI / 180.0f))
#define RAD2DEG(a) ((a) * (180.0f / M_PI))
#define M_PI	3.14159265358979323846f

#define M_AddVec3(a, b, out) ((out[0]) = (a[0]) + (b[0]), (out[1]) = (a[1]) + (b[1]), (out[2]) = (a[2]) + (b[2]))
#define M_AddVec2(a, b, out) ((out[0]) = (a[0]) + (b[0]), (out[1]) = (a[1]) + (b[1]))
#define M_SubVec3(a, b, out) ((out[0]) = (a[0]) - (b[0]), (out[1]) = (a[1]) - (b[1]), (out[2]) = (a[2]) - (b[2]))
#define M_SubVec2(a, b, out) ((out[0]) = (a[0]) - (b[0]), (out[1]) = (a[1]) - (b[1]))

typedef float			vec_t;
typedef int				ivec_t;
typedef unsigned int	uivec_t;
typedef unsigned char	bvec_t;
#ifndef byte
typedef unsigned char	byte;
#endif
typedef unsigned short int word;
typedef unsigned short int wvec_t;

typedef vec_t	vec2_t[2];
typedef vec_t	vec3_t[3];
typedef vec_t	vec4_t[4];
typedef ivec_t	ivec2_t[2];
typedef uivec_t	uivec3_t[3]; //for facelists
typedef bvec_t	bvec2_t[2];
typedef bvec_t	bvec3_t[3];	 //for single precision mesh vertices
typedef bvec_t	bvec4_t[4];  //for colors
typedef wvec_t	wvec3_t[3];  //for double precision mesh vertices

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

typedef matrix44_t transform_t;

typedef struct
{
	vec3_t			axis[3];
	vec3_t			pos;
} matrix43_t;

struct SFloatKeys;
struct SByteKeys;

//=============================================================================
// Prototypes from misc_utils.cpp
//=============================================================================
char*		fmt(const char *s, ...);
const char*	FilenameGetDir(const char *filename);
bool		IsNodeBone(INode *pNode);
Matrix3		GetBoneTM(INode *pNode, TimeValue t);
int			GetNumModifiers(INode *nodePtr);
Modifier*	FindModifier(INode *nodePtr, const Class_ID &class_id);
bool		IsMesh(INode *node, Interface *ip);
Point3		GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
vector< vector<size_t> >		CreateFaceAndVertexData(Mesh *pMaxMesh, COutMesh *pOutMesh, INode *pNode, Matrix3& tm);
void		ComputeBounds(Mesh *mesh, Matrix3 *tm, CBBoxf &bbBounds);
void		MatrixToTransform(Matrix3 &tm, transform_t *transform);
void		CopyMatrix3(Matrix3 &a, matrix43_t &b);
bool		IsNodeSelected(INode *node);
void		AllocateFloatKeys(SFloatKeys *keys, int numframes);
void		AllocateByteKeys(SByteKeys *keys, int numframes);
void		CompressFloatKeys(SFloatKeys *keys);
void		CompressByteKeys(SByteKeys *keys);
CVec3f		GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f &t1, const CVec2f &t2);
void		CalcMeshTangents(COutMesh *mesh);
//=============================================================================
#endif //__MISC_H__
