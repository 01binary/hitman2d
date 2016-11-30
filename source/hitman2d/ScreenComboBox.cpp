/*------------------------------------------------------------------*\
|
| ScreenComboBox.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ComboBox Control implementation
| Created: 03/08/2011
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
#include "ScreenListBox.h"		// using ScreenListBox
#include "ScreenComboBox.h"		// defining ScreenComboBox

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenComboBox::SZ_LISTBOXSTYLE[] = L"listboxstyle";
const WCHAR ScreenComboBox::SZ_LISTBOXNAME[] = L"combodroplist";
const WCHAR ScreenComboBox::SZ_MAXVISIBLEITEMS[] = L"maxvisibleitems";
const WCHAR ScreenComboBox::SZ_ARROWMATERIAL[] = L"arrow.material";
const WCHAR ScreenComboBox::SZ_CLASS[] = L"combobox";


/*----------------------------------------------------------*\
| ScreenComboBox implementation
\*----------------------------------------------------------*/

ScreenComboBox::ScreenComboBox(Engine& rEngine,
							   LPCWSTR pszClass,
							   Screen* pParent):
							   ScreenButtonEx(
									 rEngine,
									 pszClass,
									 pParent),
								m_pListBox(NULL),
								m_bDropped(false),
								m_nMaxVisibleItems(4)
{
	// Create list box child

	CreateListBox();

	// Set left text flag

	m_bTextAlignLeft = true;
}

ScreenComboBox::~ScreenComboBox(void)
{
	// Deallocate listbox

	if (m_pListBox != NULL)
	{
		if (true == m_bDropped)
		{
			ScreenListIterator posFind;

			if (m_rEngine.GetScreens().Find(m_pListBox, posFind) == true)
			{
				m_rEngine.GetScreens().Remove(posFind, false);
				delete m_pListBox;
			}
		}
		else
		{
			delete m_pListBox;
		}
	}
}

Object* ScreenComboBox::CreateInstance(Engine& rEngine,
									   LPCWSTR pszClass,
									   Object* pParent)
{
	return new ScreenComboBox(rEngine, pszClass,
		dynamic_cast<Screen*>(pParent));
}

void ScreenComboBox::Deserialize(const InfoElem& rRoot)
{
	ScreenButtonEx::Deserialize(rRoot);

	if (!m_pListBox)
		return;

	// Read list box style

	const Variable* pListBoxStyle = NULL;

	if (LoadVariable(&pListBoxStyle, SZ_LISTBOXSTYLE, Variable::TYPE_STRING,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_strListBoxStyle = pListBoxStyle->GetStringValue();

	m_pListBox->SetStyle(m_strListBoxStyle);

	// Read arrow material instance

	LoadMaterialInstance(m_Arrow, SZ_ARROWMATERIAL, &rRoot, m_pStyle);

	// Read items

	m_pListBox->DeserializeItems(rRoot);
}

DWORD ScreenComboBox::GetMemoryFootprint(void) const
{
	return ScreenButtonEx::GetMemoryFootprint() +
		sizeof(ScreenComboBox) - sizeof(ScreenButtonEx);
}

void ScreenComboBox::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Render button

	ScreenButtonEx::OnRender(rGraphics, prc);

	// Render arrow

	Color clrArrow = (ScreenButton::STATE_PUSHED == m_nState) ?
		m_clrTextPushed : m_clrText;

	if (IsFlagSet(DISABLED) == true)
	{
		clrArrow.SetAlpha(m_clrBackColor.GetAFloat() * GetFrontBufferBlend().GetAFloat() * 0.5f);
	}
	else
	{
		clrArrow.SetAlpha(m_clrBackColor.GetAFloat() * GetFrontBufferBlend().GetAFloat());
	}

	rGraphics.RenderQuad(m_Arrow, m_vecArrow, clrArrow);

	// Set dropdown alpha

	if (m_pListBox != NULL)
		m_pListBox->SetBlend(m_clrBlend);
}

void ScreenComboBox::OnAction(void)
{
	// Popup the list

	if (false == m_bDropped)
	{
		if (m_pListBox->GetSelectedItem() != INVALID_INDEX)
			m_pListBox->EnsureItemVisible(m_pListBox->GetSelectedItem());

		m_pListBox->ClearFlag(INVISIBLE);

		m_rEngine.GetScreens().Add(m_pListBox, true);

		m_rEngine.GetScreens().SetFocusScreen(m_pListBox);

		m_bDropped = true;
	}
	else
	{
		CloseListBox();
	}
}

int ScreenComboBox::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	switch(nNotifyID)
	{
	case ScreenListBox::NOTIFY_SELECT:
		{
			// Item selected - update text

			if (m_pListBox->GetSelectedItem() != INVALID_INDEX &&
			   m_pListBox->IsKeyboardSelection() == false)
			{
				m_strText = m_pListBox->GetItemConst(m_pListBox->GetSelectedItem())->GetText();

				m_pListBox->SetFlag(INVISIBLE);

				//m_rEngine.GetScreens().SetFocusScreen(this);

				CloseListBox();

				// Notify

				if (m_pParent != NULL)
					m_pParent->OnNotify(nNotifyID, this, nParam);

				for(ScreenListIterator pos = m_lstListeners.begin();
					pos != m_lstListeners.end();
					pos++)
				{
					(*pos)->OnNotify(nNotifyID, this, nParam);
				}
			}
		}
		break;
	case ScreenListBox::NOTIFY_DEFOCUS:
		{
			// Close drop down

			if (m_rEngine.GetScreens().GetFocusScreen() == NULL ||
			   (m_rEngine.GetScreens().GetFocusScreen()->IsDescendantOf(
					m_pListBox) == false &&
				m_rEngine.GetScreens().GetFocusScreen() != this))
			{
				CloseListBox();
			}
		}
		break;
	}

	return 0;
}

void ScreenComboBox::OnMove(const POINT& rptOldPos)
{
	ScreenButtonEx::OnMove(rptOldPos);

	// Move list if open

	if (m_pListBox != NULL)
	{
		Rect rcAbs;
		GetAbsRect(rcAbs);

		m_pListBox->SetPosition(rcAbs.left, rcAbs.top + m_psSize.cy + 2);
	}

	// Recalculate arrow position

	float fVertOffset =
		float((m_psSize.cy - m_Arrow.GetTextureCoords().GetHeight()) / 2);

	float fHorzOffset = float(m_psSize.cx - m_nMargin * 2);

	m_vecArrow.x = m_vecCachedPos.x + fHorzOffset;
	m_vecArrow.y = m_vecCachedPos.y + fVertOffset;

	// Update max drop list size

	UpdateDropListSize();
}

void ScreenComboBox::UpdateDropListSize(void)
{
	// Calculate max number of items that can be dropped

	Rect rcAbs;
	GetAbsRect(rcAbs);

	int nItemHeight = m_pListBox->GetItemCount() == 0 ? INT_MAX :
		m_pListBox->GetItem(0)->GetBackgroundLeft().GetTextureCoords().GetSize().cy;

	int nMaxItems = int(float(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight -
		rcAbs.bottom - m_pListBox->GetMargin() * 2) /
		float(nItemHeight));

	if (nMaxItems > m_nMaxVisibleItems)
		nMaxItems = m_nMaxVisibleItems;

	if (m_pListBox != NULL)
	{
		if (m_pListBox->GetItemCount() > 0)
		{
			// Resize listbox according to items added

			m_pListBox->SetSize(m_psSize.cx, (m_pListBox->GetItemCount() > nMaxItems ?
				nItemHeight * nMaxItems :
				nItemHeight * m_pListBox->GetItemCount()) + m_pListBox->GetMargin() * 2);
		}
		else
		{
			// If no items added, display arbitrarily sized blank list

			m_pListBox->SetSize(m_psSize.cx, m_psSize.cy * 3);
		}
	}
}

void ScreenComboBox::OnSize(const SIZE& rpsOldSize)
{
	ScreenButtonEx::OnSize(rpsOldSize);

	UpdateDropListSize();
}

void ScreenComboBox::OnThemeStyleChange(void)
{
	ScreenButtonEx::OnThemeStyleChange();

	if (NULL == m_pListBox)
		CreateListBox();

	// Read list box style

	const Variable* pVar = NULL;

	if (LoadVariable(&pVar, SZ_LISTBOXSTYLE, Variable::TYPE_STRING,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			m_strListBoxStyle = pVar->GetStringValue();

	m_pListBox->SetStyle(m_strListBoxStyle);

	// Read max visible items

	if (LoadVariable(&pVar, SZ_MAXVISIBLEITEMS, Variable::TYPE_INT,
		Variable::TYPE_BOOL, NULL, m_pStyle) == true)
			m_nMaxVisibleItems = pVar->GetIntValue();

	// Read arrow material instance

	LoadMaterialInstance(m_Arrow, SZ_ARROWMATERIAL, NULL, m_pStyle);
}

void ScreenComboBox::CreateListBox(void)
{
	// Create list box if needed

	m_pListBox = dynamic_cast<ScreenListBox*>(
		ScreenListBox::CreateInstance(m_rEngine,
			ScreenListBox::SZ_CLASS, NULL));

	m_pListBox->SetName(SZ_LISTBOXNAME);

	m_pListBox->SetFlags(Screen::INVISIBLE | Screen::NOACTIVATE);

	m_pListBox->RegisterEventListener(this);
}

void ScreenComboBox::CloseListBox(void)
{
	if (m_bDropped)
	{
		m_pListBox->SetFlag(INVISIBLE);

		m_rEngine.GetScreens().Remove(m_pListBox, false);

		m_rEngine.GetScreens().SetFocusScreen(this);

		m_bDropped = false;
	}
}