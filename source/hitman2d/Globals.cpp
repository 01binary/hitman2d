/*------------------------------------------------------------------*\
|
| Globals.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D global functions
| Created: 08/12/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"		// precompiled header
#include "Globals.h"	// defining global function


/*
*
* Function: FormatMemory
* 
* Purpose:  Returns float representing size in bytes passed in shortest
*			possible way (largest possible units).
*			Also returns the units as string in ppszUnits parameter.
*
*/

float Hitman2D::FormatMemory(DWORD dwSizeInBytes, LPCWSTR* ppszUnits)
{
	const LPCWSTR SZ_UNITS[] = { L"bytes", L"KB", L"MB", L"GB" };

	float fSize = float(dwSizeInBytes);
	int nUnit = 0;

	if (dwSizeInBytes >= 1024)
	{
		fSize = float(dwSizeInBytes / 1024);
		nUnit++;
	}	

	while(fSize >= 1024.0f && nUnit < 4)
	{
		fSize /= 1024.0f;
		nUnit++;
	}

	*ppszUnits = (LPCWSTR)SZ_UNITS[nUnit];

	return fSize;
}