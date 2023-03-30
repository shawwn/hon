// (C)2008 S2 Games
// simple_depth.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
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

//=============================================================================
// Vertex attributes
//=============================================================================
attribute vec4	a_vBoneIndex;
attribute vec4	a_vBoneWeight;

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
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

	vPosition = vec4((gl_Vertex * mBlend).xyz, 1.0);
#else
	vec4 vPosition = gl_Vertex;
#endif

	gl_Position    = gl_ModelViewProjectionMatrix * vPosition;
}
