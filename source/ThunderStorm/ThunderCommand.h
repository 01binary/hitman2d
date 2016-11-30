/*------------------------------------------------------------------*\
|
| ThunderCommand.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine script classes
| Created: 09/04/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_SCRIPT_H
#define THUNDER_SCRIPT_H

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
class Variable;		// referencing Variable
class Command;		// referencing Command, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, Command*> CommandMap;
typedef std::map<String, Command*>::iterator CommandMapIterator;
typedef std::map<String, Command*>::const_iterator CommandMapConstIterator;

typedef std::vector<Variable> VariableArray;
typedef std::vector<Variable>::iterator VariableArrayIterator;
typedef std::vector<Variable>::const_iterator VariableArrayConstIterator;

typedef int (*PCOMMANDCALLBACK) (Engine& rEngine, VariableArray& rParams);


/*----------------------------------------------------------*\
| Command class - command execution
\*----------------------------------------------------------*/

class Command
{
private:
	// Callback function pointer
	PCOMMANDCALLBACK m_pfnCallback;

public:
	Command(PCOMMANDCALLBACK pfnInitCallback = NULL);
	~Command(void);

public:
	//
	// Callback
	//

	PCOMMANDCALLBACK GetCallback(void) const;
	void SetCallback(PCOMMANDCALLBACK pCallback);

	//
	// Execution
	//

	int Execute(Engine& rEngine, LPCWSTR pszParams = NULL);
	int Execute(Engine& rEngine, VariableArray& rParams);
};

/*----------------------------------------------------------*\
| CommandManager class
\*----------------------------------------------------------*/

class CommandManager
{
private:
	// Keep a pointer to the engine for error reporting and changing variables
	Engine& m_rEngine;

	// Command pointers mapped to strings
	CommandMap m_mapCommands;

	WCHAR m_szBasePath[128];
	WCHAR m_szBaseExt[8];

public:
	CommandManager(Engine& rEngine);
	~CommandManager(void);

public:
	//
	// Base Path and Extension
	//

	inline LPCWSTR GetBasePath(void) const
	{
		return m_szBasePath;
	}

	inline void SetBasePath(LPCWSTR pszBasePath)
	{
		wcscpy_s(m_szBasePath, sizeof(m_szBasePath) / sizeof(WCHAR),
			pszBasePath);
	}

	inline LPCWSTR GetBaseExtension(void) const
	{
		return m_szBaseExt;
	}

	inline void SetBaseExtension(LPCWSTR pszBaseExtension)
	{
		wcscpy_s(m_szBaseExt, sizeof(m_szBaseExt) / sizeof(WCHAR),
			pszBaseExtension);
	}

	//
	// Commands
	//

	Command* Register(LPCWSTR pszName, PCOMMANDCALLBACK pCallback);

	Command* Find(LPCWSTR pszName);

	void Unregister(LPCWSTR pszName);
	void UnregisterAll(void);

	int GetCount(void) const;

	CommandMapIterator GetBeginPos(void);
	CommandMapIterator GetEndPos(void);

	CommandMapConstIterator GetBeginPosConst(void) const;
	CommandMapConstIterator GetEndPosConst(void) const;

	//
	// Parsing and Execution
	//

	void ExecuteScriptFile(LPCWSTR pszPath, LPCWSTR pszLabel = NULL);
	void ExecuteScript(LPCWSTR pszText, LPCWSTR pszLabel = NULL);

	int ExecuteStatement(LPCWSTR pszLine);
	int ExecuteStatement(LPCWSTR pszName, LPCWSTR pszParams);

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

#endif