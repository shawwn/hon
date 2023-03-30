// (C)2007 S2 Games
// effect_sky.vsh
// 
// Default effect shader
//=============================================================================


//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation
float		fSkyEpsilon;


//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position  : POSITION;
	float4 Color0    : COLOR0;
	float2 Texcoord0 : TEXCOORD0;
};


//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position  : POSITION;
	float4 Color0    : COLOR0;
	float2 Texcoord0 : TEXCOORD0;
};


//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	Out.Position	= mul(float4(In.Position, 1.0f), mWorldViewProj);
	Out.Position.z  = Out.Position.w - fSkyEpsilon;
	Out.Texcoord0	= In.Texcoord0.xy;
	Out.Color0		= In.Color0;

	return Out;
}

