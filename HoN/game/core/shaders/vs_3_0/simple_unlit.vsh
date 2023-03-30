// (C)2006 S2 Games
// mesh_unlit.vsh
// 
// Vertex shader for unlit meshes
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "../common/common.h"
#include "../common/fog.h"

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;	         // World * View * Projection transformation
float4x4	mWorldView;	             // World * View
float4x4	mWorld;			         // World transformation
float3x3	mWorldViewRotate;        // World rotation for normals

float4		vColor;

#if (NUM_BONES > 0)
	float4x3		vBones[NUM_BONES];
#endif

#if (FOG_OF_WAR == 1)
	float4x4	mFowProj;
#endif


//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position      : POSITION;
	float4 Color0        : COLOR0;
#if (FALLOFF_QUALITY == 0 && FOG_TYPE != 0)
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
	float4 Last : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
};


//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position   : POSITION;
	float2 Texcoord0  : TEXCOORD0;
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
	
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
	Out.Last = 0;
#endif
	
#if (NUM_BONES > 0)
	float3 vPosition = 0.0f;

	//
	// GPU Skinning
	// Blend bone matrix transforms for this bone
	//
	
	int4 vBlendIndex = In.BoneIndex;
	float4 vBoneWeight = In.BoneWeight / 255.0f;
	
	float4x3	mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	float3x3	mAxis = float3x3(mBlend[0], mBlend[1], mBlend[2]);
	float3		vPos = mBlend[3];
	
	vPosition = mul(In.Position, mAxis) + vPos;
#else
	float3 vPosition = In.Position;
#endif

	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0        = vColor;

#if (FALLOFF_QUALITY == 0 && FOG_TYPE != 0)
	Out.PositionView  = vPositionView;
#endif

#if (FALLOFF_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.z = length(vPositionView);
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = Fog(vPositionView);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(float4(vPosition, 1.0f), mFowProj).xy;
#endif

	return Out;
}
