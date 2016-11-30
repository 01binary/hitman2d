/*------------------------------------------------------------------*\
|
| ScreenProgressBar.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ProgressBar control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_PROGRESSBAR_H
#define SCREEN_PROGRESSBAR_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenProgressBar class
\*----------------------------------------------------------*/

class ScreenProgressBar: public Screen
{
public:
	//
	// Constants
	//

	// Elements

	static const WCHAR SZ_LEFTCORNER[];
	static const WCHAR SZ_RIGHTCORNER[];
	static const WCHAR SZ_CENTERREPEAT[];
	static const WCHAR SZ_BLIP[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	// Progress Range Minimum
	int m_nProgressMin;

	// Progress Range Maximum
	int m_nProgressMax;

	// Progress Value
	int m_nProgress;

	// Cached width of center part
	int m_nCenterWidth;

	// Cached number of blips to render
	int m_nBlips;

	// Left corner texture
	MaterialInstance m_Left;

	// Right corner texture
	MaterialInstance m_Right;

	// Blip texture
	MaterialInstance m_Blip;

	// Center repeating texture
	MaterialInstance m_Center;

	// Cached position of center texture
	Vector2 m_vecCenter;

	// Cached position of right texture
	Vector2 m_vecRight;

public:
	ScreenProgressBar(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenProgressBar(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Textures
	//

	inline MaterialInstance& GetBackgroundCenter(void)
	{
		return m_Center;
	}

	//
	// Progress
	//

	inline int GetProgressMin(void) const
	{
		return m_nProgressMin;
	}

	void SetProgressMin(int nProgressMin);

	inline int GetProgressMax(void) const
	{
		return m_nProgressMax;
	}

	void SetProgressMax(int nProgressMax);

	inline int GetProgress(void) const
	{
		return m_nProgress;
	}

	void SetProgress(int nProgress);

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
	virtual void OnSize(const SIZE& rpsOldSize);

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);

	virtual void OnThemeStyleChange(void);

	//
	// Private Functions
	//

	void CacheProgress(void);
};

} // namespace Hitman2D

#endif // SCREEN_PROGRESSBAR_H