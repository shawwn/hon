// (C)2006 S2 Games
// c_httprequest.h
//
//=============================================================================
#ifndef __C_HTTPREQUEST_H__
#define __C_HTTPREQUEST_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_core.h"

#include "curl/curl.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EHTTPPostStatus
{
	HTTP_REQUEST_IDLE,
	HTTP_REQUEST_SENDING,
	HTTP_REQUEST_SUCCESS,
	HTTP_REQUEST_ERROR
};
//=============================================================================

//=============================================================================
// CHTTPRequest
//=============================================================================
class CHTTPRequest
{
private:
	DECLARE_CORE_API;

	typedef pair<string, string>		StringPair;
	typedef vector<StringPair>			StringPairVector;
	typedef StringPairVector::iterator	StringPairVector_it;
	
	uint					m_uiID;
	CURL*					m_pCurlEasy;
	
	string					m_sURL;
	char					m_szErrorBuffer[CURL_ERROR_SIZE + 1];
	CBufferDynamic			m_bufferResponse;
	EHTTPPostStatus			m_eStatus;
	StringPairVector		m_vVariables;
	wstring					m_sResponse;

	uint					m_uiTimeStamp;
	uint					m_uiTimeout;
	uint					m_uiConnectTimeout;
	uint					m_uiLowSpeedLimit;
	uint					m_uiLowSpeedTime;

	string					m_sUserName;
	string					m_sPassword;

	// This 'type' is provided for the utility of whatever is making use of the request
	// and has no effect on the request itself
	uint					m_uiType;

	void	SendRequest(const string &sURL, bool bPost, bool bSSL);

	CHTTPRequest();

public:
	~CHTTPRequest();
	CHTTPRequest(CCore *pCore, CURL *pCurlEasy, uint uiID);

	inline uint		GetID() const												{ return m_uiID; }
	inline CURL*	GetCURL() const												{ return m_pCurlEasy; }

	inline uint		GetType() const												{ return m_uiType; }
	inline void		SetType(uint uiType)										{ m_uiType = uiType; }

	void			Completed(CURLcode code);
	void			Completed(CURLMcode code);

	void			SetTimeStamp(uint uiTimeStamp)								{ m_uiTimeStamp = uiTimeStamp; }
	uint			GetTimeStamp() const										{ return m_uiTimeStamp; }

	inline void		SetTargetURL(const string &sURL)							{ m_sURL = sURL; }
	inline void		SetUserName(const wstring &sUserName)						{ m_sUserName = WStringToUTF8(sUserName); }
	inline void		SetPassword(const wstring &sPassword)						{ m_sPassword = WStringToUTF8(sPassword); }

	inline void		ClearVariables()											{ m_vVariables.clear(); }
	inline void		AddVariable(const wstring &sName, const string &sValue)		{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(sValue))); }
	inline void		AddVariable(const wstring &sName, const wstring &sValue)	{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(WStringToUTF8(sValue)))); }
	inline void		AddVariable(const wstring &sName, uint uiVal)				{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(uiVal)))); }
	inline void		AddVariable(const wstring &sName, int iVal)					{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(iVal)))); }
	inline void		AddVariable(const wstring &sName, float fVal)				{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(fVal)))); }

	inline void		SetTimeout(uint uiSeconds)									{ m_uiTimeout = uiSeconds; }
	inline void		SetConnectTimeout(uint uiSeconds)							{ m_uiConnectTimeout = uiSeconds; }
	inline void		SetLowSpeedTimeout(uint uiRate, uint uiSeconds)				{ m_uiLowSpeedLimit = uiRate; m_uiLowSpeedTime = uiSeconds; }
	
	inline void		SendSecurePostRequest()										{ return SendRequest("https://" + m_sURL, true, true); }
	inline void		SendPostRequest()											{ return SendRequest("http://" + m_sURL, true, false); }
	inline void		SendRequest()												{ return SendRequest("http://" + m_sURL, false, false); }
	void			Reset();

	void			Wait();

	inline uint				GetNumVariables()									{ return (uint)(m_vVariables.size()); }
	inline const wstring&	GetResponse() const									{ return m_sResponse; }
	inline EHTTPPostStatus	GetStatus() const									{ return m_eStatus; }

	bool			IsActive() const											{ return m_eStatus == HTTP_REQUEST_SENDING; }
	bool			WasSuccessful() const										{ return m_eStatus == HTTP_REQUEST_SUCCESS; }
};
//=============================================================================

#endif //__C_HTTPREQUEST_H__
