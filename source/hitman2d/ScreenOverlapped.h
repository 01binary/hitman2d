/*------------------------------------------------------------------*\
|
| ScreenOverlapped.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Overlapped (Draggable Window) Screen class
| Created: 02/28/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_OVERLAPPED_H
#define SCREEN_OVERLAPPED_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenLabel.h"		// using ScreenLabel
#include "ScreenImage.h"		// using ScreenImage
#include "ScreenFrame.h"		// using ScreenFrame
#include "ScreenComboBox.h"		// using ScreenComboBox, ScreenListBox, ScreenButton/Ex

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenOverlapped class - top-level screen
\*----------------------------------------------------------*/

class ScreenOverlapped: public Screen
{
public:
	//
	// Constants
	//

	// Flags

	enum Flags
	{
		// Enable fading effects for activation, deactivation and closure
		FADEEFFECTS		= Screen::USERFLAG << 0,

		// Enable dragging
		DRAGGABLE		= Screen::USERFLAG << 1,

		// Don't focus children on activate
		NOFOCUSCHILDREN = Screen::USERFLAG << 2,

		// Don't set as foreground on activate
		NOFOREGROUND	= Screen::USERFLAG << 3,

		// Render title text
		TITLE			= Screen::USERFLAG << 4,

		// Render border
		BORDER			= Screen::USERFLAG << 5,

		// Render close button
		CAPTION_CLOSE	= Screen::USERFLAG << 6,

		// Render minimize button
		CAPTION_MIN		= Screen::USERFLAG << 7,

		// Render restore/maximize button
		CAPTION_MAX		= Screen::USERFLAG << 8,

		// Next flag to use
		USERFLAG		= Screen::USERFLAG << 9	
	};

	// Control IDs

	enum
	{
		ID_CLOSE = -1,
		ID_HELP = -2,
		ID_MIN = -3,
		ID_MAX_RESTORE = -4,
		ID_MAINFRAME = -5,
		ID_TITLE = -6
	};

	// Caption Button Types

	enum
	{
		CAP_CLOSE,
		CAP_MIN,
		CAP_MAX,
		CAP_RESTORE,
		CAP_BACKGROUND
	};

	// Timer IDs

	enum Timers
	{
		TIMER_FADE
	};

	// Fading Actions

	enum FadeActions
	{
		FADE_ACTION_NONE,
		FADE_ACTION_IN,
		FADE_ACTION_OUT
	};

	// Fading States

	enum FadeStates
	{
		FADE_STATE_NONE,
		FADE_STATE_OPENING,
		FADE_STATE_CLOSING
	};

	// Dragging States

	enum DragStates
	{
		DRAG_STATE_NONE,
		DRAG_STATE_WAITING,
		DRAG_STATE_DRAGGING
	};

	// Elements
	
	static const WCHAR SZ_TABORDER[];
	static const WCHAR SZ_DEFAULTCOMMAND[];
	static const WCHAR SZ_CANCELCOMMAND[];
	static const WCHAR SZ_TITLESTYLE[];
	static const WCHAR SZ_BORDERSTYLE[];
	static const LPCWSTR SZ_CAPSTYLE[];
	static const WCHAR SZ_TITLEMARGIN[];
	static const WCHAR SZ_CAPMARGIN[];
	static const WCHAR SZ_TITLE[];
	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	//
	// Fading
	//

	Timer* m_pFadeTimer;

	// Inactive screen alpha
	int m_nScreenFadeInactive;

	// Active screen alpha
	int m_nScreenFadeActive;

	// Step change alpha on interval
	int m_nScreenFadeStep;

	// Step change alpha interval
	float m_fScreenFadeInterval;

	int m_nFadeAlpha;

	FadeActions m_nFadeAction;

	FadeStates m_nFadeState;

	//
	// Dragging
	//

	DragStates m_nDragState;

	POINT m_ptDragOffset;

	//
	// Keyboard interface
	//

	// Last child with focus before deactivation
	// (used to re-focus that child on activation)
	Screen* m_pLastFocus;					

	// Pointers to screens to cycle through when user presses tab
	ScreenList m_arTabOrder;

	// Pointers to screens mapped to character mnemonics
	// (when key is pressed, screen is set focus to)
	std::map<int, Screen*> m_mapMnemonics;

	// ID of screen that is "clicked" when user hits enter
	int m_nDefaultCmdID;

	// ID of screen that is "clicked" when user hits esc
	int m_nCancelCmdID;

	//
	// Font
	//

	// Font used for rendering text (optional)
	Font* m_pFont;

	//
	// Built-in Elements
	//

	Screen* m_pBorder;
	Screen* m_pTitle;
	Screen* m_pCaption[3];
	Screen* m_pCaptionBack;

	// Title

	String m_strTitle;

	// Tracking resolution for re-centering

	SIZE m_sizeResolution;

public:
	ScreenOverlapped(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenOverlapped(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Parent Resolution
	//

	static ScreenOverlapped* GetOverlappedParent(Screen* pScreen);

	//
	// Title
	//

	inline const String& GetTitle(void) const
	{
		return m_strTitle;
	}

	void SetTitle(LPCWSTR pszTitle);

	//
	// Font
	//

	inline Font* GetFont(void)
	{
		return m_pFont;
	}

	void SetFont(Font* pFont);

	//
	// Keyboard Interface
	//

	inline void SetMnemonic(Screen* pScreen, int nMnemonic)
	{
		m_mapMnemonics[nMnemonic] = pScreen;
	}

	static void SetMnemonic(Screen* pScreen, LPCWSTR pszText);

	void Tab(bool bNext);

	//
	// Quick Access to Controls
	//

	template<class ControlClass> ControlClass& GetControl(int nControlID)
	{
		ControlClass* pCtl = dynamic_cast<ControlClass*>(
			m_lstChildren.FindByID(nControlID, true));

		if (NULL == pCtl)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
				__FUNCTIONW__, L"ControlClass* pCtl");

		return *pCtl;
	}

	//
	// Graceful Closure
	//

	void Close(void);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnBeginDrag(void);
	virtual void OnEndDrag(void);

	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnBeginFade(void);
	virtual void OnEndFade(void);

	virtual void OnActivate(Screen* pOldActive);
	virtual void OnDeactivate(Screen* pNewActive);

	virtual void OnFocus(Screen* pOldFocus);

	virtual void OnTimer(Timer& rTimer);

	virtual void OnCommand(int nCommandID, Screen* pSender, int nParam);

	virtual int OnNotify(int nNotifyID,
		Screen* pSender = NULL, int nParam = 0);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnThemeChange(Theme& rNewTheme);
	virtual void OnThemeStyleChange(void);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode, bool extended, bool alt);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
};

} // namespace Hitman2D

#endif // SCREEN_OVERLAPPED_H