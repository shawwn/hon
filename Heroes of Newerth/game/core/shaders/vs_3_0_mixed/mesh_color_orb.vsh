// (C)2008 S2 Games
// mesh_color_scene.vsh
// 
// ...
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

float4		vColor;

#if (NUM_BONES > 0)
float4x3	vBones[NUM_BONES];
#endif

#ifdef CLOUDS
float4x4	mCloudProj;
#endif

#if (FOG_OF_WAR == 1)
float4x4	mFowProj;
#endif

float4x4	mSceneProj;

// Material parameters
float2 		vTex0a;
float2 		vTex0b;
float2 		vTex1a;
float2 		vTex1b;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color0 : COLOR0;
	float4 Texcoord0 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float4 Texcoord1 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float4 PositionScreen : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1 || FALLOFF_QUALITY == 0)
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	float3 Normal  : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Tangent : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Binormal : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 2)
	float3 DiffLight : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float4 Refract : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#ifdef CLOUDS
	float2 TexcoordClouds : TEXCOORDX;
			#include "../common/inc_texcoord.h"
#endif
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || FALLOFF_QUALITY == 1 || FOG_OF_WAR == 1)
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
#if (TEXCOORDS == 2)
	float2 Texcoord0  : TEXCOORD0;
	float2 Texcoord1  : TEXCOORD1;
	float4 Tangent    : TEXCOORD2;
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
	
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || FALLOFF_QUALITY == 1 || FOG_OF_WAR == 1)
	Out.Last = 0;
#endif
	
	float3 vInNormal = (In.Normal / 255.0f) * 2.0f - 1.0f;
	float4 vInTangent = (In.Tangent / 255.0f) * 2.0f - 1.0f;
	
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
	
	float4x3 mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	vPosition = mul(float4(In.Position, 1.0f), mBlend);
	vNormal = mul(float4(vInNormal, 0.0f), mBlend).xyz;
	vTangent = mul(float4(vInTangent.xyz, 0.0f), mBlend).xyz;
	
	vNormal = normalize(vNormal);
	vTangent = normalize(vTangent);
#else
	float3 vPosition = In.Position;
	float3 vNormal = vInNormal;
	float3 vTangent = vInTangent.xyz;
#endif

	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Position       = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0         = vColor;
	Out.Texcoord0.xy   = In.Texcoord0 + vTex0a;
	Out.Texcoord0.zw   = In.Texcoord0 + vTex0b;
	Out.Texcoord1.xy   = In.Texcoord1 + vTex1a;
	Out.Texcoord1.zw   = In.Texcoord1 + vTex1b;
	Out.PositionScreen = mul(Out.Position, mSceneProj);
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1 || FALLOFF_QUALITY == 0)
	Out.PositionView  = vPositionView;
#else
	Out.PositionScreen.z = vPositionView.z;
#endif
#if (FALLOFF_QUALITY == 1)
	Out.Last.z = length(vPositionView);
#endif
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	Out.Normal         = mul(vNormal, mWorldViewRotate);
	Out.Tangent        = mul(vTangent, mWorldViewRotate);
	Out.Binormal       = cross(Out.Tangent, Out.Normal) * vInTangent.w;
#elif (LIGHTING_QUALITY == 2)
	float3 vLight = vSunPositionView.xyz;		
	float3 vWVNormal = mul(vNormal, mWorldViewRotate);
	float3 vCamDirection = -normalize(vPositionView);

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight      = vSunColor * fDiffuse;
	Out.Refract.xyz    = vWVNormal;
	
	float fFresnelBias = 0.2f;
	float fFresnelScale = 0.8f;
	float fFresnelPower = 7.0f;
	Out.Refract.w = 1.0f - saturate(fFresnelBias + fFresnelScale * pow(1.0f + dot(-vCamDirection, normalize(vWVNormal)), fFresnelPower));
#endif

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
