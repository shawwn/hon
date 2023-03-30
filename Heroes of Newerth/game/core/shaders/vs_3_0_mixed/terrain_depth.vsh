// (C)2007 S2 Games
// terrain_depth.vsh
// 
// Terrain depth pass
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;  // World * View * Projection transformation

float		fWorldTileSize;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
};

//=============================================================================
// Vertex shader input structure
//=============================================================================
struct VS_INPUT
{
	float  Height    : POSITION;
	float4 Data0     : TEXCOORD0;
	float4 Data1     : TEXCOORD1;
};

//=============================================================================
// Vertex Shader
//=============================================================================
VS_OUTPUT VS( VS_INPUT In )
{
	VS_OUTPUT Out;

	float2 vTile = float2(In.Data0.w, In.Data1.w);
	
	float3 vPosition = float3(vTile * fWorldTileSize, In.Height);

	Out.Position	= mul(float4(vPosition, 1.0f), mWorldViewProj);

	return Out;
}
