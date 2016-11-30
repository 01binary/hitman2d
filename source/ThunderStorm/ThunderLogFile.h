/*------------------------------------------------------------------*\
|
| ThunderLogFile.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm log file reading/writing class
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_LOG_FILE_H
#define THUNDER_LOG_FILE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderStream.h"		// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

enum PrintTypes		// Print message type
{
	PRINT_MESSAGE,		// Default
	PRINT_ECHO,			// Echo
	PRINT_ERROR,		// Error
	PRINT_WARNING,		// Warning
	PRINT_INFO,			// Informational
	PRINT_DEBUG,		// Debug
	PRINT_CLEAR,		// Clear (action)
	PRINT_COUNT			// Number of types defined
};

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class String;		// using String


/*----------------------------------------------------------*\
| LogFile
\*----------------------------------------------------------*/

class LogFile
{
public:
	//
	// Constants
	//

	static const LPCWSTR SZ_PREFIXES[PRINT_COUNT - 1];
	static const WCHAR SZ_LOGHTMLHEADERCOLOR[];
	static const WCHAR SZ_LOGHTMLHEADERIMAGE[];
	static const WCHAR SZ_LOGHTMLFOOTER[];
	static const WCHAR SZ_LOGHTMLENTRYSTART_OPEN[];
	static const WCHAR SZ_LOGHTMLENTRYSTART_CLOSE[];
	static const WCHAR SZ_LOGHTMLENTRYSPACE[];
	static const WCHAR SZ_LOGHTMLENTRYEND[];
	static const WCHAR SZ_LOGHTMLENTRYENDLINE[];

protected:
	//
	// Members
	//

	// Error stack for reporting errors
	ErrorManager* m_pErrorContext;

	// Stream used to access the file
	Stream m_Stream;

	// Enable adding date and time for each entry?
	bool m_bEnableDateTime;

	// Enable adding type for each entry?
	bool m_bEnablePrefix;

	// Currently adding to existing line?
	bool m_bAddingToLine;

public:
	LogFile(ErrorManager* m_pErrorContext = NULL, bool bEnableDateTime = true, bool bEnablePrefix = true);
	virtual ~LogFile(void);

public:
	//
	// Stream
	//

	const Stream& GetStream(void) const;
	const String& GetPath(void) const;

	String GetTitle(void) const;

	//
	// Options
	//

	bool IsDateTimeEnabled(void) const;
	void EnableDateTime(bool bEnable);

	bool IsPrefixEnabled(void) const;
	void EnablePrefix(bool bEnable);

	//
	// Operations
	//

	virtual void Open(LPCWSTR pszPath);
	virtual void Print(LPCWSTR pszText, PrintTypes nEntryType = PRINT_MESSAGE, bool bLine = true);

	//
	// Deinitialization
	//

	virtual void Empty(void);
};

/*----------------------------------------------------------*\
| LogFileHTML - color HTML logging
\*----------------------------------------------------------*/

class LogFileHTML: public LogFile
{
private:
	// Fixed background image?
	bool m_bBackgroundFixed;

	// Tile background image?
	bool m_bBackgroundRepeat;

	// Center background image horizontally?
	bool m_bBackgroundCenter;

	// Background color
	COLORREF m_rgbBackColor;

	// Text colors
	COLORREF m_arColors[PRINT_CLEAR];

	// Text colors converted to HTML colors
	String m_arHTMLColors[PRINT_CLEAR];

	// Background color converted to HTML
	String m_strHTMLBackColor;

	// Background image path
	String m_strBackImage;

	// Name of font to use
	String m_strFontName;

	// Size of font to use
	int m_nFontSize;

public:
	LogFileHTML(ErrorManager* m_pErrorStack, bool bEnableDateTime = true);
	virtual ~LogFileHTML(void);

public:
	//
	// Background Color
	//

	COLORREF GetBackColor(void) const;
	void SetBackColor(COLORREF rgbColor);

	//
	// Background Image
	//

	const String& GetBackground(void) const;
	void SetBackground(LPCWSTR pszBackground);

	//
	// Background Properties
	//

	bool GetBackgroundFixed(void) const;
	void SetBackgroundFixed(bool bFixed);

	bool GetBackgroundRepeat(void) const;
	void SetBackgroundRepeat(bool bRepeat);

	bool GetBackgroundCenter(void) const;
	void SetBackgroundCenter(bool bCenter);

	//
	// Text Colors
	//

	COLORREF GetTextColor(PrintTypes nType) const;
	void SetTextColor(PrintTypes nType, COLORREF rgbColor);

	//
	// TextureFont Name and Size
	//

	const String& GetFontName(void) const;
	void SetFontName(LPCWSTR pszFontName);

	int GetFontSize(void) const;
	void SetFontSize(int nFontSize);

	//
	// Operations
	//

	virtual void Open(LPCWSTR pszPath);
	virtual void Print(LPCWSTR pszText, PrintTypes nEntryType = PRINT_MESSAGE, bool bLine = true);

	//
	// Deinitialization
	//

	virtual void Empty(void);

private:
	//
	// Private Functions
	//

	void ConvertToHTMLColor(COLORREF rgbColor, String& strDest);
};

/*----------------------------------------------------------*\
| Message - represents printed message
\*----------------------------------------------------------*/

class Message
{
public:
	PrintTypes nEntryType;
	bool bNewLine;
	String strText;
};

} // namespace ThunderStorm

#endif