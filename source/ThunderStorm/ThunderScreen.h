/*------------------------------------------------------------------*\
|
| ThunderScreen.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine screen class(es)
| Created: 10/06/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_SCREEN_H
#define THUNDER_SCREEN_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderGraphics.h"	// using Graphics, Object, Texture, MaterialInstance
#include "ThunderTheme.h"		// using Theme, ThemeStyle

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class InfoElem;			// referencing InfoElem
class Timer;			// referencing Timer
class Font;				// referencing Font
class Engine;			// referencing Engine
class Screen;			// referencing Screen, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::list<Screen*> ScreenList;
typedef std::list<Screen*>::iterator ScreenListIterator;
typedef std::list<Screen*>::const_iterator ScreenListConstIterator;
typedef std::list<Screen*>::reverse_iterator ScreenListReverseIterator;


/*----------------------------------------------------------*\
| ScreenChildManager class - GUI child manager
\*----------------------------------------------------------*/

class ScreenChildManager
{
protected:
	//
	// Members
	//

	ScreenList m_arScreens;

public:
	ScreenChildManager(void);
	~ScreenChildManager(void);

public:
	//
	// Child Screens
	//

	void Add(Screen* pScreen, bool bTopMost = true);
	void Add(Screen* pScreen, ScreenListIterator pos);
	void Remove(Screen* pScreen, bool bDeallocate);
	void Remove(ScreenListIterator pos, bool bDeallocate);
	void RemoveAll(void);

	inline int GetCount(void) const
	{
		return int(m_arScreens.size());
	}

	inline ScreenListIterator GetBeginPos(void)
	{
		return m_arScreens.begin();
	}

	inline ScreenListIterator GetEndPos(void)
	{
		return m_arScreens.end();
	}

	inline ScreenListConstIterator GetBeginPosConst(void) const
	{
		return m_arScreens.begin();
	}

	inline ScreenListConstIterator GetEndPosConst(void) const
	{
		return m_arScreens.end();
	}

	inline Screen* GetTopMost(void)
	{
		return m_arScreens.back();
	}

	inline Screen* GetBottomMost(void)
	{
		return m_arScreens.front();
	}

	bool Find(Screen* pScreenFind, ScreenListIterator& posFind, bool bSearchChildren = false);
	Screen* FindByID(int nID, bool bSearchChildren = false);
	Screen* FindByName(LPCWSTR pszName, bool bSearchChildren = false);
	Screen* FindByClass(LPCWSTR pszClass, bool bSearchChildren = false);

	Screen* ScreenFromPoint(const POINT ptPos, bool bIgnoreDisabled, bool bIgnoreInvisible);

	//
	// Rendering
	//

	void Render(void);
	void Render(const RECT& rrc);

	//
	// Device Events
	//

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	inline void Empty(void)
	{
		RemoveAll();
	}
};

/*----------------------------------------------------------*\
| ScreenManager class - GUI manager
\*----------------------------------------------------------*/

class ScreenManager: public ScreenChildManager
{
private:
	//
	// Members
	//

	// Keep reference to the engine to access class keys and error stack
	Engine& m_rEngine;

	// Currently active screen
	Screen* m_pActiveScreen;

	// Screen that has keyboard focus and receives keyboard events from EngineWndProc
	Screen* m_pFocusScreen;

	// Screen that captured mouse, exclusive receiver of mouse events from EngineWndProc
	Screen* m_pCaptureScreen;

	// Screen the mouse is currently over
	Screen* m_pHoverScreen;

	// Currently loaded theme (empty theme if none loaded)
	Theme m_Theme;

	WCHAR m_szBasePath[256];
	WCHAR m_szBaseExt[8];

public:
	ScreenManager(Engine& rEngine);
	~ScreenManager(void);

public:
	//
	// Screens
	//

	Screen* Create(LPCWSTR pszClass = NULL, Screen* pParent = NULL);
	Screen* Load(LPCWSTR pszPath);
	Screen* Show(LPCWSTR pszPath);

	Screen* GetForegroundScreen(void);
	Screen* SetForegroundScreen(Screen* pForegroundScreen);

	Screen* SetActiveScreen(Screen* pActiveScreen);
	Screen* GetActiveScreen(void) const;

	Screen* SetFocusScreen(Screen* pFocusScreen);
	Screen* GetFocusScreen(void) const;

	Screen* SetCaptureScreen(Screen* pCaptureScreen);
	Screen* GetCaptureScreen(void) const;

	Screen* GetHoverScreen(void) const;
	Screen* SetHoverScreen(Screen* pHoverScreen);

	//
	// Theme
	//

	inline Theme& GetTheme(void)
	{
		return m_Theme;
	}

	void SetTheme(LPCWSTR pszPath);

	//
	// Base Path and Extension
	//

	inline LPCWSTR GetBasePath(void) const
	{
		return m_szBasePath;
	}

	inline LPCWSTR GetBaseExtension(void) const
	{
		return m_szBaseExt;
	}

	void SetBasePath(LPCWSTR pszBasePath);
	void SetBaseExtension(LPCWSTR pszBaseExtension);
	
	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Friends
	//

	friend class Engine;
};

/*----------------------------------------------------------*\
| Screen class - GUI element
\*----------------------------------------------------------*/

class Screen: public Object
{
public:
	//
	// Constants
	//

	// Screen flags

	enum Flags
	{
		// No special options
		DEFAULT					= 0,

		// Do not accept input?
		DISABLED				= 1 << 0,

		// Exclusively accept input?
		MODAL					= 1 << 1,

		// Should be at the top of Z-order (below other topmost screens)?
		TOPMOST					= 1 << 2,

		// Shouldn't be activated?
		NOACTIVATE				= 1 << 3,
		
		// Has buffer?
		BUFFER					= 1 << 4,

		// Buffer has alpha channel?
		BUFFERALPHA				= 1 << 5,

		// Do not render?
		INVISIBLE				= 1 << 6,

		// Render background?
		BACKGROUND				= 1 << 7,

		// Render background color?
		BACKGROUNDCOLOR			= 1 << 8,

		// When parent has buffer and this screen has to be redrawn, draw anything behind it?
		BACKGROUNDTRANSPARENT	= 1 << 9,

		// Clip rendering to size boundaries? (requires hardware support)
		CLIP					= 1 << 10,

		// Start range of user flags
		USERFLAG				= 1 << 11
	};

	// Screen Z-ordering operations

	enum ZOrderOps
	{
		// Bring to top of the order (bottom of the list)
		ZORDER_TOP,

		// Drop to bottom of the order (top of the list)
		ZORDER_BOTTOM,

		// Nudge higher in order (move 1 lower in list)
		ZORDER_UP,

		// Nudge lower in order (move 1 higher in list)
		ZORDER_DOWN,

		// Add to list when it's not yet on the list
		ZORDER_ADD,

		// Remove from list, thus it will be never rendered again
		ZORDER_REMOVE
	};

	enum
	{
		// Timer ID of background animation timer (if animated)
		BACKGROUND_ANIM_TIMER = 16
	};

	// Element Names

	// Root file element name for screen file
	static const WCHAR SZ_ROOT[];

	// Screen Class element name (string)
	static const WCHAR SZ_CLASS[];

	// Screen Style element name (string)
	static const WCHAR SZ_STYLE[];

	// Screen ID element name (int)
	static const WCHAR SZ_ID[];

	// Screen Flags element name (dword array)
	static const WCHAR SZ_FLAGS[];

	// Screen Position element name (int array)
	static const WCHAR SZ_POSITION[];

	// Screen Blend color element name (int array)
	static const WCHAR SZ_BLEND[];

	// Screen Background color element name (int array)
	static const WCHAR SZ_BGCOLOR[];

	// Screen Background material element name (material instance)
	static const WCHAR SZ_BG[];

	// Screen Size element name (int array)
	static const WCHAR SZ_SIZE[];

	// Screen auto-calculated position value (enum)
	static const WCHAR SZ_POSITION_CENTER[];

	// Screen auto-calculated size value (enum)
	static const WCHAR SZ_SIZE_FULLSCREEN[];

	// Base screen flag names (enum array)
	static const LPCWSTR SZ_FLAGS_VALUES[];

	// Base screen flag values (dword array)
	static const DWORD DW_FLAGS_VALUES[];

	// Client position - always 0,0 so it's a constant
	static const Vector2 CLIENT_POS;

protected:
	//
	// Members
	//

	// Class created from
	String m_strClass;

	// Style, if specified (refers to currently loaded gui theme)
	String m_strStyle;

	// Cached pointer to style
	ThemeStyle* m_pStyle;

	// Control ID
	int m_nID;

	// Parent screen
	Screen* m_pParent;

	// Lowest level parent with buffer (not counting self)
	Screen* m_pBufferScreen;

	// Buffer render target texture
	TextureRenderTarget m_Buffer;

	// Buffer material instance for rendering the buffer
	MaterialInstance m_BufferInst;

	// Background color
	Color m_clrBackColor;

	// Blend color (blends buffer if buffered, or background if not buffered)
	Color m_clrBlend;

	// Background texture with changeable coordinates
	MaterialInstance m_Background;

	// Position on in pixels (within parent's client area)
	POINT m_ptPos;

	// Whether to center (useful when changing screen resolution, to re-center)
	bool m_bCenter;

	// Size in pixels
	SIZE m_psSize;

	// Cached position/size on first parent that has a buffer
	Rect m_rcCachedRect;

	// Cached position on first parent that has a buffer
	Vector2 m_vecCachedPos;

	// List of child screens sorted by their Z-order
	ScreenChildManager m_lstChildren;

	// List of screens that need event notifications in addition to parent
	ScreenList m_lstListeners;

public:
	Screen(Engine& rEngine, LPCWSTR pszClass = NULL, Screen* pParent = NULL);
	virtual ~Screen(void);

public:
	//
	// Class
	//

	inline const String& GetClass(void) const
	{
		return m_strClass;
	}

	//
	// Style
	//

	inline const String& GetStyle(void) const
	{
		return m_strStyle;
	}

	virtual void SetStyle(LPCWSTR pszStyle);

	bool LoadMaterialInstance(
		MaterialInstance& rOutInstance,
		LPCWSTR pszElementName,
		const InfoElem* pSource1,
		ThemeStyle* pSource2 = NULL,
		ThemeStyle* pSource3 = NULL);

	bool LoadFont(Font** ppOutFont,
		LPCWSTR pszElementName,
		const InfoElem* pSource1,
		ThemeStyle* pSource2 = NULL,
		ThemeStyle* pSource3 = NULL);

	bool LoadColor(Color& rOutColor,
		LPCWSTR pszElementName,
		const InfoElem* pSource1,
		ThemeStyle* pSource2 = NULL,
		ThemeStyle* pSource3 = NULL);

	bool LoadVariable(const Variable** ppOutVar,
		LPCWSTR pszElementName,		
		Variable::Types nType1,
		Variable::Types nType2,
		const InfoElem* pSource1,
		ThemeStyle* pSource2 = NULL,
		ThemeStyle* pSource3 = NULL);

	//
	// ID
	//

	inline int GetID(void) const
	{
		return m_nID;
	}

	inline void SetID(int nID)
	{
		m_nID = nID;
	}

	//
	// Flags
	//

	virtual void SetFlags(DWORD dwFlags);

	bool IsVisible(void) const;
	bool IsInteractive(void) const;

	//
	// Position
	//

	inline POINT GetPosition(void) const
	{
		return m_ptPos;
	}

	void SetPosition(int x, int y);

	inline void SetPosition(const POINT& rptPos)
	{
		SetPosition(rptPos.x, rptPos.y);
	}

	//
	// Size
	//

	inline SIZE GetSize(void) const
	{
		return m_psSize;
	}

	void SetSize(int cx, int cy);

	inline void SetSize(const SIZE& rpsSize)
	{
		SetSize(rpsSize.cx, rpsSize.cy);
	}

	//
	// Parent
	//

	inline Screen* GetParent(void) const
	{
		return m_pParent;
	}

	void SetParent(Screen* pNewParent);

	bool IsDescendantOf(Screen *pScreen);

	//
	// Background
	//

	inline MaterialInstance& GetBackground(void)
	{
		return m_Background;
	}

	inline const MaterialInstance& GetBackgroundConst(void) const
	{
		return m_Background;
	}

	inline Color& GetBackColor(void)
	{
		return m_clrBackColor;
	}

	inline const Color& GetBackColorConst(void) const
	{
		return m_clrBackColor;
	}

	inline void SetBackColor(D3DCOLOR clrBackColor)
	{
		m_clrBackColor = clrBackColor;
	}

	//
	// Blend
	//

	inline Color& GetBlend(void)
	{
		return m_clrBlend;
	}

	inline const Color& GetBlendConst(void) const
	{
		return m_clrBlend;
	}

	inline void SetBlend(D3DCOLOR clrBlend)
	{
		m_clrBlend = clrBlend;
	}

	// Combined with parent blend, if parent has no buffer

	Color GetFrontBufferBlend(void) const;	

	//
	// Back Buffer
	//

	void CreateBuffer(void);

	inline TextureRenderTarget& GetBuffer(void)
	{
		return m_Buffer;
	}

	void Invalidate(void);
	void Invalidate(const RECT& rc);

	//
	// Z-order and Activation
	//

	void Activate(void);
	void Deactivate(void);

	void ZOrder(ZOrderOps nZOrder);

	//
	// Absolute/Relative Rectangles
	//

	void GetAbsRect(RECT& rrc) const;
	void GetAbsPos(POINT& rpt) const;

	inline Rect GetClientRect(void) const
	{
		return m_BufferInst.GetTextureCoords();
	}

	inline const Rect& GetBufferRect(void) const
	{
		return m_rcCachedRect;
	}

	inline Screen* GetFirstBufferedParent(void) const
	{
		return m_pBufferScreen;
	}

	//
	// Children
	//

	inline ScreenChildManager& GetChildren(void)
	{
		return m_lstChildren;
	}

	//
	// Rendering
	//

	void Render(void);
	void Render(const RECT& rrc);

	//
	// Duplication
	//

	virtual Screen* Duplicate(void) const;

	// Duplicate with different class

	Screen* Duplicate(LPCWSTR pszClass);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Event Listeners
	//

	void RegisterEventListener(Screen* pListener);
	void UnregisterEventListener(Screen* pListener);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	int Release(void);
	virtual void Empty(void);

	//
	// Handlers
	//

	virtual void OnMove(const POINT& rptOldPosition);
	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnActivate(Screen* pOldActive);
	virtual void OnDeactivate(Screen* pNewActive);

	virtual void OnFocus(Screen* pOldFocus);
	virtual void OnDefocus(Screen* pNewFocus);

	virtual void OnCapture(Screen* pOldCapture);
	virtual void OnDecapture(Screen* pNewCapture);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnThemeChange(Theme& rNewTheme);
	virtual void OnThemeStyleChange(void);

	virtual void OnSessionPause(bool bPause);

	virtual void OnTimer(Timer& rTimer);

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc = NULL);
	virtual void OnRenderBackground(Graphics& rGraphics);
	virtual void OnRenderBackground(Graphics& rGraphics, const RECT& rrc);

	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);
	virtual int OnNotify(int nNotifyID, Screen* pSender = NULL, int nParam = 0);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode, bool extended, bool alt);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseLDbl(POINT pt);
	virtual void OnMouseRDown(POINT pt);
	virtual void OnMouseRUp(POINT pt);
	virtual void OnMouseRDbl(POINT pt);
	virtual void OnMouseMDown(POINT pt);
	virtual void OnMouseMUp(POINT pt);
	virtual void OnMouseMDbl(POINT pt);
	virtual void OnMouseWheel(int nZDelta);

	virtual void OnMouseEnter(void);
	virtual void OnMouseLeave(void);

public:
	//
	// Friends
	//

	friend class ScreenChildManager;

private:
	//
	// Private Functions
	//

	void CachePosition(void);
	void CacheStyle(void);
};

} // namespace ThunderStorm

#endif // THUNDER_SCREEN_H