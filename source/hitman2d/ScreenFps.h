/*------------------------------------------------------------------*\
|
| ScreenFps.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Frame Rate Counter Screen class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_FPS_H
#define SCREEN_FPS_H

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
| ScreenFps class - frame rate counter
\*----------------------------------------------------------*/

class ScreenFps: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1010
	};

	// Elements

	static const WCHAR SZ_EXTENDED_FONT[];
	static const WCHAR SZ_EXTENDED_BACKGROUND[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Currently extended?
	bool m_bExtended;

	// Last value of frames per second counter
	int m_nLastFps;

	// Last value of seconds per frame counter
	float m_fLastSpf;

	// Font used to render sec per frame and ms per frame
	Font* m_pExtendedFont;

	// Text rect for fps
	Rect m_rcTextPos;

	// Text rect for spf
	Rect m_rcExtTextPosS;

	// Text rect for mspf
	Rect m_rcExtTextPosMs;

	// Background for simple fps display
	MaterialInstance m_normalBackground;
	
	// Background for extended version of fps
	MaterialInstance m_extendedBackground;

	// Current text for fps
	WCHAR m_szFps[8];

	// Current text for spf
	WCHAR m_szSpf[16];

	// Current text for mspf
	WCHAR m_szMspf[16];

public:
	ScreenFps(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenFps(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Extended
	//

	void SetExtended(bool bExtended);
	bool GetExtended(void) const;

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

	virtual void OnMove(const POINT& rptOldPos);
	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);
	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_FPS_H