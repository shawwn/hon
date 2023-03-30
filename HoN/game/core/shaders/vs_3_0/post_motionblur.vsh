// (C)2008 S2 Games
// post_motionblur.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
static const int KERNEL_SIZE = 12;

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position  : POSITION;
	float2 Texcoord0 : TEXCOORD0;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float4 Position  : POSITION;
	float4 Color0    : COLOR0;
	float2 Texcoord0 : TEXCOORD0;
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	float4 v = float4(In.Position.x, In.Position.y, 0.0f, 1.0f);

	Out.Position	= mul(v, mWorldViewProj);
	Out.Texcoord0   = In.Texcoord0;

	return Out;
}

