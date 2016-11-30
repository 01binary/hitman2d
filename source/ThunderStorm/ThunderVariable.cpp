/*------------------------------------------------------------------*\
|
| ThunderVariable.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm Variable classes implementation
| Created: 03/06/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderError.h"			// using ErrorManager
#include "ThunderInfoFile.h"		// using InfoFile, InfoElem
#include "ThunderVariable.h"		// defining Variable, VariableManager
#include "ThunderGlobals.h"			// using IsValidNameChar
#include "ThunderStream.h"			// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Variable type strings

const LPCWSTR Variable::SZ_TYPES[] = {
										L"undefined",
										L"int",
										L"dword",
										L"bool",
										L"float",
										L"string",
										L"enum"
									};


/*----------------------------------------------------------*\
| Variable implementation
\*----------------------------------------------------------*/

Variable::Variable(Types nVarType):
				   m_nType(nVarType),
				   m_nValue(0)
{
}

Variable::Variable(const Variable& rInit)
{
	m_nType = rInit.m_nType;

	if (TYPE_STRING == rInit.m_nType || TYPE_ENUM == rInit.m_nType)
		m_pszValue = _wcsdup(rInit.m_pszValue);
	else
		m_dwValue = rInit.m_dwValue;
}

Variable::Variable(int nValue): m_nType(TYPE_INT),
								m_nValue(nValue)
{
}

Variable::Variable(DWORD dwValue): m_nType(TYPE_DWORD),
								   m_dwValue(dwValue)
{
}

Variable::Variable(float fValue): m_nType(TYPE_FLOAT),
								  m_fValue(fValue)
{
}

Variable::Variable(LPCWSTR pszValue): m_nType(TYPE_STRING),
									  m_pszValue(_wcsdup(pszValue))
{
}

Variable::Variable(bool bValue): m_nType(TYPE_BOOL),
								 m_bValue(bValue)
{
}

Variable::~Variable(void)
{
	Empty();
}

void Variable::SetVarType(Types nNewType)
{
	Empty();

	m_nType = nNewType;
}

LPCWSTR Variable::GetVarTypeString(void) const
{
	if (m_nType < TYPE_UNDEFINED || m_nType >= TYPE_COUNT)
		return NULL;

	return SZ_TYPES[m_nType];
}

LPCWSTR Variable::GetVarTypeString(Types nType)
{
	if (nType < TYPE_UNDEFINED || nType >= TYPE_COUNT)
		return NULL;

	return SZ_TYPES[nType];
}

int Variable::GetEnumValue(const LPCWSTR* ppszEnumValueNames,
						   int nEnumCount,
						   int nDefault) const
{
	while(nEnumCount--)
	{
		if (wcscmp(ppszEnumValueNames[nEnumCount], m_pszValue) == 0)
			return nEnumCount;
	}

	return nDefault;
}

int Variable::GetEnumValue(const LPCWSTR* ppszEnumValueNames,
						   const int* pnEnumValues,
						   int nEnumCount,
						   int nDefault) const
{
	while(nEnumCount--)
	{
		if (wcscmp(ppszEnumValueNames[nEnumCount], m_pszValue) == 0)
			return pnEnumValues[nEnumCount];
	}

	return nDefault;
}

DWORD Variable::GetEnumValue(const LPCWSTR* ppszEnumValueNames,
							 const DWORD* pdwEnumValues,
							 int nEnumCount,
							 DWORD dwDefault) const
{
	while(nEnumCount--)
	{
		if (wcscmp(ppszEnumValueNames[nEnumCount], m_pszValue) == 0)
			return pdwEnumValues[nEnumCount];
	}

	return dwDefault;
}

LPCWSTR Variable::GetEnumValue(void) const
{
	return m_pszValue;
}

void Variable::SetIntValue(int nValue)
{
	if (m_nType != TYPE_INT)
		Empty();

	m_nType = TYPE_INT;
	m_nValue = nValue;
}

void Variable::SetDwordValue(DWORD dwValue)
{
	if (m_nType != TYPE_DWORD)
		Empty();

	m_nType = TYPE_DWORD;
	m_dwValue = dwValue;
}

void Variable::SetFloatValue(float fValue)
{
	if (m_nType != TYPE_FLOAT) Empty();

	m_nType = TYPE_FLOAT;
	m_fValue = fValue;
}

void Variable::SetStringValue(LPCWSTR pszValue)
{
	Empty();

	m_nType = TYPE_STRING;
	m_pszValue = _wcsdup(pszValue);
}

void Variable::SetBoolValue(bool bValue)
{
	if (m_nType != TYPE_BOOL) Empty();

	m_nType = TYPE_BOOL;
	m_bValue = bValue;
}

void Variable::SetEnumValue(int nEnumValueIndex,
							const LPCWSTR* ppszEnumValueNames,
							int nEnumCount,
							LPCWSTR pszDefault)
{
	Empty();

	m_nType = TYPE_ENUM;

	if (nEnumValueIndex < 0 || nEnumValueIndex >= nEnumCount)
		m_pszValue = pszDefault ? _wcsdup(pszDefault) : NULL;
	else
		m_pszValue = _wcsdup(ppszEnumValueNames[nEnumValueIndex]);
}

void Variable::SetEnumValue(int nEnumValue,
							const LPCWSTR* ppszEnumValueNames,
							const int* pnEnumValues,
							int nEnumCount,
							LPCWSTR pszDefault)
{
	Empty();

	m_nType = TYPE_ENUM;

	while(nEnumCount--)
	{
		if (nEnumValue == pnEnumValues[nEnumCount])
		{
			m_pszValue = _wcsdup(ppszEnumValueNames[nEnumCount]);
			return;
		}
	}

	m_pszValue = _wcsdup(pszDefault);
}

void Variable::SetEnumValue(DWORD dwEnumValue,
							const LPCWSTR* ppszEnumValueNames,
							const DWORD* pdwEnumValues,
							int nEnumCount,
							LPCWSTR pszDefault)
{
	Empty();

	m_nType = TYPE_ENUM;

	while(nEnumCount--)
	{
		if (dwEnumValue == pdwEnumValues[nEnumCount])
		{
			m_pszValue = _wcsdup(ppszEnumValueNames[nEnumCount]);
			return;
		}
	}

	m_pszValue = _wcsdup(pszDefault);
}

void Variable::SetEnumValue(LPCWSTR pszEnumValueName)
{
	Empty();

	m_nType = TYPE_ENUM;

	m_pszValue = _wcsdup(pszEnumValueName);
}

Variable& Variable::operator=(const Variable& rCopy)
{
	// Assign only the value, don't touch the name

	m_nType = rCopy.m_nType;

	if (rCopy.m_nType == TYPE_STRING || rCopy.m_nType == TYPE_ENUM)
		m_pszValue = _wcsdup(rCopy.m_pszValue);
	else
		m_dwValue = rCopy.m_dwValue;

	return *this;
}

Variable& Variable::operator=(int nValue)
{
	SetIntValue(nValue);

	return *this;
}

Variable& Variable::operator=(DWORD dwValue)
{
	SetDwordValue(dwValue);

	return *this;
}

Variable& Variable::operator=(float fValue)
{
	SetFloatValue(fValue);

	return *this;
}

Variable& Variable::operator=(LPCWSTR pszValue)
{
	SetStringValue(pszValue);

	return *this;
}

bool Variable::operator==(const Variable& rCompare) const
{
	if (m_nType != rCompare.m_nType)
		return false;

	switch(m_nType)
	{
	case TYPE_STRING:
		{
			if (wcscmp(m_pszValue, rCompare.m_pszValue) != 0)
				return false;
		}
		break;
	default:
		{
			if (m_dwValue != rCompare.m_dwValue)
				return false;
		}
		break;
	}

	return true;
}

bool Variable::operator!=(const Variable& rCompare) const
{
	if (m_nType != rCompare.m_nType)
		return true;

	switch(m_nType)
	{
	case TYPE_STRING:
		{
			if (wcscmp(m_pszValue, rCompare.m_pszValue) != 0)
				return true;
		}
		break;
	default:
		{
			if (m_dwValue != rCompare.m_dwValue)
				return true;
		}
		break;
	}

	return false;
}

int Variable::FromString(LPCWSTR psz)
{
	if (String::IsEmpty(psz) == true)
		return 0;

	// Read a variable's value from a string

	// We assume here that \r\n, \n, or space signify
	// the end of variable's value data.
	// Quoted strings (TYPE_STRING) can have these characters in them,
	// but not unquoted ones (TYPE_STRINGCONST).

	switch(m_nType)
	{
	case TYPE_UNDEFINED:
		{
			// Must determine type by the contents

			if (L'\"' == *psz)
			{
				// If starts with a quote, must be a string

				m_nType = TYPE_STRING;
			}
			else if (*psz >= L'0' && *psz <= L'9')
			{
				// Starts with a number, so this is likely a numeric value

				if (L'x' == psz[1])
				{
					// If the second char is x, this is a hex number, so we count it as dword

					m_nType = TYPE_DWORD;
				}
				else
				{
					// See if it's a float - try to find a dot within
					// a contiguous set of digits
					// (floats must start with a digit and must have a dot)

					if ((psz[1] < L'0' || psz[1] > L'9') && psz[1] != L'.')
					{
						// If that digit was the only char, that's an int.

						m_nType = TYPE_INT;
					}
					else
					{					
						LPCWSTR pszTemp = psz + 1;

						bool bOutOfDigits = false;

						while(*pszTemp != L'.')
						{
							if (*pszTemp >= L'0' && *pszTemp <= L'9')
							{
								// Continue if it's a digit

								pszTemp++;
							}
							else
							{
								// If ran out of digits
								// (and still haven't found '.'), exit

								bOutOfDigits = true;
								break;
							}
						}

						if (true == bOutOfDigits)
						{
							if (L'\0' == *pszTemp ||
								IsWhiteSpace(*pszTemp) == true || L',' == *pszTemp)
							{
								// If ran out of digits because
								// encountered end of value, this is an int

								m_nType = TYPE_INT;
							}
							else
							{
								// If ran out of digits because of
								// some other character,
								// this is an enum name that starts with a digit.

								m_nType = TYPE_ENUM;
							}
						}
						else
						{
							// If found dot and not ran out of digits, it's a float
							m_nType = TYPE_FLOAT;
						}
					}
				}
			}
			else if (wcsncmp(psz, L"true", 4) == 0 || wcsncmp(psz, L"false", 5) == 0)
			{
				// If it's exactly "true" or "false", it's a bool.

				m_nType = TYPE_BOOL;
			}
			else
			{
				if (L'-' == *psz && ( iswdigit(psz[1]) ||
					( L'-' == psz[1] && iswdigit(psz[2]) ) ))
				{
					// If the first char is - and the next is a number,
					// it's a negative int!

					m_nType = TYPE_INT;
				}
				else if (L'.' == *psz && iswdigit(psz[1]))
				{
					// If the first char is a dot and the next
					// is a number, it's a float!

					m_nType = TYPE_FLOAT;
				}
				else
				{
					// False alarm, this is just an enum

					m_nType = TYPE_ENUM;
				}
			}

			return FromString(psz);
		}
		break;
	case TYPE_INT:
		{
			swscanf(psz, L"%d", &m_nValue);

			// We need to return the length of this value in chars.
			// So, let's just count all the digits

			LPCWSTR pszTemp = psz;

			if (L'-' == *pszTemp)
			{
				// If starts with minus sign, count that too

				pszTemp++;
			}

			while(*pszTemp >= L'0' && *pszTemp <= L'9')
				pszTemp++;

			return int(pszTemp - psz);
		}
		break;
	case TYPE_DWORD:
		{
			swscanf(psz, L"%x", &m_dwValue);

			// We need to return the length of this value in chars.
			// So, skip '0x' and add 2 if it's present
			// Then count all hex digits

			LPCWSTR pszTemp = psz;

			if (L'0' == pszTemp[0] && L'x' == pszTemp[1])
				pszTemp += 2;

			while((*pszTemp >= L'0' && *pszTemp <= L'9') ||
				(*pszTemp >= L'A' && *pszTemp <= L'F'))
				pszTemp++;

			return int(pszTemp - psz);
		}
		break;
	case TYPE_BOOL:
		{
			if (wcsncmp(psz, L"true", 4) == 0)
			{
				m_bValue = true;

				return 4;
			}
			else if (wcsncmp(psz, L"false", 5) == 0)
			{
				m_bValue = false;

				return 5;
			}
			else
			{
				return 0;
			}
		}
		break;
	case TYPE_FLOAT:
		{
			swscanf(psz, L"%f", &m_fValue);

			// We need to return the length of this value in chars
			// So, count valid digits including dot

			LPCWSTR pszTemp = psz;

			while((*pszTemp >= L'0' && *pszTemp <= L'9') || L'.' == *pszTemp)
				pszTemp++;

			return int(pszTemp - psz);
		}
		break;
	case TYPE_STRING:
		{
			// Quotes around the string are needed!

			if (*psz != L'\"') break;

			LPWSTR pszEnd = wcschr((LPWSTR)&psz[1], L'\"');
			if (NULL == pszEnd) break;

			int nLen = int(pszEnd - psz - 1);

			if (m_pszValue != NULL) delete[] m_pszValue;

			try
			{
				m_pszValue = new WCHAR[nLen + 1];
			}

			catch(std::bad_alloc)
			{
				throw Error(Error::MEM_ALLOC,
					__FUNCTIONW__, (nLen + 1) * sizeof(WCHAR));
			}
	
			wcsncpy(m_pszValue, psz + 1, nLen);

			m_pszValue[nLen] = L'\0';

			// Return the length of string

			return nLen + 2;
		}
		break;
	case TYPE_ENUM:
		{
			// String enum value name, no spaces allowed

			LPCWSTR pszStart = psz;

			LPWSTR pszEnd = (LPWSTR)pszStart;

			while(IsValidNameChar(*pszEnd) == true)
				pszEnd++;

			int nLen = int(pszEnd - pszStart);

			if (m_pszValue != NULL)
				delete[] m_pszValue;

			m_pszValue = new WCHAR[nLen + 1];

			wcsncpy(m_pszValue, pszStart, nLen);

			m_pszValue[nLen] = L'\0';

			// Return the length of string

			return nLen;
		}
		break;
	default:
		{
			// If we get to here, the variable type is invalid.
			// Probably as a result of corrupted memory,
			// or an idiot programmer (or both!)
			
			_ASSERT(false);
		}
		break;
	}

	return 0;
}

String Variable::ToString(void) const
{
	String str;

	switch(m_nType)
	{
	case TYPE_INT:
		{
			str.Format(L"%d", m_nValue);
		}
		break;
	case TYPE_DWORD:
		{
			str.Format(L"0x%X", m_dwValue);
		}
		break;
	case TYPE_BOOL:
		{
			str = m_bValue ? L"true" : L"false";
		}
		break;
	case TYPE_FLOAT:
		{
			str.Format(L"%f", m_fValue);

			// Remove trailing nulls, because they are ugly and take up space

			for(LPWSTR psz = str.GetBuffer() + str.GetLength() - 1;
				L'0' == *psz && L'.' != psz[-1];
				*psz-- = L'\0');
		}
		break;
	case TYPE_STRING:
		{
			if (m_pszValue != NULL)
			{
				int nLen = int(wcslen(m_pszValue) + 2);

				try
				{
					str.Allocate(nLen);
				}

				catch(Error& rError)
				{
					UNREFERENCED_PARAMETER(rError);

					throw Error(Error::MEM_ALLOC,
						__FUNCTIONW__, (nLen + 1) * sizeof(WCHAR));
				}

				wcscpy_s(str.GetBuffer(), nLen + 1, L"\"");
				wcscat_s(str.GetBuffer(), nLen + 1, m_pszValue);
				wcscat_s(str.GetBuffer(), nLen + 1, L"\"");
			}
			else
			{				
				str = L"\"(null)\"";
			}
		}
		break;
	case TYPE_ENUM:
		{
			if (m_pszValue != NULL)
			{
				str = m_pszValue;
			}
			else
			{
				str = L"(null)";
			}
		}
		break;
	}

	return str;
}

void Variable::Serialize(Stream& rStream) const
{
	// Write type (as BYTE)

	rStream.WriteVar((LPBYTE)&m_nType);

	// Write data

	if (TYPE_STRING == m_nType)
	{
		// If string, write as string

		String str(m_pszValue);
		str.Serialize(rStream);
	}
	else
	{
		// Otherwise it's a 32-bit value
		// Write as integer no matter what it is

		rStream.WriteVar(&m_nValue);
	}
}

void Variable::Deserialize(Stream& rStream)
{
	// Read type (as BYTE)

	rStream.ReadVar((LPBYTE)&m_nType);

	// Read data

	if (TYPE_STRING == m_nType)
	{
		// If string, read as string

		String str;
		str.Deserialize(rStream);

		m_pszValue = str.GetBuffer();

		str.Detach();
	}
	else
	{
		// Otherwse it's a 32-bit value
		// Read as integer no matter what it is

		rStream.ReadVar(&m_nValue);
	}
}

void Variable::Serialize(InfoElem& rRoot) const
{
	if (TYPE_UNDEFINED == m_nType)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	switch(m_nType)
	{
	case TYPE_INT:
		rRoot.SetIntValue(m_nValue);
		break;
	case TYPE_DWORD:
		rRoot.SetDwordValue(m_dwValue);
		break;
	case TYPE_BOOL:
		rRoot.SetBoolValue(m_bValue);
		break;
	case TYPE_FLOAT:
		rRoot.SetFloatValue(m_fValue);
		break;
	case TYPE_STRING:
		rRoot.SetStringValue(m_pszValue);
		break;
	case TYPE_ENUM:
		rRoot.SetEnumValue(m_pszValue);
		break;
	}
}

void Variable::Deserialize(const InfoElem& rRoot)
{
	*this = rRoot;
}

DWORD Variable::GetMemoryFootprint(void) const
{
	DWORD dwSize = sizeof(Variable);
	
	if ((m_nType == TYPE_STRING || m_nType == TYPE_ENUM) && m_pszValue)
		dwSize += DWORD(wcslen(m_pszValue));

	return dwSize;
}

void Variable::Empty(void)
{
	if ((TYPE_STRING == m_nType ||
		TYPE_ENUM == m_nType)
		&& m_pszValue != NULL)
	{
		// If string, deallocate

		delete[] m_pszValue;
	}
	
	// Zero all bits of the value

	m_nValue = 0;

	// Reset type

	m_nType = TYPE_UNDEFINED;
}

/*----------------------------------------------------------*\
| VariableManager implementation
\*----------------------------------------------------------*/

VariableManager::VariableManager(ErrorManager* pErrorContext):
								m_pErrorContext(NULL)
{
}

VariableManager::~VariableManager(void)
{
	Empty();
}

Variable* VariableManager::Add(LPCWSTR pszName, Variable::Types nType)
{
	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	try
	{
		Variable* pVariable = new Variable(nType);

		m_mapVars[pszName] = pVariable;

		return pVariable;
	}
	
	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(Variable));
	}
}

Variable* VariableManager::Find(LPCWSTR pszName)
{
	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	VariableMapIterator posFound = m_mapVars.find(pszName);

	if (posFound == m_mapVars.end()) return NULL;

	return posFound->second;
}

const Variable* VariableManager::FindConst(LPCWSTR pszName) const
{
	if (String::IsEmpty(pszName) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	VariableMapConstIterator posFound = m_mapVars.find(pszName);

	return (posFound == m_mapVars.end() ? NULL : posFound->second);
}

void VariableManager::Remove(LPCWSTR pszName)
{
	VariableMapIterator posFound = m_mapVars.find(pszName);

	if (posFound != m_mapVars.end())
	{
		delete posFound->second;
		m_mapVars.erase(posFound);
	}
}

int VariableManager::GetCount(void) const
{
	return int(m_mapVars.size());
}

void VariableManager::RemoveAll(void)
{
	// Remove and deallocate

	for(VariableMapIterator pos = m_mapVars.begin();
		pos != m_mapVars.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapVars.clear();
}

VariableMapIterator VariableManager::GetBeginPos(void)
{
	return m_mapVars.begin();
}

VariableMapIterator VariableManager::GetEndPos(void)
{
	return m_mapVars.end();
}

VariableMapConstIterator VariableManager::GetBeginPosConst(void) const
{
	return m_mapVars.begin();
}

VariableMapConstIterator VariableManager::GetEndPosConst(void) const
{
	return m_mapVars.end();
}

void VariableManager::Serialize(Stream& rStream) const
{
	try
	{
		// Write variable count

		int nCount = int(m_mapVars.size());
		rStream.WriteVar(&nCount);

		// Write variables

		for(VariableMapConstIterator pos = m_mapVars.begin();
			pos != m_mapVars.end();
			pos++)
		{
			// Write name

			pos->first.Serialize(rStream);

			// Write type and data

			pos->second->Serialize(rStream);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_SERIALIZE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_SERIALIZE, __FUNCTIONW__, rStream.GetPath());
	}
}

void VariableManager::Deserialize(Stream& rStream)
{
	try
	{
		int nCount = 0;
		String strName;

		// Clear current vars

		Empty();

		// Read variable count

		rStream.ReadVar(&nCount);

		// Read variables

		for(int n = 0; n < nCount; n++)
		{
			// Create variable

			Variable* pVar = NULL;
			
			try
			{
				pVar = new Variable;
			}

			catch(std::bad_alloc)
			{
				if (m_pErrorContext != NULL)
					throw m_pErrorContext->Push(Error::MEM_ALLOC,
						__FUNCTIONW__, sizeof(Variable));
				else
					throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Variable));
			}

			// Read name

			strName.Deserialize(rStream);

			// Read type and data

			pVar->Deserialize(rStream);

			m_mapVars[strName] = pVar;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_DESERIALIZE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_DESERIALIZE, __FUNCTIONW__, rStream.GetPath());
	}
}

DWORD VariableManager::GetMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(VariableMapConstIterator pos = m_mapVars.begin();
		pos != m_mapVars.end();
		pos++)
	{
		dwSize += pos->second->GetMemoryFootprint();
	}

	return dwSize;
}

void VariableManager::Empty(void)
{
	RemoveAll();
}