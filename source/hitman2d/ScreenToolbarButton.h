/*------------------------------------------------------------------*\
|
| ScreenToolbarButton.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Toolbar Button class
| Created: 08/24/2013
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_TOOLBARBUTTON_H
#define SCREEN_TOOLBARBUTTON_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenButtonEx.h"		// using ScreenButtonEx

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {

/*----------------------------------------------------------*\
| ScreenToolbarButton class - toolbar button
\*----------------------------------------------------------*/

class ScreenToolbarButton: public ScreenButtonEx
{
public:
	//
	// Constants
	//

	// Flags

	enum Flags
	{
		// Button can only drop down a menu and does not carry out an action
		DROPDOWN	= ScreenButton::LAST << 0
	};

	// Elements

	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];
	static const WCHAR SZ_MENUID[];
	static const WCHAR SZ_ARROWMATERIAL[];
	static const WCHAR SZ_SEPMATERIAL[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	//
	// Members
	//

	// Menu to drop down if any (if used without DROPDOWN set, creates a secondary button used to show menu)
	int m_nMenuID;

	// Cached arrow position
	Vector2 m_vecArrow;

	// Arrow material
	MaterialInstance m_Arrow;

	// Cached separator position
	Vector2 m_vecSep;

	// Separator material
	MaterialInstance m_Sep;

public:
	ScreenToolbarButton(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenToolbarButton(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Menu ID
	//

	inline int GetMenuID(void) const
	{
		return m_nMenuID;
	}

	inline void SetMenuID(int nMenuID)
	{
		m_nMenuID = nMenuID;
	}

	//
	// Rendering
	//

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//

	virtual void OnMove(const POINT& rptOldPos);
	virtual void OnSize(const SIZE& rpsOldSize);
	virtual void OnCommand(int nCommandID, Screen* pSender, int nParam = 0);
	virtual int OnNotify(int nNotifyID, Screen* pSender, int nParam = 0);
	virtual void OnAction(void);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);

	virtual void OnMouseEnter(void);
	virtual void OnMouseLeave(void);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

private:
	//
	// Private Functions
	//

	void UpdateLayout(void);
	void DropDown(void);
};

} // namespace Hitman2D

#endif // SCREEN_TOOLBARBUTTON_H