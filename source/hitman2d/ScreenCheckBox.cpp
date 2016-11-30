/*------------------------------------------------------------------*\
|
| ScreenCheckBox.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D CheckBox Control implementation
| Created: 03/30/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "Error.h"				// using error constants
#include "ScreenCheckBox.h"		// defining ScreenCheckBox

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenCheckBox::SZ_CLASS[] = L"checkbox";


/*----------------------------------------------------------*\
| ScreenCheckBox implementation
\*----------------------------------------------------------*/

ScreenCheckBox::ScreenCheckBox(Engine& rEngine,
							   LPCWSTR pszClass,
							   Screen* pParent):

							   ScreenButton(rEngine, pszClass, pParent)
{
	m_dwFlags |= TOGGLE;
}

ScreenCheckBox::~ScreenCheckBox(void)
{
}

Object* ScreenCheckBox::CreateInstance(Engine& rEngine,
									   LPCWSTR pszClass,
									   Object* pParent)
{
	return new ScreenCheckBox(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenCheckBox::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Calculate background color

	float fBlend = GetFrontBufferBlend().GetAFloat();

	Color clrBack(m_clrBackColor);
	clrBack.SetAlpha(clrBack.GetAFloat() * fBlend);
	
	// Render background

	rGraphics.RenderQuad(m_Background, m_vecCachedPos, clrBack);

	// Render text

	if (m_pFont != NULL && m_strText.IsEmpty() == false)
	{
		Rect rc = m_rcCachedRect;

		rc.left += int(m_Background.GetTextureCoords().GetWidth() * 1.5f);

		Color clrText = m_clrText;
		
		if (STATE_PUSHED == m_nState ||
		   STATE_PUSHEDTOGGLED == m_nState ||
		   STATE_NORMALTOGGLED == m_nState ||
		   STATE_HOVERTOGGLED == m_nState)
			clrText = m_clrTextPushed;

		clrText.SetAlpha(clrBack.GetAlpha());

		m_pFont->RenderText(rc, m_strText, -1, clrText,
			Font::ALIGN_VCENTER | Font::USE_MNEMONICS);
	}
}