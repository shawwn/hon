// (C)2006 S2 Games
// shadowed.h
// 
// Shadowmap support functions
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float	fShadowmapSize;
float	fShadowmapSizeInv;

float2	vShadowFalloff;
float2	vShadowLeak;

sampler	shadowmap;

//=============================================================================
// ShadowedR32F
//=============================================================================
float	ShadowedR32F(float4 TexcoordLight)
{
#if (SHADOWMAP_FILTER_WIDTH > 0)
	float fShadow = 0.0f;

	for (float fY = -(SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fY <= (SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fY += 1.0f)
	{
		for (float fX = -(SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fX <= (SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fX += 1.0f)
		{
			// Project shadow texture coord
			float3 vShadowTexcoord = (TexcoordLight.xyz / TexcoordLight.w);
			
			vShadowTexcoord.xy += float2(fX, fY) * fShadowmapSizeInv - fShadowmapSizeInv * 0.5f;
			
			// Determine the lerp amounts
			float2 vFrac = frac(fShadowmapSize * vShadowTexcoord.xy);

			// Read shadow samples
			float4 vSamples;
			vSamples[0] = tex2D(shadowmap, vShadowTexcoord.xy).r;
			vSamples[1] = tex2D(shadowmap, vShadowTexcoord.xy + float2(fShadowmapSizeInv, 0.0f)).r;
			vSamples[2] = tex2D(shadowmap, vShadowTexcoord.xy + float2(0.0f, fShadowmapSizeInv)).r;
			vSamples[3] = tex2D(shadowmap, vShadowTexcoord.xy + float2(fShadowmapSizeInv , fShadowmapSizeInv)).r;
			
			float4 vShadow = step(vShadowTexcoord.z, vSamples);
			
			// Percentage-closer filtering
			fShadow += lerp(lerp(vShadow[0], vShadow[1], vFrac.x), lerp(vShadow[2], vShadow[3], vFrac.x), vFrac.y);
		}
	}

	return saturate(fShadow / (SHADOWMAP_FILTER_WIDTH * SHADOWMAP_FILTER_WIDTH) * vShadowLeak.x + vShadowLeak.y);

#else // SHADOWMAP_FILTER_WIDTH == 0
	float3 vShadowTexcoord = TexcoordLight.xyz / TexcoordLight.w;
	return saturate((tex2D(shadowmap, vShadowTexcoord.xy).r > vShadowTexcoord.z) * vShadowLeak.x + vShadowLeak.y); 
#endif
}


//=============================================================================
// ShadowedDepth
//=============================================================================
float	ShadowedDepth(float4 TexcoordLight)
{
#if (SHADOWMAP_FILTER_WIDTH > 1)

	float fShadow = 0.0f;

	for (float fY = -(SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fY <= (SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fY += 1.0f)
	{
		for (float fX = -(SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fX <= (SHADOWMAP_FILTER_WIDTH - 1.0f)/2.0f; fX += 1.0f)
		{
			fShadow += tex2Dproj(shadowmap,
					float4(TexcoordLight.xy + float2(fX, fY) * fShadowmapSizeInv * TexcoordLight.w,
							TexcoordLight.z,
							TexcoordLight.w)).r;
		}
	}

	return saturate(fShadow / (SHADOWMAP_FILTER_WIDTH * SHADOWMAP_FILTER_WIDTH) * vShadowLeak.x + vShadowLeak.y);

#else // SHADOWMAP_FILTER_WIDTH == 0

	return saturate(tex2Dproj(shadowmap, TexcoordLight).r * vShadowLeak.x + vShadowLeak.y);

#endif
}


//=============================================================================
// Shadowed
//=============================================================================
float	Shadowed(float4 TexcoordLight)
{
#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	return ShadowedR32F(TexcoordLight);
#elif (SHADOWMAP_TYPE == 1) // SHADOWMAP_DEPTH
	return ShadowedDepth(TexcoordLight);
#endif
}


//=============================================================================
// ShadowedD
//=============================================================================
float	ShadowedD(float4 TexcoordLight, float fDistance)
{
#ifdef SHADOW_FALLOFF
	float fFade = saturate(fDistance * vShadowFalloff.x + vShadowFalloff.y);
#else
	float fFade = 0.0f;
#endif

#if (SHADOWMAP_TYPE == 0) // SHADOWMAP_R32F
	return saturate(ShadowedR32F(TexcoordLight) + fFade);
#elif (SHADOWMAP_TYPE == 1) // SHADOWMAP_DEPTH
	return saturate(ShadowedDepth(TexcoordLight) + fFade);
#endif
}