// (C)2006 S2 Games
// terrain_shadowed.vsh
// 
// Default terrain vertex shader
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
float4x4	mWorldView;	             // World * View  (No Scale)
float4x4	mWorld;			         // World transformation

float4		vSunPositionView;

float3		vAmbient;
float3		vSunColor;
float3		vSunColorSpec;

float		fGlossiness;

float		fWorldTileSize;
float		fWorldTextureInc;
float		fWorldTexelInc;

#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		float4x4	mLightWorldViewProjTexSplit[4];  // Light's World * View * Projection * Tex
	#else
		float4x4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
	#endif
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
#ifdef TERRAIN_DEREPEAT
	float4 Texcoord0     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Texcoord1     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#else
	float2 Texcoord0     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float2 Texcoord1     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0)
	float3 Normal        : TEXCOORDX;
		#include "../common/inc_texcoord.h"
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
	float  Height    : POSITION;
	float4 Color0    : COLOR0;
	float4 Data0     : TEXCOORD0;
	float4 Data1     : TEXCOORD1;
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
	
	float2 vTile = float2(In.Data0.w, In.Data1.w);
	
	float3 vPosition = float3(vTile * fWorldTileSize, In.Height);
	float2 vTexcoord0 = vTile * fWorldTextureInc;
	float2 vTexcoord1 = vTile * fWorldTexelInc;
	
	float3 vData0 = In.Data0 / 255.0f;
	float3 vData1 = In.Data1 / 255.0f;
	
	float3 vNormal = float3(vData0.xy * 2.0f - 1.0f, vData0.z);
	float3 vTangent = vData1 * 2.0f - 1.0f;
	
	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Color0        = In.Color0;
	
#ifdef TERRAIN_DEREPEAT
	float fDistance = length(vPositionView);
	const float fShift = 8.5f;
	const float fScale = 0.75f;

	float fPureLevel = max((log(fDistance) / log(2.0f) - fShift) * fScale, 0.0f);
	float fLevel = max(floor((log(fDistance) / log(2.0f) - fShift) * fScale), 0.0f);
	
	float fLayer0Scale;
	float fLayer1Scale;
	float fLerp;
	
	if (frac(fLevel / 2.0f) > 0.0)
	{
		fLerp = frac(fPureLevel);
		
		fLayer0Scale = 1.0f / pow(2.0f, fLevel);
		fLayer1Scale = 1.0f / pow(2.0f, fLevel + 1.0f);
	}
	else
	{
		fLerp = 1.0f - frac(fPureLevel);
		
		fLayer0Scale = 1.0f / pow(2.0f, fLevel + 1.0f);		
		fLayer1Scale = 1.0f / pow(2.0f, fLevel);
	}
	
	Out.Texcoord0.xy = vTexcoord0 * fLayer0Scale;
	Out.Texcoord0.zw = vTexcoord0 * fLayer1Scale;
	
	Out.Texcoord1.xy = vTexcoord1;
	Out.Texcoord1.z = fLerp;
#else
	Out.Texcoord0 = vTexcoord0;
	Out.Texcoord1 = vTexcoord1;
#endif

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionView  = vPositionView;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = fDistance;
#endif
	
#if (LIGHTING_QUALITY == 0)
	Out.Normal        = mul(vNormal, mWorldView);
	Out.Tangent       = mul(vTangent, mWorldView);
	Out.Binormal      = cross(Out.Tangent, Out.Normal);
#elif (LIGHTING_QUALITY == 1)
	float3 vCamDirection = -normalize(vPositionView);
	float3 vLight = vSunPositionView.xyz;
	float3 vWVNormal = mul(vNormal, mWorldView);
	float3 vWVTangent = mul(vTangent, mWorldView);
	float3 vWVBinormal = cross(vWVTangent, vWVNormal);

	float3x3 mRotate = transpose(float3x3(vWVTangent, vWVBinormal, vWVNormal));
	
	Out.SunLight      = mul(vLight, mRotate);
	Out.HalfAngle     = mul(normalize(vLight + vCamDirection), mRotate);
#elif (LIGHTING_QUALITY == 2)
	float3 vCamDirection = -normalize(vPositionView);
	float3 vLight = vSunPositionView.xyz;		
	float3 vHalfAngle = normalize(vLight + vCamDirection);
	float3 vWVNormal = mul(vNormal, mWorldView);

	float fDiffuse = saturate(dot(vWVNormal, vLight));
	float fSpecular = saturate(step(0.0f, fDiffuse) * (pow(dot(vWVNormal, vHalfAngle), fGlossiness)));

	Out.DiffLight     = vSunColor * fDiffuse;
	Out.SpecLight     = vSunColorSpec * fSpecular;
#elif (LIGHTING_QUALITY == 3)
	float3 vLight = vSunPositionView.xyz;		
	float3 vWVNormal = mul(vNormal, mWorldView);

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
	Out.Last.w = Fog(vPositionView);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(float4(vPosition, 1.0f), mFowProj).xy;
#endif

	return Out;
}
