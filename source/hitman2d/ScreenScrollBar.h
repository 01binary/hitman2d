/*------------------------------------------------------------------*\
|
| ScreenScrollBar.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScrollBar Control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_SCROLLBAR_H
#define SCREEN_SCROLLBAR_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenScrollBar class
\*----------------------------------------------------------*/

class ScreenScrollBar: public Screen
{
public:
	//
	// Constants
	//

	// Flags

	enum Flags
	{
		// Horizontal scrollbar
		HORIZONTAL	= Screen::USERFLAG << 0,

		// Vertical scrollbar
		VERTICAL	= Screen::USERFLAG << 1,

		// No thumb (when using as spinner control)
		SPINNER		= Screen::USERFLAG << 2,

		// Snap thumb (when using as slider control)
		SLIDER		= Screen::USERFLAG << 3
	};

	// Notification IDs

	enum Notify
	{
		// Scrolled (see NotifyScrollTypes)
		NOTIFY_SCROLL = 3
	};

	// Scroll Types in NOTIFY_SCROLL

	enum NotifyScrollTypes
	{
		// Scrolled by clicking arrow
		SCROLL_ARROW,

		// Scrolled by dragging thumb
		SCROLL_THUMB,

		// Scrolled by dragging on shaft
		SCROLL_SHAFT,

		// Scrolled by using SetValue()
		SCROLL_USER
	};

	// Control Parts

	enum Parts
	{
		// Left arrow (if horizontal)
		PART_LEFTARROW = 0,

		// Up arrow	(if vertical)
		PART_UPARROW = 0,

		// Right arrow (if horizontal)
		PART_RIGHTARROW = 1,

		// Down arrow (if vertical)
		PART_DOWNARROW = 1,

		// Shaft left edge (if horizontal)
		PART_SHAFTLEFT = 2,

		// Shaft top edge (if vertical)
		PART_SHAFTTOP = 2,

		// Shaft center repeat
		PART_SHAFTCENTER = 3,

		// Shaft right edge (if horizontal)
		PART_SHAFTRIGHT = 4,

		// Shaft bottom edge (if vertical)
		PART_SHAFTBOTTOM = 4,

		// Thumb left edge (if horizontal)
		PART_THUMBLEFT = 5,

		// Thumb top edge (if vertical)
		PART_THUMBTOP = 5,

		// Thumb repeat left (if horizontal)
		PART_THUMBREPLEFT = 6,

		// Thumb repeat top (if vertical)
		PART_THUMBREPTOP = 6,

		// Thumb center
		PART_THUMBCENTER = 7,

		// Thumb repeat right (if horizontal)
		PART_THUMBREPRIGHT = 8,

		// Thumb repeat bottom (if vertical)
		PART_THUMBREPBOTTOM = 8,

		// Thumb right edge (if horizontal)
		PART_THUMBRIGHT = 9,

		// thumb bottom edge (if vertical)
		PART_THUMBBOTTOM = 9,

		PART_COUNT
	};

	// Hit test results

	enum PartHitTest
	{
		// Hit left arrow (if horizontal)
		HIT_LEFTARROW = 0,

		// Hit up arrow (if vertical)
		HIT_UPARROW = 0,

		// Hit right arrow (if horizontal)
		HIT_RIGHTARROW = 1,

		// Hit down arrow (if vertical)
		HIT_DOWNARROW = 1,

		// Hit thumb
		HIT_THUMB = 2,

		// Hit shaft left (if horizontal)
		HIT_SHAFTLEFT = 3,

		// Hit shaft top part (if horizontal)
		HIT_SHAFTTOP = 3,

		// Hit shaft right part (if horizontal)
		HIT_SHAFTRIGHT = 4,

		// Hit shaft bottom part (if vertical)
		HIT_SHAFTBOTTOM = 4,

		HIT_COUNT
	};

	// Part states

	enum PartStates
	{
		// Normal
		PART_STATE_NORMAL,

		// Hover (mouse over)
		PART_STATE_HOVER,

		// Pushed (mouse button)
		PART_STATE_PUSHED,

		// Disabled (can't be used)
		PART_STATE_DISABLED,

		// Number of element states defined
		PART_STATE_COUNT
	};

	// Timer event IDs sent to this screen

	enum Timers
	{
		// Wait before auto-scrolling when one of the arrows is pushed
		TIMER_WAITARROW,

		// Auto-scroll when one of the arrows is pushed
		TIMER_ARROW,

		// Wait before auto-scrolling when shaft is pushed
		TIMER_WAITSHAFT,

		// Auto-scroll when shaft is pushed, to that position
		TIMER_SHAFT
	};

	// Elements

	static const WCHAR SZ_MIN[];
	static const WCHAR SZ_MAX[];
	static const WCHAR SZ_POS[];
	static const WCHAR SZ_PAGE[];
	static const WCHAR SZ_WAITARROWAUTOSCROLL[];
	static const WCHAR SZ_ARROWAUTOSCROLLINT[];
	static const WCHAR SZ_WAITSHAFTAUTOSCROLL[];
	static const WCHAR SZ_SHAFTAUTOSCROLLINT[];
	static const WCHAR SZ_DISABLEDBLEND[];
	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];
	static const LPCWSTR SZ_PART_STATES[];
	static const LPCWSTR SZ_PARTS_HORZ[];
	static const LPCWSTR SZ_PARTS_VERT[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Scrolling range minimum
	int m_nMin;

	// Scrolling range maximum
	int m_nMax;

	// Scrolling position
	int m_nPos;

	// Scrolling page size
	float m_fPage;

	// Cached size of shaft (scroll area)
	int m_nShaftSize;

	// Cached size of thumb
	int m_nThumbSize;

	// Current thumb position in pixels
	int m_nThumbPos;

	// Minimum size of top or left edge of thumb
	int m_nMinThumbEdge;

	// Minimum size of center
	int m_nMinThumbCenter;

	// Current size of thumb center (thumb size - 2 thumb edges)
	int m_nThumbCenter;

	// Thumb edge repeat/stretch
	int m_nDistributeEdge;

	// Thumb center repeat/stretch
	int m_nDistributeCenter;

	// Number of pixels per scroll unit
	float m_fPixelsPerUnit;

	// Time to wait before auto-scrolling arrows (in seconds)
	float m_fWaitArrowAutoScroll;

	// Time to wait between arrow auto-scrolls (in seconds)
	float m_fArrowAutoScrollInterval;

	// Time to wait before auto-scrolling shaft (in seconds)
	float m_fWaitShaftAutoScroll;

	// Time to wait between shaft auto-scrolls (in seconds)
	float m_fShaftAutoScrollInterval;

	// Keep track of auto-scroll timer for easy if-set check
	Timer* m_pTimerAutoScroll;

	// Disabled blend color
	Color m_clrDisabledBlend;

	// Was mouse button pushed and not yet let up?
	bool m_bMouseDown;

	// What was hit last time mouse button was pushed
	PartHitTest m_nHitOnMouseDown;

	// Offset when dragging thumb or scrolling shaft
	POINT m_ptDragOffset;

	// Scrollbar element states
	PartStates m_nElemStates[HIT_COUNT];

	// Scrollbar element textures
	MaterialInstance m_Parts[PART_COUNT]
						 [PART_STATE_COUNT];

	// Cached hit test rectangles, also used for rendering
	RECT m_rcHitTest[HIT_COUNT];

	// Sizes for two parts of shaft
	int m_nShaftPartsSize[2];

public:
	ScreenScrollBar(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenScrollBar(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Flags
	//

	virtual void SetFlags(DWORD dwFlags);

	//
	// Values
	//

	inline int GetMin(void) const
	{
		return m_nMin;
	}

	void SetMin(int nMin);

	inline int GetMax(void) const
	{
		return m_nMax;
	}

	void SetMax(int nMax);

	inline int GetValue(void) const
	{
		return m_nPos;
	}

	void SetValue(int nValue);

	inline float GetPageSize(void) const
	{
		return m_fPage;
	}

	void SetPageSize(float fPageSize);

	inline const Color& GetDisabledBlend(void) const
	{
		return m_clrDisabledBlend;
	}

	void SetDisabledBlend(D3DCOLOR clrDisabledBlend);

	//
	// Elements
	//

	inline PartStates GetState(PartHitTest nElem)
	{
		return m_nElemStates[nElem];
	}

	inline MaterialInstance& GetBackground(Parts nElem, PartStates nState)
	{
		return m_Parts[nElem][nState];
	}

	inline const MaterialInstance& GetBackgroundConst(Parts nElem,
		PartStates nState) const
	{
		return m_Parts[nElem][nState];
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

	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDbl(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseLeave(void);

	virtual void OnKeyDown(int nKeyCode);

	virtual void OnFocus(Screen* pOldFocus);
	virtual void OnDefocus(Screen* pNewFocus);

	virtual void OnTimer(Timer& rTimer);

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	virtual void OnThemeStyleChange(void);

private:
	//
	// Private Functions
	//

	void UpdateThumbFromPosition(void);
	void UpdatePositionFromThumb(void);

	void UpdateThumbSize(void);
	void UpdateShaftSize(void);
	void UpdateShaftSplit(void);

	void ScrollShaft(void);

	PartHitTest HitTest(POINT pt);
};

} // namespace Hitman2D

#endif // SCREEN_SCROLLBAR_H