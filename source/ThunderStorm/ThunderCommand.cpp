/*------------------------------------------------------------------*\
|
| ThunderCommand.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine command class
| Created: 09/04/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderEngine.h"			// using CTunderEngine, defining Command/Map, using Variable
#include "ThunderStream.h"			// using Stream
#include "ThunderGlobals.h"			// using IsValidNameChar
#include <stdarg.h>					// using va_list, va_start, va_end, va_arg
#include <shlobj.h>					// using PathResolve

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Command class
\*----------------------------------------------------------*/

Command::Command(PCOMMANDCALLBACK pfnInitCallback):	m_pfnCallback(pfnInitCallback)
{
}

Command::~Command(void)
{
}

PCOMMANDCALLBACK Command::GetCallback(void) const
{
	return m_pfnCallback;
}

void Command::SetCallback(PCOMMANDCALLBACK pCallback)
{
	m_pfnCallback = pCallback;
}

int Command::Execute(Engine& rEngine, VariableArray& rParams)
{
	return (*m_pfnCallback)(rEngine, rParams);
}

int Command::Execute(Engine& rEngine, LPCWSTR pszParams)
{
	VariableArray arParams;

	// If no params, just execute without them

	if (NULL == pszParams)
		return (*m_pfnCallback)(rEngine, arParams);

	// Allow 8 params before having to resize
	
	arParams.reserve(8);

	// Parse params from string into arParams as Variables

	for(;;)
	{
		// Attempt to read this param

		Variable varCurParam;

		int nReadLen = varCurParam.FromString(pszParams);

		// If invalid, stop reading params

		if (0 == nReadLen) break;

		// Otherwise, add this param to the param list

		arParams.push_back(varCurParam);

		// Skip the data for the param just read

		pszParams += nReadLen;

		// Skip any white space

		EatWhiteSpace(&pszParams);

		// Examine what follows

		if (L',' == *pszParams)
		{
			// If parameter separator follows, skip it and any space that follows

			pszParams++;
			EatWhiteSpace(&pszParams);
		}
		else if (L'\r' == *pszParams || L'\n' == *pszParams)
		{
			// If this is the end of line, break out

			break;
		}
		else if (L'\0' == *pszParams)
		{
			// If this is the end of string, break out

			break;
		}
		else if (EatComment(&pszParams))
		{
			// If this is a comment, break out
			
			 break;
		}
	}

	// Call the command

	return (*m_pfnCallback)(rEngine, arParams);
}

/*----------------------------------------------------------*\
| CommandManager implementation
\*----------------------------------------------------------*/

CommandManager::CommandManager(Engine& rEngine): m_rEngine(rEngine)
{
	ZeroMemory(m_szBasePath, sizeof(m_szBasePath));
	ZeroMemory(m_szBaseExt, sizeof(m_szBaseExt));
}

CommandManager::~CommandManager(void)
{
	Empty();
}

Command* CommandManager::Register(LPCWSTR pszName, PCOMMANDCALLBACK pCallback)
{
	Command* pCmdNew = NULL;

	try
	{
		pCmdNew = new Command(pCallback);
	}
	
	catch(std::exception e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Command));
	}

	m_mapCommands[pszName] = pCmdNew;

	return pCmdNew;
}

void CommandManager::Unregister(LPCWSTR pszName)
{
	Command* pCommand = Find(pszName);

	if (pCommand != NULL)
	{
		m_mapCommands.erase(pszName);

		delete pCommand;
	}
}

Command* CommandManager::Find(LPCWSTR pszName)
{
	CommandMapIterator pos = m_mapCommands.find(pszName);

	if (pos == m_mapCommands.end())
		return NULL;

	return m_mapCommands[pszName];
}

void CommandManager::UnregisterAll(void)
{
	for(CommandMapIterator pos = m_mapCommands.begin();
		pos != m_mapCommands.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapCommands.clear();
}

int CommandManager::GetCount(void) const
{
	return int(m_mapCommands.size());
}

CommandMapIterator CommandManager::GetBeginPos(void)
{
	return m_mapCommands.begin();
}

CommandMapIterator CommandManager::GetEndPos(void)
{
	return m_mapCommands.end();
}

CommandMapConstIterator CommandManager::GetBeginPosConst(void) const
{
	return m_mapCommands.begin();
}

CommandMapConstIterator CommandManager::GetEndPosConst(void) const
{
	return m_mapCommands.end();
}

int CommandManager::ExecuteStatement(LPCWSTR pszName, LPCWSTR pszParams)
{
	// First, find a command by this name

	Command* pCmd = Find(pszName);	

	if (pCmd != NULL)
	{
		// Execute command if found

		return pCmd->Execute(m_rEngine, pszParams);
	}
	else
	{
		// If can't find the command, try to find a global (engine) variable

		Variable* pVar = m_rEngine.GetVariables().Find(pszName);

		if (pszParams != NULL)
		{
			// If variable is not found in global scope, try searching current map

			if (NULL == pVar && m_rEngine.GetCurrentMapConst())
			{
				pVar = m_rEngine.GetCurrentMap()->GetVariables().Find(pszName);
			}

			// If variable is not found and parameters are specified,
			// create and set a global one from parameters

			if (NULL == pVar)
			{
				pVar = m_rEngine.GetVariables().Add(pszName,
					Variable::TYPE_UNDEFINED);

				if (NULL == pVar) return -1;
			}

			pVar->FromString(pszParams);
		}
		else
		{
			// If variable not found and no parameters specified, exit with failure

			if (NULL == pVar)
			{
				m_rEngine.Print(L"variable or command not found.", PRINT_ERROR);
				return -1;
			}

			// If variable is found, print its value

			String str = pVar->ToString();

			m_rEngine.Print(str);
		}
	}

	return 0;
}

int CommandManager::ExecuteStatement(LPCWSTR pszLine)
{
	// Echo this action if so specified

	if (m_rEngine.GetOption(Engine::OPTION_ENABLE_ECHO) == TRUE)
	{
		LPCWSTR pszEndLine = wcschr(pszLine, L'\n');
		
		if (NULL == pszEndLine)
			pszEndLine = pszLine + wcslen(pszLine);
		else
			if (L'\r' == *pszEndLine) pszEndLine--;

		if (L'\n' == *pszEndLine) pszEndLine--;

		int nLineLen = int(pszEndLine - pszLine);

		String strEcho;

		try
		{
			strEcho.Allocate(nLineLen);
		}

		catch(Error& rError)
		{
			m_rEngine.GetErrors().Push(rError);

			m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
				__FUNCTIONW__, nLineLen + 1);

			return -1;
		}

		strEcho.CopyToBuffer(nLineLen, pszLine, nLineLen);

		m_rEngine.Print(strEcho, PRINT_ECHO);
	}

	// Find the end of name string

	LPCWSTR pszNameEnd = pszLine;

	while(IsValidNameChar(*pszNameEnd))
		pszNameEnd++;

	// Copy the name string
	
	int nNameLen = int(pszNameEnd - pszLine);

	// If name string length is invalid, exit

	if (0 == nNameLen) return -1;

	String strName;

	try
	{
		strName.Allocate(nNameLen);
	}
	
	catch(Error& rError)
	{
		m_rEngine.GetErrors().Push(rError);

		m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, nNameLen + 1);

		return -1;
	}

	strName.CopyToBuffer(nNameLen, pszLine, nNameLen);

	// Start looking for the first param after the end of name string

	LPCWSTR pszFirstParam = pszNameEnd;

	// If what follows is end of string, execute command without params and exit

	if (L'\0' == *pszFirstParam)
		return ExecuteStatement(strName, NULL);

	// Otherwise, skip any white space

	EatWhiteSpace(&pszFirstParam);

	// If we encounter a comment, execute command without params and exit

	if (EatComment(&pszFirstParam))
		return ExecuteStatement(strName, NULL);

	// Execute with this name and the parameter string

	return ExecuteStatement(strName, pszFirstParam);
}

void CommandManager::ExecuteScriptFile(LPCWSTR pszPath, LPCWSTR pszLabel)
{
	// Open the script file

	WCHAR szFullPath[MAX_PATH] = {0};
	m_rEngine.GetBaseFilePath(pszPath, m_szBasePath, m_szBaseExt, szFullPath);

	Stream stream(&m_rEngine.GetErrors(),
		m_rEngine.GetOption(Engine::OPTION_ENABLE_STREAM_CACHE) ?
		&m_rEngine.GetStreamCache() : NULL);

	try
	{
		stream.Open(szFullPath, GENERIC_READ, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	// Validate its format

	if (stream.IsUnicodeTextFile() == false)
		throw m_rEngine.GetErrors().Push(Error::FILE_NOTUNICODE,
		__FUNCTIONW__, pszPath);

	// Allocate the buffer for file contents excluding Unicode signature

	int nLen = stream.GetSize() / sizeof(WCHAR) - 1;

	String strBuffer;

	try
	{
		strBuffer.Allocate(nLen);
	}

	catch(Error& rError)
	{
		m_rEngine.GetErrors().Push(rError);

		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, nLen + 1);
	}

	// Read file contents

	LPWSTR pszBuffer = strBuffer.GetBuffer();
	
	try
	{
		stream.Read((LPVOID)pszBuffer, nLen * sizeof(WCHAR));
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_READ,
			__FUNCTIONW__, stream.GetPath());
	}

	// Null-terminate the buffer

	pszBuffer[nLen] = L'\0';

	// Get current directory

	WCHAR szCurDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szCurDir);

	// Get script directory

	WCHAR szScriptDir[MAX_PATH] = {0};
	GetAbsolutePath(pszPath, szScriptDir);
	PathRemoveFileSpec(szScriptDir);

	SetCurrentDirectory(szScriptDir);

	// Execute script

	try
	{
		ExecuteScript(pszBuffer, pszLabel);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::INTERNAL,
			__FUNCTIONW__);
	}

	// Restore the current directory

	SetCurrentDirectory(szCurDir);
}

void CommandManager::ExecuteScript(LPCWSTR pszText, LPCWSTR pszLabel)
{
	int nLabelLen = (pszLabel && *pszLabel) ?
		int(wcslen(pszLabel)) : 0;

	while(pszText != NULL)
	{
		// Skip any white space and comments

		while(EatWhiteSpaceNewline(&pszText) ||
			EatComment(&pszText));

		// If we're out of text, break out

		if (L'\0' == *pszText) break;

		// Execute the current statement

		if (nLabelLen != 0)
		{
			// If looking for label and we found it, mark it as found

			if (L':' == *pszText &&
				wcsncmp(pszText + 1, pszLabel, nLabelLen) == 0)
				nLabelLen = 0;
		}
		else
		{
			if (*pszText != L':')
			{
				// Ignore all labels when we are not looking for one

				if (wcsncmp(L"return", pszText, 6) == 0)
				{
					// This is a return statement - return immediately

					break;
				}
				else
				{
					// This is a command call or a variable get/set

					ExecuteStatement(pszText);
				}
			}
		}

		// Go to the next line

		pszText = wcschr(pszText, L'\n');
	}
}

DWORD CommandManager::GetMemoryFootprint(void) const
{
	DWORD dwSize = DWORD(m_mapCommands.size() * sizeof(Command));

	for(CommandMapConstIterator pos = m_mapCommands.begin();
		pos != m_mapCommands.end();
		pos++)
	{
		dwSize += DWORD(pos->first.GetLengthBytes());
	}

	return dwSize;
}

void CommandManager::Empty(void)
{
	UnregisterAll();
}