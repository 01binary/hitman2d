/*------------------------------------------------------------------*\
|
| ScreenTabControl.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Tab Control implementation
| Created: 04/04/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "ScreenTabControl.h"	// defining ScreenTabControl

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const int ScreenTabControl::MINTABSIZE =				100;

const WCHAR ScreenTabControl::SZ_TAB[] =				L"tab";
const WCHAR ScreenTabControl::SZ_TABSTYLEACTIVE[] =		L"tabstyleactive";
const WCHAR ScreenTabControl::SZ_TABSTYLEINACTIVE[] =	L"tabstyleinactive";
const WCHAR ScreenTabControl::SZ_CLASS[] =				L"tabcontrol";

const LPCWSTR ScreenTabControl::SZ_ELEMENTS[] =		{
														L"top.active.material",
														L"top.inactive.material",
														L"topleft.active.material",
														L"topleft.inactive.material",
														L"topright.active.material",
														L"topright.inactive.material",
														L"left.material",
														L"right.material",
														L"bottomleft.material",
														L"bottomright.material",
														L"bottom.material",
														L"center.material"
													};


/*----------------------------------------------------------*\
| ScreenTabControl implementation
\*----------------------------------------------------------*/

ScreenTabControl::ScreenTabControl(Engine& rEngine,
								   LPCWSTR pszClass,
								   Screen* pParent):

								   Screen(rEngine, pszClass, pParent),
								   m_nActiveTab(INVALID_INDEX)
{
}

ScreenTabControl::~ScreenTabControl(void)
{
}

Object* ScreenTabControl::CreateInstance(Engine& rEngine,
									     LPCWSTR pszClass,
									     Object* pParent)
{
	return new ScreenTabControl(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

int ScreenTabControl::AddTab(LPCWSTR pszName, int nContainerID, bool bUpdateLayout)
{
	// Create new tab and set it up

	ScreenButtonEx* pNewTab = dynamic_cast<ScreenButtonEx*>(
		ScreenButtonEx::CreateInstance(m_rEngine, ScreenButtonEx::SZ_CLASS, this));

	int nID = int(m_lstTabs.size());

	pNewTab->SetText(pszName);
	pNewTab->SetID(nID);
	pNewTab->SetStyle(m_strTabStyleInactive);

	// Add to children

	m_lstChildren.Add(pNewTab);

	// Add to tab list

	m_lstTabs.push_back(pNewTab);

	// Map to container ID

	m_mapTabContainers[nID] = nContainerID;

	// Update appearance

	if (true == bUpdateLayout)
		OnSize(m_psSize);

	return nID;
}

void ScreenTabControl::RemoveTab(int nID)
{
	Screen* pTab = m_lstChildren.FindByID(nID);

	if (pTab != NULL)
	{
		// Remove tab with ID from children and deallocate it

		m_lstChildren.Remove(pTab, true);

		// Remove from container mapping

		std::map<int, int>::iterator pos = m_mapTabContainers.find(nID);

		if (pos != m_mapTabContainers.end())
			m_mapTabContainers.erase(pos);

		// Adjust IDs of subsequent tabs and remove from tab list

		for(ScreenListIterator pos = m_lstTabs.begin();
			pos != m_lstTabs.end();
			pos++)
		{
			if (*pos != NULL && (*pos)->GetID() > nID)
				(*pos)->SetID((*pos)->GetID() - 1);
			else if ((*pos)->GetID() == nID)
				pos = m_lstTabs.erase(pos);
		}

		OnSize(m_psSize);
	}
}

void ScreenTabControl::RemoveAllTabs(void)
{
	for(ScreenListIterator pos = m_lstTabs.begin();
		pos != m_lstTabs.end();
		pos++)
	{
		m_lstChildren.Remove(*pos, true);
	}

	m_lstTabs.clear();

	m_mapTabContainers.clear();
}

void ScreenTabControl::SetActiveTab(int nID)
{
	if (m_nActiveTab == nID)
		return;

	Screen* pContainer = NULL;

	if (m_nActiveTab >= 0 && m_nActiveTab < int(m_lstTabs.size()))
	{
		// Deactivate previous tab

		GetTab(m_nActiveTab)->SetStyle(m_strTabStyleInactive);

		pContainer = m_lstChildren.FindByID(
			GetTabContainerID(m_nActiveTab));

		if (pContainer != NULL)
			pContainer->SetFlag(INVISIBLE);
	}

	// Active new tab

	m_nActiveTab = nID;

	ScreenButtonEx* pTab = GetTab(nID);

	if (pTab != NULL)
		pTab->SetStyle(m_strTabStyleActive);

	pContainer = m_lstChildren.FindByID(
		GetTabContainerID(nID));

	if (pContainer != NULL)
	{
		pContainer->ClearFlag(INVISIBLE);
		
		m_rEngine.GetScreens().SetFocusScreen(pContainer);
	}
}

void ScreenTabControl::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read tabs

	InfoElemConstRange range = rRoot.FindChildrenConst(SZ_TAB);

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		const InfoElem* pTabElem = pos->second;

		if (pTabElem->GetElemType() == InfoElem::TYPE_VALUE)
		{
			if (pTabElem->GetVarType() == InfoElem::TYPE_STRING)
				AddTab(pTabElem->GetStringValue(), INVALID_INDEX, false);
		}
		else if (pTabElem->GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			LPCWSTR pszName = NULL;
			int nContainerID = INVALID_INDEX;

			if (pTabElem->GetChildCount() > 0 &&
				pTabElem->GetChildConst(0)->GetVarType() == InfoElem::TYPE_STRING)
			{
				// Read tab name

				pszName = pTabElem->GetChildConst(0)->GetStringValue();
			}

			if (pTabElem->GetChildCount() > 1 &&
				pTabElem->GetChildConst(1)->GetVarType() == InfoElem::TYPE_INT)
			{
				// Read tab container ID

				nContainerID = pTabElem->GetChildConst(1)->GetIntValue();

				// Make sure this container is currently hidden

				Screen* pContainer = m_lstChildren.FindByID(nContainerID);

				if (pContainer != NULL)
					pContainer->SetFlag(INVISIBLE);
			}

			AddTab(pszName, nContainerID, false);
		}
	}

	if (m_lstTabs.empty() == false)
		SetActiveTab(0);
}

void ScreenTabControl::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Render frame...

	Color blend = GetFrontBufferBlend();

	Vector2 vecTopLeft = m_vecCachedPos;
	Vector2 vecSize;

	if (m_lstTabs.empty() == true)
	{
		// If no tabs, render empty frame...

		// Render top left

		rGraphics.RenderQuad(m_Elements[FRAME_TOPLEFT_ACTIVE],
			vecTopLeft, blend);

		// Render top

		vecTopLeft.x = vecTopLeft.x +
			float(m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetWidth());

		vecSize.x = float(m_psSize.cx -
			m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetWidth() -
			m_Elements[FRAME_TOPRIGHT_ACTIVE].GetTextureCoords().GetWidth());

		vecSize.y = float(m_Elements[FRAME_TOP_ACTIVE].GetTextureCoords().GetHeight());

		rGraphics.RenderQuad(m_Elements[FRAME_TOP_ACTIVE],
			vecTopLeft, vecSize, blend);

		// Render top right

		vecTopLeft.x = vecTopLeft.x + vecSize.x;

		rGraphics.RenderQuad(m_Elements[FRAME_TOPRIGHT_ACTIVE], vecTopLeft, blend);
	}
	else
	{
		vecSize.y = float(dynamic_cast<ScreenButtonEx*>(m_lstTabs.front())->
				GetBackgroundLeft().GetTextureCoords().GetHeight());

		vecTopLeft.y = vecTopLeft.y + vecSize.y;

		if (0 == m_nActiveTab)
		{
			// First tab is active...

			// Render active top left corner

			rGraphics.RenderQuad(m_Elements[FRAME_TOPLEFT_ACTIVE],
				vecTopLeft, blend);

			// Render active top to the end of active tab

			vecTopLeft.x = vecTopLeft.x +
				m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetWidth();

			vecSize.x = float(m_lstTabs.front()->GetSize().cx -
				m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetWidth());

			vecSize.y = float(m_Elements[FRAME_TOP_ACTIVE].GetTextureCoords().GetHeight());

			rGraphics.RenderQuad(m_Elements[FRAME_TOP_ACTIVE],
				vecTopLeft, vecSize, blend);

			// Render inactive top all the way to top right corner

			vecTopLeft.x = vecTopLeft.x + vecSize.x;

			vecSize.x = float(m_psSize.cx - m_lstTabs.front()->GetSize().cx -
				m_Elements[FRAME_TOPRIGHT_INACTIVE].GetTextureCoords().GetWidth());

			rGraphics.RenderQuad(m_Elements[FRAME_TOP_INACTIVE],
				vecTopLeft, vecSize, blend);

			// Render inactive top right corner

			vecTopLeft.x = vecTopLeft.x + vecSize.x;

			rGraphics.RenderQuad(m_Elements[FRAME_TOPRIGHT_INACTIVE],
				vecTopLeft, blend);
		}
		else
		{
			// Either the middle or the last tab is active

			// Render inactive top left corner

			rGraphics.RenderQuad(m_Elements[FRAME_TOPLEFT_INACTIVE],
				vecTopLeft, blend);

			// Render inactive top to the active tab

			vecTopLeft.x = vecTopLeft.x +
				float(m_Elements[FRAME_TOPLEFT_INACTIVE].GetTextureCoords().GetWidth());

			vecSize.x = float(GetTab(m_nActiveTab)->GetPosition().x -
				m_Elements[FRAME_TOPLEFT_INACTIVE].GetTextureCoords().GetWidth());

			vecSize.y =
				float(m_Elements[FRAME_TOP_INACTIVE].GetTextureCoords().GetHeight());

			rGraphics.RenderQuad(m_Elements[FRAME_TOP_INACTIVE], vecTopLeft, vecSize, blend);

			// Render active top below the active tab

			vecTopLeft.x = vecTopLeft.x + vecSize.x;

			vecSize.x = float(GetTab(m_nActiveTab)->GetSize().cx);

			rGraphics.RenderQuad(m_Elements[FRAME_TOP_ACTIVE], vecTopLeft, vecSize, blend);

			// Render inactive top onward

			vecTopLeft.x = vecTopLeft.x + vecSize.x;

			vecSize.x = m_vecCachedPos.x - vecTopLeft.x + float(m_psSize.cx -
				m_Elements[FRAME_TOPRIGHT_INACTIVE].GetTextureCoords().GetWidth());

			rGraphics.RenderQuad(m_Elements[FRAME_TOP_INACTIVE], vecTopLeft, vecSize, blend);

			if ((int(m_lstTabs.size()) - 1) == m_nActiveTab)
			{
				// If last tab is active...

				// Render active top right corner

				vecTopLeft.x = m_vecCachedPos.x + float(m_psSize.cx -
					m_Elements[FRAME_TOPRIGHT_ACTIVE].GetTextureCoords().GetWidth());

				rGraphics.RenderQuad(m_Elements[FRAME_TOPRIGHT_ACTIVE],
					vecTopLeft, blend);
			}
			else
			{
				// If one if the middle tabs is active...

				// Render inactive top right corner

				vecTopLeft.x = m_vecCachedPos.x + float(m_psSize.cx -
					m_Elements[FRAME_TOPRIGHT_INACTIVE].GetTextureCoords().GetWidth());

				rGraphics.RenderQuad(m_Elements[FRAME_TOPRIGHT_INACTIVE],
					vecTopLeft, blend);
			}
		}
	}

	// Render left edge

	vecTopLeft = m_vecCachedPos;

	vecTopLeft.y = vecTopLeft.y +
		float((m_lstTabs.empty() == true ? 0 : m_lstTabs.front()->GetSize().cy) +
		m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetHeight());

	vecSize.x = float(m_Elements[FRAME_LEFT].GetTextureCoords().GetWidth());

	vecSize.y = float(m_psSize.cy -
		m_Elements[FRAME_TOPLEFT_ACTIVE].GetTextureCoords().GetHeight() -
		m_Elements[FRAME_BOTTOMLEFT].GetTextureCoords().GetHeight() -
		(m_lstTabs.empty() == true ? 0 : m_lstTabs.front()->GetSize().cy));

	rGraphics.RenderQuad(m_Elements[FRAME_LEFT], vecTopLeft, vecSize, blend);

	// Render right edge

	vecTopLeft.x = m_vecCachedPos.x + float(m_psSize.cx -
		m_Elements[FRAME_RIGHT].GetTextureCoords().GetWidth());

	rGraphics.RenderQuad(m_Elements[FRAME_RIGHT], vecTopLeft, vecSize, blend);

	// Render center

	vecTopLeft.x = m_vecCachedPos.x + vecSize.x;

	vecSize.x = float(m_psSize.cx -
		m_Elements[FRAME_LEFT].GetTextureCoords().GetWidth() -
		m_Elements[FRAME_RIGHT].GetTextureCoords().GetWidth());

	vecSize.y = float(m_psSize.cy -
		m_Elements[FRAME_BOTTOM].GetTextureCoords().GetHeight() -
		m_Elements[FRAME_TOP_ACTIVE].GetTextureCoords().GetHeight() -
		(m_lstTabs.empty() == true ? 0 : m_lstTabs.front()->GetSize().cy));

	rGraphics.RenderQuad(m_Elements[FRAME_CENTER], vecTopLeft, vecSize, blend);

	// Render bottom left

	vecTopLeft.x = m_vecCachedPos.x;
	vecTopLeft.y = m_vecCachedPos.y + float(m_psSize.cy -
		m_Elements[FRAME_BOTTOM].GetTextureCoords().GetHeight());

	rGraphics.RenderQuad(m_Elements[FRAME_BOTTOMLEFT], vecTopLeft, blend);

	// Render bottom

	vecTopLeft.x = vecTopLeft.x +
		float(m_Elements[FRAME_BOTTOMLEFT].GetTextureCoords().GetWidth());

	vecSize.y = float(m_Elements[FRAME_BOTTOM].GetTextureCoords().GetHeight());
	vecSize.x = float(m_psSize.cx -
		m_Elements[FRAME_BOTTOMLEFT].GetTextureCoords().GetWidth() -
		m_Elements[FRAME_BOTTOMRIGHT].GetTextureCoords().GetWidth());

	rGraphics.RenderQuad(m_Elements[FRAME_BOTTOM], vecTopLeft, vecSize, blend);

	// Render bottom right

	vecTopLeft.x = vecTopLeft.x + vecSize.x;

	rGraphics.RenderQuad(m_Elements[FRAME_BOTTOMRIGHT], vecTopLeft, blend);

	// Render children including tab buttons

	m_lstChildren.Render();
}

void ScreenTabControl::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	if (nCommandID >= 0 && nCommandID < int(m_lstTabs.size()))
	{
		SetActiveTab(nCommandID);

		if (m_pParent != NULL)
			m_pParent->OnCommand(GetTabContainerID(nCommandID), pSender, nParam);
	}
	else
	{
		if (m_pParent != NULL)
			m_pParent->OnCommand(nCommandID, pSender, nParam);
	}
}

int ScreenTabControl::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	if (m_pParent != NULL)
		return m_pParent->OnNotify(nNotifyID, pSender, nParam);

	return Screen::OnNotify(nNotifyID, pSender, nParam);
}

void ScreenTabControl::OnKeyDown(int nKeyCode)
{
	if (m_pParent != NULL)
		m_pParent->OnKeyDown(nKeyCode);
}

void ScreenTabControl::OnKeyPress(int nAsciiCode, bool extended, bool alt)
{
	if (m_pParent != NULL)
		m_pParent->OnKeyPress(nAsciiCode, extended, alt);
}

void ScreenTabControl::OnMouseLDown(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLDown(pt);
}

void ScreenTabControl::OnMouseLUp(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLUp(pt);
}

void ScreenTabControl::OnMouseMove(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseMove(pt);
}

void ScreenTabControl::OnSize(const SIZE& rpsOldSize)
{
	Screen::OnSize(rpsOldSize);

	// Re-size frame

	// Re-position tabs

	int nLeft = 0;

	for(ScreenListIterator pos = m_lstTabs.begin();
		pos != m_lstTabs.end();
		pos++)
	{
		ScreenButtonEx* pTab = dynamic_cast<ScreenButtonEx*>(*pos);

		// Re-position tab

		pTab->SetPosition(nLeft, 0);

		// Calculate width based on text

		SIZE sizeExt = pTab->GetFont()->GetTextExtent(pTab->GetText(),
			pTab->GetText().GetLength(),
			Font::ALIGN_CENTER | Font::ALIGN_VCENTER | Font::USE_MNEMONICS,
			m_psSize.cx);

		int nWidth = sizeExt.cx + pTab->GetMargin() * 2 +
			pTab->GetBackgroundLeft().GetTextureCoords().GetWidth() +
			pTab->GetBackgroundRight().GetTextureCoords().GetWidth();

		pTab->SetSize(nWidth,
			pTab->GetBackgroundLeft().GetTextureCoords().GetHeight());

		// Prepare for next

		nLeft += pTab->GetSize().cx;
	}
}

void ScreenTabControl::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Read tab active style

	const Variable* pVar = NULL;

	if (LoadVariable(&pVar, SZ_TABSTYLEACTIVE, Variable::TYPE_STRING,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle))
			m_strTabStyleActive = pVar->GetStringValue();

	// Read tab inactive style

	if (LoadVariable(&pVar, SZ_TABSTYLEINACTIVE, Variable::TYPE_STRING,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle))
			m_strTabStyleInactive = pVar->GetStringValue();

	// Read frame elements

	for(int nElem = 0; nElem < FRAME_ELEMENT_COUNT; nElem++)
	{
		if (LoadMaterialInstance(m_Elements[nElem], SZ_ELEMENTS[nElem],
			NULL, m_pStyle) == false)
			throw m_rEngine.GetErrors().Push(Error::THEME_INVALIDSTYLEELEMENT,
				__FUNCTIONW__, SZ_ELEMENTS[nElem],
				m_pStyle->GetThemeConst().GetName());
	}
}