/*------------------------------------------------------------------*\
|
| ScreenMenuitem.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Menu Item implementation
| Created: 08/18/2013
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "ScreenMenuItem.h"	// defining ScreenMenuItem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenMenuItem::SZ_CLASS[] =		L"menuitem";
const WCHAR ScreenMenuItem::SZ_CHECKMARK[] =	L"checkmark.material";
const WCHAR ScreenMenuItem::SZ_ARROW[] =		L"arrow.material";
const WCHAR ScreenMenuItem::SZ_SUBMENUID[] =	L"submenu.id";

/*----------------------------------------------------------*\
| ScreenMenuItem implementation
\*----------------------------------------------------------*/

ScreenMenuItem::ScreenMenuItem(Engine& rEngine,
							   LPCWSTR pszClass,
							   Screen* pParent):

	ScreenButtonEx(rEngine, pszClass, pParent)
{
}

ScreenMenuItem::~ScreenMenuItem(void)
{

}

Object* ScreenMenuItem::CreateInstance(Engine& rEngine,
									   LPCWSTR pszClass,
									   Object* pParent)
{
	return new ScreenMenuItem(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenMenuItem::Deserialize(const InfoElem& rRoot)
{
	ScreenButtonEx::Deserialize(rRoot);

	// Read checkmark
	if (LoadMaterialInstance(m_Checkmark, SZ_CHECKMARK, &rRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_CHECKMARK);
	}

	// Read arrow
	if (LoadMaterialInstance(m_Arrow, SZ_ARROW, &rRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_ARROW);
	}

	// Read submenu ID
	const Variable* pVar = NULL;

	if (LoadVariable(&pVar, SZ_SUBMENUID,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, NULL) == true)
	{
		m_nSubMenuID = pVar->GetIntValue();
	}
	
	// If no height specified, set based on font
	if (m_psSize.cy == 0 && m_pFont != NULL)
	{
		SetSize(m_psSize.cx, m_pFont->GetCharHeight() + m_nMargin * 2);
	}
}

DWORD ScreenMenuItem::GetMemoryFootprint(void) const
{
	return sizeof(ScreenMenuItem) -
		sizeof(Screen) +
		Screen::GetMemoryFootprint();
}

void ScreenMenuItem::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	ScreenButtonEx::OnRender(rGraphics, prc);

	// Determine icon color to use
	Color clrIcon;

	switch (m_nState)
	{
	case ScreenButton::STATE_HOVER:
		clrIcon = m_clrTextHover;
		break;
	case ScreenButton::STATE_PUSHED:
	case ScreenButton::STATE_PUSHEDTOGGLED:
		clrIcon = m_clrTextPushed;
		break;
	case ScreenButton::STATE_NORMALTOGGLED:
		clrIcon = m_clrTextToggled;
		break;
	case ScreenButton::STATE_HOVERTOGGLED:
		clrIcon = m_clrTextHoverToggled;
		break;
	}
	
	// If disabled, less alpha
	if (IsFlagSet(DISABLED) == true)
		clrIcon.SetAlpha(clrIcon.GetAFloat() * m_clrBackColor.GetAFloat());
	else
		clrIcon.SetAlpha(clrIcon.GetAFloat() * GetFrontBufferBlend().GetAFloat());

	// Render checkmark
	if (m_bToggle)
	{
		rGraphics.RenderQuad(m_Checkmark,
			Vector2(
			m_rcCaption.left - m_Checkmark.GetTextureCoords().GetWidth() - m_nMargin,
			m_rcCaption.top),
			clrIcon);
	}
	
	// Render arrow
	if (this->m_nSubMenuID != 0)
	{
		rGraphics.RenderQuad(m_Arrow,
				Vector2(
				m_rcCachedRect.right - m_Arrow.GetTextureCoords().GetWidth() - m_nMargin,
				m_rcCachedRect.top + (m_rcCachedRect.GetHeight() - m_Arrow.GetTextureCoords().GetHeight()) / 2),
				clrIcon);
	}
}

void ScreenMenuItem::OnThemeStyleChange(void)
{
	ScreenButtonEx::OnThemeStyleChange();

	// Read checkmark
	LoadMaterialInstance(m_Checkmark, SZ_CHECKMARK, NULL, m_pStyle);

	// Read arrow
	LoadMaterialInstance(m_Arrow, SZ_ARROW, NULL, m_pStyle);
}

void ScreenMenuItem::UpdateLayout(void)
{
	// Update text position to make sure there is space for icon or checkmark
	int maxWidth = m_Icon.GetTextureCoords().GetWidth() + m_Checkmark.GetTextureCoords().GetWidth();

	m_rcCaption.left = m_rcCachedRect.left + m_nMargin + maxWidth;

	if (!m_Icon.IsEmpty())
	{
		m_rcCaption.left += m_nMargin + (m_nMargin >> 1);
	}

	if (!m_Checkmark.IsEmpty())
	{
		m_rcCaption.left += m_nMargin + (m_nMargin >> 1);
	}

	m_rcCaption.right -= (m_Arrow.IsEmpty() ? 0 : m_Arrow.GetTextureCoords().GetWidth() + m_nMargin);
}

void ScreenMenuItem::OnMove(const POINT& rptOldPos)
{
	ScreenButtonEx::OnMove(rptOldPos);

	UpdateLayout();
}

void ScreenMenuItem::OnSize(const SIZE& rpsOldSize)
{
	ScreenButtonEx::OnSize(rpsOldSize);

	UpdateLayout();
}