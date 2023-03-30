// (C)2006 S2 Games
// mesh_etheral.vsh
// 
// Etheral vertex shader
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
float3x3	mWorldViewRotate;        // World rotation for normals

float4		vSunPositionView;

float3		vAmbient;
float3		vSunColor;
float3		vSunColorSpec;

float		fGlossiness;

float4		vColor;

#if (NUM_BONES > 0)
	float4x3		vBones[NUM_BONES];
#endif

#ifdef CLOUDS
	float4x4	mCloudProj;
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
	float2 Texcoord0     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Normal        : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Tangent       : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Binormal      : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#ifdef CLOUDS
	float2 TexcoordClouds : TEXCOORDX;
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
	float3 Normal     : NORMAL;
	float2 Texcoord0  : TEXCOORD0;
	float3 Tangent    : TEXCOORD1;
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
	
	float3 vInNormal = (In.Normal / 255.0f) * 2.0f - 1.0f;
	float3 vInTangent = (In.Tangent / 255.0f) * 2.0f - 1.0f;
	
#if (NUM_BONES > 0)
	float3 vPosition = 0.0f;
	float3 vNormal = 0.0f;
	float3 vTangent = 0.0f;

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
	vNormal = mul(vInNormal, mAxis);
	vTangent = mul(vInTangent, mAxis);
	
	vNormal = normalize(vNormal);
	vTangent = normalize(vTangent);
#else
	float3 vPosition = In.Position;
	float3 vNormal = vInNormal;
	float3 vTangent = vInTangent;
#endif

	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0        = vColor;
	Out.Texcoord0     = In.Texcoord0;
	Out.PositionView  = vPositionView;
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = length(vPositionView);
#endif
	
	Out.Normal        = mul(vNormal, mWorldViewRotate);
	Out.Tangent       = mul(vTangent, mWorldViewRotate);
	Out.Binormal      = cross(Out.Tangent, Out.Normal);

#ifdef CLOUDS
	Out.TexcoordClouds = mul(float4(vPosition, 1.0f), mCloudProj).xy;
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = Fog(vPositionView);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(float4(vPosition, 1.0f), mFowProj).xy;
#endif

	return Out;
}
