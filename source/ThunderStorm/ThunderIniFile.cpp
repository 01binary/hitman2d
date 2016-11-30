/*------------------------------------------------------------------*\
|
| ThunderIniFile.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm profile reading/writing class implementation
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderIniFile.h"		// defining IniFile/Section/Key
#include "ThunderStream.h"		// using Stream
#include "ThunderError.h"		// using ErrorManager
#include "ThunderGlobals.h"		// using IsValidNameChar

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| IniFile implementation
\*----------------------------------------------------------*/

IniFile::IniFile(ErrorManager* pErrorContext): m_pErrorContext(pErrorContext)
{
}

IniFile::~IniFile(void)
{
	Empty();
}

const String& IniFile::GetPath(void) const
{
	return m_strPath;
}

const String& IniFile::GetTitle(void) const
{
	return m_strTitle;
}

IniSection* IniFile::AddSection(LPCWSTR pszName)
{
	// Make sure this section does not already exist, return it if it does

	IniSection* pSection = GetSection(pszName);
	if (pSection != NULL) return pSection;

	// Return if invalid name

	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	// Create a new section

	try
	{
		pSection = new IniSection(*this, pszName);
	}

	catch(std::bad_alloc)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniSection));
		else
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniSection));
	}

	// Add it

	m_mapSections[pszName] = pSection;
	m_arSections.push_back(pSection);

	return pSection;
}

IniSection* IniFile::GetSection(LPCWSTR pszName)
{
	// Return if invalid name

	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	// Return if not found

	IniSectionMapIterator posFind = m_mapSections.find(pszName);
	if (posFind == m_mapSections.end()) return NULL;

	// Otherwise, return the section

	return posFind->second;
}

const IniSection* IniFile::GetSectionConst(LPCWSTR pszName) const
{
	// Return if invalid name

	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	// Return if not found

	IniSectionMapConstIterator posFind = m_mapSections.find(pszName);
	if (posFind == m_mapSections.end()) return NULL;

	// Otherwise, return the section

	return posFind->second;
}

void IniFile::RemoveSection(LPCWSTR pszName)
{
	// Return if invalid name

	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	// Return if not found

	IniSectionMapIterator posFind = m_mapSections.find(pszName);
	if (posFind == m_mapSections.end()) return;

	// Otherwise, deallocate and remove this section

	IniSection* pSectionFound = posFind->second;	

	for(IniSectionArrayIterator pos = m_arSections.begin();
		pos != m_arSections.end();
		pos++)
	{
		if ((*pos) == pSectionFound)
		{
			m_arSections.erase(pos);
			break;
		}
	}

	delete pSectionFound;
	m_mapSections.erase(posFind);
}

void IniFile::RemoveAllSections(void)
{
	for(IniSectionArrayIterator pos = m_arSections.begin();
		pos != m_arSections.end();
		pos++)
	{
		delete *pos;
	}

	m_arSections.clear();
	m_mapSections.clear();
}

int IniFile::GetSectionCount(void) const
{
	return int(m_arSections.size());
}

IniSectionArrayIterator IniFile::GetFirstSectionPos(void)
{
	return m_arSections.begin();
}

IniSectionArrayIterator IniFile::GetLastSectionPos(void)
{
	return m_arSections.end();
}

void IniFile::SetStringKeyValue(LPCWSTR pszSection,
								LPCWSTR pszKey,
								LPCWSTR pszValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetStringKeyValue(pszKey, pszValue);
}

void IniFile::SetIntKeyValue(LPCWSTR pszSection,
							 LPCWSTR pszKey,
							 int nValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetIntKeyValue(pszKey, nValue);
}

void IniFile::SetFloatKeyValue(LPCWSTR pszSection,
							   LPCWSTR pszKey,
							   float fValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetFloatKeyValue(pszKey, fValue);
}

void IniFile::SetBoolKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  bool bValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetBoolKeyValue(pszKey, bValue);
}

void IniFile::SetDwordKeyValue(LPCWSTR pszSection,
							   LPCWSTR pszKey,
							   DWORD dwValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetDwordKeyValue(pszKey, dwValue);
}

void IniFile::SetEnumKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  int nEnumValueIndex,
							  const LPCWSTR* ppszEnumValueNames,
							  int nEnumCount,
							  LPCWSTR pszDefault)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetEnumKeyValue(pszKey, nEnumValueIndex,
		ppszEnumValueNames, nEnumCount, pszDefault);
}

void IniFile::SetEnumKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  int nEnumValue,
							  const LPCWSTR* ppszEnumValueNames,
							  const int* pnEnumValues,
							  int nEnumCount,
							  LPCWSTR pszDefault)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetEnumKeyValue(pszKey, nEnumValue,
		ppszEnumValueNames, pnEnumValues, nEnumCount, pszDefault);
}

void IniFile::SetEnumKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  DWORD dwEnumValue,
							  const LPCWSTR* ppszEnumValueNames,
							  const DWORD* pdwEnumValues,
							  int nEnumCount,
							  LPCWSTR pszDefault)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetEnumKeyValue(pszKey, dwEnumValue, ppszEnumValueNames,
		pdwEnumValues, nEnumCount, pszDefault);
}

void IniFile::SetEnumKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  LPCWSTR pszValue)
{
	IniSection* pSection = GetSection(pszSection);

	if (NULL == pSection)
		pSection = AddSection(pszSection);

	pSection->SetEnumKeyValue(pszKey, pszValue);
}

LPCWSTR IniFile::GetStringKeyValue(LPCWSTR pszSection,
								   LPCWSTR pszKey,
								   LPCWSTR pszDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetStringKeyValue(pszKey, pszDefault) : pszDefault;
}

int IniFile::GetIntKeyValue(LPCWSTR pszSection,
							LPCWSTR pszKey,
							int nDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetIntKeyValue(pszKey, nDefault) : nDefault;
}

float IniFile::GetFloatKeyValue(LPCWSTR pszSection,
								LPCWSTR pszKey,
								float fDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetFloatKeyValue(pszKey, fDefault) : fDefault;
}

bool IniFile::GetBoolKeyValue(LPCWSTR pszSection,
							  LPCWSTR pszKey,
							  bool bDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetBoolKeyValue(pszKey, bDefault) : bDefault;
}

DWORD IniFile::GetDwordKeyValue(LPCWSTR pszSection,
								LPCWSTR pszKey,
								DWORD dwDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetDwordKeyValue(pszKey, dwDefault) : dwDefault;
}

int IniFile::GetEnumKeyValue(LPCWSTR pszSection,
							 LPCWSTR pszKey,
							 const LPCWSTR* ppszEnumValueNames,
							 int nEnumCount,
							 int nDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetEnumKeyValue(pszKey, ppszEnumValueNames,
			nEnumCount, nDefault) : nDefault;
}

int IniFile::GetEnumKeyValue(LPCWSTR pszSection,
							 LPCWSTR pszKey,
							 const LPCWSTR* ppszEnumValueNames,
							 const int* pnEnumValues,
							 int nEnumCount,
							 int nDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetEnumKeyValue(pszKey, ppszEnumValueNames,
			pnEnumValues, nEnumCount, nDefault) : nDefault;
}

DWORD IniFile::GetEnumKeyValue(LPCWSTR pszSection,
							   LPCWSTR pszKey,
							   const LPCWSTR* ppszEnumValueNames,
							   const DWORD* pdwEnumValues,
							   int nEnumCount,
							   DWORD dwDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetEnumKeyValue(pszKey, ppszEnumValueNames,
			pdwEnumValues, nEnumCount, dwDefault) : dwDefault;
}

LPCWSTR IniFile::GetEnumKeyValue(LPCWSTR pszSection,
								 LPCWSTR pszKey,
								 LPCWSTR pszDefault) const
{
	const IniSection* pSection = GetSectionConst(pszSection);

	return pSection != NULL ?
		pSection->GetEnumKeyValue(pszKey, pszDefault) : pszDefault;
}

void IniFile::Serialize(LPCWSTR pszPath) const
{
	// Open the file

	Stream stream(m_pErrorContext);

	try
	{
		stream.Open(pszPath, GENERIC_WRITE, CREATE_ALWAYS);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_OPEN,
				__FUNCTIONW__, pszPath);
		else
			throw Error(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	// Serialize into it

	Serialize(stream);
}

void IniFile::Deserialize(LPCWSTR pszPath)
{
	// Open the file

	Stream stream(m_pErrorContext);

	try
	{
		stream.Open(pszPath, GENERIC_READ, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_OPEN,
				__FUNCTIONW__, pszPath);
		else
			throw Error(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	// Remember path and title

	m_strPath = pszPath;
	m_strTitle = stream.GetTitle();

	// Deserialize from it

	Deserialize(stream);
}

void IniFile::Serialize(Stream& rStream) const
{
	try
	{
		// Write the unicode signature

		rStream.WriteUnicodeSignature();

		// Write all sections

		if (m_arSections.size() == 0) return;

		IniSectionArrayConstIterator posBeforeLast = --m_arSections.end();

		for(IniSectionArrayConstIterator pos = m_arSections.begin();
			pos != m_arSections.end();
			pos++)
		{
			// Write section name start marker

			rStream.Write((LPCVOID)L"[", sizeof(WCHAR));

			// Write section name

			rStream.Write((LPCVOID)(*pos)->GetName().GetBufferConst(),
				DWORD((*pos)->GetName().GetLengthBytes()));

			// Write section name end marker

			rStream.Write((LPCVOID)L"]\r\n", sizeof(WCHAR) * 3);

			// Write section

			(*pos)->Serialize(rStream);

			// If not last section, add end section marker

			if (pos != posBeforeLast)
				rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);
		}
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_WRITE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_WRITE, __FUNCTIONW__, rStream.GetPath());
	}
}

void IniFile::Deserialize(Stream& rStream)
{
	// Make sure it's a unicode text file

	if (rStream.IsUnicodeTextFile() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_NOTUNICODE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_NOTUNICODE,
				__FUNCTIONW__, rStream.GetPath());
	}

	// Figure out buffer size to allocate
	// = (totalsize - size of unicode signature (1 WCHAR))

	int nBufferSize = rStream.GetSize() - sizeof(WORD);
	int nBufferLen = nBufferSize / sizeof(WCHAR);

	String strBuffer;

	try
	{
		// Allocate buffer for file contents

		strBuffer.Allocate(nBufferLen);

		// Read file contents into buffer

		rStream.Read((LPVOID)strBuffer.GetBuffer(),
			DWORD(nBufferSize));
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_READ, 
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_READ,
				__FUNCTIONW__, rStream.GetPath());
	}

	// Terminate with null character

	strBuffer.GetBuffer()[nBufferLen] = L'\0';

	// Read sections

	LPCWSTR psz = strBuffer.GetBuffer();

	while(psz != NULL && *psz != L'\0')
	{
		// Skip white space, new lines, and comments before name start

		while(EatWhiteSpaceNewline(&psz) > 0 || EatComment(&psz) > 0);

		// If we are not seeing name start marker, parse error
		
		if (L'[' != *psz)
		{
			int nLine, nColumn;

			GetStringPosition(strBuffer.GetBuffer(), psz, nLine, nColumn);

			if (m_pErrorContext != NULL)
				throw m_pErrorContext->Push(Error::FILE_PARSE, __FUNCTIONW__,
					rStream.GetPath(), nLine, nColumn, L"\"[\"");
			else
				throw Error(Error::FILE_PARSE, __FUNCTIONW__,
					rStream.GetPath(), nLine, nColumn, L"\"[\"");
		}

		// Skip the name start marker and save start name position

		psz++;

		LPCWSTR pszSectionNameStart = psz;

		// Skip over all name chars

		while(IsValidNameChar(*psz)) psz++;

		// If not seeing name end marker, invalid format

		if (*psz != L']')
		{
			int nLine, nColumn;
			GetStringPosition(strBuffer.GetBuffer(), psz, nLine, nColumn);

			if (m_pErrorContext != NULL)
				throw m_pErrorContext->Push(Error::FILE_PARSE,
					__FUNCTIONW__, rStream.GetPath(), nLine, nColumn, L"\"]\"");
			else
				throw Error(Error::FILE_PARSE,
					__FUNCTIONW__, rStream.GetPath(), nLine, nColumn, L"\"]\"");
		}

		// Read the name

		int nSectionNameLen = int(psz - pszSectionNameStart);

		String strSectionName;
		strSectionName.Allocate(nSectionNameLen);

		strSectionName.CopyToBuffer(nSectionNameLen, 
			pszSectionNameStart, nSectionNameLen);

		// Skip past the name end marker

		psz++;

		// Read the section and skip past it

		IniSection* pSection = AddSection(strSectionName);

		int nSkipLen = 0;

		pSection->Deserialize(psz, &nSkipLen);

		psz += nSkipLen;
	}
}

void IniFile::Empty(void)
{
	RemoveAllSections();
}

/*----------------------------------------------------------*\
| IniSection implementation
\*----------------------------------------------------------*/

IniSection::IniSection(IniFile& rIniFile,
					   LPCWSTR pszName):

						m_rIniFile(rIniFile),
						m_strName(pszName)
{
}

IniSection::~IniSection(void)
{
	Empty();
}

IniFile& IniSection::GetIniFile(void)
{
	return m_rIniFile;
}

const IniFile& IniSection::GetIniFileConst(void) const
{
	return m_rIniFile;
}

const String& IniSection::GetName(void) const
{
	return m_strName;
}

void IniSection::SetStringKeyValue(LPCWSTR pszKey, LPCWSTR pszValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_STRING);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value
		
	pKey->SetStringValue(pszValue);
}

void IniSection::SetIntKeyValue(LPCWSTR pszKey, int nValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_INT);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value
		
	pKey->SetIntValue(nValue);
}

void IniSection::SetFloatKeyValue(LPCWSTR pszKey, float fValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_FLOAT);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value
		
	pKey->SetFloatValue(fValue);
}

void IniSection::SetBoolKeyValue(LPCWSTR pszKey, bool bValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_BOOL);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value

	pKey->SetBoolValue(bValue);
}

void IniSection::SetDwordKeyValue(LPCWSTR pszKey, DWORD dwValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_DWORD);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value

	pKey->SetDwordValue(dwValue);
}

void IniSection::SetEnumKeyValue(LPCWSTR pszKey,
								 int nEnumValue,
								 const LPCWSTR* ppszEnumValueNames,
								 int nEnumCount,
								 LPCWSTR pszDefault)
{
	if (NULL == ppszEnumValueNames)
		return;

	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_ENUM);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value

	pKey->SetEnumValue(nEnumValue, ppszEnumValueNames,
		nEnumCount, pszDefault);
}

void IniSection::SetEnumKeyValue(LPCWSTR pszKey,
								 int nEnumValue,
								 const LPCWSTR* ppszEnumValueNames,
								 const int* pnEnumValues,
								 int nEnumCount,
								 LPCWSTR pszDefault)
{
	if (NULL == ppszEnumValueNames || NULL == pnEnumValues)
		return;

	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true) return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_ENUM);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value

	pKey->SetEnumValue(nEnumValue, ppszEnumValueNames,
		pnEnumValues, nEnumCount, pszDefault);
}

void IniSection::SetEnumKeyValue(LPCWSTR pszKey,
								 DWORD dwEnumValue,
								 const LPCWSTR* ppszEnumValueNames,
								 const DWORD* pdwEnumValues,
								 int nEnumCount,
								 LPCWSTR pszDefault)
{
	if (NULL == ppszEnumValueNames || NULL == pdwEnumValues)
		return;

	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_ENUM);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value

	pKey->SetEnumValue(dwEnumValue, ppszEnumValueNames,
		pdwEnumValues, nEnumCount, pszDefault);
}

void IniSection::SetEnumKeyValue(LPCWSTR pszKey, LPCWSTR pszValue)
{
	// Find the key

	IniKey* pKey = GetKey(pszKey);

	// If doesn't exist, create and add
	
	if (NULL == pKey)
	{
		// If doesn't exist because of bad name, return

		if (String::IsEmpty(pszKey) == true)
			return;

		try
		{
			pKey = new IniKey(pszKey, Variable::TYPE_ENUM);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniKey));
		}

		m_mapKeys[pszKey] = pKey;
		m_arKeys.push_back(pKey);
	}

	// Set value
	
	pKey->SetEnumValue(pszValue);
}

LPCWSTR IniSection::GetStringKeyValue(LPCWSTR pszKey,
									  LPCWSTR pszDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || (pKey->GetVarType() != Variable::TYPE_STRING &&
	   pKey->GetVarType() != Variable::TYPE_ENUM))
		return pszDefault;

	// Return key value

	return pKey->GetStringValue();
}

int IniSection::GetIntKeyValue(LPCWSTR pszKey, int nDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || (pKey->GetVarType() != Variable::TYPE_INT &&
		pKey->GetVarType() != Variable::TYPE_DWORD))
			return nDefault;

	// Return key value

	return pKey->GetIntValue();
}

float IniSection::GetFloatKeyValue(LPCWSTR pszKey, float fDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || pKey->GetVarType() != Variable::TYPE_FLOAT)
		return fDefault;

	// Return key value

	return pKey->GetFloatValue();
}

bool IniSection::GetBoolKeyValue(LPCWSTR pszKey, bool bDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || (pKey->GetVarType() != Variable::TYPE_BOOL &&
		pKey->GetVarType() != Variable::TYPE_INT))
			return bDefault;

	// Return key value

	return pKey->GetBoolValue();
}

DWORD IniSection::GetDwordKeyValue(LPCWSTR pszKey, DWORD dwDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || (pKey->GetVarType() != Variable::TYPE_DWORD &&
		pKey->GetVarType() != Variable::TYPE_INT))
			return dwDefault;

	// Return key value
	
	return pKey->GetDwordValue();
}

int IniSection::GetEnumKeyValue(LPCWSTR pszKey,
								const LPCWSTR* ppszEnumValueNames,
								int nEnumCount,
								int nDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || pKey->GetVarType() != Variable::TYPE_ENUM)
		return nDefault;

	// Get the value

	return pKey->GetEnumValue(ppszEnumValueNames, nEnumCount, nDefault);
}

int IniSection::GetEnumKeyValue(LPCWSTR pszKey,
								const LPCWSTR* ppszEnumValueNames,
								const int* pnEnumValues,
								int nEnumCount,
								int nDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || pKey->GetVarType() != Variable::TYPE_ENUM)
		return nDefault;

	// Get the value

	return pKey->GetEnumValue(ppszEnumValueNames, pnEnumValues,
		nEnumCount, nDefault);
}

DWORD IniSection::GetEnumKeyValue(LPCWSTR pszKey,
								  const LPCWSTR* ppszEnumValueNames,
								  const DWORD* pdwEnumValues,
								  int nEnumCount,
								  DWORD dwDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || pKey->GetVarType() != Variable::TYPE_ENUM)
		return dwDefault;

	// Get the value

	return pKey->GetEnumValue(ppszEnumValueNames, pdwEnumValues,
		nEnumCount, dwDefault);
}

LPCWSTR IniSection::GetEnumKeyValue(LPCWSTR pszKey,
									LPCWSTR pszDefault) const
{
	// Find the key

	const IniKey* pKey = GetKeyConst(pszKey);

	// If doesn't exist, return the default value

	if (NULL == pKey || pKey->GetVarType() != Variable::TYPE_ENUM)
		return pszDefault;

	return pKey->GetStringValue();
}

IniKey* IniSection::GetKey(LPCWSTR pszName)
{
	if (String::IsEmpty(pszName))
		return NULL;

	IniKeyMapIterator posFind = m_mapKeys.find(pszName);

	if (posFind == m_mapKeys.end()) return NULL;

	return posFind->second;
}

const IniKey* IniSection::GetKeyConst(LPCWSTR pszName) const
{
	IniKeyMap::const_iterator posFind = m_mapKeys.find(pszName);

	if (posFind == m_mapKeys.end()) return NULL;

	return posFind->second;
}

int IniSection::GetKeyCount(void) const
{
	return int(m_mapKeys.size());
}

void IniSection::RemoveKey(LPCWSTR pszName)
{
	IniKeyMapIterator posFind = m_mapKeys.find(pszName);

	if (posFind == m_mapKeys.end()) return;

	IniKey* pFound = posFind->second;

	m_mapKeys.erase(posFind);

	m_arKeys.erase(std::find(m_arKeys.begin(),
		m_arKeys.end(), pFound));

	delete pFound;	
}

void IniSection::RemoveAllKeys(void)
{
	for(IniKeyArrayIterator pos = m_arKeys.begin();
		pos != m_arKeys.end();
		pos++)
	{
		delete *pos;
	}

	m_arKeys.clear();
	m_mapKeys.clear();
}

IniKeyArrayIterator IniSection::GetFirstKeyPos(void)
{
	return m_arKeys.begin();
}

IniKeyArrayIterator IniSection::GetLastKeyPos(void)
{
	return m_arKeys.end();
}

void IniSection::Serialize(Stream& rStream) const
{
	for(IniKeyArrayConstIterator pos = m_arKeys.begin();
		pos != m_arKeys.end();
		pos++)
	{
		// Write name

		rStream.Write((LPCVOID)(*pos)->GetName().GetBufferConst(),
			DWORD((*pos)->GetName().GetLengthBytes()));

		// Write separator

		rStream.Write((LPCVOID)L" = ", sizeof(WCHAR) * 3);

		// Write value

		String strValue = (*pos)->ToString();
		
		rStream.Write((LPCVOID)strValue.GetBufferConst(),
			DWORD(strValue.GetLengthBytes()));

		// Write end marker

		rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);
	}
}

void IniSection::Deserialize(LPCWSTR pszBuffer, int* pnLengthRead)
{
	// Remove any current data

	Empty();

	// Read keys

	LPCWSTR psz = pszBuffer;

	while(psz)
	{
		// Skip any blank lines and comments
		// TODO: add those back in later on save!!!

		while(EatWhiteSpaceNewline(&psz) || EatComment(&psz));

		// If what follows is a null char, that means
		// we just reached the end of file

		if (*psz == L'\0') break;

		// If what follows is a section marker "[", we back up and return
		// to hand over control to parent so that it starts
		// reading the next section name

		if (*psz == L'[') break;

		// Otherwise, what follows next must be key name, and it ends at separator (=)
		// So find the separator

		LPCWSTR pszNameEnd = wcschr(psz, L'=');

		// If not found, can't parse

		if (NULL == pszNameEnd)
		{
			int nLine, nCol;

			GetStringPosition(pszBuffer, psz, nLine, nCol);

			throw Error(Error::FILE_PARSE, __FUNCTIONW__,
				m_rIniFile.GetPath(), nLine, nCol, L"name/value separator");
		}

		// Remember separator position so we can later
		// use it to find start of value

		LPCWSTR pszValueStart = pszNameEnd + 1;

		// Decrement one character behind the separator

		pszNameEnd--;

		// Decrement over any whitespace to find the trimmed end of value name

		EatWhiteSpace(&pszNameEnd, true);

		// If the value name length is 0 or negative, exit

		int nNameLen = int(pszNameEnd - psz + 1);

		if (nNameLen <= 0)
		{
			int nLine, nCol;

			GetStringPosition(pszBuffer, psz, nLine, nCol);

			throw Error(Error::FILE_PARSE, __FUNCTIONW__,
				m_rIniFile.GetPath(), nLine, nCol, L"value name");
		}

		// Otherwise, read value name into buffer

		String strKeyName;
		strKeyName.Allocate(nNameLen);

		strKeyName.CopyToBuffer(nNameLen, psz, nNameLen);

		// Find the start of value

		EatWhiteSpace(&pszValueStart);

		// Create a new key with undefined value type

		IniKey* pKey = NULL;
		
		try
		{
			pKey = new IniKey(strKeyName);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(IniKey));
		}

		// Read the value

		pKey->FromString(pszValueStart);

		// Add the new key with the read value name

		m_mapKeys[strKeyName] = pKey;
		m_arKeys.push_back(pKey);

		// Set the buffer pointer to point after the end of value

		psz = wcsstr(psz, L"\r\n");
	}

	if (pnLengthRead != NULL)
		*pnLengthRead = int(psz - pszBuffer);
}

void IniSection::Empty(void)
{
	RemoveAllKeys();
}

/*----------------------------------------------------------*\
| IniKey implementation
\*----------------------------------------------------------*/

IniKey::IniKey(LPCWSTR pszName, Variable::Types nVarType): Variable(nVarType),
														   m_strName(pszName)
{
}

IniKey::~IniKey(void)
{
}

const String& IniKey::GetName(void) const
{
	return m_strName;
}