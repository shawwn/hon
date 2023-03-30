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
float4x4	mWorldView;	             // World * View
float4x4	mWorld;			         // World transformation
float3x3	mWorldViewRotate;        // World rotation for normals

float4		vSunPositionView;

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
	float3 Position  : POSITION;
	float3 Normal    : NORMAL;
	float4 Color0    : COLOR0;
	float2 Texcoord0 : TEXCOORD0;
	float2 Texcoord1 : TEXCOORD1;
	float3 Tangent   : TEXCOORD2;
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
	
	float3 vPositionView = mul(float4(In.Position, 1.0f), mWorldView).xyz;
	
	Out.Position      = mul(float4(In.Position, 1.0f), mWorldViewProj);
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
	
	Out.Texcoord0.xy = In.Texcoord0 * fLayer0Scale;
	Out.Texcoord0.zw = In.Texcoord0 * fLayer1Scale;
	
	Out.Texcoord1.xy = In.Texcoord1;
	Out.Texcoord1.z = fLerp;
#else
	Out.Texcoord0 = In.Texcoord0;
	Out.Texcoord1 = In.Texcoord1;
#endif

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionView  = vPositionView;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = fDistance;
#endif
	
#if (LIGHTING_QUALITY == 0)
	Out.Normal        = mul(In.Normal, mWorldViewRotate);
	Out.Tangent       = mul(In.Tangent, mWorldViewRotate);
	Out.Binormal      = cross(Out.Tangent, Out.Normal);
#elif (LIGHTING_QUALITY == 1)
	float3 vCamDirection = -normalize(vPositionView);
	float3 vLight = vSunPositionView.xyz;
	float3 vWVNormal = mul(In.Normal, mWorldViewRotate);
	float3 vWVTangent = mul(In.Tangent, mWorldViewRotate);
	float3 vWVBinormal = cross(vWVTangent, vWVNormal);

	float3x3 mRotate = transpose(float3x3(vWVTangent, vWVBinormal, vWVNormal));
	
	Out.SunLight      = mul(vLight, mRotate);
	Out.HalfAngle     = mul(normalize(vLight + vCamDirection), mRotate);
#elif (LIGHTING_QUALITY == 2)
	float3 vCamDirection = -normalize(vPositionView);
	float3 vLight = vSunPositionView.xyz;		
	float3 vHalfAngle = normalize(vLight + vCamDirection);
	float3 vWVNormal = mul(In.Normal, mWorldViewRotate);

	float fDiffuse = saturate(dot(vWVNormal, vLight));
	float fSpecular = saturate(step(0.0f, fDiffuse) * (pow(dot(vWVNormal, vHalfAngle), fGlossiness)));

	Out.DiffLight     = vSunColor * fDiffuse;
	Out.SpecLight     = vSunColorSpec * fSpecular;
#elif (LIGHTING_QUALITY == 3)
	float3 vLight = vSunPositionView.xyz;		
	float3 vWVNormal = mul(In.Normal, mWorldViewRotate);

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight     =  vSunColor * fDiffuse;
#endif

#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		Out.TexcoordLight0 = mul(float4(In.Position, 1.0f), mLightWorldViewProjTexSplit[0]);
		Out.TexcoordLight1 = mul(float4(In.Position, 1.0f), mLightWorldViewProjTexSplit[1]);
		Out.TexcoordLight2 = mul(float4(In.Position, 1.0f), mLightWorldViewProjTexSplit[2]);
		Out.TexcoordLight3 = mul(float4(In.Position, 1.0f), mLightWorldViewProjTexSplit[3]);
	#else
		Out.TexcoordLight = mul(float4(In.Position, 1.0f), mLightWorldViewProjTex);
	#endif
#endif

#ifdef CLOUDS
	Out.TexcoordClouds = mul(float4(In.Position, 1.0f), mCloudProj);
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = Fog(vPositionView);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(float4(In.Position, 1.0f), mFowProj).xy;
#endif

	return Out;
}
