/*------------------------------------------------------------------*\
|
| ScreenExit.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Exit Screen implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "Globals.h"		// using global constants
#include "Game.h"			// using Game
#include "Error.h"			// using error constants
#include "ScreenExit.h"		// defining ScreenExit

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenExit::SZ_CLASS[] = L"overlapped::exit";


/*----------------------------------------------------------*\
| ScreenExit implementation
\*----------------------------------------------------------*/

ScreenExit::ScreenExit(Engine& rEngine,
					   LPCWSTR pszClass,
					   Screen* pParent):

					   ScreenOverlapped(rEngine, pszClass, pParent)
{
}

Object* ScreenExit::CreateInstance(Engine& rEngine,
								   LPCWSTR pszClass,
								   Object* pParent)
{
	return new ScreenExit(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenExit::OnCommand(int nCommandID,
						   Screen* pSender,
						   int nParam)
{
	switch(nCommandID)
	{
	case ID_OK:
		{
			if (m_rEngine.GetClientInstance())
				m_rEngine.GetClientInstance()->Exit();
		}
		break;
	case ID_CANCEL:
		{
			Close();
		}
		break;
	}
}