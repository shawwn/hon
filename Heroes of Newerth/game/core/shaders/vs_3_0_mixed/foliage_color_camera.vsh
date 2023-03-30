// (C)2006 S2 Games
// foliage_color_camera.vsh
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
float4x4	mWorldView;	             // World * View
float3x3	mWorldViewRotate;        // World rotation for normals

float4		vSunPositionView;

float3		vAmbient;
float3		vSunColor;
float3		vSunColorSpec;

#if (SHADOWS == 1)
	#ifdef SHADOW_SPLIT_PROJECTION
		float4x4	mLightWorldViewProjTexSplit[4];  // Light's World * View * Projection * Tex
	#else
		float4x4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
	#endif
#endif


float3		vCameraForward;
float3		vCameraRight;
float3		vCameraUp;
float3		vCameraPositionWorld;
float3		vUp;
float		fTime;

float		fFoliageFalloffStart;
float		fFoliageFalloffEnd;
float		fFoliageFalloffLength;
float		fFoliageAnimateSpeed;
float		fFoliageAnimateStrength;

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
	float3 Weight    : TEXCOORD1;
	float  Phase     : TEXCOORD2;
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
	
	float3 vPosition = In.Position;

	// Orthogonalize vCameraRight against In.Normal	
	float3 vRight = normalize(vCameraRight - In.Normal * dot(In.Normal, vCameraRight));
	
	vPosition += vRight * In.Weight.x;

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

	vPosition += vCameraUp * In.Weight.y;
	
	float fTopShift = sin((fTime + In.Phase) * fFoliageAnimateSpeed) * fFoliageAnimateStrength;
	vPosition.x += In.Weight.z * fTopShift;
	
	Out.Position      = mul(float4(vPosition, 1.0f), mWorldViewProj);
	
	float3 vCamOffset = vCameraPositionWorld - vPosition;

	Out.Color0        = float4(In.Color0.rgb, In.Color0.a * (1.0f - saturate((length(vCamOffset) - fFoliageFalloffStart)/(fFoliageFalloffLength))));

	float3 vPositionView = mul(float4(vPosition, 1.0f), mWorldView).xyz;
	
	Out.Texcoord0     = In.Texcoord0;
	
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	Out.PositionView  = vPositionView;
#endif
#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	Out.Last.z = length(vPositionView);
#endif
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)

	Out.Normal        = mul(In.Normal, mWorldViewRotate);

#elif (LIGHTING_QUALITY == 2 || LIGHTING_QUALITY == 3)

	float3 vLight = vSunPositionView.xyz;		
	float3 vWVNormal = mul(In.Normal, mWorldViewRotate);

	float fDiffuse = saturate(dot(vWVNormal, vLight));

	Out.DiffLight     =  vSunColor * fDiffuse;

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



