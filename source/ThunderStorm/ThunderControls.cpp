/*------------------------------------------------------------------*\
|
| ThunderControls.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine control binding class implementation
| Created: 11/19/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderIniFile.h"		// using IniFile
#include "ThunderGlobals.h"		// using INVALID_INDEX
#include "ThunderControls.h"	// defining ControlManager

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Key names

const LPCWSTR SZ_VKMAP[] =				{
											L"",
											L"left mouse button",
											L"right mouse button",
											L"cancel",
											L"middle mouse button",
											L"x mouse button 1",
											L"x mouse button 2",
											L"",
											L"backspace",
											L"tab",
											L"",
											L"",
											L"clear",
											L"enter",
											L"num enter",
											L"",
											L"shift",
											L"ctrl",
											L"alt",
											L"pause",
											L"caps lock",
											L"kana",
											L"",
											L"junja",
											L"final",
											L"hanja",
											L"",
											L"esc",
											L"convert",
											L"nonconvert",
											L"accept",
											L"mode change",
											L"space",
											L"pg up",
											L"pg down",
											L"end",
											L"home",
											L"left",
											L"up",
											L"right",
											L"down",
											L"select",
											L"print",
											L"execute",
											L"prt scn",
											L"insert",
											L"delete",
											L"help",
											L"0",
											L"1",
											L"2",
											L"3",
											L"4",
											L"5",
											L"6",
											L"7",
											L"8",
											L"9", // 0x39
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"a", // 0x41
											L"b",
											L"c",
											L"d",
											L"e",
											L"f",
											L"g",
											L"h",
											L"i",
											L"j",
											L"k",
											L"l",
											L"m",
											L"n",
											L"o",
											L"p",
											L"q",
											L"r",
											L"s",
											L"t",
											L"u",
											L"v",
											L"w",
											L"x",
											L"y",
											L"z",
											L"lwin",
											L"rwin",
											L"apps",
											L"",
											L"sleep",
											L"num 0",
											L"num 1",
											L"num 2",
											L"num 3",
											L"num 4",
											L"num 5",
											L"num 6",
											L"num 7",
											L"num 8",
											L"num 9",
											L"num *",
											L"num +",
											L"separator",
											L"num -",
											L"num .",
											L"num /",
											L"f1",
											L"f2",
											L"f3",
											L"f4",
											L"f5",
											L"f6",
											L"f7",
											L"f8",
											L"f9",
											L"f10",
											L"f11",
											L"f12",
											L"f13",
											L"f14",
											L"f15",
											L"f16",
											L"f17",
											L"f18",
											L"f19",
											L"f20",
											L"f21",
											L"f22",
											L"f23",
											L"f24",
											L"", // 0x88
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"", // 0x8F
											L"num lock",
											L"scroll lock",
											L"num =",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"", // 0x97
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"", // 0x9F
											L"lshift",
											L"rshift",
											L"lctrl",
											L"rctrl",
											L"lalt",
											L"ralt",
											L"browser back",
											L"browser forward",
											L"browser refresh",
											L"browser stop",
											L"browser search",
											L"browser favorites",
											L"browser home",
											L"volume mute",
											L"volume down",
											L"volume up",
											L"media next track",
											L"media previous track",
											L"media stop",
											L"media play/pause",
											L"launch mail",
											L"launch media select",
											L"launch app 1",
											L"launch app 2",
											L"",
											L"",
											L";",
											L"+",
											L",",
											L"-",
											L".",
											L"/",
											L"~",
											L"", // 0xC1
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"",
											L"", // 0xD7
											L"", // 0xD8
											L"",
											L"", // 0xDA
											L"[",
											L"\\",
											L"]",
											L"'"
										};

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, int> KeyNameMap;
typedef std::map<String, int>::iterator KeyNameMapIterator;
typedef std::map<String, int>::const_iterator KeyNameMapConstIterator;


/*----------------------------------------------------------*\
| ControlManager implementation
\*----------------------------------------------------------*/

void ControlManager::Serialize(LPCWSTR pszPath)
{
	IniFile controlSettings;
	WCHAR szSection[32] = {0};

	for(int n = 0; n < int(m_arNames.size()); n++)
	{
		int nBound = GetControlBoundKey(n);

		if (nBound != 0)
		{
			LPCWSTR pszName = m_arNames[n];

			LPCWSTR psz = wcschr(pszName, L'.');

			wcsncpy_s(szSection, 32, pszName, psz - pszName);

			controlSettings.SetStringKeyValue(szSection,
				psz + 1, SZ_VKMAP[nBound]);
		}
	}

	controlSettings.Serialize(pszPath);
}

void ControlManager::Deserialize(LPCWSTR pszPath)
{
	// If none registered, exit

	if (m_arNames.empty() == true)
		return;

	// Clear any previous bindings

	Empty();

	// Load profile specified

	IniFile controlSettings;
	controlSettings.Deserialize(pszPath);

	// Load key names

	KeyNameMap mapKeyNames;

	for(int n = 0; n < (sizeof(SZ_VKMAP) / sizeof(LPCWSTR)); n++)
		mapKeyNames[SZ_VKMAP[n]] = n;

	// Load controls

	WCHAR szSection[32] = {0};

	for(int n = 0; n < int(m_arNames.size()); n++)
	{
		LPCWSTR pszName = m_arNames[n];

		LPCWSTR psz = wcschr(pszName, L'.');

		wcsncpy_s(szSection, 32, pszName, psz - pszName);

		KeyNameMapConstIterator posKey =
			mapKeyNames.find(controlSettings.GetStringKeyValue(szSection,
				psz + 1, SZ_VKMAP[0]));

		if (posKey != mapKeyNames.end())
			Bind(n, posKey->second);
	}
}

LPCWSTR ControlManager::GetKeyDescription(int nKey)
{
	if (nKey < 0 || nKey >= (0xDA + 4))
		return SZ_VKMAP[0];

	return SZ_VKMAP[nKey];
}

int ControlManager::GetKeyCode(LPCWSTR pszDescription)
{
	for(int n = 0;
		n < (sizeof(SZ_VKMAP) / sizeof(LPCWSTR));
		n++)
	{
		size_t nLen = wcslen(pszDescription);

		if (wcslen(SZ_VKMAP[n]) == nLen)
		{
			if (wcsncmp(pszDescription, SZ_VKMAP[n], nLen) == 0)
				return n;
		}
	}

	return INVALID_INDEX;
}

void ControlManager::Empty(void)
{
	m_mapBinds.clear();

	for(IntArrayIterator pos = m_arRevBinds.begin();
		pos != m_arRevBinds.end();
		pos++)
	{
		*pos = KEY_UNASSIGNED;
	}
}