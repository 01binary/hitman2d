/*------------------------------------------------------------------*\
|
| stdafx.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm precompiled header
| Created: 11/01/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STD_AFX_H
#define THUNDER_STD_AFX_H

/*----------------------------------------------------------*\
| Links
\*----------------------------------------------------------*/

#pragma comment (lib, "kernel32.lib")		// Windows Base
#pragma comment (lib, "user32.lib")			// Windows GUI
#pragma comment (lib, "shlwapi.lib")		// Lightweight Shell
#pragma comment (lib, "version.lib")		// Version Information
#pragma comment (lib, "wbemuuid.lib")		// Windows Management Instrumentation
#pragma comment (lib, "dxguid.lib")			// DirectX Definitions
#pragma comment (lib, "d3d9.lib")			// Direct3D9

#ifdef _DEBUG
	#pragma comment (lib, "d3dx9.lib")		// D3DX9 debug
#else

	#pragma comment (lib, "d3dx9.lib")		// D3DX9
#endif

#pragma comment (lib, "dxerr.lib")			// DirectX 9 Errors
#pragma comment (lib, "dsound.lib")			// DirectSound8
#pragma comment (lib, "winmm.lib")			// Windows Multimedia
#pragma comment (lib, "quartz.lib")			// DirectShow
#pragma comment (lib, "strmiids.lib")		// DirectShow IIDs

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

// Disable warnings

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma warning(disable : 4201)
#pragma warning(disable : 4995)

//
// Windows headers
//

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501					// Compiling for Win2k/XP and higher
#endif

#define WIN32_LEAN_AND_MEAN					// Exclude extra windows headers
#define OEMRESOURCE							// Include OEM resource constants

#include <windows.h>						// Used for windowing, file I/O and timing
#include <shlwapi.h>						// Used for path functions
#include <shlobj.h>							// Used for path functions also

//
// DirectX headers
//

#ifdef _DEBUG
#define D3D_DEBUG_INFO						// Enable extended debugging support for DirectX
#endif

#include <d3dx9.h>							// D3DX9 helper library, also includes d3d9
#include <dxerr.h>							// Used for DXGetErrorDescription

#include <mmsystem.h>						// Used for WAVEFORMATEX (required by dsound.h) and mixer functions

#include <dsound.h>							// DirectSound8
#include <dshow.h>							// DirectShow

//
// Standard C/C++ Library headers
//

#include <basetsd.h>						// Used for DWORD_PTR
#include <cstdio>							// Used for sprintf and other formatting
#include <crtdbg.h>							// Used for memory leak detection

//
// Standard Template Library headers
//

#include <yvals.h>							// Used for __FUNCTIONW__
#include <list>								// list<>
#include <vector>							// vector<>
#include <map>								// map<,>
#include <stack>							// stack<>
#include <set>								// set<>
#include <algorithm>						// find, unique, sort, etc.

// Re-enable warnings

#pragma warning(default : 4995)
#pragma warning(default : 4201)

#endif