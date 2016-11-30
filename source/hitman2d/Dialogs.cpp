/*------------------------------------------------------------------*\
|
| Dialogs.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D dialog box procedures implementation
| Created: 08/03/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"		// precompiled header
#include "resource.h"	// using resource constants
#include "Dialogs.h"	// using resource constants, defining dialog procedures
#include "Game.h"		// using Game
#include "Globals.h"	// using DW_THUDEVICEFORMATS
#include <crtdbg.h>		// using ASSERT
#include <commctrl.h>	// using UpDown control, Slider control
#include <commdlg.h>	// using OPENFILENAME, GetSaveFileName

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const LPCWSTR SZ_LOGMODES_DLG[] =	{
										L"Disable Logging",
										L"Text Logging",
										L"HTML Logging"
									};


/*----------------------------------------------------------*\
| DialogGraphics implementation
\*----------------------------------------------------------*/

DialogGraphics::DialogGraphics(Game* pGame)
{
	_ASSERT(pGame);

	m_bHWAccel = pGame->GetHardwareAcceleration();
	m_bSWVP = pGame->GetSoftwareVertexProcessing();
	m_bPure = pGame->GetPureDevice();
	m_bShaderDebug = pGame->GetShaderDebug();
}

DialogGraphics::~DialogGraphics(void)
{
}

int DialogGraphics::Show(HWND hWndParent, LPDIRECT3D9 pD3D)
{
	_ASSERT(pD3D);

	m_pD3D = pD3D;

	return int(DialogBoxParam((HINSTANCE)GetModuleHandle(NULL),
		(LPCWSTR)IDD_GRAPHICS, hWndParent,
		(DLGPROC)DlgProc, (LPARAM)this));
}

INT_PTR CALLBACK DialogGraphics::DlgProc(HWND hDlg,
									  UINT uMsg,
									  WPARAM wParam,
									  LPARAM lParam)
{
	DialogGraphics* pData =
		(DialogGraphics*)GetWindowLongPtr(hDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			// Set data

			_ASSERT(lParam);

			SetWindowLongPtr(hDlg, GWL_USERDATA, lParam);

			pData = (DialogGraphics*)lParam;

			// Initialize controls

			SendDlgItemMessage(hDlg, IDC_HWACCEL, BM_SETCHECK,
				pData->m_bHWAccel ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hDlg, IDC_HWVP, BM_SETCHECK,
				pData->m_bSWVP ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hDlg, IDC_PURE, BM_SETCHECK,
				pData->m_bPure ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hDlg, IDC_SHD, BM_SETCHECK,
				pData->m_bShaderDebug ? BST_CHECKED : BST_UNCHECKED, 0);

			HWND hParent = GetParent(hDlg);

			int nSelFormat = int(SendDlgItemMessage(hParent,
				IDC_FORMAT, CB_GETCURSEL, 0, 0));

			int nFormatData = int(SendDlgItemMessage(hParent,
				IDC_FORMAT, CB_GETITEMDATA, nSelFormat, 0));

			BYTE* byFormatData = (BYTE*)&nFormatData;

			Graphics::DeviceFormats nFormat =
				(Graphics::DeviceFormats)byFormatData[2];

			D3DFORMAT nDispFormat = D3DFMT_X8R8G8B8;
			D3DFORMAT nBufferFormat = D3DFMT_X8R8G8B8;

			if (nFormat > Graphics::FORMAT_DESKTOP)
			{
				nDispFormat =
					(D3DFORMAT)DW_THUDEVICEFORMATS[nFormat];

				nBufferFormat = nDispFormat;
			}
			else
			{
				D3DDISPLAYMODE dmDesktop = {0};

				pData->m_pD3D->GetAdapterDisplayMode(
					D3DADAPTER_DEFAULT, &dmDesktop);

				nDispFormat = dmDesktop.Format;
				nBufferFormat = dmDesktop.Format;
			}

			// Set focus to first control in tab order

			return true;
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					// Save data

					pData->m_bHWAccel = (SendDlgItemMessage(hDlg, IDC_HWACCEL,
						BM_GETCHECK, 0, 0) == BST_CHECKED);
					
					pData->m_bSWVP = (SendDlgItemMessage(hDlg, IDC_HWVP,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_bShaderDebug = (SendDlgItemMessage(hDlg, IDC_SHD,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_bPure = (SendDlgItemMessage(hDlg, IDC_PURE,
						BM_GETCHECK, 0, 0) == BST_CHECKED);
		
					EndDialog(hDlg, IDOK);
				}
				break;
			case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
		}
		break;
	}

	return false;
}

/*----------------------------------------------------------*\
| DialogAudio implementation
\*----------------------------------------------------------*/

DialogAudio::DialogAudio(Game* pGame)
{
	_ASSERT(pGame);

	m_bExclusive = (pGame->GetEngine().GetOption(
		Engine::OPTION_EXCLUSIVE_SOUND) == TRUE);

	m_bDisableSounds = (pGame->GetEngine().GetOption(
		Engine::OPTION_DISABLE_SOUNDS) == TRUE);

	m_bDisableMusic = (pGame->GetEngine().GetOption(
		Engine::OPTION_DISABLE_MUSIC) == TRUE);
}

DialogAudio::~DialogAudio(void)
{
}

bool DialogAudio::GetExclusive(void) const
{
	return m_bExclusive;
}

bool DialogAudio::GetDisableSound(void) const
{
	return m_bDisableSounds;
}

bool DialogAudio::GetDisableMusic(void) const
{
	return m_bDisableMusic;
}

int DialogAudio::Show(HWND hWndParent)
{
	return int(DialogBoxParam((HINSTANCE)GetModuleHandle(NULL),
		(LPCWSTR)IDD_AUDIO, hWndParent,
		(DLGPROC)DlgProc, (LPARAM)this)); 
}

INT_PTR CALLBACK DialogAudio::DlgProc(HWND hDlg,
									  UINT uMsg,
									  WPARAM wParam,
									  LPARAM lParam)
{
	DialogAudio* pData =
		(DialogAudio*)GetWindowLongPtr(hDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			// Set data

			_ASSERT(lParam);

			SetWindowLongPtr(hDlg, GWL_USERDATA, lParam);

			pData = (DialogAudio*)lParam;

			// Initialize controls

			SendDlgItemMessage(hDlg, IDC_EXCLUSIVESOUND,
				BM_SETCHECK, pData->m_bExclusive ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hDlg, IDC_DISABLESOUNDS,
				BM_SETCHECK, pData->m_bDisableSounds ? BST_CHECKED : BST_UNCHECKED, 0);

			SendDlgItemMessage(hDlg, IDC_DISABLEMUSIC,
				BM_SETCHECK, pData->m_bDisableMusic ? BST_CHECKED : BST_UNCHECKED, 0);

			// Set focus to the first control in tab order

			return true;
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					pData->m_bExclusive =
						(SendDlgItemMessage(hDlg, IDC_EXCLUSIVESOUND,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_bDisableSounds =
						(SendDlgItemMessage(hDlg, IDC_DISABLESOUNDS,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_bDisableMusic =
						(SendDlgItemMessage(hDlg, IDC_DISABLEMUSIC,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					EndDialog(hDlg, IDOK);
				}
				break;
			case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
		}
		break;
	}

	return false;
}

/*----------------------------------------------------------*\
| DialogStartup implementation
\*----------------------------------------------------------*/

DialogStartup::DialogStartup(Game* pGame)
{
	if (pGame->GetLogFilePath().IsEmpty() == true)
		pGame->SetLogFilePath(NULL);

	if (pGame->GetScriptPath().IsEmpty() == true)
		pGame->SetScriptPath(NULL);

	if (pGame->GetControlsProfilePath().IsEmpty() == true)
		pGame->SetControlsProfilePath(NULL);

	m_strLogFilePath = pGame->GetLogFilePath();
	m_strScriptPath = pGame->GetScriptPath();
	m_strControlsPath = pGame->GetControlsProfilePath();

	m_nLogMode = pGame->GetLogMode();
}

DialogStartup::~DialogStartup(void)
{
}

const String& DialogStartup::GetLogFilePath(void) const
{
	return m_strLogFilePath;
}

const String& DialogStartup::GetScriptPath(void) const
{
	return m_strScriptPath;
}

const String& DialogStartup::GetControlsPath(void) const
{
	return m_strControlsPath;
}

Client::LogMode DialogStartup::GetLogMode(void) const
{
	return m_nLogMode;
}

int DialogStartup::Show(HWND hWndParent)
{
	return int(DialogBoxParam((HINSTANCE)GetModuleHandle(NULL),
		(LPCWSTR)IDD_STARTUP, hWndParent, (DLGPROC)DlgProc, (LPARAM)this));
}

INT_PTR CALLBACK DialogStartup::DlgProc(HWND hDlg,
										UINT uMsg,
										WPARAM wParam,
										LPARAM lParam)
{
	DialogStartup* pData =
		(DialogStartup*)GetWindowLongPtr(hDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			// Set Data

			_ASSERT(lParam);

			SetWindowLongPtr(hDlg, GWL_USERDATA, lParam);
			
			pData = (DialogStartup*)lParam;

			// Initialize Controls

			SetDlgItemText(hDlg, IDC_LOGFILEPATH,
				pData->m_strLogFilePath);

			SetDlgItemText(hDlg, IDC_SCRIPTFILEPATH,
				pData->m_strScriptPath);

			SetDlgItemText(hDlg, IDC_CONTROLSPATH,
				pData->m_strControlsPath);

			HWND hWndLogMode = GetDlgItem(hDlg, IDC_LOGMODE);

			for(int n = 0; n < Client::LOG_COUNT; n++)
			{
				SendMessage(hWndLogMode, CB_ADDSTRING, 0,
					(LPARAM)SZ_LOGMODES_DLG[n]);
			}

			SendMessage(hWndLogMode, CB_SETCURSEL, pData->m_nLogMode, 0);

			return true;
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_BROWSELOGPATH:
				{
					OPENFILENAME ofn = {0};

					WCHAR szCurDir[MAX_PATH] = {0};
					GetCurrentDirectory(MAX_PATH - 1, szCurDir);

					WCHAR szPath[MAX_PATH] = {0};
					wcscpy_s(szPath, MAX_PATH, pData->m_strLogFilePath);

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.Flags = OFN_CREATEPROMPT;
					ofn.lpstrTitle = L"Browse For Log File";
					ofn.lpstrFile = szPath;
					ofn.nMaxFile = MAX_PATH - 1;

					ofn.lpstrFilter = L"Log Files (*.log)\0*.log\0"
						L"Text Files (*.txt)\0*.txt\0";

					ofn.lpstrDefExt = L".log";
					ofn.lpstrInitialDir = szCurDir;

					if (GetSaveFileName(&ofn) == TRUE)
					{
						WCHAR szRelPath[128] = {0};

						PathRelativePathTo(szRelPath, szCurDir,
							FILE_ATTRIBUTE_DIRECTORY, szPath,
							FILE_ATTRIBUTE_NORMAL);

						SetDlgItemText(hDlg, IDC_LOGFILEPATH, szRelPath);

						SetCurrentDirectory(szCurDir);
					}
				}
				break;
			case IDC_CLEARLOG:
				{
					// Delete the log file

					WCHAR szRelLogPath[MAX_PATH] = {0};
					WCHAR szLogPath[MAX_PATH] = {0};

					GetDlgItemText(hDlg, IDC_LOGFILEPATH, szRelLogPath, MAX_PATH);
					GetAbsolutePath(szRelLogPath, szLogPath);

					DeleteFile(szLogPath);
				}
				break;
			case IDC_BROWSESCRIPTPATH:
				{
					OPENFILENAME ofn = {0};

					WCHAR szCurDir[MAX_PATH] = {0};
					GetCurrentDirectory(MAX_PATH - 1, szCurDir);

					WCHAR szPath[MAX_PATH] = {0};
					wcscpy_s(szPath, MAX_PATH, pData->m_strScriptPath);

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.Flags = OFN_CREATEPROMPT;
					ofn.lpstrTitle = L"Browse For Startup Script";
					ofn.lpstrFile = szPath;
					ofn.nMaxFile = MAX_PATH - 1;
					ofn.lpstrFilter = L"ThunderStorm Scripts (*.thc)\0*.thc\0";
					ofn.lpstrDefExt = L".thc";
					ofn.lpstrInitialDir = szCurDir;

					if (GetSaveFileName(&ofn) == TRUE)
					{
						WCHAR szRelPath[128] = {0};

						PathRelativePathTo(szRelPath, szCurDir,
							FILE_ATTRIBUTE_DIRECTORY, szPath,
							FILE_ATTRIBUTE_NORMAL);

						SetDlgItemText(hDlg, IDC_SCRIPTFILEPATH, szRelPath);

						SetCurrentDirectory(szCurDir);
					}
				}
				break;
			case IDC_BROWSECONTROLSPATH:
				{
					OPENFILENAME ofn = {0};

					WCHAR szCurDir[MAX_PATH] = {0};
					GetCurrentDirectory(MAX_PATH - 1, szCurDir);

					WCHAR szPath[MAX_PATH] = {0};
					wcscpy_s(szPath, MAX_PATH, pData->m_strControlsPath);

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.Flags = OFN_CREATEPROMPT;
					ofn.lpstrTitle = L"Browse For Controls Profile";
					ofn.lpstrFile = szPath;
					ofn.nMaxFile = MAX_PATH - 1;
					ofn.lpstrFilter = L"Profiles (*.ini)\0*.ini\0";
					ofn.lpstrDefExt = L".ini";
					ofn.lpstrInitialDir = szCurDir;

					if (GetSaveFileName(&ofn) == TRUE)
					{
						WCHAR szRelPath[128] = {0};

						PathRelativePathTo(szRelPath, szCurDir,
							FILE_ATTRIBUTE_DIRECTORY, szPath, FILE_ATTRIBUTE_NORMAL);

						SetDlgItemText(hDlg, IDC_CONTROLSPATH, szRelPath);

						SetCurrentDirectory(szCurDir);
					}
				}
				break;
			case IDOK:
				{
					// Transfer data from controls

					if (pData->m_strLogFilePath.Allocate(MAX_PATH) == FALSE)
					{
						MessageBox(hDlg, L"Failed to allocate memory.",
							NULL, MB_ICONSTOP);

						break;
					}

					GetDlgItemText(hDlg, IDC_LOGFILEPATH,
						pData->m_strLogFilePath.GetBuffer(), MAX_PATH - 1);

					if (pData->m_strScriptPath.Allocate(MAX_PATH) == FALSE)
					{
						MessageBox(hDlg, L"Failed to allocate memory.",
							NULL, MB_ICONSTOP);

						break;
					}

					GetDlgItemText(hDlg, IDC_SCRIPTFILEPATH,
						pData->m_strScriptPath.GetBuffer(), MAX_PATH - 1);

					if (pData->m_strControlsPath.Allocate(MAX_PATH) == FALSE)
					{
						MessageBox(hDlg, L"Failed to allocate memory.",
							NULL, MB_ICONSTOP);

						break;
					}

					GetDlgItemText(hDlg, IDC_CONTROLSPATH,
						pData->m_strControlsPath.GetBuffer(), MAX_PATH - 1);

					pData->m_nLogMode = (Client::LogMode)SendDlgItemMessage(
						hDlg, IDC_LOGMODE, CB_GETCURSEL, 0, 0);

					if (pData->m_nLogMode == CB_ERR)
						pData->m_nLogMode = Client::LOG_DISABLE;

					EndDialog(hDlg, IDOK);
				}
				break;
			case IDCANCEL:
				{
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, IDCANCEL);
		}
		break;
	}

	return false;
}

/*----------------------------------------------------------*\
| DialogConfigure implementation
\*----------------------------------------------------------*/

DialogConfigure::DialogConfigure(Game* pGame):	m_pGame(pGame),
												m_pD3D(NULL),
												m_nDevFormat(Graphics::FORMAT_DESKTOP),
												m_dwResWidth(0),
												m_dwResHeight(0),
												m_dwRefresh(0),

												m_DialogGraphics(pGame),
												m_dialogAudio(pGame),
												m_dialogStartup(pGame)
{
	_ASSERT(pGame);

	m_nDevFormat = pGame->GetDeviceFormat();
	m_dwResWidth = (DWORD)pGame->GetDisplayWidth();
	m_dwResHeight = (DWORD)pGame->GetDisplayHeight();
	m_nMultiSampleType = (int)pGame->GetMultiSampleType();
	m_dwMultiSampleQuality = pGame->GetMultiSampleQuality();
	m_dwRefresh = pGame->GetRefreshRate();
}

DialogConfigure::~DialogConfigure(void)
{
}

int DialogConfigure::Show(void)
{
	return int(DialogBoxParam((HINSTANCE)GetModuleHandle(NULL),
		(LPCWSTR)IDD_CONFIGURE, NULL, (DLGPROC)DlgProc, (LPARAM)this));
}

INT_PTR CALLBACK DialogConfigure::DlgProc(HWND hDlg,
										  UINT uMsg,
										  WPARAM wParam,
										  LPARAM lParam)
{
	DialogConfigure* pData =
		(DialogConfigure*)GetWindowLongPtr(hDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			// Get extra data

			if (lParam)
			{
				pData = (DialogConfigure*)lParam;

				SetWindowLongPtr(hDlg, GWL_USERDATA, lParam);

				// Create a D3D9 instance

				HRESULT hr = CoInitialize(NULL);

				if (FAILED(hr))
				{
					MessageBox(NULL, L"Failed to initialize COM.",
						pData->m_pGame->GetTitle(), MB_ICONSTOP);

					return false;
				}

				pData->m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

				if (NULL == pData->m_pD3D)
				{
					CoUninitialize();

					MessageBox(NULL, L"Failed to create IDirect3D9 instance.",
						pData->m_pGame->GetTitle(), MB_ICONSTOP);

					return false;
				}

				// Display default adapter information

				D3DADAPTER_IDENTIFIER9 adapterID = {0};

				if (FAILED(pData->m_pD3D->GetAdapterIdentifier(0, 0, &adapterID)))
				{
					SetDlgItemText(hDlg, IDC_STATIC_ADAPTER,
						L"Failed to retrieve adapter info.");
				}
				else
				{
					WCHAR szAdapter[256] = {0};
					mbstowcs_s(NULL, szAdapter, 256, adapterID.Description, 256);

					SetDlgItemText(hDlg, IDC_STATIC_ADAPTER, szAdapter);
				}

				// Check or uncheck full screen

				SendDlgItemMessage(hDlg, IDC_FULLSCREEN, BM_SETCHECK,
					pData->m_pGame->GetFullScreen() ? BST_CHECKED : BST_UNCHECKED, 0);

				SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_FULLSCREEN, BN_CLICKED), 0);

				// Check or uncheck vsync

				SendDlgItemMessage(hDlg, IDC_VSYNC, BM_SETCHECK,
					pData->m_pGame->GetVSync() ? BST_CHECKED : BST_UNCHECKED, 0);

				// Check or uncheck show configure (only if set by user)

				if (pData->m_pGame->GetConfigureExternal() == false)
					SendDlgItemMessage(hDlg, IDC_SHOWCONFIGURE, BM_SETCHECK,
					pData->m_pGame->GetShowConfigure() ? BST_CHECKED : BST_UNCHECKED, 0);

				// Update volume sliders

				int nPos = int(pData->m_pGame->GetEffectsVolume() * 100);

				SendDlgItemMessage(hDlg, IDC_SLIDER_EFFECTSVOLUME,
					TBM_SETRANGE, true, MAKELPARAM(0, 100));

				SendDlgItemMessage(hDlg, IDC_SLIDER_EFFECTSVOLUME,
					TBM_SETPOS, true, nPos);

				SendMessage(hDlg, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos),
					(LPARAM)GetDlgItem(hDlg, IDC_SLIDER_EFFECTSVOLUME));

				nPos = int(pData->m_pGame->GetSpeechVolume() * 100);

				SendDlgItemMessage(hDlg, IDC_SLIDER_SPEECHVOLUME,
					TBM_SETRANGE, true, MAKELPARAM(0, 100));

				SendDlgItemMessage(hDlg, IDC_SLIDER_SPEECHVOLUME,
					TBM_SETPOS, true, nPos);

				SendMessage(hDlg, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos),
					(LPARAM)GetDlgItem(hDlg, IDC_SLIDER_SPEECHVOLUME));

				nPos = int(pData->m_pGame->GetMusicVolume() * 100);

				SendDlgItemMessage(hDlg, IDC_SLIDER_MUSICVOLUME,
					TBM_SETRANGE, true, MAKELPARAM(0, 100));

				SendDlgItemMessage(hDlg, IDC_SLIDER_MUSICVOLUME,
					TBM_SETPOS, true, nPos);

				SendMessage(hDlg, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos),
					(LPARAM)GetDlgItem(hDlg, IDC_SLIDER_MUSICVOLUME));

				// Add items for audio destination

				HWND hWndAudioDest = GetDlgItem(hDlg, IDC_AUDIODEST);

				SendMessage(hWndAudioDest, CB_ADDSTRING, 0, (LPARAM)L"Speakers");
				SendMessage(hWndAudioDest, CB_ADDSTRING, 0, (LPARAM)L"Headphones");

				// Select audio destination

				SendMessage(hWndAudioDest, CB_SETCURSEL,
					pData->m_pGame->GetEngine().GetOption(
						Engine::OPTION_AUDIO_DESTINATION), 0);

				// Set dialog icon

				SendMessage(hDlg, WM_SETICON, FALSE,
					(LPARAM)LoadImage((HINSTANCE)GetModuleHandle(NULL),
					(LPCWSTR)101, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));

				SendMessage(hDlg, WM_SETICON, TRUE,
					(LPARAM)LoadIcon((HINSTANCE)GetModuleHandle(NULL),
					(LPCWSTR)101));

				// Add supported formats for selected device type

				HWND hWndFormatList = GetDlgItem(hDlg, IDC_FORMAT);

				SendMessage(hWndFormatList, CB_RESETCONTENT, 0, 0);

				D3DDISPLAYMODE dmDesktop = {0};
				pData->m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmDesktop);

				int nIndex = -1;
				int nDefaultIndex = -1;

				for(DWORD dwDevFormat = 0;
					dwDevFormat < (sizeof(DW_THUDEVICEFORMATS) / sizeof(DWORD));
					dwDevFormat++)
				{
					bool bFullScreen = false;
					bool bWindowed = false;
			
					if (0 == dwDevFormat)
					{
						if (SUCCEEDED(pData->m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
							pData->m_DialogGraphics.GetHWAccel() == true ?
								D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
							dmDesktop.Format,
							dmDesktop.Format, false)))
							bFullScreen = true;

						if (SUCCEEDED(pData->m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
							pData->m_DialogGraphics.GetHWAccel() == true ?
								D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
							dmDesktop.Format,
							dmDesktop.Format, true)))
							bWindowed = true;
					}
					else
					{
						if (SUCCEEDED(pData->m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
							pData->m_DialogGraphics.GetHWAccel() == true ?
								D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
							(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat],
							(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat], false)))
							bFullScreen = true;

						if (SUCCEEDED(pData->m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
							pData->m_DialogGraphics.GetHWAccel() == true ?
								D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
							(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat],
							dmDesktop.Format, true)))
							bWindowed = true;
					}

					if (true == bFullScreen || true == bWindowed)
					{
						// Add format

						nIndex = int(SendMessage(hWndFormatList, CB_ADDSTRING, 0,
							(LPARAM)SZ_THUDEVICEFORMATS[dwDevFormat]));

						// Encode format style in extra data

						BYTE byExtraData[4] = {(BYTE)bFullScreen,
							(BYTE)bWindowed, (BYTE)dwDevFormat, 0};

						SendMessage(hWndFormatList, CB_SETITEMDATA,
							nIndex, *(int*)&byExtraData);

						// See if this should be the default index

						if (pData->m_nDevFormat > Graphics::FORMAT_DESKTOP)
						{
							if (dwDevFormat == DWORD(pData->m_nDevFormat))
								nDefaultIndex = nIndex;
						}
						else
						{
							if (dwDevFormat == Graphics::FORMAT_DESKTOP)
								nDefaultIndex = nIndex;
						}
					}
				}

				int nFormatSel = 0;

				if (nIndex != -1)
				{
					nFormatSel = nDefaultIndex == -1 ? 0 : nDefaultIndex;

					SendMessage(hWndFormatList, CB_SETCURSEL, nFormatSel, 0);

					SendMessage(hDlg, WM_COMMAND,
						MAKEWPARAM(IDC_FORMAT, CBN_SELCHANGE), 0);
				}
			}

			SetForegroundWindow(hDlg);

			return TRUE;
		}
		break;
	case WM_HSCROLL:
		{
			if (LOWORD(wParam) == SB_THUMBPOSITION ||
				LOWORD(wParam) == SB_THUMBTRACK)
			{
				WCHAR szPos[8] = {0};

				StringCbPrintfW(szPos, 16, L"%d%%", HIWORD(wParam));

				if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_SLIDER_EFFECTSVOLUME))
				{
					SetDlgItemText(hDlg, IDC_STATIC_EFFECTSVOLUME, szPos);
				}
				else if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_SLIDER_SPEECHVOLUME))
				{
					SetDlgItemText(hDlg, IDC_STATIC_SPEECHVOLUME, szPos);
				}
				else if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_SLIDER_MUSICVOLUME))
				{
					SetDlgItemText(hDlg, IDC_STATIC_MUSICVOLUME, szPos);
				}
			}
		}
		break;
	case WM_NOTIFY:
		{
			switch(wParam)
			{
			case IDC_SPIN_FORMAT:
				{
					// Format spinned

					LPNMUPDOWN pData = (LPNMUPDOWN)lParam;

					int nFormat = int(SendDlgItemMessage(hDlg, IDC_FORMAT, CB_GETCURSEL, 0, 0));
					int nCount = int(SendDlgItemMessage(hDlg, IDC_FORMAT, CB_GETCOUNT, 0, 0));

					if (pData->iDelta < 0 && !nFormat)
					{
						nFormat = nCount - 1;
					}
					else if (pData->iDelta > 0 && nFormat == (nCount - 1))
					{
						nFormat = 0;
					}
					else
					{
						nFormat += pData->iDelta;
					}

					SendDlgItemMessage(hDlg, IDC_FORMAT, CB_SETCURSEL, nFormat, 0);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_FORMAT, CBN_SELCHANGE), 0);
				}
				break;
			case IDC_SPIN_RESOLUTION:
				{
					// Resolution spinned

					LPNMUPDOWN pData = (LPNMUPDOWN)lParam;

					int nItem = int(SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETCURSEL, 0, 0));
					int nCount = int(SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_GETCOUNT, 0, 0));

					if (pData->iDelta < 0 && 0 == nItem)
					{
						nItem = nCount - 1;
					}
					else if (pData->iDelta > 0 && nItem == (nCount - 1))
					{
						nItem = 0;
					}
					else
					{
						nItem += pData->iDelta;
					}

					SendDlgItemMessage(hDlg, IDC_RESOLUTION, CB_SETCURSEL, nItem, 0);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_RESOLUTION, CBN_SELCHANGE), 0);
				}
				break;
			case IDC_SPIN_REFRESH:
				{
					// Refresh spinned

					LPNMUPDOWN pData = (LPNMUPDOWN)lParam;

					int nItem = int(SendDlgItemMessage(hDlg, IDC_REFRESH, CB_GETCURSEL, 0, 0));
					int nCount = int(SendDlgItemMessage(hDlg, IDC_REFRESH, CB_GETCOUNT, 0, 0));

					if (pData->iDelta < 0 && 0 == nItem)
					{
						nItem = nCount - 1;
					}
					else if (pData->iDelta > 0 && nItem == (nCount - 1))
					{
						nItem = 0;
					}
					else
					{
						nItem += pData->iDelta;
					}

					SendDlgItemMessage(hDlg, IDC_REFRESH, CB_SETCURSEL, nItem, 0);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_REFRESH, CBN_SELCHANGE), 0);
				}
				break;
			case IDC_SPIN_ANTIALIAS:
				{
					// Anti-alias spinned

					LPNMUPDOWN pData = (LPNMUPDOWN)lParam;

					int nItem = int(SendDlgItemMessage(hDlg, IDC_ANTIALIAS, CB_GETCURSEL, 0, 0));
					int nCount = int(SendDlgItemMessage(hDlg, IDC_ANTIALIAS, CB_GETCOUNT, 0, 0));

					if (pData->iDelta < 0 && 0 == nItem)
					{
						nItem = nCount - 1;
					}
					else if (pData->iDelta > 0 && nItem == (nCount - 1))
					{
						nItem = 0;
					}
					else
					{
						nItem += pData->iDelta;
					}

					SendDlgItemMessage(hDlg, IDC_ANTIALIAS, CB_SETCURSEL, nItem, 0);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_ANTIALIAS, CBN_SELCHANGE), 0);
				}
				break;
			case IDC_SPIN_AAQ:
				{
					// Anti-alias spinned

					LPNMUPDOWN pData = (LPNMUPDOWN)lParam;

					int nItem = int(SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY, CB_GETCURSEL, 0, 0));
					int nCount = int(SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY, CB_GETCOUNT, 0, 0));

					if (pData->iDelta < 0 && 0 == nItem)
					{
						nItem = nCount - 1;
					}
					else if (pData->iDelta > 0 && nItem == (nCount - 1))
					{
						nItem = 0;
					}
					else
					{
						nItem += pData->iDelta;
					}

					SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY, CB_SETCURSEL, nItem, 0);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_ANTIALIASQUALITY, CBN_SELCHANGE), 0);
				}
				break;
			}
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_REFRESH:
				{
					if (HIWORD(wParam) != CBN_SELCHANGE) break;

					pData->m_dwRefresh = DWORD(SendDlgItemMessage(hDlg,
						IDC_REFRESH, CB_GETITEMDATA,
						SendDlgItemMessage(hDlg, IDC_REFRESH, CB_GETCURSEL, 0, 0), 0));
				}
				break;
			case IDC_RESOLUTION:
				{
					if (HIWORD(wParam) != CBN_SELCHANGE) break;

					// Get index of currently selected resolution

					int nCurSel = int(SendDlgItemMessage(hDlg,
						IDC_RESOLUTION, CB_GETCURSEL, 0, 0));

					// If no resolution selected, exit immediately

					if (-1 == nCurSel) break;

					// Clear refresh combo box

					HWND hWndRefresh = GetDlgItem(hDlg, IDC_REFRESH);

					SendMessage(hWndRefresh, CB_RESETCONTENT, 0, 0);

					// Get selected format		

					D3DDISPLAYMODE dmDesktop = {0};

					pData->m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
						&dmDesktop);

					int nFormatID = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETCURSEL, 0, 0));

					int nFormatStyle = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETITEMDATA, nFormatID, 0));

					BYTE* pbFormatStyle = (BYTE*)&nFormatStyle;

					D3DFORMAT nFormat = nFormatID ?
						(D3DFORMAT)DW_THUDEVICEFORMATS[(int)pbFormatStyle[2]] :
						dmDesktop.Format;

					// Get selected resolution width and height by decoding extra data

					int nExtraData = int(SendDlgItemMessage(hDlg, IDC_RESOLUTION,
						CB_GETITEMDATA, nCurSel, 0));

					DWORD dwWidth = (DWORD)LOWORD(nExtraData);
					DWORD dwHeight = (DWORD)HIWORD(nExtraData);

					// List refresh rates for selected format and resolution

					DWORD dwModeCount =
						pData->m_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, nFormat);

					D3DDISPLAYMODE dm = {0};

					String str;

					int nDefaultRefresh = -1;

					int nIndex = -1;

					bool bDesktopOnly = (SendDlgItemMessage(hDlg, IDC_FULLSCREEN,
						BM_GETCHECK, 0, 0) == BST_UNCHECKED);

					if (true == bDesktopOnly)
					{
						nIndex = int(SendMessage(hWndRefresh,
							CB_ADDSTRING, 0, (LPARAM)L"Desktop"));

						SendMessage(hWndRefresh, CB_SETITEMDATA,
							nIndex, (LPARAM)dmDesktop.RefreshRate);
					}

					for(DWORD dwMode = 0; dwMode < dwModeCount; dwMode++)
					{
						pData->m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,
							nFormat, dwMode, &dm);

						if (dm.Width == dwWidth && dm.Height == dwHeight)
						{
							str.Format(L"%d Hz", dm.RefreshRate);

							nIndex = int(SendMessage(hWndRefresh, CB_ADDSTRING,
								0, (LPARAM)str.GetBufferConst()));

							// Set extra data to refresh rate

							SendMessage(hWndRefresh, CB_SETITEMDATA,
								nIndex, (LPARAM)dm.RefreshRate);

							// See if this should be default

							if (dm.RefreshRate == pData->m_dwRefresh)
								nDefaultRefresh = nIndex;
						}
					}

					// Set default refresh

					if (true == bDesktopOnly || nDefaultRefresh == -1)
					{
						SendMessage(hWndRefresh, CB_SETCURSEL, 0, 0);
					}
					else
					{
						SendMessage(hWndRefresh, CB_SETCURSEL,
							nDefaultRefresh, 0);
					}

					// Add supported anti-alias levels for resolution

					HWND hWndAAList = GetDlgItem(hDlg, IDC_ANTIALIAS);

					SendMessage(hWndAAList, CB_RESETCONTENT, 0, 0);

					SendMessage(hWndAAList, CB_ADDSTRING, 0, (LPARAM)L"No Anti-Alias");
					SendMessage(hWndAAList, CB_SETITEMDATA, 0, 0);

					WCHAR szItem[64] = {0};

					for(int nAALevel = D3DMULTISAMPLE_2_SAMPLES, nItemID = 1;
						nAALevel < D3DMULTISAMPLE_16_SAMPLES + 1;
						nAALevel++)
					{
						if (SUCCEEDED(
							pData->m_pD3D->CheckDeviceMultiSampleType(
								D3DADAPTER_DEFAULT,
								pData->m_DialogGraphics.GetHWAccel() == true ?
										D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
								nFormat,
								bDesktopOnly,
								(D3DMULTISAMPLE_TYPE)nAALevel,
								NULL)))
						{
							swprintf_s(szItem, L"%dx MSAA", nAALevel);

							SendMessage(hWndAAList, CB_ADDSTRING, 0, (LPARAM)szItem);
							SendMessage(hWndAAList, CB_SETITEMDATA, nItemID, nAALevel);

							if (pData->m_nMultiSampleType == nAALevel)
								SendMessage(hWndAAList, CB_SETCURSEL, nItemID, 0);

							nItemID++;
						}							
					}

					if (pData->m_nMultiSampleType == 0)
						SendMessage(hWndAAList, CB_SETCURSEL, 0, 0);

					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_ANTIALIAS, CBN_SELCHANGE), 0);

					pData->m_dwResWidth = dwWidth;
					pData->m_dwResHeight = dwHeight;
				}
				break;
			case IDC_FORMAT:
				{
					if (HIWORD(wParam) != CBN_SELCHANGE)
						break;

					// Clear resolution combo box

					HWND hWndResolution = GetDlgItem(hDlg, IDC_RESOLUTION);

					SendMessage(hWndResolution, CB_RESETCONTENT, 0, 0);

					// Get selected format						

					D3DDISPLAYMODE dmDesktop = {0};

					pData->m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
						&dmDesktop);

					int nFormatID = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETCURSEL, 0, 0));

					// Decode format style from extra data

					int nFormatStyle = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETITEMDATA, nFormatID, 0));

					BYTE* pbFormatStyle = (BYTE*)&nFormatStyle;

					bool bFullScreen = pbFormatStyle[0] ? true : false;
					bool bWindowed = pbFormatStyle[1] ? true : false;

					// List all resolutions with selected format

					D3DFORMAT nFormat = nFormatID ?
						(D3DFORMAT)DW_THUDEVICEFORMATS[int(pbFormatStyle[2])] :
						dmDesktop.Format;

					DWORD dwModeCount = pData->m_pD3D->GetAdapterModeCount(
						D3DADAPTER_DEFAULT, nFormat);						

					int nDefaultRes = -1;
					D3DDISPLAYMODE dm = {0};

					DWORD dwLastWidth = 0;
					DWORD dwLastHeight = 0;

					String str;

					// Add adapter mode for desktop

					for(DWORD dw = 0; dw < dwModeCount; dw++)
					{
						pData->m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,
							nFormat, dw, &dm);

						if (dm.Width != dwLastWidth || dm.Height != dwLastHeight)
						{
							// Adapter modes repeat for each refresh rate, so filter out duplicates

							dwLastWidth = dm.Width;
							dwLastHeight = dm.Height;

							str.Format(L"%dx%d", dm.Width, dm.Height);

							// Wide is considered something other than a 4:3 ratio

							if (abs(float(dm.Width) / float(dm.Height) - 4.0f / 3.0f) >= 0.001f)
								str += L" (wide screen)";

							// Indicate desktop resolution

							if (dm.Width == dmDesktop.Width && dm.Height == dmDesktop.Height &&
							   dm.RefreshRate == dmDesktop.RefreshRate)
								str += L" *";

							// Encode resolution width and height in extra data

							int nIndex = int(SendMessage(hWndResolution, CB_ADDSTRING, 0,
								(LPARAM)str.GetBufferConst()));

							SendMessage(hWndResolution, CB_SETITEMDATA, nIndex,
								MAKELPARAM((WORD)dm.Width, (WORD)dm.Height));

							// See if this should be default

							if (nDefaultRes == -1 && dm.Height == pData->m_dwResHeight &&
								dm.Width == pData->m_dwResWidth)
								nDefaultRes = nIndex;
						}
					}

					// Update full screen checkbox

					HWND hWndFullScreen = GetDlgItem(hDlg, IDC_FULLSCREEN);

					if (true == bFullScreen && false == bWindowed)
					{
						// Set full screen checkbox and disable it

						SendMessage(hWndFullScreen, BM_SETCHECK, BST_CHECKED, 0);

						EnableWindow(hWndFullScreen, false);
					}
					else if (false == bFullScreen && true == bWindowed)
					{
						// Clear full screen checkbox and disable it

						SendMessage(hWndFullScreen, BM_SETCHECK, BST_UNCHECKED, 0);

						EnableWindow(hWndFullScreen, false);
					}
					else
					{
						// Enable full screen checkbox

						EnableWindow(hWndFullScreen, true);
					}

					// Select item with default index

					if (nDefaultRes != -1)
					{
						SendMessage(hWndResolution, CB_SETCURSEL, nDefaultRes, 0);

						// Send notification to update refresh rates

						SendMessage(hDlg, WM_COMMAND,
							MAKEWPARAM(IDC_RESOLUTION, CBN_SELCHANGE), 0);
					}

					pData->m_nDevFormat = (Graphics::DeviceFormats)int(pbFormatStyle[2]);
				}
				break;
			case IDC_ANTIALIAS:
				{
					// List quality stops for selected anti-alias level

					if (HIWORD(wParam) != CBN_SELCHANGE)
						break;					

					// Get selected format						

					D3DDISPLAYMODE dmDesktop = {0};

					pData->m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,
						&dmDesktop);

					int nFormatID = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETCURSEL, 0, 0));

					// Decode format style from extra data

					int nFormatStyle = int(SendDlgItemMessage(hDlg, IDC_FORMAT,
						CB_GETITEMDATA, nFormatID, 0));

					BYTE* pbFormatStyle = (BYTE*)&nFormatStyle;

					// List all resolutions with selected format

					D3DFORMAT nFormat = nFormatID ?
						(D3DFORMAT)DW_THUDEVICEFORMATS[int(pbFormatStyle[2])] :
						dmDesktop.Format;

					// Get windowed status

					bool bWindowed = (SendDlgItemMessage(hDlg, IDC_FULLSCREEN,
						BM_GETCHECK, 0, 0) == BST_UNCHECKED);

					// Get selected anti-alias level

					int nAntiAliasID =
						SendDlgItemMessage(hDlg, IDC_ANTIALIAS, CB_GETCURSEL, 0, 0);

					int nAntiAliasLevel =
						(-1 == nAntiAliasID) ? 0 :
						SendDlgItemMessage(hDlg, IDC_ANTIALIAS, CB_GETITEMDATA,
							nAntiAliasID, 0);

					// Get number of quality stops

					SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY, CB_RESETCONTENT, 0, 0);

					DWORD dwQualityLevels = 0;

					if (SUCCEEDED(pData->m_pD3D->CheckDeviceMultiSampleType(
						D3DADAPTER_DEFAULT,
						pData->m_DialogGraphics.GetHWAccel() == true ?
										D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
						nFormat,
						bWindowed,
						(D3DMULTISAMPLE_TYPE)nAntiAliasLevel,
						&dwQualityLevels)))
					{
						WCHAR szItem[8] = {0};

						for(DWORD n = 0; n < dwQualityLevels; n++)
						{
							swprintf_s(szItem, L"Level %d", n);

							SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY,
								CB_ADDSTRING, 0, (LPARAM)szItem);

							if (n == pData->m_dwMultiSampleQuality)
								SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY,
								CB_SETCURSEL, n, 0);
						}
					}

					pData->m_nMultiSampleType = nAntiAliasLevel;

					EnableWindow(GetDlgItem(hDlg, IDC_ANTIALIASQUALITY), nAntiAliasLevel > 0);
				}
				break;
			case IDC_ANTIALIASQUALITY:
				{
					if (HIWORD(wParam) != BN_CLICKED) break;

					int nQualityID = SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY,
						CB_GETCURSEL, 0, 0);

					if (nQualityID == CB_ERR)
						pData->m_dwMultiSampleQuality = 0;
					else
						pData->m_dwMultiSampleQuality = DWORD(
							SendDlgItemMessage(hDlg, IDC_ANTIALIASQUALITY,
							CB_GETITEMDATA, nQualityID, 0));
				}
				break;
			case IDC_FULLSCREEN:
				{
					if (HIWORD(wParam) != BN_CLICKED) break;

					// If full screen not checked, disable refresh

					SendMessage(hDlg, WM_COMMAND,
						MAKEWPARAM(IDC_RESOLUTION, CBN_SELCHANGE), 0);

					if (SendDlgItemMessage(hDlg, IDC_FULLSCREEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						EnableWindow(GetDlgItem(hDlg, IDC_REFRESH), true);
						EnableWindow(GetDlgItem(hDlg, IDC_SPIN_REFRESH), true);
					}
					else
					{
						EnableWindow(GetDlgItem(hDlg, IDC_REFRESH), false);
						EnableWindow(GetDlgItem(hDlg, IDC_SPIN_REFRESH), false);
					}
				}
				break;
			case IDC_VIDEO_ADVANCED:
				{
					// Display advanced video

					if (pData->m_DialogGraphics.Show(hDlg, pData->m_pD3D) == IDOK)
					{
						// Refresh

						SendMessage(hDlg, WM_INITDIALOG, 0, 0);
					}
				}
				break;
			case IDC_AUDIO_ADVANCED:
				{
					// Display advanced audio

					pData->m_dialogAudio.Show(hDlg);
				}
				break;
			case IDC_STARTUP_ADVANCED:
				{
					// Display advanced startup

					pData->m_dialogStartup.Show(hDlg);
				}
				break;
			case IDSAVELAUNCH:
			case IDSAVEEXIT:
				{
					// Save Settings

					pData->m_pGame->SetHardwareAcceleration(pData->m_DialogGraphics.GetHWAccel());
					pData->m_pGame->SetDeviceFormat(pData->m_nDevFormat);
					pData->m_pGame->SetDisplayWidth((int)pData->m_dwResWidth);
					pData->m_pGame->SetDisplayHeight((int)pData->m_dwResHeight);
					pData->m_pGame->SetMultiSampleType((D3DMULTISAMPLE_TYPE)pData->m_nMultiSampleType);
					pData->m_pGame->SetMultiSampleQuality(pData->m_dwMultiSampleQuality);
					pData->m_pGame->SetFullScreen(SendDlgItemMessage(hDlg,
						IDC_FULLSCREEN, BM_GETCHECK, 0, 0) == BST_CHECKED);

					if (pData->m_pGame->GetFullScreen() == true)
					{
						pData->m_pGame->SetRefreshRate(DWORD(
							SendDlgItemMessage(hDlg, IDC_REFRESH, CB_GETITEMDATA,
								SendDlgItemMessage(hDlg, IDC_REFRESH, CB_GETCURSEL, 0, 0), 0)));
					}

					pData->m_pGame->SetPureDevice(pData->m_DialogGraphics.GetPure());
					pData->m_pGame->SetSoftwareVertexProcessing(pData->m_DialogGraphics.GetSWVP());
					pData->m_pGame->SetShaderDebug(pData->m_DialogGraphics.GetShaderDebug());
					pData->m_pGame->SetPureDevice(pData->m_DialogGraphics.GetPure());
					pData->m_pGame->SetVSync(SendDlgItemMessage(hDlg, IDC_VSYNC,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_pGame->GetEngine().SetOption(
						Engine::OPTION_EXCLUSIVE_SOUND,
						pData->m_dialogAudio.GetExclusive());

					pData->m_pGame->GetEngine().SetOption(
						Engine::OPTION_DISABLE_SOUNDS,
						pData->m_dialogAudio.GetDisableSound());

					pData->m_pGame->GetEngine().SetOption(
						Engine::OPTION_DISABLE_MUSIC,
						pData->m_dialogAudio.GetDisableMusic());

					pData->m_pGame->GetEngine().SetOption(
						Engine::OPTION_AUDIO_DESTINATION,
						SendDlgItemMessage(hDlg, IDC_AUDIODEST, CB_GETCURSEL, 0, 0));

					pData->m_pGame->SetEffectsVolume(float(SendDlgItemMessage(hDlg,
						IDC_SLIDER_EFFECTSVOLUME, TBM_GETPOS, 0, 0)) / 100.0f);

					pData->m_pGame->SetSpeechVolume(float(SendDlgItemMessage(hDlg,
						IDC_SLIDER_SPEECHVOLUME, TBM_GETPOS, 0, 0)) / 100.0f);

					pData->m_pGame->SetMusicVolume(float(SendDlgItemMessage(hDlg,
						IDC_SLIDER_MUSICVOLUME, TBM_GETPOS, 0, 0)) / 100.0f);

					pData->m_pGame->SetLogMode(pData->m_dialogStartup.GetLogMode());

					pData->m_pGame->SetShowConfigure(SendDlgItemMessage(hDlg, IDC_SHOWCONFIGURE,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					pData->m_pGame->SetLogFilePath(
						pData->m_dialogStartup.GetLogFilePath());

					pData->m_pGame->SetScriptPath(
						pData->m_dialogStartup.GetScriptPath());

					pData->m_pGame->SetControlsProfilePath(
						pData->m_dialogStartup.GetControlsPath());

					EndDialog(hDlg, LOWORD(wParam));
				}
				break;
			case IDDISCARDLAUNCH:
			case IDCANCEL:
				{
					// Discard

					EndDialog(hDlg, LOWORD(wParam));
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			// Discard

			EndDialog(hDlg, IDCANCEL);
		}
		break;
	case WM_DESTROY:
		{
			// Cleanup

			pData->m_pD3D->Release();
		}
		break;
	}

	return false;
}

DialogError::DialogError(void): m_hErrorIcon(NULL),
								m_pszText(NULL),
								m_bNotAgain(false)
{
}

DialogError::~DialogError(void)
{
}

bool DialogError::GetNotAgain(void) const
{
	return m_bNotAgain;
}

int DialogError::Show(HWND hWndParent, LPCWSTR pszText)
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	if (NULL == m_hErrorIcon)
		m_hErrorIcon = (HICON)LoadImage(hInstance,
		(LPCWSTR)IDI_HITMAN2DERROR, IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR);

	m_pszText = pszText;

	// Make sure cursor is displayed

	EnableWindow(hWndParent, FALSE);

	SetCursor(LoadCursor(hInstance, (LPCWSTR)IDC_ARROW));

	while(ShowCursor(true) == 0);

	// Show the dialog

	return int(DialogBoxParam((HINSTANCE)GetModuleHandle(NULL),
		(LPCWSTR)IDD_ERROR, hWndParent, DlgProc, (LPARAM)this));
}

INT_PTR CALLBACK DialogError::DlgProc(HWND hDlg,
									  UINT uMsg,
									  WPARAM wParam,
									  LPARAM lParam)
{
	DialogError* pData =
		(DialogError*)GetWindowLongPtr(hDlg, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			_ASSERT(lParam);

			SetWindowLongPtr(hDlg, GWL_USERDATA, lParam);

			pData = (DialogError*)lParam;

			SetWindowText(GetDlgItem(hDlg, IDC_TEXT),
				pData->m_pszText);

			// Resize the dialog down if required			

			HDC hDC = GetDC(hDlg);

			HGDIOBJ hOldFont = SelectObject(hDC,
				GetStockObject(DEFAULT_GUI_FONT));		

			TEXTMETRIC tm;
			GetTextMetrics(hDC, &tm);			

			LPCWSTR pszLine = pData->m_pszText;

			int nMaxWidth = 0;
			int nHeight = 0;

			for(;;)
			{
				LPCWSTR pszEndLine = wcschr(pszLine, L'\n');

				if (NULL == pszEndLine)
					pszEndLine = pszLine + wcslen(pszLine);

				SIZE size;

				GetTextExtentPoint32(hDC, pszLine,
					int(pszEndLine - pszLine), &size);

				if (nMaxWidth < size.cx)
					nMaxWidth = size.cx;

				nHeight += size.cy;

				if (L'\0' == *pszEndLine)
					break;

				pszLine = pszEndLine + 1;
			}

			SelectObject(hDC, hOldFont);

			ReleaseDC(hDlg, hDC);

			HWND hWndBox = GetDlgItem(hDlg, IDC_TEXT);

			RECT rcBox;
			GetClientRect(hWndBox, &rcBox);

			RECT rcWindow;
			GetWindowRect(hDlg, &rcWindow);

			// Resize error description box

			nMaxWidth += GetSystemMetrics(SM_CXEDGE) * 10;
			nHeight += GetSystemMetrics(SM_CYEDGE) * 10;

			RECT rcWindowBox;
			GetWindowRect(hWndBox, &rcWindowBox);
			ScreenToClient(hDlg, (LPPOINT)&rcWindowBox.left);
			ScreenToClient(hDlg, (LPPOINT)&rcWindowBox.right);

			SetWindowPos(hWndBox, NULL, 0, 0, nMaxWidth, nHeight,
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

			int nHeightDifference = nHeight - rcBox.bottom;

			// Re-position the check box

			HWND hWndCheckBox = GetDlgItem(hDlg, IDC_NOTAGAIN);

			RECT rcCheckBox;

			GetWindowRect(hWndCheckBox, &rcCheckBox);
			ScreenToClient(hDlg, (LPPOINT)&rcCheckBox);
			ScreenToClient(hDlg, (LPPOINT)&rcCheckBox.right);

			SetWindowPos(hWndCheckBox, 0, rcCheckBox.left,
				rcCheckBox.top + nHeightDifference, 0, 0,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

			// Resize and reposition error dialog

			int nNewWidth = nMaxWidth + rcWindowBox.left +
				GetSystemMetrics(SM_CXEDGE) * 10;

			if (nNewWidth < rcCheckBox.right)
				nNewWidth = rcCheckBox.right;

			int nNewHeight = rcWindow.bottom - rcWindow.top +
				nHeight - (rcBox.bottom - rcBox.top);

			RECT rcDesktop;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, false);				

			MoveWindow(hDlg,
				((rcDesktop.right - rcDesktop.left) - nNewWidth) / 2,
				((rcDesktop.bottom - rcDesktop.top) - nNewHeight) / 2,
				nNewWidth, nNewHeight, false);

			// Re-position the buttons

			HWND hWndOK = GetDlgItem(hDlg, IDOK);
			HWND hWndCopy = GetDlgItem(hDlg, IDCOPY);

			RECT rcOK, rcCopy;

			GetWindowRect(hWndOK, &rcOK);
			GetWindowRect(hWndCopy, &rcCopy);

			int nWidthOK = rcOK.right - rcOK.left;
			int nWidthCopy = rcCopy.right - rcCopy.left;
			int nSpaceBetween = rcCopy.left - rcOK.right;
			int nDistBetween = rcCopy.left - rcOK.left;
			int nTotalWidth = nWidthOK + nWidthCopy + nSpaceBetween;

			ScreenToClient(hDlg, (LPPOINT)&rcOK);
			ScreenToClient(hDlg, (LPPOINT)&rcCopy);

			rcOK.left = (nNewWidth - nTotalWidth) / 2;
			rcCopy.left = rcOK.left + nDistBetween;

			rcOK.top = rcCopy.top =
				rcCheckBox.bottom + GetSystemMetrics(SM_CYEDGE) * 4;

			SetWindowPos(hWndOK, 0, rcOK.left, rcOK.top +
				nHeightDifference, 0, 0,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

			SetWindowPos(hWndCopy, 0, rcCopy.left, rcCopy.top +
				nHeightDifference, 0, 0,
				SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

			return true;
		}
		break;
	case WM_CLOSE:
		{
			pData->m_bNotAgain =
				(SendDlgItemMessage(hDlg, IDC_NOTAGAIN,
				BM_GETCHECK, 0, 0) == BST_CHECKED);

			EndDialog(hDlg, IDOK);
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					pData->m_bNotAgain = 
						(SendDlgItemMessage(hDlg, IDC_NOTAGAIN,
						BM_GETCHECK, 0, 0) == BST_CHECKED);

					EndDialog(hDlg, IDOK);
				}
				break;
			case IDCOPY:
				{
					SendDlgItemMessage(hDlg, IDC_TEXT, EM_SETSEL, 0, -1);
					SendDlgItemMessage(hDlg, IDC_TEXT, WM_COPY, 0, 0);
				}
				break;
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {0};
			HDC hDC = BeginPaint(hDlg, &ps);

			DrawIconEx(hDC, 12, 12, pData->m_hErrorIcon, 48, 48,
				0, NULL, DI_IMAGE | DI_MASK);

			EndPaint(hDlg, &ps);
		}
		break;
	}

	return false;
}