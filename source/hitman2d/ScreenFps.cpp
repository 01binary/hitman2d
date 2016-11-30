/*------------------------------------------------------------------*\
|
| ScreenFps.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScreenFps implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"		// precompiled header
#include "Globals.h"	// using global constants
#include "Game.h"		// using Game
#include "Error.h"		// using error constants
#include "ScreenFps.h"	// defining ScreenFps

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenFps::SZ_EXTENDED_FONT[] =			L"extended-font";
const WCHAR ScreenFps::SZ_EXTENDED_BACKGROUND[] =	L"extended-background.material";
const WCHAR ScreenFps::SZ_CLASS[] =					L"overlapped::fps";


/*----------------------------------------------------------*\
| ScreenFps implementation
\*----------------------------------------------------------*/

ScreenFps::ScreenFps(Engine& rEngine,
					 LPCWSTR pszClass,
					 Screen* pParent):

					 ScreenOverlapped(rEngine, pszClass, pParent),
					 m_bExtended(false),
					 m_nLastFps(-1),
					 m_fLastSpf(-1.0f),
					 m_pExtendedFont(NULL)

{
	m_extendedBackground.Empty();

	ZeroMemory(m_szFps, sizeof(m_szFps));
	ZeroMemory(m_szSpf, sizeof(m_szSpf));
	ZeroMemory(m_szMspf, sizeof(m_szMspf));
}

ScreenFps::~ScreenFps(void)
{
	if (m_pFont != NULL)
		m_pFont->Release();

	if (m_pExtendedFont != NULL)
		m_pExtendedFont->Release();
}

Object* ScreenFps::CreateInstance(Engine& rEngine,
								  LPCWSTR pszClass,
								  Object* pParent)
{
	return new ScreenFps(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenFps::SetExtended(bool bExtended)
{
	m_bExtended = bExtended;

	if (true == bExtended)
		m_Background = m_extendedBackground;
	else
		m_Background = m_normalBackground;

	m_psSize.cy = m_Background.GetTextureCoords().GetHeight();

	// When expanded or contracted, recache text rectangles

	OnMove(m_ptPos);
}

bool ScreenFps::GetExtended(void) const
{
	return m_bExtended;
}

void ScreenFps::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	if (m_nLastFps != rGraphics.GetFPS())
	{
		// Cache FPS text

		m_nLastFps = rGraphics.GetFPS();

		StringCchPrintf(m_szFps, 8, L"%d", m_nLastFps);

		// Cache SPF and MSPF text

		m_fLastSpf = m_rEngine.GetFrameTime();

		StringCchPrintf(m_szSpf, 16, L"%.3f", m_fLastSpf);

		StringCchPrintf(m_szMspf, 16, L"%.2f", m_fLastSpf * 1000.0f);
	}

	// Don't render if fps is still 0 from the time screen was opened
	// (incidentally, this may cause counter to never appear if frame rate is always 0)

	if (L'\0' == *m_szFps) return;

	// Render background

	Screen::OnRenderBackground(rGraphics);
	
	// Render text

	if (m_pFont != NULL)
		m_pFont->RenderText(m_rcTextPos, m_szFps);

	if (true == m_bExtended && m_pExtendedFont != NULL)
	{
		m_pExtendedFont->RenderText(m_rcExtTextPosS, m_szSpf);
		m_pExtendedFont->RenderText(m_rcExtTextPosMs, m_szMspf);
	}
}

void ScreenFps::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	// Read extended font

	if (LoadFont(&m_pExtendedFont, SZ_EXTENDED_FONT,
		&rRoot, m_pStyle) == false && m_pFont != NULL)
	{
		m_pFont->AddRef();
		m_pExtendedFont->Release();
		m_pExtendedFont = m_pFont;
	}

	// Read extended background

	LoadMaterialInstance(m_extendedBackground, SZ_EXTENDED_BACKGROUND,
		&rRoot, m_pStyle);

	// Cache normal background from default background (which is required)

	m_normalBackground = m_Background;

	// Cache text rectangles for the first time

	OnMove(m_ptPos);
}

void ScreenFps::OnThemeStyleChange(void)
{
	ScreenOverlapped::OnThemeStyleChange();

	// Reload extended font

	if (LoadFont(&m_pExtendedFont, SZ_EXTENDED_FONT,
		NULL, m_pStyle) == false && m_pFont != NULL)
	{
		m_pFont->AddRef();
		m_pExtendedFont->Release();
		m_pExtendedFont = m_pFont;
	}

	// Read extended background

	LoadMaterialInstance(m_extendedBackground, SZ_EXTENDED_BACKGROUND,
		NULL, m_pStyle);

	// Cache normal background from default background (which is required)

	m_normalBackground = m_Background;

	// Cache text rectangles

	OnMove(m_ptPos);
}

DWORD ScreenFps::GetMemoryFootprint(void) const
{
	return sizeof(ScreenFps) -
		sizeof(ScreenOverlapped) * 2 +
		ScreenOverlapped::GetMemoryFootprint();
}

void ScreenFps::OnMove(const POINT& rptOldPos)
{
	ScreenOverlapped::OnMove(rptOldPos);
	
	if (NULL == m_pFont)
		return;	

	// Cache fps text rect	

	m_rcTextPos.left = m_ptPos.x + 10;
	m_rcTextPos.right = m_ptPos.x + m_psSize.cx;
	m_rcTextPos.bottom = m_ptPos.y + m_psSize.cy;

	if (true == m_bExtended)
	{
		// Cache text rects

		m_rcTextPos.top = m_ptPos.y +
			(m_psSize.cy / 2 - m_pFont->GetLineSpacing()) / 2;

		m_rcExtTextPosS.left = m_ptPos.x + 10;
		m_rcExtTextPosS.top = m_rcTextPos.top + m_psSize.cy / 2 - 2;
		m_rcExtTextPosS.right = m_ptPos.x + m_psSize.cx;
		m_rcExtTextPosS.bottom = m_ptPos.y + m_psSize.cy;

		CopyRect(&m_rcExtTextPosMs, &m_rcExtTextPosS);

		m_rcExtTextPosMs.top += m_pExtendedFont->GetCharHeight() - 2;
	}
	else
	{
		m_rcTextPos.top = m_ptPos.y +
			(m_psSize.cy - m_pFont->GetLineSpacing()) / 2;
	}
}
