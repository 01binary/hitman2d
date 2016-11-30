/*------------------------------------------------------------------*\
|
| ScreenPause.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Pause Screen implementation
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
#include "ScreenStart.h"	// using ScreenStart
#include "ScreenPause.h"	// defining ScreenPause

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenPause::SZ_VERTLOGOOFFSET[] =		L"logo.verticaloffset";
const WCHAR ScreenPause::SZ_HORZLOGOOFFSET[] =		L"logo.horizontaloffset";
const WCHAR ScreenPause::SZ_VERTBUTTONOFFSET[] =	L"buttons.verticaloffset";
const WCHAR ScreenPause::SZ_HORZBUTTONOFFSET[] =	L"buttons.horizontaloffset";
const WCHAR ScreenPause::SZ_CLASS[] =				L"overlapped::pause";


/*----------------------------------------------------------*\
| ScreenPause class implementation
\*----------------------------------------------------------*/

ScreenPause::ScreenPause(Engine& rEngine,
						 LPCWSTR pszClass,
						 Screen* pParent):

						 ScreenOverlapped(rEngine, pszClass, pParent),
						 m_pSelection(NULL),
						 m_fBlurFactor(1.5f),
						 m_Blur(rEngine)
{
}

ScreenPause::~ScreenPause(void)
{
	// Release material instance before texture will be released

	m_OverlayInst.Empty();
}

Object* ScreenPause::CreateInstance(Engine& rEngine,
									LPCWSTR pszClass,
									Object* pParent)
{
	return new ScreenPause(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenPause::OnRenderBackground(Graphics& rGraphics)
{
	// Render blurred background

	Color clrBlend = m_clrBackColor;

	if (FADE_STATE_OPENING == m_nFadeState ||
		FADE_STATE_CLOSING == m_nFadeState)
		clrBlend.SetAlpha(m_clrBlend.GetAlpha());

	rGraphics.RenderQuad(m_OverlayInst, m_vecCachedPos, clrBlend);
}

void ScreenPause::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	CacheLayout(&rRoot);
}

void ScreenPause::OnThemeStyleChange(void)
{
	ScreenOverlapped::OnThemeStyleChange();

	CacheLayout();
}

void ScreenPause::CacheLayout(const InfoElem* pDocRoot)
{
	// Background must be specified

	if (m_Background.IsEmpty() == true)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_Background.m_pSharedInstance");

	// Read horizontal/vertical offset of the logo

	const Variable* pVar = NULL;
	
	LoadVariable(&pVar, SZ_VERTLOGOOFFSET,
		Variable::TYPE_INT, Variable::TYPE_INT, pDocRoot, m_pStyle);

	int nLogoOffsetVert = (pVar != NULL) ? pVar->GetIntValue() : 0;

	LoadVariable(&pVar, SZ_HORZLOGOOFFSET,
		Variable::TYPE_INT, Variable::TYPE_INT, pDocRoot, m_pStyle);

	int nLogoOffsetHorz = (pVar != NULL) ? pVar->GetIntValue() : 0;

	// Read horizontal/vertical offset of the buttons

	LoadVariable(&pVar, SZ_VERTBUTTONOFFSET,
		Variable::TYPE_INT, Variable::TYPE_INT, pDocRoot, m_pStyle);

	int nButtonOffsetVert = (pVar != NULL) ? pVar->GetIntValue() : 0;

	LoadVariable(&pVar, SZ_HORZBUTTONOFFSET,
		Variable::TYPE_INT, Variable::TYPE_INT, pDocRoot, m_pStyle);

	int nButtonOffsetHorz = (pVar != NULL) ? pVar->GetIntValue() : 0;

	// Calculate max height and width taken by logos

	int nMaxLogoHeight = 0;
	int nMaxWidth = 0;

	for(int nID = ID_IMAGE_FIRST; nID <= ID_IMAGE_LAST; nID++)
	{
		Screen* pImage = m_lstChildren.FindByID(nID);

		if (pImage != NULL)
		{
			if (nMaxLogoHeight < pImage->GetSize().cy)
				nMaxLogoHeight = pImage->GetSize().cy;

			nMaxWidth += pImage->GetSize().cx;
		}
	}

	// Calculate total height and max width taken by buttons

	int nButtonsHeight = 0;

	for(int nID = ID_BUTTON_FIRST; nID <= ID_BUTTON_LAST; nID++)
	{
		Screen* pButton = m_lstChildren.FindByID(nID);

		if (pButton != NULL)
		{
			nButtonsHeight += pButton->GetSize().cy;

			if (pButton->GetSize().cx > nMaxWidth)
				nMaxWidth = pButton->GetSize().cx;
		}
	}

	// Calculate vertical and horizontal position for all controls

	int nTop = (m_psSize.cy - (nButtonsHeight + nMaxLogoHeight)) / 2;
	int nLeft = (m_psSize.cx - nMaxWidth) / 2;

	// Automatically position controls

	Screen* pImageLogoText = m_lstChildren.FindByID(ID_IMAGE_FIRST);
	Screen* pImageLogo = m_lstChildren.FindByID(ID_IMAGE_LAST);

	if (pImageLogoText != NULL && pImageLogo != NULL)
	{
		int nRelX = pImageLogo->GetPosition().x -
			pImageLogoText->GetPosition().x;

		int nRelY = pImageLogo->GetPosition().y -
			pImageLogoText->GetPosition().y;

		pImageLogoText->SetPosition(nLeft + nLogoOffsetHorz, nTop);
		pImageLogo->SetPosition(nLeft + nRelX + nLogoOffsetHorz, nTop + nRelY);

		nTop += (nMaxLogoHeight + nLogoOffsetVert);
	}

	nLeft += nButtonOffsetHorz;

	for(int nID = ID_BUTTON_FIRST; nID <= ID_BUTTON_LAST; nID++)
	{
		ScreenButton* pButton =
			dynamic_cast<ScreenButton*>(m_lstChildren.FindByID(nID));

		if (pButton != NULL)
		{
			pButton->SetPosition(nLeft, nTop);

			if (pButton->GetFont() != NULL)
				pButton->SetSize(pButton->GetSize().cx,
					pButton->GetFont()->GetCharHeight());

			nTop += (pButton->GetSize().cy + nButtonOffsetVert);
		}
	}

	// Get the selection image

	m_pSelection = m_lstChildren.FindByID(0);

	// Load blur material

	if (LoadMaterialInstance(m_BlurInst, ScreenStart::SZ_BLURMATERIAL,
		pDocRoot, m_pStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_BlurInst");
	}

	// Read blur factor

	if (LoadVariable(&pVar, ScreenStart::SZ_BLURFACTOR, Variable::TYPE_FLOAT,
		Variable::TYPE_INT, pDocRoot, m_pStyle) == true)
	{
		if (pVar->GetVarType() == Variable::TYPE_FLOAT)
			m_fBlurFactor = pVar->GetFloatValue();
		else
			m_fBlurFactor = float(pVar->GetIntValue());
	}

	// Create final render target to contain blurred image

	m_Blur.Allocate(m_psSize.cx, m_psSize.cy);

	if (m_Blur.GetRefCount() == 0)
		m_Blur.AddRef();

	// Cache blurred background

	CacheOverlay();
}

void ScreenPause::OnLostDevice(bool bRecreate)
{
	m_Blur.OnLostDevice(bRecreate);
}

void ScreenPause::OnResetDevice(bool bRecreate)
{
	m_Blur.OnResetDevice(bRecreate);

	CacheOverlay();
}

void ScreenPause::CacheOverlay(void)
{
	// Create temp target to contain baked image

	TextureRenderTarget temp(m_rEngine);

	temp.Allocate(m_psSize.cx >> 1, m_psSize.cy >> 1);
	temp.AddRef();

	// Downsample the image of everything
	// behind this screen to the temp texture...

	temp.BeginScene();

	m_rEngine.GetGraphics().Clear(m_rEngine.GetBackColorConst());

	// Temporarily set view matrix for downsampling

	D3DXMATRIX mtx;
	D3DXMatrixScaling(&mtx, 0.5f, 0.5f, 1.0f);

	D3DXMATRIX mtxOld =
		m_rEngine.GetGraphics().GetStates()->GetTransform(D3DTS_VIEW);

	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_VIEW, &mtx);

	// Render on temp target

	m_rEngine.GetGraphics().BeginBatch();

	ScreenManager& rScreens = m_rEngine.GetScreens();

	for(ScreenListIterator pos = rScreens.GetBeginPos();
		pos != rScreens.GetEndPos() && *pos != this;
		pos++)
	{
		(*pos)->Render();
	}

	m_rEngine.GetGraphics().EndBatch();

	// Restore view matrix

	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_VIEW, &mtxOld);

	temp.EndScene();

	// Render temp texture on quad with blur material to final texture
	// This effectively 'bakes' the blur effect to increase run-time speed

	m_BlurInst.SetBaseTexture(&temp);

	m_BlurInst.SetTextureCoords(0, 0,
		m_psSize.cx >> 1, m_psSize.cy >> 1);

	m_BlurInst.SetVector2(SZ_SHADERCONST_TEXELSIZE,
		Vector2(1.0f / float(m_psSize.cx >> 1),
				1.0f / float(m_psSize.cy >> 1)));
	
	m_BlurInst.SetScalar(SZ_SHADERCONST_FACTOR, m_fBlurFactor);

	m_Blur.BeginScene();

	m_rEngine.GetGraphics().Clear(m_rEngine.GetBackColorConst());

	m_rEngine.GetGraphics().RenderQuad(m_BlurInst,
		Vector2(), Vector2(m_psSize));

	m_BlurInst.SetBaseTexture(NULL);

	// Calculate background tiling

	float fBackgroundHeight =
		float(m_Background.GetTextureCoords().GetHeight());

	int nBackgroundTile = int(ceilf(float(m_psSize.cy) /
			fBackgroundHeight));

	// Render tiling background

	m_rEngine.GetGraphics().BeginBatch();

	Vector2 vecPos;

	Vector2 vecSize(m_psSize.cx,
		m_Background.GetTextureCoords().GetHeight());

	for(int n = 0;
		n < nBackgroundTile;
		n++, vecPos.y = vecPos.y + fBackgroundHeight)
	{
		m_rEngine.GetGraphics().RenderQuad(m_Background,
			vecPos, vecSize);
	}

	m_rEngine.GetGraphics().EndBatch();

	m_Blur.EndScene();

	// Setup overlay material

	m_OverlayInst = m_Background;
	m_OverlayInst.SetBaseTexture(&m_Blur);
	m_OverlayInst.SetTextureCoords(GetClientRect());
}

int ScreenPause::OnNotify(int nNotifyID, Screen* pSender, int nParam)
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

DWORD ScreenPause::GetMemoryFootprint(void) const
{
	return sizeof(ScreenPause) -
		sizeof(ScreenOverlapped) * 2 +
		ScreenOverlapped::GetMemoryFootprint();
}

void ScreenPause::OnBeginFade(void)
{
	if (FADE_ACTION_IN == m_nFadeAction &&
		FADE_STATE_OPENING == m_nFadeState)
		m_rEngine.PauseSession(true);
}

void ScreenPause::OnEndFade(void)
{
	if (FADE_ACTION_OUT == m_nFadeAction &&
		FADE_STATE_CLOSING == m_nFadeState)
		m_rEngine.PauseSession(false);
}