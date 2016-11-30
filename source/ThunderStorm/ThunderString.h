/*------------------------------------------------------------------*\
|
| ThunderString.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm string class
| Created: 11/01/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STRING_H
#define THUNDER_STRING_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class String;		// referencing String, declared below
class Stream;		// referencing Stream

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<String> StringArray;
typedef	std::vector<String>::iterator StringArrayIterator;
typedef std::vector<String>::const_iterator StringArrayConstIterator;


/*----------------------------------------------------------*\
| String class
\*----------------------------------------------------------*/

class String
{
private:
	//
	// Constants
	//

	static const WCHAR SZ_NULLSTRINGW[];
	static const char SZ_NULLSTRINGA[];

private:
	//
	// Members
	//

	LPWSTR m_psz;

public:
	String(LPCWSTR pszInit = NULL);
	String(const String& strInit);
	String(LPCSTR pszInit);
	~String(void);

public:
	//
	// Operators
	//

	String& operator=(LPCWSTR pszAssign);
	String& operator=(const String& rAssign);
	String& operator=(LPCSTR pszAssign);

	bool operator==(LPCWSTR pszCompare) const;
	bool operator==(const String& strCompare) const;

	bool operator!=(LPCWSTR pszCompare) const;
	bool operator!=(const String& strCompare) const;

	bool operator>(const String& strCompare) const;
	bool operator>(LPCWSTR pszCompare) const;

	bool operator<(const String& strCompare) const;
	bool operator<(LPCWSTR pszCompare) const;

	void operator+=(LPCWSTR pszAppend);
	void operator+=(const String& strAppend);

	String operator+(LPCWSTR pszAppend);
	String operator+(const String& strAppend);

	operator LPWSTR(void);
	operator LPCWSTR(void) const;

	//
	// Buffer
	//

	LPWSTR GetBuffer(void);
	LPCWSTR GetBufferConst(void) const;

	LPWSTR Allocate(int nLength);
	LPWSTR Reallocate(int nLength);
	
	void Attach(LPWSTR pszBuffer);
	void Detach(void);

	//
	// Operations
	//

	String& ToLower(void);
	String& ToUpper(void);

	//
	// Length
	//

	int GetLength(void) const;
	int GetLengthBytes(void) const;
	bool IsEmpty(void) const;

	//
	// Search
	//

	int ReverseFind(WCHAR ch) const;
	int Find(WCHAR ch) const;

	LPCWSTR Find(LPCWSTR psz) const;

	//
	// Substring
	//

	String Substring(int nStart = 0, int nLen = -1) const;

	String Right(int nLen) const;
	String Left(int nLen) const;
	
	//
	// Loading from resources
	//

	int LoadString(HINSTANCE hInstance, UINT uID);
	static int LoadString(HINSTANCE hInstance, UINT uID, LPWSTR& pszOut);

	//
	// Formatting
	//

	int Format(LPCWSTR pszFormat, ...);
	int Format(HINSTANCE hInstance, UINT uFormat, ...);

	int Format(LPCWSTR pszFormat, va_list pArgs);
	int Format(HINSTANCE hInstance, UINT uFormat, va_list pArgs);

	static int FormatLength(LPCWSTR pszFormat, va_list va);
	static int FormatLength(HINSTANCE hInstance, UINT uID, va_list va);

	//
	// Copying
	//

	void CopyToBuffer(int nBufferLen, LPCWSTR pszSrc, int nSrcLen);

	char* ToAsciiAllocate(void) const;	// Return value needs to be released by user using free()

	//
	// Validation
	//

	inline static bool IsEmpty(LPCWSTR psz)
	{
		return (NULL == psz || L'\0' == *psz);
	}

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_STRING_H