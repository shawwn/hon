// (C)2006 S2 Games
// leaf_depth_opacity.vsh
// 
// Renders a alpha-tested leaf depth
//=============================================================================


//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;		// World * View * Projection transformation

float4		vLeafClusters[48];
//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position   : POSITION;
	float2 Texcoord0  : TEXCOORD0;
};


//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position  : POSITION;
	float2 Texcoord0 : TEXCOORD0;
	float2 Wind      : TEXCOORD1;
	float3 Leaf      : TEXCOORD2;
};


//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;
	
	float4 vPos = float4(In.Position, 1.0f) + (vLeafClusters[In.Leaf.x] * In.Leaf.y);
	Out.Position = mul(vPos, mWorldViewProj);
	Out.Texcoord0 = In.Texcoord0;

	return Out;
}

