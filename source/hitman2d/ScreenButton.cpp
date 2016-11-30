/*------------------------------------------------------------------*\
|
| ScreenButton.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Button Control implementation
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
#include "ScreenOverlapped.h"	// using ScreenOverlapped
#include "ScreenLabel.h"		// using ScreenLabel
#include "ScreenButton.h"		// defining ScreenButton

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenButton::SZ_CLASS[] =					L"button";
const WCHAR ScreenButton::SZ_COMMAND[] =				L"command";
const WCHAR ScreenButton::SZ_SELECTED[] =				L"selected";
const WCHAR ScreenButton::SZ_PUSHEDOFFSETX[] =			L"pushed.offset.x";
const WCHAR ScreenButton::SZ_PUSHEDOFFSETY[] =			L"pushed.offset.y";
const WCHAR ScreenButton::SZ_TEXTCOLOR[] =				L"text.color";
const WCHAR ScreenButton::SZ_TEXTCOLORPUSHED[] =		L"text.pushed.color";
const WCHAR ScreenButton::SZ_TEXTCOLORHOVER[] =			L"text.hover.color";
const WCHAR ScreenButton::SZ_TEXTCOLORTOGGLED[] =		L"text.toggled.color";
const WCHAR ScreenButton::SZ_TEXTCOLORHOVERTOGGLED[] =	L"text.hovertoggled.color";
const WCHAR ScreenButton::SZ_TEXTONLY[] =				L"text.only";
const WCHAR ScreenButton::SZ_TEXTLEFT[] =				L"text.align.left";

const LPCWSTR ScreenButton::SZ_FLAGS[] =			{
														L"notifystate",
														L"toggle",
														L"radio"
													};

const DWORD ScreenButton::DW_FLAGS[] =				{
														ScreenButton::NOTIFYSTATE,
														ScreenButton::TOGGLE,
														ScreenButton::RADIO
													};

const LPCWSTR ScreenButton::SZ_STATETEX[] =			{
														L"normal.material",
														L"hover.material",
														L"pushed.material",
														L"disabled.material",
														L"normaltoggled.material",
														L"hovertoggled.material",
														L"pushedtoggled.material",
														L"disabledtoggled.material"
													};

const LPCWSTR ScreenButton::SZ_STATEBLEND[] =		{
														L"normal.color",
														L"hover.color",
														L"pushed.color",
														L"disabled.color",
														L"normaltoggled.color",
														L"hovertoggled.color",
														L"pushedtoggled.color",
														L"disabledtoggled.color"
													};


/*----------------------------------------------------------*\
| ScreenButton implementation
\*----------------------------------------------------------*/

ScreenButton::ScreenButton(Engine& rEngine,
						   LPCWSTR pszClass,
						   Screen* pParent):

						   Screen(rEngine, pszClass, pParent),
						   m_nState(ScreenButton::STATE_NORMAL),
						   m_bPushed(false),
						   m_bHover(false),
						   m_bToggle(false),
						   m_bNoStateTextures(true),
						   m_bTextOnly(false),
						   m_bTextAlignLeft(false),
						   m_pFont(NULL)
{
	for(int n = 0; n < STATE_COUNT; n++)
		m_clrStateBlends[n] = 0xFFFFFFFF;

	ZeroMemory(&m_ptPushedTextOffset, sizeof(POINT));
}

ScreenButton::~ScreenButton(void)
{
}

Object* ScreenButton::CreateInstance(Engine& rEngine,
									 LPCWSTR pszClass,
									 Object* pParent)
{
	return new ScreenButton(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenButton::SetFlags(DWORD dwFlags)
{
	DWORD dwOldFlags = m_dwFlags;

	Screen::SetFlags(dwFlags);

	States nOldState = m_nState;

	if (IsFlagSet(DISABLED) == true)
	{
		// If now disabled...

		m_nState = (true == m_bToggle ? STATE_DISABLEDTOGGLED :
			STATE_DISABLED);
	}
	else if (dwOldFlags & DISABLED)
	{
		// If was disabled and now is not...

		if (true == m_bPushed)
		{
			m_nState = m_bToggle ? STATE_PUSHEDTOGGLED :
				STATE_PUSHED;
		}
		else
		{
			if (true == m_bHover)
				m_nState = (true == m_bToggle ? STATE_HOVERTOGGLED :
					STATE_HOVER);
			else
				m_nState = (true == m_bToggle ? STATE_NORMALTOGGLED :
					STATE_NORMAL);
		}
	}

	if (m_nState != nOldState)
		UpdateState(true);
}

void ScreenButton::SetToggle(bool bToggle)
{
	m_bToggle = bToggle;

	if (IsFlagSet(DISABLED) == true)
	{
		m_nState = bToggle ? STATE_DISABLEDTOGGLED :
			STATE_DISABLED;
	}
	else
	{
		if (true == m_bPushed)
		{
			m_nState = bToggle ? STATE_PUSHEDTOGGLED :
				STATE_PUSHED;
		}
		else
		{
			if (true == m_bHover)
				m_nState = (true == bToggle ? STATE_HOVERTOGGLED :
					STATE_HOVER);
			else
				m_nState = (true == bToggle ? STATE_NORMALTOGGLED :
					STATE_NORMAL);
		}
	}

	UpdateState(true);
}

void ScreenButton::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read button flags (optional)

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
	{
		m_dwFlags |= pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD));
	}	

	// Read command (optional)

	pElem = rRoot.FindChildConst(SZ_COMMAND,
		InfoElem::TYPE_VALUE, Variable::TYPE_STRING);

	if (pElem != NULL)
	{
		LPCWSTR pszCommand = pElem->GetStringValue();

		int nLen = int(wcslen(pszCommand));
		int nPosLabel = nLen - 1;

		for(; nPosLabel != 0 && pszCommand[nPosLabel] != L':'; nPosLabel--);

		WCHAR szScriptPath[MAX_PATH] = {0};

		if (nPosLabel > 0)
		{
			// Get script path

			wcsncpy_s(szScriptPath, MAX_PATH, pszCommand, nPosLabel);

			// Get script label

			int nLabelLen = nLen - nPosLabel;

			try
			{
				m_strCommandLabel.Allocate(nLabelLen);
			}

			catch(std::bad_alloc e)
			{
				throw Error(Error::MEM_ALLOC,
					__FUNCTIONW__, sizeof(WCHAR) * (nLabelLen + 1));
			}

			m_strCommandLabel.CopyToBuffer(nLabelLen,
				pszCommand + nPosLabel + 1,
				nLabelLen);
		}
		else
		{
			// Get script path

			wcscpy_s(szScriptPath, MAX_PATH, pszCommand);
		}

		// Make sure script path is absolute

		if (PathIsRelative(szScriptPath) == TRUE)
		{
			WCHAR szCurDir[MAX_PATH] = {0};
			GetCurrentDirectory(MAX_PATH, szCurDir);

			LPCWSTR pszDirs[] = { szCurDir, NULL };
			PathResolve(szScriptPath, pszDirs, PRF_FIRSTDIRDEF);
		}

		m_strCommandScript = szScriptPath;
		
		// Pre-load script, if stream cache is enabled

		if (m_rEngine.GetOption(Engine::OPTION_ENABLE_STREAM_CACHE) == TRUE)
		{
			Stream stream(&m_rEngine.GetErrors(), &m_rEngine.GetStreamCache());

			try
			{
				stream.Open(m_strCommandScript, GENERIC_READ,
					OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
			}

			catch(Error& rError)
			{
				UNREFERENCED_PARAMETER(rError);

				Game::PrintLastError(m_rEngine);
			}
		}
	}

	// Read selected

	pElem = rRoot.FindChildConst(SZ_SELECTED,
		InfoElem::TYPE_VALUE);

	if (pElem != NULL && (pElem->GetVarType() == Variable::TYPE_INT ||
	   pElem->GetVarType() == Variable::TYPE_BOOL))
	{
		m_bToggle = pElem->GetBoolValue();
	}

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Read text

	const Variable* pVar = NULL;

	if (LoadVariable(&pVar, ScreenLabel::SZ_TEXT,
		Variable::TYPE_STRING, Variable::TYPE_STRING,
		&rRoot, m_pStyle, pSharedStyle) == true)
			m_strText = pVar->GetStringValue();

	// Register mnemonic if one is present in text

	if (m_strText.IsEmpty() == false)
	{
		ScreenOverlapped::SetMnemonic(this, m_strText);
	}

	// Read text color

	LoadColor(m_clrText, SZ_TEXTCOLOR,
		&rRoot, m_pStyle, pSharedStyle);

	// Read pushed text color

	if (LoadColor(m_clrTextPushed, SZ_TEXTCOLORPUSHED,
		&rRoot, m_pStyle, pSharedStyle) == false)
			m_clrTextPushed = m_clrText;

	// Read hover text color

	if (LoadColor(m_clrTextHover, SZ_TEXTCOLORHOVER,
		&rRoot, m_pStyle, pSharedStyle) == false)
			m_clrTextHover = m_clrText;

	// Read toggled text color

	if (LoadColor(m_clrTextToggled, SZ_TEXTCOLORTOGGLED,
		&rRoot, m_pStyle, pSharedStyle) == false)
			m_clrTextToggled = m_clrTextPushed;

	// Read toggled hover text color

	if (LoadColor(m_clrTextHoverToggled, SZ_TEXTCOLORHOVERTOGGLED,
		&rRoot, m_pStyle, pSharedStyle) == false)
			m_clrTextHoverToggled = m_clrTextPushed;

	// Read text only (not a flag because it's a part of visual style)

	if (LoadVariable(&pVar, SZ_TEXTONLY,
		Variable::TYPE_BOOL, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
			m_bTextOnly = pVar->GetBoolValue();

	// Read text align left

	if (LoadVariable(&pVar, SZ_TEXTLEFT,
		Variable::TYPE_BOOL, Variable::TYPE_INT, &rRoot, m_pStyle) == true)
			m_bTextAlignLeft = pVar->GetBoolValue();

	// Read font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, &rRoot, m_pStyle, pSharedStyle);

	// Read pushed offset

	if (LoadVariable(&pVar, SZ_PUSHEDOFFSETX,
		Variable::TYPE_INT, Variable::TYPE_BOOL,
		&rRoot, m_pStyle, pSharedStyle) == true)
			m_ptPushedTextOffset.x = pVar->GetIntValue();

	if (LoadVariable(&pVar, SZ_PUSHEDOFFSETY,
		Variable::TYPE_INT, Variable::TYPE_BOOL, &rRoot,
		m_pStyle, pSharedStyle) == true)
			m_ptPushedTextOffset.y = pVar->GetIntValue();

	// Read state textures and blends

	try
	{
		for(int nState = STATE_NORMAL;
			nState < STATE_COUNT;
			nState++)
		{
			if (LoadMaterialInstance(m_states[nState],
				SZ_STATETEX[nState],
				&rRoot, m_pStyle) == true)
				m_bNoStateTextures = false;

			LoadColor(m_clrStateBlends[nState],
				SZ_STATEBLEND[nState],
				&rRoot, m_pStyle);
		}

		// Validate toggle

		if (true == m_bToggle && IsFlagSet(TOGGLE) == false)
			m_bToggle = false;

		// Set initial state

		if (IsFlagSet(DISABLED) == true)
		{
			m_nState = (true == m_bToggle ? STATE_DISABLEDTOGGLED :
				STATE_DISABLED);
		}
		else if (true == m_bPushed)
		{
			m_nState = (true == m_bToggle ? STATE_PUSHEDTOGGLED :
				STATE_PUSHED);
		}
		else if (true == m_bHover)
		{
			m_nState = (true == m_bToggle ? STATE_HOVERTOGGLED :
				STATE_HOVER);
		}
		else
		{
			m_nState = (true == m_bToggle ? STATE_NORMALTOGGLED :
				 STATE_NORMAL);
		}

		UpdateState(false);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void ScreenButton::OnThemeStyleChange(void)
{
	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Reload text	

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, ScreenLabel::SZ_TEXT,
		Variable::TYPE_STRING, Variable::TYPE_STRING,
		NULL, m_pStyle, pSharedStyle) == true)
			m_strText = pVar->GetStringValue();

	// Reload text color

	LoadColor(m_clrText, SZ_TEXTCOLOR,
		NULL, m_pStyle, pSharedStyle);

	// Reload pushed text color

	if (LoadColor(m_clrTextPushed, SZ_TEXTCOLORPUSHED,
		NULL, m_pStyle, pSharedStyle) == false)
			m_clrTextPushed = m_clrText;

	// Reload text only

	if (LoadVariable(&pVar, SZ_TEXTONLY,
		Variable::TYPE_BOOL, Variable::TYPE_INT, NULL, m_pStyle) == true)
			m_bTextOnly = pVar->GetBoolValue();

	// Reload text align left

	if (LoadVariable(&pVar, SZ_TEXTLEFT,
		Variable::TYPE_BOOL, Variable::TYPE_INT, NULL, m_pStyle) == true)
			m_bTextAlignLeft = pVar->GetBoolValue();

	// Read font

	LoadFont(&m_pFont, SZ_SCREEN_FONT, NULL, m_pStyle, pSharedStyle);

	// Read pushed offset

	if (LoadVariable(&pVar, SZ_PUSHEDOFFSETX,
		Variable::TYPE_INT, Variable::TYPE_BOOL,
		NULL, m_pStyle, pSharedStyle) == true)
			m_ptPushedTextOffset.x = pVar->GetIntValue();

	if (LoadVariable(&pVar, SZ_PUSHEDOFFSETY,
		Variable::TYPE_INT, Variable::TYPE_BOOL,
		NULL, m_pStyle, pSharedStyle) == true)
			m_ptPushedTextOffset.y = pVar->GetIntValue();

	// Read state textures and blends

	try
	{
		for(int nState = STATE_NORMAL;
			nState < STATE_COUNT;
			nState++)
		{
			if (LoadMaterialInstance(m_states[nState],
				SZ_STATETEX[nState],
				NULL, m_pStyle) == true)
				m_bNoStateTextures = false;

			LoadColor(m_clrStateBlends[nState],
				SZ_STATEBLEND[nState],
				NULL, m_pStyle);
		}

		// Validate toggle

		if (true == m_bToggle && IsFlagSet(TOGGLE) == false)
			m_bToggle = false;

		// Set initial state

		if (IsFlagSet(DISABLED) == true)
		{
			m_nState = (true == m_bToggle ? STATE_DISABLEDTOGGLED :
				STATE_DISABLED);
		}
		else if (true == m_bPushed)
		{
			m_nState = (true == m_bToggle ? STATE_PUSHEDTOGGLED :
				STATE_PUSHED);
		}
		else if (true == m_bHover)
		{
			m_nState = (true == m_bToggle ? STATE_HOVERTOGGLED :
				STATE_HOVER);
		}
		else
		{
			m_nState = (true == m_bToggle ? STATE_NORMALTOGGLED :
				 STATE_NORMAL);
		}

		UpdateState(false);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::THEME_INVALIDSTYLE,
			__FUNCTIONW__, m_strStyle);
	}
}

DWORD ScreenButton::GetMemoryFootprint(void) const
{
	return sizeof(ScreenButton) - sizeof(Screen) * 2 +
		   Screen::GetMemoryFootprint() +
		   m_strCommandScript.GetLengthBytes() +
		   m_strCommandLabel.GetLengthBytes();
}

void ScreenButton::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Calculate background color

	float fBlend = GetFrontBufferBlend().GetAFloat();

	Color clrBack(m_clrBackColor);
	
	if (IsFlagSet(DISABLED) == true)
		clrBack.SetAlpha(clrBack.GetAFloat() * fBlend * 0.5f);
	else
		clrBack.SetAlpha(clrBack.GetAFloat() * fBlend);
	
	// Render background

	if (!m_bTextOnly && !m_Background.IsEmpty())
		rGraphics.RenderQuad(m_Background, m_vecCachedPos, clrBack);

	// Render text

	if (m_pFont != NULL && m_strText.IsEmpty() == false)
	{
		Rect rc = m_rcCachedRect;

		if (STATE_PUSHED == m_nState ||
		   STATE_PUSHEDTOGGLED == m_nState)
		{
			rc.left += m_ptPushedTextOffset.x;
			rc.top += m_ptPushedTextOffset.y;
		}

		if (true == m_bTextOnly)
		{
			m_pFont->RenderText(rc, m_strText, -1, m_clrBackColor,
				(true == m_bTextAlignLeft ? 0 : Font::ALIGN_CENTER) |
				Font::ALIGN_VCENTER | Font::USE_MNEMONICS);
		}
		else
		{
			Color clrText;
			
			if (STATE_PUSHED == m_nState ||
			   STATE_PUSHEDTOGGLED == m_nState ||
			   STATE_NORMALTOGGLED == m_nState ||
			   STATE_HOVERTOGGLED == m_nState)
				clrText = m_clrTextPushed;
			else
				clrText = m_clrText;

			clrText.SetAlpha(clrText.GetAFloat() *
				GetFrontBufferBlend().GetAFloat());

			m_pFont->RenderText(rc, m_strText, -1, clrText,
				(true == m_bTextAlignLeft ? 0 : Font::ALIGN_CENTER) |
				Font::ALIGN_VCENTER | Font::USE_MNEMONICS);
		}
	}
}

void ScreenButton::OnMouseLDown(POINT pt)
{
	// Make sure the button has focus and capture

	m_rEngine.GetScreens().SetFocusScreen(this);
	m_rEngine.GetScreens().SetCaptureScreen(this);

	// Update state

	m_bPushed = true;

	if (IsFlagSet(TOGGLE) == true)
	{
		if (true == m_bToggle)
			m_nState = STATE_PUSHEDTOGGLED;
		else
			m_nState = STATE_PUSHED;
	}
	else
	{
		m_nState = STATE_PUSHED;
	}

	UpdateState(true);

	// Notify

	if (m_pParent != NULL)
	{
		// Make sure parent is active

		Screen* pParent = m_pParent;

		Screen* pActive = m_rEngine.GetScreens().GetActiveScreen();

		while(pParent->GetParent() != NULL)
		{
			pParent = pParent->GetParent();

			if (pParent == pActive)
				break;
		}

		if (pParent != pActive)
			m_rEngine.GetScreens().SetActiveScreen(pParent);

		m_pParent->OnNotify(NOTIFY_SELECT,
			this, NOTIFY_SELECT_PUSHED);
	}
}

void ScreenButton::OnMouseLUp(POINT pt)
{
	// Reset capture

	if (m_rEngine.GetScreens().GetCaptureScreen() == this)
		m_rEngine.GetScreens().SetCaptureScreen(NULL);

	// Update state

	m_bPushed = false;

	if (true == m_bHover ||
	  (GetClientRect().PtInRect(pt) == true))
	{
		// Mouse was let up while it was over the button

		OnAction();
	}
	else
	{
		// Mouse was let up when it was out of the button

		if (IsFlagSet(TOGGLE) == true)
			m_nState = m_bToggle ? STATE_NORMALTOGGLED : STATE_NORMAL;
		else
			m_nState = STATE_NORMAL;

		UpdateState(true);

		// Notify

		if (m_pParent != NULL)
		{
			m_pParent->OnNotify(NOTIFY_SELECT,
				this, m_bHover ? NOTIFY_SELECT_HOVER : NOTIFY_SELECT_NONE);
		}
	}
}

void ScreenButton::OnMouseEnter(void)
{
	// If the mouse is down, don't become hover if not pushed

	m_bHover = true;

	if (true == m_bPushed)
	{
		m_nState = (true == m_bToggle) ?
			STATE_PUSHEDTOGGLED : STATE_PUSHED;
	}
	else
	{
		m_nState = (true == m_bToggle) ?
			STATE_HOVERTOGGLED : STATE_HOVER;
	}
	
	UpdateState(true);

	//if (dynamic_cast<ScreenButton*>(m_rEngine.GetScreens().GetFocusScreen()) != NULL)
	//	m_rEngine.GetScreens().SetFocusScreen(this);

	// Notify

	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(NOTIFY_SELECT,
			this, NOTIFY_SELECT_HOVER);
	}
}

void ScreenButton::OnMouseLeave(void)
{
	m_bHover = false;

	m_nState = (true == m_bToggle) ?
		STATE_NORMALTOGGLED : STATE_NORMAL;

	UpdateState(true);

	// Notify

	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(NOTIFY_SELECT,
			this, NOTIFY_SELECT_NONE);
	}
}

void ScreenButton::OnMouseWheel(int nZDelta)
{
	// Pass to parent

	if (m_pParent != NULL)
		m_pParent->OnMouseWheel(nZDelta);
}

void ScreenButton::OnKeyDown(int nKeyCode)
{
	if (VK_RETURN == nKeyCode)
	{
		OnAction();
	}
	else if (VK_SPACE == nKeyCode)
	{
		// Simulate mouse down

		m_bPushed = true;

		if (IsFlagSet(TOGGLE) == true)
		{
			if (true == m_bToggle)
				m_nState = STATE_PUSHEDTOGGLED;
			else
				m_nState = STATE_PUSHED;
		}
		else
		{
			m_nState = STATE_PUSHED;
		}

		UpdateState(true);

		// Notify

		if (m_pParent != NULL)
			m_pParent->OnNotify(NOTIFY_SELECT, this, NOTIFY_SELECT_PUSHED);
	}
	else
	{
		// Transfer to parent, for mnemonic processing and cancel command

		if (m_pParent != NULL)
			m_pParent->OnKeyDown(nKeyCode);
	}
}

void ScreenButton::OnKeyUp(int nKeyCode)
{
	if (VK_SPACE == nKeyCode)
	{
		// Simulate mouse up

		m_bPushed = false;

		OnAction();

		m_nState =
			true == m_bToggle ? STATE_HOVERTOGGLED : STATE_HOVER;

		if (m_pParent != NULL)
			m_pParent->OnNotify(NOTIFY_SELECT, this, NOTIFY_SELECT_NONE);

		UpdateState(true);
	}
}

void ScreenButton::OnFocus(Screen* pOldFocus)
{
	States nNewState = STATE_HOVER;
	
	if (IsFlagSet(DISABLED))
	{
		nNewState = (true == m_bToggle) ?
			STATE_DISABLEDTOGGLED : STATE_DISABLED;
	}
	else
	{
		nNewState = (true == m_bToggle) ?
			STATE_HOVERTOGGLED : STATE_HOVER;
	}

	if (m_nState != nNewState)
	{
		m_nState = nNewState;

		UpdateState(true);
	}

	// Notify

	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(NOTIFY_SELECT, this, NOTIFY_SELECT_HOVER);
	}
}

void ScreenButton::OnDefocus(Screen* pNewFocus)
{
	m_bHover = false;
	m_bPushed = false;

	States nNewState = STATE_NORMAL;

	if (IsFlagSet(DISABLED) == true)
	{
		nNewState = (true == m_bToggle) ?
			STATE_DISABLEDTOGGLED : STATE_DISABLED;
	}
	else
	{
		nNewState = (true == m_bToggle) ?
			STATE_NORMALTOGGLED : STATE_NORMAL;
	}

	if (m_nState != nNewState)
	{
		m_nState = nNewState;

		UpdateState(true);
	}

	// Notify

	if (m_pParent != NULL)
		m_pParent->OnNotify(NOTIFY_SELECT, this, NOTIFY_SELECT_NONE);
}

void ScreenButton::UpdateState(bool bRender)
{
	// Change the background to current state texture

	if (false == m_bNoStateTextures)
		m_Background = m_states[m_nState];

	// Change blend to current state blend

	m_clrBackColor = m_clrStateBlends[m_nState];

	// Invalidate

	Invalidate();
}

void ScreenButton::OnAction(void)
{
	if (IsFlagSet(TOGGLE) == true)
	{
		if (IsFlagSet(RADIO) == true &&
		   false == m_bToggle &&
		   m_pParent != NULL)
		{
			// If radio, untoggle other radios in group

			for(ScreenListIterator pos =
				m_pParent->GetChildren().GetBeginPos();
				pos != m_pParent->GetChildren().GetEndPos();
				pos++)
			{
				// Don't untoggle self

				if (this == (*pos)) continue;

				if ((*pos)->GetID() == GetID() &&
				   (*pos)->GetClass() == GetClass() &&
				   (*pos)->IsFlagSet(RADIO))
				{
					ScreenButton* pButton =
						dynamic_cast<ScreenButton*>(*pos);

					if (true == pButton->m_bToggle)
					{
						pButton->m_bToggle = false;
						pButton->m_nState = STATE_NORMAL;

						pButton->UpdateState(true);
					}
				}
			}				
		}

		m_bToggle = !m_bToggle;

		m_nState = m_bToggle ? STATE_HOVERTOGGLED :
			STATE_HOVER;

		UpdateState(true);
	}
	else
	{
		m_nState = STATE_HOVER;

		UpdateState(true);			
	}

	if (m_strCommandScript.IsEmpty() == true)
	{
		// Call parent's OnCommand if no commands specified

		if (m_pParent != NULL)
			m_pParent->OnCommand(m_nID, this);

		// Notify listeners

		for(ScreenListIterator pos = m_lstListeners.begin();
			pos != m_lstListeners.end();
			pos++)
		{
			if (*pos != NULL)
				(*pos)->OnCommand(m_nID, this);
		}
	}
	else
	{
		// Execute command script

		try
		{
			m_rEngine.GetCommands().ExecuteScriptFile(m_strCommandScript,
													  m_strCommandLabel);
		}

		catch(Error& rError)
		{
			UNREFERENCED_PARAMETER(rError);

			// Don't let bad commands close down application

			Game::PrintLastError(m_rEngine);
		}
	}
}