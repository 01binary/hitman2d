/*------------------------------------------------------------------*\
|
| ScreenPause.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Pause Screen class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_PAUSE_H
#define SCREEN_PAUSE_H

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
| ScreenPause class - pause screen
\*----------------------------------------------------------*/

class ScreenPause: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1006,

		ID_BUTTON_FIRST = 103,
		ID_BUTTON_LAST = 107,

		ID_IMAGE_FIRST = 101,
		ID_IMAGE_LAST = 102
	};

	// Elements

	static const WCHAR SZ_VERTLOGOOFFSET[];
	static const WCHAR SZ_HORZLOGOOFFSET[];
	static const WCHAR SZ_VERTBUTTONOFFSET[];
	static const WCHAR SZ_HORZBUTTONOFFSET[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Selection mark that gets moved around
	Screen* m_pSelection;

	// Blur factor for the shader
	float m_fBlurFactor;

	// Dynamically generated blur texture
	TextureRenderTarget m_Blur;

	// Blur material
	MaterialInstance m_BlurInst;

	// Overlay material
	MaterialInstance m_OverlayInst;

public:
	ScreenPause(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenPause(void);

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

	virtual void OnRenderBackground(Graphics& rGraphics);

	virtual int OnNotify(int nNotifyID,
		Screen* pSender = NULL, int nParam = 0);

	virtual void OnBeginFade(void);
	virtual void OnEndFade(void);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnThemeStyleChange(void);
	
	//
	// Private Functions
	//

	void CacheLayout(const InfoElem* pDocRoot = NULL);
	void CacheOverlay(void);
};

} // namespace Hitman2D

#endif // SCREEN_PAUSE_H