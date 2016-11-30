/*------------------------------------------------------------------*\
|
| ScreenStart.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Start Screen class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_START_H
#define SCREEN_START_H

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
| ScreenStart class - main menu
\*----------------------------------------------------------*/

class ScreenStart: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1001,
		ID_BUTTON_FIRST = 101,
		ID_BUTTON_LAST = 106,
		ID_CORNER_FIRST = 201,
		ID_CORNER_LAST = 205,
		ID_SELECTION = 205
	};

	// Element Names

	static const WCHAR SZ_BLURFACTOR[];
	static const WCHAR SZ_BLURMATERIAL[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Fading in for the first time? If so, fade the whole screen
	bool m_bFirstTimeFade;

	// Current mix of back buffer to blur if blurring
	Color m_clrBlurBlend;

	// Blur factor for the shader
	float m_fBlurFactor;

	// Selection image
	Screen* m_pSelection;

	// Downsampled texture of back buffer used to render blur
	// Created to contain generated snapshot on load
	TextureRenderTarget m_Blur;

	// Material for rendering blur
	MaterialInstance m_BlurInst;

	// Material for rendering overlay
	MaterialInstance m_OverlayInst;

public:
	ScreenStart(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenStart(void);

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
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnDeactivate(Screen* pNewActive);

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc = NULL);
	virtual void OnTimer(Timer& rTimer);

	virtual int OnNotify(int nNotifyID,
		Screen* pSender = NULL, int nParam = 0);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Private Functions
	//

	void CacheOverlay(void);
};

} // namespace Hitman2D

#endif // SCREEN_START_H