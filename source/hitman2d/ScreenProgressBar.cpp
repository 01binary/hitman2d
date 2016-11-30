/*------------------------------------------------------------------*\
|
| ScreenProgressBar.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ProgressBar Control implementation
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
#include "ScreenProgressBar.h"	// defining ScreenProgressBar

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenProgressBar::SZ_LEFTCORNER[] =	L"left.material";
const WCHAR ScreenProgressBar::SZ_RIGHTCORNER[] =	L"right.material";
const WCHAR ScreenProgressBar::SZ_CENTERREPEAT[] =	L"center.material";
const WCHAR ScreenProgressBar::SZ_BLIP[] =			L"blip.material";
const WCHAR ScreenProgressBar::SZ_CLASS[] =			L"progressbar";


/*----------------------------------------------------------*\
| ScreenProgressBar implementation
\*----------------------------------------------------------*/

ScreenProgressBar::ScreenProgressBar(Engine& rEngine,
									 LPCWSTR pszClass,
									 Screen* pParent):

									 Screen(rEngine, pszClass, pParent),
									 m_nProgress(50),
									 m_nProgressMin(0),
									 m_nProgressMax(100),
									 m_nBlips(0),
									 m_nCenterWidth(0)
{
}

ScreenProgressBar::~ScreenProgressBar(void)
{
}

Object* ScreenProgressBar::CreateInstance(Engine& rEngine,
										  LPCWSTR pszClass,
										  Object* pParent)
{
	return new ScreenProgressBar(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenProgressBar::SetProgressMin(int nProgressMin)
{
	m_nProgressMin = nProgressMin;

	CacheProgress();
}

void ScreenProgressBar::SetProgressMax(int nProgressMax)
{
	m_nProgressMax = nProgressMax;

	CacheProgress();
}

void ScreenProgressBar::SetProgress(int nProgress)
{
	m_nProgress = nProgress;

	CacheProgress();
}

void ScreenProgressBar::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Render left corner

	rGraphics.RenderQuad(m_Left, m_vecCachedPos, GetFrontBufferBlend());

	// Render right corner

	rGraphics.RenderQuad(m_Right, m_vecRight, GetFrontBufferBlend());

	// Render center repeat

	rGraphics.RenderQuad(m_Center, m_vecCenter,
		Vector2(m_nCenterWidth,
				m_Center.GetTextureCoords().GetHeight()),
		GetFrontBufferBlend());

	// Render blips

	if (m_nBlips > 0)
	{
		Vector3 vecBlip = m_vecCenter;

		float fBlipWidth =
			float(m_Blip.GetTextureCoords().GetWidth());

		int nBlip = m_nBlips;

		while(nBlip-- > 0)
		{
			rGraphics.RenderQuad(m_Blip, vecBlip, GetFrontBufferBlend());

			vecBlip.x += fBlipWidth;
		}
	}
}

void ScreenProgressBar::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Read left corner material

	if (LoadMaterialInstance(m_Left, SZ_LEFTCORNER,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_LEFTCORNER,
			rRoot.GetDocumentConst().GetPath());
	}

	// Read right corner material

	if (LoadMaterialInstance(m_Right, SZ_RIGHTCORNER,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_RIGHTCORNER,
			rRoot.GetDocumentConst().GetPath());
	}

	// Read center repeat material

	if (LoadMaterialInstance(m_Center, SZ_CENTERREPEAT,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_CENTERREPEAT,
			rRoot.GetDocumentConst().GetPath());
	}

	// Read blip material

	if (LoadMaterialInstance(m_Blip, SZ_BLIP,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_BLIP,
			rRoot.GetDocumentConst().GetPath());
	}
}

void ScreenProgressBar::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Reload left corner material

	LoadMaterialInstance(m_Left, SZ_LEFTCORNER,
		NULL, m_pStyle, pSharedStyle);

	// Reload right corner material

	LoadMaterialInstance(m_Right, SZ_RIGHTCORNER,
		NULL, m_pStyle, pSharedStyle);

	// Reload center repeat material

	LoadMaterialInstance(m_Center, SZ_CENTERREPEAT,
		NULL, m_pStyle, pSharedStyle);

	// Reload blip material

	LoadMaterialInstance(m_Blip, SZ_BLIP,
		NULL, m_pStyle, pSharedStyle);
}

DWORD ScreenProgressBar::GetMemoryFootprint(void) const
{
	return sizeof(ScreenProgressBar) - sizeof(Screen) * 2 +
		   Screen::GetMemoryFootprint();
}

void ScreenProgressBar::OnMove(const POINT& rptOldPos)
{
	m_vecCenter = m_vecCachedPos;
	m_vecCenter.x += float(m_Left.GetTextureCoords().GetWidth());

	m_vecRight = m_vecCenter;
	m_vecRight.x += float(m_nCenterWidth);
}

void ScreenProgressBar::OnSize(const SIZE& rpsOldSize)
{
	m_nCenterWidth = m_psSize.cx -
		m_Left.GetTextureCoords().GetWidth() -
		m_Right.GetTextureCoords().GetWidth();

	CacheProgress();
	
	m_vecRight = m_vecCenter;
	m_vecRight.x += float(m_nCenterWidth);
}

void ScreenProgressBar::OnMouseMove(POINT pt)
{
	// Adjust point for screen space

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	// Call parent's handler

	if (m_pParent != NULL)
		m_pParent->OnMouseMove(pt);
}

void ScreenProgressBar::OnMouseLDown(POINT pt)
{
	// Adjust point for screen space

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	// Call parent's handler

	if (m_pParent != NULL)
		m_pParent->OnMouseLDown(pt);
}

void ScreenProgressBar::OnMouseLUp(POINT pt)
{
	// Adjust point for screen space

	pt.x += m_ptPos.x;
	pt.y += m_ptPos.y;

	// Call parent's handler

	if (m_pParent != NULL)
		m_pParent->OnMouseLUp(pt);
}

void ScreenProgressBar::CacheProgress(void)
{
	float fProgress =
		float(m_nProgress + m_nProgressMin) / float(m_nProgressMax);

	int nBlipWidth = m_Blip.GetTextureCoords().GetWidth();

	if (0 == nBlipWidth)
		return;

	if (0 == m_nCenterWidth)
		return;

	m_nBlips = int(float(m_nCenterWidth / nBlipWidth) * fProgress);
}