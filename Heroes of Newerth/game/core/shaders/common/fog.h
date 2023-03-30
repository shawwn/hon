// (C)2006 S2 Games
// fog.h
// 
// Fog support functions
//=============================================================================

//=============================================================================
// Global variables
//=============================================================================
float		fFogStart;
float		fFogEnd;
float		fFogDelta;
float		fFogScale;
float3		vFogColor;
float		fFogDensity;
float3		vFog;

//=============================================================================
// Fog
//=============================================================================
float	Fog(float3 PositionView)
{
#if (FOG_TYPE == 0) // FOG_NONE
	return 0.0f;
#elif (FOG_TYPE == 1) // FOG_LINEAR
	return saturate(length(PositionView) * vFog.x + vFog.y) * vFog.z;
#elif (FOG_TYPE == 2) // FOG_EXP2
	return 1.0f - exp2(-length(PositionView) * fFogDensity);
#elif (FOG_TYPE == 3) // FOG_EXP
	return 1.0f - exp(-length(PositionView) * fFogDensity);
#elif (FOG_TYPE == 4) // FOG_HERMITE
	return smoothstep(fFogStart, fFogEnd, length(PositionView)) * fFogScale;
#endif
}


//=============================================================================
// FogD
//=============================================================================
float	FogD(float fDistance)
{
#if (FOG_TYPE == 0) // FOG_NONE
	return 0.0f;
#elif (FOG_TYPE == 1) // FOG_LINEAR
	return saturate(fDistance * vFog.x + vFog.y) * vFog.z;
#elif (FOG_TYPE == 2) // FOG_EXP2
	return 1.0f - exp2(-fDistance * fFogDensity);
#elif (FOG_TYPE == 3) // FOG_EXP
	return 1.0f - exp(-fDistance * fFogDensity);
#elif (FOG_TYPE == 4) // FOG_HERMITE
	return smoothstep(fFogStart, fFogEnd, fDistance) * fFogScale;
#endif
}