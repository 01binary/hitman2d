/*------------------------------------------------------------------*\
|
| ScreenStats.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Statistics Screen class
| Created: 03/03/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_STATS_H
#define SCREEN_STATS_H

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
| ScreenStats class
\*----------------------------------------------------------*/

class ScreenStats: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Class Name
	static const WCHAR SZ_CLASS[];

public:
	ScreenStats(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenStats(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Events
	//

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc = NULL);
};

} // namespace Hitman2D

#endif // SCREEN_STATS_H