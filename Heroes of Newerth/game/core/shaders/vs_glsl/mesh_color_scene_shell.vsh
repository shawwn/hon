// (C)2009 S2 Games
// mesh_color_scene_shell.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform mat3	mWorldViewRotate;        // World rotation for normals

uniform vec4	vColor;

uniform vec3	vSunPositionView;

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

#if (NUM_BONES > 0)
	#ifdef NON_SQUARE_MATRIX
uniform mat3x4	vBones[NUM_BONES];
	#else
uniform vec4	vBones3[NUM_BONES * 3];
	#endif
#endif

#ifdef CLOUDS
uniform mat4	mCloudProj;
#endif

#if (FOG_OF_WAR == 1)
uniform mat4	mFowProj;
#endif

uniform mat4	mSceneProj;

uniform float	fOffset;			// Vertex Offset

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;

#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
varying vec3	v_vNormal;
varying vec3	v_vTangent;
varying vec3	v_vBinormal;
#elif (LIGHTING_QUALITY == 2)
varying vec3	v_vDiffLight;
varying vec3	v_vRefract;
#endif

varying vec4	v_vPositionScreen;

#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1 || FALLOFF_QUALITY == 0)
varying vec3	v_vPositionView;
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
attribute vec4	a_vTangent;

#if (NUM_BONES > 0)
attribute vec4	a_vBoneIndex;
attribute vec4	a_vBoneWeight;
#endif

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1)) || FOG_OF_WAR == 1)
	v_vData0 = vec4(0.0, 0.0, 0.0, 0.0);
#endif

#if (NUM_BONES > 0)
	vec4 vPosition;
	vec3 vNormal;
	vec3 vTangent;

	//
	// GPU Skinning
	// Blend bone matrix transforms for this bone
	//
	
	vec4 vBlendIndex = a_vBoneIndex;
	vec4 vBoneWeight = a_vBoneWeight;
	
	#ifdef NON_SQUARE_MATRIX	
	mat3x4 mBlend = mat3x4(0.0);
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[int(vBlendIndex[i])] * vBoneWeight[i];
	#else
	mat4 mBlend = mat4(0.0);
	mBlend[3][3] = 1.0;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
	{
		mBlend[0] += vBones3[int(vBlendIndex[i]) * 3 + 0] * vBoneWeight[i];
		mBlend[1] += vBones3[int(vBlendIndex[i]) * 3 + 1] * vBoneWeight[i];
		mBlend[2] += vBones3[int(vBlendIndex[i]) * 3 + 2] * vBoneWeight[i];
	}
	#endif

	vPosition = vec4((gl_Vertex * mBlend).xyz, 1.0);
	vNormal = normalize((vec4(gl_Normal, 0.0) * mBlend).xyz);
	vTangent = normalize((vec4(a_vTangent.xyz, 0.0) * mBlend).xyz);
#else
	vec4 vPosition = gl_Vertex;
	vec3 vNormal = gl_Normal;
	vec3 vTangent = a_vTangent.xyz;
#endif

	vPosition.xyz += vNormal * fOffset;

	vec3 vPositionView = (gl_ModelViewMatrix * vPosition).xyz;

	gl_Position       = gl_ModelViewProjectionMatrix * vPosition;
	gl_TexCoord[0]    = gl_MultiTexCoord0;
	v_vColor          = vColor;
	v_vPositionScreen = mSceneProj * gl_Position;
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1)
	v_vNormal         = mWorldViewRotate * vNormal;
	v_vTangent        = mWorldViewRotate * vTangent;
	v_vBinormal       = cross(v_vTangent, v_vNormal) * a_vTangent.w;
#elif (LIGHTING_QUALITY == 2)
	vec3 vLight = vSunPositionView;		
	vec3 vWVNormal = mWorldViewRotate * vNormal;

	float fDiffuse = max(dot(vWVNormal, vLight), 0.0);

	v_vDiffLight      = vSunColor * fDiffuse;
	v_vRefract        = vWVNormal;
#endif
	
#if (LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1 || FALLOFF_QUALITY == 0)
	v_vPositionView  = vPositionView;
#endif
	
#if (SHADOWS == 1)
	v_vTexcoordLight = mLightWorldViewProjTex * vPosition;
#endif

#ifdef CLOUDS
	v_vClouds = (mCloudProj * vPosition).xy;
#endif

#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0 || SHADOWS == 1))
	v_vData0.z = length(vPositionView);
#endif

#if (FOG_QUALITY == 1 && FOG_TYPE != 0)
	#if (FOG_TYPE == 0) 
	// FOG_NONE
	v_vData0.w = 0.0;
	#elif (FOG_TYPE == 1) 
	// FOG_LINEAR
	v_vData0.w = clamp(length(vPositionView) * vFog.x + vFog.y, 0.0, 1.0) * vFog.z;
	#elif (FOG_TYPE == 2) 
	// FOG_EXP2
	v_vData0.w = 1.0 - exp2(-length(vPositionView) * fFogDensity);
	#elif (FOG_TYPE == 3) 
	// FOG_EXP
	v_vData0.w = 1.0 - exp(-length(vPositionView) * fFogDensity);
	#elif (FOG_TYPE == 4) 
	// FOG_HERMITE
	v_vData0.w = smoothstep(fFogStart, fFogEnd, length(vPositionView)) * fFogScale;
	#endif
#endif

#if (FOG_OF_WAR == 1)
	v_vData0.xy = (mFowProj * vPosition).xy;
#endif
}
