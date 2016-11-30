/*------------------------------------------------------------------*\
|
| ScreenConsole.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Console Screen class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_CONSOLE_H
#define SCREEN_CONSOLE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenOverlapped.h"	// using ScreenOverlapped

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenConsole class - command console
\*----------------------------------------------------------*/

class ScreenConsole: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1007
	};

	// Elements
	
	static const WCHAR SZ_PROMPTTEXTURE[];
	static const WCHAR SZ_CARETTEXTURE[];
	static const WCHAR SZ_BORDERTEXTURE[];
	static const WCHAR SZ_PROMPTFLASH[];
	static const WCHAR SZ_HISTORYLINES[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	// Timer IDs

	enum Timers
	{
		TIMER_SLIDE = 1
	};

	// Private Constants

	enum Constants
	{
		MARGIN_WIDTH = 5,
		MARGIN_HEIGHT = 5,
		PROMPT_ALPHA_STEP = 10
	};

	// Move Actions

	enum MoveActions
	{
		MOVE_NONE,
		MOVE_IN,
		MOVE_OUT
	};

	// Key Code Aliases

	enum KeyCodes
	{
		KEY_PASTE = 22
	};
	
private:
	//
	// Members
	//

	// Is console fully open?
	bool m_bFullOpen;

	// Is console re-opening?
	bool m_bReopen;

	// Is console re-closing?
	bool m_bReclose;

	// Was console toggling key just pressed?
	bool m_bJustToggled;

	// Flash prompt character?
	bool m_bEnableFlash;

	// Last time prompt flashed
	float m_fLastFlash;

	// Current prompt alpha
	int m_nPromptAlpha;

	// Whether to increase alpha up or down (to flash)
	bool m_bFlashUp;

	// Editing mode: replace mode instead of insert?
	bool m_bReplaceMode;

	// Adding to last printed line instead of printing a new line?
	bool m_bAddToLine;

	// Width of one character
	int m_nColumnWidth;

	// Height of a line of characters
	int m_nLineHeight;

	// Width of the prompt character
	int m_nPromptWidth;

	// Total number of lines allocated for the console
	int m_nLines;

	// Number of characters per line
	int m_nColumns;

	// Number of characters in the input line
	int m_nInputColumns;

	// First visible line
	int m_nFirstVisible;

	// Vertical position of the console screen when its fully open
	int m_nOpenPos;

	// Number of columns by which input line is currently scrolled
	int m_nInputScroll;

	// Index of column in the input line where the caret is
	int m_nCaretPos;

	// Index of currently used line in history buffer
	int m_nHistoryPos;

	// Number of lines in history buffer
	int m_nHistoryLines;

	// Number of used lines in history buffer
	int m_nHistoryUsed;

	// Text line colors for message types
	Color m_clrText[PRINT_CLEAR];

	// Type of movement if any
	MoveActions m_nMoveAction;

	// Timer fired when console is in movement
	Timer* m_pMoveTimer;

	// Text output lines in the console (each is a pointer to string)
	LPWSTR* m_ppszLines;

	// Line used for inputting text, kept separate from text lines
	LPWSTR m_pszInputLine;

	// Lines of previously entered, saved commands
	LPWSTR* m_ppszHistory;

	// Rectangles calculated for text lines
	LPRECT m_prcLines;

	// Colors for each text output line
	D3DCOLOR* m_pclrLines;

	// Calculated position for the prompt character
	Vector2 m_vecPrompt;

	// Calculated position for the caret character
	Vector2 m_vecCaret;

	// Calculated position for the bottom border
	Vector2 m_vecBorder;

	// Material used to draw prompt character
	MaterialInstance m_Prompt;

	// Material used to draw caret character
	MaterialInstance m_Caret;

	// Material used to draw console border
	MaterialInstance m_Border;

	// Cached text from line display
	TextBlock m_CachedText;

	// Next command for command cycling
	CommandMapIterator m_posNextCommand;

public:
	ScreenConsole(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenConsole(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Rendering
	//

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	//
	// Operations
	//

	void Toggle(bool bFullOpen = false);

	void Print(LPCWSTR pszText = NULL,
		PrintTypes nPrintType = PRINT_MESSAGE,
		bool bLine = true);

	void Scroll(void);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Deinitialization
	//

	virtual void Empty(void);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnActivate(Screen* pOldActive);
	virtual void OnDeactivate(Screen* pNewActive);

	virtual void OnTimer(Timer& rTimer);

	virtual void OnKeyPress(int nAsciiCode, bool extended, bool alt);
	virtual void OnKeyDown(int nKeyCode);

	virtual void OnMouseLDown(POINT pt);

	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnThemeStyleChange(void);

private:
	//
	// Private Operations
	//

	void ClearInput(void);
	void UpdateLines(void);
	void UpdateCaret(void);
};

} // namespace Hitman2D

#endif // SCREEN_CONSOLE_H