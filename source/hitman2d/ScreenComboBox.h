/*------------------------------------------------------------------*\
|
| ScreenComboBox.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ComboBox control class
| Created: 03/08/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_COMBO_BOX_H
#define SCREEN_COMBO_BOX_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenListBox.h"	// using ScreenListBox, deriving from ScreenButtonEx

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class ScreenListBox;		// referencing ScreenListBox


/*----------------------------------------------------------*\
| ScreenButton class - minimal skinning support
\*----------------------------------------------------------*/

class ScreenComboBox: public ScreenButtonEx
{
public:
	//
	// Constants
	//

	// Elements

	static const WCHAR SZ_LISTBOXSTYLE[];
	static const WCHAR SZ_MAXVISIBLEITEMS[];
	static const WCHAR SZ_ARROWMATERIAL[];
	static const WCHAR SZ_LISTBOXNAME[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	//
	// Members
	//

	String m_strListBoxStyle;
	ScreenListBox* m_pListBox;
	bool m_bDropped;
	int m_nMaxVisibleItems;

	Vector2 m_vecArrow;
	MaterialInstance m_Arrow;

public:
	ScreenComboBox(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenComboBox(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// List
	//

	inline int AddItem(LPCWSTR pszText, LPCWSTR pszDescription = NULL, bool bUpdate = true, bool bLoadStyle = true, ScreenButtonEx** ppCreatedItem = NULL)
	{
		return m_pListBox->AddItem(pszText, pszDescription, bUpdate, bLoadStyle, ppCreatedItem);
	}

	inline ScreenButtonEx* AddItem(bool bUpdate = true, bool bLoadStyle = true)
	{
		return m_pListBox->AddItem(bUpdate, bLoadStyle);
	}

	inline ScreenButtonEx* InsertItem(int nIndex)
	{
		return m_pListBox->InsertItem(nIndex);
	}

	inline int GetItemCount(void) const
	{
		return m_pListBox->GetItemCount();
	}

	inline const ScreenButtonEx* GetItemConst(int nIndex) const
	{
		return m_pListBox->GetItemConst(nIndex);
	}

	inline ScreenButtonEx* GetItem(int nIndex)
	{
		return m_pListBox->GetItem(nIndex);
	}

	inline void RemoveItem(int nIndex)
	{
		m_pListBox->RemoveItem(nIndex);
	}

	inline void RemoveAllItems(void)
	{
		m_pListBox->RemoveAllItems();
	}

	inline ButtonExArrayIterator GetFirstItemPos(void)
	{
		return m_pListBox->GetFirstItemPos();
	}

	inline ButtonExArrayConstIterator GetFirstItemPosConst(void)
	{
		return m_pListBox->GetFirstItemPosConst();
	}

	inline ButtonExArrayIterator GetLastItemPos(void)
	{
		return m_pListBox->GetLastItemPos();
	}

	inline ButtonExArrayConstIterator GetLastItemPosConst(void)
	{
		return m_pListBox->GetLastItemPosConst();
	}

	inline int GetSelectedItem(void) const
	{
		return m_pListBox->GetSelectedItem();
	}

	inline bool IsKeyboardSelection(void) const
	{
		return m_pListBox->IsKeyboardSelection();
	}

	inline void SetSelectedItem(int nSelectedItem)
	{
		m_pListBox->SetSelectedItem(nSelectedItem);
	}

	inline bool IsItemVisible(int nIndex) const
	{
		return m_pListBox->IsItemVisible(nIndex);
	}

	inline void EnsureItemVisible(int nIndex)
	{
		m_pListBox->EnsureItemVisible(nIndex);
	}

	inline void PreallocateItems(int nCount)
	{
		m_pListBox->PreallocateItems(nCount);
	}

	inline int GetMargin(void) const
	{
		return m_pListBox->GetMargin();
	}

	inline void SetMargin(int nMargin)
	{
		m_pListBox->SetMargin(nMargin);
	}

	inline void Update(void)
	{
		UpdateDropListSize();

		m_pListBox->Update();
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
	virtual void OnAction(void);
	virtual int OnNotify(int nNotifyID, Screen* pSender, int nParam = 0);
	virtual void OnMove(const POINT& rptOldPos);
	virtual void OnSize(const SIZE& rpsOldSize);
	virtual void OnThemeStyleChange(void);

private:
	//
	// Private Functions
	//

	void CreateListBox(void);
	void CloseListBox(void);
	void UpdateDropListSize(void);
};

} // namespace Hitman2D

#endif // SCREEN_COMBO_BOX_H