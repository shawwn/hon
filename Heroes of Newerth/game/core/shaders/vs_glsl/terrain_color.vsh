// (C)2008 S2 Games
// terrain_color.vsh
// 
// Terrain
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform mat4	mWorldViewOffset;        // World * View Offset

uniform vec3	vWorldSizes;

uniform vec3	vSunPositionWorld;

uniform vec3	vAmbient;
uniform vec3	vSunColor;
uniform vec3	vSunColorSpec;

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

//=============================================================================
// Varying variables
//=============================================================================
#if (LIGHTING_QUALITY == 0)
varying vec3	v_vNormal;
varying vec3	v_vTangent;
varying vec3	v_vBinormal;
#elif (LIGHTING_QUALITY == 1)
varying vec3	v_vHalfAngle;
varying vec3	v_vSunLight;
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
attribute float	a_fHeight;
attribute vec4	a_vData0;
attribute vec4	a_vData1;

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
	v_vData0 = vec4(0.0, 0.0, 0.0, 0.0);
#endif

	vec2 vTile = vec2(a_vData0.w, a_vData1.w);
	
	vec4 vPosition = vec4(vTile * vWorldSizes.x, a_fHeight, 1.0);
	vec2 vTexcoord0 = vTile * vWorldSizes.y;
	vec2 vTexcoord1 = vTile * vWorldSizes.z;
	
	vec3 vNormal = a_vData0.xyz / 255.0 * 2.0 - 1.0;
	vec3 vTangent = a_vData1.xyz / 255.0 * 2.0 - 1.0;

	vec3 vPositionOffset = (mWorldViewOffset * vPosition).xyz;
	
#ifdef TERRAIN_DEREPEAT
	float fDistance = length(vPositionOffset);
	const float fShift = 8.5;
	const float fScale = 0.75;

	float fPureLevel = max(log2(fDistance) * fScale - fShift * fScale, 0.0);
	float fLevel = floor(fPureLevel);
	
	float fLayer0Scale;
	float fLayer1Scale;
	float fLerp;
	
	if (fract(fLevel * 0.5) > 0.0)
	{
		fLerp = fract(fPureLevel);
		
		fLayer0Scale = 1.0 / pow(2.0, fLevel);
		fLayer1Scale = fLayer0Scale * 0.5;
	}
	else
	{
		fLerp = 1.0 - fract(fPureLevel);
		
		fLayer0Scale = 1.0 / pow(2.0, fLevel + 1.0);		
		fLayer1Scale = fLayer0Scale * 2.0;
	}
	
	gl_TexCoord[0].xy = vTexcoord0 * fLayer0Scale;
	gl_TexCoord[0].zw = vTexcoord0 * fLayer1Scale;
	
	gl_TexCoord[1].xy = vTexcoord1;
	gl_TexCoord[1].z  = fLerp;
	gl_TexCoord[1].w  = 0.0;
#else
	gl_TexCoord[0]   = vec4(vTexcoord0, 0.0, 0.0);
	
	#ifdef TERRAIN_ALPHAMAP
	gl_TexCoord[1]   = vec4(vTexcoord1, 0.0, 0.0);
	#endif
#endif

	gl_Position      = gl_ModelViewProjectionMatrix * vPosition;
	gl_FrontColor    = gl_Color;
	
#if (LIGHTING_QUALITY == 0)
	v_vNormal        = vNormal;
	v_vTangent       = vTangent;
	v_vBinormal      = cross(v_vTangent, v_vNormal);
#elif (LIGHTING_QUALITY == 1)
	vec3 vCamDirection = -normalize(vPositionOffset);
	vec3 vLight = vSunPositionWorld;
	vec3 vWVNormal = vNormal;
	vec3 vWVTangent = vTangent;
	vec3 vWVBinormal = cross(vWVTangent, vWVNormal);

	mat3 mRotate = mat3(vWVTangent, vWVBinormal, vWVNormal);
	
	v_vSunLight      = vLight * mRotate;
	v_vHalfAngle     = normalize(vLight + vCamDirection) * mRotate;
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
	v_vData0.z = length(vPositionOffset);
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	#if (FOG_TYPE == 0) 
	// FOG_NONE
	v_vData0.w = 0.0;
	#elif (FOG_TYPE == 1) 
	// FOG_LINEAR
	v_vData0.w = clamp(length(vPositionOffset.xyz) * vFog.x + vFog.y, 0.0, 1.0) * vFog.z;
	#elif (FOG_TYPE == 2) 
	// FOG_EXP2
	v_vData0.w = 1.0 - exp2(-length(vPositionOffset.xyz) * fFogDensity);
	#elif (FOG_TYPE == 3) 
	// FOG_EXP
	v_vData0.w = 1.0 - exp(-length(vPositionOffset.xyz) * fFogDensity);
	#elif (FOG_TYPE == 4) 
	// FOG_HERMITE
	v_vData0.w = smoothstep(fFogStart, fFogEnd, length(vPositionOffset.xyz)) * fFogScale;
	#endif
#endif

#if (FOG_OF_WAR == 1)
	v_vData0.xy = (mFowProj * vPosition).xy;
#endif
}
