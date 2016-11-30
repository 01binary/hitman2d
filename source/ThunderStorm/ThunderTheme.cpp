/*------------------------------------------------------------------*\
|
| ThunderTheme.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm screen class(es) implementation
| Created: 10/22/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderTheme.h"		// defining Theme, ThemeStyle
#include "ThunderInfoFile.h"	// using InfoFile/Elem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Theme::SZ_THEME[]						= L"theme";
const WCHAR Theme::SZ_THEMESTYLE_DEFAULT[]			= L"default";

const WCHAR ThemeStyle::SZ_THEMESTYLE[]				= L"style";
const WCHAR ThemeStyle::SZ_THEMESTYLEPART_MATERIAL[]= L"material";
const WCHAR ThemeStyle::SZ_THEMESTYLEPART_FONT[]	= L"font";
const WCHAR ThemeStyle::SZ_THEMESTYLEPART_COLOR[]	= L"color";


/*----------------------------------------------------------*\
| Theme implementation
\*----------------------------------------------------------*/

Theme::Theme(Engine& rEngine): m_rEngine(rEngine)
{
}

Theme::~Theme(void)
{
	Empty();
}

ThemeStyle* Theme::GetStyle(LPCWSTR pszName)
{
	ThemeStyleMapIterator posFind =
		m_mapStylesByName.find(pszName);

	if (posFind == m_mapStylesByName.end())
		return NULL;

	return posFind->second;
}

ThemeStyle* Theme::GetStyle(LPCWSTR pszName, LPCWSTR pszClassName)
{
	ThemeStyleMapConstRange posFindRange =
		m_mapStylesByClass.equal_range(pszClassName);

	if (posFindRange.first == m_mapStylesByClass.end())
		return NULL;

	for(ThemeStyleMapConstIterator pos = posFindRange.first;
		pos != posFindRange.second;
		pos++)
	{
		if (pos->second->GetName() == pszName)
			return pos->second;
	}

	return NULL;
}

ThemeStyle* Theme::GetDefaultClassStyle(LPCWSTR pszClassName)
{
	ThemeStyleMapConstRange posFindRange =
		m_mapStylesByClass.equal_range(pszClassName);

	if (posFindRange.first == m_mapStylesByClass.end())
		return NULL;

	for(ThemeStyleMapConstIterator pos = posFindRange.first;
		pos != posFindRange.second;
		pos++)
	{
		if (pos->second->GetName() == SZ_THEMESTYLE_DEFAULT ||
		   pos->second->GetName().IsEmpty() == true)
			return pos->second;
	}

	return NULL;
}

ThemeStyleArrayIterator Theme::GetBeginStyle(void)
{
	return m_arStyles.begin();
}

ThemeStyleArrayIterator Theme::GetEndStyle(void)
{
	return m_arStyles.end();
}

int Theme::GetStyleCount(void) const
{
	return int(m_arStyles.size());
}

void Theme::Deserialize(LPCWSTR pszPath)
{
	Stream stream(&m_rEngine.GetErrors());

	WCHAR szFullPath[MAX_PATH] = {0};
	GetAbsolutePath(pszPath, szFullPath);

	try
	{
		stream.Open(szFullPath, GENERIC_READ, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	// Set current directory to theme file path	

	PUSH_CURRENT_DIRECTORY(szFullPath);

	try
	{
		Deserialize(stream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, pszPath);
	}

	// Restore current directory

	POP_CURRENT_DIRECTORY();
}

void Theme::Deserialize(Stream& rStream)
{
	// Read theme

	InfoFile themeFile(&m_rEngine.GetErrors());

	try
	{
		themeFile.Deserialize(rStream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Theme document must have a root element

	if (themeFile.GetRoot() == NULL)
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
		__FUNCTIONW__, rStream.GetPath());

	// Read theme name

	const InfoElem* pElem = themeFile.GetRoot();

	if (pElem->GetName() != SZ_THEME ||
	   pElem->GetVarType() != Variable::TYPE_STRING)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());
	}

	m_strName = themeFile.GetRoot()->GetStringValue();

	if (m_strName.IsEmpty())
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
		__FUNCTIONW__, rStream.GetPath());

	// Read children (styles or variables)

	ThemeStyleMapIterator posFind;

	try
	{
		for(InfoElemConstIterator pos = pElem->GetBeginChildPosConst();
			pos != pElem->GetEndChildPosConst();
			pos++)
		{
			if ((*pos)->GetName() == ThemeStyle::SZ_THEMESTYLE)
			{
				// Read style

				ThemeStyle* pThemeStyle = new ThemeStyle(*this);

				pThemeStyle->Deserialize(**pos);

				m_mapStylesByName.insert(
					std::pair<String, ThemeStyle*>(pThemeStyle->GetName(), pThemeStyle));

				if (pThemeStyle->GetClass().IsEmpty() == false)
					m_mapStylesByClass.insert(std::pair<String, ThemeStyle*>(
							pThemeStyle->GetClass(), pThemeStyle));

				m_arStyles.push_back(pThemeStyle);
			}
			else
			{
				// Read variable

				*m_Variables.Add((*pos)->GetName()) = **pos;
			}
		}
	}

	catch(std::bad_alloc e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(ThemeStyle));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Theme::Empty(void)
{
	// Deallocate styles

	for(ThemeStyleArrayIterator pos = m_arStyles.begin();
		pos != m_arStyles.end();
		pos++)
	{
		delete *pos;
	}

	m_arStyles.clear();
	m_mapStylesByName.clear();
	m_mapStylesByClass.clear();

	m_Variables.RemoveAll();
}

/*----------------------------------------------------------*\
| ThemeStyle implementation
\*----------------------------------------------------------*/

ThemeStyle::ThemeStyle(Theme& rTheme): m_rTheme(rTheme)
{
}

ThemeStyle::~ThemeStyle(void)
{
	Empty();
}

const MaterialInstance* ThemeStyle::GetMaterialInstanceConst(LPCWSTR pszName) const
{
	MaterialInstanceMapConstIterator posFind =
		m_mapMaterialElems.find(pszName);

	if (posFind == m_mapMaterialElems.end())
		return NULL;

	return &posFind->second;
}

const Font* ThemeStyle::GetFontConst(LPCWSTR pszName) const
{
	FontMapConstIterator posFind =
		m_mapFontElems.find(pszName);

	if (posFind == m_mapFontElems.end()) return NULL;

	return posFind->second;
}

MaterialInstance* ThemeStyle::GetMaterialInstance(LPCWSTR pszName)
{
	MaterialInstanceMapIterator posFind =
		m_mapMaterialElems.find(pszName);

	if (posFind == m_mapMaterialElems.end())
		return NULL;

	return &posFind->second;
}

Font* ThemeStyle::GetFont(LPCWSTR pszName)
{
	FontMapIterator posFind =
		m_mapFontElems.find(pszName);

	if (posFind == m_mapFontElems.end()) return NULL;

	return posFind->second;
}

const Color* ThemeStyle::GetColor(LPCWSTR pszName) const
{
	ColorMapConstIterator posFind =
		m_mapColorElems.find(pszName);

	if (posFind == m_mapColorElems.end()) return NULL;

	return &posFind->second;
}

const Variable* ThemeStyle::GetVariable(LPCWSTR pszName) const
{
	return m_Variables.FindConst(pszName);
}

const Variable* ThemeStyle::GetVariable(LPCWSTR pszName, Variable::Types nType) const
{
	const Variable* pVar = m_Variables.FindConst(pszName);

	if (!pVar || pVar->GetVarType() != nType) return NULL;

	return pVar;
}

void ThemeStyle::Deserialize(const InfoElem& rRoot)
{
	if (rRoot.GetVarType() != Variable::TYPE_STRING)
		throw m_rTheme.m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
		__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_THEMESTYLE,
			Variable::GetVarTypeString(Variable::TYPE_STRING));

	try
	{
		// Read style name

		LPCWSTR pszName = rRoot.GetStringValue();

		LPCWSTR pszClass = wcschr(pszName, L'#');

		if (pszClass != NULL)
		{
			// If # class separator was present, parse out the default class

			int nLen = int(pszClass - pszName);

			m_strClass.Allocate(nLen);

			m_strClass.CopyToBuffer(nLen, pszName, nLen);

			nLen = int(wcslen(pszClass + 1));

			m_strName.Allocate(nLen);

			m_strName.CopyToBuffer(nLen, pszClass + 1, nLen);
		}
		else
		{
			m_strName = pszName;
		}

		// Read style elements

		for(InfoElemConstIterator pos = rRoot.GetBeginChildPosConst();
			pos != rRoot.GetEndChildPosConst();
			pos++)
		{
			const InfoElem& rElem = **pos;
			const String& strElemName = rElem.GetName();

			if (strElemName.Right(int(wcslen(SZ_THEMESTYLEPART_MATERIAL))) ==
				SZ_THEMESTYLEPART_MATERIAL)
			{
				// Read material element

				MaterialInstance inst;
				inst.Deserialize(m_rTheme.m_rEngine, rElem);

				m_mapMaterialElems[strElemName] = inst;
			}
			else if (strElemName.Right(int(wcslen(SZ_THEMESTYLEPART_COLOR))) ==
				SZ_THEMESTYLEPART_COLOR)
			{
				// Read color element

				m_mapColorElems[strElemName].Deserialize(rElem);
			}
			else if (strElemName.Right(int(wcslen(SZ_THEMESTYLEPART_FONT))) ==
				SZ_THEMESTYLEPART_FONT)
			{
				// Read font element

				Font* pFont = m_rTheme.m_rEngine.GetFonts().LoadInstance(rElem);

				pFont->AddRef();

				m_mapFontElems[strElemName] = pFont;					
			}
			else
			{
				// Read variable

				*m_Variables.Add(strElemName) = rElem.GetValueConst();
			}
		}
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(MaterialInstance));
	}

	catch(Error& rError)
	{
		m_rTheme.m_rEngine.GetErrors().Push(rError);

		throw m_rTheme.m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void ThemeStyle::Empty(void)
{
	m_mapMaterialElems.clear();

	// Release font elements

	for(FontMapIterator pos = m_mapFontElems.begin();
		pos != m_mapFontElems.end();
		pos++)
	{
		pos->second->Release();
	}

	m_mapFontElems.clear();

	// Remove all variables

	m_Variables.RemoveAll();
}