// (C)2006 S2 Games
// terrain_color.vsh
// 
// Terrain
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
float4x4	mWorldViewOffset;	     // World * View  (No Scale)
float4x4	mWorld;			         // World transformation

float4		vSunPositionWorld;

float3		vAmbient;
float3		vSunColor;

float3		vWorldSizes;

#if (SHADOWS == 1)
float4x4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
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
	float4 Position : POSITION;
	float4 Color0 : COLOR0;
#ifdef TERRAIN_DEREPEAT
	float4 Texcoord0 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Texcoord1 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#else
	float2 Texcoord0 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	#ifdef TERRAIN_ALPHAMAP
	float2 Texcoord1 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	#endif
#endif
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	float3 PositionOffset : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0)
	float3 Normal : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Tangent : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 Binormal : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 1)
	float3 HalfAngle : TEXCOORDX;
		#include "../common/inc_texcoord.h"
	float3 SunLight : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 2)
	float3 DiffLight : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (SHADOWS == 1)
	float4 TexcoordLight : TEXCOORDX; // Texcoord in light texture space
		#include "../common/inc_texcoord.h"
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
	
	float4 vPosition = float4(vTile * vWorldSizes.x, In.Height, 1.0f);
	float2 vTexcoord0 = vTile * vWorldSizes.y;
	float2 vTexcoord1 = vTile * vWorldSizes.z;
	
	float3 vData0 = In.Data0.xyz / 255.0f;
	
	float3 vNormal = float3(vData0.xy * 2.0f - 1.0f, vData0.z);
	float3 vTangent = In.Data1.xyz / 255.0f * 2.0f - 1.0f;
	
	float3 vPositionOffset = mul(vPosition, mWorldViewOffset).xyz;
	
	Out.Position      = mul(vPosition, mWorldViewProj);
	Out.Color0        = In.Color0;
	
	float fDistance = length(vPositionOffset);
	
#ifdef TERRAIN_DEREPEAT
	const float fShift = 8.5f;
	const float fScale = 0.75f;

	float fPureLevel = max(log2(fDistance) * fScale - fShift * fScale, 0.0f);
	float fLevel = floor(fPureLevel);
	
	float fLayer0Scale;
	float fLayer1Scale;
	float fLerp;
	
	if (frac(fLevel / 2.0f) > 0.0)
	{
		fLerp = frac(fPureLevel);
		
		fLayer0Scale = 1.0f / pow(2.0f, fLevel);
		fLayer1Scale = fLayer0Scale * 0.5f;
	}
	else
	{
		fLerp = 1.0f - frac(fPureLevel);
		
		fLayer0Scale = 1.0f / pow(2.0f, fLevel + 1.0f);		
		fLayer1Scale = fLayer0Scale * 2.0f;
	}
	
	Out.Texcoord0.xy = vTexcoord0 * fLayer0Scale;
	Out.Texcoord0.zw = vTexcoord0 * fLayer1Scale;
	
	Out.Texcoord1.xy = vTexcoord1;
	Out.Texcoord1.z = fLerp;
#else
	Out.Texcoord0 = vTexcoord0;
	
	#ifdef TERRAIN_ALPHAMAP
	Out.Texcoord1 = vTexcoord1;
	#endif
#endif

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionOffset  = vPositionOffset;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = fDistance;
#endif
	
#if (LIGHTING_QUALITY == 0)
	Out.Normal        = vNormal;
	Out.Tangent       = vTangent;
	Out.Binormal      = cross(Out.Tangent, Out.Normal);
#elif (LIGHTING_QUALITY == 1)
	float3 vCamDirection = -normalize(vPositionOffset);
	float3 vLight = vSunPositionWorld.xyz;
	float3 vWVNormal = vNormal;
	float3 vWVTangent = vTangent;
	float3 vWVBinormal = cross(vWVTangent, vWVNormal);

	float3x3 mRotate = transpose(float3x3(vWVTangent, vWVBinormal, vWVNormal));
	
	Out.SunLight      = mul(vLight, mRotate);
	Out.HalfAngle     = mul(normalize(vLight + vCamDirection), mRotate);
#elif (LIGHTING_QUALITY == 2)
	float3 vLight = vSunPositionWorld.xyz;		
	float3 vWVNormal = vNormal;

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight     = vSunColor * fDiffuse;
#endif

#if (SHADOWS == 1)
	Out.TexcoordLight = mul(vPosition, mLightWorldViewProjTex);
#endif

#ifdef CLOUDS
	Out.TexcoordClouds = mul(vPosition, mCloudProj).xy;
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = Fog(vPositionOffset);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(vPosition, mFowProj).xy;
#endif

	return Out;
}
