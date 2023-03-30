// (C)2009 S2 Games
// c_cvar.h
//
//=============================================================================
#ifndef __C_CVAR_H__
#define __C_CVAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consoleelement.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define CVAR(type, name, def)	CCVar<type> name(L#name, def);
#define EXTERN_CVAR(type, name)	extern CCVar<type> name;
//=============================================================================

//=============================================================================
// ICVar
//=============================================================================
class ICVar : public IConsoleElement
{
private:
	ICVar();
	ICVar(const ICVar&);

protected:

public:
	ICVar(const wstring &sName) :
	IConsoleElement(sName, CONSOLE_VARIABLE)
	{
	}

	virtual ~ICVar()	{}

	virtual wstring	GetString() const = 0;
	virtual void	Set(const wstring &sValue) = 0;
};
//=============================================================================

//=============================================================================
// CCVar
//=============================================================================
template <class T>
class CCVar : public ICVar
{
private:
	CCVar();
	CCVar(const CCVar&);

	T	m_Value;

public:
	~CCVar()	{}

	CCVar(const wstring &sName, T _Default) :
	ICVar(sName),
	m_Value(_Default)
	{
	}

	void	Set(T _Value)					{ m_Value = _Value; }
	void	Set(const wstring &sValue)		{ AtoX(sValue, m_Value); }

	wstring	GetString() const				{ return XtoW(m_Value); }
	T		Get() const						{ return m_Value; }

	bool	operator==(const T &_Value)		{ return m_Value == _Value; }
	bool	operator!=(const T &_Value)		{ return m_Value != _Value; }
	bool	operator<=(const T &_Value)		{ return m_Value <= _Value; }
	bool	operator>=(const T &_Value)		{ return m_Value >= _Value; }
	bool	operator<(const T &_Value)		{ return m_Value < _Value; }
	bool	operator>(const T &_Value)		{ return m_Value > _Value; }

	T		operator=(const T &_Value)		{ m_Value = _Value; return m_Value; }
	T		operator+=(const T &_Value)		{ m_Value += _Value; return m_Value; }
	T		operator-=(const T &_Value)		{ m_Value -= _Value; return m_Value; }
	T		operator*=(const T &_Value)		{ m_Value *= _Value; return m_Value; }
	T		operator/=(const T &_Value)		{ m_Value /= _Value; return m_Value; }
	T		operator%=(const T &_Value)		{ m_Value %= _Value; return m_Value; }

	operator T() const						{ return m_Value; }
};
//=============================================================================

//=============================================================================
// CCVar<wstring>
//=============================================================================
template<>
class CCVar<wstring> : public ICVar
{
private:
	CCVar();
	CCVar(const CCVar<wstring>&);

	wstring	m_sValue;

public:
	~CCVar()	{}

	CCVar(const wstring &sName, const wstring &sDefault) :
	ICVar(sName),
	m_sValue(sDefault)
	{
	}

	void			Set(const wstring &sValue)			{ m_sValue = sValue; }

	wstring			GetString() const					{ return m_sValue; }
	const wstring&	Get() const							{ return m_sValue; }

	const wstring&	operator=(const wstring &sValue)	{ m_sValue = sValue; return m_sValue; }
	const wstring&	operator+=(const wstring &sValue)	{ m_sValue += sValue; return m_sValue; }

	operator const wstring&() const						{ return m_sValue; }
};
//=============================================================================

//=============================================================================
// CCVar<string>
//=============================================================================
template<>
class CCVar<string> : public ICVar
{
private:
	CCVar();
	CCVar(const CCVar<string>&);

	string	m_sValue;

public:
	~CCVar()	{}

	CCVar(const wstring &sName, const string &sDefault) :
	ICVar(sName),
	m_sValue(sDefault)
	{
	}

	void			Set(const wstring &sValue)			{ m_sValue = WideToSingle(sValue); }

	wstring			GetString() const					{ return SingleToWide(m_sValue); }
	const string&	Get() const							{ return m_sValue; }

	const string&	operator=(const string &sValue)		{ m_sValue = sValue; return m_sValue; }
	const string&	operator+=(const string &sValue)	{ m_sValue += sValue; return m_sValue; }

	operator const string&() const						{ return m_sValue; }
};
//=============================================================================

#endif //__C_CVAR_H__