/*------------------------------------------------------------------*\
|
| ScreenStart.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Start Screen implementation
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
#include "ScreenButton.h"	// using ScreenButton
#include "ScreenStart.h"	// defining ScreenStart

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Class Name
const WCHAR ScreenStart::SZ_CLASS[] =			L"overlapped::start";

// Element Names
const WCHAR ScreenStart::SZ_BLURFACTOR[] =		L"blurfactor";
const WCHAR ScreenStart::SZ_BLURMATERIAL[] =	L"blur.material";


/*----------------------------------------------------------*\
| ScreenStart class - main menu
\*----------------------------------------------------------*/

ScreenStart::ScreenStart(Engine& rEngine,
						 LPCWSTR pszClass,
						 Screen* pParent):

						 ScreenOverlapped(rEngine, pszClass, pParent),
						 m_bFirstTimeFade(true),
						 m_clrBlurBlend(255, 255, 255, 0),
						 m_fBlurFactor(1.1f),
						 m_pSelection(NULL),
						 m_Blur(rEngine)
{
}

ScreenStart::~ScreenStart(void)
{
	// Force the instances to get emptied before the texture

	m_OverlayInst.Empty();
}

Object* ScreenStart::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenStart(rEngine, pszClass,
		dynamic_cast<Screen*>(pParent));
}

void ScreenStart::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	// Get selection image

	m_pSelection = m_lstChildren.FindByID(ID_SELECTION);

	if (m_pSelection != NULL &&
	   m_pSelection->IsFlagSet(Screen::INVISIBLE) == false)
		m_pSelection->SetFlag(Screen::INVISIBLE);

	// Load the blur shader

	if (LoadMaterialInstance(m_BlurInst, SZ_BLURMATERIAL,
		&rRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_BLURMATERIAL,
			rRoot.GetDocumentConst().GetPath());
	}

	// Read blur factor

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_BLURFACTOR, Variable::TYPE_FLOAT,
		Variable::TYPE_INT, &rRoot, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_FLOAT)
			m_fBlurFactor = pVar->GetFloatValue();
		else
			m_fBlurFactor = float(pVar->GetIntValue());
	}

	// Create final render target to contain blurred image

	const D3DXIMAGE_INFO& rInfo =
		m_Background.GetBaseTexture()->GetInfo();	

	m_Blur.Allocate(rInfo.Width, rInfo.Height);

	m_Blur.AddRef();

	CacheOverlay();
}

void ScreenStart::CacheOverlay(void)
{
	// Create half size temp target to contain downsampled image
	// The temp target has non-pow2 size to cause blur to sample with clamping

	TextureRenderTarget temp(m_rEngine);

	temp.Allocate(m_Background.GetTextureCoords().GetWidth() >> 1,
				  m_Background.GetTextureCoords().GetHeight() >> 1);

	temp.AddRef();

	// Downsample screen's image 2x to the temp texture...

	temp.BeginScene();

	m_rEngine.GetGraphics().Clear(m_rEngine.GetBackColorConst());

	// Temporarily reset position for rendering on temp target

	POINT ptOldPos = { m_ptPos.x, m_ptPos.y };
	SetPosition(0, 0);

	// Temporarily set 1/2 size scaling matrix to accomplish downsampling

	D3DXMATRIX mtx;
	D3DXMatrixScaling(&mtx, 0.5f, 0.5f, 0.0f);

	m_rEngine.GetGraphics().GetStates()->SetTransform(
		D3DTS_VIEW, &mtx);

	// Temporarily hide corners to prevent from
	// their inclusion in blur calculations

	for(int nCorner = ID_CORNER_FIRST;
		nCorner < ID_CORNER_LAST;
		nCorner++)
	{
		Screen* pScreen = m_lstChildren.FindByID(nCorner);

		if (pScreen != NULL)
			pScreen->SetFlag(Screen::INVISIBLE);
	}

	// Temporarily hide the image scroller if present

	Screen* pScroller = m_lstChildren.FindByClass(L"imagescroller");

	if (pScroller != NULL)
		pScroller->SetFlag(Screen::INVISIBLE);

	// Render on temp target
	// Make sure to render the client area without any overlays
	// Otherwise we'll crash with a blue screen!

	m_rEngine.GetGraphics().BeginBatch();

	ScreenOverlapped::OnRender(m_rEngine.GetGraphics());

	m_rEngine.GetGraphics().EndBatch();

	// Restore scale, but don't restore position yet

	D3DXMatrixIdentity(&mtx);
	m_rEngine.GetGraphics().GetStates()->SetTransform(
		D3DTS_VIEW, &mtx);

	temp.EndScene();

	// Render temp texture on quad with blur material to final texture
	// This effectively 'bakes' the blur effect to increase run-time speed

	m_BlurInst.SetBaseTexture(&temp);

	m_BlurInst.SetTextureCoords(0, 0,
		m_Background.GetTextureCoords().right >> 1,
		m_Background.GetTextureCoords().bottom >> 1);

	m_BlurInst.SetVector2(SZ_SHADERCONST_TEXELSIZE,
		Vector2(1.0f / float(m_Background.GetTextureCoords().GetWidth() >> 1),
				1.0f / float(m_Background.GetTextureCoords().GetHeight() >> 1)));

	m_BlurInst.SetScalar(SZ_SHADERCONST_FACTOR, m_fBlurFactor);

	m_Blur.BeginScene();

	m_rEngine.GetGraphics().Clear(Color::BLEND_ZERO);

	m_rEngine.GetGraphics().RenderQuad(m_BlurInst, Vector2(),
		Vector2(m_Background.GetTextureCoords().right,
				m_Background.GetTextureCoords().bottom));

	// Render corners again to make sure we have sharp corners
	// Corners must be image controls with IDs from 1001 to 1004.

	m_rEngine.GetGraphics().BeginBatch();

	for(int nCorner = ID_CORNER_FIRST;
		nCorner < ID_CORNER_LAST;
		nCorner++)
	{
		Screen* pScreen = m_lstChildren.FindByID(nCorner);

		if (pScreen != NULL)
		{
			pScreen->ClearFlag(Screen::INVISIBLE);
			pScreen->Render();
		}
	}

	// Show image scroller if exists

	if (pScroller != NULL)
		pScroller->ClearFlag(Screen::INVISIBLE);

	m_rEngine.GetGraphics().EndBatch();

	m_Blur.EndScene();

	m_BlurInst.SetBaseTexture(NULL);

	// Restore position

	SetPosition(ptOldPos);

	// Setup overlay material

	m_OverlayInst = m_Background;
	m_OverlayInst.SetBaseTexture(&m_Blur);
}

int ScreenStart::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	ScreenOverlapped::OnNotify(nNotifyID, pSender, nParam);

	if (ScreenButton::NOTIFY_SELECT == nNotifyID &&
	   m_pSelection != NULL)
	{
		switch(nParam)
		{
		case ScreenButton::NOTIFY_SELECT_NONE:
			{
				// Make sure selection image is invisible

				if (m_pSelection->IsFlagSet(Screen::INVISIBLE) == false)
					m_pSelection->SetFlag(Screen::INVISIBLE);
			}
			break;
		case ScreenButton::NOTIFY_SELECT_HOVER:
		case ScreenButton::NOTIFY_SELECT_PUSHED:
			{
				// Show selection only for main verbs

				if (pSender->GetID() < ID_BUTTON_FIRST ||
				   pSender->GetID() >= ID_BUTTON_LAST)
					break;

				// Move around the selection image

				m_pSelection->SetPosition(
					pSender->GetPosition().x - m_pSelection->GetSize().cx,
					pSender->GetPosition().y +
						(pSender->GetSize().cy - m_pSelection->GetSize().cy) / 2);

				// Make sure selection image is visible

				if (m_pSelection->IsFlagSet(Screen::INVISIBLE) == true)
					m_pSelection->ClearFlag(Screen::INVISIBLE);

				// Make sure its blend matches that of the button

				m_pSelection->SetBlend(pSender->GetBackColorConst());
			}
			break;
		}
	}

	return 0;
}

void ScreenStart::OnDeactivate(Screen* pNewActive)
{
	// If was fading in for the first time and cut midstream,
	// wrap up the fading

	m_nFadeAction = FADE_ACTION_NONE;
	m_bFirstTimeFade = false;

	ScreenOverlapped::OnDeactivate(pNewActive);
}

void ScreenStart::OnLostDevice(bool bRecreate)
{
	ScreenOverlapped::OnLostDevice(bRecreate);

	m_Blur.OnLostDevice(bRecreate);
}

void ScreenStart::OnResetDevice(bool bRecreate)
{
	ScreenOverlapped::OnResetDevice(bRecreate);

	m_Blur.OnResetDevice(bRecreate);

	CacheOverlay();
}

void ScreenStart::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	if (m_clrBlurBlend.GetAlpha() < Color::MAX_CHANNEL)
		ScreenOverlapped::OnRender(rGraphics, prc);

	// Render blur overlay if blurring

	if (m_clrBlurBlend.GetAlpha() > 0 && false == m_bFirstTimeFade)
		rGraphics.RenderQuad(m_OverlayInst, m_vecCachedPos, m_clrBlurBlend);
}

void ScreenStart::OnTimer(Timer& rTimer)
{
	if (rTimer.GetID() != TIMER_FADE)
	{
		ScreenOverlapped::OnTimer(rTimer);
		return;
	}

	if (true == m_bFirstTimeFade)
	{
		// If first time, fade in

		ScreenOverlapped::OnTimer(rTimer);

		if (FADE_ACTION_NONE == m_nFadeAction)
			m_bFirstTimeFade = false;
	}
	else
	{
		// If activating or deactivating, fade the overlay

		int nBlurAlpha = m_clrBlurBlend.GetAlpha();

		if (FADE_ACTION_IN == m_nFadeAction)
		{
			// Fade out overlay

			nBlurAlpha -= m_nScreenFadeStep;

			if (nBlurAlpha <= Color::MIN_CHANNEL)
			{
				nBlurAlpha = Color::MIN_CHANNEL;

				m_nFadeAction = FADE_ACTION_NONE;
				m_nFadeState = FADE_STATE_NONE;
				
				m_rEngine.GetTimers().Remove(this, TIMER_FADE);

				m_pFadeTimer = NULL;
			}
		}
		else
		{
			// Fade in overlay

			nBlurAlpha += m_nScreenFadeStep;

			if (nBlurAlpha >= Color::MAX_CHANNEL)
			{
				// Remove fading timer

				m_rEngine.GetTimers().Remove(this, TIMER_FADE);

				// If was closing, release

				if (FADE_STATE_CLOSING == m_nFadeState)
				{
					Release();
					return;
				}

				nBlurAlpha = Color::MAX_CHANNEL;

				m_nFadeAction = FADE_ACTION_NONE;
				m_nFadeState = FADE_STATE_NONE;
				m_pFadeTimer = NULL;
			}
		}

		m_clrBlurBlend.SetAlpha(nBlurAlpha);
	}
}

DWORD ScreenStart::GetMemoryFootprint(void) const
{
	return sizeof(ScreenStart) -
		sizeof(ScreenOverlapped) * 2 +
		ScreenOverlapped::GetMemoryFootprint();
}