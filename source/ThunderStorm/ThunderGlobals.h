/*------------------------------------------------------------------*\
|
| ThunderGlobals.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm global constants, classes, and functions
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_GLOBALS_H
#define THUNDER_GLOBALS_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Macros
\*----------------------------------------------------------*/

#define SAFERELEASE(pComPtr) if (pComPtr != NULL) { pComPtr->Release(); pComPtr = NULL; }
#define SAFERELEASE_D(pComPtr) if (pComPtr != NULL) { LONG lRefs = pComPtr->Release(); _ASSERT(lRefs < 1); m_rEngine.PrintDebug(L"ptr has %d refs after release", lRefs); pComPtr = NULL; }

#define PUSH_CURRENT_DIRECTORY(pszOfPath) \
	WCHAR szCurDir[MAX_PATH] = {0}; \
	WCHAR szPushDir[MAX_PATH] = {0}; \
	GetCurrentDirectory(MAX_PATH, szCurDir); \
	wcscpy_s(szPushDir, MAX_PATH, pszOfPath); \
	if (*PathFindExtension(szPushDir) != L'\0') PathRemoveFileSpec(szPushDir); \
	SetCurrentDirectory(szPushDir);

#define POP_CURRENT_DIRECTORY() \
	SetCurrentDirectory(szCurDir);
	

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const LPVOID INVALID_PTR = (const LPVOID)0xFFFFFFFFFF;
const DWORD INVALID_VALUE = 0xFFFFFFFF;
const DWORD DEFAULT_VALUE = INVALID_VALUE;
const int INVALID_INDEX = -1;

/*----------------------------------------------------------*\
| Functions
\*----------------------------------------------------------*/

//
// Numerics
//

DWORD Pow2(DWORD dwNumber);

//
// Parsing
//

bool IsValidNameChar(WCHAR ch);

int EatWhiteSpace(LPCWSTR* ppsz, bool bBackward = false);

int EatWhiteSpaceNewline(LPCWSTR* ppsz);

int EatComment(LPCWSTR* ppsz);

void GetStringPosition(LPCWSTR pszIn, LPCWSTR psz, int& nLine, int& nColumn);

bool IsWhiteSpace(WCHAR ch);

//
// File System
//

void GetRelativePath(LPCWSTR pszPath, LPWSTR pszOut);
void GetAbsolutePath(LPCWSTR pszPath, LPWSTR pszOut);

} // namespace ThunderStorm

#endif