// (C)2008 S2 Games
// effect_refract_mask_anim.vsh
// 
// Particle refraction vertex shader
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation
float4x4	mSceneProj;
float2		vTexOffset;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position         : POSITION;
	float4 Color0           : COLOR0;
	float4 Texcoord0        : TEXCOORD0;
	float2 Texcoord1        : TEXCOORD1;
	float4 PositionScreen   : TEXCOORD2;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position  : POSITION;
	float4 Color0    : COLOR0;
	float4 Texcoord0 : TEXCOORD0;
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	Out.Position         = mul(float4(In.Position, 1.0f), mWorldViewProj);
	Out.Texcoord0        = In.Texcoord0;
	Out.Texcoord1        = In.Texcoord0.xy;
	Out.PositionScreen   = mul(Out.Position, mSceneProj);
	Out.Color0           = In.Color0;
	
	Out.Texcoord0.xy += vTexOffset;

	return Out;
}

