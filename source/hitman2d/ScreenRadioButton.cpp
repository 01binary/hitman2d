/*------------------------------------------------------------------*\
|
| ScreenRadioButton.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D RadioButton Control implementation
| Created: 03/31/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "Error.h"				// using error constants
#include "ScreenRadioButton.h"	// defining ScreenRadioButton

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenRadioButton::SZ_CLASS[] = L"radiobutton";


/*----------------------------------------------------------*\
| ScreenRadioButton implementation
\*----------------------------------------------------------*/

ScreenRadioButton::ScreenRadioButton(Engine& rEngine,
									 LPCWSTR pszClass,
									 Screen* pParent):

									 ScreenCheckBox(rEngine, pszClass, pParent)
{
	m_dwFlags |= RADIO;
}

ScreenRadioButton::~ScreenRadioButton(void)
{
}

Object* ScreenRadioButton::CreateInstance(Engine& rEngine,
										  LPCWSTR pszClass,
										  Object* pParent)
{
	return new ScreenRadioButton(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}