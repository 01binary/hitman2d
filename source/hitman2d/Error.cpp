/*------------------------------------------------------------------*\
|
| Error.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D error class
| Created: 08/05/2008
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"
#include "Error.h"

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const LPCWSTR SZ_GAMEERRORS[] =		{
										L"Invalid engine version (0x%x), required 0x%x.",
										L"Configuration dialog aborted.",
										L"%s"
									};


/*----------------------------------------------------------*\
| ErrorGame implementation
\*----------------------------------------------------------*/

ErrorGame::ErrorGame(int nCode, LPCWSTR pszFunctionName, ...)
{
	m_nCode = nCode;

	va_list pArgList;

	va_start(pArgList, pszFunctionName);

	BuildDescription(SZ_GAMEERRORS[m_nCode - Error::USER],
		pszFunctionName, pArgList);

	va_end(pArgList);
}