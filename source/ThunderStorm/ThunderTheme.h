/*------------------------------------------------------------------*\
|
| ThunderTheme.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm theme class(es)
| Created: 10/22/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_THEME_H
#define THUNDER_THEME_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderGraphics.h"	// using Graphics, Object, Texture, MaterialInstance
#include "ThunderVariable.h"	// using VariableManager
#include "ThunderFont.h"		// using Font

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class ThemeStyle;				// referencing ThemeStyle, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::multimap<String, ThemeStyle*> ThemeStyleMap;
typedef std::multimap<String, ThemeStyle*>::iterator ThemeStyleMapIterator;
typedef std::multimap<String, ThemeStyle*>::const_iterator ThemeStyleMapConstIterator;
typedef std::pair<ThemeStyleMapConstIterator, ThemeStyleMapConstIterator> ThemeStyleMapConstRange;

typedef std::vector<ThemeStyle*> ThemeStyleArray;
typedef std::vector<ThemeStyle*>::iterator ThemeStyleArrayIterator;

typedef std::map<String, MaterialInstance> MaterialInstanceMap;
typedef std::map<String, MaterialInstance>::iterator MaterialInstanceMapIterator;
typedef std::map<String, MaterialInstance>::const_iterator MaterialInstanceMapConstIterator;

typedef std::map<String, Font*> FontMap;
typedef std::map<String, Font*>::iterator FontMapIterator;
typedef std::map<String, Font*>::const_iterator FontMapConstIterator;

typedef std::map<String, Color> ColorMap;
typedef std::map<String, Color>::iterator ColorMapIterator;
typedef std::map<String, Color>::const_iterator ColorMapConstIterator;


/*----------------------------------------------------------*\
| Theme - GUI theme
\*----------------------------------------------------------*/

class Theme
{
public:
	//
	// Constants
	//

	// Element names

	// Root theme file element name
	static const WCHAR SZ_THEME[];

	// Default theme style name
	static const WCHAR SZ_THEMESTYLE_DEFAULT[];

private:
	//
	// Members
	//

	// Keep reference to the engine used for loading textures
	Engine& m_rEngine;

	// Name of this theme
	String m_strName;					

	// Styles this theme has
	ThemeStyleArray m_arStyles;

	// Styles mapped by name
	ThemeStyleMap m_mapStylesByName;

	// Styles mapped by screen classes they are made for
	ThemeStyleMap m_mapStylesByClass;

	// Additional metadata for the theme
	VariableManager m_Variables;

public:
	Theme(Engine& rEngine);
	~Theme(void);

public:
	//
	// Name
	//

	inline const String& GetName(void) const
	{
		return m_strName;
	}

	inline void SetName(LPCWSTR pszName)
	{
		m_strName = pszName;
	}

	//
	// Styles
	//

	ThemeStyle* GetStyle(LPCWSTR pszName);
	ThemeStyle* GetStyle(LPCWSTR pszName, LPCWSTR pszClassName);
	ThemeStyle* GetDefaultClassStyle(LPCWSTR pszClassName);

	ThemeStyleArrayIterator GetBeginStyle(void);
	ThemeStyleArrayIterator GetEndStyle(void);

	int GetStyleCount(void) const;

	//
	// Variables
	//

	inline const Variable* GetVariable(LPCWSTR pszName) const
	{
		return m_Variables.FindConst(pszName);
	}

	//
	// Serialization
	//

	void Deserialize(LPCWSTR pszPath);	
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Friends
	//

	friend class ThemeStyle;
};

/*----------------------------------------------------------*\
| ThemeStyle - GUI theme style
\*----------------------------------------------------------*/

class ThemeStyle
{
public:
	//
	// Constants
	//

	// Theme style element name
	static const WCHAR SZ_THEMESTYLE[];

	// Material theme style element name
	static const WCHAR SZ_THEMESTYLEPART_MATERIAL[];

	// Font theme style element name
	static const WCHAR SZ_THEMESTYLEPART_FONT[];

	// Color theme style element name
	static const WCHAR SZ_THEMESTYLEPART_COLOR[];

private:
	//
	// Members
	//

	Theme& m_rTheme;				// Keep reference to theme for loading textures

	String m_strName;				// Style name
	String m_strClass;				// Class of screens this style applies to

	MaterialInstanceMap
		m_mapMaterialElems;			// Texture elements

	FontMap m_mapFontElems;			// Font elements

	ColorMap m_mapColorElems;		// Color elements

	VariableManager m_Variables;	// Additional metadata for the style

public:
	ThemeStyle(Theme& rTheme);
	~ThemeStyle(void);

public:
	//
	// Theme
	//

	inline Theme& GetTheme(void)
	{
		return m_rTheme;
	}

	inline const Theme& GetThemeConst(void) const
	{
		return m_rTheme;
	}

	//
	// Name
	//

	inline const String& GetName(void) const
	{
		return m_strName;
	}

	inline void SetName(LPCWSTR pszName)
	{
		m_strName = pszName;
	}

	//
	// Class
	//

	inline const String& GetClass(void) const
	{
		return m_strClass;
	}

	inline void SetClass(LPCWSTR pszClass)
	{
		m_strClass = pszClass;
	}

	//
	// Elements
	//

	const MaterialInstance* GetMaterialInstanceConst(LPCWSTR pszName) const;
	const Font* GetFontConst(LPCWSTR pszName) const;

	MaterialInstance* GetMaterialInstance(LPCWSTR pszName);
	Font* GetFont(LPCWSTR pszName);

	const Color* GetColor(LPCWSTR pszName) const;
	const Variable* GetVariable(LPCWSTR pszName) const;
	const Variable* GetVariable(LPCWSTR pszName, Variable::Types nType) const;

	//
	// Serialization
	//
	
	void Deserialize(const InfoElem& rRoot);

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_THEME_H