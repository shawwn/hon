// (C)2006 S2 Games
// mesh2_shadow_opacity.vsh
// 
// Renders a alpha-tested mesh into a shadowmap
//=============================================================================


//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;		// World * View * Projection transformation
#if (NUM_BONES > 0)
	float4x3		vBones[NUM_BONES];
#endif


//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position   : POSITION;
	float2 Texcoord0  : TEXCOORD0;
#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	float2 Depth      : TEXCOORD1;
#endif
};


//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position   : POSITION;
	float2 Texcoord0  : TEXCOORD0;
#if (NUM_BONES > 0)
	int4 BoneIndex    : TEXCOORD4;
	float4 BoneWeight : TEXCOORD5;
#endif
};


//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;
	
#if (NUM_BONES > 0)
	float3 vPosition = 0.0f;

	//
	// GPU Skinning
	// Blend bone matrix transforms for this bone
	//
	
	int4 vBlendIndex = In.BoneIndex;
	float4 vBoneWeight = In.BoneWeight / 255.0f;
	
	float4x3	mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	float3x3	mAxis = float3x3(mBlend[0], mBlend[1], mBlend[2]);
	float3		vPos = mBlend[3];
	
	vPosition = mul(In.Position, mAxis) + vPos;
#else
	float3 vPosition = In.Position;
#endif
	
	Out.Position = mul(float4(vPosition, 1.0f), mWorldViewProj);
	Out.Texcoord0 = In.Texcoord0;
#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	Out.Depth = Out.Position.zw;
#endif

	return Out;
}

