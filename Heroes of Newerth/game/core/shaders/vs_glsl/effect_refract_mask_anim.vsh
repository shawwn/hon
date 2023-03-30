// (C)2009 S2 Games
// effect_refract_mask_anim.vsh
// 
// ...
//=============================================================================

//=============================================================================
// Uniform variables
//=============================================================================
uniform mat4	mSceneProj;

uniform vec2	vTexOffset;

//=============================================================================
// Varying variables
//=============================================================================
varying vec4	v_vColor;
varying vec4	v_vPositionScreen;

//=============================================================================
// Vertex shader
//=============================================================================
void main()
{
	gl_Position       = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0]    = gl_MultiTexCoord0;
	gl_TexCoord[1].xy = gl_MultiTexCoord0.xy + vTexOffset;
	v_vColor          = gl_Color;
	v_vPositionScreen = mSceneProj * gl_Position;
}
