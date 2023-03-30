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
// GetSplitLightTexcoord
//=============================================================================
float4	GetSplitLightTexcoord(float4 TexcoordLight0, float4 TexcoordLight1, float4 TexcoordLight2, float4 TexcoordLight3)
{
	float2 vShadowTex0 = (TexcoordLight0.xy / TexcoordLight0.w);

	float4 vTexcoordLight;

	if (vShadowTex0.x > 0.5f && vShadowTex0.y < 0.5f)
		vTexcoordLight = TexcoordLight0;

	float2 vShadowTex1 = (TexcoordLight1.xy / TexcoordLight1.w);

	if (vShadowTex1.x < 0.5f && vShadowTex1.y < 0.5f)
		vTexcoordLight =  TexcoordLight1;

	float2 vShadowTex2 = (TexcoordLight2.xy / TexcoordLight2.w);

	if (vShadowTex2.x < 0.5f && vShadowTex2.y > 0.5f)
		vTexcoordLight = TexcoordLight2;

	float2 vShadowTex3 = (TexcoordLight3.xy / TexcoordLight3.w);

	if (vShadowTex3.x > 0.5f && vShadowTex3.y > 0.5f)
		vTexcoordLight = TexcoordLight3;

	return vTexcoordLight;
}


//=============================================================================
// GetSplitLightColor
//=============================================================================
float4	GetSplitLightColor(float4 TexcoordLight0, float4 TexcoordLight1, float4 TexcoordLight2, float4 TexcoordLight3)
{
	float2 vShadowTex0 = (TexcoordLight0.xy / TexcoordLight0.w);

	float4 vColor;

	if (vShadowTex0.x > 0.5f && vShadowTex0.y < 0.5f)
		vColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

	float2 vShadowTex1 = (TexcoordLight1.xy / TexcoordLight1.w);

	if (vShadowTex1.x < 0.5f && vShadowTex1.y < 0.5f)
		vColor = float4(0.0f, 1.0f, 0.0f, 1.0f);

	float2 vShadowTex2 = (TexcoordLight2.xy / TexcoordLight2.w);

	if (vShadowTex2.x < 0.5f && vShadowTex2.y > 0.5f)
		vColor = float4(0.0f, 0.0f, 1.0f, 1.0f);

	float2 vShadowTex3 = (TexcoordLight3.xy / TexcoordLight3.w);

	if (vShadowTex3.x > 0.5f && vShadowTex3.y > 0.5f)
		vColor = float4(1.0f, 1.0f, 0.0f, 1.0f);

	//return float4(vShadowTex0.x, vShadowTex0.y, 0.0f, 1.0f);
	return vColor;
}


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
			float2 vShadowTexC = (TexcoordLight.xy / TexcoordLight.w) + float2(fX, fY) * fShadowmapSizeInv - fShadowmapSizeInv * 0.5f;
			
			// Determine the lerp amounts
			float2 vLerps = frac(fShadowmapSize * vShadowTexC);

			float fDepth = TexcoordLight.z / TexcoordLight.w;

			// Read shadow samples
			float fSamples[4];
			fSamples[0] = (tex2D(shadowmap, vShadowTexC).r < fDepth) ? 0.0f : 1.0f;
			fSamples[1] = (tex2D(shadowmap, vShadowTexC + float2(fShadowmapSizeInv, 0.0f)).r < fDepth) ? 0.0f : 1.0f;
			fSamples[2] = (tex2D(shadowmap, vShadowTexC + float2(0.0f, fShadowmapSizeInv)).r < fDepth) ? 0.0f : 1.0f;
			fSamples[3] = (tex2D(shadowmap, vShadowTexC + float2(fShadowmapSizeInv , fShadowmapSizeInv)).r < fDepth) ? 0.0f : 1.0f;

			// Percentage-closer filtering
			fShadow += lerp(lerp(fSamples[0], fSamples[1], vLerps.x), lerp(fSamples[2], fSamples[3], vLerps.x), vLerps.y);
		}
	}

	return fShadow / (SHADOWMAP_FILTER_WIDTH * SHADOWMAP_FILTER_WIDTH) * vShadowLeak.x + vShadowLeak.y;

#else // SHADOWMAP_FILTER_WIDTH == 0

	float2 vShadowTexC = (TexcoordLight.xy / TexcoordLight.w);
	float fDepth = TexcoordLight.z / TexcoordLight.w;

	return ((tex2D(shadowmap, vShadowTexC).r < fDepth) ? 0.0f : 1.0f) * vShadowLeak.x + vShadowLeak.y; 

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