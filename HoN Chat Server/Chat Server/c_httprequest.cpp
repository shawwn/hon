// (C)2006 S2 Games
// c_httprequest.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_common.h"

#include "c_httprequest.h"
#include "c_httpmanager.h"
#include "c_console.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR(uint, http_defaultTimeout,			30);
CVAR(uint, http_defaultConnectTimeout,	5);
CVAR(uint, http_defaultLowSpeedLimit,	10);
CVAR(uint, http_defaultLowSpeedTimeout,	25);
CVAR(bool, http_useCompression,			true);
CVAR(bool, http_printDebugInfo,			false);
//=============================================================================

/*====================
  CHTTPRequest::~CHTTPRequest
  ====================*/
CHTTPRequest::~CHTTPRequest()
{
}


/*====================
  CHTTPRequest::CHTTPRequest
  ====================*/
CHTTPRequest::CHTTPRequest(CCore *pCore, CURL *pCurlEasy, uint uiID) :
NULL_CORE_API,

m_uiID(uiID),
m_pCurlEasy(pCurlEasy),
m_eStatus(HTTP_REQUEST_IDLE),

m_uiTimeout(http_defaultTimeout),
m_uiConnectTimeout(http_defaultConnectTimeout),
m_uiLowSpeedLimit(http_defaultLowSpeedLimit),
m_uiLowSpeedTime(http_defaultLowSpeedTimeout),

m_uiType(0)
{
	INIT_CORE_API(pCore);
	memset(m_szErrorBuffer, 0, sizeof(m_szErrorBuffer));
}


/*====================
  CHTTPRequest::Completed
  ====================*/
void	CHTTPRequest::Completed(CURLcode code)
{
	if (code == CURLE_OK)
	{
		m_eStatus = HTTP_REQUEST_SUCCESS;
		m_bufferResponse << byte(0);
		m_sResponse = UTF8ToWString(m_bufferResponse.Get());
	}
	else
	{
		m_pConsole->Std() << _T("Request returned: ") << curl_easy_strerror(code) << newl;
		if (m_szErrorBuffer[0] != 0)
			m_pConsole->Std() << _T("Error buffer: ") << m_szErrorBuffer << newl;

		m_pHTTPManager->IncrementFailedRequests();
		m_eStatus = HTTP_REQUEST_ERROR;
	}
}

void	CHTTPRequest::Completed(CURLMcode code)
{
	if (code == CURLM_OK)
	{
		m_eStatus = HTTP_REQUEST_SUCCESS;
		m_bufferResponse << byte(0);
		m_sResponse = UTF8ToWString(m_bufferResponse.Get());
	}
	else
	{
		m_pConsole->Std() << _T("Request returned: ") << curl_multi_strerror(code) << newl;
		if (m_szErrorBuffer[0] != 0)
			m_pConsole->Std() << _T("Error buffer: ") << m_szErrorBuffer << newl;

		m_pHTTPManager->IncrementFailedRequests();
		m_eStatus = HTTP_REQUEST_ERROR;
	}
}


/*====================
  CHTTPRequest::SendRequest
  ====================*/
size_t	CURLWriteMemoryCallback(void *pSrc, size_t zSize, size_t zBlocks, void *pDest)
{
	size_t zTotalSize(zSize * zBlocks);
	CBufferDynamic *pBuffer(static_cast<CBufferDynamic*>(pDest));

	pBuffer->Append(pSrc, INT_SIZE(zTotalSize));

	return zTotalSize;
}

int CURLDebugCallback(CURL *pCurlEasy, curl_infotype type, char *pString, size_t zSize, void *pData)
{
	CConsole *pConsole(static_cast<CConsole*>(pData));

	CBufferDynamic buffer(uint(zSize + 1));
	buffer.Write(pString, uint(zSize));
	buffer << byte(0);

	switch (type)
	{
	case CURLINFO_TEXT:
		pConsole->Admin() << L"CURL: " << buffer.Get();
		break;
	case CURLINFO_HEADER_IN:
		pConsole->Admin() << L"CURL (header in): " << NormalizeLineBreaks(buffer.Get());
		break;
	case CURLINFO_HEADER_OUT:
		pConsole->Admin() << L"CURL (header out): " << NormalizeLineBreaks(buffer.Get());
		break;
	case CURLINFO_DATA_IN:
		pConsole->Admin() << L"CURL (data in): " << NormalizeLineBreaks(buffer.Get());
		break;
	case CURLINFO_DATA_OUT:
		pConsole->Admin() << L"CURL (data out): " << NormalizeLineBreaks(buffer.Get());
		break;
	}
	return 0;
}

void	CHTTPRequest::SendRequest(const string &sURL, bool bPost, bool bSSL)
{
	PROFILE("CHTTPRequest::SendRequest")

	if (bPost && m_vVariables.empty())
	{
		m_pConsole->Admin() << _T("CHTTPRequest::SendRequest() - POST request has no parameters") << newl;
		return;
	}

	m_bufferResponse.Clear();
	m_sResponse.clear();

	// Assemble POST string
	if (bPost)
	{
		string sVarString;
		for (StringPairVector_it it(m_vVariables.begin()); it != m_vVariables.end(); ++it)
			sVarString += (sVarString.empty() ? "" : "&") + it->first + "=" + it->second;

		curl_easy_setopt(m_pCurlEasy, CURLOPT_POST, 1l);
		curl_easy_setopt(m_pCurlEasy, CURLOPT_COPYPOSTFIELDS, sVarString.c_str());
		curl_easy_setopt(m_pCurlEasy, CURLOPT_POSTFIELDSIZE, -1l);
	}

	// Use IPv4
	curl_easy_setopt(m_pCurlEasy, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

	// Set target URL
	curl_easy_setopt(m_pCurlEasy, CURLOPT_URL, sURL.c_str());

	// Set timeout parameters
	curl_easy_setopt(m_pCurlEasy, CURLOPT_TIMEOUT, m_uiTimeout);
	curl_easy_setopt(m_pCurlEasy, CURLOPT_LOW_SPEED_LIMIT, m_uiLowSpeedLimit);
	curl_easy_setopt(m_pCurlEasy, CURLOPT_LOW_SPEED_TIME, m_uiLowSpeedTime);
	curl_easy_setopt(m_pCurlEasy, CURLOPT_CONNECTTIMEOUT, m_uiConnectTimeout);

	// Debugging
	if (http_printDebugInfo)
	{
		curl_easy_setopt(m_pCurlEasy, CURLOPT_VERBOSE, 1l);
		curl_easy_setopt(m_pCurlEasy, CURLOPT_DEBUGFUNCTION, CURLDebugCallback);
		curl_easy_setopt(m_pCurlEasy, CURLOPT_DEBUGDATA, static_cast<void*>(m_pConsole));
	}

	// Error buffer
	memset(m_szErrorBuffer, 0, sizeof(m_szErrorBuffer));
	curl_easy_setopt(m_pCurlEasy, CURLOPT_ERRORBUFFER, (void*)m_szErrorBuffer);

	// SSL
	if (bSSL)
	{
		curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYPEER, 1l);
		curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYHOST, 2l);
		curl_easy_setopt(m_pCurlEasy, CURLOPT_CAINFO, L"ca-bundle.crt");
	}

	// Authentication
	if (!m_sUserName.empty())
		curl_easy_setopt(m_pCurlEasy, CURLOPT_USERNAME, m_sUserName.c_str());
	if (!m_sPassword.empty())
		curl_easy_setopt(m_pCurlEasy, CURLOPT_PASSWORD, m_sPassword.c_str());

	// Compression
	if (http_useCompression)
		curl_easy_setopt(m_pCurlEasy, CURLOPT_ENCODING, "gzip,deflate");

	// Output
	curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEFUNCTION, CURLWriteMemoryCallback);
	curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEDATA, static_cast<void*>(&m_bufferResponse));

	m_eStatus = HTTP_REQUEST_SENDING;
	m_pHTTPManager->SendRequest(this);
}


/*====================
  CHTTPRequest::Reset
  ====================*/
void	CHTTPRequest::Reset()
{
	PROFILE("CHTTPRequest::Reset")

	m_eStatus = HTTP_REQUEST_IDLE;
	m_vVariables.clear();
	m_bufferResponse.Clear();
	m_sResponse.clear();
	memset(m_szErrorBuffer, 0, sizeof(m_szErrorBuffer));

	m_uiTimeout = http_defaultTimeout;
	m_uiConnectTimeout = http_defaultConnectTimeout;
	m_uiLowSpeedLimit = http_defaultLowSpeedLimit;
	m_uiLowSpeedTime = http_defaultLowSpeedTimeout;

	m_uiType = 0;
}


/*====================
  CHTTPRequest::Wait
  ====================*/
void	CHTTPRequest::Wait()
{
	while (m_eStatus == HTTP_REQUEST_SENDING)
		m_pHTTPManager->Frame();
}
