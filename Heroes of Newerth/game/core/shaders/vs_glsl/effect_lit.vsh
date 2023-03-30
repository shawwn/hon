// (C)2008 S2 Games
// effect_lit.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform vec3	vAmbient;
uniform vec3	vSunColor;

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
	gl_TexCoord[0] = gl_MultiTexCoord0;

#if (LIGHTING != 0)
	v_vColor       = vec4(gl_Color.rgb * (vAmbient + vSunColor), gl_Color.a);
#else
	v_vColor       = gl_Color;
#endif
}
