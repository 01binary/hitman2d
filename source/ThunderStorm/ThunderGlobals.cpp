/*------------------------------------------------------------------*\
|
| ThunderGlobals.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm global classes and functions implementation
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderGlobals.h"		// defining global functions


/*
*
* Function: Pow2
* 
* Purpose:  Returns next highest power of 2 for given number,
*			unless it's already a power of 2.
*
*/

DWORD ThunderStorm::Pow2(DWORD dwNumber)
{
	// Stolen from
	// http://acius2.blogspot.com/2007/11/calculating-next-power-of-2.html

	DWORD dwPow2 = 1;
	
	while( dwPow2 < dwNumber )
		dwPow2 <<= 1;

	return dwPow2;
}

/*
*
* Function: IsValidNameChar
* 
* Purpose:  Names of variables, commands, markup elements, ini sections,
* and ini keys can contain:
*	Uppercase and lowercase letters
*	Digits
*	These special characters: '-', '.', '?', '@', '_', '`'
*	Cannot start with a digit, unless it is a string constant variable type.
*
*/

bool ThunderStorm::IsValidNameChar(WCHAR ch)
{
	return ((ch > 47 && ch < 58)  ||	// Digits
			(ch > 62 && ch < 91)  ||	// Uppercase letters and '?', '@'
			(ch > 94 && ch < 123) ||	// Lowercase letters and '`', '_'
			L'-' == ch  ||
			L'.' == ch);
}

/*
*
* Function: EatWhiteSpace
*
* Purpose: Skips all the white space characters (space, tab)
* until the next non-whitespace character, and updates the string pointer.
*
*/

int ThunderStorm::EatWhiteSpace(LPCWSTR* ppsz, bool bBackward)
{
	int count = 0;
	LPCWSTR psz = *ppsz;

	while(*psz != L'\0')
	{
		if (L' ' == *psz || L'\t' == *psz)
		{
			if (true == bBackward)
				psz--;
			else
				psz++;
		}
		else
		{
			break;
		}

		count++;
	}

	*ppsz = psz;

	return count;
}

/*
*
* Function: EatWhiteSpaceNewline
* 
* Purpose: Skips all the white space characters (space, tab, crlf)
* until the next non-whitespace character, and updates the string pointer.
*
*/

int ThunderStorm::EatWhiteSpaceNewline(LPCWSTR* ppsz)
{
	int count = 0;
	LPCWSTR psz = *ppsz;

	while(*psz != L'\0')
	{
		if (L' ' == *psz ||
		   L'\t' == *psz ||
		   L'\r' == *psz ||
		   L'\n' == *psz)
		{
			psz++;
		}
		else
		{
			break;
		}

		count++;
	}

	*ppsz = psz;

	return count;
}

/*
*
* Function: EatComment
*
* Purpose: If string contains a comment, skips to the next character
* following the comment, and updates string pointer.
* Supported are assembly style, C style and C++ style comments.
*
*/

int ThunderStorm::EatComment(LPCWSTR* ppsz)
{
	LPCWSTR psz = *ppsz;
	LPCWSTR pszEnd = NULL;

	if (L';' == *psz || (L'/' == psz[0] && L'/' == psz[1]))
	{
		pszEnd = wcschr(psz, L'\n');

		if (pszEnd) pszEnd++;
	}
	else if (L'/' == psz[0] && L'*' == psz[1])
	{
		pszEnd = wcsstr(psz, L"*/");

		if (pszEnd) pszEnd += 2;
	}
	else
	{
		return 0;
	}

	if (pszEnd != NULL)
		*ppsz = pszEnd;
	else
		*ppsz = psz + wcslen(psz);

	return int(pszEnd - psz);
}

/*
*
* Function: GetStringPosition
*
* Purpose: Returns line and character index of a given string pointer
* within a given string pointer (through nLine and nColumn).
*
*/

void ThunderStorm::GetStringPosition(LPCWSTR pszIn, LPCWSTR psz, int& nLine, int& nColumn)
{
	nLine = 1;

	LPCWSTR pszLf = wcschr(pszIn, L'\n');

	if (NULL == pszLf)
	{
		nColumn = int(psz - pszIn);
		return;
	}

	LPCWSTR pszPrevLf = pszIn;

	while(pszLf != NULL && pszLf < psz)
	{
		pszPrevLf = pszLf;

		nLine++;

		pszLf = wcschr(pszLf + 1, L'\n');
	}

	nColumn = int(psz - pszPrevLf);
}

/*
*
* Function: IsWhiteSpace
*
* Purpose: checks if given character is whitespace.
*
*/

bool ThunderStorm::IsWhiteSpace(WCHAR ch)
{
	return (L' ' == ch || L'\t' == ch || L'\r' == ch || '\n' == ch);
}

/*
*
* Function: GetRelativePath
*
* Purpose: given absolute path, return path relative
* to current directory.
*
* Assuming that output is at least MAX_PATH characters long.
*
*/

void ThunderStorm::GetRelativePath(LPCWSTR pszPath, LPWSTR pszOut)
{
	WCHAR szCurDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szCurDir);

	PathRelativePathTo(pszOut,
					   szCurDir,
					   FILE_ATTRIBUTE_DIRECTORY,
					   pszPath,
					   FILE_ATTRIBUTE_NORMAL);
}

/*
*
* Function: GetAbsolutePath
*
* Purpose: given relative path, return absolute path,
* assuming relative path is in current directory.
*
* Assuming that output is at least MAX_PATH characters long.
*
*/

void ThunderStorm::GetAbsolutePath(LPCWSTR pszPath, LPWSTR pszOut)
{	
	wcscpy_s(pszOut, MAX_PATH, pszPath);

	if (PathIsRelative(pszPath) == TRUE)
	{
		WCHAR szCurDir[MAX_PATH] = {0};
		GetCurrentDirectory(MAX_PATH, szCurDir);

		LPCWSTR pszDirs[2] = { szCurDir, NULL };

		PathResolve(pszOut, pszDirs, PRF_FIRSTDIRDEF);
	}
}