/*------------------------------------------------------------------*\
|
| ScreenMenu.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Frame Control implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "Globals.h"		// using SZ_SCREEN_MARGIN
#include "ScreenMenu.h"		// defining ScreenMenu
#include "ScreenMenuItem.h"	// using ScreenMenuItem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenMenu::SZ_CLASS[] =		L"menu";
const WCHAR ScreenMenu::SZ_POPUPOFFSETX[] =	L"popup.offset.x";
const WCHAR ScreenMenu::SZ_POPUPOFFSETY[] =	L"popup.offset.y";

/*----------------------------------------------------------*\
| ScreenFrame implementation
\*----------------------------------------------------------*/

ScreenMenu::ScreenMenu(Engine& rEngine, LPCWSTR pszClass, Screen* pParent):
					   ScreenFrame(rEngine, pszClass, pParent),
					   m_nMargin(0)
{
}

ScreenMenu::~ScreenMenu(void)
{
}

Object* ScreenMenu::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenMenu(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenMenu::Popup(POINT ptScreenLocation)
{
	SetPosition(ptScreenLocation.x + m_ptPopupOffset.x,
				ptScreenLocation.y + m_ptPopupOffset.y);

	if (IsFlagSet(INVISIBLE))
	{
		ClearFlag(INVISIBLE);
	}

	ZOrder(ZORDER_TOP);

	m_rEngine.GetScreens().SetFocusScreen(GetFirstItem());
}

void ScreenMenu::Popup(Screen* pBottomLeftOf)
{
	if (pBottomLeftOf == NULL)
	{
		return;
	}

	RECT rcAbsolute;
	pBottomLeftOf->GetAbsRect(rcAbsolute);

	POINT ptPosition = { rcAbsolute.left, rcAbsolute.bottom };

	Screen* pParent = GetParent();

	while (pParent != NULL)
	{
		ptPosition.x -= pParent->GetPosition().x;
		ptPosition.y -= pParent->GetPosition().y;

		pParent = pParent->GetParent();
	}

	Popup(ptPosition);
}

void ScreenMenu::Hide(void)
{
	// Hide the popup
	SetFlag(INVISIBLE);

	// Notify no-selection
	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(ScreenButton::NOTIFY_SELECT, NULL, ScreenButton::NOTIFY_SELECT_NONE);
	}

	for (ScreenList::iterator pos = m_lstListeners.begin();
		 pos != m_lstListeners.end();
		 pos++)
	{
		if (*pos != NULL)
		{
			(*pos)->OnNotify(ScreenButton::NOTIFY_SELECT, NULL, ScreenButton::NOTIFY_SELECT_NONE);
		}
	}
}

void ScreenMenu::Deserialize(const InfoElem& rRoot)
{
	ScreenFrame::Deserialize(rRoot);

	// Invisible by default until pop up

	SetFlag(INVISIBLE);

	// Read margin
	
	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_nMargin = pVar->GetIntValue();

	// Read popup offset

	if (LoadVariable(&pVar, SZ_POPUPOFFSETX,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_ptPopupOffset.x = pVar->GetIntValue();
	else
		m_ptPopupOffset.x = 0;

	if (LoadVariable(&pVar, SZ_POPUPOFFSETY,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_ptPopupOffset.y = pVar->GetIntValue();
	else
		m_ptPopupOffset.y = 0;

	UpdateLayout();
}

void ScreenMenu::PopupSubMenu(Screen* pSourceItem)
{
	if (pSourceItem == NULL)
	{
		return;
	}

	RECT rcAbsolute;
	pSourceItem->GetAbsRect(rcAbsolute);

	POINT ptPosition = { rcAbsolute.right, rcAbsolute.top };
	Popup(ptPosition);
}

Screen* ScreenMenu::GetFirstItem(void)
{
	for (ScreenListIterator pos = m_lstChildren.GetBeginPos();
			pos != m_lstChildren.GetEndPos();
			pos++)
	{
		if (dynamic_cast<ScreenMenu*>(*pos) == NULL)
		{
			return *pos;
		}
	}

	return NULL;
}

void ScreenMenu::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// Item selected - pass along to parent
	if (m_pParent != NULL)
	{
		m_pParent->OnCommand(nCommandID, pSender, nParam);
	}

	// Pass along to listeners
	for (ScreenList::iterator pos = m_lstListeners.begin();
		 pos != m_lstListeners.end();
		 pos++)
	{
		if (*pos != NULL)
			(*pos)->OnCommand(nCommandID, pSender, nParam);
	}

	Hide();
}

int ScreenMenu::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	if (nNotifyID == ScreenButton::NOTIFY_SELECT &&
		nParam == ScreenButton::NOTIFY_SELECT_NONE)
	{
		// Close if the element under mouse is not one of our children
		if (m_rEngine.GetScreens().GetFocusScreen() != NULL)
		{
			if (m_rEngine.GetScreens().GetFocusScreen() == this ||
				m_rEngine.GetScreens().GetFocusScreen()->IsDescendantOf(this))
			{
				return 0;
			}
		}

		Hide();
	}
	else
	{
		// Item highlighted - display a submenu if sender is a valid menu item with sub menu ID
		ScreenMenuItem* pSubMenuItem = dynamic_cast<ScreenMenuItem*>(pSender);

		if (pSubMenuItem != NULL && pSubMenuItem->GetSubMenuID() != INVALID_INDEX)
		{
			ScreenMenu* pSubMenu = NULL;

			for (ScreenListIterator pos = m_lstChildren.GetBeginPos();
				 pos != m_lstChildren.GetEndPos();
				 pos++)
			{
				pSubMenu = dynamic_cast<ScreenMenu*>(*pos);

				if (pSubMenu != NULL && pSubMenu->GetID() == pSubMenuItem->GetSubMenuID())
				{
					pSubMenu->PopupSubMenu(pSender);
					return 0;
				}
			}
		}
	}

	return 0;
}

void ScreenMenu::OnKeyDown(int nKeyCode)
{
	// TODO: handle selection
}

void ScreenMenu::UpdateLayout()
{
	// Set width and height of all children and self
	int nTotalHeight = m_nMargin;
	int nMaxWidth = 0;
	int nMaxIconWidth = 0;

	for (ScreenListIterator pos = m_lstChildren.GetBeginPos();
		 pos != m_lstChildren.GetEndPos();
		 pos++)
	{
		ScreenMenuItem* pMenuItem = dynamic_cast<ScreenMenuItem*>(*pos);

		if (pMenuItem != NULL && pMenuItem->GetFont() != NULL)
		{
			nMaxIconWidth = pMenuItem->GetCheckmark().GetTextureCoords().GetWidth() +
				pMenuItem->GetIcon().GetTextureCoords().GetWidth();

			// Take into account checkmark if the style is set appropriately
			if (!pMenuItem->GetCheckmark().IsEmpty())
			{
				nMaxIconWidth += pMenuItem->GetMargin() + (pMenuItem->GetMargin() >> 1);
			}

			// Take into account the icon if menu item has it
			if (!pMenuItem->GetIcon().IsEmpty())
			{
				nMaxIconWidth += pMenuItem->GetMargin() + (pMenuItem->GetMargin() >> 1);
			}

			// Take into account the arrow if menu item has a submenu
			if (m_lstChildren.FindByID(pMenuItem->GetSubMenuID(), FALSE))
			{
				nMaxIconWidth += pMenuItem->GetMargin() + pMenuItem->GetArrow().GetTextureCoords().GetWidth();
			}

			pMenuItem->SetPosition(m_nMargin, nTotalHeight);

			if (!pMenuItem->GetText().IsEmpty())
			{
				SIZE psTextExtent =
					pMenuItem->GetFont()->GetTextExtent(pMenuItem->GetText(), -1, 0, 1000);

				pMenuItem->SetSize(psTextExtent.cx + nMaxIconWidth,
					psTextExtent.cy + pMenuItem->GetMargin() * 2);

				if (pMenuItem->GetSize().cx > nMaxWidth)
				{
					nMaxWidth = pMenuItem->GetSize().cx;
				}
			}
			else
			{
				pMenuItem->SetSize(pMenuItem->GetSize().cx,
					pMenuItem->GetFont()->GetCharHeight() + pMenuItem->GetMargin() * 2);
			}

			nTotalHeight += pMenuItem->GetSize().cy;
		}
	}

	nTotalHeight += m_nMargin;

	SetSize(nMaxWidth + m_nMargin * 2, nTotalHeight + m_nMargin * 2);

	for (ScreenListIterator pos = m_lstChildren.GetBeginPos();
		 pos != m_lstChildren.GetEndPos();
		 pos++)
	{
		ScreenMenuItem* pMenuItem = dynamic_cast<ScreenMenuItem*>(*pos);

		if (pMenuItem != NULL)
		{
			pMenuItem->SetSize(nMaxWidth, pMenuItem->GetSize().cy);
		}
	}
}