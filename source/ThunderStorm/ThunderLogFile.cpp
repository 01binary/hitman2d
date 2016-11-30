/*------------------------------------------------------------------*\
|
| ThunderLogFile.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm log file reading/writing class implementation
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderError.h"		// using ErrorManager
#include "ThunderLogFile.h"		// defining LogFile
#include "ThunderString.h"		// using String
#include <time.h>				// using time(), localtime() and wcsftime()

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const LPCWSTR LogFile::SZ_PREFIXES[PRINT_COUNT - 1] = {
														  L"<echo",
														  L"<error",
														  L"<warning",
														  L"<info",
														  L"<debug"
													  };

const WCHAR LogFile::SZ_LOGHTMLHEADERCOLOR[]			= L"<html>\r\n\r\n<body bgcolor=\"%s\">\r\n\r\n<font face=\"%s\" size=\"%dem\">\r\n\r\n<pre>";
const WCHAR LogFile::SZ_LOGHTMLHEADERIMAGE[]			= L"<html>\r\n\r\n<body bgcolor=\"%s\" background=\"%s\" style=\"%s\">\r\n\r\n<font face=\"%s\" size=\"%dem\">\r\n\r\n<pre>";
const WCHAR LogFile::SZ_LOGHTMLFOOTER[]					= L"</font></pre>\r\n\r\n</body>\r\n\r\n</html>";
const WCHAR LogFile::SZ_LOGHTMLENTRYSTART_OPEN[]		= L"<font color=\"";
const WCHAR LogFile::SZ_LOGHTMLENTRYSTART_CLOSE[]		= L"\">";
const WCHAR LogFile::SZ_LOGHTMLENTRYSPACE[]				= L"                        ";
const WCHAR LogFile::SZ_LOGHTMLENTRYEND[]				= L"</font>";
const WCHAR LogFile::SZ_LOGHTMLENTRYENDLINE[]			= L"\r\n";


/*----------------------------------------------------------*\
| LogFile implementation
\*----------------------------------------------------------*/

LogFile::LogFile(ErrorManager* pErrorContext,
								 bool bEnableDateTime,
								 bool bEnablePrefix):

								 m_pErrorContext(pErrorContext),
								 m_Stream(pErrorContext),
								 m_bEnableDateTime(bEnableDateTime),
								 m_bEnablePrefix(bEnablePrefix),
								 m_bAddingToLine(false)
{
}

LogFile::~LogFile(void)
{
	Empty();
}

const Stream& LogFile::GetStream(void) const
{
	return m_Stream;
}

const String& LogFile::GetPath(void) const
{
	return m_Stream.GetPath();
}

String LogFile::GetTitle(void) const
{
	return m_Stream.GetTitle();
}

bool LogFile::IsDateTimeEnabled(void) const
{
	return m_bEnableDateTime;
}

void LogFile::EnableDateTime(bool bEnable)
{
	m_bEnableDateTime = bEnable;
}

bool LogFile::IsPrefixEnabled(void) const
{
	return m_bEnablePrefix;
}

void LogFile::EnablePrefix(bool bEnable)
{
	m_bEnablePrefix = bEnable;
}

void LogFile::Open(LPCWSTR pszPath)
{
	try
	{
		// Open the file and keep it open

		m_Stream.Open(pszPath, GENERIC_WRITE, OPEN_ALWAYS);
		
		if (m_Stream.GetSize() == 0)
		{
			// If the file has zero length, add unicode signature

			m_Stream.WriteUnicodeSignature();
		}
		else
		{
			// Otherwise, set to the end of file

			m_Stream.SetPosition(0, Stream::MOVE_END);
		}
	}
	
	catch(Error& rError)
	{
		if (m_pErrorContext != NULL)
			m_pErrorContext->Push(rError);

		throw m_pErrorContext->Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}
}

void LogFile::Print(LPCWSTR pszText, 
					PrintTypes nEntryType,
					bool bLine)
{
	// Clearing?

	if (PRINT_CLEAR == nEntryType)
	{
		bool bWasOpen = m_Stream.IsOpen();

		String strName = m_Stream.GetPath();

		// Close the stream

		m_Stream.Empty();

		// Delete the log file

		DeleteFile(strName);

		// Reopen if was opened

		if (true == bWasOpen)
			Open(strName);

		return;
	}

	// If not writing, return

	if (m_Stream.IsWriting() == false) return;

	try
	{
		// If no text specified, print empty newline

		if (String::IsEmpty(pszText) == true)
		{
			if (true == bLine)
				m_Stream.Write((LPCVOID)L"\r\n", 2 * sizeof(WCHAR));

			return;
		}

		// Start the line

		bool bFirstLinePrefixed = m_bAddingToLine ? true : false;

		m_bAddingToLine = !bLine;

		// Parse and write the text

		LPCWSTR pszStart = pszText;
		int nLen = 0;

		for(;; pszText++)
		{
			if (L'\r' == *pszText ||
				L'\n' == *pszText ||
				L'\0' == *pszText)
			{
				if (false == m_bAddingToLine	&& true == bLine)
				{
					if (true == m_bEnableDateTime)
					{
						// If date/time enabled, write date/time

						WCHAR szDateTime[64] = {0};

						time_t ttCurTime;
						time(&ttCurTime);

						wcsftime(szDateTime, 32, L"%m/%d/%Y %I:%M:%S %p",
							localtime(&ttCurTime));

						if (m_bEnablePrefix)
							wcscat_s(szDateTime, 64, L" ");
						else
							wcscat_s(szDateTime, 64, L": ");

						m_Stream.Write((LPCVOID)szDateTime,
							DWORD(wcslen(szDateTime) * sizeof(WCHAR)));
					}

					// If prefix enabled, write prefix

					if (true == m_bEnablePrefix && nEntryType > PRINT_MESSAGE)
					{
						m_Stream.Write((LPCVOID)SZ_PREFIXES[int(nEntryType) - 1],
							DWORD(wcslen(SZ_PREFIXES[int(nEntryType) - 1]) * sizeof(WCHAR)));

						m_Stream.Write((LPCVOID)L"> : ", 2 * sizeof(WCHAR));

						bFirstLinePrefixed = true;
					}
				}

				// Found end of line - write this line

				nLen = int(pszText - pszStart);

				m_Stream.Write((LPCVOID)pszStart, nLen * sizeof(WCHAR));

				if (true == bLine)
				{
					// Write newline

					m_Stream.Write((LPCVOID)L"\r\n", 2 * sizeof(WCHAR));
				}
				else
				{
					bLine = true;
				}

				// Skip \r in text

				if (L'\r' == *pszText) pszText++;

				pszStart = pszText + 1;
			}

			if (L'\0' == *pszText) break;
		}
	}
	
	catch(Error& rError)
	{
		if (m_pErrorContext != NULL)
			m_pErrorContext->Push(rError);

		throw m_pErrorContext->Push(Error::FILE_WRITE,
			__FUNCTIONW__, m_Stream.GetPath());
	}
}

void LogFile::Empty(void)
{
	m_Stream.Empty();
}

/*----------------------------------------------------------*\
| LogFileHTML implementation
\*----------------------------------------------------------*/

LogFileHTML::LogFileHTML(ErrorManager* m_pErrorStack,
						 bool bEnableDateTime):
						 LogFile(m_pErrorStack, bEnableDateTime, false),
						 m_bBackgroundFixed(false),
						 m_bBackgroundRepeat(true),
						 m_bBackgroundCenter(false),
						 m_rgbBackColor(0xFFFFFFFF),
						 m_strHTMLBackColor(L"#FFFFFF"),
						 m_strFontName(L"Courier New"),
						 m_nFontSize(3)
{
	for(int n = 0; n < PRINT_CLEAR; n++)
	{
		m_arColors[n] = 0;
		m_arHTMLColors[n] = L"#000000";
	}
}

LogFileHTML::~LogFileHTML(void)
{
	Empty();
}

COLORREF LogFileHTML::GetBackColor(void) const
{
	return m_rgbBackColor;
}

void LogFileHTML::SetBackColor(COLORREF rgbColor)
{
	m_rgbBackColor = rgbColor;

	ConvertToHTMLColor(rgbColor, m_strHTMLBackColor);
}

const String& LogFileHTML::GetBackground(void) const
{
	return m_strBackImage;
}

void LogFileHTML::SetBackground(LPCWSTR pszBackground)
{
	if (pszBackground && *pszBackground)
		m_strBackImage = pszBackground;
	else
		m_strBackImage.Empty();
}

bool LogFileHTML::GetBackgroundFixed(void) const
{
	return m_bBackgroundFixed;
}

void LogFileHTML::SetBackgroundFixed(bool bFixed)
{
	m_bBackgroundFixed = bFixed;
}

bool LogFileHTML::GetBackgroundRepeat(void) const
{
	return m_bBackgroundRepeat;
}

void LogFileHTML::SetBackgroundRepeat(bool bRepeat)
{
	m_bBackgroundRepeat = bRepeat;
}

bool LogFileHTML::GetBackgroundCenter(void) const
{
	return m_bBackgroundCenter;
}

void LogFileHTML::SetBackgroundCenter(bool bCenter)
{
	m_bBackgroundCenter = bCenter;
}

COLORREF LogFileHTML::GetTextColor(PrintTypes nType) const
{
	_ASSERT(nType >= 0 || nType < PRINT_CLEAR);

	return m_arColors[nType];
}

void LogFileHTML::SetTextColor(PrintTypes nType, COLORREF rgbColor)
{
	_ASSERT(nType >= 0 || nType < PRINT_CLEAR);

	m_arColors[nType] = rgbColor;

	ConvertToHTMLColor(rgbColor, m_arHTMLColors[nType]);
}

const String& LogFileHTML::GetFontName(void) const
{
	return m_strFontName;
}

void LogFileHTML::SetFontName(LPCWSTR pszFontName)
{
	m_strFontName = pszFontName;
}

int LogFileHTML::GetFontSize(void) const
{
	return m_nFontSize;
}

void LogFileHTML::SetFontSize(int nFontSize)
{
	m_nFontSize = nFontSize;
}

void LogFileHTML::ConvertToHTMLColor(COLORREF rgbColor, String& strDest)
{
	const BYTE* pbColorValues = (BYTE*)&rgbColor;

	strDest.Format(L"#%.2X%.2X%.2X", int(pbColorValues[2]),
		int(pbColorValues[1]), int(pbColorValues[0]));
}

void LogFileHTML::Open(LPCWSTR pszPath)
{
	try
	{
		// Open the file and keep it open

		m_Stream.Open(pszPath, GENERIC_WRITE, OPEN_ALWAYS);
	}
	
	catch(Error& rError)
	{
		if (m_pErrorContext != NULL)
			m_pErrorContext->Push(rError);

		throw m_pErrorContext->Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	if (m_Stream.GetSize() == 0)
	{
		// If the file has zero length, add header

		String strHeader;

		if (m_strBackImage.IsEmpty() == true)
		{
			strHeader.Format(SZ_LOGHTMLHEADERCOLOR, m_strHTMLBackColor,
				m_strFontName, m_nFontSize);
		}
		else
		{
			String strStyle = L"";

			if (false == m_bBackgroundRepeat)
			{
				strStyle += L"background-repeat: no-repeat";

				if (m_bBackgroundCenter) strStyle += L"; ";
			}

			if (true == m_bBackgroundCenter)
			{
				strStyle += L"background-position: top center";

				if (m_bBackgroundFixed) strStyle += L"; ";
			}

			if (true == m_bBackgroundFixed)
				strStyle += L"background-attachment: fixed";

			if (PathIsRelative(m_strBackImage) == TRUE)
			{
				strHeader.Format(SZ_LOGHTMLHEADERIMAGE, m_strHTMLBackColor,
					m_strBackImage, strStyle, m_strFontName, m_nFontSize);
			}
			else
			{
				String strFullPath = L"file://";
				strFullPath += m_strBackImage;

				strHeader.Format(SZ_LOGHTMLHEADERIMAGE, m_strHTMLBackColor,
					strFullPath, strStyle, m_strFontName, m_nFontSize);
			}
		}

		try
		{
			m_Stream.Write((LPCVOID)strHeader.GetBufferConst(),
				strHeader.GetLengthBytes());
		}
		
		catch(Error& rError)
		{
			if (m_pErrorContext != NULL)
				m_pErrorContext->Push(rError);

			throw m_pErrorContext->Push(Error::FILE_WRITE, __FUNCTIONW__, pszPath);
		}
	}
	else
	{
		// Otherwise, set to the end of file and overwrite the HTML footer

		m_Stream.SetPosition(-int(sizeof(SZ_LOGHTMLFOOTER) - sizeof(WCHAR)),
			Stream::MOVE_END);
	}
}

void LogFileHTML::Print(LPCWSTR pszText,
						PrintTypes nEntryType,
						bool bLine)
{
	// Clearing?

	if (PRINT_CLEAR == nEntryType)
	{
		LogFile::Print(NULL, PRINT_CLEAR, false);
		return;
	}

	// If not writing, return

	if (m_Stream.IsWriting() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
	}

	try
	{
		// If no text specified and line can be printed, print empty line

		if (String::IsEmpty(pszText) == true)
		{
			if (true == bLine)
			{
				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_OPEN,
					sizeof(SZ_LOGHTMLENTRYSTART_OPEN) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)m_arHTMLColors[nEntryType].GetBufferConst(),
					m_arHTMLColors[nEntryType].GetLengthBytes());

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_CLOSE,
					sizeof(SZ_LOGHTMLENTRYSTART_CLOSE) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYEND,
					sizeof(SZ_LOGHTMLENTRYEND) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYENDLINE,
					sizeof(SZ_LOGHTMLENTRYENDLINE) - sizeof(WCHAR));
			}

			return;
		}

		// Parse and write the text

		LPCWSTR pszStart = pszText;
		int nLen = 0;

		for(;; pszText++)
		{
			if (L'\r' == *pszText ||
				L'\n' == *pszText ||
				L'\0' == *pszText)
			{
				// Start the line

				if (false == m_bAddingToLine &&
				   true == m_bEnableDateTime)
				{
					// If date/time enabled and not adding to line, write date/time

					WCHAR szDateTime[64] = {0};

					time_t ttCurTime;
					time(&ttCurTime);

					wcsftime(szDateTime, 64, L"%m/%d/%Y %I:%M:%S %p",
						localtime(&ttCurTime));

					wcscat_s(szDateTime, 64, L": ");

					m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_OPEN,
						sizeof(SZ_LOGHTMLENTRYSTART_OPEN) - sizeof(WCHAR));

					m_Stream.Write((LPCVOID)m_arHTMLColors[PRINT_ECHO].GetBufferConst(),
						m_arHTMLColors[PRINT_ECHO].GetLengthBytes());

					m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_CLOSE,
						sizeof(SZ_LOGHTMLENTRYSTART_CLOSE) - sizeof(WCHAR));

					m_Stream.Write((LPCVOID)szDateTime,
						DWORD(wcslen(szDateTime) * sizeof(WCHAR)));

					m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYEND,
						sizeof(SZ_LOGHTMLENTRYEND) - sizeof(WCHAR));
				}

				// Write header

				if (false == m_bEnableDateTime)
					m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSPACE,
						sizeof(SZ_LOGHTMLENTRYSPACE) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_OPEN,
					sizeof(SZ_LOGHTMLENTRYSTART_OPEN) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)m_arHTMLColors[nEntryType].GetBufferConst(),
					m_arHTMLColors[nEntryType].GetLengthBytes());

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYSTART_CLOSE,
					sizeof(SZ_LOGHTMLENTRYSTART_CLOSE) - sizeof(WCHAR));

				nLen = int(pszText - pszStart);

				if (nLen > 0)
					m_Stream.Write((LPCVOID)pszStart, nLen * sizeof(WCHAR));

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYEND,
					sizeof(SZ_LOGHTMLENTRYEND) - sizeof(WCHAR));

				m_Stream.Write((LPCVOID)SZ_LOGHTMLENTRYENDLINE,
					sizeof(SZ_LOGHTMLENTRYENDLINE) - sizeof(WCHAR));

				// Skip \r in text

				if (L'\r' == *pszText)
					pszText++;

				pszStart = pszText + 1;
			}

			if (L'\0' == *pszText)
				break;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_WRITE,
			__FUNCTIONW__, m_Stream.GetPath());
		else
			throw Error(Error::FILE_WRITE,
			__FUNCTIONW__, m_Stream.GetPath());
	}
}

void LogFileHTML::Empty(void)
{
	// Print HTML footer

	try
	{
		if (m_Stream.IsOpen() == true)
			m_Stream.Write((LPCVOID)SZ_LOGHTMLFOOTER,
			sizeof(SZ_LOGHTMLFOOTER) - sizeof(WCHAR));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_WRITE,
				__FUNCTIONW__, m_Stream.GetPath());
		else
			throw Error(Error::FILE_WRITE, __FUNCTIONW__, m_Stream.GetPath());
	}

	LogFile::Empty();
}