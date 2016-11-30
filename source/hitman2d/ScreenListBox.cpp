/*------------------------------------------------------------------*\
|
| ScreenListBox.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ListBox Control implementation
| Created: 03/02/2011
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
#include "ScreenListBox.h"		// defining ScreenListBox

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenListBox::SZ_SCROLLBAR_MARGIN[] =	L"scrollbarmargin";
const WCHAR ScreenListBox::SZ_FRAMESTYLE[] =		L"framestyle";
const WCHAR ScreenListBox::SZ_ITEMSTYLE[] =			L"itemstyle";
const WCHAR ScreenListBox::SZ_SCROLLSTYLE[] =		L"scrollstyle";
const WCHAR ScreenListBox::SZ_ITEMHEIGHT[] =		L"itemheight";
const WCHAR ScreenListBox::SZ_ALLOWITEMDESELECT[] =	L"allowitemdeselect";
const WCHAR ScreenListBox::SZ_ITEM[] =				L"item";
const WCHAR ScreenListBox::SZ_CLASS[] =				L"listbox";

const LPCWSTR ScreenListBox::SZ_FLAGS[] =			{
														L"disablekeyboard"
													};

const DWORD ScreenListBox::DW_FLAGS[] =				{
														ScreenListBox::DISABLE_KEYBOARD
													};


/*----------------------------------------------------------*\
| ScreenListBox implementation
\*----------------------------------------------------------*/

ScreenListBox::ScreenListBox(Engine& rEngine,
							 LPCWSTR pszClass,
							 Screen* pParent):

							 Screen(rEngine, pszClass, pParent),
							 m_nFirstVisibleItem(INVALID_INDEX),
							 m_nVisibleItems(INVALID_INDEX),
							 m_nSelectedItem(INVALID_INDEX),
							 m_nItemHeight(16),
							 m_nMargin(10),
							 m_nScrollMargin(10),
							 m_pFrame(NULL),
							 m_pScrollbar(NULL),
							 m_bKeyboardSel(false),
							 m_bItemDeselect(false)
{
}

ScreenListBox::~ScreenListBox(void)
{
}

Object* ScreenListBox::CreateInstance(Engine& rEngine,
									  LPCWSTR pszClass,
									  Object* pParent)
{
	return new ScreenListBox(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

int ScreenListBox::AddItem(LPCWSTR pszText, LPCWSTR pszDescription, bool bUpdate, bool bLoadStyle, ScreenButtonEx** ppCreatedItem)
{
	ScreenButtonEx* pNewItem = AddItem(bUpdate, bLoadStyle);

	if (pszText != NULL)
		pNewItem->SetText(pszText);

	if (pszDescription != NULL)
		pNewItem->SetDescription(pszDescription);

	if (ppCreatedItem != NULL)
		*ppCreatedItem = pNewItem;

	return int(m_arItems.size() - 1);
}

ScreenButtonEx* ScreenListBox::AddItem(bool bUpdate, bool bLoadStyle)
{
	// Create new item

	ScreenButtonEx* pNewItem = NULL;

	try
	{
		pNewItem =
			new ScreenButtonEx(m_rEngine, ScreenButtonEx::SZ_CLASS, this);
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(Screen));
	}

	// Set flags for toggle

	pNewItem->SetFlag(ScreenButtonEx::TOGGLE);

	// Set its ID to item ID

	pNewItem->SetID(int(m_arItems.size()));

	// Set its style - named the same but different class
	
	if (bLoadStyle)
	{
		pNewItem->SetStyle(m_strItemStyle);
	}

	// Add to list of items and to list of children

	m_arItems.push_back(pNewItem);

	m_lstChildren.Add(pNewItem);

	if (true == bUpdate)
		Update();

	return pNewItem;
}

ScreenButtonEx* ScreenListBox::InsertItem(int nIndex)
{
	// Create new item

	ScreenButtonEx* pNewItem = NULL;

	try
	{
		pNewItem =
			new ScreenButtonEx(m_rEngine, ScreenButtonEx::SZ_CLASS, this);
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(Screen));
	}

	// Set flags for toggle

	pNewItem->SetFlag(ScreenButtonEx::TOGGLE);

	// Set its ID to item ID

	pNewItem->SetID(nIndex);

	// Set its style - named the same but different class
	
	pNewItem->SetStyle(m_strItemStyle);

	// Insert into list of items and children

	m_arItems.insert(m_arItems.begin() + nIndex, pNewItem);

	m_lstChildren.Add(pNewItem);

	// Update visible items

	m_nVisibleItems = INVALID_INDEX;

	UpdateVisible();

	// Update scroll range
	
	UpdateScrollRange();

	// Invalidate, if not rendered directly on screen

	Invalidate();

	// Return created item

	return pNewItem;
}

void ScreenListBox::RemoveItem(int nIndex)
{
	if (nIndex == m_nSelectedItem)
		SetSelectedItem(-1);
			
	// Deallocate item from children

	m_lstChildren.Remove(m_arItems[nIndex], true);

	// Remove item from items

	m_arItems.erase(m_arItems.begin() + nIndex);

	// Re-calculate the index on all subsequent items

	int nID = nIndex;

	for(ButtonExArrayIterator pos = m_arItems.begin() + nIndex;
		pos != m_arItems.end();
		pos++)
	{
		(*pos)->SetID(nID++);
	}

	// Update visible items

	if (nIndex < (m_nFirstVisibleItem + m_nVisibleItems))
	{
		if (m_nFirstVisibleItem >= int(m_arItems.size()))
			m_nFirstVisibleItem = int(m_arItems.size()) - 1;

		m_nVisibleItems = INVALID_INDEX;

		UpdateVisible();

		// Update scroll bar

		UpdateScrollRange();

		// Invalidate if not rendered directly on screen

		Invalidate();
	}
}

void ScreenListBox::RemoveAllItems(void)
{
	// Reset selected item if any

	SetSelectedItem(INVALID_INDEX);

	// Deallocate all items

	for(ButtonExArrayIterator pos = m_arItems.begin();
		pos != m_arItems.end();
		pos++)
	{
		m_lstChildren.Remove(*pos, true);
	}

	// Clear item list

	m_arItems.clear();

	// Update visible items

	m_nVisibleItems = INVALID_INDEX;

	UpdateVisible();	

	// Invalidate if not rendered directly on screen

	Invalidate();
}

void ScreenListBox::SetSelectedItem(int nSelectedItem)
{
	if (nSelectedItem < -1 || nSelectedItem >= int(m_arItems.size()))
		return;

	// Remember currently selected item

	int nOldSel = m_nSelectedItem;

	//if (m_nSelectedItem != nSelectedItem)
	{
		// Untoggle last item

		if (m_nSelectedItem != -1)
			m_arItems[m_nSelectedItem]->SetToggle(false);		

		// Change selection status

		m_nSelectedItem = nSelectedItem;

		// Toggle the item

		if (nSelectedItem != -1 &&
		   m_arItems[nSelectedItem]->GetToggle() == false)
			m_arItems[nSelectedItem]->SetToggle(true);

		// If newly selected item or old item is visible, invalidate

		if (IsItemVisible(m_nSelectedItem) == true ||
		   IsItemVisible(nOldSel) == true)
		{
			// Invalidate if not rendered directly to screen

			Invalidate();
		}

		// Notify parent

		if (m_pParent != NULL)
			m_pParent->OnNotify(NOTIFY_SELECT, this, nOldSel);

		// Notify listeners

		for(ScreenListIterator pos = m_lstListeners.begin();
			pos != m_lstListeners.end();
			pos++)
		{
			if (*pos != NULL)
				(*pos)->OnNotify(NOTIFY_SELECT, this, nOldSel);
		}

		m_bKeyboardSel = false;
	}
}

bool ScreenListBox::IsItemVisible(int nIndex) const
{
	return (nIndex >= m_nFirstVisibleItem &&
			nIndex < m_nFirstVisibleItem + m_nVisibleItems);
}

void ScreenListBox::EnsureItemVisible(int nIndex)
{
	// If already visible, check if any part is cut off

	bool bMakeFirst = true;

	if (IsItemVisible(nIndex) == true)
	{
		if (0 == m_nVisibleItems) return;

		Screen* pItem = m_arItems[nIndex - m_nFirstVisibleItem];

		if ((pItem->GetPosition().y + pItem->GetSize().cy) >= m_psSize.cy)
		{
			// Bottom is cut off. Scroll to make this the last item

			bMakeFirst = false;
		}
		else if (pItem->GetPosition().y >= 0)
		{
			// No part is cut off, the item should already be visible

			return;
		}
	}
	else
	{
		if (nIndex >= m_nFirstVisibleItem + m_nVisibleItems)
		{
			// The invisible item is below the bottom.
			// Scroll to make it the last item

			bMakeFirst = false;
		}
	}

	if (true == bMakeFirst)
		m_nFirstVisibleItem = nIndex;
	else
		m_nFirstVisibleItem = nIndex - m_nVisibleItems + 1;

	UpdateVisible();
}

void ScreenListBox::SetMargin(int nMargin)
{
	m_nMargin = nMargin;

	// Update visible items

	m_nVisibleItems = -1;

	UpdateVisible();
}

void ScreenListBox::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read flags

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
		m_dwFlags |= pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD));

	// Read margin
	
	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_nMargin = pVar->GetIntValue();

	// Read scroll bar margin
	
	if (LoadVariable(&pVar, SZ_SCROLLBAR_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_nScrollMargin = pVar->GetIntValue();

	// Read frame style

	if (LoadVariable(&pVar, SZ_FRAMESTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_strFrameStyle = pVar->GetStringValue();

	// Read item style

	if (LoadVariable(&pVar, SZ_ITEMSTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_strItemStyle = pVar->GetStringValue();

	// Read scrollbar style

	if (LoadVariable(&pVar, SZ_SCROLLSTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_strScrollStyle = pVar->GetStringValue();

	// Read item height

	if (LoadVariable(&pVar, SZ_ITEMHEIGHT,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, &rRoot, m_pStyle) == true)
			m_nItemHeight = pVar->GetIntValue();

	// Read item deselect

	if (LoadVariable(&pVar, SZ_ALLOWITEMDESELECT,
		Variable::TYPE_BOOL, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
			m_bItemDeselect = pVar->GetBoolValue();

	// Read items

	DeserializeItems(rRoot);
}

void ScreenListBox::DeserializeItems(const InfoElem& rRoot)
{
	InfoElemConstRange range = rRoot.FindChildrenConst(SZ_ITEM);

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		ScreenButtonEx* pItem = AddItem(false);

		if (NULL == pItem)
			break;

		const InfoElem& rElem = *pos->second;

		if (rElem.GetElemType() == InfoElem::TYPE_VALUE)
		{
			// Read text

			if (rElem.GetVarType() == Variable::TYPE_STRING)
				 pItem->SetText(rElem.GetStringValue());
		}
		else if (rElem.GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			// Read text

			if (rElem.GetChildCount() > 0 &&
			   rElem.GetChildConst(0)->GetVarType() == Variable::TYPE_STRING)
				 pItem->SetText(rElem.GetChildConst(0)->GetStringValue());

			// Read description

			if (rElem.GetChildCount() > 1 &&
			   rElem.GetChildConst(1)->GetVarType() == Variable::TYPE_STRING)
			{
				// Find and resolve any newline escape codes in description

				if (wcsstr(rElem.GetChildConst(1)->GetStringValue(), L"\\n") != NULL)
				{
					String str = rElem.GetChildConst(1)->GetStringValue();

					for(LPWSTR psz = str.GetBuffer(),
						pszEnd = str.GetBuffer() + str.GetLength();
						psz != pszEnd;
						psz++)
					{
						if (L'\\' == *psz && L'n' == psz[1])
						{
							psz[0] = L'\r';
							psz[1] = L'\n';
							psz++;
						}
					}

					pItem->SetDescription(str);
				}
				else
				{
					pItem->SetDescription(rElem.GetChildConst(1)->GetStringValue());
				}
			}

			// Read icon

			if (rElem.GetChildCount() > 2 &&
				rElem.GetChildConst(2)->GetVarType() == Variable::TYPE_STRING)
			{
				if (L'#' == rElem.GetChildConst(2)->GetStringValue()[0])
				{
					// Read style name for the material instance

					ThemeStyle* pStyle = m_rEngine.GetScreens().GetTheme().GetStyle(
						&rElem.GetChildConst(2)->GetStringValue()[1]);

					LoadMaterialInstance(pItem->GetIcon(), SZ_SCREEN_BACKGROUND,
						NULL, pStyle, NULL);
				}
				else
				{
					// Read inline material instance - material

					pItem->GetIcon().SetMaterial(m_rEngine.GetMaterials().Load(
						rElem.GetChildConst(2)->GetStringValue()));

					Rect rcTexCoords;

					if (rElem.GetChildCount() > 3)
					{
						if (rElem.GetChildConst(3)->GetVarType() == Variable::TYPE_STRING)
						{
							// Read material instance - base texture

							pItem->GetIcon().SetBaseTexture(m_rEngine.GetTextures().Load(
								rElem.GetChildConst(3)->GetStringValue()));

							if (rElem.GetChildCount() > 7)
							{
								// Read material instance - texture coordinates

								rcTexCoords.Deserialize(rElem, 4);
							}
						}
						else if (rElem.GetChildConst(3)->GetVarType() == Variable::TYPE_INT &&
								rElem.GetChildCount() > 6)
						{
							// Read material instance - texture coordinates

							rcTexCoords.Deserialize(rElem, 3);
						}
					}
					
					pItem->GetIcon().SetTextureCoords(rcTexCoords);
				}
			}
		}
	}

	Update();
}

void ScreenListBox::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Reload margin

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_nMargin = pVar->GetIntValue();

	// Reload scroll bar margin

	if (LoadVariable(&pVar, SZ_SCROLLBAR_MARGIN,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_nScrollMargin = pVar->GetIntValue();

	// Reload frame style

	if (LoadVariable(&pVar, SZ_FRAMESTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_strFrameStyle = pVar->GetStringValue();

	// Reload item style

	if (LoadVariable(&pVar, SZ_ITEMSTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_strItemStyle = pVar->GetStringValue();

	// Reload scrollbar style

	if (LoadVariable(&pVar, SZ_SCROLLSTYLE,
		Variable::TYPE_STRING, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_strScrollStyle = pVar->GetStringValue();

	// Reload item height

	if (LoadVariable(&pVar, SZ_ITEMHEIGHT,
		Variable::TYPE_INT, Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_nItemHeight = pVar->GetIntValue();

	// Reload item deselect

	if (LoadVariable(&pVar, SZ_ALLOWITEMDESELECT,
		Variable::TYPE_BOOL, Variable::TYPE_INT, NULL, m_pStyle) == true)
			m_bItemDeselect = pVar->GetBoolValue();

	// Set frame style

	if (m_pFrame != NULL)
	{
		m_pFrame->SetStyle(m_strFrameStyle);
	}
	else if (m_strFrameStyle.IsEmpty() == false)
	{
		// Create frame (if style is set)

		m_pFrame = dynamic_cast<ScreenFrame*>(ScreenFrame::CreateInstance(m_rEngine,
			ScreenFrame::SZ_CLASS, this));

		m_pFrame->SetFlags(ScreenFrame::AUTOSIZE);
		m_pFrame->SetStyle(m_strFrameStyle);

		m_lstChildren.Add(m_pFrame, false);
	}

	// Set scrollbar style

	if (m_pScrollbar != NULL)
	{
		m_pScrollbar->SetStyle(m_strScrollStyle);
	}
	else
	{
		// Create scrollbar

		m_pScrollbar = dynamic_cast<ScreenScrollBar*>(ScreenScrollBar::CreateInstance(
			m_rEngine, ScreenScrollBar::SZ_CLASS, this));

		m_pScrollbar->SetFlag(ScreenScrollBar::VERTICAL);
		m_pScrollbar->SetStyle(m_strScrollStyle);

		m_lstChildren.Add(m_pScrollbar, true);

		m_pScrollbar->SetFlag(INVISIBLE);
	}

	// Set item styles	

	for(ButtonExArrayIterator pos = m_arItems.begin();
		pos != m_arItems.end();
		pos++)
	{
		// Set item style

		if (*pos != NULL)
			(*pos)->SetStyle(m_strItemStyle);
	}

	OnSize(m_psSize);
}

int ScreenListBox::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	switch(nNotifyID)
	{
	case ScreenScrollBar::NOTIFY_SCROLL:
		{
			// Scrollbar got scrolled

			if (nParam != ScreenScrollBar::SCROLL_USER)
			{
				// Update first visible item depending on scrollbar position

				ScreenScrollBar* pScroll = dynamic_cast<ScreenScrollBar*>(m_pScrollbar);

				m_nFirstVisibleItem = pScroll->GetValue();

				UpdateVisible();
			}
		}
		break;
	case ScreenButton::NOTIFY_SELECT:
		{
			if (ScreenButton::NOTIFY_SELECT_NONE == nParam)
			{
				// One of the buttons lost focus, notify parent

				if (m_pParent != NULL)
					m_pParent->OnNotify(NOTIFY_DEFOCUS, this);

				for(ScreenListIterator pos = m_lstListeners.begin();
					pos != m_lstListeners.end();
					pos++)
				{
					if (*pos != NULL)
						(*pos)->OnNotify(NOTIFY_DEFOCUS, this);
				}
			}
		}
		break;
	}	

	return 0;
}

void ScreenListBox::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// One of the item buttons got pushed

	if (nCommandID >= 0 && nCommandID < int(m_arItems.size()))
	{
		if (true == m_bItemDeselect)
		{
			if (m_nSelectedItem == nCommandID)
				SetSelectedItem(INVALID_INDEX);
			else
				SetSelectedItem(nCommandID);
		}
		else
		{
			SetSelectedItem(nCommandID);

			if (m_arItems[m_nSelectedItem]->GetToggle() == false)
				m_arItems[m_nSelectedItem]->SetToggle(true);
		}		
	}
}

void ScreenListBox::OnMouseWheel(int nZDelta)
{
	int nNewFirstVisible = m_nFirstVisibleItem + nZDelta / -WHEEL_DELTA;

	if (nNewFirstVisible < 0 ||
	   (nNewFirstVisible + m_nVisibleItems - 1) >= int(m_arItems.size()))
		return;

	m_nFirstVisibleItem = nNewFirstVisible;

	m_pScrollbar->SetValue(nNewFirstVisible);

	UpdateVisible();
}

void ScreenListBox::OnKeyDown(int nKeyCode)
{
	if (m_dwFlags & DISABLE_KEYBOARD)
	{
		if (m_pParent != NULL)
			m_pParent->OnKeyDown(nKeyCode);

		return;
	}

	if (m_arItems.empty() == true)
		return;

	switch(nKeyCode)
	{
	case VK_UP:
	case VK_LEFT:
		{
			m_bKeyboardSel = true;

			if (-1 == m_nSelectedItem)
			{
				if (m_arItems[0] != NULL &&
				   m_arItems[0]->IsFlagSet(DISABLED) == false)
					SetSelectedItem(0);
			}
			else
			{
				int nNewSel = m_nSelectedItem - 1;

				if (nNewSel >= 0 &&
				   m_arItems[nNewSel] != NULL &&
				   m_arItems[nNewSel]->IsFlagSet(DISABLED) == false)
					SetSelectedItem(nNewSel);
			}

			if (m_nSelectedItem != INVALID_INDEX)
			{
				m_rEngine.GetScreens().SetFocusScreen(m_arItems[m_nSelectedItem]);

				EnsureItemVisible(m_nSelectedItem);
			}
		}
		break;
	case VK_DOWN:
	case VK_RIGHT:
		{
			m_bKeyboardSel = true;

			if (-1 == m_nSelectedItem)
			{
				if (m_arItems[0] != NULL &&
				   m_arItems[0]->IsFlagSet(DISABLED) == false)
					SetSelectedItem(0);
			}
			else
			{
				int nNewSel = m_nSelectedItem + 1;

				if (nNewSel < int(m_arItems.size()) &&
				   m_arItems[nNewSel] != NULL &&
				   m_arItems[nNewSel]->IsFlagSet(DISABLED) == false)
					SetSelectedItem(nNewSel);
			}

			if (m_nSelectedItem != INVALID_INDEX)
			{
				m_rEngine.GetScreens().SetFocusScreen(m_arItems[m_nSelectedItem]);

				EnsureItemVisible(m_nSelectedItem);
			}
		}
		break;
	case VK_HOME:
		{
			m_bKeyboardSel = true;

			if (m_arItems[0] != NULL &&
			   m_arItems[0]->IsFlagSet(DISABLED) == false)
				SetSelectedItem(0);

			EnsureItemVisible(m_nSelectedItem);

			if (m_nSelectedItem != INVALID_INDEX)
				m_rEngine.GetScreens().SetFocusScreen(m_arItems[m_nSelectedItem]);
		}
		break;
	case VK_END:
		{
			m_bKeyboardSel = true;

			int nNewSel = int(m_arItems.size()) - 1;

			if (m_arItems[nNewSel] != NULL &&
			   m_arItems[nNewSel]->IsFlagSet(DISABLED) == false)
				SetSelectedItem(nNewSel);

			EnsureItemVisible(m_nSelectedItem);

			if (m_nSelectedItem != INVALID_INDEX)
				m_rEngine.GetScreens().SetFocusScreen(m_arItems[m_nSelectedItem]);
		}
		break;
	default:
		{
			// Pass through to parent if not handled

			if (m_pParent != NULL)
				m_pParent->OnKeyDown(nKeyCode);
		}
		break;
	}
}

void ScreenListBox::OnFocus(Screen* pOldFocus)
{
	// If have items, focus first visible non-disabled

	if (m_nVisibleItems > 0 && m_nFirstVisibleItem != -1)
	{
		int nFocus = m_nFirstVisibleItem;

		while(nFocus < m_nFirstVisibleItem + m_nVisibleItems &&
			m_arItems[nFocus]->IsFlagSet(DISABLED) == true)
				nFocus++;

		if (nFocus >= m_nFirstVisibleItem + m_nVisibleItems)
			return;

		m_rEngine.GetScreens().SetFocusScreen(m_arItems[nFocus]);
	}
}

void ScreenListBox::OnSize(const SIZE& rpsOldSize)
{	
	Screen::OnSize(rpsOldSize);

	// Resize frame

	if (m_pFrame != NULL)
		m_pFrame->OnSize(rpsOldSize);

	// Resize/reposition scrollbar (whether or not it's visible)

	if (m_pScrollbar != NULL)
	{
		m_pScrollbar->SetSize(m_pScrollbar->GetSize().cx,
			m_psSize.cy - m_nMargin * 2);

		m_pScrollbar->SetPosition(m_psSize.cx -
			m_pScrollbar->GetSize().cx - m_nScrollMargin,
			m_nMargin);
	}
	
	// Update visible items

	m_nVisibleItems = -1;

	UpdateVisible();

	UpdateScrollRange();
}

void ScreenListBox::UpdateVisible(void)
{
	// Calculate number of visible items

	if (m_arItems.empty() == false)
	{
		int nSpaceAvailable = m_psSize.cy - m_nMargin * 2;

		m_nVisibleItems = nSpaceAvailable / m_nItemHeight;

		// Account for any down-rounding and up-rounding errors

		int nSpaceLeft = nSpaceAvailable - m_nItemHeight * m_nVisibleItems;

		if (nSpaceLeft >= m_nItemHeight)
			m_nVisibleItems++;
		else if (nSpaceLeft < 0)
			m_nVisibleItems--;
	}
	else
	{
		m_nVisibleItems = 0;
		m_nFirstVisibleItem = 0;
	}

	bool bScroll = false;

	if (m_pScrollbar != NULL)
	{
		// See if there is going to be overflow that requires scrolling

		if (m_nVisibleItems < int(m_arItems.size()))
		{
			// Show scrollbar

			if (m_pScrollbar->IsFlagSet(INVISIBLE) == true)
				m_pScrollbar->ClearFlag(INVISIBLE);

			float fPageSize = float(m_arItems.size() - m_nVisibleItems + 1) *
				float(m_nVisibleItems) / float(m_arItems.size());

			m_pScrollbar->SetPageSize(fPageSize);
			
			if (m_pScrollbar->GetValue() != m_nFirstVisibleItem)
				m_pScrollbar->SetValue(m_nFirstVisibleItem);

			bScroll = true;
		}
		else
		{
			// Hide scrollbar

			if (m_pScrollbar->IsFlagSet(INVISIBLE) == false)
				m_pScrollbar->SetFlag(INVISIBLE);

			if (m_nFirstVisibleItem > 0)
				m_nFirstVisibleItem = 0;
		}
	}

	// Reposition items and update visibility

	int nLastBottom = m_nMargin;

	int nItemWidth = m_psSize.cx - m_nMargin - m_nScrollMargin -
		((true == bScroll) ? m_pScrollbar->GetSize().cx : 0);

	for(int n = 0;
		n < int(m_arItems.size());
		n++)
	{
		ScreenButtonEx* pItem = m_arItems[n];

		if (n >= m_nFirstVisibleItem && n < (m_nFirstVisibleItem + m_nVisibleItems))
		{
			// If visible, show and reposition

			if (pItem->IsFlagSet(INVISIBLE) == true)
				pItem->ClearFlag(INVISIBLE);

			pItem->SetPosition(m_nMargin, nLastBottom);
			pItem->SetSize(nItemWidth, m_nItemHeight);

			nLastBottom += m_nItemHeight;

			// If this is a selected item and not toggled, toggle it

			if (m_nSelectedItem == n && pItem->GetToggle() == false)
				pItem->SetToggle(true);
		}
		else
		{
			// If not visible, hide

			if (pItem->IsFlagSet(INVISIBLE) == false)
				pItem->SetFlag(INVISIBLE);
		}
	}
}

void ScreenListBox::UpdateScrollRange(void)
{
	if (NULL == m_pScrollbar)
		return;

	int nMax = int(m_arItems.size()) - m_nVisibleItems;

	if (nMax <= 0)
		nMax = 1;

	m_pScrollbar->SetMax(nMax);
}

void ScreenListBox::Update(void)
{
	// Update visible items

	m_nVisibleItems = -1;

	UpdateVisible();

	// Update scroll range

	UpdateScrollRange();

	// Invalidate, if not rendered directly on screen

	Invalidate();
}