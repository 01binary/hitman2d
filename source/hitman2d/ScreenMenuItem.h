/*------------------------------------------------------------------*\
|
| ScreenMenuItem.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Menu Item class
| Created: 08/18/2013
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_MENUITEM_H
#define SCREEN_MENUITEM_H

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
| ScreenMenu class - popup menu
\*----------------------------------------------------------*/

class ScreenMenuItem: public ScreenButtonEx
{
public:
	//
	// Constants
	//

	// Class Name

	static const WCHAR SZ_CLASS[];

	// Elements

	static const WCHAR SZ_CHECKMARK[];
	static const WCHAR SZ_ARROW[];
	static const WCHAR SZ_SUBMENUID[];

private:
	//
	// Members
	//

	//
	int m_nSubMenuID;
	MaterialInstance m_Checkmark;
	MaterialInstance m_Arrow;

public:
	ScreenMenuItem(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenMenuItem(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Menu
	//

	inline int GetSubMenuID(void)
	{
		return m_nSubMenuID;
	}
	
	MaterialInstance& GetCheckmark(void)
	{
		return m_Checkmark;
	}

	MaterialInstance& GetArrow(void)
	{
		return m_Arrow;
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
	virtual void OnMove(const POINT& rptOldPos);
	virtual void OnSize(const SIZE& rpsOldSize);
	virtual void OnThemeStyleChange(void);
	
private:
	//
	// Private Members
	//

	void UpdateLayout(void);
};

} // namespace Hitman2D

#endif	// SCREEN_MENUITEM_H