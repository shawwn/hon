// (C)2009 S2 Games
// mesh_color.vsh
// 
// Default mesh vertex shader
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform mat4	mProj;

uniform vec4	vColor;

#if (NUM_BONES > 0)
	#ifdef NON_SQUARE_MATRIX
uniform mat3x4	vBones[NUM_BONES];
	#else
uniform vec4	vBones3[NUM_BONES * 3];
	#endif
#endif

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;

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
		vec4 vInVertex = vec4(normalize(gl_Normal)*1.5, 0.0);
		vInVertex += gl_Vertex;
		
#if (NUM_BONES > 0)
	vec4 vPosition;

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

	vPosition = vec4((vInVertex * mBlend).xyz, 1.0);
#else
	vec4 vPosition = vInVertex;
#endif

	vec4 vProj = vec4(mProj);
	gl_Position      = (gl_ModelViewProjectionMatrix * vPosition);
	gl_Position      -= normalize(vProj)*2.5;
	v_vColor         = vColor;
	
}

