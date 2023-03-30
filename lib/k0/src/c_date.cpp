// (C)2005 S2 Games
// c_date.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k0_common.h"

#include <time.h>

#include "c_date.h"
//=============================================================================

/*====================
  CDate::CDate
  ====================*/
CDate::CDate(bool b)
{
	time_t		t;
	DECLARE_TIME_STRUCT_S(datetime);

	time(&t);
	LOCALTIME_S(&t, datetime);
	m_iYear = datetime->tm_year + 1900;
	m_iMonth = datetime->tm_mon + 1;
	m_iDay = datetime->tm_mday;

	m_iHour = datetime->tm_hour;
	m_iMinute = datetime->tm_min;
	m_iSecond = datetime->tm_sec;
}

CDate::CDate(time_t t)
{
	DECLARE_TIME_STRUCT_S(datetime);
	LOCALTIME_S(&t, datetime);

	m_iYear = datetime->tm_year + 1900;
	m_iMonth = datetime->tm_mon + 1;
	m_iDay = datetime->tm_mday;

	m_iHour = datetime->tm_hour;
	m_iMinute = datetime->tm_min;
	m_iSecond = datetime->tm_sec;
}


/*====================
  CDate::IsDay
  ====================*/
bool	CDate::IsDay(EMonth eMonth, int iDay)
{
	return (eMonth == m_iMonth && iDay == m_iDay);
}

bool	CDate::IsDay(int iYear, EMonth eMonth, int iDay)
{
	return (iYear == m_iYear && eMonth == m_iMonth && iDay == m_iDay);
}


/*====================
  CDate::GetDateString
  ====================*/
wstring	CDate::GetDateString(uint uiFlags, wchar_t cSeperator)
{
	if (m_iYear == -1)
		return L"<INVALID DATE>";

	wstring sYear;
	if (uiFlags & DATE_SHORT_YEAR)
		sYear = XtoW(m_iYear % 100, FMT_PADZERO, 2);
	else
		sYear = XtoW(m_iYear);

	wstring sDate;
	if (!(uiFlags & DATE_YEAR_LAST))
		sDate += sYear + cSeperator;

	if (uiFlags & DATE_MONTH_FIRST)
		sDate += XtoW(m_iMonth, FMT_PADZERO, 2) + cSeperator + XtoW(m_iDay, FMT_PADZERO, 2);
	else
		sDate += XtoW(m_iDay, FMT_PADZERO, 2) + cSeperator + XtoW(m_iMonth, FMT_PADZERO, 2);

	if (uiFlags & DATE_YEAR_LAST)
		sDate += cSeperator + sYear;

	return sDate;
}


/*====================
  CDate::GetTimeString
  ====================*/
wstring CDate::GetTimeString(uint uiFlags, wchar_t cSeperator)
{
	if (m_iHour == -1)
		return L"<INVALID TIME>";

	wstring sTime;
	if (uiFlags & TIME_TWELVE_HOUR)
		sTime = XtoW((m_iHour + 11) % 12, FMT_PADZERO, 2) + cSeperator + XtoW(m_iMinute, FMT_PADZERO, 2);
	else
		sTime = XtoW(m_iHour, FMT_PADZERO, 2) + cSeperator + XtoW(m_iMinute, FMT_PADZERO, 2);

	if (!(uiFlags & TIME_NO_SECONDS))
		sTime += cSeperator + XtoW(m_iSecond, FMT_PADZERO, 2);

	if (uiFlags & TIME_TWELVE_HOUR)
		sTime += (m_iHour < 12) ? L" AM" : L" PM";

	return sTime;
}


/*====================
  CDate::GetString
  ====================*/
wstring	CDate::GetString()
{
	if (m_iHour == -1)
		return GetDateString();
	else
		return GetDateString() + L" " + GetTimeString();
}


/*====================
  CDate::operator==
  ====================*/
bool	CDate::operator==(const CDate &B)
{
	// Check for uninitialized value
	if (m_iYear < 0 || B.m_iYear < 0)
		return false;

	// Check date equality
	if (m_iYear == B.m_iYear &&
		m_iMonth == B.m_iMonth &&
		m_iDay == B.m_iDay)
	{
		// If time is uninitialized, it's a match
		if (m_iHour < 0 && B.m_iHour < 0)
			return true;

		// Check for a time match
		if (m_iHour == B.m_iHour &&
			m_iMinute == B.m_iMinute &&
			m_iSecond == B.m_iSecond)
			return true;
	}

	return false;
}


/*====================
  CDate::operator>
  ====================*/
bool	CDate::operator>(const CDate &B)
{
	// Check for uninitialized values
	if (m_iYear < 0 || B.m_iYear < 0)
		return false;

	if (m_iYear > B.m_iYear)
		return true;
	if (m_iYear < B.m_iYear)
		return false;

	if (m_iMonth > B.m_iMonth)
		return true;
	if (m_iMonth < B.m_iMonth)
		return false;

	if (m_iDay > B.m_iDay)
		return true;
	if (m_iDay < B.m_iDay)
		return false;

	// If we made it here, the date is equal
	// If time is uninitialized, we stop
	if (m_iHour < 0 || B.m_iHour < 0)
		return false;

	if (m_iHour > B.m_iHour)
		return true;
	if (m_iHour < B.m_iHour)
		return false;

	if (m_iMinute > B.m_iMinute)
		return true;
	if (m_iMinute < B.m_iMinute)
		return false;

	if (m_iSecond > B.m_iSecond)
		return true;

	return false;
}


/*====================
  CDate::operator<
  ====================*/
bool	CDate::operator<(const CDate &B)
{
	// Check for uninitialized values
	if (m_iYear < 0 || B.m_iYear < 0)
		return false;

	if (m_iYear < B.m_iYear)
		return true;
	if (m_iYear > B.m_iYear)
		return false;

	if (m_iMonth < B.m_iMonth)
		return true;
	if (m_iMonth > B.m_iMonth)
		return false;

	if (m_iDay < B.m_iDay)
		return true;
	if (m_iDay > B.m_iDay)
		return false;

	// If we made it here, the date is equal
	// If time is uninitialized, we stop
	if (m_iHour < 0 || B.m_iHour < 0)
		return false;

	if (m_iHour < B.m_iHour)
		return true;
	if (m_iHour > B.m_iHour)
		return false;

	if (m_iMinute < B.m_iMinute)
		return true;
	if (m_iMinute > B.m_iMinute)
		return false;

	if (m_iSecond < B.m_iSecond)
		return true;

	return false;
}


/*====================
  CDate::operator>=
  ====================*/
bool	CDate::operator>=(const CDate &B)
{
	// Check for uninitialized values
	if (m_iYear < 0 || B.m_iYear < 0)
		return false;

	if (m_iYear < B.m_iYear)
		return false;
	if (m_iMonth < B.m_iMonth)
		return false;
	if (m_iDay < B.m_iDay)
		return false;

	// If we made it here, the date is greater than or equal
	// If time is uninitialized, we stop
	if (m_iHour < 0 || B.m_iHour < 0)
		return true;

	if (m_iHour < B.m_iHour)
		return false;
	if (m_iMinute < B.m_iMinute)
		return false;
	if (m_iSecond < B.m_iSecond)
		return false;

	return true;
}


/*====================
  CDate::operator<=
  ====================*/
bool	CDate::operator<=(const CDate &B)
{
	// Check for uninitialized values
	if (m_iYear < 0 || B.m_iYear < 0)
		return false;

	if (m_iYear > B.m_iYear)
		return false;
	if (m_iMonth > B.m_iMonth)
		return false;
	if (m_iDay > B.m_iDay)
		return false;

	// If we made it here, the date is less than or equal
	// If time is uninitialized, we stop
	if (m_iHour < 0 || B.m_iHour < 0)
		return true;

	if (m_iHour > B.m_iHour)
		return false;
	if (m_iMinute > B.m_iMinute)
		return false;
	if (m_iSecond > B.m_iSecond)
		return false;

	return true;
}


/*====================
  CDate::operator+
  ====================*/
CDate	CDate::operator+(const CDate &B)
{
	time_t t;
	DECLARE_TIME_STRUCT_S(datetime);

	// This will set the DST value for us
	time(&t);
	LOCALTIME_S(&t, datetime);

	// Get all the sums
	datetime->tm_sec = m_iSecond + B.m_iSecond;
	datetime->tm_min = m_iMinute + B.m_iMinute;
	datetime->tm_hour = m_iHour + B.m_iHour;
	datetime->tm_mday = m_iDay + B.m_iDay;
	datetime->tm_mon = m_iMonth + B.m_iMonth;
	datetime->tm_year = m_iYear + B.m_iYear;

	// This will normalize all the values and give us a valid time_t
	t = mktime(datetime);
	if (t == -1)
		return CDate(-1, 0, 0);

	// Retrieve the individual values from the new time_t
	LOCALTIME_S(&t, datetime);
	return CDate(datetime->tm_year, datetime->tm_mon, datetime->tm_mday, datetime->tm_hour, datetime->tm_min, datetime->tm_sec);
}


/*====================
  CDate::operator-
  ====================*/
CDate	CDate::operator-(const CDate &B)
{
	time_t t;
	DECLARE_TIME_STRUCT_S(datetime);

	// This will set the DST value for us
	time(&t);
	LOCALTIME_S(&t, datetime);

	// Get all the sums
	datetime->tm_sec = m_iSecond - B.m_iSecond;
	datetime->tm_min = m_iMinute - B.m_iMinute;
	datetime->tm_hour = m_iHour - B.m_iHour;
	datetime->tm_mday = m_iDay - B.m_iDay;
	datetime->tm_mon = m_iMonth - B.m_iMonth;
	datetime->tm_year = m_iYear - B.m_iYear;

	// This will normalize all the values and give us a valid time_t
	t = mktime(datetime);
	if (t == -1)
		return CDate(-1, 0, 0);

	// Retrieve the individual values from the new time_t
	LOCALTIME_S(&t, datetime);
	return CDate(datetime->tm_year, datetime->tm_mon, datetime->tm_mday, datetime->tm_hour, datetime->tm_min, datetime->tm_sec);
}


/*====================
  CDate::operator+=
  ====================*/
CDate&	CDate::operator+=(const CDate &B)
{
	time_t t;
	DECLARE_TIME_STRUCT_S(datetime);

	// This will set the DST value for us
	time(&t);
	LOCALTIME_S(&t, datetime);

	// Get all the sums
	datetime->tm_sec += B.m_iSecond;
	datetime->tm_min += B.m_iMinute;
	datetime->tm_hour += B.m_iHour;
	datetime->tm_mday += B.m_iDay;
	datetime->tm_mon += B.m_iMonth;
	datetime->tm_year += B.m_iYear;

	// This will normalize all the values and give us a valid time_t
	t = mktime(datetime);
	if (t == -1)
	{
		Set(-1, 0, 0, -1, 0, 0);
		return *this;
	}

	// Retrieve the individual values from the new time_t
	LOCALTIME_S(&t, datetime);
	Set(datetime->tm_year, datetime->tm_mon, datetime->tm_mday, datetime->tm_hour, datetime->tm_min, datetime->tm_sec);
	return *this;
}


/*====================
  CDate::operator-=
  ====================*/
CDate&	CDate::operator-=(const CDate &B)
{
	time_t t;
	DECLARE_TIME_STRUCT_S(datetime);

	// This will set the DST value for us
	time(&t);
	LOCALTIME_S(&t, datetime);

	// Get all the sums
	datetime->tm_sec -= B.m_iSecond;
	datetime->tm_min -= B.m_iMinute;
	datetime->tm_hour -= B.m_iHour;
	datetime->tm_mday -= B.m_iDay;
	datetime->tm_mon -= B.m_iMonth;
	datetime->tm_year -= B.m_iYear;

	// This will normalize all the values and give us a valid time_t
	t = mktime(datetime);
	if (t == -1)
	{
		Set(-1, 0, 0, -1, 0, 0);
		return *this;
	}

	// Retrieve the individual values from the new time_t
	LOCALTIME_S(&t, datetime);
	Set(datetime->tm_year, datetime->tm_mon, datetime->tm_mday, datetime->tm_hour, datetime->tm_min, datetime->tm_sec);
	return *this;
}
