// (C)2009 S2 Games
// post_blur_h.vsh
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
	
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -11.5, 0.0);
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -9.5, 0.0);
	gl_TexCoord[1].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -7.5, 0.0);
	gl_TexCoord[2].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -5.5, 0.0);
	gl_TexCoord[2].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -3.5, 0.0);
	gl_TexCoord[3].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * -1.5, 0.0);
	gl_TexCoord[3].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 0.0, 0.0);
	gl_TexCoord[4].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 1.5, 0.0);
	gl_TexCoord[4].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 3.5, 0.0);
	gl_TexCoord[5].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 5.5, 0.0);
	gl_TexCoord[5].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 7.5, 0.0);
	gl_TexCoord[6].xy = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 9.5, 0.0);
	gl_TexCoord[6].zw = gl_MultiTexCoord0.xy + vec2(vTexelSize.x * 11.5, 0.0);
}
