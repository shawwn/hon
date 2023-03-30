// (C)2008 S2 Games
// post_down4.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Constants
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation
float4		vTexelSize;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position  : POSITION;
	float2 Texcoord0 : TEXCOORD0;
	float2 Texcoord1 : TEXCOORD1;
	float2 Texcoord2 : TEXCOORD2;
	float2 Texcoord3 : TEXCOORD3;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float2 Position  : POSITION;
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
	
	// Pack Texcoords to save additions in pixel shader
	Out.Texcoord0 = In.Texcoord0 * 4.0f + vTexelSize.xy;
	Out.Texcoord1 = In.Texcoord0 * 4.0f + float2(vTexelSize.x, -vTexelSize.y);
	Out.Texcoord2 = In.Texcoord0 * 4.0f + float2(-vTexelSize.x, vTexelSize.y);
	Out.Texcoord3 = In.Texcoord0 * 4.0f - vTexelSize.xy;
	
	return Out;
}

