// (C)2006 S2 Games
// leaf.vsh
// 
// Default leaf vertex shader without shadowmap support
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

float4		vColor;

float4		vCameraForward;

#ifdef CLOUDS
	float4x4	mCloudProj;
#endif

#if (FOG_OF_WAR == 1)
	float4x4	mFowProj;
#endif

float4		vLeafClusters[48];
//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position      : POSITION;
	float4 Color0        : COLOR0;
	float2 Texcoord0     : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	float3 PositionView : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#endif
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	float3 Normal        : TEXCOORDX;
		#include "../common/inc_texcoord.h"
#elif (LIGHTING_QUALITY == 2 || LIGHTING_QUALITY == 3)
	float3 DiffLight  : TEXCOORDX;
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
	float3 Position     : POSITION;
	float3 Normal       : NORMAL;
	float2 Texcoord0    : TEXCOORD0;
	float2 Wind         : TEXCOORD1;
	float3 Leaf         : TEXCOORD2;
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

	float4 vPosition = float4(In.Position, 1.0f) + (vLeafClusters[In.Leaf.x] * In.Leaf.y);
	
#ifdef SMOOTH_TREE_NORMALS
	float3 vNormal = lerp(In.Normal, -vCameraForward.xyz, 0.5);
#else
	float3 vNormal = In.Normal;
#endif
	float3 vPositionView = mul(vPosition, mWorldView).xyz;
	
	Out.Position      = mul(vPosition, mWorldViewProj);
	Out.Color0        = vColor;
	Out.Texcoord0     = In.Texcoord0;

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionView  = vPositionView;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = length(vPositionView);
#endif
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	Out.Normal        = mul(vNormal, mWorldViewRotate);
#elif (LIGHTING_QUALITY == 2 || LIGHTING_QUALITY == 3)
	float3 vLight = vSunPositionView.xyz;		
	float3 vWVNormal = mul(vNormal, mWorldViewRotate);
	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight     =  vSunColor * fDiffuse;
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
