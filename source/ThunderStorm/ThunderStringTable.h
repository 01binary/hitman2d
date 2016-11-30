/*------------------------------------------------------------------*\
|
| ThunderStringTable.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm string table class
| Created: 07/02/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STRING_TABLE_H
#define THUNDER_STRING_TABLE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"		// using Resource, String
#include "ThunderStream.h"			// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, String> StringMap;
typedef std::map<String, String>::iterator StringMapIterator;
typedef std::map<String, String>::const_iterator StringMapConstIterator;


/*----------------------------------------------------------*\
| StringTable class
\*----------------------------------------------------------*/

class StringTable: public Resource
{
private:
	//
	// Members
	//

	StringMap m_mapEntries;

public:
	StringTable(Engine& rEngine);
	virtual ~StringTable(void);

public:
	//
	// Strings
	//

	LPCWSTR GetString(LPCWSTR pszName) const;
	LPCWSTR GetString(int nID) const;

	StringMapConstIterator GetFirstStringPosConst(void) const;
	StringMapConstIterator GetLastStringPosConst(void) const;

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);
};

} // namespace ThunderStorm

#endif // THUNDER_STRING_TABLE_H