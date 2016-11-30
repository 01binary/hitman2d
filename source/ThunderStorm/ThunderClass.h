/*------------------------------------------------------------------*\
|
| ThunderClass.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine dynamic class creation
| Created: 10/17/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_CLASS_H
#define THUNDER_CLASS_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"	// using String

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;		// referencing Engine
class Object;		// referencing Object

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef Object* (*PCREATECLASSCALLBACK) (Engine& rEngine, LPCWSTR pszClass, Object* pOwner);

typedef std::map<String, PCREATECLASSCALLBACK> CallbackMap;
typedef std::map<String, PCREATECLASSCALLBACK>::iterator CallbackMapIterator;
typedef std::map<String, PCREATECLASSCALLBACK>::const_iterator CallbackMapConstIterator;


/*----------------------------------------------------------*\
| ClassManager class
\*----------------------------------------------------------*/

class ClassManager
{
private:
	// Keep reference to the engine
	Engine& m_rEngine;

	// Map of registered classes by class name
	CallbackMap m_mapClasses;

public:
	ClassManager(Engine& rEngine);
	~ClassManager(void);

public:
	//
	// Registration
	//

	void Register(LPCWSTR pszClass, PCREATECLASSCALLBACK pCreateCallback);
	void Unregister(LPCWSTR pszClass);

	//
	// Creation
	//

	Object* Create(LPCWSTR pszClass, Object* pOwner = NULL);

	//
	// Enumeration
	//

	CallbackMapIterator GetBeginPos(void);
	CallbackMapIterator GetEndPos(void);
	CallbackMapConstIterator GetBeginPosConst(void) const;
	CallbackMapConstIterator GetEndPosConst(void) const;

	int GetCount(void) const;

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_CLASS_H