// (C)2008 S2 Games
// sky.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================\
uniform vec4	vColor;

uniform float	fSkyEpsilon;

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;

//=============================================================================
// Vertex shader
//=============================================================================
void main()
{
	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position.z  = gl_Position.w - fSkyEpsilon;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	v_vColor       = gl_Color * vColor;
}
