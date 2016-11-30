/*------------------------------------------------------------------*\
|
| ScreenOverlapped.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScreenOverlapped implementation
| Created: 02/28/2011
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
#include "ScreenOverlapped.h"	// defining ScreenOverlapped

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenOverlapped::SZ_CLASS[] =			L"overlapped";
const WCHAR ScreenOverlapped::SZ_TABORDER[] =		L"taborder";
const WCHAR ScreenOverlapped::SZ_DEFAULTCOMMAND[] =	L"defaultcommand";
const WCHAR ScreenOverlapped::SZ_CANCELCOMMAND[] =	L"cancelcommand";
const WCHAR ScreenOverlapped::SZ_CAPMARGIN[] =		L"caption.margin";
const WCHAR ScreenOverlapped::SZ_TITLEMARGIN[] =	L"title.margin";
const WCHAR ScreenOverlapped::SZ_TITLE[] =			L"title";
const WCHAR ScreenOverlapped::SZ_TITLESTYLE[] =		L"title.style";
const WCHAR ScreenOverlapped::SZ_BORDERSTYLE[] =	L"border.style";

const LPCWSTR ScreenOverlapped::SZ_CAPSTYLE[] ={	
													L"capclose.style",
													L"capmin.style",
													L"capmax.style",
													L"capres.style",
													L"capbackground.style"
												};

const LPCWSTR ScreenOverlapped::SZ_FLAGS[] =	{
													L"fadeeffects",
													L"draggable",
													L"nofocuschildren",
													L"noforeground",
													L"title",
													L"border",
													L"captionclose"
												};

const DWORD ScreenOverlapped::DW_FLAGS[] =		{
													ScreenOverlapped::FADEEFFECTS,
													ScreenOverlapped::DRAGGABLE,
													ScreenOverlapped::NOFOCUSCHILDREN,												
													ScreenOverlapped::NOFOREGROUND,
													ScreenOverlapped::TITLE,
													ScreenOverlapped::BORDER,
													ScreenOverlapped::CAPTION_CLOSE
												};


/*----------------------------------------------------------*\
| ScreenOverlapped implementation
\*----------------------------------------------------------*/

ScreenOverlapped::ScreenOverlapped(Engine& rEngine, LPCWSTR pszClass, Screen* pParent):
								   Screen(rEngine, pszClass, pParent),
								   m_pFadeTimer(NULL),
								   m_nScreenFadeInactive(200),
								   m_nScreenFadeActive(255),
								   m_nScreenFadeStep(9),
								   m_fScreenFadeInterval(0.01f),
								   m_nFadeAlpha(255),
								   m_nFadeAction(FADE_ACTION_NONE),
								   m_nFadeState(FADE_STATE_OPENING),
								   m_nDragState(DRAG_STATE_NONE),
								   m_nDefaultCmdID(-1),
								   m_nCancelCmdID(-1),
								   m_pFont(NULL),
								   m_pLastFocus(NULL),
								   m_pBorder(NULL),
								   m_pTitle(NULL),
								   m_pCaptionBack(NULL)
{
	m_ptDragOffset.x = 0;
	m_ptDragOffset.y = 0;

	m_sizeResolution.cx = int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth);
	m_sizeResolution.cy = int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight);

	ZeroMemory(m_pCaption, sizeof(m_pCaption));

	OnThemeChange(rEngine.GetScreens().GetTheme());
}

ScreenOverlapped::~ScreenOverlapped(void)
{
	// Remove timer

	m_rEngine.GetTimers().Remove(this, TIMER_FADE);
}

Object* ScreenOverlapped::CreateInstance(Engine& rEngine,
										 LPCWSTR pszClass,
										 Object* pParent)
{
	return new ScreenOverlapped(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

ScreenOverlapped* ScreenOverlapped::GetOverlappedParent(Screen* pScreen)
{
	Screen* pOverlappedParent = pScreen->GetParent();

	while (pOverlappedParent != NULL &&
		   dynamic_cast<ScreenOverlapped*>(pOverlappedParent) == NULL)
	{
		pOverlappedParent = pOverlappedParent->GetParent();
	}

	return dynamic_cast<ScreenOverlapped*>(pOverlappedParent);
}

void ScreenOverlapped::SetTitle(LPCWSTR pszTitle)
{
	m_strTitle = pszTitle;

	dynamic_cast<ScreenLabel*>(m_pTitle)->SetText(pszTitle);
}

void ScreenOverlapped::SetFont(Font* pFont)
{
	if (m_pFont != NULL)
		m_pFont->Release();

	m_pFont = pFont;

	pFont->AddRef();
}

void ScreenOverlapped::SetMnemonic(Screen* pScreen, LPCWSTR pszText)
{
	while (*pszText != '\0' && !(pszText[0] == '&' && pszText[1] != '&'))
		pszText++;

	if (*pszText == '&' && pszText[1] != '&' && pszText[1] != '\0')
	{
		ScreenOverlapped* pOverlapped = ScreenOverlapped::GetOverlappedParent(pScreen);

		if (pOverlapped != NULL)
		{
			pOverlapped->SetMnemonic(pScreen, tolower(pszText[1]));
		}
	}
}

void ScreenOverlapped::Tab(bool bNext)
{
	if (m_arTabOrder.empty() == true) return;

	// If currently focused screen is us, set to first control in tab order

	if (m_rEngine.GetScreens().GetFocusScreen() == this)
		m_rEngine.GetScreens().SetFocusScreen(m_arTabOrder.front());
	
	// Find currently focused control in tab order

	for(ScreenListIterator pos = m_arTabOrder.begin();
		pos != m_arTabOrder.end();
		pos++)
	{
		if (*pos == m_rEngine.GetScreens().GetFocusScreen() ||
		   m_rEngine.GetScreens().GetFocusScreen()->IsDescendantOf(*pos))
		{
			if (true == bNext)
			{
				do
				{
					// Set focus to the next control in tab order

					pos++;

					// Wrap if required

					if (pos == m_arTabOrder.end())
						pos = m_arTabOrder.begin();

					// Keep going if disabled or invisible

				} while (!(*pos)->IsInteractive());
			}
			else
			{
				do
				{
					// Set focus to the previous control in tab order

					if (pos == m_arTabOrder.begin())
					{
						// Wrap if required

						pos = --m_arTabOrder.end();
					}
					else
					{
						pos--;
					}

					// Keep going if disabled or invisible

				} while (!(*pos)->IsInteractive());
			}

			m_rEngine.GetScreens().SetFocusScreen(*pos);

			return;
		}
	}
}

void ScreenOverlapped::Close(void)
{
	m_nFadeState = FADE_STATE_CLOSING;

	Deactivate();
}

int ScreenOverlapped::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
	switch(nNotifyID)
	{
	case ScreenButton::NOTIFY_SELECT:
		{
			if (0 == nParam)
				return false;

			ThemeStyle* pShared =
				m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

			if (NULL == pShared)
				return false;

			const Variable* pVar =
				pShared->GetVariable(nParam == ScreenButton::NOTIFY_SELECT_HOVER ?
					SZ_VAR_SNDSCREENSELECT :
					SZ_VAR_SNDSCREENACTION);

			if (NULL == pVar || pVar->GetVarType() != Variable::TYPE_STRING)
				return false;

			if (String::IsEmpty(pVar->GetStringValue()) == true)
				return true;

			Sound* pSound = NULL;

			try
			{
				pSound = m_rEngine.GetSounds().Load(pVar->GetStringValue());

				if (NULL == pSound)
					return true;
			}

			catch(std::exception)
			{
				Game::PrintLastError(m_rEngine);
				return false;
			}

			try
			{
				SoundInstance* pSoundInstance = pSound->Play(false);

				Game* pGame =
					dynamic_cast<Game*>(m_rEngine.GetClientInstance());

				if (pGame != NULL)
					pSoundInstance->SetVolume(pGame->GetEffectsVolume());
			}

			catch(std::exception)
			{
				Game::PrintLastError(m_rEngine);

				return false;
			}
		}
		break;
	}

	return true;
}

void ScreenOverlapped::OnLostDevice(bool bRecreate)
{
	Screen::OnLostDevice(bRecreate);

	// Remember last resolution
	m_sizeResolution.cx = int(m_rEngine.GetGraphics().GetLastDeviceParams().BackBufferWidth);
	m_sizeResolution.cy = int(m_rEngine.GetGraphics().GetLastDeviceParams().BackBufferHeight);
}

void ScreenOverlapped::OnResetDevice(bool bRecreate)
{
	Screen::OnResetDevice(bRecreate);

	if (!m_bCenter)
	{
		// Re-position the screen when resolution changes
		float left = float(m_ptPos.x) / float(m_sizeResolution.cx);
		float top = float(m_ptPos.y) / float(m_sizeResolution.cy);

		float newWidth = float(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth);
		float newHeight = float(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight);

		this->SetPosition(int(left * newWidth), int(top * newHeight));
	}
}

void ScreenOverlapped::OnThemeChange(Theme &rNewTheme)
{
	Screen::OnThemeChange(rNewTheme);

	// Get shared style

	ThemeStyle* pShared =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Reload variables

	if (pShared != NULL)
	{
		const Variable* pVar = pShared->GetVariable(SZ_VAR_SCREENFADEACTIVE,
			Variable::TYPE_INT);

		if (pVar != NULL)
			m_nScreenFadeActive = pVar->GetIntValue();

		pVar = pShared->GetVariable(SZ_VAR_SCREENFADEINACTIVE, Variable::TYPE_INT);

		if (pVar != NULL)
			m_nScreenFadeInactive = pVar->GetIntValue();

		pVar = pShared->GetVariable(SZ_VAR_SCREENFADESTEP, Variable::TYPE_INT);

		if (pVar != NULL)
			m_nScreenFadeStep = pVar->GetIntValue();

		pVar = pShared->GetVariable(SZ_VAR_SCREENFADEINTERVAL, Variable::TYPE_FLOAT);

		if (pVar != NULL)
			m_fScreenFadeInterval = pVar->GetFloatValue();
	}
}

void ScreenOverlapped::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Reload font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, NULL, m_pStyle,
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE));

	const Variable* pVar = NULL;

	// Re-create or modify title

	if (IsFlagSet(TITLE) == true)
	{
		if (NULL == m_pTitle)
		{
			m_pTitle = dynamic_cast<Screen*>(ScreenLabel::CreateInstance(m_rEngine,
				ScreenLabel::SZ_CLASS, this));

			m_pTitle->SetID(ID_TITLE);
			m_pTitle->SetFlag(DISABLED);

			m_lstChildren.Add(m_pTitle);

			dynamic_cast<ScreenLabel*>(m_pTitle)->SetText(m_strTitle);
		}

		if (LoadVariable(&pVar, SZ_TITLESTYLE, Variable::TYPE_STRING,
			Variable::TYPE_STRING, NULL, m_pStyle) == true)
		{		
			m_pTitle->SetStyle(pVar->GetStringValue());
		}
	}

	// Re-create or modify border

	if (IsFlagSet(BORDER) == true)
	{
		if (NULL == m_pBorder)
		{
			m_pBorder = dynamic_cast<Screen*>(ScreenFrame::CreateInstance(m_rEngine,
				ScreenFrame::SZ_CLASS, this));

			m_pBorder->SetID(ID_MAINFRAME);
			m_pBorder->SetFlags(ScreenFrame::AUTOSIZE | DISABLED);

			m_lstChildren.Add(m_pBorder);
		}

		if (LoadVariable(&pVar, SZ_BORDERSTYLE, Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pBorder->SetStyle(pVar->GetStringValue());
		}
	}

	// Re-create or modify caption button(s)

	if (IsFlagSet(CAPTION_CLOSE) == true)
	{
		if (NULL == m_pCaption[2])
		{
			m_pCaption[2] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
				ScreenButton::SZ_CLASS, this));

			m_pCaption[2]->SetID(ID_CLOSE);

			m_lstChildren.Add(m_pCaption[2]);
		}

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_CLOSE], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[2]->SetStyle(pVar->GetStringValue());
		}
	}

	if (IsFlagSet(CAPTION_MAX) == true)
	{
		if (NULL == m_pCaption[1])
		{
			m_pCaption[1] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
				ScreenButton::SZ_CLASS, this));

			m_pCaption[1]->SetID(ID_MAX_RESTORE);

			m_lstChildren.Add(m_pCaption[1]);
		}

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_RESTORE], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[1]->SetStyle(pVar->GetStringValue());
		}
	}

	if (IsFlagSet(CAPTION_MIN) == true)
	{
		if (NULL == m_pCaption[0])
		{
			m_pCaption[0] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
				ScreenButton::SZ_CLASS, this));

			m_pCaption[0]->SetID(ID_MIN);

			m_lstChildren.Add(m_pCaption[0]);
		}

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_MIN], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[0]->SetStyle(pVar->GetStringValue());
		}
	}
}

DWORD ScreenOverlapped::GetMemoryFootprint(void) const
{
	return sizeof(ScreenOverlapped) - sizeof(Screen) * 2 +
		Screen::GetMemoryFootprint();
}

void ScreenOverlapped::OnBeginDrag(void)
{
	ThemeStyle* pShared =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	if (NULL == pShared)
		return;

	const Variable* pVar =
		pShared->GetVariable(SZ_VAR_SNDSCREENSTARTDRAG);

	if (pVar != NULL &&
		pVar->GetVarType() == Variable::TYPE_STRING &&
		String::IsEmpty(pVar->GetStringValue()) == false)
	{
		Sound* pSnd =
			m_rEngine.GetSounds().Load(pVar->GetStringValue());

		if (pSnd != NULL)
		{
			Game* pGame =
				dynamic_cast<Game*>(m_rEngine.GetClientInstance());

			if (NULL == pGame) return;

			pSnd->Play()->SetVolume(pGame->GetEffectsVolume());
		}
		else
		{
			Game::PrintLastError(m_rEngine);
		}
	}
}

void ScreenOverlapped::OnEndDrag(void)
{
	ThemeStyle* pShared =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	if (NULL == pShared)
		return;

	const Variable* pVar =
		pShared->GetVariable(SZ_VAR_SNDSCREENENDDRAG);

	if (pVar != NULL &&
	   pVar->GetVarType() == Variable::TYPE_STRING &&
	   String::IsEmpty(pVar->GetStringValue()) == false)
	{
		Sound* pSnd =
			m_rEngine.GetSounds().Load(pVar->GetStringValue());

		if (pSnd != NULL)
		{
			Game* pGame =
				dynamic_cast<Game*>(m_rEngine.GetClientInstance());

			if (NULL == pGame) return;

			pSnd->Play()->SetVolume(pGame->GetEffectsVolume());
		}
		else
		{
			Game::PrintLastError(m_rEngine);
		}
	}
}

void ScreenOverlapped::OnSize(const SIZE& rpsOldSize)
{
	// Resize frame if present

	if (m_pBorder != NULL)
		m_pBorder->OnSize(rpsOldSize);

	// Get margin

	int nMargin = 0, nTitleMargin = 0, nCapMargin = 0;

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_SCREEN_MARGIN, Variable::TYPE_INT,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			nMargin = pVar->GetIntValue();

	if (LoadVariable(&pVar, SZ_TITLEMARGIN, Variable::TYPE_INT,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			nTitleMargin = pVar->GetIntValue();

	if (LoadVariable(&pVar, SZ_CAPMARGIN, Variable::TYPE_INT,
		Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
			nCapMargin = pVar->GetIntValue();

	// Re-position title if present

	if (m_pTitle != NULL)
	{
		Font* pTitleFont = dynamic_cast<ScreenLabel*>(m_pTitle)->GetFont();

		m_pTitle->SetPosition(nMargin, nTitleMargin);

		if (pTitleFont != NULL)
			m_pTitle->SetSize(m_psSize.cx - nMargin * 2,
				pTitleFont->GetCharHeight());
	}

	// Re-position caption button background

	if (m_pCaptionBack != NULL)
	{
		m_pCaptionBack->SetPosition(m_psSize.cx - nCapMargin -
			m_pCaptionBack->GetBackgroundConst().GetTextureCoords().GetWidth(),
			nCapMargin);

		m_pCaptionBack->SetSize(
			m_pCaptionBack->GetBackgroundConst().GetTextureCoords().GetWidth(),
			m_pCaptionBack->GetBackgroundConst().GetTextureCoords().GetHeight());
	}

	// Re-position caption button(s)

	if (m_pCaption[2] != NULL)
	{
		m_pCaption[2]->SetPosition(m_psSize.cx - nCapMargin -
			m_pCaption[2]->GetBackgroundConst().GetTextureCoords().GetWidth(),
			nCapMargin);

		m_pCaption[2]->SetSize(
			m_pCaption[2]->GetBackgroundConst().GetTextureCoords().GetWidth(),
			m_pCaption[2]->GetBackgroundConst().GetTextureCoords().GetHeight());
	}

	for(int n = 1; n >= 0; n--)
	{
		if (NULL == m_pCaption[n]) continue;

		if (m_pCaption[n + 1] != NULL)
		{
			m_pCaption[n]->SetPosition(m_pCaption[n + 1]->GetPosition().x -
				m_pCaption[n + 1]->GetSize().cx - nCapMargin,
				nCapMargin);
		}
		else
		{
			m_pCaption[n]->SetPosition(m_psSize.cx - nCapMargin -
				m_pCaption[n]->GetBackgroundConst().GetTextureCoords().GetWidth(),
				nCapMargin);
		}

		m_pCaption[n]->SetSize(
			m_pCaption[n]->GetBackgroundConst().GetTextureCoords().GetWidth(),
			m_pCaption[n]->GetBackgroundConst().GetTextureCoords().GetHeight());
	}
}

void ScreenOverlapped::OnBeginFade(void)
{
	// Default handler
}

void ScreenOverlapped::OnEndFade(void)
{
	// Default handler
}

void ScreenOverlapped::OnActivate(Screen* pOldActive)
{
	// Set as foreground

	if (IsFlagSet(NOFOREGROUND) == false)
		m_rEngine.GetScreens().SetForegroundScreen(this);

	// If focused screen is not us or a our descendant, set focus to us

	if (m_rEngine.GetScreens().GetFocusScreen() == NULL ||
	   (m_rEngine.GetScreens().GetFocusScreen() != this &&
	   m_rEngine.GetScreens().GetFocusScreen()->IsDescendantOf(this) == false))
	{
		m_rEngine.GetScreens().SetFocusScreen(this);
	}

	// If capture screen is not us or our descendant, reset capture

	if (m_rEngine.GetScreens().GetCaptureScreen() != NULL &&
	   (m_rEngine.GetScreens().GetCaptureScreen() != this &&
	   m_rEngine.GetScreens().GetCaptureScreen()->IsDescendantOf(this) == false))
	{
		m_rEngine.GetScreens().SetCaptureScreen(NULL);
	}

	// Start fading in

	if (IsFlagSet(FADEEFFECTS) == true)
	{
		m_nFadeAction = FADE_ACTION_IN;

		if (NULL == m_pFadeTimer)
		{
			m_pFadeTimer =
				m_rEngine.GetTimers().Add(this,
										  m_fScreenFadeInterval,
										  TIMER_FADE);

			m_nFadeAlpha = (FADE_STATE_OPENING == m_nFadeState) ?
							0 : m_nScreenFadeInactive;

			OnBeginFade();
		}
	}
	else
	{
		m_nFadeAction = FADE_ACTION_IN;
		
		OnBeginFade();
		OnEndFade();
	}

	if (FADE_STATE_OPENING == m_nFadeState)
		m_nFadeState = FADE_STATE_NONE;

	// Play activation sound

	if (NULL == pOldActive)
	{
		ThemeStyle* pShared =
			m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

		if (NULL == pShared)
			return;

		const Variable* pVar =
			pShared->GetVariable(SZ_VAR_SNDSCREENSELECT);

		if (pVar != NULL &&
		   pVar->GetVarType() == Variable::TYPE_STRING &&
		   String::IsEmpty(pVar->GetStringValue()) == false)
		{
			try
			{
				Sound* pSnd =
					m_rEngine.GetSounds().Load(pVar->GetStringValue());

				if (NULL == pSnd)
					return;

				Game* pGame =
					dynamic_cast<Game*>(m_rEngine.GetClientInstance());

				if (NULL == pGame)
					return;

				pSnd->Play()->SetVolume(pGame->GetEffectsVolume());
			}

			catch(Error& rError)
			{
				UNREFERENCED_PARAMETER(rError);

				Game::PrintLastError(m_rEngine);
			}
		}
	}
}

void ScreenOverlapped::OnDeactivate(Screen* pNewActive)
{
	// Play deactivation sound

	if (NULL == pNewActive)
	{
		ThemeStyle* pShared =
			m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

		if (NULL == pShared)
			return;

		const Variable* pVar =
			pShared->GetVariable(SZ_VAR_SNDSCREENDEACTIVATE);

		if (pVar != NULL &&
		   pVar->GetVarType() == Variable::TYPE_STRING &&
		   String::IsEmpty(pVar->GetStringValue()) == false)
		{
			try
			{
				Sound* pSnd = m_rEngine.GetSounds().Load(pVar->GetStringValue());

				Game* pGame =
					dynamic_cast<Game*>(m_rEngine.GetClientInstance());

				if (NULL == pGame) return;

				if (pSnd != NULL)
					pSnd->Play()->SetVolume(pGame->GetEffectsVolume());
			}

			catch(Error& rError)
			{
				UNREFERENCED_PARAMETER(rError);

				Game::PrintLastError(m_rEngine);
			}
		}
	}

	// Start fading out

	m_nFadeAction = FADE_ACTION_OUT;

	if (IsFlagSet(FADEEFFECTS) == true)
	{
		Game* pGame =
			dynamic_cast<Game*>(m_rEngine.GetClientInstance());

		if (NULL == pGame) return;		

		if (NULL == m_pFadeTimer)
		{
			m_pFadeTimer =
				m_rEngine.GetTimers().Add(this,
					m_fScreenFadeInterval,
					TIMER_FADE);

			OnBeginFade();
		}
	}
	else if (FADE_STATE_CLOSING == m_nFadeState)
	{
		// Close immediately

		OnBeginFade();
		OnEndFade();

		Release();

		return;
	}

	// If currently focused screen is an eventual child of this screen,
	// save it as a last focused screen so that we can set focus on it when
	// this screen is focused once again

	Screen* pFocusScreen = m_rEngine.GetScreensConst().GetFocusScreen();

	if (pFocusScreen != NULL && pFocusScreen != this)
	{
		Screen* pFocusParent = pFocusScreen->GetParent();

		if (pFocusParent != NULL)
		{
			while(pFocusParent->GetParent() != NULL)
				pFocusParent = pFocusParent->GetParent();

			if (this == pFocusParent)
				m_pLastFocus = pFocusScreen;
		}
	}
}

void ScreenOverlapped::OnFocus(Screen* pOldFocus)
{
	// If any child already has focus, we're good

	if (pOldFocus != NULL && pOldFocus->IsDescendantOf(this))
	{
		return;
	}

	if (m_pLastFocus != NULL)
	{
		// If there was a focused child, focus that child

		m_rEngine.GetScreens().SetFocusScreen(m_pLastFocus);

		m_pLastFocus = NULL;
	}
	else
	{
		// Otherwise, set focus to the first control in tab order

		if (m_arTabOrder.empty() == true ||
			IsFlagSet(NOFOCUSCHILDREN)) return;

		if (m_arTabOrder.front() != NULL)
			m_rEngine.GetScreens().SetFocusScreen(m_arTabOrder.front());
	}
}

void ScreenOverlapped::OnTimer(Timer& rTimer)
{
	if (rTimer.GetID() == TIMER_FADE)
	{
		Game* pGame =
			dynamic_cast<Game*>(m_rEngine.GetClientInstance());

		if (NULL == pGame)
			return;

		// Fade in or fade out

		if (FADE_ACTION_IN == m_nFadeAction)
		{
			m_nFadeAlpha += m_nScreenFadeStep;

			if (m_nFadeAlpha > m_nScreenFadeActive)
			{
				m_rEngine.GetTimers().Remove(this, TIMER_FADE);
				m_pFadeTimer = NULL;

				OnEndFade();

				m_nFadeAction = FADE_ACTION_NONE;

				m_nFadeAlpha = m_nScreenFadeActive;
			}

			// Set alpha

			m_clrBlend.SetAlpha(m_nFadeAlpha);
		}
		else if (FADE_ACTION_OUT == m_nFadeAction)
		{
			m_nFadeAlpha -= m_nScreenFadeStep;

			if (m_nFadeAlpha <=
			  (FADE_STATE_CLOSING == m_nFadeState ? 0 : m_nScreenFadeInactive))
			{
				m_rEngine.GetTimers().Remove(this, TIMER_FADE);
				m_pFadeTimer = NULL;

				OnEndFade();

				m_nFadeAction = FADE_ACTION_NONE;

				m_nFadeAlpha = (FADE_STATE_CLOSING == m_nFadeState ?
								0 : m_nScreenFadeInactive);

				if (FADE_STATE_CLOSING == m_nFadeState)
				{
					Release();
					return;
				}
			}

			// Set alpha

			m_clrBlend.SetAlpha(m_nFadeAlpha);
		}
	}
}

void ScreenOverlapped::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	if (ID_CLOSE == nCommandID)
		Close();
}

void ScreenOverlapped::OnKeyPress(int nAsciiCode, bool extended, bool alt)
{
	
}

void ScreenOverlapped::OnKeyDown(int nKeyCode)
{
	switch(nKeyCode)
	{
	case VK_TAB:
		{
			Tab(ControlManager::IsKeyPressed(VK_SHIFT) == false);
		}
		break;
	case VK_UP:
	case VK_LEFT:
		{
			Tab(false);
		}
		break;
	case VK_DOWN:
	case VK_RIGHT:
		{
			Tab(true);
		}
		break;
	case VK_RETURN:
		{
			// Default command

			ScreenButton* pButton =
				dynamic_cast<ScreenButton*>(GetChildren().FindByID(m_nDefaultCmdID));

			if (pButton != NULL)
				pButton->OnAction();
		}
		break;
	case VK_ESCAPE:
		{
			// Cancel command

			ScreenButton* pButton =
				dynamic_cast<ScreenButton*>(GetChildren().FindByID(m_nCancelCmdID));

			if (pButton != NULL)
				pButton->OnAction();
		}
		break;
	default:
		{
			// Handle mnemonics

			if (ControlManager::IsKeyPressed(VK_MENU) && m_mapMnemonics.find(tolower(nKeyCode)) != m_mapMnemonics.end())
			{
				Screen* pMnemonicOwner = m_mapMnemonics[tolower(nKeyCode)];

				if (pMnemonicOwner == NULL)
				{
					return;
				}

				ScreenButton* pButton =
					dynamic_cast<ScreenButton*>(pMnemonicOwner);

				if (pButton != NULL)
				{
					// If screen is a button, fire its command event

					pButton->OnAction();
				}
				else
				{
					// If not a button, set focus

					m_rEngine.GetScreens().SetFocusScreen(pMnemonicOwner);
				}
			}
		}
		break;
	}
}

void ScreenOverlapped::OnMouseMove(POINT pt)
{
	if (m_nDragState != DRAG_STATE_NONE)
	{
		// Fire OnBeginDrag

		if (DRAG_STATE_WAITING == m_nDragState)
		{
			m_nDragState = DRAG_STATE_DRAGGING;

			OnBeginDrag();
		}

		// Move screen with mouse

		SetPosition(m_ptPos.x + (pt.x - m_ptDragOffset.x),
			        m_ptPos.y + (pt.y - m_ptDragOffset.y));
	}
}

void ScreenOverlapped::OnMouseLDown(POINT pt)
{
	// Accept mouse only in client area
	// If this screen is modal, this event will be fired even if mouse is outside

	if (IsFlagSet(MODAL) &&
	   (pt.x < 0 || pt.y < 0 || pt.x > GetSize().cx || pt.y > GetSize().cy))
		return;

	if (IsFlagSet(NOACTIVATE) == false)
		Activate();

	// Set focus

	if (m_rEngine.GetScreens().GetFocusScreen() != this)
		m_rEngine.GetScreens().SetFocusScreen(this);

	// Start dragging

	if (IsFlagSet(DRAGGABLE) == true)
	{
		m_ptDragOffset.x = pt.x;
		m_ptDragOffset.y = pt.y;

		m_nDragState = DRAG_STATE_WAITING;

		// Capture

		m_rEngine.GetScreens().SetCaptureScreen(this);
	}
}

void ScreenOverlapped::OnMouseLUp(POINT pt)
{
	if (m_nDragState != DRAG_STATE_NONE)
	{
		m_nDragState = DRAG_STATE_NONE;

		m_rEngine.GetScreens().SetCaptureScreen(NULL);
	}
}

void ScreenOverlapped::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read overlapped flags

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
	{
		m_dwFlags |= pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD));
	}

	// Read font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, &rRoot, m_pStyle,
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE));

	// Read tab order (optional)

	Screen* pScreen = NULL;

	pElem = rRoot.FindChildConst(SZ_TABORDER);

	if (pElem != NULL)
	{
		if (pElem->GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			for(InfoElemConstIterator pos = pElem->GetBeginChildPosConst();
				pos != pElem->GetEndChildPosConst();
				pos++)
			{
				if ((*pos)->GetVarType() == Variable::TYPE_STRING)
				{
					// Read tab item as screen name

					pScreen = m_lstChildren.FindByName((*pos)->GetStringValue(), true);
				}
				else if ((*pos)->GetVarType() == Variable::TYPE_INT)
				{
					// Read tab item as screen id

					pScreen = m_lstChildren.FindByID((*pos)->GetIntValue(), true);
				}

				if (pScreen != NULL)
					m_arTabOrder.push_back(pScreen);
			}
		}
		else if (pElem->GetElemType() == InfoElem::TYPE_VALUE)
		{
			if (pElem->GetVarType() == Variable::TYPE_STRING)
			{
				// Read tab item as screen name

				pScreen = m_lstChildren.FindByName(pElem->GetStringValue(), true);
			}
			else if (pElem->GetVarType() == Variable::TYPE_INT)
			{
				// Read tab item as screen id

				pScreen = m_lstChildren.FindByID(pElem->GetIntValue(), true);
			}

			if (pScreen != NULL)
				m_arTabOrder.push_back(pScreen);
		}
	}

	// Read default command (optional)

	pElem = rRoot.FindChildConst(SZ_DEFAULTCOMMAND,
		InfoElem::TYPE_VALUE,
		Variable::TYPE_INT);

	if (pElem != NULL)
		m_nDefaultCmdID = pElem->GetIntValue();

	// Read cancel command (optional)

	pElem = rRoot.FindChildConst(SZ_CANCELCOMMAND,
		InfoElem::TYPE_VALUE,
		Variable::TYPE_INT);

	if (pElem != NULL)
		m_nCancelCmdID = pElem->GetIntValue();

	// Read title

	pElem = rRoot.FindChildConst(SZ_TITLE,
		InfoElem::TYPE_VALUE,
		Variable::TYPE_STRING);

	if (pElem != NULL)
		m_strTitle = pElem->GetStringValue();

	// Create title

	const Variable* pVar = NULL;

	if (IsFlagSet(TITLE) == true)
	{
		m_pTitle = dynamic_cast<Screen*>(ScreenLabel::CreateInstance(m_rEngine,
			ScreenLabel::SZ_CLASS, this));

		m_pTitle->SetID(ID_TITLE);
		m_pTitle->SetFlag(DISABLED);

		if (LoadVariable(&pVar, SZ_TITLESTYLE, Variable::TYPE_STRING,
			Variable::TYPE_STRING, NULL, m_pStyle) == true)
		{		
			m_pTitle->SetStyle(pVar->GetStringValue());
		}

		dynamic_cast<ScreenLabel*>(m_pTitle)->SetText(m_strTitle);

		m_lstChildren.Add(m_pTitle, m_lstChildren.GetBeginPos());
	}

	// Re-create or modify border

	if (IsFlagSet(BORDER) == true)
	{
		m_pBorder = dynamic_cast<Screen*>(ScreenFrame::CreateInstance(m_rEngine,
			ScreenFrame::SZ_CLASS, this));

		m_pBorder->SetID(ID_MAINFRAME);
		m_pBorder->SetFlag(ScreenFrame::AUTOSIZE | DISABLED);

		if (LoadVariable(&pVar, SZ_BORDERSTYLE, Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pBorder->SetStyle(pVar->GetStringValue());
		}

		m_lstChildren.Add(m_pBorder, m_lstChildren.GetBeginPos());
	}

	// Re-create or modify caption button(s)

	if (IsFlagSet(CAPTION_MIN) == true ||
	   IsFlagSet(CAPTION_MAX) == true ||
	   IsFlagSet(CAPTION_CLOSE) == true)
	{
		// Create caption buttons image if has style specified

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_BACKGROUND], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaptionBack = dynamic_cast<Screen*>(ScreenImage::CreateInstance(m_rEngine,
				ScreenImage::SZ_CLASS, this));

			m_pCaptionBack->SetStyle(pVar->GetStringValue());

			m_pCaptionBack->SetFlag(DISABLED);

			m_lstChildren.Add(m_pCaptionBack);
		}
	}

	if (IsFlagSet(CAPTION_CLOSE) == true)
	{
		m_pCaption[2] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
			ScreenButton::SZ_CLASS, this));

		m_pCaption[2]->SetID(ID_CLOSE);

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_CLOSE], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[2]->SetStyle(pVar->GetStringValue());
		}

		m_lstChildren.Add(m_pCaption[2]);
	}

	if (IsFlagSet(CAPTION_MAX) == true)
	{
		m_pCaption[1] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
				ScreenButton::SZ_CLASS, this));

		m_pCaption[1]->SetID(ID_MAX_RESTORE);

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_RESTORE], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[1]->SetStyle(pVar->GetStringValue());
		}

		m_lstChildren.Add(m_pCaption[1]);
	}

	if (IsFlagSet(CAPTION_MIN) == true)
	{
		m_pCaption[0] = dynamic_cast<Screen*>(ScreenButton::CreateInstance(m_rEngine,
			ScreenButton::SZ_CLASS, this));

		m_pCaption[0]->SetID(ID_MIN);

		if (LoadVariable(&pVar, SZ_CAPSTYLE[CAP_MIN], Variable::TYPE_STRING,
			Variable::TYPE_UNDEFINED, NULL, m_pStyle) == true)
		{
			m_pCaption[0]->SetStyle(pVar->GetStringValue());
		}

		m_lstChildren.Add(m_pCaption[0]);
	}
}