// (C)2008 S2 Games
// post_blur_h.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
static const int KERNEL_SIZE = 13;

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
	float4 Texcoord0 : TEXCOORD0;
	float4 Texcoord1 : TEXCOORD1;
	float4 Texcoord2 : TEXCOORD2;
	float4 Texcoord3 : TEXCOORD3;
	float4 Texcoord4 : TEXCOORD4;
	float4 Texcoord5 : TEXCOORD5;
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
	Out.Texcoord0.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * -10.5f);
	Out.Texcoord0.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * -8.5f);
	Out.Texcoord1.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * -6.5f);
	Out.Texcoord1.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * -4.5f);
	Out.Texcoord2.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * -2.5f);
	Out.Texcoord2.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * -0.5f);
	
	Out.Texcoord3.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * 0.5f);
	Out.Texcoord3.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * 2.5f);
	Out.Texcoord4.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * 4.5f);
	Out.Texcoord4.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * 6.5f);
	Out.Texcoord5.xy = In.Texcoord0 + float2(0.0f, vTexelSize.y * 8.5f);
	Out.Texcoord5.zw = In.Texcoord0 + float2(0.0f, vTexelSize.y * 10.5f);

	return Out;
}

