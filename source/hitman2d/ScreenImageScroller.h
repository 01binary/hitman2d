/*------------------------------------------------------------------*\
|
| ScreenImageScroller.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ImageScroller control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_IMAGE_SCROLLER_H
#define SCREEN_IMAGE_SCROLLER_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenImageScroller class
\*----------------------------------------------------------*/

class ScreenImageScroller: public Screen
{
public:
	//
	// Constants
	//

	// Direction for scrolling image

	enum SCROLLDIR
	{
		SCROLL_XPOS,
		SCROLL_XNEG,
		SCROLL_YPOS,
		SCROLL_YNEG,
		SCROLL_COUNT
	};

	// Elements

	static const WCHAR SZ_SCROLLINTERVAL[];
	static const WCHAR SZ_SCROLLDISTANCE[];
	static const WCHAR SZ_SCROLLDIRECTION[];
	static const LPCWSTR SZ_SCROLLDIRECTIONS[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	float m_fXScale;
	float m_fYScale;
	float m_fScrollInterval;
	int m_nScrollDist;
	int m_nScrollOffset;
	SCROLLDIR m_nScrollDir;

public:
	ScreenImageScroller(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenImageScroller(void);

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
	// Events
	//

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);
	virtual void OnTimer(Timer& rTimer);

	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_IMAGE_SCROLLER_H