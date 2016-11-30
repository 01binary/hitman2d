/*------------------------------------------------------------------*\
|
| ScreenConsole.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScreenConsole implementation
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
#include "ScreenConsole.h"	// defining ScreenConsole

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenConsole::SZ_PROMPTTEXTURE[] =	L"prompt.material";
const WCHAR ScreenConsole::SZ_CARETTEXTURE[] =	L"caret.material";
const WCHAR ScreenConsole::SZ_BORDERTEXTURE[] =	L"border.material";
const WCHAR ScreenConsole::SZ_PROMPTFLASH[] =	L"prompt.flash";
const WCHAR ScreenConsole::SZ_HISTORYLINES[] =	L"historylines";
const WCHAR ScreenConsole::SZ_CLASS[] =			L"overlapped::console";


/*----------------------------------------------------------*\
| ScreenConsole implementation
\*----------------------------------------------------------*/

ScreenConsole::ScreenConsole(Engine& rEngine,
						     LPCWSTR pszClass,
						     Screen* pParent):

						     ScreenOverlapped(rEngine, pszClass, pParent),
						     m_bFullOpen(false),
						     m_bReopen(false),
						     m_bReclose(false),
						     m_bJustToggled(false),
						     m_bEnableFlash(true),
						     m_fLastFlash(0.0f),
						     m_nPromptAlpha(0),
						     m_bFlashUp(true),
						     m_bReplaceMode(false),
						     m_bAddToLine(false),
						     m_nColumnWidth(0),
						     m_nLineHeight(0),
						     m_nPromptWidth(0),
						     m_nLines(0),
						     m_nColumns(0),
						     m_nInputColumns(0),
						     m_nFirstVisible(0),
						     m_nOpenPos(0),
						     m_nInputScroll(0),
						     m_nCaretPos(0),
						     m_nHistoryPos(0),
						     m_nHistoryLines(0),
						     m_nHistoryUsed(0),
						     m_nMoveAction(MOVE_NONE),
						     m_pMoveTimer(NULL),											   
						     m_ppszLines(NULL),
						     m_pszInputLine(NULL),
						     m_ppszHistory(NULL),
						     m_prcLines(NULL),
						     m_pclrLines(NULL)
{
	m_Prompt.Empty();
	m_Caret.Empty();
	m_Border.Empty();

	m_posNextCommand = rEngine.GetCommands().GetEndPos();
}

ScreenConsole::~ScreenConsole(void)
{
	Empty();
}

Object* ScreenConsole::CreateInstance(Engine& rEngine,
									  LPCWSTR pszClass,
									  Object* pParent)
{
	return new ScreenConsole(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenConsole::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	// Flash prompt

	if (m_rEngine.GetRunTimeDelta(m_fLastFlash) > 0.01f)
	{
		m_fLastFlash = m_rEngine.GetRunTime();

		if (true == m_bFlashUp)
		{
			m_nPromptAlpha += PROMPT_ALPHA_STEP;

			if (m_nPromptAlpha > 255)
			{
				m_nPromptAlpha = 255;
				m_bFlashUp = false;
			}
		}
		else
		{
			m_nPromptAlpha -= PROMPT_ALPHA_STEP;

			if (m_nPromptAlpha < 0)
			{
				m_nPromptAlpha = 0;
				m_bFlashUp = true;
			}
		}
	}

	// Render background

	OnRenderBackground(rGraphics);

	// Render prompt

	rGraphics.RenderQuad(m_Prompt, m_vecPrompt,
		D3DCOLOR_ARGB(m_nPromptAlpha, 255, 255, 255));

	// Render caret

	if ((MOVE_NONE == m_nMoveAction && true == m_bEnableFlash) ?
		true : (m_nPromptAlpha > 127))
		rGraphics.RenderQuad(m_Caret, m_vecCaret);

	// Render border

	if (false == m_bFullOpen)
		rGraphics.RenderQuad(m_Border, m_vecBorder);

	// Render text output lines

	if (false == m_CachedText.IsDirty())
	{
		// If cache still integral, render from cache

		m_CachedText.Render();
	}
	else
	{
		// If cache is dirty and console not moving, cache current text
		// Otherwise, render text directly

		for(int n = m_nLines - 1; n >= m_nFirstVisible;	n--)
		{
			m_pFont->RenderText(m_prcLines[n], m_ppszLines[n], -1,
				m_pclrLines[n], Font::ALIGN_LEFT,
				(MOVE_NONE == m_nMoveAction) ? &m_CachedText : NULL);
		}
		
		if (MOVE_NONE == m_nMoveAction)
		{
			m_CachedText.Cache();

			m_CachedText.Render();
		}
	}

	// Render input line

	m_pFont->RenderText(m_prcLines[m_nLines], m_pszInputLine + m_nInputScroll, -1,
		m_clrText[PRINT_MESSAGE]);
}

void ScreenConsole::Toggle(bool bFullOpen)
{
	if (m_rEngine.GetScreens().GetActiveScreen() != this)
	{
		m_bFullOpen = bFullOpen;

		m_rEngine.GetScreens().SetActiveScreen(this);
		m_rEngine.GetScreens().SetFocusScreen(this);

		ClearFlag(INVISIBLE);
		ClearFlag(DISABLED);
	}
	else
	{
		if (ControlManager::IsKeyPressed(VK_SHIFT) == true)
		{
			if (0 == m_ptPos.y)
			{
				// If fully open, close to half-size

				m_bFullOpen = false;
				m_bReopen = false;
				m_bReclose = true;

				m_nOpenPos = -m_psSize.cy / 2;

				OnDeactivate(this);
			}
			else
			{
				// If not fully open, open to full size

				m_bReopen = true;
				m_bReclose = false;

				OnActivate(this);
			}
		}
		else
		{
			Deactivate();
		}
	}

	m_bJustToggled = true;
}

void ScreenConsole::Print(LPCWSTR pszText,
						  PrintTypes nPrintType,
						  bool bLine)
{
	if (NULL == m_ppszLines)
		return;

	if (PRINT_CLEAR == nPrintType)
	{
		for(int n = 0; n < m_nLines; n++)
		{
			ZeroMemory(m_ppszLines[n], m_nColumns * sizeof(WCHAR));			
			m_pclrLines[n] = m_clrText[PRINT_MESSAGE];
		}

		m_CachedText.Empty();

		return;
	}

	m_CachedText.Empty();

	if (NULL == pszText)
	{
		if (true == m_bAddToLine)
		{
			m_bAddToLine = false;

			wcscat_s(m_ppszLines[m_nLines - 1], m_nColumns + 1, L" ");
		}
		else
		{
			Scroll();

			*m_ppszLines[m_nLines - 1] = L' ';
		}

		return;
	}

	int nInitialLen = true == m_bAddToLine ? 
		int(wcslen(m_ppszLines[m_nLines - 1])) : 0;

	int nLen = nInitialLen;

	for(LPCWSTR pszStart = pszText ;; pszText++)
	{
		switch(*pszText)
		{
		case L'\r':
		case L'\n':
			{
				// CR breaks the line

				if (true == m_bAddToLine)
				{
					m_bAddToLine = false;

					wcsncat_s(m_ppszLines[m_nLines - 1],
						m_nColumns + 1, pszStart, nLen - nInitialLen);
				}
				else
				{
					Scroll();

					wcsncpy_s(m_ppszLines[m_nLines - 1],
						m_nColumns + 1, pszStart, nLen);
				}

				m_ppszLines[m_nLines - 1][nLen] = L'\0';

				m_pclrLines[m_nLines - 1] = m_clrText[nPrintType];

				// skip LF

				if (L'\r' == pszText[0] && L'\n' == pszText[1])
					pszText++;

				pszStart = pszText + 1;

				nLen = 0;
			}
			break;
		case L'\t':
			{
				// Ignore tabs

				nLen++;
			}
			break;
		default:
			{
				nLen++;

				if (nLen == m_nColumns || L'\0' == *pszText)
				{
					// Wrap or End Text

					if (true == m_bAddToLine)
					{
						m_bAddToLine = false;

						wcsncat_s(m_ppszLines[m_nLines - 1],
							m_nColumns + 1, pszStart, nLen - nInitialLen);
					}
					else
					{
						Scroll();

						wcsncpy_s(m_ppszLines[m_nLines - 1],
							m_nColumns + 1, pszStart, nLen);
					}

					if (false == bLine)
						m_bAddToLine = true;

					m_ppszLines[m_nLines - 1][nLen] = L'\0';

					m_pclrLines[m_nLines - 1] = m_clrText[nPrintType];

					if (L'\0' == *pszText)
						return;

					pszStart = pszText + 1;

					nLen = 0;			
				}
			}
			break;
		}
	}
}

void ScreenConsole::Scroll(void)
{
	// Scroll by 1 line

	LPWSTR pszTemp = NULL;

	for(int nFrom = 1, nTo = 0;
		nTo < (m_nLines - 1);
		nFrom++, nTo++)
	{
		// Swap pointers for each line

		pszTemp = m_ppszLines[nTo];
		m_ppszLines[nTo] = m_ppszLines[nFrom];
		m_ppszLines[nFrom] = pszTemp;

		// Offset line colors

		m_pclrLines[nTo] = m_pclrLines[nFrom];
	}

	// Clear the last line that just got freed up

	if (*m_ppszLines[m_nLines - 1])
		ZeroMemory(m_ppszLines[m_nLines - 1], m_nColumns * sizeof(WCHAR));
}

void ScreenConsole::ClearInput(void)
{
	ZeroMemory(m_pszInputLine, m_nInputColumns * sizeof(WCHAR));

	m_nInputScroll = 0;
	m_nCaretPos = 0;
}

void ScreenConsole::UpdateLines(void)
{
	// Figure out visibility

	if (m_ptPos.y < 0)
		m_nFirstVisible = (-m_ptPos.y) / m_nLineHeight;
	else
		m_nFirstVisible = 0;

	if (m_nFirstVisible >= m_nLines)
		m_nFirstVisible = m_nLines - 1;

	// Update rectangles for visible lines (including the input line)

	if (m_nFirstVisible >= m_nLines)
		m_nFirstVisible = m_nLines - 1;

	SetRect(&m_prcLines[m_nFirstVisible],
		m_ptPos.x + MARGIN_WIDTH,
		m_ptPos.y + MARGIN_HEIGHT + m_nLineHeight * m_nFirstVisible,
		m_ptPos.x + m_psSize.cx - MARGIN_WIDTH * 2,
		m_ptPos.y + MARGIN_WIDTH + m_nLineHeight * (m_nFirstVisible + 1));

	for(int n = m_nFirstVisible + 1;
		n < (m_nLines + 1);
		n++)
	{
		CopyRect(&m_prcLines[n], &m_prcLines[n - 1]);
		OffsetRect(&m_prcLines[n], 0, m_nLineHeight);
	}

	// The input line rect (the last one) is offset by one char to allow
	// space for prompt char

	// Note: there are actually m_nLines+1 total lines allocated,
	// so next two lines do read valid data

	m_prcLines[m_nLines].top += 2;
	m_prcLines[m_nLines].left += m_nPromptWidth + 2;

	// Cache border position

	m_vecBorder.Set(float(m_ptPos.x), float(m_ptPos.y + m_psSize.cy));

	UpdateCaret();
}

void ScreenConsole::UpdateCaret(void)
{
	// Cache caret position

	m_vecCaret.x = float(m_ptPos.x + MARGIN_WIDTH + m_nColumnWidth *
		m_nCaretPos + m_nPromptWidth + 2);

	m_vecCaret.y = float(m_ptPos.y + MARGIN_HEIGHT + m_nLineHeight *
		m_nLines + 2);

	// Cache prompt position

	m_vecPrompt.x = float(m_ptPos.x + MARGIN_WIDTH);
	m_vecPrompt.y = m_vecCaret.y;
}

void ScreenConsole::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Read prompt material (not optional)

	if (LoadMaterialInstance(m_Prompt, SZ_PROMPTTEXTURE,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_PROMPTTEXTURE);
	}

	// Read caret material (not optional)

	if (LoadMaterialInstance(m_Caret, SZ_CARETTEXTURE,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_PROMPTTEXTURE);
	}

	// Read border material (not optional)

	if (LoadMaterialInstance(m_Border, SZ_BORDERTEXTURE,
		&rRoot, m_pStyle, pSharedStyle) == false)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(),
			SZ_PROMPTTEXTURE);
	}

	// Read message colors

	for(int n = 0; n < PRINT_CLEAR; n++)
	{
		LoadColor(m_clrText[n], SZ_MESSAGECOLORS[n], &rRoot,
			m_pStyle, pSharedStyle);
	}

	// Read prompt flash (optional)

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_PROMPTFLASH,
		Variable::TYPE_BOOL, Variable::TYPE_INT,
		&rRoot, m_pStyle, pSharedStyle) == true)
			m_bEnableFlash = pVar->GetBoolValue();

	// Read number of history lines

	const InfoElem* pElem = rRoot.FindChildConst(SZ_HISTORYLINES,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
		m_nHistoryLines = pElem->GetIntValue();
	else
		m_nHistoryLines = 8;

	// Resize to fit screen

	m_psSize.cx =
		int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth);

	m_psSize.cy =
		int(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight);

	// Set initial out-of-screen position

	SetPosition(0, -m_psSize.cy);

	// Resize background and border texture coordinates to full screen

	Rect rcBackgroundCoords = m_Background.GetTextureCoords();

	rcBackgroundCoords.right = rcBackgroundCoords.left + m_psSize.cx;
	rcBackgroundCoords.bottom = rcBackgroundCoords.top + m_psSize.cy;
	
	m_Background.SetTextureCoords(rcBackgroundCoords);

	// Pre-calculate sizes and distances

	m_nPromptWidth = m_Prompt.GetTextureCoords().GetWidth() + 2;

	m_nColumnWidth = m_pFont->GetAveCharWidth();
	m_nLineHeight = m_pFont->GetLineSpacing();

	// Determine number of lines, columns in each line, and input line columns

	m_nLines = (m_psSize.cy - MARGIN_HEIGHT * 2) / m_nLineHeight - 1;
	m_nColumns = (m_psSize.cx - MARGIN_WIDTH * 2) / m_nColumnWidth;
	m_nInputColumns = m_nColumns * 2;

	// Allocate output lines

	try
	{
		m_ppszLines = new LPWSTR[m_nLines];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(LPWSTR) * m_nLines);
	}

	// Allocate output line colors

	try
	{
		m_pclrLines = new D3DCOLOR[m_nLines];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(D3DCOLOR) * m_nLines);
	}

	try
	{
		for(int n = 0; n < m_nLines; n++)
		{
			// Allocate columns for each output line

			m_ppszLines[n] = new WCHAR[m_nColumns + 1];

			// Clear columns for each output line

			ZeroMemory(m_ppszLines[n], (m_nColumns + 1) * sizeof(WCHAR));

			// Set default color for each output line

			m_pclrLines[n] = 0xFFFFFFFF;
		}
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (m_nColumns + 1));
	}

	// Allocate output line rectangles (extra 1 for input line)

	try
	{
		m_prcLines = new RECT[m_nLines + 1];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(RECT) * (m_nLines + 1));
	}

	// Allocate history lines

	try
	{
		m_ppszHistory = new LPWSTR[m_nHistoryLines];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(LPWSTR) * m_nHistoryLines);
	}

	// Allocate columns for each history line

	try
	{
		for(int n = 0; n < m_nHistoryLines; n++)
		{
			m_ppszHistory[n] = new WCHAR[m_nInputColumns + 1];

			ZeroMemory(m_ppszHistory[n], (m_nInputColumns + 1) * sizeof(WCHAR));
		}
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (m_nInputColumns - 1));
	}

	// Allocate input line

	try
	{
		m_pszInputLine = new WCHAR[m_nInputColumns + 1];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (m_nInputColumns + 1));
	}

	// Clear input line

	ClearInput();

	// Update text line rectangles

	UpdateLines();

	// Notify client console is available

	Game* pClient = dynamic_cast<Game*>(m_rEngine.GetClientInstance());

	if (pClient != NULL)
		pClient->SetConsole(this);
}

void ScreenConsole::OnThemeStyleChange(void)
{
	ScreenOverlapped::OnThemeStyleChange();

	// Get shared style

	ThemeStyle* pSharedStyle =
		m_rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	// Reload prompt material

	LoadMaterialInstance(m_Prompt, SZ_PROMPTTEXTURE,
		NULL, m_pStyle, pSharedStyle);

	// Reload caret material (not optional)

	LoadMaterialInstance(m_Caret, SZ_CARETTEXTURE,
		NULL, m_pStyle, pSharedStyle);

	// Reload border material (not optional)

	LoadMaterialInstance(m_Border, SZ_BORDERTEXTURE,
		NULL, m_pStyle, pSharedStyle);

	// Reload message colors

	for(int n = 0; n < PRINT_CLEAR; n++)
	{
		LoadColor(m_clrText[n], SZ_MESSAGECOLORS[n], NULL,
			m_pStyle, pSharedStyle);
	}

	// Reload prompt flash (optional)

	const Variable* pVar = NULL;
	
	if (LoadVariable(&pVar, SZ_PROMPTFLASH,
		Variable::TYPE_BOOL, Variable::TYPE_INT,
		NULL, m_pStyle, pSharedStyle) == true)
			m_bEnableFlash = pVar->GetBoolValue();
}

void ScreenConsole::Empty(void)
{
	ScreenOverlapped::Empty();

	// Notify client console is unavailable

	Game* pGame =
		dynamic_cast<Game*>(m_rEngine.GetClientInstance());

	if (pGame != NULL)
		pGame->SetConsole(NULL);

	// Deallocate output lines

	if (m_ppszLines != NULL)
	{
		for(int n = 0; n < m_nLines; n++)
			delete[] m_ppszLines[n];

		delete[] m_ppszLines;
	}

	// Deallocate output line colors

	delete[] m_pclrLines;

	// Deallocate output line rectangles

	delete[] m_prcLines;

	// Deallocate input line

	delete[] m_pszInputLine;

	// Deallocate history lines

	if (m_ppszHistory != NULL)
	{
		for(int n = 0; n < m_nHistoryLines; n++)
			delete[] m_ppszHistory[n];

		delete[] m_ppszHistory;
	}

	// Stop timer if any

	m_rEngine.GetTimers().Remove(this, TIMER_SLIDE);
}

DWORD ScreenConsole::GetMemoryFootprint(void) const
{
	return sizeof(ScreenConsole) -
		   sizeof(ScreenOverlapped) * 2 +
		   ScreenOverlapped::GetMemoryFootprint() +
		   m_nLines * m_nColumns * sizeof(WCHAR) +
		   m_nInputColumns * sizeof(WCHAR) +
		   m_nHistoryLines * m_nColumns * sizeof(WCHAR);
}

void ScreenConsole::OnActivate(Screen* pOldActive)
{
	ZOrder(ZORDER_TOP);

	ClearFlag(DISABLED);

	if (ControlManager::IsKeyPressed(VK_SHIFT) == true)
		m_bFullOpen = true;

	if (NULL == m_pMoveTimer)
	{
		m_pMoveTimer = m_rEngine.GetTimers().Add(this,
			0.01f, TIMER_SLIDE);

		if (false == m_bReopen)
			SetPosition(m_ptPos.x, -m_psSize.cy);

		m_nOpenPos = m_bFullOpen ? 0 : -m_psSize.cy / 2;
	}

	m_nMoveAction = MOVE_IN;

	m_rEngine.GetScreens().SetFocusScreen(this);
}

void ScreenConsole::OnDeactivate(Screen* pNewActive)
{
	// If deactivating on closure, exit

	ScreenListIterator pos;

	if (m_rEngine.GetScreens().Find(this, pos) == false)
		return;

	if (false == m_bReclose)
		SetFlag(DISABLED);

	if (NULL == m_pMoveTimer)
	{
		m_pMoveTimer = m_rEngine.GetTimers().Add(this,
			0.01f, TIMER_SLIDE);
	}

	m_nMoveAction = MOVE_OUT;
}

void ScreenConsole::OnTimer(Timer& rTimer)
{
	const int MOVE_DIST = 20;

	m_CachedText.Empty();

	switch(m_nMoveAction)
	{
	case MOVE_IN:
		{
			int nPos = m_ptPos.y;

			if (m_ptPos.y < m_nOpenPos)
			{
				nPos += MOVE_DIST;

				if (nPos > m_nOpenPos)
				{
					nPos = m_nOpenPos;
					m_nMoveAction = MOVE_NONE;

					m_bReopen = false;
				}
			}
			else if (m_ptPos.y == m_nOpenPos)
			{
				m_nMoveAction = MOVE_NONE;

				m_bReopen = false;
			}

			SetPosition(m_ptPos.x, nPos);

			UpdateLines();
		}
		break;
	case MOVE_OUT:
		{
			if (true == m_bReclose)
			{
				int nPos = m_ptPos.y;

				if (m_ptPos.y > m_nOpenPos)
				{
					nPos -= MOVE_DIST;

					if (nPos < m_nOpenPos)
					{
						nPos = m_nOpenPos;

						m_nMoveAction = MOVE_NONE;

						m_bReclose = false;
					}
				}
				else
				{
					m_nMoveAction = MOVE_NONE;

					m_bReclose = false;
				}

				SetPosition(m_ptPos.x, nPos);
			}
			else
			{
				int nPos = m_ptPos.y;

				if (m_ptPos.y > -m_psSize.cy)
				{
					nPos -= MOVE_DIST;

					if (nPos < -m_psSize.cy)
					{
						m_nMoveAction = MOVE_NONE;

						SetFlag(INVISIBLE);
					}
				}
				else
				{
					m_nMoveAction = MOVE_NONE;

					SetFlag(INVISIBLE);
				}

				SetPosition(m_ptPos.x, nPos);
			}

			UpdateLines();
		}
		break;
	default:
		{
			m_rEngine.GetTimers().Remove(this, TIMER_SLIDE);
			m_pMoveTimer = NULL;
		}
		break;
	}
}

void ScreenConsole::OnKeyPress(int nAsciiCode, bool extended, bool alt)
{
	if (nAsciiCode < ' ')
	{
		// Ignore shortcuts except Ctrl+V to paste

		if (KEY_PASTE == nAsciiCode) OnKeyDown(KEY_PASTE);
	}
	else
	{
		// Ignore console toggling key if just toggled

		if (true == m_bJustToggled)
		{
			m_bJustToggled = false;

			Game* pGame =
				dynamic_cast<Game*>(m_rEngine.GetClientInstance());

			if (NULL == pGame) return;

			if (int(VkKeyScanW(WCHAR(nAsciiCode))) ==
				pGame->GetControls().GetControlBoundKey(Game::CONTROL_DEBUG_CONSOLE) ||
				ControlManager::IsKeyPressed(VK_SHIFT))
				return;
		}

		// Ignore input when in motion because of shift key

		if (true == m_bReopen || true == m_bReclose)
			return;

		// Process the character entered

		m_nHistoryPos = -1;

		if ((m_nCaretPos + m_nInputScroll + 1) == m_nInputColumns)
			return;

		if (true == m_bReplaceMode)
		{
			m_pszInputLine[m_nCaretPos + m_nInputScroll] = WCHAR(nAsciiCode);
		}
		else
		{
			for(int n = m_nInputColumns - 1;
				n >= (m_nCaretPos + m_nInputScroll);
				n--)
			{
				m_pszInputLine[n] = m_pszInputLine[n - 1];
			}

			m_pszInputLine[m_nCaretPos + m_nInputScroll] = WCHAR(nAsciiCode);
		}

		if ((m_nColumns - 3) == m_nCaretPos)
		{
			m_nInputScroll++;
		}
		else
		{
			m_nCaretPos++;
		}

		UpdateCaret();
	}
}

void ScreenConsole::OnKeyDown(int nKeyCode)
{
	switch(nKeyCode)
	{
	case VK_BACK:
		{
			m_nHistoryPos = -1;

			if (0 == m_nCaretPos) break;

			if (0 == m_nInputScroll)
				m_nCaretPos--;
			else
				m_nInputScroll--;

			int nLen = int(wcslen(m_pszInputLine));

			for(int n = (m_nCaretPos + m_nInputScroll); n < nLen; n++)
				m_pszInputLine[n] = m_pszInputLine[n + 1];

			UpdateCaret();
		}
		break;
	case VK_LEFT:
		{
			if (0 == (m_nCaretPos + m_nInputScroll))
				break;

			if (m_nInputScroll != 0)
				m_nInputScroll--;
			else
				m_nCaretPos--;

			UpdateCaret();
		}
		break;
	case VK_RIGHT:
		{
			if (m_nInputScroll != 0 &&
			   ((m_nCaretPos + m_nInputScroll) < m_nInputColumns))
			{
				// Scroll the input line if already scrolling

				if (L'\0' == m_pszInputLine[m_nCaretPos + m_nInputScroll])
				  break;

				m_nInputScroll++;
			}
			else
			{
				if (m_nCaretPos < (m_nColumns - 3))
				{
					// Move caret

					if (L'\0' == m_pszInputLine[m_nColumns - 1] &&
					   L'\0' == m_pszInputLine[m_nCaretPos])
					   break;

					m_nCaretPos++;
				}
				else
				{
					// Start scrolling the input line if caret is already at the end

					if (L'\0' == m_pszInputLine[m_nCaretPos + m_nInputScroll] &&
					   L'\0' == m_pszInputLine[m_nCaretPos + m_nInputScroll + 1])
						break;

					m_nInputScroll++;
				}
			}

			UpdateCaret();
		}
		break;
	case VK_UP:
		{
			if (*m_pszInputLine != '\0')
			{
				if (m_nHistoryPos <= 0)
				{
					m_nHistoryPos = m_nHistoryUsed - 1;

					if (m_nHistoryPos <= -1) break;
				}
				else
				{
					if (m_nHistoryPos <= -1)
						m_nHistoryPos = 0;
					else
						m_nHistoryPos--;
				}

				ClearInput();
			}

			if (m_nHistoryPos > -1)
			{
				wcscpy_s(m_pszInputLine, m_nInputColumns - 1,
					m_ppszHistory[m_nHistoryPos]);

				OnKeyDown(VK_END);
			}
		}
		break;
	case VK_DOWN:
		{
			if (m_nHistoryPos >= (m_nHistoryUsed - 1))
			{
				m_nHistoryPos = 0;
			}
			else
			{
				if (m_nHistoryPos <= -1)
					m_nHistoryPos = m_nHistoryUsed - 1;
				else
					m_nHistoryPos++;
			}

			if (m_nHistoryPos == -1) break;

			ClearInput();

			wcscpy_s(m_pszInputLine, m_nInputColumns - 1,
				m_ppszHistory[m_nHistoryPos]);

			OnKeyDown(VK_END);
		}
		break;
	case VK_DELETE:
		{
			// Shift chars in line by one (one gets deleted)

			int nLen = int(wcslen(m_pszInputLine));

			for(int n = (m_nCaretPos + m_nInputScroll); n < nLen; n++)
				m_pszInputLine[n] = m_pszInputLine[n + 1];
		}
		break;
	case VK_HOME:
		{
			m_nCaretPos = 0;
			m_nInputScroll = 0;

			UpdateCaret();
		}
		break;
	case VK_END:
		{
			m_nCaretPos = int(wcslen(m_pszInputLine));

			if (m_nCaretPos > (m_nColumns - 3))
			{
				m_nInputScroll = m_nCaretPos - (m_nColumns - 3);
				m_nCaretPos = m_nColumns - 3;
			}

			UpdateCaret();
		}
		break;
	case VK_INSERT:
		{
			m_bReplaceMode = !m_bReplaceMode;
		}
		break;
	case VK_TAB:
		{
			// List registered commands

			if (m_rEngine.GetCommands().GetCount() == 0)
			{
				m_posNextCommand = m_rEngine.GetCommands().GetEndPos();
				return;
			}

			if (m_posNextCommand == m_rEngine.GetCommands().GetEndPos())
			{
				m_posNextCommand = m_rEngine.GetCommands().GetBeginPos();
			}
			else
			{
				m_posNextCommand++;

				if (m_posNextCommand == m_rEngine.GetCommands().GetEndPos())
					m_posNextCommand = m_rEngine.GetCommands().GetBeginPos();
			}

			ClearInput();

			wcscpy_s(m_pszInputLine, m_nInputColumns - 1,
				m_posNextCommand->first);

			OnKeyDown(VK_END);
		}
		break;
	case VK_RETURN:
		{
			if (NULL == m_pszInputLine) break;

			// Enter command

			m_rEngine.GetCommands().ExecuteStatement(m_pszInputLine);

			// Update history

			for(int n = 0; n < m_nHistoryUsed; n++)
			{
				if (wcscmp(m_ppszHistory[n], m_pszInputLine) == 0)
				{
					m_nHistoryPos = n;

					ClearInput();

					UpdateCaret();

					return;
				}
			}

			m_nHistoryUsed++;

			if (-1 == m_nHistoryPos)
				m_nHistoryPos = m_nHistoryUsed - 1;
			else
				m_nHistoryPos++;

			if (m_nHistoryUsed == m_nHistoryLines)
			{
				m_nHistoryUsed--;

				ZeroMemory(m_ppszHistory[m_nHistoryUsed - 1],
						   sizeof(WCHAR) *m_nInputColumns);

				m_nHistoryPos = m_nHistoryUsed - 1;
			}

			if ((m_nHistoryUsed - m_nHistoryPos) > 1)
			{
				// Move entries down below first

				for(int n = m_nHistoryUsed - 1; n > m_nHistoryPos; n--)
				{
					LPWSTR psz = m_ppszHistory[n];
					m_ppszHistory[n] = m_ppszHistory[n - 1];
					m_ppszHistory[n - 1] = psz;
				}
			}

			wcscpy_s(m_ppszHistory[m_nHistoryPos], m_nInputColumns,
				m_pszInputLine);

			ClearInput();

			UpdateCaret();
		}
		break;
	case KEY_PASTE:
		{
			// Ctrl+V

			if (OpenClipboard(m_rEngine.GetGameWindow()) == FALSE)
			{
				m_rEngine.Print(L"failed to open clipboard.", PRINT_ERROR);
				return;
			}

			LPCWSTR psz = (LPCWSTR)GetClipboardData(CF_UNICODETEXT);

			int nPos = m_nInputScroll + m_nCaretPos;
			int nNewLen = int(wcslen(psz));				
			int nOldLen = int(wcslen(m_pszInputLine + nPos));
			int nOldLast = nPos + nOldLen; // last char in old string
			int nInsertLast = nPos + nNewLen; // last char in inserted string

			// Do not go past allocated memory

			if (nOldLast + nNewLen >= m_nInputColumns)
				break;

			// Offset chars by the number of new chars to be inserted

			for(int nFrom = nPos, nInto = nInsertLast;
				nFrom < nOldLast;
				nFrom++, nInto++)
			{
				m_pszInputLine[nInto] = m_pszInputLine[nFrom];
			}

			// Copy new data

			wcsncpy_s(m_pszInputLine + nPos, m_nInputColumns - nPos,
				psz, nNewLen);

			CloseClipboard();

			// Set cursor to end of new data

			if (nInsertLast > (m_nColumns - 3))
			{
				m_nInputScroll = nInsertLast - (m_nColumns - 3);

				nInsertLast -= m_nInputScroll;
			}
			else
			{
				m_nInputScroll = 0;
			}

			m_nCaretPos = nInsertLast;

			UpdateCaret();
		}
		break;
	}
}

void ScreenConsole::OnMouseLDown(POINT pt)
{
	if (m_rEngine.GetScreens().GetFocusScreen() != this)
		m_rEngine.GetScreens().SetFocusScreen(this);
}

void ScreenConsole::OnLostDevice(bool bRecreate)
{
	m_CachedText.Empty();
}

void ScreenConsole::OnResetDevice(bool bRecreate)
{
	Client* pClient = m_rEngine.GetClientInstance();

	if (NULL == pClient)
		return;

	if (m_psSize.cx != pClient->GetDisplayWidth() ||
	   m_psSize.cy != pClient->GetDisplayHeight())
	{
		// Resize to fit the whole screen if resolution changed

		SetSize(pClient->GetDisplayWidth(), pClient->GetDisplayHeight());

		// Re-position

		if (IsFlagSet(INVISIBLE) == true)
		{
			SetPosition(0, -pClient->GetDisplayHeight());
		}
		else
		{
			if (true == m_bFullOpen)
			{
				SetPosition(0, 0);
			}
			else
			{
				SetPosition(0, -pClient->GetDisplayHeight() / 2);
			}
		}

		// Update graphical elements

		UpdateLines();
	}
}

void ScreenConsole::OnSize(const SIZE& rpsOldSize)
{
	// These tasks will be done first time by Deserialize

	if (NULL == m_ppszLines)
		return;

	if (rpsOldSize.cx == m_psSize.cx && rpsOldSize.cy == m_psSize.cy)
		return;

	// Resize background and border texture coordinates to full screen

	Rect rcBackgroundCoords = m_Background.GetTextureCoords();

	rcBackgroundCoords.right = rcBackgroundCoords.left + m_psSize.cx;
	rcBackgroundCoords.bottom = rcBackgroundCoords.top + m_psSize.cy;
	
	m_Background.SetTextureCoords(rcBackgroundCoords);

	// Re-calculate number of columns and lines

	int nNewLines = (m_psSize.cy - MARGIN_HEIGHT * 2) / m_nLineHeight - 1;
	int nNewColumns = (m_psSize.cx - MARGIN_WIDTH * 2) / m_nColumnWidth;
	int nNewInputColumns = nNewColumns * 2;

	// Allocate output lines

	LPWSTR* ppszNewLines = NULL;

	try
	{
		ppszNewLines = new LPWSTR[nNewLines];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(LPWSTR) * nNewLines);
	}

	// Allocate output line colors

	D3DCOLOR* pclrNewLines = NULL;

	try
	{
		pclrNewLines = new D3DCOLOR[nNewLines];
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(D3DCOLOR) * nNewLines);
	}

	try
	{
		for(int n = 0; n < nNewLines; n++)
		{
			// Allocate columns for each output line

			ppszNewLines[n] = new WCHAR[nNewColumns + 1];

			// Clear columns for each output line

			ZeroMemory(ppszNewLines[n], (nNewColumns + 1) * sizeof(WCHAR));

			if (n < m_nLines)
			{
				// Copy text and color from previously allocated line

				int nLenToCopy = wcslen(m_ppszLines[n]);

				if (nLenToCopy > 0)
				{
					wcsncpy_s(ppszNewLines[n], nNewColumns, m_ppszLines[n],
						nLenToCopy < nNewColumns - 1 ? nLenToCopy : nNewColumns - 1);
				}

				pclrNewLines[n] = m_pclrLines[n];
			}
			else
			{
				// Set default color for each output line

				pclrNewLines[n] = 0xFFFFFFFF;
			}
		}
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (nNewColumns + 1));
	}

	// Allocate output line rectangles (extra 1 for input line)

	LPRECT prcNewLines = NULL;

	try
	{
		prcNewLines = new RECT[nNewLines + 1];

		ZeroMemory(prcNewLines, sizeof(RECT) * (nNewLines + 1));
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(RECT) * (nNewLines + 1));
	}

	// Re-allocate columns for each history line
	// (history line count doesn't change because it's not resolution dependent)

	try
	{
		for(int n = 0; n < m_nHistoryLines; n++)
		{
			LPWSTR pszNewHistoryLine = new WCHAR[nNewInputColumns + 1];

			ZeroMemory(pszNewHistoryLine, (nNewInputColumns + 1) * sizeof(WCHAR));

			int nLenToCopy = wcslen(m_ppszHistory[n]);

			if (nLenToCopy > 0)
			{
				wcsncpy_s(pszNewHistoryLine, nNewInputColumns, m_ppszHistory[n],
					nLenToCopy < nNewInputColumns - 1 ? nLenToCopy : nNewInputColumns - 1);
			}
			
			delete[] m_ppszHistory[n];

			m_ppszHistory[n] = pszNewHistoryLine;
		}
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (nNewInputColumns + 1));
	}

	// Allocate input line

	LPWSTR pszNewInputLine = NULL;

	try
	{
		pszNewInputLine = new WCHAR[nNewInputColumns + 1];

		ZeroMemory(pszNewInputLine, sizeof(WCHAR) * (nNewInputColumns + 1));

		int nLenToCopy = wcslen(m_pszInputLine);

		if (nLenToCopy > 0)
		{
			wcsncpy_s(pszNewInputLine, nNewInputColumns, m_pszInputLine,
				nLenToCopy < nNewInputColumns - 1 ? nLenToCopy : nNewInputColumns - 1);
		}
	}

	catch(std::bad_alloc e)
	{
		UNREFERENCED_PARAMETER(e);

		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(WCHAR) * (nNewInputColumns - 1));
	}

	// Deallocate and re-point old arrays

	for(int n = 0; n < m_nLines; n++)
	{
		delete[] m_ppszLines[n];
		m_ppszLines[n] = NULL;
	}

	delete[] m_ppszLines;
	m_ppszLines = ppszNewLines;	

	delete[] m_pclrLines;
	m_pclrLines = pclrNewLines;

	delete[] m_pszInputLine;
	m_pszInputLine = pszNewInputLine;	

	delete[] m_prcLines;
	m_prcLines = prcNewLines;

	m_nLines = nNewLines;
	m_nColumns = nNewColumns;
	m_nInputColumns = nNewInputColumns;

	// Update placement

	m_nOpenPos = (true == m_bFullOpen) ? 0 : -m_psSize.cy / 2;

	if (m_nInputScroll > m_nInputColumns - 1)
		m_nInputScroll = m_nInputColumns - 1;

	if (m_nCaretPos > m_nInputColumns - 1)
		m_nCaretPos = m_nInputColumns - 1;

	UpdateLines();
	UpdateCaret();
}