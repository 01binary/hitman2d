/*------------------------------------------------------------------*\
|
| ScreenRadioButton.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D RadioButton control class
| Created: 03/31/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_RADIOBUTTON_H
#define SCREEN_RADIOBUTTON_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenCheckBox.h"		// using ScreenCheckBox

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenRadioButton class
\*----------------------------------------------------------*/

class ScreenRadioButton: public ScreenCheckBox
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_CLASS[];

public:
	ScreenRadioButton(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenRadioButton(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);
};

} // namespace Hitman2D

#endif // SCREEN_RADIOBUTTON_H