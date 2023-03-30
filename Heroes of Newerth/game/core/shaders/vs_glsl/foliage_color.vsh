// (C)2008 S2 Games
// foliage_color.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform mat4	mWorldViewOffset;        // World * View Offset

uniform vec4	vColor;

uniform vec3	vSunPositionWorld;

uniform vec3	vAmbient;
uniform vec3	vSunColor;

uniform float	fFogStart;
uniform float	fFogEnd;
uniform float	fFogScale;
uniform float	fFogDensity;
uniform vec3	vFog;

#if (SHADOWS == 1)
uniform mat4	mLightWorldViewProjTex;  // Light's World * View * Projection * Tex
#endif

#ifdef CLOUDS
uniform mat4	mCloudProj;
#endif

#if (FOG_OF_WAR == 1)
uniform mat4	mFowProj;
#endif

uniform float	fTime;

uniform vec4	vFoliage;

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;

#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
varying vec3	v_vNormal;
#elif (LIGHTING_QUALITY == 2)
varying vec3	v_vDiffLight;
#endif

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
varying vec3	v_vPositionOffset;
#endif

#if (SHADOWS == 1)
varying vec4	v_vTexcoordLight;
#endif

#ifdef CLOUDS
varying vec2	v_vClouds;
#endif

#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
varying vec4	v_vData0;
#endif

//=============================================================================
// Vertex attributes
//=============================================================================
attribute vec4 	a_vNormal;
attribute vec4	a_vData;

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
	v_vData0 = vec4(0.0, 0.0, 0.0, 0.0);
#endif

	//
	// Position and animate vertex
	//
	
	vec4 vPosition = gl_Vertex;
	vec3 vNormal = a_vNormal.xyz / 255.0 * 2.0 - 1.0;
	
	vec4 vData = a_vData / 255.0;
	
	const float PI = 3.14159265358979323846;
	
	vec2 vTexcoord0 = vData.xy;
	float fWeight = vData.z;
	float fPhase = vData.w * PI;
		
	float fTopShift = sin((fTime + fPhase) * vFoliage.z) * vFoliage.w;
	vPosition.x += fWeight * fTopShift;

	vec3 vPositionOffset = (mWorldViewOffset * vPosition).xyz;
	float fCamDistance = length(vPositionOffset);

	gl_Position      = gl_ModelViewProjectionMatrix * vPosition;
	gl_TexCoord[0]   = vec4(vTexcoord0.xy, 0.0, 0.0);
	v_vColor         = vec4(1.0, 1.0, 1.0, 1.0 - clamp(fCamDistance * vFoliage.x + vFoliage.y, 0.0, 1.0));
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	v_vNormal        = vNormal;
#elif (LIGHTING_QUALITY == 2)
	vec3 vLight = vSunPositionWorld;		
	vec3 vWVNormal = vNormal;

	float fDiffuse = max(dot(vWVNormal, vLight), 0.0);

	v_vDiffLight     = vSunColor * fDiffuse;
#endif
	
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	v_vPositionOffset  = vPositionOffset;
#endif
	
#if (SHADOWS == 1)
	v_vTexcoordLight = mLightWorldViewProjTex * vPosition;
#endif

#ifdef CLOUDS
	v_vClouds = (mCloudProj * vPosition).xy;
#endif

#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	v_vData0.z = fCamDistance;
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	#if (FOG_TYPE == 0) 
	// FOG_NONE
	v_vData0.w = 0.0;
	#elif (FOG_TYPE == 1) 
	// FOG_LINEAR
	v_vData0.w = clamp(length(vPositionOffset) * vFog.x + vFog.y, 0.0, 1.0) * vFog.z;
	#elif (FOG_TYPE == 2) 
	// FOG_EXP2
	v_vData0.w = 1.0 - exp2(-length(vPositionOffset) * fFogDensity);
	#elif (FOG_TYPE == 3) 
	// FOG_EXP
	v_vData0.w = 1.0 - exp(-length(vPositionOffset) * fFogDensity);
	#elif (FOG_TYPE == 4) 
	// FOG_HERMITE
	v_vData0.w = smoothstep(fFogStart, fFogEnd, length(vPositionOffset)) * fFogScale;
	#endif
#endif

#if (FOG_OF_WAR == 1)
	v_vData0.xy = (mFowProj * vPosition).xy;
#endif
}
