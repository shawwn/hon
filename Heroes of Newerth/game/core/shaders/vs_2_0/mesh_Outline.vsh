// (C)2008 S2 Games
// mesh_color.vsh
// 
// Outline mesh vertex shader
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;          // World * View * Projection transformation
float4x4	mProj;        

float4		vColor;

#if (NUM_BONES > 0)
float4x3	vBones[NUM_BONES];
#endif

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color0 : COLOR0;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position   : POSITION0;
	float3 Normal     : NORMAL0;
#if (NUM_BONES > 0)
	int4 BoneIndex    : TEXCOORD_BONEINDEX;
	float4 BoneWeight : TEXCOORD_BONEWEIGHT;
#endif
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

#if (NUM_BONES > 0)
	float4 vPosition = 0.0f;
	// GPU Skinning Blend bone matrix transforms for this bone
	
	int4 vBlendIndex = In.BoneIndex;
	float4 vBoneWeight = In.BoneWeight / 255.0f;
	
	float4x3 mBlend = 0.0f;
	for (int i = 0; i < NUM_BONE_WEIGHTS; ++i)
		mBlend += vBones[vBlendIndex[i]] * vBoneWeight[i];

	vPosition = float4(mul(float4(In.Position, 1.0f), mBlend).xyz, 1.0f);
#else
	float4 vPosition = float4(In.Position, 1.0f);
#endif
	
	float4 vProj = normalize(mProj);
	Out.Position = mul(vPosition, mWorldViewProj);
	Out.Position += mul(float4(normalize(In.Normal)*1.3f, 0.0f), mWorldViewProj);
	Out.Position.xyz += normalize(mProj);
	Out.Color0 = vColor;

	return Out;
}
