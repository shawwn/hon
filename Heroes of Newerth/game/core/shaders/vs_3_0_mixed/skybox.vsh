// (C)2006 S2 Games
// skybox.vsh
// 
// Sky-box vertex shader
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProjInv;  // Inverse World * View * Projection transformation
float4		vColor;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color0 : COLOR0;
	float3 Texcoord0 : TEXCOORD0;
};

//=============================================================================
// Vertex shader input structure (Mesh data)
//=============================================================================
struct VS_INPUT
{
	float4 Position  : POSITION;
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	Out.Position	= In.Position;
	Out.Color0		= vColor;
	Out.Texcoord0	= mul(In.Position, mWorldViewProjInv);

	return Out;
}
