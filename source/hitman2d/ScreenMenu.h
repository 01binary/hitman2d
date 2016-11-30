/*------------------------------------------------------------------*\
|
| ScreenMenu.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Popup Menu class
| Created: 08/18/2013
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_MENU_H
#define SCREEN_MENU_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenFrame.h"		// using ScreenFrame

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class ScreenMenuItem;			// using ScreenMenuItem

/*----------------------------------------------------------*\
| ScreenMenu class - popup menu
\*----------------------------------------------------------*/

class ScreenMenu: public ScreenFrame
{
public:
	//
	// Constants
	//

	// Class Name

	static const WCHAR SZ_CLASS[];

	// Elements

	static const WCHAR SZ_POPUPOFFSETX[];
	static const WCHAR SZ_POPUPOFFSETY[];

private:
	// Edge margin between frame and contained items
	int m_nMargin;

	// Offset from parent when popping up
	POINT m_ptPopupOffset;

public:
	ScreenMenu(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenMenu(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Menu
	//

	inline int GetMargin(void) const
	{
		return m_nMargin;
	}

	inline void SetMargin(int nMargin)
	{
		m_nMargin = nMargin;

		this->UpdateLayout();
	}

	inline POINT GetPopupOffset(void) const
	{
		return m_ptPopupOffset;
	}
	
	inline void SetPopupOffset(POINT ptPopupOffset)
	{
		m_ptPopupOffset.x = ptPopupOffset.x;
		m_ptPopupOffset.y = ptPopupOffset.y;
	}

	void Popup(POINT ptScreenLocation);
	void Popup(Screen* pBottomLeftOf);
	void Hide(void);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//

	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);
	virtual int OnNotify(int nNotifyID, Screen* pSender = NULL, int nParam = 0);

	virtual void OnKeyDown(int nKeyCode);

private:
	//
	// Private Members
	//

	Screen* GetFirstItem(void);
	void PopupSubMenu(Screen* pSourceItem);
	void UpdateLayout(void);
};

} // namespace Hitman2D

#endif // SCREEN_MENU_H