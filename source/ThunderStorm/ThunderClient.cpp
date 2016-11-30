/*------------------------------------------------------------------*\
|
| ThunderClient.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm client class implementation
| Created: 03/31/06
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderClient.h"		// defining Client
#include "ThunderStringTable.h" // using StringTable
#include "ThunderSprite.h"		// using Sprite
#include "ThunderRegion.h"		// using RegionSet
#include "ThunderSound.h"		// using Sound
#include "ThunderMusic.h"		// using Music
#include "ThunderVideo.h"		// using Video
#include "ThunderInfoFile.h"	// using InfoFile
#include "ThunderIniFile.h"		// using IniFile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Profile sections and keys

const WCHAR SZ_PROFILE_SECTION_VIDEO[]		= L"video";
const WCHAR SZ_PROFILE_KEY_HWACCEL[]		= L"hwaccel";
const WCHAR SZ_PROFILE_KEY_RESOLUTION[]		= L"resolution";
const WCHAR SZ_PROFILE_KEY_DEVFORMAT[]		= L"format";
const WCHAR SZ_PROFILE_KEY_MSAA[]			= L"antialias";
const WCHAR SZ_PROFILE_KEY_MSAAQUALITY[]	= L"aaquality";
const WCHAR SZ_PROFILE_KEY_REFRESH[]		= L"refresh";
const WCHAR SZ_PROFILE_KEY_FULLSCREEN[]		= L"fullscreen";
const WCHAR SZ_PROFILE_KEY_VSYNC[]			= L"vsync";
const WCHAR SZ_PROFILE_KEY_SVP[]			= L"swvp";
const WCHAR SZ_PROFILE_KEY_SHD[]			= L"shaderdebug";
const WCHAR SZ_PROFILE_KEY_PUREDEVICE[]		= L"puredevice";
const WCHAR SZ_PROFILE_KEY_BATCHPRIMCOUNT[] = L"maxbatchprim";
const WCHAR SZ_PROFILE_SECTION_AUDIO[]		= L"audio";
const WCHAR SZ_PROFILE_KEY_EXCLUSIVESOUND[] = L"exclusivesound";
const WCHAR SZ_PROFILE_KEY_DISABLESOUND[]	= L"disablesounds";
const WCHAR SZ_PROFILE_KEY_DISABLEMUSIC[]	= L"disablemusic";
const WCHAR SZ_PROFILE_KEY_AUDIODEST[]		= L"audiodestination";
const WCHAR SZ_PROFILE_SECTION_STARTUP[]	= L"startup";
const WCHAR SZ_PROFILE_KEY_CMDSHOW[]		= L"showcommand";
const WCHAR SZ_PROFILE_KEY_CONTROLS[]		= L"controlspath";
const WCHAR	SZ_PROFILE_KEY_LOGFILE[]		= L"logfilepath";
const WCHAR SZ_PROFILE_KEY_LOGMODE[]		= L"logmode";
const WCHAR SZ_PROFILE_KEY_UITHEME[]		= L"themepath";
const WCHAR SZ_PROFILE_KEY_BASEDIR[]		= L"basedir";

const WCHAR SZ_BASE_PATH_TEXTURES[]			= L"textures";
const WCHAR SZ_BASE_EXT_TEXTURES[]			= L".png";
const WCHAR SZ_BASE_EXT_TEXTURES_EX[]		= L".dds";

const WCHAR SZ_BASE_PATH_MATERIALS[]		= L"materials";
const WCHAR SZ_BASE_EXT_MATERIALS[]			= L".thl";

const WCHAR SZ_BASE_PATH_ANIMATIONS[]		= L"animations";
const WCHAR SZ_BASE_EXT_ANIMATIONS[]		= L".tha";

const WCHAR SZ_BASE_PATH_REGIONS[]			= L"textures";
const WCHAR SZ_BASE_EXT_REGIONS[]			= L".thr";

const WCHAR SZ_BASE_PATH_EFFECTS[]			= L"effects";
const WCHAR SZ_BASE_EXT_EFFECTS[]			= L".fx";

const WCHAR SZ_BASE_PATH_SPRITES[]			= L"sprites";
const WCHAR SZ_BASE_EXT_SPRITES[]			= L".ths";

const WCHAR SZ_BASE_PATH_SCREENS[]			= L"screens";
const WCHAR SZ_BASE_EXT_SCREENS[]			= L".thn";

const WCHAR SZ_BASE_PATH_FONTS[]			= L"fonts";
const WCHAR SZ_BASE_EXT_FONTS[]				= L".thf";

const WCHAR SZ_BASE_PATH_MAPS[]				= L"maps";
const WCHAR SZ_BASE_EXT_MAPS[]				= L".thm";

const WCHAR SZ_BASE_PATH_SCRIPTS[]			= L"scripts";
const WCHAR SZ_BASE_EXT_SCRIPTS[]			= L".thc";

const WCHAR SZ_BASE_PATH_STRINGS[]			= L"strings";
const WCHAR SZ_BASE_EXT_STRINGS[]			= L".tht";

const WCHAR SZ_BASE_PATH_VIDEOS[]			= L"videos";
const WCHAR SZ_BASE_EXT_VIDEOS[]			= L".mpg";

const WCHAR SZ_BASE_PATH_MUSIC[]			= L"music";
const WCHAR SZ_BASE_EXT_MUSIC[]				= L".mp3";

const WCHAR SZ_BASE_PATH_SOUNDS[]			= L"sounds";
const WCHAR SZ_BASE_EXT_SOUNDS[]			= L".wav";

const WCHAR SZ_BASE_PATH_SAVE[]				= L"save";
const WCHAR SZ_BASE_EXT_SAVE[]				= L".thv";

const WCHAR SZ_EXT_LOG[]					= L".log";
const WCHAR SZ_EXT_HTML[]					= L".html";
const WCHAR SZ_EXT_INI[]					= L".ini";

const WCHAR SZ_DEFAULT_CONTROLSPATH[]		= L".\\controls.ini";

// Device formats

const LPCWSTR SZ_DEVICEFORMATS[] =	{
										L"desktop",
										L"X8R8G8B8",
										L"X1R5G5B5",
										L"R5G6B5"
									};

const int N_DEVICEFORMATS[] =		{
										Graphics::FORMAT_DESKTOP,
										Graphics::FORMAT_X8R8G8B8,
										Graphics::FORMAT_X1R5G5B5,
										Graphics::FORMAT_R5G6B5
									};

// Show window flags

const int N_SWFLAGS[] =			{
										SW_HIDE,
										SW_MAXIMIZE,
										SW_MINIMIZE,
										SW_RESTORE,
										SW_SHOW,
										SW_SHOWDEFAULT,
										SW_SHOWMAXIMIZED,
										SW_SHOWMINIMIZED,
										SW_SHOWMINNOACTIVE,
										SW_SHOWNA,
										SW_SHOWNOACTIVATE,
										SW_SHOWNORMAL
									};

const LPCWSTR SZ_SWFLAGS[] =	{
										L"SW_HIDE",
										L"SW_MAXIMIZE",
										L"SW_MINIMIZE",
										L"SW_RESTORE",
										L"SW_SHOW",
										L"SW_SHOWDEFAULT",
										L"SW_SHOWMAXIMIZED",
										L"SW_SHOWMINIMIZED",
										L"SW_SHOWMINNOACTIVE",
										L"SW_SHOWNA",
										L"SW_SHOWNOACTIVATE",
										L"SW_SHOWNORMAL"
									};

// Audio paths

const LPCWSTR SZ_AUDIO_PATHS[] =	{
										L"speakers",
										L"headphones"
									};

// Logging modes

const LPCWSTR SZ_LOGMODES[] =		{
										L"disable",
										L"text",
										L"html"
									};


/*----------------------------------------------------------*\
| Client implementation
\*----------------------------------------------------------*/

Client::Client(DWORD dwIconID,
			   LPCWSTR pszTitle,
			   LPCWSTR pszDescription,
			   int nResolutionWidth,
			   int nResolutionHeight,
			   bool bFullScreen,
			   Graphics::DeviceFormats nDeviceFormat,
			   D3DMULTISAMPLE_TYPE nMultiSampleType,
			   DWORD dwMultiSampleQuality,
			   DWORD dwRefreshRate,
			   bool bVSync,
			   bool bHardwareAcceleration,
			   bool bSoftwareVertexProcessing,
			   bool bShaderDebug,
			   bool bPureDevice,
			   LogMode nLogMode):

			   m_dwIconID(dwIconID),	
			   m_strTitle(pszTitle ? pszTitle : L"ThunderStorm Game"),
			   m_strDescription(pszDescription ?
				pszDescription : L"ThunderStorm Game Boilerplate"),

			   m_nCmdShow(SW_SHOW),

			   m_nResolutionWidth(nResolutionWidth),
			   m_nResolutionHeight(nResolutionHeight),
			   m_bFullScreen(bFullScreen),
			   
			   m_nDeviceFormat(nDeviceFormat),

			   m_nMSAALevel(nMultiSampleType),
			   m_dwMSAAQuality(dwMultiSampleQuality),

			   m_dwRefreshRate(dwRefreshRate),									
			   m_bVSync(bVSync),

			   m_bHardwareAcceleration(bHardwareAcceleration),
			   m_bSoftwareVertexProcessing(bSoftwareVertexProcessing),
			   m_bShaderDebug(bShaderDebug),
			   m_bPureDevice(bPureDevice),
		
			   m_bRunning(false),

			   m_pLogFile(NULL),
			   m_nLogMode(LOG_DISABLE),

			   m_bMute(true)
{
	m_Engine.SetClientInstance(this);

	ZeroMemory(m_nVersion, sizeof(m_nVersion));

	m_strBaseDir.Allocate(MAX_PATH);
	GetCurrentDirectory(MAX_PATH - 1, m_strBaseDir);

	SetLogMode(nLogMode);
}

Client::~Client(void)
{
	Empty();
}

Resource* Client::LoadResource(LPCWSTR pszPath, float fPersistenceTime)
{
	if (String::IsEmpty(pszPath))
		return NULL;

	// Determine resource type based on known extensions

	String strExt = PathFindExtension(pszPath);

	if (strExt.IsEmpty() == true)
		return NULL;

	strExt.ToLower();

	if (strExt == SZ_BASE_EXT_MATERIALS)
	{
		// Load material

		return m_Engine.GetMaterials().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_EFFECTS)
	{
		// Load effect

		return m_Engine.GetMaterials().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_TEXTURES ||
			strExt == SZ_BASE_EXT_TEXTURES_EX)
	{
		// Load texture

		return m_Engine.GetTextures().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_ANIMATIONS)
	{
		// Load animation

		return m_Engine.GetAnimations().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_SPRITES)
	{
		// Load sprite

		return m_Engine.GetSprites().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_STRINGS)
	{
		// Load string table

		return m_Engine.GetStrings().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_REGIONS)
	{
		// Load region set

		return m_Engine.GetRegions().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_SOUNDS)
	{
		// Load sound

		return m_Engine.GetSounds().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_MUSIC)
	{
		// Load music

		return m_Engine.GetMusic().Load(pszPath, fPersistenceTime);
	}
	else if (strExt == SZ_BASE_EXT_VIDEOS)
	{
		// Load video

		return m_Engine.GetVideos().Load(pszPath, fPersistenceTime);
	}

	// Other format not supported by this function

	return NULL;
}

void Client::SetProfilePath(LPCWSTR pszProfilePath)
{
	if (String::IsEmpty(pszProfilePath) == false)
	{
		m_strProfilePath = pszProfilePath;
	}
	else
	{
		// If not specified, set to default (current directory\exetitle.ini)

		m_strProfilePath = L".\\";

		m_strProfilePath += m_strExeTitle;

		m_strProfilePath += SZ_EXT_INI;
	}
}

void Client::SetControlsProfilePath(LPCWSTR pszControlsPath)
{
	if (String::IsEmpty(pszControlsPath) == false)
		m_strControlsProfilePath = pszControlsPath;
	else
		m_strControlsProfilePath = SZ_DEFAULT_CONTROLSPATH;
}

void Client::SetLogFilePath(LPCWSTR pszLogFilePath)
{
	if (String::IsEmpty(pszLogFilePath) == false)
	{
		m_strLogFilePath = pszLogFilePath;
	}
	else
	{
		// If not specified, set to default (current directory\exetitle.log)

		m_strLogFilePath = L".\\";

		m_strLogFilePath += m_strExeTitle;

		if (m_nLogMode == LOG_HTML)
			m_strLogFilePath += SZ_EXT_HTML;
		else
			m_strLogFilePath += SZ_EXT_LOG;
	}
}

void Client::SetThemePath(LPCWSTR pszThemePath)
{
	m_strThemePath = pszThemePath;
}

void Client::Run(LPCWSTR pszCmdLine, int nCmdShow)
{
	// Start running

	bool bInitializedInstance = false;

	m_bRunning = true;

	try
	{
		// Process command line

		m_nCmdShow = nCmdShow;

		ProcessCommandLine(pszCmdLine);

		// Load settings

		DeserializeSettings();

		// Initialize

		InitializeEngine();
		InitializeInstance();

		bInitializedInstance = true;

		// Start game loop

		while(true == m_bRunning)
		{
			DoEvents();

			if (GetActiveWindow() != m_Engine.GetGameWindow())
				OnDeactivate();

			Update();

			Render();
		}

		// Save settings

		SerializeSettings();

		// Destroy

		DestroyInstance();
	}
	
	catch(Error& rError)
	{
		// Handle error

		OnError(rError);

		// Destroy

		if (true == bInitializedInstance)
			DestroyInstance();
	}
}

int Client::DoEvents(void)
{
	int nMessagesProcessed = 0;
	MSG msg;

	while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) Exit();

		nMessagesProcessed++;
	}

	return nMessagesProcessed;
}

void Client::Wait(float fRunTimeSeconds)
{
	try
	{
		float fTargetTime = m_Engine.GetRunTime() + fRunTimeSeconds;

		do 
		{
			if (DoEvents() == 0)
			{
				if (GetActiveWindow() != m_Engine.GetGameWindow())
					OnDeactivate();
				
				Update();		

				Render();
			}

		} while(m_Engine.GetRunTime() < fTargetTime && true == m_bRunning);
	}
	
	catch(Error& rError)
	{
		OnError(rError);
	}
}

void Client::OnDeactivate(void)
{
	Print(L"entering deactivated state.", PRINT_INFO);

	// Pause session, and remember if it was paused

	bool bPaused = m_Engine.IsSessionPaused();

	if (false == bPaused)
		m_Engine.PauseSession(true);

	MSG msg;

	do
	{
		if (GetMessage(&msg, m_Engine.GetGameWindow(), 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	} while(GetActiveWindow() != m_Engine.GetGameWindow());

	// Reset device if it was lost

	m_Engine.GetGraphics().Reset();

	// If session was running, unpause it

	if (false == bPaused)
		m_Engine.PauseSession(false);

	Print(L"exiting deactivated state.", PRINT_INFO);
}

void Client::SetLogMode(LogMode nLogMode)
{
	if (m_pLogFile != NULL)
	{
		// Fire log end event

		OnLogEnd();

		// Destroy old log object

		delete m_pLogFile;
		m_pLogFile = NULL;

		// Delete current log file if switching from text to HTML or back

		if (nLogMode != LOG_DISABLE && m_nLogMode != LOG_DISABLE)
			DeleteFile(m_strLogFilePath);
	}

	// Set new log mode

	m_nLogMode = nLogMode;

	// If muted, don't prepare for logging

	if (true == m_bMute)
		return;

	try
	{
		// Create new log instance

		if (LOG_TEXT == nLogMode)
			m_pLogFile = new LogFile(&m_Engine.GetErrors());
		else if (LOG_HTML == nLogMode)
			m_pLogFile = new LogFileHTML(&m_Engine.GetErrors());
		else
			return;

		if (NULL == m_pLogFile)
			throw m_Engine.GetErrors().Push(Error::MEM_ALLOC, __FUNCTIONW__,
				m_nLogMode == LOG_TEXT ? sizeof(LogFile) : sizeof(LogFileHTML));

		if (m_strLogFilePath.IsEmpty() == true)
		{
			// If no log path specified, set default

			SetLogFilePath(NULL);
		}
		else
		{
			// If specified, make sure correct extension is used

			LPWSTR pszExt = PathFindExtension(m_strLogFilePath);

			if (LOG_TEXT == nLogMode)
			{
				if (wcscmp(pszExt, SZ_EXT_HTML) == 0)
					wcscpy_s(pszExt, 4, SZ_EXT_LOG);
			}
			else if (LOG_HTML == m_nLogMode)
			{
				if (wcscmp(pszExt, SZ_EXT_LOG) == 0)
					wcscpy_s(pszExt, 5, SZ_EXT_HTML);
			}
		}

		// Fire LogStart event to allow client to set logging format properties

		OnLogStart();

		// Make sure current directory is base directory

		PUSH_CURRENT_DIRECTORY(m_strBaseDir);

		// Open or create log file

		m_pLogFile->Open(m_strLogFilePath);

		// Restore current directory

		POP_CURRENT_DIRECTORY();
	}

	catch(Error&)
	{
		throw m_Engine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, m_strLogFilePath);
	}
}

void Client::ProcessCommandLine(LPCWSTR pszCmdLine)
{
	// Get game executable file name

	WCHAR szExePath[MAX_PATH] = {0};
	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);

	// Get game executable version information

	DWORD dwDummy = 0;

	DWORD dwVersionInfoSize =
		GetFileVersionInfoSize(szExePath, &dwDummy);

	LPBYTE pbVersionInfo = NULL;

	if (dwVersionInfoSize > 0)
	{
		try
		{
			pbVersionInfo = new BYTE[dwVersionInfoSize];
		}

		catch(std::bad_alloc e)
		{
			throw m_Engine.GetErrors().Push(Error::MEM_ALLOC,
				__FUNCTIONW__, int(dwVersionInfoSize));
		}
		
		if (GetFileVersionInfo(szExePath, 0, dwVersionInfoSize,
			(LPVOID)pbVersionInfo) == FALSE)
		{
			delete[] pbVersionInfo;

			throw m_Engine.GetErrors().Push(Error::WIN_SYS_GETFILEVERSIONINFO,
				__FUNCTIONW__);
		}

		// Get File Version

		VS_FIXEDFILEINFO* pFixed = NULL;
		VerQueryValue((LPVOID)pbVersionInfo, L"\\", (LPVOID*)&pFixed, NULL);

		WORD* pwFileVersion = (WORD*)&pFixed->dwFileVersionMS;

		m_nVersion[0] = int(pwFileVersion[1]);
		m_nVersion[1] = int(pwFileVersion[0]);
		m_nVersion[2] = int(pwFileVersion[3]);
		m_nVersion[3] = int(pwFileVersion[2]);

		// Get Translation Info

		BYTE* pbTranslation = NULL;

		if (VerQueryValue((LPVOID)pbVersionInfo, L"\\VarFileInfo\\Translation",
			(LPVOID*)&pbTranslation, NULL))
		{
			// Use first language encountered

			WCHAR szLanguagePath[128] = {0};
			wcscpy_s(szLanguagePath, 128, L"\\StringFileInfo\\");

			LPWSTR pszLanguage = szLanguagePath + wcslen(szLanguagePath);
			swprintf_s(pszLanguage + 0, 3, L"%.2x", pbTranslation[1]);
			swprintf_s(pszLanguage + 2, 3, L"%.2x", pbTranslation[0]);
			swprintf_s(pszLanguage + 4, 3, L"%.2x", pbTranslation[3]);
			swprintf_s(pszLanguage + 6, 3, L"%.2x", pbTranslation[2]);

			// Get Description

			LPWSTR pszKey = pszLanguage + wcslen(pszLanguage);
			LPWSTR pszValue = NULL;
			DWORD dwLen = 0;

			wcscpy_s(pszKey, 128 - (pszKey - szLanguagePath),
				L"\\FileDescription");

			VerQueryValue((LPVOID)pbVersionInfo, szLanguagePath,
				(LPVOID*)&pszValue, (PUINT)&dwLen);

			if (dwLen > 0) m_strTitle = pszValue;

			// Get Comments

			wcscpy_s(pszKey, 128 - (pszKey - szLanguagePath),
				L"\\Comments");

			VerQueryValue((LPVOID)pbVersionInfo, szLanguagePath,
				(LPVOID*)&pszValue, (PUINT)&dwLen);

			if (dwLen > 0)
				m_strDescription = pszValue;
		}

		delete[] pbVersionInfo;
	}

	// Get game executable file title minus extension

	LPWSTR pszExeTitle = PathFindFileName(szExePath);

	int nExeTitleLen = int(wcslen(pszExeTitle) - 4);
	m_strExeTitle.Allocate(nExeTitleLen);

	wcsncpy(m_strExeTitle.GetBuffer(), pszExeTitle, nExeTitleLen);
	m_strExeTitle.GetBuffer()[nExeTitleLen] = L'\0';

	// Get game executable directory

	int nExeDirLen = int(pszExeTitle - szExePath);
	m_strExeDir.Allocate(nExeDirLen);

	wcsncpy(m_strExeDir, szExePath, nExeDirLen);
	m_strExeDir.GetBuffer()[nExeDirLen] = L'\0';

	// Get command line string

	LPCWSTR psz = PathGetArgs(pszCmdLine);

	// Start extracting arguments

	String strArg;

	while(String::IsEmpty(psz) == false)
	{
		// Skip white space

		EatWhiteSpace(&psz);

		// Ran out of characters

		if (L'\0' == *psz) break;

		// Mark the start of argument

		LPCWSTR pszArgStart = psz;

		// If this is a quoted argument, skip until next quote

		if (L'\"' == *psz)
		{
			pszArgStart++;

			psz = wcschr(psz + 1, L'\"');

			// If could not find closing quote, bad argument format

			if (NULL == psz) break;
		}

		// Find the end of argument by the start of next one

		psz = wcschr(psz, L' ');

		// Extract argument		

		if (NULL == psz)
		{
			// Last argument on the command line

			strArg = pszArgStart;

			// Make sure not to include the closing quote, if any

			if (L'\"' == strArg[strArg.GetLength() - 1])
				strArg[strArg.GetLength() - 1] = L'\0';
		}
		else
		{
			// Argument in the middle of the command line

			int nArgLen = int(psz - pszArgStart) -
				(L'\"' == psz[-1] ? 1 : 0);

			strArg.Allocate(nArgLen);
			
			wcsncpy(strArg.GetBuffer(), pszArgStart, nArgLen);

			strArg.GetBuffer()[nArgLen] = L'\0';
		}

		// Add extracted argument to argument array

		m_arCommandArgs.push_back(strArg);
	}
}

void Client::DeserializeSettings(void)
{
	IniFile profile(&m_Engine.GetErrors());

	if (m_strProfilePath.IsEmpty())
		SetProfilePath(NULL);

	try
	{
		profile.Deserialize(m_strProfilePath);
	}

	catch(Error& rError)
	{
		// Ignore errors from not being able to load profile

		UNREFERENCED_PARAMETER(rError);

		while(m_Engine.GetErrors().GetCount())
			m_Engine.GetErrors().Pop();
	}

	DeserializeSettings(profile);
}

void Client::DeserializeSettings(const IniFile& rProfile)
{
	//
	// Read video settings
	//

	// Read resolution

	LPCWSTR pszResolution = rProfile.GetEnumKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_RESOLUTION, L"");

	if (swscanf(pszResolution, L"%dx%d",
		&m_nResolutionWidth, &m_nResolutionHeight) != 2)
	{
		// If invalid value format, use default resolution

		m_nResolutionWidth = 800;
		m_nResolutionHeight = 600;
	}

	// Read full screen

	m_bFullScreen = rProfile.GetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_FULLSCREEN, false);

	// Read device format

	m_nDeviceFormat = (Graphics::DeviceFormats)rProfile.GetEnumKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_DEVFORMAT,
		SZ_DEVICEFORMATS, N_DEVICEFORMATS, Graphics::FORMAT_COUNT,
		Graphics::FORMAT_DESKTOP);

	// Read MSAA level

	m_nMSAALevel = (D3DMULTISAMPLE_TYPE)rProfile.GetIntKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_MSAA);

	// Read MSAA quality

	m_dwMSAAQuality = (DWORD)rProfile.GetIntKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_MSAAQUALITY);

	// Read refresh rate

	m_dwRefreshRate = rProfile.GetDwordKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_REFRESH, DEFAULT_VALUE);

	// Read vsync

	m_bVSync = rProfile.GetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_VSYNC, true);

	// Read hardware acceleration

	m_bHardwareAcceleration = rProfile.GetBoolKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_HWACCEL, true);

	// Read software vertex processing

	m_bSoftwareVertexProcessing = rProfile.GetBoolKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_SVP, false);

	// Read shader debug

	m_bShaderDebug = rProfile.GetBoolKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_SHD, false);

	// Read pure device

	m_bPureDevice = rProfile.GetBoolKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_PUREDEVICE, true);

	// Read optional batch primitive count (not written back)

	int nMaxBatchPrimCount = rProfile.GetIntKeyValue(
		SZ_PROFILE_SECTION_VIDEO, SZ_PROFILE_KEY_BATCHPRIMCOUNT);

	// If batch count is zero, don't set (use engine default)

	if (nMaxBatchPrimCount != 0)
		m_Engine.SetOption(Engine::OPTION_MAX_BATCH_PRIM,
			nMaxBatchPrimCount > 100 ? nMaxBatchPrimCount : 100);

	//
	// Read audio settings
	//

	// Read exclusive sound

	m_Engine.SetOption(Engine::OPTION_EXCLUSIVE_SOUND,
		rProfile.GetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_EXCLUSIVESOUND, true));

	// Read disable sounds

	m_Engine.SetOption(Engine::OPTION_DISABLE_SOUNDS,
		rProfile.GetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_DISABLESOUND, false));

	// Read disable music

	m_Engine.SetOption(Engine::OPTION_DISABLE_MUSIC,
		rProfile.GetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_DISABLEMUSIC, false));

	// Read audio destination

	m_Engine.SetOption(Engine::OPTION_AUDIO_DESTINATION,
		rProfile.GetEnumKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_AUDIODEST, SZ_AUDIO_PATHS, 2, 0));

	//
	// Read startup settings
	//

	// Read command show

	m_nCmdShow = rProfile.GetEnumKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_CMDSHOW, SZ_SWFLAGS, N_SWFLAGS,
		sizeof(N_SWFLAGS) / sizeof(int), m_nCmdShow);

	// Read control settings path

	SetControlsProfilePath(rProfile.GetStringKeyValue(
		SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_CONTROLS, NULL));

	// Read log file path

	SetLogFilePath(rProfile.GetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_LOGFILE, NULL));

	// Read theme path

	SetThemePath(rProfile.GetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_UITHEME, NULL));

	// Read log mode

	m_nLogMode = (LogMode)rProfile.GetEnumKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_LOGMODE, SZ_LOGMODES, LOG_COUNT, 0);

	//
	// Read control settings
	//

	RegisterControls();

	try
	{
		m_Controls.Deserialize(m_strControlsProfilePath);
	}

	catch(Error&)
	{
		ResetControls();
	}
}

void Client::RegisterControls(void)
{
	// Requires implementation by user
}

void Client::ResetControls(void)
{
	// Requires implementation by user
}

bool Client::CheckDeviceCaps(const D3DCAPS9& rCaps)
{
	// Shader Model 2 or above required

	if (D3DSHADER_VERSION_MAJOR(rCaps.VertexShaderVersion) < 2 ||
	   D3DSHADER_VERSION_MAJOR(rCaps.PixelShaderVersion) < 2)
	   return false;

	// Hardware T & L required

	if (~rCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ||
	   ~rCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
		return false;

	// Scissor rect required

	if (~rCaps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST)
		return false;

	return true;
}

void Client::InitializeEngine(void)
{
	// Set pre-initialization options

	if (true == m_bShaderDebug)
		m_Engine.SetOptionEx(Engine::OPTION_EFFECT_COMPILE_FLAGS,
			D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION);

	// Initialize engine

	m_Engine.Initialize(
		m_strTitle,						// Window title
		m_dwIconID,						// Window icon resource ID
		m_bFullScreen,					// Full screen or windowed
		m_nResolutionWidth,				// Back buffer width
		m_nResolutionHeight,			// Back buffer height
		m_nDeviceFormat,				// Back buffer format
		m_nMSAALevel,					// Anti-alias sample level
		m_dwMSAAQuality,				// Anti-alias quality
		m_dwRefreshRate,				// Refresh rate
		m_bVSync,						// Presentation interval (vsync or not)
		m_bHardwareAcceleration,		// Hardware acceleration (HAL or REF)
		m_bSoftwareVertexProcessing,	// Use software vertex processing (useful for vertex shader debugging)
		m_bPureDevice,					// Create pure device
		true);							// Start session
}

void Client::InitializeInstance(void)
{
	// Set default base paths and extensions

	m_Engine.SetBaseSavePath(SZ_BASE_PATH_SAVE);
	m_Engine.SetBaseSaveExtension(SZ_BASE_EXT_SAVE);

	m_Engine.GetTextures().SetBasePath(SZ_BASE_PATH_TEXTURES);
	m_Engine.GetTextures().SetBaseExtension(SZ_BASE_EXT_TEXTURES);

	m_Engine.GetRegions().SetBasePath(SZ_BASE_PATH_REGIONS);
	m_Engine.GetRegions().SetBaseExtension(SZ_BASE_EXT_REGIONS);

	m_Engine.GetMaterials().SetBasePath(SZ_BASE_PATH_MATERIALS);
	m_Engine.GetMaterials().SetBaseExtension(SZ_BASE_EXT_MATERIALS);

	m_Engine.GetAnimations().SetBasePath(SZ_BASE_PATH_ANIMATIONS);
	m_Engine.GetAnimations().SetBaseExtension(SZ_BASE_EXT_ANIMATIONS);

	m_Engine.GetEffects().SetBasePath(SZ_BASE_PATH_EFFECTS);
	m_Engine.GetEffects().SetBaseExtension(SZ_BASE_EXT_EFFECTS);

	m_Engine.GetSprites().SetBasePath(SZ_BASE_PATH_SPRITES);
	m_Engine.GetSprites().SetBaseExtension(SZ_BASE_EXT_SPRITES);

	m_Engine.GetVideos().SetBasePath(SZ_BASE_PATH_VIDEOS);
	m_Engine.GetVideos().SetBaseExtension(SZ_BASE_EXT_VIDEOS);

	m_Engine.GetMusic().SetBasePath(SZ_BASE_PATH_MUSIC);
	m_Engine.GetMusic().SetBaseExtension(SZ_BASE_EXT_MUSIC);

	m_Engine.GetSounds().SetBasePath(SZ_BASE_PATH_SOUNDS);
	m_Engine.GetSounds().SetBaseExtension(SZ_BASE_EXT_SOUNDS);

	m_Engine.GetFonts().SetBasePath(SZ_BASE_PATH_FONTS);
	m_Engine.GetFonts().SetBaseExtension(SZ_BASE_EXT_FONTS);

	m_Engine.GetStrings().SetBasePath(SZ_BASE_PATH_STRINGS);
	m_Engine.GetStrings().SetBaseExtension(SZ_BASE_EXT_STRINGS);

	m_Engine.GetScreens().SetBasePath(SZ_BASE_PATH_SCREENS);
	m_Engine.GetScreens().SetBaseExtension(SZ_BASE_EXT_SCREENS);	

	m_Engine.GetMaps().SetBasePath(SZ_BASE_PATH_MAPS);
	m_Engine.GetMaps().SetBaseExtension(SZ_BASE_EXT_MAPS);

	m_Engine.GetCommands().SetBasePath(SZ_BASE_PATH_SCRIPTS);
	m_Engine.GetCommands().SetBaseExtension(SZ_BASE_EXT_SCRIPTS);

	// Load user interface theme

	m_Engine.GetScreens().SetTheme(m_strThemePath);

	Print(L"loaded user interface theme", PRINT_INFO);

	// Set log mode

	SetLogMode(m_nLogMode);
}

void Client::Update(void)
{
	m_Engine.Update();
}

void Client::Render(void)
{
	Graphics& rGraphics = m_Engine.GetGraphics();

	rGraphics.Clear(m_Engine.GetBackColorConst());

	rGraphics.BeginScene();

	rGraphics.BeginBatch();

	m_Engine.Render();

	rGraphics.EndBatch();

	rGraphics.EndScene();

	rGraphics.Present();
}

void Client::SerializeSettings(void)
{
	IniFile profile(&m_Engine.GetErrors());

	SerializeSettings(profile);

	try
	{			
		profile.Serialize(m_strProfilePath);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		// Ignore errors from not being able to save profile

		while(m_Engine.GetErrors().GetCount())
			m_Engine.GetErrors().Pop();
	}
}

void Client::SerializeSettings(IniFile& rProfile)
{
	//
	// Save video settings
	//

	// Save resolution

	String strResolution;

	strResolution.Format(L"%dx%d", m_nResolutionWidth,
		m_nResolutionHeight);

	rProfile.SetEnumKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_RESOLUTION, strResolution);

	// Save full screen

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_FULLSCREEN, m_bFullScreen);

	// Save device format

	rProfile.SetEnumKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_DEVFORMAT, int(m_nDeviceFormat),
		SZ_DEVICEFORMATS, N_DEVICEFORMATS, Graphics::FORMAT_COUNT);

	// Save msaa level

	rProfile.SetIntKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_MSAA, int(m_nMSAALevel));

	// Save msaa quality

	rProfile.SetIntKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_MSAAQUALITY, int(m_dwMSAAQuality));

	// Save refresh rate

	if (INVALID_VALUE == m_dwRefreshRate)
	{
		rProfile.SetEnumKeyValue(SZ_PROFILE_SECTION_VIDEO,
			SZ_PROFILE_KEY_REFRESH, SZ_DEVICEFORMATS[0]);
	}
	else
	{
		rProfile.SetDwordKeyValue(SZ_PROFILE_SECTION_VIDEO,
			SZ_PROFILE_KEY_REFRESH, m_dwRefreshRate);
	}

	// Save vsync

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_VSYNC, m_bVSync);

	// Save hardware acceleration

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_HWACCEL, m_bHardwareAcceleration);

	// Save software vertex processing

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_SVP, m_bSoftwareVertexProcessing);

	// Save shader debug

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_SHD, m_bShaderDebug);

	// Save pure device

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_VIDEO,
		SZ_PROFILE_KEY_PUREDEVICE, m_bPureDevice);

	//
	// Save audio settings
	//

	// Save exclusive sound

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_EXCLUSIVESOUND,
		m_Engine.GetOption(Engine::OPTION_EXCLUSIVE_SOUND) == TRUE);

	// Save disable sounds

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_DISABLESOUND,
		m_Engine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE);

	// Save disable music

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_DISABLEMUSIC,
		m_Engine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE);

	// Save audio path

	rProfile.SetEnumKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_AUDIODEST,
		m_Engine.GetOption(Engine::OPTION_AUDIO_DESTINATION),
		SZ_AUDIO_PATHS, 2, SZ_AUDIO_PATHS[0]);

	//
	// Save startup settings
	//

	// Write control settings path

	rProfile.SetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_CONTROLS, m_strControlsProfilePath);

	// Save log mode

	rProfile.SetEnumKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_LOGMODE, m_nLogMode, SZ_LOGMODES,
		LOG_COUNT, SZ_LOGMODES[LOG_DISABLE]);

	// Save log file path

	rProfile.SetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_LOGFILE, m_strLogFilePath);

	// Write theme path

	rProfile.SetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_UITHEME, m_strThemePath);

	//
	// Write control settings
	//

	m_Controls.Serialize(m_strControlsProfilePath);
}

void Client::DestroyInstance(void)
{
	// Nothing to do by default
}

void Client::ApplyGraphicsSettings(void)
{
	// Keep track of whether a complete device re-create is required

	bool bRecreateRequired = false;

	// Determine device type

	D3DDEVTYPE nDevType = (true == m_bHardwareAcceleration) ?
		D3DDEVTYPE_HAL : D3DDEVTYPE_REF;

	if (nDevType != m_Engine.GetGraphics().GetDeviceType())
		bRecreateRequired = true;

	m_Engine.GetGraphics().SetDeviceType(nDevType);

	// Get device capabilities

	LPDIRECT3D9 pD3D = m_Engine.GetGraphics().GetDirect3D();
	D3DCAPS9 devcaps;
	HRESULT hr = 0;

	if (true == bRecreateRequired)
	{
		hr = pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
			nDevType, &devcaps);

		if (FAILED(hr))
			throw m_Engine.GetErrors().Push(Error::D3D_GETDEVICECAPS,
				__FUNCTIONW__, hr);
	}
	else
	{
		CopyMemory(&devcaps, &m_Engine.GetGraphics().GetDeviceCaps(),
			sizeof(D3DCAPS9));
	}

	// Validate device capabilities

	if (true == m_bHardwareAcceleration &&
	   m_Engine.GetClientInstance() != NULL)
	{
		if (m_Engine.GetClientInstance()->CheckDeviceCaps(devcaps) == false)
			   throw m_Engine.GetErrors().Push(
			   Error::D3D_ADAPTERCAPS, __FUNCTIONW__);
	}

	m_Engine.GetGraphics().SetDeviceCaps(devcaps);

	// Set creation flags based on caps and passed options

	DWORD dwDevFlags = 0;

	if (false == m_bHardwareAcceleration ||
	   true == m_bSoftwareVertexProcessing)
	{
		dwDevFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		if (devcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		{
			dwDevFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

			if (true == m_bPureDevice &&
				devcaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
				dwDevFlags |= D3DCREATE_PUREDEVICE;
		}
		else
		{
			dwDevFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

	if (m_Engine.GetGraphics().GetDeviceFlags() != dwDevFlags)
	{
		bRecreateRequired = true;
		m_Engine.GetGraphics().SetDeviceFlags(dwDevFlags);
	}

	// Get current parameters

	D3DPRESENT_PARAMETERS d3dpp;

	CopyMemory(&d3dpp, &m_Engine.GetGraphics().GetDeviceParams(),
		sizeof(D3DPRESENT_PARAMETERS));	

	// Keep track of changes to window style

	BOOL bOriginalWindowed = d3dpp.Windowed;

	// Update vsync setting

	if (true == m_bVSync)
	{
		// If v-sync is requested, see if it's available and use it

		if (devcaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE)
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		// If only v-sync is available, use it. Otherwise, use immediate

		if (~devcaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	D3DDISPLAYMODE d3ddm = {0};

	if (true == m_bFullScreen)
	{
		D3DFORMAT nD3DDevFormat = D3DFMT_UNKNOWN;

		if (Graphics::FORMAT_DESKTOP == m_nDeviceFormat)
		{
			// Select desktop format if specified

			pD3D->GetAdapterDisplayMode(
				D3DADAPTER_DEFAULT, &d3ddm);

			nD3DDevFormat = d3ddm.Format;
		}
		else
		{
			// Otherwise, select the exact format

			nD3DDevFormat =
				D3DFORMAT(Graphics::DW_DEVICE_FORMATS[m_nDeviceFormat]);
		}		

		// Make sure at least one adapter mode with that format exists

		DWORD dwModeCount =
			pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, nD3DDevFormat);

		if (0 == dwModeCount)
			throw m_Engine.GetErrors().Push(Error::D3D_ADAPTERMODE,
				__FUNCTIONW__);

		// If so, find the one with the needed resolution and frequency

		while(dwModeCount-- > 0)
		{
			hr = pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,
				nD3DDevFormat, dwModeCount, &d3ddm);

			if (FAILED(hr))
				throw m_Engine.GetErrors().Push(Error::D3D_ENUMADAPTERMODES,
					__FUNCTIONW__, hr);

			if (int(d3ddm.Width) == m_nResolutionWidth &&
			   int(d3ddm.Height) == m_nResolutionHeight)
			{
				// If we have a specific refresh rate specified and
				// this mode does not have it, skip this mode

				if (m_dwRefreshRate != DEFAULT_VALUE &&
				   d3ddm.RefreshRate != m_dwRefreshRate) continue;

				// We found a device mode that has size, format, and refresh rate

				break;
			}
		}

		// If mode was not found, exit

		if (INVALID_VALUE == dwModeCount)
			throw m_Engine.GetErrors().Push(Error::D3D_ADAPTERMODE,
				__FUNCTIONW__);

		// Otherwise, use it

		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.BackBufferWidth = d3ddm.Width;
		d3dpp.BackBufferHeight = d3ddm.Height;
		d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
	}
	else
	{
		// Set back buffer format to desktop

		hr = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

		if (FAILED(hr))
			throw m_Engine.GetErrors().Push(Error::D3D_GETADAPTERDISPLAYMODE,
				__FUNCTIONW__, D3DADAPTER_DEFAULT, hr);

		d3dpp.Windowed = TRUE;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.BackBufferWidth = DWORD(m_nResolutionWidth);
		d3dpp.BackBufferHeight = DWORD(m_nResolutionHeight);
		d3dpp.FullScreen_RefreshRateInHz = 0;
	}

	// Set multi-sampling parameters

	DWORD dwQualityStops = 0;

	hr = pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
		nDevType, d3dpp.BackBufferFormat, !m_bFullScreen,
		m_nMSAALevel, &dwQualityStops);

	while(FAILED(hr) && (m_nMSAALevel - 1) > D3DMULTISAMPLE_NONE)
	{
		m_nMSAALevel = (D3DMULTISAMPLE_TYPE)(m_nMSAALevel - 1);

		hr = pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
			nDevType, d3dpp.BackBufferFormat, !m_bFullScreen,
			m_nMSAALevel, &dwQualityStops);
	}

	d3dpp.MultiSampleType = m_nMSAALevel;

	if (m_dwMSAAQuality > dwQualityStops - 1)
		m_dwMSAAQuality = dwQualityStops - 1;

	d3dpp.MultiSampleQuality = m_dwMSAAQuality;

	// Resize game window

	if (m_Engine.GetOption(Engine::OPTION_MANAGE_GAME_WINDOW) == TRUE)
	{
		HWND hGameWindow = m_Engine.GetGameWindow();

		if (true == m_bFullScreen)
		{
			if (TRUE == bOriginalWindowed)
			{
				// Changing from window to full screen

				m_Engine.PrintDebug(L"Changing from window to full screen");

				SetWindowLong(hGameWindow, GWL_STYLE, WS_POPUP);

				SetWindowPos(hGameWindow, NULL, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);			

				SetWindowPos(hGameWindow, NULL, 0, 0,
					m_nResolutionWidth,
					m_nResolutionHeight,
					SWP_NOZORDER);
			}
			else
			{
				// Resizing full-screen

				m_Engine.PrintDebug(L"Resizing full screen");

				MoveWindow(hGameWindow, 0, 0,
					m_nResolutionWidth, m_nResolutionHeight, FALSE);
			}
		}
		else
		{
			int nWidth = m_nResolutionWidth +
				GetSystemMetrics(SM_CXFIXEDFRAME) * 2;

			int nHeight = m_nResolutionHeight +
				GetSystemMetrics(SM_CYFIXEDFRAME) * 2 +
				GetSystemMetrics(SM_CYCAPTION);

			int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

			if (TRUE == bOriginalWindowed)
			{
				// Resizing window

				m_Engine.PrintDebug(L"Resizing window");

				MoveWindow(hGameWindow,
					(nScreenWidth - nWidth) / 2,
					(nScreenHeight - nHeight) / 2,
					nWidth,
					nHeight, FALSE);
			}
			else
			{
				// Changing from full screen to window

				m_Engine.PrintDebug(L"Changing from full screen to window");

				WNDCLASSEX wcx = {0};
				WCHAR szClassName[MAX_PATH] = {0};
				WCHAR szTitle[MAX_PATH] = {0};

				HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

				GetClassName(hGameWindow, szClassName, MAX_PATH);
				GetClassInfoEx(hInstance, szClassName, &wcx);
				GetWindowText(hGameWindow, szTitle, MAX_PATH);

				DoEvents();

				DestroyWindow(hGameWindow);

				hGameWindow = CreateWindowEx(0, szClassName, szTitle,
					WS_CAPTION | WS_SYSMENU,
					(nScreenWidth - nWidth) / 2, (nScreenHeight - nHeight) / 2,
					nWidth, nHeight, (HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);

				SetProp(hGameWindow, Engine::SZ_WNDPROP, (HANDLE)&m_Engine);

				m_Engine.SetGameWindow(hGameWindow, false);

				d3dpp.hDeviceWindow = hGameWindow;

				bRecreateRequired = true;
			}			
		}
	}

	m_Engine.GetGraphics().SetDeviceParams(d3dpp);

	ShowWindow(d3dpp.hDeviceWindow, SW_SHOW);
	UpdateWindow(d3dpp.hDeviceWindow);

	SetFocus(d3dpp.hDeviceWindow);
	SetForegroundWindow(d3dpp.hDeviceWindow);

	m_Engine.GetGraphics().Reset(true == bRecreateRequired ?
		Graphics::RECREATE_DEVICE : Graphics::RESET_DEVICE);
}

void Client::OnSessionStart(void)
{
	// Default handler
}

void Client::OnSessionSave(Stream& rStream)
{
	// Default handler
}

void Client::OnSessionLoad(Stream& rStream)
{
	// Default handler
}

void Client::OnSessionEnd(void)
{
	// Default handler
}

void Client::OnSessionPause(bool bPause)
{
	// Default handler
}

void Client::OnError(const Error& rError)
{	
	// Hide the game window

	if (m_Engine.GetGameWindow() != NULL)
		ShowWindow(m_Engine.GetGameWindow(), SW_HIDE);

	if (m_Engine.GetErrors().GetCount() > 0)
	{
		// Display any errors on the stack

		while(m_Engine.GetErrors().GetCount())
		{
			MessageBox(NULL,
				m_Engine.GetErrors().GetLastError().GetDescription(),
				m_strTitle, MB_ICONSTOP);

			m_Engine.GetErrors().Pop();
		}
	}
	else
	{
		// If no errors on the stack, display current error

		MessageBox(NULL, rError.GetDescription(),
			m_strTitle, MB_ICONSTOP);
	}

	Exit();
}

DWORD Client::GetMemoryFootprint(void) const
{
	DWORD dwSize =
		sizeof(Client) - sizeof(Engine) +
		m_Engine.GetMemoryFootprint() +
		DWORD(m_strBaseDir.GetLengthBytes()) +
		DWORD(m_strExeDir.GetLengthBytes()) +
		DWORD(m_strExeTitle.GetLengthBytes()) +
		DWORD(m_strProfilePath.GetLengthBytes()) +
		DWORD(m_strLogFilePath.GetLengthBytes()) +
		DWORD(m_strTitle.GetLengthBytes()) +
		DWORD(m_strDescription.GetLengthBytes());

	for(StringArray::const_iterator pos = m_arCommandArgs.begin();
		pos != m_arCommandArgs.end();
		pos++)
	{
		dwSize += (DWORD)(*pos).GetLengthBytes();
	}

	if (m_pLogFile != NULL)
		dwSize += (DWORD)m_pLogFile->GetPath().GetLengthBytes();

	Stream streamExe;
	WCHAR szExePath[MAX_PATH] = {0};

	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH - 1);

	try
	{
		streamExe.Open(szExePath, GENERIC_READ, OPEN_EXISTING);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		// Ignore error
	}

	if (streamExe.IsOpen())
		dwSize += streamExe.GetSize();

	return dwSize;
}

void Client::Print(LPCWSTR pszString, PrintTypes nPrintType, bool bLine)
{
	if (true == m_bMute)
	{
		Message msg;
		msg.nEntryType = nPrintType;
		msg.strText = pszString;
		msg.bNewLine = bLine;

		m_arMutedMessages.push_back(msg);
	}
	else if (m_nLogMode != LOG_DISABLE &&
			nPrintType != PRINT_CLEAR)
	{
		// If logging was muted, create the log interface

		if (NULL == m_pLogFile)
			SetLogMode(m_nLogMode);

		// If logging is enabled, write to log (unless clearing)
		// Ignore any log errors, not that important

		try
		{
			m_pLogFile->Print(pszString, nPrintType, bLine);
		}

		catch(Error&) {}
	}

	// If debug mode is on, output to immediate

#ifdef _DEBUG
	if (PRINT_DEBUG == nPrintType && false == m_bMute)
	{
		OutputDebugString(pszString);
		OutputDebugString(L"\n");
	}
#endif
}

void Client::PrintEx(LPCWSTR pszString, PrintTypes nPrintType, bool bLogDateTime, bool bLine)
{
	if (m_pLogFile != NULL)
	{
		bool bDateTimeEnabled = m_pLogFile->IsDateTimeEnabled();

		if (bDateTimeEnabled != bLogDateTime)
			m_pLogFile->EnableDateTime(bLogDateTime);

		Print(pszString, nPrintType, bLine);

		if (bDateTimeEnabled != bLogDateTime)
			m_pLogFile->EnableDateTime(bDateTimeEnabled);
	}
	else
	{
		Print(pszString, nPrintType, bLine);
	}
}

void Client::PrintMute(bool bMute)
{
	if (m_bMute == bMute)
		return;

	if (true == m_bMute && false == bMute)
	{
		// Unmuting - print accumulated messages

		m_bMute = false;

		for(MessageArrayIterator pos = m_arMutedMessages.begin();
			pos != m_arMutedMessages.end();
			pos++)
		{
			Print(pos->strText, pos->nEntryType, pos->bNewLine);
		}

		m_arMutedMessages.clear();
	}
	else
	{
		m_bMute = bMute;
	}
}

void Client::OnLogStart(void)
{
	// Default handler
}

void Client::OnLogEnd(void)
{
	// Default handler
}

void Client::OnProgress(ProgressTypes nType,
						ProgressSubTypes nSubType,
						int nProgress,
						int nProgressMax)
{
	// Default handler
}

void Client::OnMediaNotify(MediaNotify nNotify, Resource* pMedia)
{
	// Default handler
	UNREFERENCED_PARAMETER(pMedia);
}

void Client::OnThemeChange(Theme& rNewTheme)
{
	// Default handler
	UNREFERENCED_PARAMETER(rNewTheme);
}

void Client::OnLostDevice(bool bRecreate)
{
	// Default handler
}

void Client::OnResetDevice(bool bRecreate)
{
	// Default handler
}

void Client::OnKeyDown(int nKeyCode)
{
	// Translate control ID

	int nControlID = m_Controls.Translate(nKeyCode);

	if (nControlID != INVALID_INDEX)
		OnControl(nControlID);
}

void Client::OnKeyUp(int nKeyCode)
{
	// Default handler
	UNREFERENCED_PARAMETER(nKeyCode);
}

void Client::OnKeyPress(int nAsciiCode, bool extended, bool alt)
{
	// Default handler
	UNREFERENCED_PARAMETER(nAsciiCode);
}

void Client::OnControl(int nControlID)
{
	// Default handler
	UNREFERENCED_PARAMETER(nControlID);
}

void Client::Empty(void)
{
	// Stop logging

	SetLogMode(LOG_DISABLE);

	Exit();
}