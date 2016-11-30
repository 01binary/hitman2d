/*------------------------------------------------------------------*\
|
| ScreenExit.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Exit Screen class
| Created: 03/03/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_EXIT_H
#define SCREEN_EXIT_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenOverlapped.h"	// using ScreenOverlapped

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenExit class
\*----------------------------------------------------------*/

class ScreenExit: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1005,

		ID_OK = 101,
		ID_CANCEL = 102
	};

	// Class Name
	static const WCHAR SZ_CLASS[];

public:
	ScreenExit(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenExit(void) {};

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Events
	//

	virtual void OnCommand(int nCommandID,
		Screen* pSender = NULL, int nParam = 0);
};

} // namespace Hitman2D

#endif // SCREEN_EXIT_H