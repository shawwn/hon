// (C)2007 S2 Games
// mesh_unlit_shell_2denviro_anim.vsh
// 
// 
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "../common/common.h"
#include "../common/fog.h"

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;			// World * View * Projection transformation
float4x4	mWorldView;				// World * View
float4x4	mWorld;					// World transformation
float3x3	mWorldViewRotate;		// World rotation for normals

float4		vColor;

#if (NUM_BONES > 0)
float4x3	vBones[NUM_BONES];
#endif

#if (FOG_OF_WAR == 1)
float4x4	mFowProj;
#endif

float4		vCameraPosition;

float		fTexRotate;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position      : POSITION;
	float4 Color0        : COLOR0;
	float2 Texcoord0     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#if (FALLOFF_QUALITY == 0 && FOG_TYPE != 0)
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && FOG_TYPE != 0) || FOG_OF_WAR == 1)
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
	
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && FOG_TYPE != 0) || FOG_OF_WAR == 1)
	Out.Last = 0;
#endif
	
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

	vPosition += vNormal * vColor.a;

	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0.rgb    = vColor.rgb;
	Out.Color0.a      = 1.0f;
	
	// 2d Enviro reflection
	float3 vCameraDirection = normalize(vPosition - vCameraPosition);
	float3 vRelfection = vNormal * 2.0f * dot(vNormal, vCameraDirection) - vCameraDirection;
	
	float2 vTexcoord = -float3(vRelfection * 0.5f + 0.5f).yz;
	
	// Texcoord animation
	float fS, fC;
	sincos(fTexRotate, fS, fC);
	
	float2x2 mTexRotate;
	mTexRotate[0][0] = fC;
	mTexRotate[1][0] = -fS;
	mTexRotate[0][1] = fS;
	mTexRotate[1][1] = fC;
	vTexcoord = mul(mTexRotate, vTexcoord);
	
	float2 vTexTranslate;
	vTexTranslate[0] = 0.5f + 0.5f * fC + 0.5f * fS;
	vTexTranslate[1] = 0.5f - 0.5f * fS + 0.5f * fC;
	vTexcoord += vTexTranslate;
	
	Out.Texcoord0     = vTexcoord;

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
