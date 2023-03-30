// (C)2007 S2 Games
// terrain_shadow.vsh
// 
// Renders terrain into a shadowmap
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float4x4	mWorldViewProj;		// World * View * Projection transformation

float		fWorldTileSize;

//=============================================================================
// Vertex shader output structure
//=============================================================================
struct VS_OUTPUT
{
	float4 Position : POSITION;
#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	float2 Depth : TEXCOORD0;
#endif
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
		
	float3 vData0 = In.Data0 / 255.0f;
	float3 vNormal = float3(vData0.xy * 2.0f - 1.0f, vData0.z);
	
	float3 vPosition = float3(vTile * fWorldTileSize, In.Height) - vNormal * 4.0f;

	Out.Position = mul(float4(vPosition, 1.0f), mWorldViewProj);

#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	Out.Depth = Out.Position.zw;
#endif

	return Out;
}
