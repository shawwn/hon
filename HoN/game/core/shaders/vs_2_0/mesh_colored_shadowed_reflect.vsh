// (C)2007 S2 Games
// mesh_colored_shadowed_reflect.vsh
// 
// Default mesh vertex shader with shadowmap support
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
float4x4	mWorld;			         // World transformation
float4x4	mWorldRotate;            // World rotation for normals

float4		vCameraPositionWorld;
float4		vSunPositionWorld;

float3		vAmbient;
float3		vSunColor;
float3		vSunColorSpec;

float		fGlossiness;

#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		float4x4	mLightWorldViewProjTexSplit[4];  // Light's World * View * Projection * Tex
	#else
		float4x4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
	#endif
#endif

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
	float3 PositionWorld : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Normal        : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#if (LIGHTING_QUALITY == 0)
	float3 Tangent       : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Binormal      : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 1)
	float3 HalfAngle     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 SunLight      : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 2)
	float3 DiffLight  : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 SpecLight  : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 3)
	float3 DiffLight  : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		float4 TexcoordLight0 : TEXCOORDX;
			#include "../common/inc_texcoord.h"
		float4 TexcoordLight1 : TEXCOORDX;
			#include "../common/inc_texcoord.h"
		float4 TexcoordLight2 : TEXCOORDX;
			#include "../common/inc_texcoord.h"
		float4 TexcoordLight3 : TEXCOORDX;
			#include "../common/inc_texcoord.h"
	#else
		float4 TexcoordLight : TEXCOORDX; // Texcoord in light texture space
			#include "../common/inc_texcoord.h"
	#endif
#endif
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
	float4 Color0     : COLOR0;
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

	float3 vPositionWorld = mul(float4(vPosition, 1.0f), mWorld).xyz;
	float3 vCamDirection = normalize(vCameraPositionWorld.xyz - vPositionWorld);
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0        = vColor * In.Color0;
	Out.Texcoord0     = In.Texcoord0;
	Out.PositionWorld  = vPositionWorld;
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = length(vCamDirection);
#endif
	
	Out.Normal        = mul(vNormal, mWorldRotate);
	
#if (LIGHTING_QUALITY == 0)
	Out.Tangent       = mul(vTangent, mWorldRotate);
	Out.Binormal      = cross(Out.Tangent, Out.Normal);
#elif (LIGHTING_QUALITY == 1)
	float3 vLight = vSunPositionWorld.xyz;
	float3 vWVNormal = Out.Normal;
	float3 vWVTangent = mul(vTangent, mWorldRotate);
	float3 vWVBinormal = cross(vWVTangent, vWVNormal);

	float3x3 mRotate = transpose(float3x3(vWVTangent, vWVBinormal, vWVNormal));
	
	Out.SunLight      = mul(vLight, mRotate);
	Out.HalfAngle     = mul(normalize(vLight + vCamDirection), mRotate);
#elif (LIGHTING_QUALITY == 2)
	float3 vLight = vSunPositionWorld.xyz;		
	float3 vHalfAngle = normalize(vLight + vCamDirection);
	float3 vWVNormal = Out.Normal;

	float fDiffuse = saturate(dot(vWVNormal, vLight));
	float fSpecular = saturate(step(0.0f, fDiffuse) * (pow(dot(vWVNormal, vHalfAngle), fGlossiness)));

	Out.DiffLight     = vSunColor * fDiffuse;
	Out.SpecLight     = vSunColorSpec * fSpecular;
#elif (LIGHTING_QUALITY == 3)
	float3 vLight = vSunPositionWorld.xyz;		
	float3 vWVNormal = Out.Normal;

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight     =  vSunColor * fDiffuse;
#endif

#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		Out.TexcoordLight0 = mul(float4(vPosition, 1.0f), mLightWorldViewProjTexSplit[0]);
		Out.TexcoordLight1 = mul(float4(vPosition, 1.0f), mLightWorldViewProjTexSplit[1]);
		Out.TexcoordLight2 = mul(float4(vPosition, 1.0f), mLightWorldViewProjTexSplit[2]);
		Out.TexcoordLight3 = mul(float4(vPosition, 1.0f), mLightWorldViewProjTexSplit[3]);
	#else
		Out.TexcoordLight = mul(float4(vPosition, 1.0f), mLightWorldViewProjTex);
	#endif
#endif

#ifdef CLOUDS
	Out.TexcoordClouds = mul(float4(vPosition, 1.0f), mCloudProj).xy;
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = FogD(length(vCamDirection));
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(float4(vPosition, 1.0f), mFowProj).xy;
#endif

	return Out;
}
