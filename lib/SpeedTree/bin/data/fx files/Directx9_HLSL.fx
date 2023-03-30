///////////////////////////////////////////////////////////////////////  
//	SpeedTree Effects
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
// Shader Variables

// world * view * projection matrix
float4x4	g_mWorldViewProjection;		

// textures
texture		g_texCompositeShadowMap;
texture		g_texCompositeLeafMap;
texture		g_texBark;
texture		g_texBarkNormalMap;

// fog start, fog end
float2		g_vFogParams;					

// lighting
float4		g_vLightPosition;
float4		g_vLightDiffuse;
float4		g_vLightAmbient;
float		g_fGlobalSaturation = 1.6f;
float		g_fGlobalAmbient = 0.25f;


// leaf table
float4		g_avLeafClusters[48];

// speedwind
float4x4	g_amWindMatrices[4];
float4x4	g_amLeafAngleMatrices[4];

// other
float4      g_vTreePosition;
float4		g_vMaterialDiffuse;
float4		g_vMaterialAmbient;
float		g_fLeafLightingAdjustment;
float4		g_vCameraDir;


///////////////////////////////////////////////////////////////////////  
// Samplers

sampler CompositeShadowSampler = sampler_state
{
    Texture = <g_texCompositeShadowMap>;
    MipFilter = linear;
    MinFilter = linear;
    MagFilter = linear;
};

sampler CompositeLeafSampler = sampler_state
{
    Texture = <g_texCompositeLeafMap>;
    MipFilter = linear;
    MinFilter = linear;
    MagFilter = linear;
    AddressU = wrap;
    AddressV = wrap;
};

sampler BarkSampler = sampler_state
{
    Texture = <g_texBark>;
    MipFilter = linear;
    MinFilter = linear;
    MagFilter = linear;
    AddressU = wrap;
    AddressV = wrap;
};

sampler BarkNormalMapSampler = sampler_state
{
    Texture = <g_texBarkNormalMap>;
    MipFilter = linear;
    MinFilter = linear;
    MagFilter = linear;
    AddressU = wrap;
    AddressV = wrap;
};


///////////////////////////////////////////////////////////////////////  
// Shader Output Structures

struct BranchVSOutput
{
    float4	vPosition				: POSITION;
    float4	vColor					: COLOR0;
    float2	vBaseTexCoords			: TEXCOORD0;
    float2	vSelfShadowTexCoords	: TEXCOORD1;
    float	fFog					: FOG;
};

struct FrondVSOutput
{
    float4	vPosition				: POSITION;
    float4	vColor					: COLOR0;
    float2	vBaseTexCoords			: TEXCOORD0;
    float2	vSelfShadowTexCoords	: TEXCOORD1;
    float	fFog					: FOG;
};

struct LeafVSOutput
{
    float4	vPosition		: POSITION;
    float4	vColor			: COLOR0;
    float2	vBaseTexCoords	: TEXCOORD0;
    float	fFog			: FOG;
};


///////////////////////////////////////////////////////////////////////  
// Branch Vertex Shader

BranchVSOutput BranchVS(float4 vPosition			: POSITION, 
						float3 vNormal				: NORMAL,
						float4 vBaseTexCoords		: TEXCOORD0,
						float2 vWindAttribs			: TEXCOORD1,
						float3 vBinormal			: TEXCOORD2,
						float3 vTangent				: TEXCOORD3)
{
	BranchVSOutput Output;
	
	float4 vWindyPoint = lerp(vPosition,										// interpolate between original position
							mul(vPosition, g_amWindMatrices[vWindAttribs.x]),	//	and full wind-blown position
							vWindAttribs.y);									//	using the wind weight
	vWindyPoint.rgb += g_vTreePosition.rgb;										// move to tree position
	Output.vPosition = mul(vWindyPoint, g_mWorldViewProjection);				// project to screen
	
	Output.vBaseTexCoords = vBaseTexCoords.xy;									// pass texcoords through
	Output.vSelfShadowTexCoords = vBaseTexCoords.zw;							// pass texcoords through
	
	Output.vColor.r = 0.5f + 0.5f * dot(g_vLightPosition.xyz, vTangent);		// compute tangent bumpmap effect
	Output.vColor.g = 0.5f + 0.5f * dot(g_vLightPosition.xyz, vBinormal);		// compute binormal bumpmap effect
	Output.vColor.b = 0.5f + 0.5f * dot(g_vLightPosition.xyz, vNormal);			// compute normal bumpmap effect
	Output.vColor.a = 1.0f;														// don't use alpha
	
	Output.fFog = (g_vFogParams[1] - Output.vPosition.z) / 
					(g_vFogParams[1] - g_vFogParams[0]);						// compute linear fog
	
	return Output;
}


///////////////////////////////////////////////////////////////////////  
// Frond Vertex Shader

FrondVSOutput FrondVS(float4 vPosition				: POSITION, 
						float3 vNormal				: NORMAL,
						float4 vBaseTexCoords		: TEXCOORD0,
						float2 vWindAttribs			: TEXCOORD1)
{
	FrondVSOutput Output;
	
	float4 vWindyPoint = lerp(vPosition,										// interpolate between original position
							mul(vPosition, g_amWindMatrices[vWindAttribs.x]),	//	and full wind-blown position
							vWindAttribs.y);									//	using the wind weight
	vWindyPoint.rgb += g_vTreePosition.rgb;										// move to tree position
	Output.vPosition = mul(vWindyPoint, g_mWorldViewProjection);				// project to screen
	
	Output.vBaseTexCoords = vBaseTexCoords.xy;									// pass texcoords through
	Output.vSelfShadowTexCoords = vBaseTexCoords.zw;							// pass texcoords through
	
	float3 vMovedNormal = (vNormal - g_vCameraDir) * 0.5f;						// average normal with inverse camera direction to 
																				//	smooth the lighting on the fronds for the side of 
																				//	the tree at which you are looking (not necessary, but
																				//	a nice effect)
	
	Output.vColor = dot(vMovedNormal, g_vLightPosition) *						// diffuse lighting
					g_vLightDiffuse * g_vMaterialDiffuse +						// diffuse color
					g_vLightAmbient * g_vMaterialAmbient;						// ambient color
	Output.vColor.a = 1.0f;														// we only want the texture alpha
	
	Output.fFog = (g_vFogParams[1] - Output.vPosition.z) / 
					(g_vFogParams[1] - g_vFogParams[0]);						// compute linear fog
	
	return Output;
}


///////////////////////////////////////////////////////////////////////  
// Leaf Vertex Shader

LeafVSOutput LeafVS(float4 vPosition				: POSITION, 
						float3 vNormal				: NORMAL,
						float2 vBaseTexCoords		: TEXCOORD0,
						float2 vWindAttribs			: TEXCOORD1,
						float3 vLeafAttribs			: TEXCOORD2)
{
	LeafVSOutput Output;
	
	float4 vWindyPoint = lerp(vPosition,										// interpolate between original position
							mul(vPosition, g_amWindMatrices[vWindAttribs.x]),	//	and full wind-blown position
							vWindAttribs.y);									//	using the wind weight
	vWindyPoint.rgb += g_vTreePosition.rgb;										// move to tree position
	vWindyPoint.xyz += mul(g_avLeafClusters[vLeafAttribs.x] * vLeafAttribs.y, 
						g_amLeafAngleMatrices[vLeafAttribs.z]);					// leaf placement using SpeedWind leaf rocking
	Output.vPosition = mul(vWindyPoint, g_mWorldViewProjection);				// project to screen
	
	Output.vBaseTexCoords = vBaseTexCoords;										// pass texcoords through
	
	float3 vMovedNormal = (vNormal - g_vCameraDir) * 0.5f;						// average normal with inverse camera direction to 
																				//	smooth the lighting on the leaves for the side of 
																				//	the tree at which you are looking (not necessary, but
																				//	a nice effect)
																				
	Output.vColor = dot(vMovedNormal, g_vLightPosition) *						// diffuse lighting
					g_vLightDiffuse * g_vMaterialDiffuse +						// diffuse color
					g_vLightAmbient * g_vMaterialAmbient;						// ambient color
	Output.vColor.a = 1.0f;														// we only want the texture alpha
	
	Output.fFog = (g_vFogParams[1] - Output.vPosition.z) / 
					(g_vFogParams[1] - g_vFogParams[0]);						// compute linear fog
	
	return Output;
}


///////////////////////////////////////////////////////////////////////  
// Branch Pixel Shader

float4 BranchPS(BranchVSOutput In): COLOR
{
	float4 texBranch = tex2D(BarkSampler, In.vBaseTexCoords);
	float4 texShadow = tex2D(CompositeShadowSampler, In.vSelfShadowTexCoords);
	float4 texNormal = tex2D(BarkNormalMapSampler, In.vBaseTexCoords);

	// compute bump mapping lighting
	float3 vHalves = { 0.5f, 0.5f, 0.5f };
	float fDot3Lighting = dot(texNormal - vHalves, In.vColor - vHalves) * 4.0f;
	
	float4 vOutputColor = texBranch * texShadow * g_vMaterialDiffuse;		// compute the unlit color
    vOutputColor.rgb *= fDot3Lighting * g_fGlobalSaturation;				// dot3 lighting & saturation
	vOutputColor.rgb += texBranch * g_vMaterialAmbient * g_fGlobalAmbient;	// ambient (add the texture back in so it doesn't get washed out)
 
    return vOutputColor;
}


///////////////////////////////////////////////////////////////////////  
// Frond Pixel Shader

float4 FrondPS(FrondVSOutput In) : COLOR
{
    float4 texFrond = tex2D(CompositeLeafSampler, In.vBaseTexCoords);
    float4 texShadow = tex2D(CompositeShadowSampler, In.vSelfShadowTexCoords);
    
    float4 vOutputColor = texFrond * texShadow * In.vColor;		// compute the color due to lighting
    vOutputColor.rgb *= g_fGlobalSaturation;					// saturation
	vOutputColor.rgb += texFrond * g_fGlobalAmbient;			// ambient (add the texture back in so it doesn't get washed out)
	
	return vOutputColor;
}


///////////////////////////////////////////////////////////////////////  
// Leaf Pixel Shader

float4 LeafPS(LeafVSOutput In) : COLOR
{
    float4 texLeaf = tex2D(CompositeLeafSampler, In.vBaseTexCoords);
    
    float4 vOutputColor = texLeaf * In.vColor;					// compute the color due to lighting
    vOutputColor.rgb *= g_fGlobalSaturation;					// saturation
	vOutputColor.rgb += texLeaf * g_fGlobalAmbient;				// ambient (add the texture back in so it doesn't get washed out)										
    
	return vOutputColor;
}


///////////////////////////////////////////////////////////////////////  
// Techniques

technique TBranch
{
	pass P0
	{
		VertexShader = compile vs_1_1 BranchVS( );
		
		// this could be compiled to a lower ps version, but 1.1 doesn't have
		//	enough instructions for the bump mapping and lighting effects and
		//	1.4 has some quirks. If you don't need bumpmapping, you could use 
		//	1.1 like on the other pixel shaders
		PixelShader = compile ps_2_0 BranchPS( );	
	}
}

technique TFrond
{
	pass P0
	{
		VertexShader = compile vs_1_1 FrondVS( );
		PixelShader = compile ps_1_1 FrondPS( );
	}
}

technique TLeaf
{
	pass P0
	{
		VertexShader = compile vs_1_1 LeafVS( );
		PixelShader = compile ps_1_1 LeafPS( );
	}
}
