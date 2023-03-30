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
	float2 Texcoord0 : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#ifdef TERRAIN_ALPHAMAP
	float2 Texcoord1 : TEXCOORDX;
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
	
	Out.Texcoord0 = vTexcoord0;
	
#ifdef TERRAIN_ALPHAMAP
	Out.Texcoord1 = vTexcoord1;
#endif

	float3 vLight = vSunPositionWorld.xyz;		
	float3 vWVNormal = vNormal;

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	float3 vDiffLight = vSunColor * fDiffuse;
	float3 vAmbientLight = vAmbient;
	
	Out.Color0.rgb *= vDiffLight + vAmbientLight;	
	
	return Out;
}
