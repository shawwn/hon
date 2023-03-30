// (C)2008 S2 Games
// mesh_shadow_cliff.vsh
// 
// Renders a mesh into a shadowmap
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;		// World * View * Projection transformation

#if (NUM_BONES > 0)
float4x3	vBones[NUM_BONES];
#endif

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	float2 Depth : TEXCOORD0;
#endif
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
#if (NUM_BONES > 0)
	int4 BoneIndex : TEXCOORD_BONEINDEX;
	float4 BoneWeight : TEXCOORD_BONEWEIGHT;
#endif
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;
	
	float3 vInNormal = (In.Normal / 255.0f) * 2.0f - 1.0f;
	
#if (NUM_BONES > 0)
	float3 vPosition = 0.0f;
	float3 vNormal = 0.0f;

	//
	// GPU Skinning
	// Blend bone matrix transforms for this bone
	//
	
	int4 vBlendIndex = In.BoneIndex;
	float4 vBoneWeight = In.BoneWeight / 255.0f;
	
	float4x3 mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	vPosition = mul(float4(In.Position, 1.0f), mBlend);
	vNormal = mul(float4(vInNormal, 0.0f), mBlend).xyz;
	
	vNormal = normalize(vNormal);
#else
	float3 vPosition = In.Position;
	float3 vNormal = vInNormal;
#endif

	vPosition -= vNormal * 4.0f;

	Out.Position = mul(float4(vPosition, 1.0f), mWorldViewProj);

#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	Out.Depth = Out.Position.zw;
#endif

	return Out;
}
