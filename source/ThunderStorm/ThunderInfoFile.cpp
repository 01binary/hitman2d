/*------------------------------------------------------------------*\
|
| ThunderInfoFile.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm Markup Language reader/writer implementation
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderGlobals.h"		// using string functions
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// defining InfoFile/Element

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| InfoFile implementation
\*----------------------------------------------------------*/

InfoFile::InfoFile(ErrorManager* pErrorContext):
				   m_pRoot(NULL),
				   m_pErrorContext(pErrorContext),
				   m_pszTempData(NULL)
{
}

InfoFile::~InfoFile(void)
{
	Empty();
}

void InfoFile::Empty(void)
{
	if (m_pRoot != NULL)
	{
		delete m_pRoot;
		m_pRoot = NULL;
	}
}

InfoElem* InfoFile::CreateSetRoot(LPCWSTR pszName, bool bHasValue)
{
	try
	{
		m_pRoot = new InfoElem(*this, NULL, pszName,
			bHasValue ? InfoElem::TYPE_VALUEBLOCK : InfoElem::TYPE_BLOCK);
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(InfoElem));
	}

	return m_pRoot;
}

void InfoFile::Serialize(LPCWSTR pszPath) const
{
	Stream stream(m_pErrorContext);

	try
	{
		stream.Open(pszPath, GENERIC_WRITE, CREATE_NEW);
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

	Serialize(stream);
}

void InfoFile::Deserialize(LPCWSTR pszPath)
{
	Stream stream(m_pErrorContext);

	try
	{
		stream.Open(pszPath, GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
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

	Deserialize(stream);
}

void InfoFile::Serialize(Stream& rStream) const
{
	// Write the root

	if (m_pRoot != NULL)
		m_pRoot->Serialize(rStream);
}

void InfoFile::Deserialize(Stream& rStream)
{
	// Update path and title

	m_strPath = rStream.GetPath();
	m_strTitle = rStream.GetTitle();

	// Must be unicode text format

	if (rStream.IsUnicodeTextFile() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_NOTUNICODE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_NOTUNICODE,
				__FUNCTIONW__, rStream.GetPath());
	}
	
	try
	{
		// Create a read buffer

		m_pszTempData = reinterpret_cast<LPCWSTR>(rStream.CreateReadBuffer(
			rStream.GetSize() + 2));

		// Clean up previous root if any

		delete m_pRoot;

		// Create a new root

		CreateSetRoot(NULL, false);

		// Read the root element

		m_pRoot->Deserialize(m_pszTempData);

		m_pszTempData = NULL;
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_DESERIALIZE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_DESERIALIZE,
				__FUNCTIONW__, rStream.GetPath());
	}
}

/*----------------------------------------------------------*\
| InfoElem implementation
\*----------------------------------------------------------*/

InfoElem::InfoElem(InfoFile& rDocument,
				   InfoElem* pParent,
				   LPCWSTR pszName,
				   InfoElemTypes nElemType,
				   Variable::Types nVarType):

				   Variable(nVarType),
				   m_rDocument(rDocument),
				   m_pParent(pParent),
				   m_nElemType(nElemType),
				   m_strName(pszName)
{
	m_arChildren.reserve(8);
}

InfoElem::InfoElem(const InfoElem& rInit):
				   Variable(rInit.GetVarType()),
				   m_rDocument(rInit.m_rDocument),
				   m_pParent(rInit.m_pParent),
				   m_nElemType(rInit.m_nElemType),
				   m_strName(rInit.m_strName)

{
	for(InfoElemConstIterator pos = rInit.GetBeginChildPosConst();
		pos != rInit.GetEndChildPosConst();
		pos++)
	{
		AddChild(new InfoElem(**pos));
	}
}

void InfoElem::SetElemType(InfoElemTypes nElemType)
{
	if (TYPE_BLOCK == m_nElemType || TYPE_VALUEBLOCK == m_nElemType)
	{
		if (TYPE_VALUELIST == nElemType)
		{
			// Clear names and children of all children

			for(InfoElemIterator pos = GetBeginChildPos();
				pos != GetEndChildPos();
				pos++)
			{
				(*pos)->Empty();
			}
		}
		else if (TYPE_VALUE == nElemType)
		{
			// Clear all children

			RemoveAllChildren();
		}
	}

	if (TYPE_VALUEBLOCK == m_nElemType && TYPE_BLOCK == nElemType)
		Variable::Empty();

	m_nElemType = nElemType;
}

InfoElem* InfoElem::CreateChild(LPCWSTR pszName,
								InfoElemTypes nElemType,
								Variable::Types nVarType)
{
	// Allocate new child

	try
	{
		InfoElem* pNewElem = new InfoElem(m_rDocument, this,
			pszName, nElemType, nVarType);

		return pNewElem;
	}

	catch(std::bad_alloc e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(InfoElem));
	}
}

void InfoElem::AddChild(InfoElem* pChild)
{
	// Validate parent

	_ASSERT(pChild->m_pParent == this);

	// Add to list

	m_arChildren.push_back(pChild);

	// Add to map
	
	m_mapChildren.insert(
		std::pair<String, InfoElem*>(pChild->GetName(), pChild));
}

InfoElem* InfoElem::AddChild(LPCWSTR pszName,
							 InfoElemTypes nElemType,
							 Variable::Types nVarType)
{
	// Allocate new child

	InfoElem* pNewElem = NULL;

	try
	{
		pNewElem = new InfoElem(m_rDocument, this,
			pszName, nElemType, nVarType);
	}

	catch(std::bad_alloc e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(InfoElem));
	}

	// Add to list

	m_arChildren.push_back(pNewElem);

	// Add to map

	m_mapChildren.insert(
		std::pair<String, InfoElem*>(pNewElem->GetName(), pNewElem));

	return pNewElem;
}

void InfoElem::RemoveChild(InfoElem* pChild, bool bDeallocate)
{
	// Remove from list

	for(InfoElemIterator pos = m_arChildren.begin();
		pos != m_arChildren.end();
		pos++)
	{
		if (*pos == pChild)
		{
			m_arChildren.erase(pos);
			break;
		}
	}
	
	// Remove from map

	InfoElemRange range =
		m_mapChildren.equal_range(pChild->GetName());

	for(InfoElemRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		if (pos->second == pChild)
		{
			m_mapChildren.erase(pos);
			break;
		}
	}

	// Deallocate if specified

	if (true == bDeallocate)
		delete pChild;
}

void InfoElem::RemoveChild(InfoElemIterator posChild, bool bDeallocate)
{
#ifdef _DEBUG
	_ASSERT(posChild != m_arChildren.end());
#endif

	InfoElem* pChild = *posChild;

	// Remove from list

	m_arChildren.erase(posChild);

	// Remove from map

	InfoElemRange range = m_mapChildren.equal_range(pChild->GetName());

	for(InfoElemRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		if (pos->second == pChild)
		{
			m_mapChildren.erase(pos);
			break;
		}
	}

	// Deallocate if specified

	if (true == bDeallocate)
		delete pChild;
}

void InfoElem::RemoveAllChildren(void)
{
	// Deallocate all children in the list

	for(InfoElemIterator pos = m_arChildren.begin();
		pos != m_arChildren.end();
		pos++)
	{
		delete *pos;
	}

	// Clear list and map

	m_arChildren.clear();
	m_mapChildren.clear();
}

InfoElem* InfoElem::FindChild(LPCWSTR pszName,
							  InfoElemTypes nElemType,
							  Variable::Types nVarType,
							  bool bRecursive)
{
	InfoElemRange range = m_mapChildren.equal_range(pszName);

	if (range.first == range.second)
	{
		if (true == bRecursive)
		{
			for(InfoElemIterator posChild = m_arChildren.begin();
				posChild != m_arChildren.end();
				posChild++)
			{
				InfoElem* pFound = (*posChild)->FindChild(pszName, nElemType,
					nVarType, true);

				if (pFound != NULL)
					return pFound;
			}
		}

		return NULL;
	}

	for(InfoElemRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		if (nElemType != InfoElem::TYPE_ANY && pos->second->m_nElemType != nElemType)
			continue;

		if (nVarType != Variable::TYPE_UNDEFINED && pos->second->m_nType == nVarType)
			continue;

		return pos->second;
	}

	return NULL;
}

const InfoElem* InfoElem::FindChildConst(LPCWSTR pszName,
										 InfoElemTypes nElemType, 
										 Variable::Types nVarType,
										 bool bRecursive) const
{
	InfoElemConstRange range = m_mapChildren.equal_range(pszName);

	if (range.first == range.second)
	{
		if (true == bRecursive)
		{
			for(InfoElemConstIterator posChild = m_arChildren.begin();
				posChild != m_arChildren.end();
				posChild++)
			{
				InfoElem* pFound = (*posChild)->FindChild(pszName, nElemType,
					nVarType, true);

				if (pFound != NULL)
					return pFound;
			}
		}

		return NULL;
	}

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		if (nElemType != InfoElem::TYPE_ANY && pos->second->m_nElemType != nElemType)
			continue;

		if (nVarType != Variable::TYPE_UNDEFINED && pos->second->m_nType != nVarType)
			continue;

		return pos->second;
	}

	return NULL;
}

InfoElemRange InfoElem::FindChildren(LPCWSTR pszName, bool bRecursive)
{
	InfoElemRange range = m_mapChildren.equal_range(pszName);

	if (range.first != range.second)
		return range;

	for(InfoElemIterator pos = m_arChildren.begin();
		pos != m_arChildren.end();
		pos++)
	{
		range = (*pos)->FindChildren(pszName, true);

		if (range.first != m_mapChildren.end())
			break;
	}

	return range;
}

InfoElemConstRange InfoElem::FindChildrenConst(LPCWSTR pszName, bool bRecursive) const
{
	InfoElemConstRange range = m_mapChildren.equal_range(pszName);

	if (range.first != range.second)
		return range;

	if (true == bRecursive)
	{
		for(InfoElemConstIterator pos = m_arChildren.begin();
			pos != m_arChildren.end();
			pos++)
		{
			range = (*pos)->FindChildrenConst(pszName, true);

			if (range.first != m_mapChildren.end())
				break;
		}
	}

	return range;
}

void InfoElem::Deserialize(LPCWSTR pszData, int* pnLengthRead)
{
	// We begin reading an element by skipping over any
	// comments or blank lines until we encounter a character
	// that could be in element name

	LPCWSTR pszCur = pszData;
	LPCWSTR pszStart = NULL;

	while(EatWhiteSpaceNewline(&pszCur) > 0 ||
		EatComment(&pszCur) > 0);

	// What follows must be this element's name.
	// So we mark the start of element's name.

	pszStart = pszCur;

	// Then we skip through all the characters that
	// can be present in the element's name...

	while(IsValidNameChar(*pszCur) == true) pszCur++;

	// Now we jumped over the element's name.
	// Mark the end of element's name, and copy it.

	int nNameLen = int(pszCur - pszStart);

	if (nNameLen <= 0)
	{
		// Invalid format

		int nLine, nColumn;

		GetStringPosition(m_rDocument.m_pszTempData, pszCur, nLine, nColumn);

		if (m_rDocument.m_pErrorContext != NULL)
			throw m_rDocument.m_pErrorContext->Push(Error::FILE_PARSE,
				__FUNCTIONW__, m_rDocument.GetPath(),
				nLine, nColumn, L"element name");
		else
			throw Error(Error::FILE_PARSE, __FUNCTIONW__,
				m_rDocument.GetPath(), nLine, nColumn, L"element name");
	}

	m_strName.Allocate(nNameLen);
	m_strName.CopyToBuffer(nNameLen, pszStart, nNameLen);

	if (L':' == *pszCur)
	{
		// If what follows is ':', this is a value element.

		// Skip the ':'

		pszCur++;

		// Skip any white space, new lines, or comments

		while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);

		// Read the first value

		int nValueLen = FromString(pszCur);

		// If failed, invalid format

		if (0 == nValueLen)
		{
			int nLine, nColumn;

			GetStringPosition(m_rDocument.m_pszTempData,
				pszCur, nLine, nColumn);

			if (GetDocumentConst().m_pErrorContext != NULL)
				throw GetDocumentConst().m_pErrorContext->Push(Error::FILE_PARSE,
					__FUNCTIONW__, GetDocumentConst().GetPath(),
					nLine, nColumn, L"element value");
			else
				throw Error(Error::FILE_PARSE, __FUNCTIONW__,
					GetDocumentConst().GetPath(), nLine,
					nColumn, L"element value");
		}

		// Skip over value data

		pszCur += nValueLen;

		// Skip any white space and newlines as well as comments

		while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);

		// If what follows is a value separator, this is a value list

		if (L',' == *pszCur)
		{
			m_nElemType = TYPE_VALUELIST;

			// Turn the first value into a child

			AddChild(NULL, TYPE_VALUE)->FromVariable(GetValue());

			// Clear the type of this element
			
			Variable::Empty();

			// Read other values as children as well

			for(;;)
			{
				// Skip the ','

				pszCur++;

				// Skip any white space and newlines as well as
				// comments before the value

				while(EatWhiteSpaceNewline(&pszCur) > 0 ||
					EatComment(&pszCur) > 0);

				// Create a new child

				InfoElem* pChild = CreateChild();

				// Attempt to parse the value string

				nValueLen = pChild->FromString(pszCur);

				if (nValueLen > 0)
				{
					// If parsed successfully, add child

					AddChild(pChild);

					// Skip the value string

					pszCur += nValueLen;
				}
				else
				{
					// Otherwise, destroy this child and fail

					delete pChild;

					int nLine, nColumn;

					GetStringPosition(m_rDocument.m_pszTempData,
						pszCur, nLine, nColumn);

					if (GetDocumentConst().m_pErrorContext != NULL)
						throw GetDocumentConst().m_pErrorContext->Push(Error::FILE_PARSE,
							__FUNCTIONW__, GetDocumentConst().GetPath(),
							nLine, nColumn, L"element value");
					else
						throw Error(Error::FILE_PARSE, __FUNCTIONW__,
							GetDocumentConst().GetPath(), nLine,
							nColumn, L"element value");
				}				

				// Skip any white space and new lines as well as
				// comments after the value

				while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);

				// If what follows is not a value separator, stop reading values

				if (L',' != *pszCur) break;
			}			
		}
		else
		{
			// This is a simple single-value element, not a value list
			// Plus, we already read the first value (which turns out to be the only one)
			// so we are done with the values

			m_nElemType = TYPE_VALUE;
		}
	}
	else
	{
		// When we get to here, this must be a block type element
		// So we skip whitespace, newlines and comments and see what's next

		while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);
		
		if (*pszCur != L'{')
		{
			// If we don't get a '{', it's a value-block element,
			// meaning the value goes first, then the block

			m_nElemType = TYPE_VALUEBLOCK;

			// Read the value

			int nValueLen = FromString(pszCur);

			// Skip over value data

			pszCur += nValueLen;

			// Skip any white space and comments after value data

			while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);

			// If still no '{', invalid format

			if (*pszCur != L'{')
			{
				int nLine, nColumn;

				GetStringPosition(m_rDocument.m_pszTempData, pszCur,
					nLine, nColumn);

				if (GetDocumentConst().m_pErrorContext != NULL)
					throw GetDocumentConst().m_pErrorContext->Push(Error::FILE_PARSE,
						__FUNCTIONW__, GetDocumentConst().GetPath(),
						nLine, nColumn, L"\"{\"");
				else
					throw Error(Error::FILE_PARSE, __FUNCTIONW__,
						GetDocumentConst().GetPath(), nLine, nColumn, L"\"{\"");
			}
		}
		else
		{
			// If found '{' immediately, it's just a simple block element

			m_nElemType = TYPE_BLOCK;
		}

		// Skip over the '{' and start reading children until we encounter '}'

		pszCur++;

		for(;;)
		{
			// Skip any white space, newlines or comments

			while(EatWhiteSpaceNewline(&pszCur) > 0 || EatComment(&pszCur) > 0);

			// If found end of string, invalid format

			if (L'\0' == *pszCur)
			{
				int nLine, nColumn;

				GetStringPosition(m_rDocument.m_pszTempData,
					pszCur - 1, nLine, nColumn);

				if (GetDocumentConst().m_pErrorContext != NULL)
					throw GetDocumentConst().m_pErrorContext->Push(Error::FILE_PARSE,
						__FUNCTIONW__, GetDocumentConst().GetPath(),
						nLine, nColumn, L"element name");
				else
					throw Error(Error::FILE_PARSE, __FUNCTIONW__,
						GetDocumentConst().GetPath(), nLine, nColumn, L"element name");
			}

			// If found end of block, stop reading children

			if (L'}' == *pszCur) break;

			// Create a new child

			InfoElem* pChild = CreateChild();

			// Read it from the buffer and get offset past read data

			int nOffset = 0;

			try
			{
				pChild->Deserialize(pszCur, &nOffset);
			}
			catch (Error&)
			{
				// Make sure to destroy child if throwing
				delete pChild;
				throw;
			}

			pszCur += nOffset;

			// Add it to the list of chidlren

			AddChild(pChild);
		}

		// Advance past the '}'

		pszCur++;
	}

	// Set the length read

	if (pnLengthRead != NULL)
		*pnLengthRead = int(pszCur - pszData);
}

void InfoElem::Serialize(Stream& rStream, int nLevel) const
{
	try
	{
		bool bInList =
			m_pParent != NULL && TYPE_VALUELIST == m_pParent->m_nElemType;

		if (true == bInList)
		{
			// Write value

			String strTemp = ToString();

			rStream.Write((LPCVOID)ToString().GetBufferConst(),
				strTemp.GetLengthBytes());
		}
		else
		{
			// Write tabs before, according to level...

			for(int n = 0; n < nLevel; n++)
				rStream.Write((LPCVOID)L"\t", sizeof(WCHAR));

			// Write name as simple unicode string

			rStream.Write((LPCVOID)m_strName.GetBufferConst(),
				m_strName.GetLengthBytes());

			// Write value

			if (TYPE_VALUE == m_nElemType || TYPE_VALUELIST == m_nElemType)
				rStream.Write((LPCVOID)L": ", sizeof(WCHAR) * 2);

			if (TYPE_VALUE == m_nElemType || TYPE_VALUEBLOCK == m_nElemType)
			{
				String strTemp = ToString();

				rStream.Write((LPCVOID)strTemp.GetBufferConst(),
					strTemp.GetLengthBytes());
			}
		}

		// Write children if any

		if (m_nElemType > TYPE_VALUE)
		{
			if (TYPE_VALUELIST == m_nElemType)
			{
				for(InfoElemConstIterator pos = m_arChildren.begin();
					pos != m_arChildren.end();
					pos++)
				{
					(*pos)->Serialize(rStream, nLevel + 1);

					if ((pos + 1) != m_arChildren.end())
						rStream.Write((LPCVOID)L", ", sizeof(WCHAR) * 2);
				}

				rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);
			}
			else
			{
				rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);

				for(int n = 0; n < nLevel; n++)
					rStream.Write((LPCVOID)L"\t", sizeof(WCHAR));

				rStream.Write((LPCVOID)L"{\r\n", sizeof(WCHAR) * 3);

				for(InfoElemConstIterator pos = m_arChildren.begin();
					pos != m_arChildren.end();
					pos++)
				{
					(*pos)->Serialize(rStream, nLevel + 1);

					if ((pos + 1) != m_arChildren.end())
						rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);
				}

				for(int n = 0; n < nLevel; n++)
					rStream.Write((LPCVOID)L"\t", sizeof(WCHAR));

				rStream.Write((LPCVOID)L"}\r\n", sizeof(WCHAR) * 3);
			}
		}

		if (TYPE_VALUE == m_nElemType && false == bInList)
			rStream.Write((LPCVOID)L"\r\n", sizeof(WCHAR) * 2);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_rDocument.m_pErrorContext != NULL)
			throw m_rDocument.m_pErrorContext->Push(Error::FILE_SERIALIZE,
				__FUNCTIONW__, rStream.GetPath());
		else
			throw Error(Error::FILE_SERIALIZE, __FUNCTIONW__, rStream.GetPath());
	}
}

int InfoElem::ToIntArray(int* pnArray, int nElements, int nStart) const
{
	if (nElements < 1) return 0;

	int nCount = 0;

	for(InfoElemConstIterator pos = m_arChildren.begin() + nStart;
		pos != m_arChildren.end();
		pos++)
	{
		pnArray[nCount++] = (*pos)->m_nValue;

		if (nCount == nElements) break;
	}

	return nCount;
}

int InfoElem::ToFloatArray(float* pfArray, int nElements, int nStart) const
{
	if (nElements < 1) return 0;

	int nCount = 0;

	for(InfoElemConstIterator pos = m_arChildren.begin() + nStart;
		pos != m_arChildren.end();
		pos++)
	{
		pfArray[nCount++] = (*pos)->m_fValue;

		if (nCount == nElements) break;
	}

	return nCount;
}

int InfoElem::ToDwordArray(DWORD* pdwArray, int nElements, int nStart) const
{
	if (nElements < 1) return 0;

	int nCount = 0;

	for(InfoElemConstIterator pos = m_arChildren.begin() + nStart;
		pos != m_arChildren.end();
		pos++)
	{
		pdwArray[nCount++] = (*pos)->m_dwValue;

		if (nCount == nElements) break;
	}

	return nCount;
}

void InfoElem::FromIntArray(const int* pnArray, int nElements)
{
	m_nElemType = TYPE_VALUELIST;

	for(const int *pnCur = pnArray,
		*pnEnd = pnArray + nElements;
		pnCur != pnEnd; pnCur++)
	{
		AddChild(NULL, TYPE_VALUE,
			Variable::TYPE_INT)->SetIntValue(*pnCur);
	}
}

void InfoElem::FromFloatArray(const float* pfArray, int nElements)
{
	m_nElemType = TYPE_VALUELIST;

	for(const float *pfCur = pfArray,
		*pfEnd = pfArray + nElements;
		pfCur != pfEnd; pfCur++)
	{
		AddChild(NULL, TYPE_VALUE,
			Variable::TYPE_FLOAT)->SetFloatValue(*pfCur);
	}
}

void InfoElem::FromDwordArray(const DWORD* pdwArray, int nElements)
{
	m_nElemType = TYPE_VALUELIST;

	for(const DWORD *pdwCur = pdwArray,
		*pdwEnd = pdwArray + DWORD(nElements);
		pdwCur != pdwEnd; pdwCur++)
	{
		AddChild(NULL, TYPE_VALUE,
			Variable::TYPE_DWORD)->SetDwordValue(*pdwCur);
	}
}

DWORD InfoElem::ToFlags(const LPCWSTR* ppszFlagNames, 
						const DWORD* pdwFlagValues,
						int nFlagCount, DWORD dwDefault, 
						int nStart) const
{
	DWORD dwFlags = 0;

	if (TYPE_VALUE == m_nElemType)
	{
		if (TYPE_ENUM == m_nType)
			dwFlags |= GetEnumValue(ppszFlagNames, pdwFlagValues, nFlagCount);
	}
	else if (TYPE_VALUELIST == m_nElemType)
	{
		for(int n = nStart; n < GetChildCount(); n++)
		{
			const InfoElem* pElem = GetChildConst(n);

			if (pElem->GetVarType() != TYPE_ENUM)
				continue;

			dwFlags |= pElem->GetEnumValue(ppszFlagNames,
				pdwFlagValues,
				nFlagCount);
		}
	}
	else
	{
		dwFlags = dwDefault;
	}

	return dwFlags;
}

void InfoElem::FromFlags(DWORD dwFlags,
						 const LPCWSTR* pszFlagNames,
						 const DWORD* pdwFlagValues,
						 int nFlagCount)
{
	for(int n = 0; n < nFlagCount; n++)
	{
		if (dwFlags & pdwFlagValues[n])
			AddChild()->SetEnumValue(n, pszFlagNames, nFlagCount);
	}
}

void InfoElem::Empty(void)
{
	Variable::Empty();

	RemoveAllChildren();
}

void InfoElem::Release(void)
{
	if (m_pParent != NULL)
	{
		m_pParent->RemoveChild(this, true);
	}
	else
	{
		m_rDocument.m_pRoot = NULL;

		delete this;
	}
}

InfoElem& InfoElem::operator=(const InfoElem& rAssign)
{
	Empty();

	// Copy desc

	m_nElemType = rAssign.m_nElemType;
	m_strName = rAssign.m_strName;

	// Copy value

	FromVariable(*static_cast<Variable*>(this));

	// Copy children

	for(InfoElemConstIterator pos = rAssign.GetBeginChildPosConst();
		pos != rAssign.GetEndChildPosConst();
		pos++)
	{
		AddChild(new InfoElem(**pos));
	}

	return *this;
}

bool InfoElem::operator==(const InfoElem& rCompare) const
{
	// Compare desc

	if (rCompare.m_nElemType != m_nElemType)
		return false;

	if (rCompare.m_strName != m_strName)
		return false;

	// Compare value

	if (GetValueConst() != rCompare.GetValueConst())
		return false;

	// Compare children

	if (m_arChildren.size() != rCompare.m_arChildren.size())
		return false;

	for(InfoElemConstIterator pos = m_arChildren.begin();
		pos != m_arChildren.end();
		pos++)
	{
		for(InfoElemConstIterator posWith = rCompare.m_arChildren.begin();
			posWith != rCompare.m_arChildren.end();
			posWith++)
		{
			if (*pos != *posWith)
				return false;
		}
	}

	return true;
}