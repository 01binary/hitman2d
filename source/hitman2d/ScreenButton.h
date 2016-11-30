/*------------------------------------------------------------------*\
|
| ScreenButton.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Button control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_BUTTON_H
#define SCREEN_BUTTON_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenButton class - minimal skinning support
\*----------------------------------------------------------*/

class ScreenButton: public Screen
{
public:
	//
	// Constants
	//

	// Flags

	enum Flags
	{
		// Notify on mouse activation
		NOTIFYSTATE	= Screen::USERFLAG << 0,	

		// Is a toggle button
		TOGGLE		= Screen::USERFLAG << 1,

		// Is a radio button (ID defines radio group)
		RADIO		= Screen::USERFLAG << 2,

		// Last flag defined
		LAST		= Screen::USERFLAG << 5
	};

	// Notification IDs

	enum NotifyTypes
	{
		// Selection state changed (see SCREENBUTTON_NOTIFY_SELECT_TYPES)
		NOTIFY_SELECT,

		// Number of state notifications defined
		NOTIFY_COUNT
	};

	// Selection Types in NOTIFY_SELECT

	enum NotifySelectTypes
	{
		// Deselected (with mouse or by focus)
		NOTIFY_SELECT_NONE,

		// Selected (with mouse or by focus)
		NOTIFY_SELECT_HOVER,

		// Pushed (with mouse or keyboard)
		NOTIFY_SELECT_PUSHED
	};

	// Button States

	enum States
	{
		// Normal
		STATE_NORMAL,

		// Hover (when focused or when mouse is over)
		STATE_HOVER,

		// Pushed (by mouse or by keyboard)
		STATE_PUSHED,

		// Does not accept input
		STATE_DISABLED,

		// Normal, while toggled (on)
		STATE_NORMALTOGGLED,
		
		// Hover, while toggled (on)
		STATE_HOVERTOGGLED,

		// Pushed, while toggled (on)
		STATE_PUSHEDTOGGLED,

		// Disabled, while toggled (on)
		STATE_DISABLEDTOGGLED,

		// Number of states defined
		STATE_COUNT
	};	

	// Elements

	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];
	static const WCHAR SZ_COMMAND[];
	static const WCHAR SZ_SELECTED[];
	static const WCHAR SZ_PUSHEDOFFSETX[];
	static const WCHAR SZ_PUSHEDOFFSETY[];
	static const WCHAR SZ_TEXTCOLOR[];
	static const WCHAR SZ_TEXTCOLORPUSHED[];
	static const WCHAR SZ_TEXTCOLORHOVER[];
	static const WCHAR SZ_TEXTCOLORTOGGLED[];
	static const WCHAR SZ_TEXTCOLORHOVERTOGGLED[];
	static const WCHAR SZ_TEXTONLY[];
	static const WCHAR SZ_TEXTLEFT[];
	static const LPCWSTR SZ_STATETEX[];
	static const LPCWSTR SZ_STATEBLEND[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	//
	// Members
	//

	// Current state texture to display as background
	States m_nState;

	// Pushed?
	bool m_bPushed;

	// Mouse over?
	bool m_bHover;

	// If toggle button, is toggled?
	bool m_bToggle;

	// Do not use textures for each state, use the "background" texture
	bool m_bNoStateTextures;

	// Render only text with no background?
	bool m_bTextOnly;

	// Align text left instead of center?
	bool m_bTextAlignLeft;

	// File name of script to execute on command
	String m_strCommandScript;

	// Label to start executing from
	String m_strCommandLabel;

	// Optional text to render
	String m_strText;

	// Font to use for rendering text
	Font* m_pFont;

	// Offset at which to render text when pushed
	POINT m_ptPushedTextOffset;

	// State textures
	MaterialInstance m_states[STATE_COUNT];

	// Blend ARGB for each state
	Color m_clrStateBlends[STATE_COUNT];

	// Text color - normal, pushed, and hover
	Color m_clrText;
	Color m_clrTextPushed;
	Color m_clrTextHover;
	Color m_clrTextToggled;
	Color m_clrTextHoverToggled;

public:
	ScreenButton(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenButton(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);
	
	//
	// Flags override
	//

	virtual void SetFlags(DWORD dwFlags);
	
	//
	// Text
	//

	inline const String& GetText(void) const
	{
		return m_strText;
	}

	inline void SetText(LPCWSTR pszText)
	{
		m_strText = pszText;
	}

	inline const Color& GetTextColor(void) const
	{
		return m_clrText;
	}

	inline void SetTextColor(D3DCOLOR clrText)
	{
		m_clrText = clrText;
	}

	inline const Font* GetFont(void) const
	{
		return m_pFont;
	}

	//
	// Toggle
	//

	inline bool GetToggle(void) const
	{
		return m_bToggle;
	}

	void SetToggle(bool bToggle);

	//
	// Command
	//

	inline const String& GetCommandScript(void) const
	{
		return m_strCommandScript;
	}

	inline void SetCommandScript(LPCWSTR pszCommand)
	{
		m_strCommandScript = pszCommand;
	}

	inline const String& GetCommandLabel(void) const
	{
		return m_strCommandLabel;
	}

	inline void SetCommandLabel(LPCWSTR pszLabel)
	{
		m_strCommandLabel = pszLabel;
	}

	//
	// State
	//

	inline States GetState(void) const
	{
		return m_nState;
	}

	//
	// State Textures
	//

	inline const MaterialInstance&
		GetBackgroundConst(States nState) const
	{
		return m_states[nState];
	}

	inline MaterialInstance& GetBackground(States nState)
	{
		return m_states[nState];
	}

	inline Color& GetBlend(States nState)
	{
		return m_clrStateBlends[nState];
	}

	inline const Color& GetBlendConst(States nState) const
	{
		return m_clrStateBlends[nState];
	}

	inline void SetBlend(States nState, D3DCOLOR clrBlend)
	{
		m_clrStateBlends[nState] = clrBlend;
	}

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

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseEnter(void);
	virtual void OnMouseLeave(void);
	virtual void OnMouseWheel(int nZDelta);
	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);
	virtual void OnFocus(Screen* pOldFocus);
	virtual void OnDefocus(Screen* pNewFocus);
	virtual void OnAction(void);

	virtual void OnThemeStyleChange(void);

private:
	//
	// Private Functions
	//

	void UpdateState(bool bRender);
};

} // namespace Hitman2D

#endif // SCREEN_BUTTON_H