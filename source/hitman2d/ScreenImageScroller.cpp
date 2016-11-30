/*------------------------------------------------------------------*\
|
| ScreenImageScroller.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ImageScroller Control implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "Globals.h"				// using global constants
#include "Game.h"					// using Game
#include "Error.h"					// using error constants
#include "ScreenImageScroller.h"	// defining ScreenImageScroller

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenImageScroller::SZ_CLASS[] =			L"imagescroller";
const WCHAR ScreenImageScroller::SZ_SCROLLINTERVAL[] =	L"scroll.interval";
const WCHAR ScreenImageScroller::SZ_SCROLLDISTANCE[] =	L"scroll.distance";
const WCHAR ScreenImageScroller::SZ_SCROLLDIRECTION[] =	L"scroll.direction";

const LPCWSTR ScreenImageScroller::SZ_SCROLLDIRECTIONS[] =	{
														L"xpos",
														L"xneg",
														L"ypos",
														L"yneg"
															};

/*----------------------------------------------------------*\
| ScreenImageScroller implementation
\*----------------------------------------------------------*/

ScreenImageScroller::ScreenImageScroller(Engine& rEngine,
										 LPCWSTR pszClass,
										 Screen* pParent):
										 Screen(rEngine, pszClass, pParent),
										 m_fXScale(0.0f),
										 m_fYScale(0.0f),
										 m_fScrollInterval(0.01f),
										 m_nScrollDist(1),
										 m_nScrollOffset(0),
										 m_nScrollDir(SCROLL_XPOS)
{
}

ScreenImageScroller::~ScreenImageScroller(void)
{
	// Stop scroll timer

	m_rEngine.GetTimers().Remove(this);
}

Object* ScreenImageScroller::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenImageScroller(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenImageScroller::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// If no size specificed, size to background

	if (0 == m_psSize.cx || 0 == m_psSize.cy)
	{
		m_psSize.cx = m_Background.GetTextureCoords().GetWidth();
		m_psSize.cy = m_Background.GetTextureCoords().GetHeight();
	}

	// Calculate scale if we are sized differently than background

	m_fXScale = float(m_psSize.cx) /
		float(m_Background.GetTextureCoords().GetWidth());

	m_fYScale = float(m_psSize.cy) /
		float(m_Background.GetTextureCoords().GetHeight());

	// If background loaded and background flag not specified, add it

	if (m_Background.IsEmpty() == false)
		m_dwFlags |= Screen::BACKGROUND;

	// Read scroll interval

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCROLLINTERVAL,
		Variable::TYPE_FLOAT, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_FLOAT)
			m_fScrollInterval = pVar->GetFloatValue();
		else
			m_fScrollInterval = float(pVar->GetIntValue());
	}

	// Read scroll distance

	if (LoadVariable(&pVar, SZ_SCROLLDISTANCE,
		Variable::TYPE_INT, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
			m_nScrollDist = pVar->GetIntValue();

	// Read scroll direction

	if (LoadVariable(&pVar, SZ_SCROLLDIRECTION,
		Variable::TYPE_ENUM, Variable::TYPE_ENUM, &rRoot, m_pStyle) == true)
			m_nScrollDir = SCROLLDIR(pVar->GetEnumValue(
				SZ_SCROLLDIRECTIONS, SCROLL_COUNT, 0));

	// Start scroll timer

	m_rEngine.GetTimers().Add(this, m_fScrollInterval);
}

void ScreenImageScroller::OnThemeStyleChange(void)
{
	// Calculate scale if we are sized differently than background

	m_fXScale = float(m_psSize.cx) /
		float(m_Background.GetTextureCoords().GetWidth());

	m_fYScale = float(m_psSize.cy) /
		float(m_Background.GetTextureCoords().GetHeight());

	// Reload scroll interval

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCROLLINTERVAL,
		Variable::TYPE_FLOAT, Variable::TYPE_INT, NULL, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_FLOAT)
			m_fScrollInterval = pVar->GetFloatValue();
		else
			m_fScrollInterval = float(pVar->GetIntValue());
	}

	// Reload scroll distance

	if (LoadVariable(&pVar, SZ_SCROLLDISTANCE,
		Variable::TYPE_INT, Variable::TYPE_INT, NULL, m_pStyle) == true)
			m_nScrollDist = pVar->GetIntValue();

	// Reload scroll direction

	if (LoadVariable(&pVar, SZ_SCROLLDIRECTION,
		Variable::TYPE_ENUM, Variable::TYPE_ENUM, NULL, m_pStyle) == true)
			m_nScrollDir = SCROLLDIR(pVar->GetEnumValue(
				SZ_SCROLLDIRECTIONS, SCROLL_COUNT, 0));

	Timer* pTimer = m_rEngine.GetTimers().Find(this);

	if (pTimer != NULL)
		pTimer->SetInterval(m_fScrollInterval);
}

void ScreenImageScroller::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	Rect rcOldCoords = m_Background.GetTextureCoords();
	Rect rcFirstCoords = m_Background.GetTextureCoords();
	Rect rcSecondCoords = m_Background.GetTextureCoords();

	if (m_nScrollDir < SCROLL_YPOS)
	{
		rcFirstCoords.left = rcFirstCoords.right - m_nScrollOffset;
		rcSecondCoords.right -= m_nScrollOffset;
	}
	else
	{
		rcFirstCoords.top = rcFirstCoords.bottom - m_nScrollOffset;
		rcSecondCoords.bottom -= m_nScrollOffset;
	}

	m_Background.SetTextureCoords(rcFirstCoords);

	Vector2 vecSize(float(rcFirstCoords.GetWidth()) * m_fXScale,
				float(rcFirstCoords.GetHeight()) * m_fYScale);

	rGraphics.RenderQuad(m_Background, m_vecCachedPos, vecSize,
		GetFrontBufferBlend());

	m_Background.SetTextureCoords(rcSecondCoords);

	if (m_nScrollDir < SCROLL_YPOS)
	{
		rGraphics.RenderQuad(m_Background,
			Vector2(m_vecCachedPos.x + vecSize.x, m_vecCachedPos.y),
			Vector2(float(rcSecondCoords.GetWidth()) * m_fXScale,
					float(rcSecondCoords.GetHeight()) * m_fYScale),
			GetFrontBufferBlend());		
	}
	else
	{
		rGraphics.RenderQuad(m_Background,
			Vector2(m_vecCachedPos.x, m_vecCachedPos.y + vecSize.y),
				Vector2(float(rcSecondCoords.GetWidth()) * m_fXScale,
						float(rcSecondCoords.GetHeight()) * m_fYScale),
					GetFrontBufferBlend());		
	}

	m_Background.SetTextureCoords(rcOldCoords);
}

void ScreenImageScroller::OnTimer(Timer& rTimer)
{
	switch(m_nScrollDir)
	{
	case SCROLL_XPOS:
		{
			m_nScrollOffset += m_nScrollDist;

			if (m_nScrollOffset > m_Background.GetTextureCoords().GetWidth())
				m_nScrollOffset = 0;
		}
		break;
	case SCROLL_XNEG:
		{
			m_nScrollOffset -= m_nScrollDist;

			if (m_nScrollOffset < 0)
				m_nScrollOffset = m_Background.GetTextureCoords().GetWidth() - 1;
		}
		break;
	case SCROLL_YPOS:
		{
			m_nScrollOffset += m_nScrollDist;

			if (m_nScrollOffset > m_Background.GetTextureCoords().GetHeight())
				m_nScrollOffset = 0;
		}
		break;
	case SCROLL_YNEG:
		{
			m_nScrollOffset -= m_nScrollDist;

			if (m_nScrollOffset < 0)
				m_nScrollOffset = m_Background.GetTextureCoords().GetHeight() - 1;
		}
		break;
	}	
}