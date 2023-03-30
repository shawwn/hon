// (C)2010 S2 Games
// c_regexp.h
//
//	Definitions etc. for regexp(3) routines.
// Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
// not the System V one.
//
//=============================================================================
#ifndef __C_REGEXP_H__
#define __C_REGEXP_H__

//=============================================================================
// Declarations
//=============================================================================
struct regexp;
//=============================================================================

//=============================================================================
// CRegexp
//=============================================================================
class CRegexp
{
private:
	regexp*		m_pRegex;

	void		Init(const char* pExprUTF8);

public:
	CRegexp(const char* pExprUTF8) : m_pRegex(NULL)	{ Init(pExprUTF8); }
	CRegexp(const tstring& sExpr) : m_pRegex(NULL)	{ Init(TStringToUTF8(sExpr).c_str()); }
	~CRegexp();

	// perform a regular expression match against a single string.
	bool		Match(const char* pTestUTF8);
	bool		Match(const tstring& sStr)			{ return Match(TStringToUTF8(sStr).c_str()); }
};
//=============================================================================

#endif //__C_REGEXP_H__
