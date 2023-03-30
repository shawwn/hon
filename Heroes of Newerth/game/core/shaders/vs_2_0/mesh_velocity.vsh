// (C)2008 S2 Games
// mesh_velocity.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "../common/common.h"

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;	         // World * View * Projection transformation
float4x4	mWorldView;	             // World * View
float3x3	mWorldViewRotate;        // World rotation for normals

#if (NUM_BONES > 0)
float4x3		vBones[NUM_BONES];
#endif

float		fTime;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position      : POSITION;
	float2 Texcoord0     : TEXCOORD0;
	float ViewDepth      : TEXCOORD1;
	float4 Position0     : TEXCOORD2;
	float4 Position1     : TEXCOORD3;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position   : POSITION;
	float3 Normal     : NORMAL;
#if (TEXCOORDS > 0)
	float2 Texcoord0  : TEXCOORD0;
#endif
#if (NUM_BONES > 0)
	int4 BoneIndex    : TEXCOORD_BONEINDEX;
	float4 BoneWeight : TEXCOORD_BONEWEIGHT;
#endif
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;
	
	float3 vInNormal = (In.Normal / 255.0f) * 2.0f - 1.0f;
	
#if (NUM_BONES > 0)
	float3 vPosition = 0.0f;
	float3 vNormal = 0.0f;

	//
	// GPU Skinning
	// Blend bone matrix transforms for this bone
	//
	
	int4 vBlendIndex = In.BoneIndex;
	float4 vBoneWeight = In.BoneWeight / 255.0f;
	
	float4x3 mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	vPosition = mul(float4(In.Position, 1.0f), mBlend);
	vNormal = mul(float4(vInNormal, 0.0f), mBlend).xyz;
	
	vNormal = normalize(vNormal);
#else
	float3 vPosition = In.Position;
	float3 vNormal = vInNormal;
#endif

	float3 vSpeed = float3(0.0f, 32.0f, 0.0f);

	float3 vPosition0 = vPosition - vSpeed;
	float3 vPosition1 = vPosition + vSpeed;
	
	if (dot(vSpeed, vNormal) >= 0.0f)
		vPosition += vSpeed;
	else
		vPosition -= vSpeed;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Position0     = mul(float4(vPosition0, 1.0f), mWorldViewProj);
	Out.Position1     = mul(float4(vPosition1, 1.0f), mWorldViewProj);
	Out.Texcoord0     = In.Texcoord0;
	Out.ViewDepth     = -mul(float4(vPosition, 1.0f), mWorldView).z / 3000.0f;

	return Out;
}
