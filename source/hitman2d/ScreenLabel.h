/*------------------------------------------------------------------*\
|
| ScreenLabel.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Label control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_LABEL_H
#define SCREEN_LABEL_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenLabel class - static text
\*----------------------------------------------------------*/

class ScreenLabel: public Screen
{
public:
	//
	// Constants
	//

	// Elements

	static const WCHAR SZ_TEXT[];
	static const WCHAR SZ_TEXTRENDERFLAGS[];
	static const LPCWSTR SZ_TEXT_FLAGS[];
	static const DWORD DW_TEXT_FLAGS[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	Font* m_pFont;
	DWORD m_dwTextFlags;
	String m_strText;

public:
	ScreenLabel(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenLabel(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Text
	//

	inline const String& GetText(void) const
	{
		return m_strText;
	}

	inline void SetText(LPCWSTR pszText)
	{
		m_strText = pszText;
	}

	inline DWORD GetTextFlags(void) const
	{
		return m_dwTextFlags;
	}

	inline void SetTextFlags(DWORD dwTextFlags)
	{
		m_dwTextFlags = dwTextFlags;
	}

	inline Font* GetFont(void)
	{
		return m_pFont;
	}

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

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);
	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_LABEL_H