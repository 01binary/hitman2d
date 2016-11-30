/*------------------------------------------------------------------*\
|
| ThunderStringTable.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm string table file format implementation
| Created: 07/02/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderStringTable.h"		// defining StringTable
#include "ThunderEngine.h"			// using Engine

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR SZ_STRINGTABLE[] = L"stringtable";


/*----------------------------------------------------------*\
| StringTable implementation
\*----------------------------------------------------------*/

StringTable::StringTable(Engine& rEngine): Resource(rEngine)
{
}

StringTable::~StringTable(void)
{
	Empty();
}

LPCWSTR StringTable::GetString(LPCWSTR pszName) const
{
	StringMapConstIterator pos = m_mapEntries.find(pszName);

	if (pos == m_mapEntries.end()) return NULL;

	return pos->second;
}

LPCWSTR StringTable::GetString(int nID) const
{
	// Strings are referenced by string key, so convert int key to string key

	WCHAR szName[16] = {0};
	_itow_s(nID, szName, 16, 10);

	return GetString(szName);
}

StringMapConstIterator StringTable::GetFirstStringPosConst(void) const
{
	return m_mapEntries.begin();
}

StringMapConstIterator StringTable::GetLastStringPosConst(void) const
{
	return m_mapEntries.end();
}

void StringTable::Deserialize(LPCWSTR pszPath)
{
	Empty();

	// Note: to eliminate the overhead of InfoFile, the following code
	//       is a specialized text format parser, but the format still
	//       starts with a root element.

	// Open the text file

	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(pszPath, GENERIC_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	// Make sure it's a unicode text file

	if (stream.IsUnicodeTextFile() == false)
		throw m_rEngine.GetErrors().Push(Error::FILE_NOTUNICODE, __FUNCTIONW__, stream.GetPath());

	// Read the whole file into a buffer (null terminated)

	stream.CreateReadBuffer(stream.GetSize() + 1);

	// Clear current data and set new name

	Empty();

	m_strName = pszPath;

	// Parse the buffer and load strings into memory

	LPCWSTR pszBuffer = (LPCWSTR)stream.GetReadBufferConst();
	LPCWSTR psz = pszBuffer;

	String strEntryName;
	String strEntryValue;

	bool bReadRoot = false;

	for(;;)
	{
		// Skip any comments, white space and newlines

		while(EatWhiteSpaceNewline(&psz) > 0 || EatComment(&psz) > 0);

		// Stop reading if reached the end of file

		if (L'\0' == *psz || L'}' == *psz) break;

		// What follows must be entry name/identifier, mark it

		LPCWSTR pszStart = psz;

		// Skip all valid name characters

		while(IsValidNameChar(*psz) == true) psz++;

		// Read the entry name

		int nLen = int(psz - pszStart);

		if (nLen <= 0)
		{
			// Invalid format

			int nLine, nColumn;
			GetStringPosition(pszBuffer, pszStart, nLine, nColumn);

			throw m_rEngine.GetErrors().Push(Error::FILE_PARSE, __FUNCTIONW__,
				pszPath, nLine, nColumn, L"entry name");
		}

		strEntryName.Reallocate(nLen);

		strEntryName.CopyToBuffer(nLen, pszStart, nLen);

		// If the entry name is "stringtable", we must have read the root element

		if (strEntryName == SZ_STRINGTABLE)
		{
			bReadRoot = true;

			// Skip any comments, white space and newlines before file start marker

			while(EatWhiteSpaceNewline(&psz) > 0 || EatComment(&psz) > 0);

			// If file start marker not found, format error

			if (L'{' != *psz)
			{
				int nLine, nColumn;
				GetStringPosition(pszBuffer, psz, nLine, nColumn);

				throw m_rEngine.GetErrors().Push(Error::FILE_PARSE,
					__FUNCTIONW__, pszPath, nLine, nColumn, L"{");
			}

			// Skip past the file start marker

			psz++;
		}
		else
		{
			// Skip any comments, whitespaces and newlines
			// before the value start marker

			while(EatWhiteSpaceNewline(&psz) > 0 || EatComment(&psz) > 0);

			// What follows must be value start marker

			if (*psz != L'\"')
			{
				// Format error

				int nLine, nColumn;
				GetStringPosition(pszBuffer, psz, nLine, nColumn);

				throw m_rEngine.GetErrors().Push(Error::FILE_PARSE,
					__FUNCTIONW__, pszPath, nLine, nColumn, L"\"");
			}

			// If root not yet read, parse error

			if (false == bReadRoot)
				throw m_rEngine.GetErrors().Push(Error::FILE_PARSE,
					__FUNCTIONW__, pszPath, 1, 1, L"root element");

			pszStart = ++psz;

			// Find value end marker, ignoring any \" escape sequences

			while(psz != NULL)
			{
				psz = wcschr(psz, L'\"');

				if (L'\\' != psz[-1])
					break;
				else
					psz++;
			}

			if (NULL == psz)
			{
				// Parse error

				int nLine, nColumn;
				GetStringPosition(pszBuffer, pszStart, nLine, nColumn);

				throw m_rEngine.GetErrors().Push(Error::FILE_PARSE,
					__FUNCTIONW__, pszPath, nLine, nColumn, L"\"");
			}

			// Allocate space for value

			nLen = int(psz - pszStart);

			try
			{
				strEntryValue.Reallocate(nLen);
			}

			catch(Error& rError)
			{
				UNREFERENCED_PARAMETER(rError);

				throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
					__FUNCTIONW__, (nLen + 1) * sizeof(WCHAR));
			}

			// Parse value

			LPWSTR pszValue = strEntryValue.GetBuffer();

			for(; pszStart < psz; pszStart++)
			{
				if (L'\\' == *pszStart)
				{
					// Potential escape character

					switch(pszStart[1])
					{				
					case L'n':
						*pszValue++ = L'\n';
						break;
					case L'r':
						*pszValue++ = L'\r';
						break;
					case L'\\':
						*pszValue++ = L'\\';
						break;
					case L'\"':
						*pszValue++ = L'\"';
						break;
					case L't':
						*pszValue++ = L'\t';
						break;
					}

					pszStart++;
				}
				else
				{
					// Any other character except for tabs, which are ignored

					if (*pszStart != L'\t')
						*pszValue++ = *pszStart;
				}
			}

			// Null-terminate value

			*pszValue = L'\0';

			// Skip value end marker

			psz++;

			// Add entry to index

			m_mapEntries[strEntryName] = strEntryValue;
		}
	}
}

DWORD StringTable::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(StringTable);

	for(StringMapConstIterator pos = m_mapEntries.begin();
		pos != m_mapEntries.end();
		pos++)
	{
		dwSize += DWORD(pos->first.GetLengthBytes()) +
			DWORD(pos->second.GetLengthBytes());
	}

	return dwSize;
}

void StringTable::Empty(void)
{
	m_mapEntries.clear();
}

void StringTable::Remove(void)
{
	m_rEngine.GetStrings().Remove(this);
}