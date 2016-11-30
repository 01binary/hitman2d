/*------------------------------------------------------------------*\
|
| ScreenLabel.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Label Control implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "Globals.h"		// using global constants
#include "Game.h"			// using Game
#include "Error.h"			// using error constants
#include "ScreenButton.h"	// using ScreenButton
#include "ScreenLabel.h"	// defining ScreenLabel

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenLabel::SZ_TEXT[] =				L"text";
const WCHAR ScreenLabel::SZ_TEXTRENDERFLAGS[] =		L"textflags";
const WCHAR ScreenLabel::SZ_CLASS[] =				L"label";

const LPCWSTR ScreenLabel::SZ_TEXT_FLAGS[] =	{
													L"left",
													L"center",
													L"right",
													L"vcenter",
													L"usemnemonics"
												};

const DWORD ScreenLabel::DW_TEXT_FLAGS[] =		{
													Font::ALIGN_LEFT,
													Font::ALIGN_CENTER,
													Font::ALIGN_RIGHT,
													Font::ALIGN_VCENTER,
													Font::USE_MNEMONICS
												};


/*----------------------------------------------------------*\
| ScreenLabel implementation
\*----------------------------------------------------------*/

ScreenLabel::ScreenLabel(Engine& rEngine,
						 LPCWSTR pszClass,
						 Screen* pParent):
						 Screen(rEngine, pszClass, pParent),
						 m_pFont(NULL),
						 m_dwTextFlags(0)
{
}

ScreenLabel::~ScreenLabel(void)
{
}

Object* ScreenLabel::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenLabel(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenLabel::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Render the text

	if (m_pFont != NULL)
		m_pFont->RenderText(m_rcCachedRect, m_strText, -1,
			GetFrontBufferBlend(), m_dwTextFlags);
}

void ScreenLabel::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Read font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, &rRoot, m_pStyle, pSharedStyle);

	// Read text align

	const Variable* pFlagsVar = NULL;
	
	if (LoadVariable(&pFlagsVar, SZ_TEXTRENDERFLAGS,
		Variable::TYPE_ENUM, Variable::TYPE_ENUM,
		&rRoot, m_pStyle, pSharedStyle) == true)
		m_dwTextFlags = pFlagsVar->GetEnumValue(SZ_TEXT_FLAGS, DW_TEXT_FLAGS,
			sizeof(DW_TEXT_FLAGS) / sizeof(DWORD));

	// Read text

	const InfoElem* pElem = rRoot.FindChildConst(SZ_TEXT,
		InfoElem::TYPE_VALUE, Variable::TYPE_STRING);

	if (pElem != NULL)
		m_strText = pElem->GetStringValue();
}

void ScreenLabel::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Reload font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, NULL, m_pStyle, pSharedStyle);

	// Reload text align

	const Variable* pFlagsVar = NULL;
	
	if (LoadVariable(&pFlagsVar, SZ_TEXTRENDERFLAGS,
		Variable::TYPE_ENUM, Variable::TYPE_ENUM,
		NULL, m_pStyle, pSharedStyle) == true)
			m_dwTextFlags = pFlagsVar->GetEnumValue(SZ_TEXT_FLAGS, DW_TEXT_FLAGS,
				sizeof(DW_TEXT_FLAGS) / sizeof(DWORD));
}

DWORD ScreenLabel::GetMemoryFootprint(void) const
{
	return sizeof(ScreenLabel) - sizeof(Screen) * 2 +
		Screen::GetMemoryFootprint();
}