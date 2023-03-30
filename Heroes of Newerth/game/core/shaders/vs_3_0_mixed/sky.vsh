// (C)2006 S2 Games
// sky.vsh
// 
// Sky vertex shader
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation
float4		vColor;
float		fSkyEpsilon;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color0 : COLOR0;
	float2 Texcoord0 : TEXCOORD0;
};

//=============================================================================
// Vertex shader input structure (Mesh data)
//=============================================================================
struct VS_INPUT
{
	float3 Position  : POSITION;
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
	Out.Color0		= vColor;
	Out.Texcoord0	= In.Texcoord0;

	return Out;
}
