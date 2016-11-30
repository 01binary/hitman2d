/*------------------------------------------------------------------*\
|
| ScreenListBox.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ListBox control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_LISTBOX_H
#define SCREEN_LISTBOX_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenScrollBar.h"	// referencing ScreenScrollBar
#include "ScreenFrame.h"		// referencing ScreenFrame
#include "ScreenButtonEx.h"		// referencing ScreenButtonExArray

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenListBox class - list box
\*----------------------------------------------------------*/

class ScreenListBox: public Screen
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// Pass all keystrokes to parent
		DISABLE_KEYBOARD = Screen::USERFLAG
	};

	enum Notify
	{
		// Selection Changed (param = last sel item)
		NOTIFY_SELECT = 4,

		// Lost focus
		NOTIFY_DEFOCUS = 5
	};

	// Elements

	static const WCHAR SZ_SCROLLBAR_MARGIN[];
	static const WCHAR SZ_FRAMESTYLE[];
	static const WCHAR SZ_ITEMSTYLE[];
	static const WCHAR SZ_SCROLLSTYLE[];
	static const WCHAR SZ_ITEMHEIGHT[];
	static const WCHAR SZ_ALLOWITEMDESELECT[];
	static const WCHAR SZ_ITEM[];

	// Flags

	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	// Index of first visible item
	int m_nFirstVisibleItem;

	// Number of items that could be visible
	int m_nVisibleItems;

	// Index of currently selected item
	int m_nSelectedItem;

	// Height of items
	int m_nItemHeight;

	// Edge margin
	int m_nMargin;

	// Edge margin for scrollbar
	int m_nScrollMargin;

	// Selection from keyboard?
	bool m_bKeyboardSel;

	// Allow item de-selection
	bool m_bItemDeselect;

	// Frame style
	String m_strFrameStyle;

	// Scrollbar style
	String m_strScrollStyle;

	// Item style
	String m_strItemStyle;

	// Frame child
	ScreenFrame* m_pFrame;

	// ScrollBar child
	ScreenScrollBar* m_pScrollbar;

	// Item children
	ButtonExArray m_arItems;

public:
	ScreenListBox(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenListBox(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Items
	//

	int AddItem(LPCWSTR pszText, LPCWSTR pszDescription, bool bUpdate = true, bool bLoadStyle = true, ScreenButtonEx** ppCreatedItem = NULL);

	ScreenButtonEx* AddItem(bool bUpdate = true, bool bLoadStyle = true);

	ScreenButtonEx* InsertItem(int nIndex);

	inline int GetItemCount(void) const
	{
		return int(m_arItems.size());
	}

	inline const ScreenButtonEx* GetItemConst(int nIndex) const
	{
		if (nIndex < 0 || nIndex >= int(m_arItems.size()))
			return NULL;

		return m_arItems[nIndex];
	}

	inline ScreenButtonEx* GetItem(int nIndex)
	{
		if (nIndex < 0 || nIndex >= int(m_arItems.size()))
			return NULL;

		return m_arItems[nIndex];
	}

	void RemoveItem(int nIndex);
	void RemoveAllItems(void);

	inline ButtonExArrayIterator GetFirstItemPos(void)
	{
		return m_arItems.begin();
	}

	inline ButtonExArrayConstIterator GetFirstItemPosConst(void)
	{
		return m_arItems.begin();
	}

	inline ButtonExArrayIterator GetLastItemPos(void)
	{
		return m_arItems.end();
	}

	inline ButtonExArrayConstIterator GetLastItemPosConst(void)
	{
		return m_arItems.end();
	}

	inline int GetSelectedItem(void) const
	{
		return m_nSelectedItem;
	}

	inline bool IsKeyboardSelection(void) const
	{
		// Returns true if last selection was made by keyboard

		return m_bKeyboardSel;
	}

	inline bool GetAllowItemDeselect(void) const
	{
		return m_bItemDeselect;
	}

	inline void SetAllowItemDeselect(bool bItemDeselect)
	{
		m_bItemDeselect = bItemDeselect;
	}

	void SetSelectedItem(int nSelectedItem);

	bool IsItemVisible(int nIndex) const;
	void EnsureItemVisible(int nIndex);

	inline void PreallocateItems(int nCount)
	{
		m_arItems.reserve(nCount);
	}

	inline int GetMargin(void) const
	{
		return m_nMargin;
	}

	void SetMargin(int nMargin);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//

	virtual int OnNotify(int nNotifyID,
		Screen* pSender = NULL, int nParam = 0);

	virtual void OnCommand(int nCommandID,
		Screen* pSender = NULL, int nParam = 0);

	virtual void OnMouseWheel(int nZDelta);

	virtual void OnKeyDown(int nKeyCode);

	virtual void OnFocus(Screen* pOldFocus);

	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnThemeStyleChange(void);

private:
	void UpdateVisible(void);
	void UpdateScrollRange(void);

public:
	void DeserializeItems(const InfoElem& rRoot);
	void Update(void);
};

} // namespace Hitman2D

#endif // SCREEN_LISTBOX_H