// (C)2006 S2 Games
// foliage_color.vsh
// 
// Default foliage vertex shader with shadowmap support
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
float4x4	mWorldViewOffset;        // World * View Offset

float4		vSunPositionWorld;

float3		vAmbient;
float3		vSunColor;

#if (SHADOWS == 1)
	float4x4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
#endif

float3		vCameraForward;
float3		vCameraRight;
float3		vCameraUp;
float3		vCameraPositionWorld;
float3		vUp;
float		fTime;

float4		vFoliage;

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
	float2 Texcoord0 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	float3 PositionOffset : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	float3 Normal : TEXCOORDX;
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
	float3 Position : POSITION;
	float3 Normal : TEXCOORD0;
	float4 Data : TEXCOORD1;
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
	
	//
	// Position and animate vertex
	//
	
	float4 vPosition = float4(In.Position, 1.0f);
	float3 vNormal = (In.Normal / 255.0f) * 2.0f - 1.0f;
	
	float4 vData = In.Data / 255.0f;
	
	const float PI = 3.14159265358979323846f;
	
	float2 vTexcoord0 = vData.xy;
	float fWeight = vData.z;
	float fPhase = vData.w * PI;
		
	float fTopShift = sin((fTime + fPhase) * vFoliage.z) * vFoliage.w;
	vPosition.x += fWeight * fTopShift;

	Out.Position       = mul(vPosition, mWorldViewProj);
	
	float3 vCamOffset = vCameraPositionWorld - vPosition.xyz;

	Out.Color0         = float4(float3(1.0f, 1.0f, 1.0f), 1.0f - saturate((length(vCamOffset) * vFoliage.x + vFoliage.y)));

	float3 vPositionOffset = mul(vPosition, mWorldViewOffset).xyz;
	
	Out.Texcoord0      = vTexcoord0;
	
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionOffset = vPositionOffset;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = length(vPositionOffset);
#endif
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	Out.Normal         = vNormal;
#elif (LIGHTING_QUALITY == 2)
	float3 vLight = vSunPositionWorld.xyz;		
	float3 vWVNormal = vNormal;

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight      = vSunColor * fDiffuse;
#endif

#if (SHADOWS == 1)
	Out.TexcoordLight = mul(vPosition, mLightWorldViewProjTex);
#endif

#ifdef CLOUDS
	Out.TexcoordClouds = mul(vPosition, mCloudProj).xy;
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	Out.Last.w = Fog(vPositionView);
#endif

#if (FOG_OF_WAR == 1)
	Out.Last.xy = mul(vPosition, mFowProj).xy;
#endif

	return Out;
}
