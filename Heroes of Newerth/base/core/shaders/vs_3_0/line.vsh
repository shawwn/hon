// (C)2006 S2 Games
// line.vsh
// 
// Colored line shader
//=============================================================================


//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation

float4		vColor;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position      : POSITION;
	float4 Color0        : COLOR0;
};


//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float3 Position  : POSITION;
	float4 Color0    : COLOR0;
};


//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	Out.Position	= mul(float4(In.Position, 1.0f), mWorldViewProj);
	Out.Color0		= vColor * In.Color0;

	return Out;
}

