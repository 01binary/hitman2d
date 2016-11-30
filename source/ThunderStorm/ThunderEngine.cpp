/*------------------------------------------------------------------*\
|
| ThunderEngine.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine class(es) implementation
| Created: 04/08/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoFile/Elem
#include "ThunderClient.h"		// using Client (includes ThunderEngine.h)
#include "ThunderFont.h"		// using TextureFont
#include "ThunderSprite.h"		// using Sprite
#include "ThunderSound.h"		// using SoundInstance
#include "ThunderVideo.h"		// using Video (includes ThunderMusic.h)
#include "ThunderTileMap.h"		// using TileMap
#include "ThunderScreen.h"		// using Screen
#include "ThunderRegion.h"		// using Region/Set
#include "ThunderStringTable.h"	// using StringTable

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const BYTE Engine::VERSION[4]						= {2, 1, 0, 0};
const BYTE Engine::SAV_SIGNATURE[4]					= "THV";
const BYTE Engine::SAV_FORMAT_VERSION[4]			= {2, 1, 0, 0};
const int Engine::DEFAULT_TILE_SIZE					= 32;
const float Engine:: TIME_EPSILON					= 0.001f;
const float Engine::DEFAULT_STREAMCACHEDURATION		= 60.0f * 2.0f;
const int Engine::DEFAULT_STREAMCACHEFREQUENCY		= 1;
const int Engine::DEFAULT_RESOURCECACHEFREQUENCY	= 1;
const int Engine::DEFAULT_SOUNDCACHEFREQUENCY		= 1;
const WCHAR Engine::SZ_WNDPROP[]					= L"thu";
const WCHAR Engine::SZ_GAMEWNDCLASS[]				= L"ThunderGameWndClass";


/*----------------------------------------------------------*\
| Engine implementation
\*----------------------------------------------------------*/

Engine::Engine(void) :	m_hGameWindow(NULL),
						m_nOldGameWindowProc(0),

						m_bSessionStarted(false),
						m_bSessionPaused(false),

						m_qwCurTick(0),
						m_qwLastTick(0),
						
						m_fFreq(0.0f),
						m_fFrameTime(0.0f),
						m_fTime(0.0f),
						m_fRunTime(0.0f),
						m_fTimeMultiplier(1.0f),	

						m_fResourceCacheAccum(0.0f),
						m_fStreamCacheAccum(0.0f),
						m_fSoundCacheAccum(0.0f),

						#pragma warning(disable : 4355)

						m_Graphics(*this),

						m_Textures(*this),
						m_CubeTextures(*this),
						m_Regions(*this),
						m_Animations(*this),
						m_Materials(*this),
						m_Effects(*this),
						m_Sprites(*this),
						m_Sounds(*this),
						m_Music(*this),
						m_Videos(*this),
						m_Fonts(*this),
						m_Strings(*this),

						m_Maps(*this),

						m_Screens(*this),

						m_Audio(*this),

						m_Commands(*this),

						m_Classes(*this),

						m_StreamCache(*this, DEFAULT_STREAMCACHEDURATION),

						#pragma warning(default : 4355)

						m_pVideo(NULL),

						m_pMap(NULL),

						m_clrBackColor(D3DCOLOR_XRGB(0, 0, 0)),

						m_pClientInstance(NULL),

						m_Variables(&m_ErrorStack)
										
{
	ZeroMemory(m_szBaseSavePath, sizeof(m_szBaseSavePath));
	ZeroMemory(m_szBaseSaveExt, sizeof(m_szBaseSaveExt));

	ResetOptions();
}

void Engine::Empty(void)
{
	// Disable resource cache to prevent entries from being
	// created by other entries that are being destroyed

	m_nOptions[OPTION_ENABLE_RESOURCE_CACHE] = FALSE;

	// End the current session if any

	if (true == m_bSessionStarted)
		EndSession();

	// Clear timers

	m_Timers.Empty();

	// Clear screen system

	m_Screens.Empty();

	// Clear custom cursor if any

	m_CustomCursor.Empty();

	// Clear playing video if any

	if (m_pVideo != NULL)
		m_pVideo->Stop();

	// Unload sounds

	m_Audio.GetSoundInstances().Empty();
	m_Sounds.Empty();

	// Evict cached streams

	m_StreamCache.Empty();

	// Evict cached resources
	
	m_ResourceCache.Empty();

	// Clear fonts

	m_Fonts.Empty();

	// Clear shared material instances

	m_Graphics.SetWireframeMaterial(NULL);
	m_Graphics.GetMaterialInstancePool().Empty();

	// Unload resources
	
	m_Music.Empty();
	m_Videos.Empty();

	m_Sprites.Empty();
	m_Animations.Empty();
	m_Strings.Empty();
	m_Materials.Empty();
	m_Effects.Empty();	
	m_Textures.Empty();
	m_CubeTextures.Empty();

	// Clean up graphics system

	m_Graphics.Empty();	

	// Clean up sound system

	m_Audio.Empty();

	// Unload COM

	if (TRUE == m_nOptions[OPTION_MANAGE_COM])
		CoUninitialize();

	// Unsubclass game window if was subclassed

	if (m_nOldGameWindowProc != 0)
		UnsubclassGameWindow();

	// Destroy game window

	if (TRUE == m_nOptions[OPTION_MANAGE_GAME_WINDOW])
	{
		RemoveProp(m_hGameWindow, SZ_WNDPROP);
		DestroyWindow(m_hGameWindow);
	}
}

DWORD Engine::GetVersion(void)
{
	return *((const DWORD*)VERSION);
}

void Engine::SetGameWindow(HWND hWnd, bool bSubclass)
{
	// Un-subclass current render window if subclassed

	if (IsGameWindowSubclassed() == true)
		UnsubclassGameWindow();

	// Set a new one

	m_hGameWindow = hWnd;

	// Subclass a new one if specified

	if (true == bSubclass)
		SubclassGameWindow();
}

void Engine::SubclassGameWindow(void)
{
	if (NULL == m_hGameWindow ||
		GetProp(m_hGameWindow, SZ_WNDPROP) != NULL) 
		throw GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	SetProp(m_hGameWindow, SZ_WNDPROP, (HANDLE)this);

	m_nOldGameWindowProc = (WNDPROC)SetWindowLongPtr(m_hGameWindow,
		GWL_WNDPROC, LONG_PTR(EngineWndProc));
}

void Engine::UnsubclassGameWindow(void)
{
	if (NULL == m_hGameWindow ||
		GetProp(m_hGameWindow, SZ_WNDPROP) != NULL)
		return;

	RemoveProp(m_hGameWindow, SZ_WNDPROP);

	// Compiling for 32-bit platform only

	SetWindowLongPtr(m_hGameWindow, GWL_WNDPROC, LONG(m_nOldGameWindowProc));

	m_nOldGameWindowProc = 0;
}

void Engine::Initialize(HWND hGameWindow,
						bool bSubclassWindow,
						bool bFullScreen,
						int nResolutionWidth,
						int nResolutionHeight,
						Graphics::DeviceFormats nFormat,
						D3DMULTISAMPLE_TYPE nMultiSampleType,
						DWORD dwMultiSampleQuality,
						DWORD dwBufferRefresh,
						bool bVSync,
						bool bHardwareAcceleration,
						bool bSoftwareVertexProcessing,
						bool bPureDevice,
						bool bStartSession)
{
	//
	// Initialize performance counter
	//

	INT64 qwFreq = 0;

	if (QueryPerformanceFrequency((LARGE_INTEGER*)&qwFreq) == FALSE)
	{
		throw m_ErrorStack.Push(Error::WIN_SYS_QUERYPERFORMANCEFREQUENCY, 
			__FUNCTIONW__, GetLastError());
	}
	else
	{
		m_fFreq = (float)qwFreq;

		if (QueryPerformanceCounter((LARGE_INTEGER*)&m_qwLastTick) == FALSE)
		{
			throw m_ErrorStack.Push(Error::WIN_SYS_QUERYPERFORMANCECOUNTER,
				__FUNCTIONW__, GetLastError());
		}
	}

	//
	// Initialize COM if specified (required to use DirectX)
	//

	HRESULT hr = S_OK;
	
	if (TRUE == m_nOptions[OPTION_MANAGE_COM])
	{
		hr = CoInitialize(NULL);

		if (hr != S_OK && hr != S_FALSE)
		{
			// Failed to initialize COM

			throw GetErrors().Push(Error::WIN_SYS_COINITIALIZE,
				__FUNCTIONW__, hr);
		}
	}	

	// Set and subclass the game window if not done already

	if (m_hGameWindow != hGameWindow)
	{
		m_hGameWindow = hGameWindow;

		if (true == bSubclassWindow)
		{
			// If window already has engine window procedure, do not subclass

			if (GetWindowLong(hGameWindow, GWL_WNDPROC) != LONG_PTR(EngineWndProc))
				SubclassGameWindow();
		}
	}
	
	//
	// Initialize graphics system
	//

	m_Graphics.Initialize(hGameWindow,
		bFullScreen, nResolutionWidth, nResolutionHeight, nFormat,
		nMultiSampleType, dwMultiSampleQuality, dwBufferRefresh, bVSync,
		bHardwareAcceleration, bSoftwareVertexProcessing, bPureDevice);

	//
	// Initialize sound system - ok if doesn't work
	//

	try
	{
		if (FALSE == m_nOptions[OPTION_DISABLE_SOUNDS])
			m_Audio.Initialize();
	}

	catch(Error& rError)
	{
		PrintError(L"Failed to initialize audio system.");
		PrintError(rError.GetDescription());

		m_nOptions[OPTION_DISABLE_SOUNDS] = TRUE;
	}

	//
	// Start session if specified
	//

	if (true == bStartSession)
		StartSession();
}

void Engine::Initialize(LPCWSTR pszWindowTitle,
						DWORD dwWindowIconResourceID,
						bool bFullScreen,
						int nResolutionWidth,
						int nResolutionHeight,
						Graphics::DeviceFormats nFormat,
						D3DMULTISAMPLE_TYPE nMultiSampleType,
						DWORD dwMultiSampleQuality,
						DWORD dwBufferRefresh,
						bool bVSync,
						bool bHardwareAcceleration,
						bool bSoftwareVertexProcessing,
						bool bPureDevice,
						bool bStartSession)
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	// Register game window class

	WNDCLASSEX wcx = {0};

	wcx.cbSize = sizeof(WNDCLASSEX);	
	wcx.hCursor = LoadCursor(NULL, (LPCWSTR)IDC_ARROW);
	wcx.lpfnWndProc = (WNDPROC)EngineWndProc;
	wcx.lpszClassName = SZ_GAMEWNDCLASS;
	wcx.hInstance = hInstance;

	if (DEFAULT_VALUE == dwWindowIconResourceID)
	{
		// If icon not specified, use default windows icon

		wcx.hIcon = LoadIcon(NULL, (LPCWSTR)IDI_APPLICATION);
	}
	else
	{
		// If icon specified, load from current executable file's resources

		wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(dwWindowIconResourceID));

		wcx.hIconSm = (HICON)LoadImage(hInstance,
			MAKEINTRESOURCE(dwWindowIconResourceID), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON),
			GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	}

	if (RegisterClassEx(&wcx) == NULL)
		throw GetErrors().Push(Error::WIN_GUI_REGISTERCLASSEX,
		__FUNCTIONW__, GetLastError());

	// Create game window
	
	if (true == bFullScreen)
	{
		m_hGameWindow = CreateWindowEx(0, SZ_GAMEWNDCLASS, pszWindowTitle,
			0, 0, 0, nResolutionWidth, nResolutionHeight,
			(HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);

		if (NULL == m_hGameWindow)
			throw GetErrors().Push(Error::WIN_GUI_CREATEWINDOWEX,
				__FUNCTIONW__, GetLastError());

		// Resize the game window to full screen and remove title bar

		SetWindowLong(m_hGameWindow, GWL_STYLE, WS_POPUP);

		// Redraw window frame

		SetWindowPos(m_hGameWindow, NULL, 0, 0,
			nResolutionWidth, nResolutionHeight,
			SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME);
	}
	else
	{
		int nWidth = nResolutionWidth +
			GetSystemMetrics(SM_CXFIXEDFRAME) * 2;

		int nHeight = nResolutionHeight +
			GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
			GetSystemMetrics(SM_CYCAPTION);

		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		m_hGameWindow = CreateWindowEx(0, SZ_GAMEWNDCLASS, pszWindowTitle,
			WS_CAPTION | WS_SYSMENU,
			(nScreenWidth - nWidth) / 2, (nScreenHeight - nHeight) / 2,
			nWidth, nHeight, (HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);

		if (NULL == m_hGameWindow)
			throw GetErrors().Push(Error::WIN_GUI_CREATEWINDOWEX,
				__FUNCTIONW__, GetLastError());
	}

	// Make sure it has an engine pointer in a prop

	SetProp(m_hGameWindow, SZ_WNDPROP, (HANDLE)this);

	// We are now managing it

	m_nOptions[OPTION_MANAGE_GAME_WINDOW] = TRUE;

	// Initialize engine with this window

	Initialize(m_hGameWindow, false, bFullScreen,
		nResolutionWidth, nResolutionHeight, nFormat,
		nMultiSampleType, dwMultiSampleQuality, dwBufferRefresh, bVSync,
		bHardwareAcceleration, bSoftwareVertexProcessing,
		bPureDevice, bStartSession);
}

LRESULT CALLBACK Engine::EngineWndProc(HWND hWnd,
									   UINT uMsg,
									   WPARAM wParam,
									   LPARAM lParam)
{
	Engine* pEngine =
		reinterpret_cast<Engine*>(GetProp(hWnd, SZ_WNDPROP));

	switch(uMsg)
	{
	case WM_MOUSEMOVE:
		{
			// Update custom cursor position

			pEngine->m_vecCursorPos.x = float(LOWORD(lParam));
			pEngine->m_vecCursorPos.y = float(HIWORD(lParam));
			
			// Get cursor position

			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			// Update hover screen
			
			if (pEngine->m_Screens.m_pCaptureScreen != NULL)
			{
				RECT rcAbs;
				pEngine->m_Screens.m_pCaptureScreen->GetAbsRect(rcAbs);

				if (PtInRect(&rcAbs, pt) == TRUE)
				{
					if (pEngine->m_Screens.m_pHoverScreen !=
						pEngine->m_Screens.m_pCaptureScreen)
						pEngine->m_Screens.SetHoverScreen(
							pEngine->m_Screens.m_pCaptureScreen);
				}
				else
				{
					pEngine->m_Screens.SetHoverScreen(NULL);
				}
			}
			else
			{
				pEngine->m_Screens.SetHoverScreen(
					pEngine->m_Screens.ScreenFromPoint(pt, true, true));
			}

			Screen* pMouseTarget = pEngine->m_Screens.m_pCaptureScreen ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.m_pHoverScreen;
			
			if (TRUE == pEngine->m_nOptions[OPTION_SCREEN_EVENTS] &&
			   pMouseTarget != NULL)
			{
				// Notify screen

				POINT ptOffset;
				pMouseTarget->GetAbsPos(ptOffset);

				pt.x -= ptOffset.x;
				pt.y -= ptOffset.y;

				pMouseTarget->OnMouseMove(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseMove(pt);
			}
		}
		break;
	case WM_SETCURSOR:
		{
			BOOL bShow = pEngine->m_nOptions[OPTION_CUSTOM_CURSOR] ?
						 FALSE : pEngine->m_nOptions[OPTION_SHOW_CURSOR];

			CURSORINFO ci = {0};

			ci.cbSize = sizeof(CURSORINFO);
			GetCursorInfo(&ci);

			if (bShow != (ci.flags == CURSOR_SHOWING))
				ShowCursor(bShow);
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			// Support extended key information

			switch(wParam)
			{
			case VK_RETURN:
				if (lParam & 0x1000000)
					wParam = ControlManager::VK_NUMRETURN;
				break;
			case VK_SHIFT:
				wParam = ControlManager::IsKeyPressed(VK_LSHIFT) == true ?
					VK_LSHIFT : VK_RSHIFT;
				break;
			case VK_CONTROL:
				wParam = ControlManager::IsKeyPressed(VK_LCONTROL) == true ?
					VK_LCONTROL : VK_RCONTROL;
				break;
			case VK_MENU:
				wParam = ControlManager::IsKeyPressed(VK_LMENU) == true ?
					VK_LMENU : VK_RMENU;
				break;		
			}

			if (TRUE == pEngine->m_nOptions[OPTION_GAME_EVENTS] &&
			   pEngine->m_pClientInstance != NULL)
			{
				// Notify game

				pEngine->m_pClientInstance->OnKeyDown(int(wParam));
			}

			if (TRUE == pEngine->m_nOptions[OPTION_SCREEN_EVENTS] &&
			   pEngine->m_Screens.m_pFocusScreen != NULL &&
			   pEngine->m_Screens.m_pFocusScreen->IsFlagSet(Screen::DISABLED) == false)
			{
				// Notify screen

				pEngine->m_Screens.m_pFocusScreen->OnKeyDown(int(wParam));
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnKeyDown(int(wParam));
			}

			// Do not pass to default window proc if not subclassed

			if (0 == pEngine->m_nOldGameWindowProc)
				return 0;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			// Support extended key information

			switch(wParam)
			{
			case VK_SHIFT:
				wParam = ControlManager::IsKeyPressed(VK_LSHIFT) == true ?
					VK_LSHIFT : VK_RSHIFT;
				break;
			case VK_CONTROL:
				wParam = ControlManager::IsKeyPressed(VK_LCONTROL) == true ?
					VK_LCONTROL : VK_RCONTROL;
				break;
			case VK_MENU:
				wParam = ControlManager::IsKeyPressed(VK_LMENU) == true ?
					VK_LMENU : VK_RMENU;
				break;		
			}

			if (TRUE == pEngine->m_nOptions[OPTION_GAME_EVENTS] &&
			   pEngine->m_pClientInstance != NULL)
			{
				// Notify game

				pEngine->m_pClientInstance->OnKeyUp(int(wParam));
			}

			if (TRUE == pEngine->m_nOptions[OPTION_SCREEN_EVENTS] &&
			   pEngine->m_Screens.m_pFocusScreen != NULL &&
			   pEngine->m_Screens.m_pFocusScreen->IsFlagSet(
			   Screen::DISABLED) == false)
			{
				// Notify screen

				pEngine->m_Screens.m_pFocusScreen->OnKeyUp(int(wParam));
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
					pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnKeyUp(int(wParam));
			}

			// Do not pass to default window proc if not subclassed

			if (0 == pEngine->m_nOldGameWindowProc) return 0;
		}
		break;
	case WM_CHAR:
		{
			if (TRUE == pEngine->m_nOptions[OPTION_GAME_EVENTS] &&
			   pEngine->m_pClientInstance != NULL)
			{
				// Notify game

				pEngine->m_pClientInstance->OnKeyPress((int)wParam,
					(lParam & KF_EXTENDED) > 0,
					(lParam & KF_ALTDOWN) > 0);
			}

			if (TRUE == pEngine->m_nOptions[OPTION_SCREEN_EVENTS] &&
			   pEngine->m_Screens.m_pFocusScreen != NULL &&
			   pEngine->m_Screens.m_pFocusScreen->IsFlagSet(Screen::DISABLED) == false)
			{
				// Notify screen

				pEngine->m_Screens.m_pFocusScreen->OnKeyPress((int)wParam,
					(lParam & KF_EXTENDED) > 0,
					(lParam & KF_ALTDOWN) > 0);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnKeyPress(int(wParam));
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			if (pEngine->m_Screens.m_pFocusScreen != NULL &&
				pEngine->m_Screens.m_pFocusScreen->IsFlagSet(
					Screen::DISABLED) == false)
			{
				// Notify screen

				pEngine->m_Screens.m_pFocusScreen->OnMouseWheel(
					int(GET_WHEEL_DELTA_WPARAM(wParam)));
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseWheel(
					int(GET_WHEEL_DELTA_WPARAM(wParam)));
			}
			
			// Update hover screen

			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(pEngine->m_hGameWindow, &pt);
			
			if (pEngine->m_Screens.m_pCaptureScreen != NULL)
			{
				RECT rcAbs;
				pEngine->m_Screens.m_pCaptureScreen->GetAbsRect(rcAbs);

				if (PtInRect(&rcAbs, pt) == TRUE)
				{
					if (pEngine->m_Screens.m_pHoverScreen !=
						pEngine->m_Screens.m_pCaptureScreen)
						pEngine->m_Screens.SetHoverScreen(
							pEngine->m_Screens.m_pCaptureScreen);
				}
				else
				{
					pEngine->m_Screens.SetHoverScreen(NULL);
				}
			}
			else
			{
				pEngine->m_Screens.SetHoverScreen(
					pEngine->m_Screens.ScreenFromPoint(pt, true, true));
			}

			Screen* pMouseTarget = pEngine->m_Screens.m_pCaptureScreen ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.m_pHoverScreen;
			
			if (TRUE == pEngine->m_nOptions[OPTION_SCREEN_EVENTS] &&
			   pMouseTarget != NULL)
			{
				// Notify screen

				POINT ptOffset;
				pMouseTarget->GetAbsPos(ptOffset);

				pt.x -= ptOffset.x;
				pt.y -= ptOffset.y;

				pMouseTarget->OnMouseMove(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseMove(pt);
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseLDown(pt);
			}
			else
			{
				pEngine->m_Screens.SetFocusScreen(NULL);

				if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
				{
					// Notify map

					pEngine->m_pMap->OnMouseLDown(pt);
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseLUp(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseLUp(pt);
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseLDbl(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseLDbl(pt);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseRDown(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseRDown(pt);
			}
		}
		break;
	case WM_RBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseRUp(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseRUp(pt);
			}
		}
		break;
	case WM_RBUTTONDBLCLK:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseRDbl(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseRDbl(pt);
			}
		}
		break;
	case WM_MBUTTONDOWN:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseMDown(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseMDown(pt);
			}
		}
		break;
	case WM_MBUTTONUP:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseMUp(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				    pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseMUp(pt);
			}
		}
		break;
	case WM_MBUTTONDBLCLK:
		{
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };

			pEngine->m_Screens.SetHoverScreen(
				pEngine->m_Screens.m_pCaptureScreen != NULL ?
				pEngine->m_Screens.m_pCaptureScreen :
				pEngine->m_Screens.ScreenFromPoint(pt, true, true));

			if (pEngine->m_Screens.m_pHoverScreen != NULL)
			{
				// Notify screen

				RECT rcAbs;
				pEngine->m_Screens.m_pHoverScreen->GetAbsRect(rcAbs);

				pt.x -= rcAbs.left;
				pt.y -= rcAbs.top;

				pEngine->m_Screens.m_pHoverScreen->OnMouseMDbl(pt);
			}
			else if (TRUE == pEngine->m_nOptions[OPTION_MAP_EVENTS] &&
				pEngine->m_pMap != NULL)
			{
				// Notify map

				pEngine->m_pMap->OnMouseMDbl(pt);
			}
		}
		break;
	case WM_ERASEBKGND:
		{
			return 1;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);

			RECT rcClient;
			GetClientRect(hWnd, &rcClient);

			// Create temporary brush of engine background color

			HBRUSH hBrush = CreateSolidBrush(pEngine->GetBackColorConst());

			if (pEngine->m_pVideo != NULL)
			{
				// Paint background color over parts not obscured by the video

				HRGN hRgnClient = CreateRectRgnIndirect(&rcClient);

				HRGN hRgnVideo = CreateRectRgnIndirect(
					&pEngine->m_pVideo->GetDestVideoPosition());
				
				if (CombineRgn(hRgnClient, hRgnClient, hRgnVideo, RGN_DIFF) != NULLREGION)
					FillRgn(hDC, hRgnClient, hBrush);

				DeleteObject(hRgnClient);
				DeleteObject(hRgnVideo);

				// Notify video to repaint itself

				pEngine->m_pVideo->GetWindowlessControl()->RepaintVideo(hWnd, hDC);
			}
			else
			{
				// Paint background color over the whole client area				

				FillRect(hDC, &rcClient, hBrush);
			}

			DeleteObject(hBrush);

			EndPaint(hWnd, &ps);

			return 0;
		}
		break;
	case WM_DISPLAYCHANGE:
		{
			// If there is a currently playing video, notify it

			if (pEngine->m_pVideo != NULL)
				pEngine->m_pVideo->GetWindowlessControl()->DisplayModeChanged();

			return 0;
		}
		break;
	case WM_DSHOWNOTIFY:
		{
			// Check through all music resources and any active video resources to
			// see which one caused the event. If it's looping, restart, otherwise stop.

			long lEventCode = 0;
			long lParam1 = 0;
			long lParam2 = 0;

			HRESULT hr = 0;

			if (pEngine->m_pVideo != NULL)
			{
				for(hr = pEngine->m_pVideo->GetMediaEvent()->GetEvent(
						&lEventCode, &lParam1, &lParam2, 0);
					SUCCEEDED(hr);
					hr = pEngine->m_pVideo->GetMediaEvent()->GetEvent(
						&lEventCode, &lParam1, &lParam2, 0))
				{
					if (EC_COMPLETE == lEventCode)
					{
						// Completed playing!

						pEngine->m_pVideo->Stop();

						if (TRUE == pEngine->m_nOptions[OPTION_GAME_EVENTS] &&
						   pEngine->m_pClientInstance != NULL)
						{
							pEngine->m_pClientInstance->OnMediaNotify(
								Client::MEDIA_END, pEngine->m_pVideo);
						}

						return 0;
					}

					pEngine->m_pVideo->GetMediaEvent()->FreeEventParams(
						lEventCode, lParam1, lParam2);					
				}
			}

			for(ResourceManager<Music>::Iterator pos =
				pEngine->GetMusic().GetBeginPos();
				pos != pEngine->GetMusic().GetEndPos();
				pos++)
			{
				Music* pMusic = pos->second;

				if (NULL == pMusic)
					continue;

				for(hr = pMusic->GetMediaEvent()->GetEvent(&lEventCode,
						&lParam1, &lParam2, 0);
					SUCCEEDED(hr);
					hr = pMusic->GetMediaEvent()->GetEvent(&lEventCode,
						&lParam1, &lParam2, 0))
				{
					if (EC_COMPLETE == lEventCode)
					{
						// Completed playing!

						if (pMusic->IsFlagSet(Music::LOOPING) == true)
						{
							pMusic->Restart();
						}
						else
						{
							pMusic->Stop();

							if (TRUE == pEngine->m_nOptions[OPTION_GAME_EVENTS] &&
							   pEngine->m_pClientInstance != NULL)
							{
								pEngine->m_pClientInstance->OnMediaNotify(
									Client::MEDIA_END,  pMusic);
							}
						}

						return 0;
					}

					pMusic->GetMediaEvent()->FreeEventParams(lEventCode,
						lParam1, lParam2);					
				}
			}

			return 0;
		}
		break;
	case WM_CLOSE:
		{
			if (0 == pEngine->m_nOldGameWindowProc)
			{
				// Only process WM_CLOSE if not subclassed.

				PostQuitMessage(0);
				return 0;
			}
		}
		break;
	}	

	if (pEngine != NULL && pEngine->m_nOldGameWindowProc != 0)
	{
		return CallWindowProc(WNDPROC(pEngine->m_nOldGameWindowProc),
			hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Engine::StartSession()
{
	if (true == m_bSessionStarted)
		EndSession();

	m_bSessionStarted = true;
	m_bSessionPaused = false;
	m_fTime = 0.0f;

	// Notify client

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
		m_pClientInstance != NULL)
		m_pClientInstance->OnSessionStart();
}

void Engine::EndSession(void)
{
	// Validate state

	if (false == m_bSessionStarted)
		throw m_ErrorStack.Push(Error::INVALID_CALL, __FUNCTIONW__);

	// Notify client

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
		m_pClientInstance != NULL)
		m_pClientInstance->OnSessionEnd();

	m_bSessionStarted = false;

	// Unload maps

	m_Maps.Empty();
}

void Engine::JoinSession(LPCWSTR pszHostName)
{
	if (FALSE == m_nOptions[OPTION_ENABLE_NETWORKING])
		throw GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	if (NULL == pszHostName || L'\0' == *pszHostName)
		throw GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	// TODO: check if networking is properly initialized, and port is valid

	// TODO: detect is pszHostName is just an IP address or a host name. If host name, resolve to IP

	// TODO: async connect, loop until windowproc will receive connect. Notify client OnSessionJoined, and if return is true
	//       request server session data (with map name, server time, server vars)

	// TODO: there must be a link between Actor and server/client. Probably implemented in Actor itself,
	//		 where actors send/receive only modified data, including user data (perhaps through Update() mechanism?)

	// all the while, don't forget OnProgress while waiting for connection, and while waiting for server header/data
	// that way client can at least display a dialog.
}

void Engine::DeserializeSession(LPCWSTR pszPath)
{
	// Get full save file path

	WCHAR szFullPath[MAX_PATH] = {0};
	
	if (*(PathFindExtension(pszPath)) != L'\0')
		GetAbsolutePath(pszPath, szFullPath);
	else
		GetBaseFilePath(pszPath, m_szBaseSavePath, m_szBaseSaveExt, szFullPath);

	// Set current directory to save file directory

	PUSH_CURRENT_DIRECTORY(szFullPath);

	// Open save file

	Stream stream(&m_ErrorStack);

	try
	{
		stream.Open(szFullPath, GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_OPEN,
			__FUNCTIONW__, stream.GetPath());
	}

	// Read session from savefile

	DeserializeSession(stream);

	// Restore current directory

	POP_CURRENT_DIRECTORY();
}

void Engine::DeserializeSession(Stream& rStream)
{
	// Cannot load while session is open; must close first

	if (true == m_bSessionStarted)
		EndSession();

	// Notify

	bool bProgressNotify = (TRUE == m_nOptions[OPTION_PROGRESS_EVENTS] &&
		m_pClientInstance != NULL);

	if (true == bProgressNotify)
	{
		m_pClientInstance->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_SESSION, 0, 1);

		m_pClientInstance->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_SESSION_HEADER, 0, 1);
	}

	// Read signature and version

	DWORD dwSignature, dwVersion;

	try
	{
		rStream.ReadVar(&dwSignature);
		rStream.ReadVar(&dwVersion);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_READ, 
			__FUNCTIONW__, rStream.GetPath());
	}

	// Validate signature and version

	if (dwSignature != *(const DWORD*)SAV_SIGNATURE)
		throw m_ErrorStack.Push(Error::FILE_SIGNATURE,
			__FUNCTIONW__, rStream.GetPath());

	if (dwVersion != *(const DWORD*)SAV_FORMAT_VERSION)
		m_ErrorStack.Push(Error::FILE_VERSION,
			__FUNCTIONW__, rStream.GetPath());

	int nCurrentMap = 0;

	try
	{
		// Read engine time

		rStream.ReadVar(&m_fTime);

		// Read engine time multiplier

		rStream.ReadVar(&m_fTimeMultiplier);

		// Read variables

		m_Variables.Deserialize(rStream);

		// Read current map index
	
		rStream.ReadVar(&nCurrentMap);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_READ,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Notify

	if (true == bProgressNotify)
		m_pClientInstance->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_SESSION_HEADER, 1, 1);

	// Read maps

	m_Maps.Deserialize(rStream);

	// Set current map based on index

	if (nCurrentMap != INVALID_INDEX && m_Maps.GetCount() > 0)
	{
		int nMap = 0;

		for(TileMapArrayIterator pos = m_Maps.GetBeginPos();
			pos != m_Maps.GetEndPos();
			pos++, nMap++)
		{
			if (nMap == nCurrentMap) m_pMap = *pos;
		}
	}

	// Notify

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
		m_pClientInstance != NULL)
		m_pClientInstance->OnSessionLoad(rStream);

	if (true == bProgressNotify)
		m_pClientInstance->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_SESSION, 1, 1);

	m_bSessionStarted = true;
}

void Engine::SerializeSession(LPCWSTR pszPath) const
{
	// Get full path

	WCHAR szFullPath[MAX_PATH] = {0};
	
	if (*(PathFindExtension(pszPath)) != L'\0')
		GetAbsolutePath(pszPath, szFullPath);
	else
		GetBaseFilePath(pszPath, m_szBaseSavePath, m_szBaseSaveExt, szFullPath);

	// Set current directory to savefile directory

	PUSH_CURRENT_DIRECTORY(szFullPath);

	// Open savefile

	Stream stream(&m_ErrorStack);

	try
	{
		stream.Open(szFullPath, GENERIC_WRITE, CREATE_ALWAYS);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_OPEN,
			__FUNCTIONW__, stream.GetPath());
	}

	// Write session to savefile

	try
	{
		SerializeSession(stream);
	}
	
	catch(Error& rError)
	{
		// Restore current directory

		POP_CURRENT_DIRECTORY();

		throw rError;
	}

	// Restore current directory

	POP_CURRENT_DIRECTORY();
}

void Engine::SerializeSession(Stream& rStream) const
{
	if (false == m_bSessionStarted)
		throw m_ErrorStack.Push(Error::INVALID_CALL, __FUNCTIONW__);

	// Notify

	bool bProgressNotify =
		(TRUE == m_nOptions[OPTION_PROGRESS_EVENTS] &&
		m_pClientInstance != NULL);

	if (true == bProgressNotify)
	{
		m_pClientInstance->OnProgress(Client::PROGRESS_SAVE,
			Client::PROGRESS_SESSION, 0, 1);

		m_pClientInstance->OnProgress(Client::PROGRESS_SAVE,
			Client::PROGRESS_SESSION_HEADER, 0, 1);
	}

	try
	{
		// Write session signature

		rStream.Write(SAV_SIGNATURE, sizeof(SAV_SIGNATURE));

		// Write session format version

		rStream.Write(SAV_FORMAT_VERSION, sizeof(SAV_SIGNATURE));

		// Write session time

		rStream.WriteVar(&m_fTime);

		// Write session time multiplier

		rStream.WriteVar(&m_fTimeMultiplier);

		// Write variables

		m_Variables.Serialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_WRITE,
			__FUNCTIONW__, rStream.GetPath());
	}	

	// Notify

	if (true == bProgressNotify)
		m_pClientInstance->OnProgress(Client::PROGRESS_SAVE, 
			Client::PROGRESS_SESSION_HEADER, 1, 1);

	try
	{
		// Get current map index

		int nCurrentMap = INVALID_INDEX;
		int n = 0;

		for(TileMapArrayConstIterator pos = m_Maps.GetBeginPosConst();
			pos != m_Maps.GetEndPosConst();
			pos++, n++)
		{
			if (*pos == m_pMap)
			{
				nCurrentMap = n;
				break;
			}
		}

		// Write current map index

		rStream.WriteVar(&nCurrentMap);

		// Write maps

		m_Maps.Serialize(rStream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_ErrorStack.Push(Error::FILE_WRITE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Notify

	if (true == bProgressNotify)
		m_pClientInstance->OnProgress(Client::PROGRESS_SAVE,
			Client::PROGRESS_SESSION, 1, 1);

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] && m_pClientInstance != NULL)
		m_pClientInstance->OnSessionSave(rStream);
}

void Engine::PauseSession(bool bPause)
{
	if (m_bSessionPaused == bPause)
		return;

	m_bSessionPaused = bPause;

	// Notify client
	
	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
		m_pClientInstance != NULL)
		m_pClientInstance->OnSessionPause(bPause);

	// Notify screens

	if (TRUE == m_nOptions[OPTION_SCREEN_EVENTS])
	{
		for(ScreenListIterator pos = m_Screens.GetBeginPos();
			pos != m_Screens.GetEndPos();
			pos++)
		{
			if (*pos != NULL)
				(*pos)->OnSessionPause(bPause);
		}
	}

	// Notify maps

	if (TRUE == m_nOptions[OPTION_MAP_EVENTS])
	{
		for(TileMapArrayIterator pos = m_Maps.GetBeginPos();
			pos != m_Maps.GetEndPos();
			pos++)
		{
			if (*pos != NULL)
				(*pos)->OnSessionPause(bPause);
		}
	}

	// Suspend or resume loaded media resources

	SoundInstanceManager& rSoundInstances =
		m_Audio.GetSoundInstances();

	if (true == bPause)
	{
		// Pause all playing sounds	

		for(SoundInstanceArrayIterator pos = rSoundInstances.GetBeginPos();
			pos != rSoundInstances.GetEndPos();
			pos++)
		{
			if ((*pos)->IsFlagSet(SoundInstance::PLAYING) == true)
			{
				(*pos)->SetFlag(SoundInstance::SUSPENDED);
				(*pos)->Pause(true);				
			}
		}

		// Pause all playing music

		for(ResourceManager<Music>::Iterator pos = m_Music.GetBeginPos();
			pos != m_Music.GetEndPos();
			pos++)
		{
			Music* pMusic = pos->second;
			if (NULL == pMusic) continue;

			if (pMusic->IsFlagSet(Music::PLAYING) == true)
			{
				pMusic->SetFlag(Music::SUSPENDED);
				pMusic->Pause(true);
			}
		}

		// Pause currently playing video if any

		if (m_pVideo != NULL &&
			m_pVideo->IsFlagSet(Video::PLAYING) == true)
		{
			m_pVideo->SetFlag(Video::SUSPENDED);
			m_pVideo->Pause(true);
		}
	}
	else
	{
		// Resume suspended sounds

		for(SoundInstanceArrayIterator pos = rSoundInstances.GetBeginPos();
			pos != rSoundInstances.GetEndPos();
			pos++)
		{
			if ((*pos)->IsFlagSet(SoundInstance::SUSPENDED) == true)
			{
				(*pos)->ClearFlag(SoundInstance::SUSPENDED);
				(*pos)->Pause(false);
			}
		}

		// Resume suspended music

		for(ResourceManager<Music>::Iterator pos = m_Music.GetBeginPos();
			pos != m_Music.GetEndPos();
			pos++)
		{
			Music* pMusic = pos->second;
			if (NULL == pMusic) continue;

			if (pMusic->IsFlagSet(Music::SUSPENDED) == true)
			{
				pMusic->ClearFlag(Music::SUSPENDED);
				pMusic->Pause(false);
			}
		}

		// Resume suspended video if any

		if (m_pVideo != NULL &&
			m_pVideo->IsFlagSet(Video::SUSPENDED) == true)
		{
			m_pVideo->ClearFlag(Video::SUSPENDED);
			m_pVideo->Pause(false);
		}
	}
}

void Engine::SetOption(Options nIndex, int nValue)
{
	// Perform any validation or required actions
	// Note: cases go in the order of most-used to
	// least-used for performance reasons

	switch(nIndex)
	{
	case OPTION_SHOW_CURSOR:
		{
			if (TRUE == m_nOptions[OPTION_CUSTOM_CURSOR])
			{
				// Unload current cursor

				::SetCursor(NULL);

				// Make sure cursor is hidden as many times
				// as shown so it's hidden

				while(::ShowCursor(false) >= 0);
			}
			else
			{
				if (nValue != 0)
				{
					::SetCursor(LoadCursor(NULL, IDC_ARROW));		
					
					// Make sure cursor is shown as many times
					// as it is hidden so it is visible

					while(::ShowCursor(true) < 1);
				}
				else
				{
					// Make sure cursor is hidden as many times
					// as shown so it's hidden

					while(::ShowCursor(false) >= 0);
				}
			}

			m_nOptions[OPTION_SHOW_CURSOR] = nValue;
		}
		break;
	case OPTION_CUSTOM_CURSOR:
		{
			m_nOptions[OPTION_CUSTOM_CURSOR] = nValue;

			SetOption(OPTION_SHOW_CURSOR, m_nOptions[OPTION_SHOW_CURSOR]);
		}
		break;
	case OPTION_DISABLE_SOUNDS:
	case OPTION_DISABLE_MUSIC:
	case OPTION_EXCLUSIVE_SOUND:
	case OPTION_ENABLE_NETWORKING:
	case OPTION_MANAGE_COM:
		{
			if (true == m_bSessionStarted)
				throw m_ErrorStack.Push(Error::INVALID_CALL, __FUNCTIONW__);
		}
		break;
	case OPTION_AUDIO_DESTINATION:
		{
			m_nOptions[nIndex] = nValue;

			if (true == m_bSessionStarted)
				m_Audio.SetDestination(Audio::Destinations(nValue));

			return;
		}
		break;

	} // switch

	// And update the option value

	m_nOptions[nIndex] = nValue;
}

void Engine::ResetOptions(void)
{
	// Zero all options

	ZeroMemory(m_nOptions, sizeof(m_nOptions));

	// Set default values for options that are not 0 by default

	m_nOptions[OPTION_TILE_SIZE] = DEFAULT_TILE_SIZE;
	m_nOptions[OPTION_MANAGE_COM] = TRUE;
	m_nOptions[OPTION_RENDER_SCREENS] = TRUE;
	m_nOptions[OPTION_RENDER_MAP] = TRUE;
	m_nOptions[OPTION_SCREEN_EVENTS] = TRUE;
	m_nOptions[OPTION_GAME_EVENTS] = TRUE;
	m_nOptions[OPTION_RESOURCE_CACHE_FREQUENCY] = DEFAULT_RESOURCECACHEFREQUENCY;
	m_nOptions[OPTION_STREAM_CACHE_FREQUENCY] = DEFAULT_STREAMCACHEFREQUENCY;
	m_nOptions[OPTION_SOUND_CACHE_FREQUENCY] = DEFAULT_SOUNDCACHEFREQUENCY;

	// Set options that require processing
	
	SetOptionEx(OPTION_EFFECT_COMPILE_FLAGS, D3DXSHADER_OPTIMIZATION_LEVEL3);
	SetOption(OPTION_MAX_BATCH_PRIM, 10000);
	SetOption(OPTION_SHOW_CURSOR, TRUE);
}

void Engine::ResetCursorPosition()
{
	RECT rcClient;
	GetClientRect(m_hGameWindow, &rcClient);

	RECT rcWindow;
	ClientToScreen(m_hGameWindow, (LPPOINT)&rcClient.left);
	ClientToScreen(m_hGameWindow, (LPPOINT)&rcClient.right);

	POINT ptCursorPos;
	GetCursorPos(&ptCursorPos);

	if (PtInRect(&rcWindow, ptCursorPos) == TRUE)
	{
		// If inside the window, set to that position

		ScreenToClient(m_hGameWindow, &ptCursorPos);

		m_vecCursorPos.x = float(ptCursorPos.x);
		m_vecCursorPos.y = float(ptCursorPos.y);
	}
	else
	{
		// If outside the window, center in the window

		Texture* pBase = m_CustomCursor.GetBaseTexture();

		if (pBase != NULL)
		{
			m_vecCursorPos.x =
				float(rcClient.right - pBase->GetInfo().Width) / 2.0f;

			m_vecCursorPos.y =
				float(rcClient.bottom - pBase->GetInfo().Height) / 2.0f;
		}
		else
		{
			// Assume custom cursor will be 32x32 if no material yet set

			m_vecCursorPos.x =
				float(rcClient.right - 32) / 2.0f;

			m_vecCursorPos.y =
				float(rcClient.bottom - 32) / 2.0f;
		}
	}
}

void Engine::SetCursorPosition(const Vector2& vecCursorPosition)
{
	m_vecCursorPos = vecCursorPosition;

	POINT pt = { int(vecCursorPosition.x),
				 int(vecCursorPosition.y) };

	ClientToScreen(m_hGameWindow, &pt);

	SetCursorPos(pt.x, pt.y);
}

void Engine::GetBaseFilePath(LPCWSTR pszName,
							 LPCWSTR pszDefaultSubDir,
							 LPCWSTR pszDefaultExtensionWithDot,
							 LPWSTR pszOutBaseFilePath) const
{
	if ((PathFindFileName(pszName) != pszName) &&
		PathFileExists(pszName) == TRUE)
	{
		// If not file tile and exists, return the same name

		wcscpy_s(pszOutBaseFilePath, MAX_PATH, pszName);
		return;
	}

	if (m_pClientInstance != NULL)
	{
		wcscpy_s(pszOutBaseFilePath, MAX_PATH,
			m_pClientInstance->GetBaseDirectory());
	}
	else
	{
		GetModuleFileName(GetModuleHandle(NULL),
			pszOutBaseFilePath, MAX_PATH);

		PathRemoveFileSpec(pszOutBaseFilePath);
	}

	PathAppend(pszOutBaseFilePath, pszDefaultSubDir);

	PathAppend(pszOutBaseFilePath, pszName);

	// If no extension found (and extension is required), append extension

	if (String::IsEmpty(pszDefaultExtensionWithDot) == false)
	{
		LPWSTR pszExt = PathFindExtension(pszOutBaseFilePath);

		if (NULL == *pszExt ||
			wcscmp(pszExt, pszDefaultExtensionWithDot) != 0)
			wcscpy_s(pszExt, wcslen(pszDefaultExtensionWithDot) + 1,
				pszDefaultExtensionWithDot);
	}
}

void Engine::Render(void)
{
	// If there is a full screen video playing, don't render

	if (m_pVideo != NULL) return;

	// Render map

	if (TRUE == m_nOptions[OPTION_RENDER_MAP] && m_pMap != NULL)
		m_pMap->Render();

	// Render screens

	if (TRUE == m_nOptions[OPTION_RENDER_SCREENS])
		m_Screens.Render();

	// Render custom cursor

	RenderCustomCursor();
}

void Engine::RenderCustomCursor(void)
{
	if (TRUE == m_nOptions[OPTION_SHOW_CURSOR] &&
	   TRUE == m_nOptions[OPTION_CUSTOM_CURSOR] &&
	   m_CustomCursor.IsEmpty() == false)
	{
		m_Graphics.RenderQuad(m_CustomCursor, m_vecCursorPos);
	}
}

void Engine::OnLostDevice(bool bRecreate)
{
	// Notify objects that care about lost device...
	// Important: objects that may own a reference to other objects must be notified first
	// Otherwise they may try to release objects (for example textures) that have just been released.

	// Notify maps of lost device

	if (TRUE == m_nOptions[OPTION_MAP_EVENTS])
		m_Maps.OnLostDevice(bRecreate);

	// Notify screens of lost device

	if (TRUE == m_nOptions[OPTION_SCREEN_EVENTS])
		m_Screens.OnLostDevice(bRecreate);

	// Notify client of lost device

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
	   m_pClientInstance != NULL)
	{
		m_pClientInstance->OnLostDevice(bRecreate);
	}

	// Notify fonts of lost device

	m_Fonts.OnLostDevice(bRecreate);

	// Notify effects of lost device

	m_Effects.OnLostDevice(bRecreate);

	// Notify textures of lost device

	m_Textures.OnLostDevice(bRecreate);
	m_CubeTextures.OnLostDevice(bRecreate);

	// Notify cached resources of lost device

	m_ResourceCache.OnLostDevice(bRecreate);
}

void Engine::OnResetDevice(bool bRecreate)
{
	// Notify textures of reset device

	m_Textures.OnResetDevice(bRecreate);
	m_CubeTextures.OnResetDevice(bRecreate);

	// Notify effects of reset device

	m_Effects.OnResetDevice(bRecreate);

	// Notify cached resources of reset device

	m_ResourceCache.OnResetDevice(bRecreate);

	// Notify fonts of reset device

	m_Fonts.OnResetDevice(bRecreate);

	// Notify screens of reset device

	if (TRUE == m_nOptions[OPTION_SCREEN_EVENTS])
		m_Screens.OnResetDevice(bRecreate);

	// Notify maps of reset device

	if (TRUE == m_nOptions[OPTION_MAP_EVENTS])
		m_Maps.OnResetDevice(bRecreate);

	// Notify client of reset device

	if (TRUE == m_nOptions[OPTION_GAME_EVENTS] &&
	   m_pClientInstance != NULL)
	{
		m_pClientInstance->OnResetDevice(bRecreate);
	}
}

void Engine::SetCurrentVideo(Video* pVideo)
{
	if (m_pVideo == pVideo) return;

	if (pVideo != NULL) pVideo->AddRef();

	if (m_pVideo != NULL)
	{
		if (m_pVideo->IsFlagSet(Video::PLAYING) == true)
			m_pVideo->Stop();

		m_pVideo->Release();
	}

	m_pVideo = pVideo;
}

void Engine::Update(void)
{
	// Update frame time

	if (QueryPerformanceCounter((LARGE_INTEGER*)&m_qwCurTick) == FALSE)
		throw GetErrors().Push(Error::WIN_SYS_QUERYPERFORMANCECOUNTER,
			__FUNCTIONW__, GetLastError());

	m_fFrameTime = float(m_qwCurTick - m_qwLastTick) / m_fFreq;
	m_qwLastTick = m_qwCurTick;	

	// Update run time

	m_fRunTime += m_fFrameTime;	

	// Update stream cache
	
	if (TRUE == m_nOptions[OPTION_ENABLE_STREAM_CACHE])
	{
		m_fStreamCacheAccum += m_fFrameTime;
		
		if (m_fStreamCacheAccum >
		   float(m_nOptions[OPTION_STREAM_CACHE_FREQUENCY]) - TIME_EPSILON)
		{
			m_StreamCache.Update();

			m_fStreamCacheAccum = 0.0f;
		}
	}

	// Update resource cache

	if (TRUE == m_nOptions[OPTION_ENABLE_RESOURCE_CACHE])
	{
		m_fResourceCacheAccum += m_fFrameTime;

		if (m_fResourceCacheAccum >
		   float(m_nOptions[OPTION_RESOURCE_CACHE_FREQUENCY]) - TIME_EPSILON)
		{
			m_ResourceCache.Update();

			m_fResourceCacheAccum = 0.0f;
		}
	}

	// Update sound instances

	m_fSoundCacheAccum += m_fFrameTime;

	if (m_fSoundCacheAccum >
		float(m_nOptions[OPTION_SOUND_CACHE_FREQUENCY]) - TIME_EPSILON)
	{
		m_Audio.Update(m_fRunTime);

		m_fSoundCacheAccum = 0.0f;
	}

	// Update timers

	m_Timers.Update(m_fRunTime);

	// Update session if not paused

	if (false == m_bSessionPaused)
	{
		// Update session time

		m_fTime += m_fFrameTime * m_fTimeMultiplier;

		// Update maps

		m_Maps.Update();
	}
}

void Engine::Print(LPCWSTR pszText, PrintTypes nType, bool bLine)
{
	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(pszText, nType, bLine);
}

void Engine::PrintMessage(LPCWSTR pszText, ...)
{
	va_list pArgs;
	va_start(pArgs, pszText);

	String str;
	str.Format(pszText, pArgs);

	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(str, PRINT_MESSAGE, true);

	va_end(pArgs);
}

void Engine::PrintError(LPCWSTR pszText, ...)
{
	va_list pArgs;
	va_start(pArgs, pszText);

	String str;
	str.Format(pszText, pArgs);

	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(str, PRINT_ERROR, true);

	va_end(pArgs);
}

void Engine::PrintWarning(LPCWSTR pszText, ...)
{
	va_list pArgs;
	va_start(pArgs, pszText);

	String str;
	str.Format(pszText, pArgs);

	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(str, PRINT_WARNING, true);

	va_end(pArgs);
}

void Engine::PrintInfo(LPCWSTR pszText, ...)
{
	va_list pArgs;
	va_start(pArgs, pszText);

	String str;
	str.Format(pszText, pArgs);

	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(str, PRINT_INFO, true);

	va_end(pArgs);
}

void Engine::PrintDebug(LPCWSTR pszText, ...)
{
	va_list pArgs;
	va_start(pArgs, pszText);

	String str;
	str.Format(pszText, pArgs);

	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(str, PRINT_DEBUG, true);

	va_end(pArgs);
}

void Engine::PrintBlank(void)
{
	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(NULL, PRINT_MESSAGE, true);
}

void Engine::PrintClear(void)
{
	if (m_pClientInstance != NULL)
		m_pClientInstance->Print(NULL, PRINT_CLEAR, false);
}

DWORD Engine::GetMemoryFootprint(void) const
{
	DWORD dwSize = sizeof(Engine);

	if (m_Graphics.IsInitialized() == true)
	{
		// Back buffer and front buffer memory

		dwSize += m_Graphics.GetDeviceParams().BackBufferWidth *
				  m_Graphics.GetDeviceParams().BackBufferHeight *
				  (m_Graphics.GetDeviceParams().BackBufferFormat ==
					D3DFMT_X8B8G8R8 ? 4 : 2) * 2;
	}

	dwSize += m_Textures.GetMemoryFootprint();

	dwSize += m_Animations.GetMemoryFootprint();

	dwSize += m_Materials.GetMemoryFootprint();

	dwSize += m_Effects.GetMemoryFootprint();

	dwSize += m_Sprites.GetMemoryFootprint();

	dwSize += m_Sounds.GetMemoryFootprint();

	dwSize += m_Music.GetMemoryFootprint();

	dwSize += m_Fonts.GetMemoryFootprint();

	dwSize += m_Strings.GetMemoryFootprint();

	dwSize += m_ResourceCache.GetMemoryFootprint();

	dwSize += m_Maps.GetMemoryFootprint();

	dwSize += m_Screens.GetMemoryFootprint();

	dwSize += m_Commands.GetMemoryFootprint();

	dwSize += m_Variables.GetMemoryFootprint();

	dwSize += m_Timers.GetCount() * sizeof(Timer);

	dwSize += m_Audio.GetSoundInstancesConst().GetCount() *
		sizeof(SoundInstance);

	dwSize += m_ErrorStack.GetCount() * sizeof(Error);

	dwSize += m_Classes.GetMemoryFootprint();

	dwSize += m_StreamCache.GetMemoryFootprint();

	dwSize += m_ResourceCache.GetMemoryFootprint();

	return dwSize;
}