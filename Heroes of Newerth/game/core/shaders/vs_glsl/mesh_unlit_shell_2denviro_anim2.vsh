// (C)2008 S2 Games
// mesh_unlit_shell_2denviro_anim2.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform vec4	vColor;

uniform float	fFogStart;
uniform float	fFogEnd;
uniform float	fFogScale;
uniform float	fFogDensity;
uniform vec3	vFog;

#if (NUM_BONES > 0)
	#ifdef NON_SQUARE_MATRIX
uniform mat3x4	vBones[NUM_BONES];
	#else
uniform vec4	vBones3[NUM_BONES * 3];
	#endif
#endif

#if (FOG_OF_WAR == 1)
uniform mat4	mFowProj;
#endif

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;

#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
varying vec3	v_vPositionView;
#endif

#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0)) || FOG_OF_WAR == 1)
varying vec4	v_vData0;
#endif

uniform vec3	vCameraPosition;

uniform float	fTexRotate;

//=============================================================================
// Vertex attributes
//=============================================================================
#if (NUM_BONES > 0)
attribute vec4	a_vBoneIndex;
attribute vec4	a_vBoneWeight;
#endif

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
#if ((FOG_QUALITY == 1 && FOG_TYPE != 0) || (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0)) || FOG_OF_WAR == 1)
	v_vData0 = vec4(0.0, 0.0, 0.0, 0.0);
#endif

#if (NUM_BONES > 0)
	vec4 vPosition;
	vec3 vNormal;

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
#else
	vec4 vPosition = gl_Vertex;
	vec3 vNormal = gl_Normal;
#endif

	vPosition.xyz += vNormal * vColor.a;

	vec3 vPositionView = (gl_ModelViewMatrix * vPosition).xyz;

	gl_Position      = gl_ModelViewProjectionMatrix * vPosition;
	
	// 2d Enviro reflection
	vec3 vCameraDirection = normalize(vPosition.xyz - vCameraPosition);
	vec3 vReflection = vNormal * 2.0 * dot(vNormal, vCameraDirection) - vCameraDirection;
	
	vec2 vTexcoord = -vec3(vReflection * 0.5 + 0.5).yz;
	
	// Texcoord animation
	float fS = sin(fTexRotate);
	float fC = cos(fTexRotate);
	
	mat2 mTexRotate;
	mTexRotate[0][0] = fC;
	mTexRotate[1][0] = -fS;
	mTexRotate[0][1] = fS;
	mTexRotate[1][1] = fC;
	vTexcoord = vTexcoord * mTexRotate;
	
	vec2 vTexTranslate;
	vTexTranslate[0] = 0.5 + 0.5 * fC + 0.5 * fS;
	vTexTranslate[1] = 0.5 - 0.5 * fS + 0.5 * fC;
	vTexcoord += vTexTranslate;
	
	gl_TexCoord[0]   = vec4(vTexcoord, 0.0, 0.0);
	v_vColor         = vec4(vColor.rgb, 1.0);
	
#if (LIGHTING_QUALITY == 0 || FALLOFF_QUALITY == 0)
	v_vPositionView  = vPositionView;
#endif

#if (FALLOFF_QUALITY == 1 && (FOG_TYPE != 0))
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
