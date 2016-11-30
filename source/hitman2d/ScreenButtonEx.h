/*------------------------------------------------------------------*\
|
| ScreenButtonEx.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ButtonEx control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_BUTTONEX_H
#define SCREEN_BUTTONEX_H

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
| Declarations
\*----------------------------------------------------------*/

class ScreenButtonEx;		// referencing ScreenButtonEx (declared below)

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<ScreenButtonEx*> ButtonExArray;
typedef std::vector<ScreenButtonEx*>::iterator ButtonExArrayIterator;
typedef std::vector<ScreenButtonEx*>::const_iterator ButtonExArrayConstIterator;


/*----------------------------------------------------------*\
| ScreenButtonEx class - fixed height skinning
\*----------------------------------------------------------*/

class ScreenButtonEx: public ScreenButton
{
public:
	//
	// Constants
	//

	// Icon align

	enum IconAlign
	{
		ICON_ALIGN_LEFT,
		ICON_ALIGN_RIGHT,
		ICON_ALIGN_COUNT
	};

	// Elements

	static const WCHAR SZ_CAPTION[];
	static const WCHAR SZ_DESCRIPTION[];
	static const WCHAR SZ_CAPTIONFONT[];
	static const WCHAR SZ_DESCRIPTIONFONT[];
	static const WCHAR SZ_CAPTIONCOLOR[];
	static const WCHAR SZ_DESCRIPTIONCOLOR[];
	static const WCHAR SZ_DESCRIPTIONCOLORPUSHED[];
	static const WCHAR SZ_ICON[];
	static const WCHAR SZ_ICON_ALIGN[];
	static const WCHAR SZ_LEFTCORNER[];
	static const WCHAR SZ_RIGHTCORNER[];
	static const WCHAR SZ_CENTERREPEAT[];
	static const WCHAR SZ_DISABLESTRETCHING[];
	static const LPCWSTR SZ_ICON_ALIGN_TYPES[];

	// Class Name

	static const WCHAR SZ_CLASS[];

protected:
	// Font used to render description
	Font* m_pFontDescription;

	// Blend used to render description - normal & pushed
	Color m_clrDescription;
	Color m_clrDescriptionPushed;

	// Description text
	String m_strDescription;

	// Icon texture
	MaterialInstance m_Icon;

	// Left corner texture
	MaterialInstance m_Left;

	// Right corner texture
	MaterialInstance m_Right;

	// Center texture that stretches
	MaterialInstance m_CenterRepeat;

	// Cached position of right corner texture
	Vector2 m_vecRightPos;

	// Cached position of center texture
	Vector2 m_vecCenterPos;

	// Cached position of icon texture
	Vector2 m_vecIconPos;

	// Cached position of caption text
	Rect m_rcCaption;

	// Cached position of description text
	Rect m_rcDescription;

	// Margin space
	int m_nMargin;

	// Icon alignment
	IconAlign m_nIconAlign;

	// Repeat center material instead of clamp (performance default)
	bool m_bDisableStretching;

public:
	ScreenButtonEx(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenButtonEx(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Text
	//

	inline const String& GetDescription(void) const
	{
		return m_strDescription;
	}

	inline void SetDescription(LPCWSTR pszDescription)
	{
		m_strDescription = pszDescription;
	}

	//
	// Fonts
	//

	inline Font* GetDescriptionFont(void)
	{
		return m_pFontDescription;
	}

	void SetDescriptionFont(Font* pDescriptionFont);

	//
	// Colors
	//

	inline Color& GetDescriptionColor(void)
	{
		return m_clrDescription;
	}

	inline const Color& GetDescriptionColorConst(void) const
	{
		return m_clrDescription;
	}

	inline void SetDescriptionColor(D3DCOLOR clrDescription)
	{
		m_clrDescription = clrDescription;
	}

	//
	// Textures
	//

	inline MaterialInstance& GetIcon(void)
	{
		return m_Icon;
	}

	inline MaterialInstance& GetBackgroundLeft(void)
	{
		return m_Left;
	}

	inline MaterialInstance& GetBackgroundRight(void)
	{
		return m_Right;
	}

	inline MaterialInstance& GetBackgroundCenterRepeat(void)
	{
		return m_CenterRepeat;
	}

	//
	// Layout
	//

	inline int GetMargin(void) const
	{
		return m_nMargin;
	}

	inline void SetMargin(int nIconMargin)
	{
		m_nMargin = nIconMargin;

		UpdateLayout();
	}

	inline IconAlign GetIconAlign(void) const
	{
		return m_nIconAlign;
	}

	inline void SetIconAlign(IconAlign nIconAlign)
	{
		m_nIconAlign = nIconAlign;

		UpdateLayout();
	}

	inline bool IsTextOnly(void) const
	{
		return m_bTextOnly;
	}

	inline void SetTextOnly(bool bTextOnly)
	{
		m_bTextOnly = bTextOnly;

		UpdateLayout();
	}

	inline bool IsTextAlignLeft(void) const
	{
		return m_bTextAlignLeft;
	}

	inline void SetTextAlignLeft(bool bTextAlignLeft)
	{
		m_bTextAlignLeft = bTextAlignLeft;
	}

	inline bool IsStretchingDisabled(void) const
	{
		return m_bDisableStretching;
	}

	inline void SetStretchingDisabled(bool bDisabled)
	{
		m_bDisableStretching = bDisabled;
	}

	void UpdateLayout(void); // Call after changing texture coordinates

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
	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_BUTTONEX_H