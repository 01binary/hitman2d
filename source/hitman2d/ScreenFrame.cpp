/*------------------------------------------------------------------*\
|
| ScreenFrame.cpp
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
#include "Globals.h"		// using global constants
#include "Game.h"			// using Game
#include "Error.h"			// using error constants
#include "ScreenFrame.h"	// defining ScreenFrame

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenFrame::SZ_CLASS[] =		L"frame";

const LPCWSTR ScreenFrame::SZ_PARTS[] = {
											L"topleft.material",
											L"topright.material",
											L"bottomleft.material",
											L"bottomright.material",
											L"top.material",
											L"bottom.material",
											L"left.material",
											L"right.material",
											L"center.material"
										};

const LPCWSTR ScreenFrame::SZ_FLAGS[] = {
											L"autosize"
										};

const DWORD ScreenFrame::DW_FLAGS[] =	{
											ScreenFrame::AUTOSIZE
										};


/*----------------------------------------------------------*\
| ScreenFrame implementation
\*----------------------------------------------------------*/

ScreenFrame::ScreenFrame(Engine& rEngine, LPCWSTR pszClass, Screen* pParent):
						 Screen(rEngine, pszClass, pParent)
{
}

ScreenFrame::~ScreenFrame(void)
{
}

Object* ScreenFrame::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenFrame(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenFrame::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Render Top Left

	rGraphics.RenderQuad(m_Elements[PART_TOPLEFT],
		 m_vecCachedPos, GetFrontBufferBlend());

	// Render Top Right

	rGraphics.RenderQuad(m_Elements[PART_TOPRIGHT],
		m_vecTopRight, GetFrontBufferBlend());

	// Render Bottom Right

	rGraphics.RenderQuad(m_Elements[PART_BOTTOMRIGHT],
		m_vecBottomRight, GetFrontBufferBlend());

	// Render Bottom Left

	rGraphics.RenderQuad(m_Elements[PART_BOTTOMLEFT],
		Vector3(m_vecCachedPos.x, m_vecBottomRight.y, 0.0f),
		GetFrontBufferBlend());

	// Calculate repeats

	float fWidthStretch = float(m_psSize.cx -
				m_Elements[PART_TOPLEFT].GetTextureCoords().GetWidth() * 2);

	float fHeightStretch = float(m_psSize.cy -
				m_Elements[PART_TOPLEFT].GetTextureCoords().GetHeight() * 2);

	// Render Center Repeat

	rGraphics.RenderQuad(m_Elements[PART_CENTER],
		m_vecCenter,
		Vector2(fWidthStretch, fHeightStretch),
		GetFrontBufferBlend());

	// Render Left Repeat

	rGraphics.RenderQuad(m_Elements[PART_LEFT],
		Vector2(m_vecCachedPos.x, m_vecCenter.y),
		Vector2(float(m_Elements[PART_TOPLEFT].GetTextureCoords().GetWidth()),
				fHeightStretch),
		GetFrontBufferBlend());

	// Render Top Repeat

	rGraphics.RenderQuad(m_Elements[PART_TOP],
		Vector2(m_vecCenter.x, m_vecCachedPos.y),
		Vector2(fWidthStretch,
				float(m_Elements[PART_TOP].GetTextureCoords().GetHeight())),
		GetFrontBufferBlend());

	// Render Right Repeat

	rGraphics.RenderQuad(m_Elements[PART_RIGHT],
		Vector2(m_vecTopRight.x, m_vecCenter.y),
		Vector2(float(m_Elements[PART_TOPLEFT].GetTextureCoords().GetWidth()),
				fHeightStretch),
		GetFrontBufferBlend());

	// Render Bottom Repeat

	rGraphics.RenderQuad(m_Elements[PART_BOTTOM],
		Vector2(m_vecCenter.x, m_vecBottomRight.y),
		Vector2(fWidthStretch,
				float(m_Elements[PART_BOTTOM].GetTextureCoords().GetHeight())),
		GetFrontBufferBlend());

	// Render everything else

	Screen::OnRender(rGraphics, prc);
}

void ScreenFrame::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	// Pass through

	if (m_pParent != NULL)
		m_pParent->OnCommand(nCommandID, pSender, nParam);
}

int ScreenFrame::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	// Pass through

	if (m_pParent != NULL)
		return m_pParent->OnNotify(nNotifyID, pSender, nParam);

	return Screen::OnNotify(nNotifyID, pSender, nParam);
}

void ScreenFrame::OnKeyDown(int nKeyCode)
{
	// Pass through

	if (m_pParent != NULL)
		m_pParent->OnKeyDown(nKeyCode);
}

void ScreenFrame::OnMouseLDown(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLDown(pt);
}

void ScreenFrame::OnMouseLUp(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseLUp(pt);
}

void ScreenFrame::OnMouseMove(POINT pt)
{
	// Pass through

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	if (m_pParent != NULL)
		m_pParent->OnMouseMove(pt);
}

void ScreenFrame::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read specific flags

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
		SetFlags(m_dwFlags | pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD)));

	// Read elements

	for(int n = 0; n < PART_COUNT; n++)
	{
		if (LoadMaterialInstance(m_Elements[n], SZ_PARTS[n],
			&rRoot, m_pStyle) == false)
		{
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
				__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_PARTS[n]);
		}
	}
}

void ScreenFrame::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Reload elements

	for(int n = 0; n < PART_COUNT; n++)
	{
		LoadMaterialInstance(m_Elements[n], SZ_PARTS[n],
			NULL, m_pStyle);
	}
}

DWORD ScreenFrame::GetMemoryFootprint(void) const
{
	return sizeof(ScreenFrame) -
		sizeof(Screen) * 2 +
		Screen::GetMemoryFootprint();
}

void ScreenFrame::OnMove(const POINT& ptOldPosition)
{
	Screen::OnMove(ptOldPosition);

	// Recalculate Top Right

	m_vecTopRight.x = m_vecCachedPos.x + float(m_psSize.cx -
		m_Elements[PART_TOPLEFT].GetTextureCoords().GetWidth());

	m_vecTopRight.y = m_vecCachedPos.y;

	// Recalculate Bottom Right

	m_vecBottomRight.x = m_vecTopRight.x;

	m_vecBottomRight.y = m_vecCachedPos.y + float(m_psSize.cy -
		m_Elements[PART_BOTTOM].GetTextureCoords().GetHeight());

	// Recalculate Center

	m_vecCenter.x = float(m_vecCachedPos.x +
		m_Elements[PART_TOPLEFT].GetTextureCoords().GetWidth());

	m_vecCenter.y = float(m_vecCachedPos.y +
		m_Elements[PART_TOPLEFT].GetTextureCoords().GetHeight());
}

void ScreenFrame::OnSize(const SIZE& psOldSize)
{
	if (IsFlagSet(AUTOSIZE) == true && m_pParent != NULL)
	{
		if (m_pParent->GetSize().cx != m_psSize.cx ||
		   m_pParent->GetSize().cy != m_psSize.cy)
		{
			SetSize(m_pParent->GetSize());
			return;
		}
	}

	OnMove(m_ptPos);
}