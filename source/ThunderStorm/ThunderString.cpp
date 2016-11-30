/*------------------------------------------------------------------*\
|
| ThunderString.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm string implementation
| Created: 11/01/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderError.h"		// using Error, defining String
#include "ThunderStream.h"		// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR String::SZ_NULLSTRINGW[]	= L"(null)";
const char String::SZ_NULLSTRINGA[]		= "(null)";


/*----------------------------------------------------------*\
| String definition
\*----------------------------------------------------------*/

String::String(LPCWSTR pszInit): m_psz(NULL)
{
	if (pszInit != NULL)
		m_psz = _wcsdup(pszInit);
}

String::String(const String& strInit): m_psz(NULL)
{
	if (strInit.IsEmpty() == false)
		m_psz = _wcsdup(strInit);
}

String::String(LPCSTR pszInit): m_psz(NULL)
{
	if (NULL == pszInit)
		return;

	int nLen = int(strlen(pszInit));

	m_psz = reinterpret_cast<LPWSTR>(calloc(nLen + 1, sizeof(WCHAR)));

	if (NULL == m_psz)
		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, (nLen + 1) * sizeof(WCHAR));

	mbstowcs(m_psz, pszInit, nLen);

	m_psz[nLen] = L'\0';
}

String::~String(void)
{
	Empty();
}

String& String::operator=(LPCWSTR pszAssign)
{
	if (pszAssign == m_psz)
		return *this;

	Empty();

	if (pszAssign != NULL) m_psz = _wcsdup(pszAssign);

	return *this;
}

String& String::operator=(const String& strAssign)
{
	Empty();

	if (strAssign.IsEmpty() == false)
		m_psz = _wcsdup(strAssign.m_psz);

	return *this;
}

String& String::operator=(LPCSTR pszAssign)
{
	Empty();

	if (pszAssign != NULL)
	{
		int nLen = int(strlen(pszAssign));

		m_psz = reinterpret_cast<LPWSTR>(calloc(nLen + 1, sizeof(WCHAR)));

		if (NULL == m_psz)
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
				(nLen + 1) * sizeof(WCHAR));

		mbstowcs(m_psz, pszAssign, nLen);

		m_psz[nLen] = L'\0';
	}

	return *this;
}

bool String::operator==(LPCWSTR pszCompare) const
{
	if (NULL == m_psz || NULL == pszCompare)
	{
		if (m_psz == pszCompare)
			return true;
		else
			return false;
	}

	return (wcscmp(m_psz, pszCompare) == 0);
}

bool String::operator==(const String& strCompare) const
{
	if (NULL == m_psz || NULL == strCompare.m_psz)
	{
		if (m_psz == strCompare.m_psz)
			return true;
		else
			return false;
	}

	return (wcscmp(m_psz, strCompare.m_psz) == 0);
}

bool String::operator!=(LPCWSTR pszCompare) const
{
	if (NULL == m_psz || NULL == pszCompare)
	{
		if (m_psz == pszCompare)
			return false;
		else
			return true;
	}

	return (wcscmp(m_psz, pszCompare) != 0);
}

bool String::operator!=(const String& strCompare) const
{
	if (NULL == m_psz || NULL == strCompare.m_psz)
	{
		if (m_psz == strCompare.m_psz)
			return false;
		else
			return true;
	}


	return (wcscmp(m_psz, strCompare.m_psz) != 0);
}

bool String::operator>(LPCWSTR pszCompare) const
{
	if (NULL == m_psz || NULL == pszCompare)
	{
		if (m_psz == pszCompare)
			return false;
		else
			return (m_psz > pszCompare);
	}


	return (wcscmp(m_psz, pszCompare) > 0);
}

bool String::operator>(const String& strCompare) const
{
	if (NULL == m_psz || NULL == strCompare.m_psz)
	{
		if (m_psz == strCompare.m_psz)
			return false;
		else
			return (m_psz > strCompare.m_psz);
	}

	return (wcscmp(m_psz, strCompare.m_psz) > 0);
}

bool String::operator<(LPCWSTR pszCompare) const
{
	if (NULL == m_psz || NULL == pszCompare)
	{
		if (m_psz == pszCompare)
			return false;
		else
			return (m_psz < pszCompare);
	}

	return (wcscmp(m_psz, pszCompare) < 0);
}

bool String::operator<(const String& strCompare) const
{
	if (NULL == m_psz || NULL == strCompare.m_psz)
	{
		if (m_psz == strCompare.m_psz)
			return false;
		else
			return (m_psz < strCompare.m_psz);
	}

	return (wcscmp(m_psz, strCompare.m_psz) < 0);
}

void String::operator+=(LPCWSTR pszAppend)
{
	if (NULL == pszAppend || L'\0' == *pszAppend)
		return;

	if (NULL == m_psz)
	{
		m_psz = _wcsdup(pszAppend);
	}
	else
	{
		int nTargetLen = int(wcslen(m_psz) + wcslen(pszAppend) + 1);
		
		Reallocate(nTargetLen);
		
		wcscat_s(m_psz, nTargetLen, pszAppend);
	}
}

void String::operator+=(const String& strAppend)
{
	if (strAppend.IsEmpty() == true) return;

	if (NULL == m_psz)
	{
		m_psz = _wcsdup(strAppend.m_psz);
	}
	else
	{
		int nTargetLen = int(wcslen(m_psz) + wcslen(strAppend.m_psz) + 1);

		Reallocate(nTargetLen);

		wcscat_s(m_psz, nTargetLen, strAppend.m_psz);
	}
}

String String::operator+(LPCWSTR pszAppend)
{
	String strNew = *this;
	strNew += pszAppend;

	return strNew;
}

String String::operator+(const String& strAppend)
{
	String strNew = *this;
	strNew += strAppend;

	return strNew;
}

LPWSTR String::GetBuffer(void)
{
	return m_psz;
}

LPCWSTR String::GetBufferConst(void) const
{
	return m_psz;
}

String::operator LPCWSTR(void) const
{
	return m_psz;
}

String::operator LPWSTR(void)
{
	return m_psz;
}

LPWSTR String::Allocate(int nLength)
{
	Empty();

	m_psz = (LPWSTR)malloc((size_t)(nLength + 1) * sizeof(WCHAR));
	
	if (NULL == m_psz)
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
			(nLength + 1) * sizeof(WCHAR));

	return m_psz;
}

LPWSTR String::Reallocate(int nLength)
{
	m_psz = (LPWSTR)realloc((LPVOID)m_psz, (size_t)(nLength + 1) * sizeof(WCHAR));
	
	if (NULL == m_psz)
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
			(nLength + 1) * sizeof(WCHAR));

	return m_psz;
}

void String::Empty(void)
{
	if (m_psz != NULL)
	{
		free(m_psz);
		m_psz = NULL;
	}
}

void String::Attach(LPWSTR pszBuffer)
{
	Empty();

	m_psz = pszBuffer;
}

void String::Detach(void)
{
	m_psz = NULL;
}

String& String::ToLower(void)
{
	_wcslwr(m_psz);

	return *this;
}

String& String::ToUpper(void)
{
	_wcsupr(m_psz);

	return *this;
}

int String::GetLength(void) const
{
	return (m_psz != NULL ? int(wcslen(m_psz)) : 0);
}

int String::GetLengthBytes(void) const
{
	return (m_psz != NULL ? int(wcslen(m_psz)) * sizeof(WCHAR) : 0);
}

bool String::IsEmpty(void) const
{
	return (NULL == m_psz || L'\0' == *m_psz);
}

String String::Substring(int nStart, int nLen) const
{
	String str;

	if (-1 == nLen)
	{
		nLen = GetLength() - nStart;

		if (nLen < 0) return str;
	}
	else if (nStart + nLen >= GetLength())
	{
		return str;
	}

	str.Allocate(nLen + 1);
	str.CopyToBuffer(nLen, m_psz + nStart, nLen);

	return str;
}

String String::Right(int nLen) const
{
	if (nLen < 1 || NULL == m_psz)
		return String();

	int nRealLen = int(wcslen(m_psz));

	if (nLen > nRealLen)
		nLen = nRealLen;

	return String(&m_psz[nRealLen - nLen]);
}

String String::Left(int nLen) const
{
	if (nLen < 1 || NULL == m_psz)
		return String();

	int nRealLen = int(wcslen(m_psz));

	if (nLen > nRealLen)
		nLen = nRealLen;

	String str;

	str.Allocate(nLen);
	str.CopyToBuffer(nLen, m_psz, nLen);

	return str;
}

int String::ReverseFind(WCHAR ch) const
{
	LPWSTR pszFind = m_psz + wcslen(m_psz);

	while(*--pszFind != L'\0')
	{
		if (*pszFind == ch)
			return int(pszFind - m_psz);
	}

	return -1;
}

int String::Find(WCHAR ch) const
{
	LPWSTR pszFind = m_psz;

	while(*pszFind++)
	{
		if (*pszFind == ch)
			return int(pszFind - m_psz);
	}

	return -1;
}

LPCWSTR String::Find(LPCWSTR psz) const
{
	return wcsstr(m_psz, psz);
}

int String::LoadString(HINSTANCE hInstance, UINT uID)
{
	Empty();

	LPWSTR pszReadOnly = NULL;

	int nLen = ::LoadString(hInstance, uID, (LPWSTR)&pszReadOnly, 0);

	Allocate(nLen);

	CopyToBuffer(nLen, pszReadOnly, nLen);

	return nLen;
}

int String::Format(LPCWSTR pszFormat, ...)
{
	va_list pArgs;
	va_start(pArgs, pszFormat);

	int nRet = Format(pszFormat, pArgs);

	va_end(pArgs);

	return nRet;
}

int String::Format(HINSTANCE hInstance, UINT uFormat, ...)
{
	va_list pArgs;
	va_start(pArgs, uFormat);

	int nRet = Format(hInstance, uFormat, pArgs);

	va_end(pArgs);

	return nRet;
}

int String::Format(LPCWSTR pszFormat, va_list pArgs)
{
	int nEstimate = FormatLength(pszFormat, pArgs);	

	if (nEstimate == -1) return -1;

	if (!Reallocate(nEstimate)) return -1;

	return vswprintf_s(m_psz, nEstimate + 1, pszFormat, pArgs);
}

int String::Format(HINSTANCE hInstance, UINT uFormat, va_list pArgs)
{
	String strFormat;
	strFormat.LoadString(hInstance, uFormat);

	int nFormatLength = FormatLength(strFormat, pArgs);
	Reallocate(nFormatLength);

	int nRet = vswprintf_s(m_psz, nFormatLength, strFormat, pArgs);

	return nRet;
}

int String::LoadString(HINSTANCE hInstance, UINT uID, LPWSTR& pszOut)
{
	LPWSTR pszReadOnly = NULL;

	int nLength = ::LoadString(hInstance, uID, pszReadOnly, 0);

	if (nLength)
		pszOut = _wcsdup(pszReadOnly);

	return nLength;
}

int String::FormatLength(LPCWSTR pszFormat, va_list va)
{
	int nMaxLen = 0;
	int nSkip = 0;

	for(; *pszFormat != L'\0'; pszFormat++)
	{
		if (L'%' == *pszFormat)
		{
			// If %%, skip

			if (L'%' == pszFormat[1])
			{
				nSkip++;
				nMaxLen++;
				continue;
			}

			if (nSkip != 0)
			{
				nSkip--;
				nMaxLen++;
				continue;
			}

			// Skip to flag characters

			pszFormat++;

			// Process flag characters and skip past them

			for(; *pszFormat != L'\0'; pszFormat++)
			{
				if (L'#' == *pszFormat)
					nMaxLen += 2;
				else if (L'+' == *pszFormat || L' ' == *pszFormat)
					nMaxLen++;
				else if (L'-' == *pszFormat)
					continue;
				else
					break;
			}

			// Process width and skip past it

			int nWidth = _wtoi(pszFormat);

			while(iswdigit(*pszFormat)) pszFormat++;

			if (L'\0' == *pszFormat || nWidth < 0) return -1;

			// Process precision and skip past it

			int nPrecision = 0;

			if (L'.' == *pszFormat)
			{
				pszFormat++;

				nPrecision = _wtoi(pszFormat);

				if (nPrecision < 0) return -1;

				while(iswdigit(*pszFormat))
					pszFormat++;
			}

			// Process any modifiers and skip past them
			// 1 if "h", 2 if "l" (L), 4 if int64

			int nModifier = 0; 

			if (L'h' == *pszFormat)
			{
				nModifier = 1;
				pszFormat++;
			}
			else if (L'l' == *pszFormat)
			{
				nModifier = 2;
				pszFormat++;
			}
			else if (wcsncmp(pszFormat, L"I64", 3) == 0)
			{
				nModifier = 3;
				pszFormat++;
			}

			// Process the specifier and skip past

			int nItemLen = 0;
			
			switch(int(*pszFormat))
			{
			case L'c':
			case L'C':
				{
					nItemLen = 2;

					switch(nModifier)
					{
					case 0:						
						va_arg(va, WCHAR);
						break;
					case 1:
						va_arg(va, char);
						break;
					case 2:
						va_arg(va, wchar_t);
						break;
					}
				}
				break;
			case L's':
				{
					switch(nModifier)
					{
					case 0:
						{
							LPCWSTR pszArg = va_arg(va, LPCWSTR);

							nItemLen = pszArg ?
								max(1, int(wcslen(pszArg))) :
								sizeof(SZ_NULLSTRINGW) / sizeof(WCHAR);
						}
						break;
					case 1:
						{
							LPCSTR pszArg = va_arg(va, LPCSTR);

							nItemLen = pszArg ?
								max(1, int(strlen(pszArg))) :
								sizeof(SZ_NULLSTRINGA) / sizeof(char);
						}
						break;
					case 2:
						{
							LPWSTR pszArg = va_arg(va, LPWSTR);
							
							nItemLen = pszArg ?
								max(1, int(wcslen(pszArg))) :
								sizeof(SZ_NULLSTRINGW) / sizeof(WCHAR);
						}
						break;
					}
				}
				break;
			case L'S':
				{
					switch(nModifier)
					{
					case 0:
						{
#ifdef _UNICODE
							LPWSTR pszArg = va_arg(va, LPWSTR);

							nItemLen = pszArg ?
								max(1, int(wcslen(pszArg))) :
								sizeof(SZ_NULLSTRINGW) / sizeof(WCHAR);
#else
							LPCSTR pszArg = va_arg(va, LPCSTR);

							nItemLen = pszArg ?
								max(1, int(strlen(pszArg))):
								sizeof(SZ_NULLSTRINGA) / sizeof(char);
#endif
						}
						break;
					case 1:
						{
							LPCSTR pszArg = va_arg(va, LPCSTR);

							nItemLen = pszArg ?
								max(1, int(strlen(pszArg))):
								sizeof(SZ_NULLSTRINGA) / sizeof(char);
						}
						break;
					case 2:
						{
							LPWSTR pszArg = va_arg(va, LPWSTR);
							
							nItemLen = pszArg ?
								max(1, int(wcslen(pszArg))) :
								sizeof(SZ_NULLSTRINGW) / sizeof(WCHAR);
						}
						break;
					}
				}
				break;
			}

			if (nItemLen > 0)
			{
				// Adjust item length for strings

				if (nPrecision > 0)
					nItemLen = min(nItemLen, nPrecision);

				nItemLen = max(nItemLen, nWidth);
			}
			else
			{
				// Intrical specifiers...

				switch(int(*pszFormat))
				{
				case L'd':
				case L'i':
				case L'u':
				case L'x':
				case L'X':
				case L'o':
					if (4 == nModifier)
						va_arg(va, __int64);
					else
						va_arg(va, int);

					nItemLen = max(32, nWidth + nPrecision);
					break;
				case L'e':
				case L'g':
				case L'G':
					va_arg(va, double);
					nItemLen = max(128, nWidth + nPrecision);
					break;
				case L'f':
					{
						va_arg(va, double);
						nItemLen = max(nWidth, nPrecision + 312 + 6);
					}
					break;
				case L'p':
					va_arg(va, void*);
					nItemLen = max(32, nWidth + nPrecision);
					break;
				case L'n':
					va_arg(va, int);
					break;
				default:
					return -1;
					break;
				}
			}

			nMaxLen += nItemLen;
		}
		else
		{
			nMaxLen++;
		}
	}

	return nMaxLen;
}

int String::FormatLength(HINSTANCE hInstance, UINT uFormat, va_list va)
{
	String strFormat;
	strFormat.LoadString(hInstance, uFormat);

	return FormatLength(strFormat, va);
}

void String::CopyToBuffer(int nBufferLen, LPCWSTR pszSrc, int nSrcLen)
{
	wcsncpy_s(m_psz, nBufferLen + 1, pszSrc, nSrcLen);
	
	if (nBufferLen > nSrcLen)
		m_psz[nSrcLen] = L'\0';
	else
		m_psz[nBufferLen] = L'\0';
}

char* String::ToAsciiAllocate(void) const
{
	if (NULL == m_psz)
		return NULL;

	int nLen = int(wcslen(m_psz));

	char* psz = reinterpret_cast<char*>(malloc(nLen + 1));

	if (NULL == psz)
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, nLen + 1);

	wcstombs(psz, m_psz, static_cast<size_t>(nLen));

	psz[nLen] = '\0';

	return psz;
}

void String::Serialize(Stream& rStream) const
{
	// Write length of data

	int nLen = GetLength();

	rStream.WriteVar(&nLen);

	// Write data

	if (nLen)
		rStream.Write((LPCVOID)m_psz, nLen * sizeof(WCHAR));
}

void String::Deserialize(Stream& rStream)
{
	// Read length of data

	int nLen = 0;

	rStream.ReadVar(&nLen);

	// Read data

	Allocate(nLen);

	rStream.Read((LPVOID)m_psz, nLen * sizeof(WCHAR));

	// Null-terminate

	m_psz[nLen] = L'\0';
}