// (C)2008 S2 Games
// c_gamma.h
//
// Gamma manager
//=============================================================================
#ifndef __C_GAMMA_H__
#define __C_GAMMA_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SGammaRamp
{
	ushort	unRed[256];
	ushort	unGreen[256];
	ushort	unBlue[256];
};
//=============================================================================

//=============================================================================
// CGamma
//=============================================================================
class CGamma
{
private:
	float		m_fD3DGamma;
	float		m_fGDIGamma;

	bool		m_bSaved;
	SGammaRamp	m_cSavedGDIGammaRamp;

	void	UpdateD3DGamma(float fGamma);
	void	UpdateGDIGamma(float fGamma);

public:
	~CGamma();
	CGamma();

	void	SaveGDIGammaRamp();
	void	RestoreGDIGammaRamp();

	void	Update(float fGamma);
};

extern CGamma	g_Gamma;
//=============================================================================

#endif //__C_GAMMA_H__
