/*------------------------------------------------------------------*\
|
| ScreenImage.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Image Control implementation
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
#include "ScreenImage.h"	// defining ScreenImage

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenImage::SZ_CLASS[] = L"image";


/*----------------------------------------------------------*\
| ScreenImage implementation
\*----------------------------------------------------------*/

ScreenImage::ScreenImage(Engine& rEngine,
						 LPCWSTR pszClass,
						 Screen* pParent):
						 Screen(rEngine, pszClass, pParent)
{
}

ScreenImage::~ScreenImage(void)
{
}

Object* ScreenImage::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenImage(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenImage::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// If no size specified, size to background

	if ((0 == m_psSize.cx || 0 == m_psSize.cy) &&
		m_Background.IsEmpty() == false)
	{
		m_psSize.cx = m_Background.GetTextureCoords().GetWidth();
		m_psSize.cy = m_Background.GetTextureCoords().GetHeight();
	}

	// If background loaded and background flag not specified, add it

	if (m_Background.IsEmpty() == false)
		m_dwFlags |= Screen::BACKGROUND;
	else
		m_dwFlags &= Screen::BACKGROUND;
}

void ScreenImage::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// If background loaded and background flag not specified, add it

	if (m_Background.IsEmpty() == false)
		m_dwFlags |= Screen::BACKGROUND;
	else
		m_dwFlags &= Screen::BACKGROUND;
}

void ScreenImage::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// Pass through

	if (m_pParent != NULL)
		m_pParent->OnCommand(nCommandID, pSender, nParam);
}

int ScreenImage::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	// Pass through

	if (m_pParent != NULL)
		return m_pParent->OnNotify(nNotifyID, pSender, nParam);

	return Screen::OnNotify(nNotifyID, pSender, nParam);
}

void ScreenImage::OnFocus(Screen* pOldFocus)
{
	// Focus first child if have children

	if (m_lstChildren.GetCount() == 0)
		return;

	m_rEngine.GetScreens().SetFocusScreen(
		m_lstChildren.GetTopMost());
}

void ScreenImage::OnKeyDown(int nKeyCode)
{
	// Pass through

	if (m_pParent != NULL)
		m_pParent->OnKeyDown(nKeyCode);
}

void ScreenImage::OnMouseLDown(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLDown(pt);
}

void ScreenImage::OnMouseLUp(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLUp(pt);
}

void ScreenImage::OnMouseMove(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseMove(pt);
}