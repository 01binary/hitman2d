/*------------------------------------------------------------------*\
|
| ScreenCredits.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Credits Screen class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_CREDITS_H
#define SCREEN_CREDITS_H

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
| Declarations
\*----------------------------------------------------------*/

class ScreenImage;				// referencing ScreenImage


/*----------------------------------------------------------*\
| ScreenCredits class
\*----------------------------------------------------------*/

class ScreenCredits: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1008
	};

	// Elements

	static const WCHAR SZ_CREDITSPATH[];
	static const WCHAR SZ_SCROLLINTERVAL[];
	static const WCHAR SZ_SCROLLDISTANCE[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	enum Timers
	{
		TIMER_SCROLL = 1
	};

private:
	//
	// Members
	//

	// Height of one line of text
	int m_nLineHeight;

	// Number of lines that can be visible at the same time
	int m_nVisibleLines;

	// Index of first currently visible line
	int m_nFirstVisibleLine;

	// Offset of first line rendered
	int m_nRenderOffset;

	// Pixels scrolled each time
	int m_nScrollDistance;

	// Scroll interval in seconds
	float m_fScrollInterval;

	// Timer that scrolls lines
	Timer* m_pScrollTimer;

	// Pointer to screen that contains scrolling text
	ScreenImage* m_pTextContainer;

	// Array of text lines parsed from credits text	
	std::vector<LPWSTR> m_arLines;

public:
	ScreenCredits(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenCredits(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Deinitialization
	//

	virtual void Empty(void);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnTimer(Timer& rTimer);
	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_CREDITS_H