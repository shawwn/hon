// (C)2009 S2 Games
// post_blur_v.vsh
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
	
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -11.5);
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -9.5);
	gl_TexCoord[1].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -7.5);
	gl_TexCoord[2].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -5.5);
	gl_TexCoord[2].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -3.5);
	gl_TexCoord[3].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * -1.5);
	gl_TexCoord[3].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 0.0);
	gl_TexCoord[4].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 1.5);
	gl_TexCoord[4].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 3.5);
	gl_TexCoord[5].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 5.5);
	gl_TexCoord[5].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 7.5);
	gl_TexCoord[6].xy = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 9.5);
	gl_TexCoord[6].zw = gl_MultiTexCoord0.xy + vec2(0.0, vTexelSize.y * 11.5);
}
