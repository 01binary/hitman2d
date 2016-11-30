/*------------------------------------------------------------------*\
|
| ScreenToolbarButton.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Toolbar Button implementation
| Created: 08/23/2013
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "Globals.h"				// using global constants
#include "ScreenMenu.h"				// using ScreenMenu
#include "ScreenToolbarButton.h"	// defining ScreenToolbarButton

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenToolbarButton::SZ_CLASS[] =	L"toolbarbutton";
const WCHAR ScreenToolbarButton::SZ_MENUID[] =	L"menuid";
const WCHAR ScreenToolbarButton::SZ_ARROWMATERIAL[] = L"arrow.material";
const WCHAR ScreenToolbarButton::SZ_SEPMATERIAL[] = L"separator.material";

const LPCWSTR ScreenToolbarButton::SZ_FLAGS[] =	{
													L"dropdown"
												};

const DWORD ScreenToolbarButton::DW_FLAGS[] =	{
													ScreenToolbarButton::DROPDOWN
												};

/*----------------------------------------------------------*\
| ScreenToolbarButton implementation
\*----------------------------------------------------------*/

ScreenToolbarButton::ScreenToolbarButton(Engine& rEngine,
									     LPCWSTR pszClass,
									     Screen* pParent):

	ScreenButtonEx(rEngine, pszClass, pParent),
	m_nMenuID(INVALID_INDEX)
{
}

ScreenToolbarButton::~ScreenToolbarButton(void)
{
}

Object* ScreenToolbarButton::CreateInstance(Engine& rEngine,
										   LPCWSTR pszClass,
										   Object* pParent)
{
	return new ScreenToolbarButton(
		rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenToolbarButton::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	
}

void ScreenToolbarButton::Deserialize(const InfoElem& rRoot)
{
	ScreenButtonEx::Deserialize(rRoot);

	// Read menu ID

	const Variable* pVar = NULL;

	if (LoadVariable(&pVar, SZ_MENUID,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, NULL) == true)
	{
		m_nMenuID = pVar->GetIntValue();
	}

	// Read flags (optional)

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
	{
		// Use SetFlags to execute handler that creates arrow button when needed.
		// Note: for same reason, must be called after reading menu ID and arrow button style

		SetFlags(m_dwFlags | pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD)));

		if (IsFlagSet(DROPDOWN))
			SetFlag(TOGGLE);
	}

	// Read arrow material instance

	LoadMaterialInstance(m_Arrow, SZ_ARROWMATERIAL, &rRoot, m_pStyle);

	// Read separator material instance

	LoadMaterialInstance(m_Arrow, SZ_SEPMATERIAL, &rRoot, m_pStyle);

	// Resize to text and icon if size not specified

	if (m_psSize.cy == 0 || m_psSize.cx == 0)
	{
		SIZE extent = { 0, 0 };
		
		if (m_pFont != NULL && !m_strText.IsEmpty())
		{
			extent = m_pFont->GetTextExtent(m_strText, -1, 0, 1000);
		}

		if (!m_Icon.IsEmpty())
		{
			extent.cx += (extent.cx ? m_nMargin : 0) + m_Icon.GetTextureCoords().GetWidth();

			if (extent.cy < m_Icon.GetTextureCoords().GetHeight())
				extent.cy = m_Icon.GetTextureCoords().GetHeight();
		}

		SetSize(extent.cx,
			extent.cy + m_nMargin * 2);
	}
}

void ScreenToolbarButton::OnMove(const POINT& rptOldPos)
{
	Screen::OnMove(rptOldPos);

	UpdateLayout();
}

void ScreenToolbarButton::OnSize(const SIZE& rpsOldSize)
{
	Screen::OnSize(rpsOldSize);

	UpdateLayout();
}

void ScreenToolbarButton::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// Untoggle when popup menu returns a command
	SetToggle(false);
}

int ScreenToolbarButton::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	//if (nNotifyID == NOTIFY_SELECT &&
	//	pSender != this &&
	//	nParam == NOTIFY_SELECT_NONE)
	//{
	//	if (m_pArrowButton != NULL && pSender == m_pArrowButton)
	//		return 0;

	//	// Untoggle when popup menu is hidden
	//	if (IsFlagSet(DROPDOWN))
	//	{
	//		SetToggle(false);
	//	}
	//	else
	//	{
	//		//m_pArrowButton->SetToggle(false);
	//	}
	//}
	//else if (nNotifyID == NOTIFY_SELECT &&
	//		pSender == m_pArrowButton &&
	//		nParam == NOTIFY_SELECT_PUSHED)
	//{
	//	/*if (!m_pArrowButton->GetToggle())
	//	{
	//		DropDown();
	//	}
	//	else
	//	{
	//		SetToggle(false);
	//	}*/
	//}

	return 0;
}

void ScreenToolbarButton::OnAction()
{
	ScreenButtonEx::OnAction();

	// If dropdown, the whole button drops menu
	if (IsFlagSet(DROPDOWN) && m_nMenuID != INVALID_INDEX)
	{
		DropDown();
	}
}

void ScreenToolbarButton::OnKeyDown(int nKeyCode)
{
}

void ScreenToolbarButton::OnKeyUp(int nKeyCode)
{
}

void ScreenToolbarButton::OnMouseMove(POINT pt)
{
}

void ScreenToolbarButton::OnMouseLDown(POINT pt)
{
}

void ScreenToolbarButton::OnMouseLUp(POINT pt)
{
}

void ScreenToolbarButton::OnMouseEnter(void)
{
}

void ScreenToolbarButton::OnMouseLeave(void)
{
}

DWORD ScreenToolbarButton::GetMemoryFootprint(void) const
{
	return sizeof(ScreenToolbarButton) -
		sizeof(ScreenButtonEx) +
		ScreenButtonEx::GetMemoryFootprint();
}

void ScreenToolbarButton::UpdateLayout(void)
{
	// Resize and reposition the arrow button
	int nArrowSpace = 0;

	// TODO: arrow rect

	// Cache center texture position

	m_vecCenterPos.x = m_vecCachedPos.x + 
		float(m_Left.GetTextureCoords().GetWidth());

	m_vecCenterPos.y = m_vecCachedPos.y;

	// Cache right texture position

	m_vecRightPos.x = m_vecCachedPos.x +
		float(m_psSize.cx - m_Right.GetTextureCoords().GetWidth());

	m_vecRightPos.y = m_vecCachedPos.y;

	// Cache icon position

	if (ICON_ALIGN_LEFT == m_nIconAlign)
		m_vecIconPos.x = m_vecCachedPos.x + float(m_nMargin);
	else
		m_vecIconPos.x = m_vecCachedPos.x +
			float(m_psSize.cx - nArrowSpace - m_nMargin - m_Icon.GetTextureCoords().GetWidth());

	m_vecIconPos.y = m_vecCachedPos.y + floor(
		float(m_psSize.cy -
		m_Icon.GetTextureCoords().GetHeight()) / 2.0f);

	// Cache caption position

	if (m_Icon.GetTextureCoords().GetWidth() > 0)
	{
		if (ICON_ALIGN_LEFT == m_nIconAlign)
		{
			m_rcCaption.left = m_rcCachedRect.left +
				m_Icon.GetTextureCoords().GetWidth() + m_nMargin * 2;
		}
		else
		{
			m_rcCaption.left = m_rcCachedRect.left + m_Icon.GetTextureCoords().GetWidth() + m_nMargin * 2;
		}
	}
	else
	{
		m_rcCaption.left = m_rcCachedRect.left + m_nMargin + m_nMargin / 2;
	}


	m_rcCaption.top = m_rcCachedRect.top;

	m_rcCaption.right = m_rcCachedRect.left + m_psSize.cx - nArrowSpace - m_nMargin;

	if (m_strDescription.IsEmpty() == true)
		m_rcCaption.bottom = m_rcCachedRect.top + m_psSize.cy;
	else
		m_rcCaption.bottom = m_rcCaption.top +
			m_pFont->GetLineSpacing();
}

void ScreenToolbarButton::DropDown(void)
{
	// Get associated menu
	if (m_pParent == NULL)
	{
		return;
	}

	ScreenMenu* pMenu = dynamic_cast<ScreenMenu*>(
		m_pParent->GetChildren().FindByID(m_nMenuID, true));

	if (pMenu == NULL)
	{
		return;
	}
	
	pMenu->UnregisterEventListener(this);
	pMenu->RegisterEventListener(this);

	/*if (!IsFlagSet(DROPDOWN) && !m_pArrowButton)
	{
		return;
	}*/

	bool bToggle = m_bToggle;

	if (bToggle)
	{
		pMenu->Popup(this);
	}
	else
	{
		pMenu->SetFlag(INVISIBLE);

		if (m_rEngine.GetScreens().GetFocusScreen() != this)
		{
			m_rEngine.GetScreens().SetFocusScreen(this);
		}
	}
}