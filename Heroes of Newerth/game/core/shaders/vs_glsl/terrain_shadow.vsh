// (C)2008 S2 Games
// terrain_shadow.vsh
// 
// Terrain
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform vec4	vColor;
uniform float	fWorldTileSize;

//=============================================================================
// Varying variables
//=============================================================================
#if (SHADOWMAP_TYPE == 0) 
// SHADOWMAP_R32F
varying vec2	v_vDepth;
#endif

//=============================================================================
// Vertex attributes
//=============================================================================
attribute float	a_fHeight;
attribute vec4	a_vData0;
attribute vec4	a_vData1;

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
	vec2 vTile = vec2(a_vData0.w, a_vData1.w);
	vec3 vOffset = (a_vData0.xyz / 255.0 * 2.0 - 1.0) * 4.0; // vNormal * 4.0
	
	gl_Position    = gl_ModelViewProjectionMatrix * vec4(vec3(vTile * fWorldTileSize, a_fHeight) - vOffset, 1.0);
	
#if (SHADOWMAP_TYPE == 0) 
// SHADOWMAP_R32F
	v_vDepth       = gl_Position.zw;
#endif
}
