/*------------------------------------------------------------------*\
|
| ThunderIniFile.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm profile reading/writing class
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_INI_FILE_H
#define THUNDER_INI_FILE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"		// using String, string functions
#include "ThunderVariable.h"	// using Variable
#include "ThunderStream.h"		// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class IniSection;		// referencing IniSection, declared below
class IniKey;			// referencing IniKey, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, IniSection*> IniSectionMap;
typedef std::map<String, IniSection*>::iterator IniSectionMapIterator;
typedef std::map<String, IniSection*>::const_iterator IniSectionMapConstIterator;

typedef std::vector<IniSection*> IniSectionArray;
typedef std::vector<IniSection*>::iterator IniSectionArrayIterator;
typedef std::vector<IniSection*>::const_iterator IniSectionArrayConstIterator;

typedef std::map<String, IniKey*> IniKeyMap;
typedef std::map<String, IniKey*>::iterator IniKeyMapIterator;
typedef std::map<String, IniKey*>::const_iterator IniKeyMapConstIterator;

typedef std::vector<IniKey*> IniKeyArray;
typedef std::vector<IniKey*>::iterator IniKeyArrayIterator;
typedef std::vector<IniKey*>::const_iterator IniKeyArrayConstIterator;


/*----------------------------------------------------------*\
| IniFile
\*----------------------------------------------------------*/

class IniFile
{
private:
	// File path
	String m_strPath;

	// File title
	String m_strTitle;

	// Map of sections for quick search
	IniSectionMap m_mapSections;

	// Array of sections for sequential traversal
	IniSectionArray m_arSections;

	// Error stack for reporting errors
	ErrorManager* m_pErrorContext;

public:
	IniFile(ErrorManager* pErrorContext = NULL);
	~IniFile(void);

public:
	//
	// Path and Title
	//

	const String& GetPath(void) const;
	const String& GetTitle(void) const;

	//
	// Sections
	//

	IniSection* AddSection(LPCWSTR pszName);

	IniSection* GetSection(LPCWSTR pszName);
	const IniSection* GetSectionConst(LPCWSTR pszName) const;

	void RemoveSection(LPCWSTR pszName);
	void RemoveAllSections(void);
	int GetSectionCount(void) const;

	IniSectionArrayIterator GetFirstSectionPos(void);
	IniSectionArrayIterator GetLastSectionPos(void);

	//
	// Quick read/write values
	//

	void SetStringKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, LPCWSTR pszValue);
	void SetIntKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, int nValue);
	void SetFloatKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, float fValue);
	void SetBoolKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, bool bValue);
	void SetDwordKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, DWORD dwValue);
	void SetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, int nEnumValueIndex, const LPCWSTR* ppszEnumValueNames, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, int nEnumValue, const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, DWORD dwEnumValue, const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, LPCWSTR pszValue);

	LPCWSTR GetStringKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, LPCWSTR pszDefault = NULL) const;
	int GetIntKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, int nDefault = 0) const;
	float GetFloatKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, float fDefault = 0.0f) const;
	bool GetBoolKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, bool bDefault = false) const;
	DWORD GetDwordKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, DWORD dwDefault = 0x0) const;
	int GetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, int nEnumCount, int nDefault = -1) const;
	int GetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, int nDefault = -1) const;
	DWORD GetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, DWORD dwDefault = -1) const;
	LPCWSTR GetEnumKeyValue(LPCWSTR pszSection, LPCWSTR pszKey, LPCWSTR pszDefault = NULL) const;

	//
	// Serialization
	//

	void Serialize(LPCWSTR pszPath) const;
	void Deserialize(LPCWSTR pszPath);

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| IniSection
\*----------------------------------------------------------*/

class IniSection
{
private:
	IniFile& m_rIniFile;

	String m_strName;

	IniKeyMap m_mapKeys;
	IniKeyArray m_arKeys;

public:
	IniSection(IniFile& rIniFile, LPCWSTR pszName);
	~IniSection(void);

public:
	//
	// Container
	//

	IniFile& GetIniFile(void);
	const IniFile& GetIniFileConst(void) const;

	//
	// Name
	//

	const String& GetName(void) const;

	//
	// Keys
	//

	void SetStringKeyValue(LPCWSTR pszKey, LPCWSTR pszValue);
	void SetIntKeyValue(LPCWSTR pszKey, int nValue);
	void SetFloatKeyValue(LPCWSTR pszKey, float fValue);
	void SetBoolKeyValue(LPCWSTR pszKey, bool bValue);
	void SetDwordKeyValue(LPCWSTR pszKey, DWORD dwValue);
	void SetEnumKeyValue(LPCWSTR pszKey, int nEnumValue, const LPCWSTR* ppszEnumValueNames, int nEnumCount, LPCWSTR	pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszKey, int nEnumValue, const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszKey, DWORD dwEnumValue, const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, LPCWSTR pszDefault = NULL);
	void SetEnumKeyValue(LPCWSTR pszKey, LPCWSTR pszValue);

	LPCWSTR GetStringKeyValue(LPCWSTR pszKey, LPCWSTR pszDefault) const;
	int GetIntKeyValue(LPCWSTR pszKey, int nDefault) const;
	float GetFloatKeyValue(LPCWSTR pszKey, float fDefault) const;
	bool GetBoolKeyValue(LPCWSTR pszKey, bool bDefault) const;
	DWORD GetDwordKeyValue(LPCWSTR pszKey, DWORD dwDefault) const;
	int GetEnumKeyValue(LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, int nEnumCount, int nDefault = -1) const;
	int GetEnumKeyValue(LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, const int* pnEnumValues, int nEnumCount, int nDefault = -1) const;
	DWORD GetEnumKeyValue(LPCWSTR pszKey, const LPCWSTR* ppszEnumValueNames, const DWORD* pdwEnumValues, int nEnumCount, DWORD dwDefault = 0) const;
	LPCWSTR GetEnumKeyValue(LPCWSTR pszKey, LPCWSTR pszDefault) const;

	IniKey* GetKey(LPCWSTR pszName);
	const IniKey* GetKeyConst(LPCWSTR pszName) const;

	void RemoveKey(LPCWSTR pszName);
	void RemoveAllKeys(void);

	int GetKeyCount(void) const;

	IniKeyArrayIterator GetFirstKeyPos(void);
	IniKeyArrayIterator GetLastKeyPos(void);

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(LPCWSTR pszBuffer, int* pnLengthRead = NULL);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| IniKey
\*----------------------------------------------------------*/

class IniKey: public Variable
{
private:
	String m_strName;

public:
	IniKey(LPCWSTR pszName, Variable::Types nVarType = Variable::TYPE_UNDEFINED);
	~IniKey(void);

public:
	const String& GetName(void) const;
};

} // namespace ThunderStorm

#endif