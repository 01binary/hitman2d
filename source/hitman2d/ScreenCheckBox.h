/*------------------------------------------------------------------*\
|
| ScreenCheckBox.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D CheckBox control class
| Created: 03/30/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_CHECKBOX_H
#define SCREEN_CHECKBOX_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenButton.h"	// using ScreenButton

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenCheckBox class
\*----------------------------------------------------------*/

class ScreenCheckBox: public ScreenButton
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_CLASS[];

public:
	ScreenCheckBox(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenCheckBox(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Events
	//
	
	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);
};

} // namespace Hitman2D

#endif // SCREEN_BUTTON_H