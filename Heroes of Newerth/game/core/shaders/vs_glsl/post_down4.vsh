// (C)2009 S2 Games
// post_down4.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform vec4	vTexelSize;

//=============================================================================
// Varying variables
//=============================================================================

//=============================================================================
// Vertex Shader
//=============================================================================
void main()
{
	gl_Position       = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xy, 0.0, 1.0);
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy * 4.0 + vTexelSize.xy;
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy * 4.0 + vec2(vTexelSize.x, -vTexelSize.y);
	gl_TexCoord[2].xy = gl_MultiTexCoord0.xy * 4.0 + vec2(-vTexelSize.x, vTexelSize.y);
	gl_TexCoord[3].xy = gl_MultiTexCoord0.xy * 4.0 - vTexelSize.xy;
}
