/*------------------------------------------------------------------*\
|
| ThunderClient.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm game class
| Created: 03/31/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_GAME_H
#define THUNDER_GAME_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"		// using String
#include "ThunderEngine.h"		// using Engine and RESOLUTIONS
#include "ThunderControls.h"	// using ControlManager
#include "ThunderLogFile.h"		// using LogFile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class IniFile;					// referencing IniFile
class InfoFile;					// referencing InfoFile

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Message> MessageArray;
typedef	std::vector<Message>::iterator MessageArrayIterator;
typedef std::vector<Message>::const_iterator MessageArrayConstIterator;


/*----------------------------------------------------------*\
| Client class - base class for client games
\*----------------------------------------------------------*/

class Client
{
public:
	//
	// Constants
	//

	// Module version components

	enum VersionComponents
	{
		// Major
		VERSION_MAJOR,

		// Minor
		VERSION_MINOR,

		// Revision
		VERSION_REVISION,

		// Build number
		VERSION_BUILD,

		// Number of version components defined
		VERSION_COUNT
	};

	// Logging modes

	enum LogMode
	{
		// Disable logging
		LOG_DISABLE,

		// Text mode
		LOG_TEXT,

		// HTML mode
		LOG_HTML,

		// Number of logging modes defined
		LOG_COUNT,
	};

	// Media event types

	enum MediaNotify
	{
		// Media played to the end
		MEDIA_END,
	};

	// Type of progress notification (::OnProgress)

	enum ProgressTypes
	{
		// Loading session/map/map instance
		PROGRESS_LOAD,

		// Saving session/map/map instance
		PROGRESS_SAVE
	};

	enum ProgressSubTypes
	{
		// Subtype/flags of progress event

		// Loading session, max = 1
		PROGRESS_SESSION,

		// Loading session header, max = 1
		PROGRESS_SESSION_HEADER,

		// Loading maps, max = number of maps
		PROGRESS_SESSION_MAPS,

		// Loading map, max = 1
		PROGRESS_MAP,

		// Loading map header, max = 1
		PROGRESS_MAP_HEADER,

		// Loading map instance, max = 2
		PROGRESS_MAP_INSTANCE,

		// Loading map materials, max = number of materials
		PROGRESS_MAP_MATERIALS,

		// Loading map animation, max = number of animations
		PROGRESS_MAP_ANIMATIONS,

		// Loading map sounds, max = number of sounds
		PROGRESS_MAP_SOUNDS,

		// Loading map music, max = number of music files
		PROGRESS_MAP_MUSIC,

		// Loading map tiles, max = number of tiles
		PROGRESS_MAP_TILES,

		// Loading map layers, max = number of layers
		PROGRESS_MAP_LAYERS,

		// Loading map actors, max = number of actors
		PROGRESS_MAP_ACTORS,

		// Loading map user data, max = 1
		PROGRESS_MAP_USER,

		// Number of progress events defined
		PROGRESS_COUNT
	};

protected:
	//
	// Engine
	//

	// Engine instance
	Engine m_Engine;

	//
	// State
	//

	// Currently running the game loop?
	bool m_bRunning;

	//
	// Info
	//

	// Base directory (working directory)
	String m_strBaseDir;

	// Executable directory
	String m_strExeDir;

	// Executable title
	String m_strExeTitle;

	// Profile path
	String m_strProfilePath;

	// Profile path for storing key bindings
	String m_strControlsProfilePath;

	// Log file path
	String m_strLogFilePath;

	// User interface theme path
	String m_strThemePath;

	// Window resource icon ID
	DWORD m_dwIconID;

	// Title
	String m_strTitle;

	// Description
	String m_strDescription;

	// Version
	int m_nVersion[VERSION_COUNT];

	// Show command
	int m_nCmdShow;

	// Command line arguments
	StringArray m_arCommandArgs;

	//
	// Settings
	//

	// Video

	// Resolution width
	int m_nResolutionWidth;

	// Resolution height
	int m_nResolutionHeight;

	// Full screen or windowed
	bool m_bFullScreen;
	
	// Device back buffer format
	Graphics::DeviceFormats m_nDeviceFormat;

	// Multi-sampling level
	D3DMULTISAMPLE_TYPE m_nMSAALevel;

	// Multi-sampling quality
	DWORD m_dwMSAAQuality;

	// Refresh rate
	DWORD m_dwRefreshRate;

	// Vsync or immediate presentation	
	bool m_bVSync;
	
	// Use hardware acceleration (HAL/REF)
	bool m_bHardwareAcceleration;

	// Use software vertex processing even with a HAL device
	bool m_bSoftwareVertexProcessing;

	// Enable shader debugging? (sets engine option)
	bool m_bShaderDebug;

	// Create pure device
	bool m_bPureDevice;

	// Controls

	// Key bindings
	ControlManager m_Controls;

	//
	// Diagnostics
	//

	// Text or HTML log file, depending on log mode
	LogFile* m_pLogFile;

	// Logging mode
	LogMode m_nLogMode;

	// Mute diagnostic messages received with Print()?
	bool m_bMute;

	// When diagnostic mute is set, messages accumulate here
	MessageArray m_arMutedMessages;

public:
	Client(DWORD dwIconID = DEFAULT_VALUE,
		LPCWSTR pszTitle = NULL,
		LPCWSTR pszDescription = NULL,
		int nResolutionWidth = 640,
		int nResolutionHeight = 480,
		bool bFullScreen = false,
		Graphics::DeviceFormats nDeviceFormat = Graphics::FORMAT_X8R8G8B8,
		D3DMULTISAMPLE_TYPE nMultiSampleType = D3DMULTISAMPLE_NONE,
		DWORD dwMultiSampleQuality = 0,
		DWORD dwRefreshRate = DEFAULT_VALUE,
		bool bVSync = false,
		bool bHardwareAcceleration = true,
		bool bSoftwareVertexProcessing = false,
		bool bShaderDebug = false,
		bool bPureDevice = false,
		LogMode nLogMode = LOG_DISABLE);

	virtual ~Client(void);

public:
	//
	// Engine instance
	//

	inline Engine& GetEngine(void)
	{
		return m_Engine;
	}

	//
	// Execution
	//	

	void Run(LPCWSTR pszCmdLine = NULL, int nCmdShow = SW_SHOW);

	inline bool IsRunning(void) const
	{
		return m_bRunning;
	}

	void Wait(float fRunTimeSeconds);
	int DoEvents(void);

	inline void Exit(void)
	{
		m_bRunning = false;
	}

	virtual void ProcessCommandLine(LPCWSTR pszCmdLine);
	virtual void DeserializeSettings(void);
	virtual void DeserializeSettings(const IniFile& rProfile);
	virtual bool CheckDeviceCaps(const D3DCAPS9& rCaps);
	virtual void InitializeEngine(void);
	virtual void InitializeInstance(void);
	virtual void Update(void);
	virtual void Render(void);
	virtual void SerializeSettings(void);
	virtual void SerializeSettings(IniFile& rProfile);
	virtual void DestroyInstance(void);

	//
	// Controls
	//

	inline ControlManager& GetControls(void)
	{
		return m_Controls;
	}

	virtual void RegisterControls(void);
	virtual void ResetControls(void);

	//
	// Info
	//

	inline DWORD GetIconID(void) const
	{
		return m_dwIconID;
	}

	inline void SetIconID(DWORD dwIconID)
	{
		m_dwIconID = dwIconID;
	}

	inline const String& GetTitle(void) const
	{
		return m_strTitle;
	}

	inline void SetTitle(LPCWSTR pszTitle)
	{
		m_strTitle = pszTitle;
	}

	inline const String& GetDescription(void) const
	{
		return m_strDescription;
	}

	inline void SetDescription(LPCWSTR pszDescription)
	{
		m_strDescription = pszDescription;
	}

	inline const int* GetVersion(void) const
	{
		return m_nVersion;
	}

	inline int GetVersion(VersionComponents nComponent) const
	{
		if (nComponent >= VERSION_MAJOR || nComponent <= VERSION_BUILD)
			return m_nVersion[nComponent];
		else
			return INVALID_INDEX;
	}

	inline void SetVersion(const int pnVersion[4])
	{
		CopyMemory(m_nVersion, pnVersion, sizeof(m_nVersion));
	}

	inline void SetVersion(VersionComponents nComponent, int nValue)
	{
		if (nComponent >= VERSION_MAJOR || nComponent <= VERSION_BUILD)
			m_nVersion[nComponent] = nValue;
	}

	//
	// Settings
	//

	// Video

	inline int GetDisplayWidth(void) const
	{
		return m_nResolutionWidth;
	}

	inline void SetDisplayWidth(int nDisplayWidth)
	{
		m_nResolutionWidth = nDisplayWidth;
	}

	inline int GetDisplayHeight(void) const
	{
		return m_nResolutionHeight;
	}

	inline void SetDisplayHeight(int nDisplayHeight)
	{
		m_nResolutionHeight = nDisplayHeight;
	}

	inline bool GetFullScreen(void) const
	{
		return m_bFullScreen;
	}

	inline void SetFullScreen(bool bFullScreen)
	{
		m_bFullScreen = bFullScreen;
	}

	inline Graphics::DeviceFormats GetDeviceFormat(void) const
	{
		return m_nDeviceFormat;
	}

	inline void SetDeviceFormat(Graphics::DeviceFormats nFormat)
	{
		m_nDeviceFormat = nFormat;
	}

	inline D3DMULTISAMPLE_TYPE GetMultiSampleType(void) const
	{
		return m_nMSAALevel;
	}

	inline void SetMultiSampleType(D3DMULTISAMPLE_TYPE nMultiSampleType)
	{
		m_nMSAALevel = nMultiSampleType;
	}

	inline DWORD GetMultiSampleQuality(void) const
	{
		return m_dwMSAAQuality;
	}

	inline void SetMultiSampleQuality(DWORD dwQuality)
	{
		m_dwMSAAQuality = dwQuality;
	}

	inline DWORD GetRefreshRate(void) const
	{
		return m_dwRefreshRate;
	}

	inline void SetRefreshRate(DWORD dwRefreshRate)
	{
		m_dwRefreshRate = dwRefreshRate;
	}

	inline bool GetVSync(void) const
	{
		return m_bVSync;
	}

	inline void SetVSync(bool bVSync)
	{
		m_bVSync = bVSync;
	}

	inline bool GetHardwareAcceleration(void) const
	{
		return m_bHardwareAcceleration;
	}

	inline void SetHardwareAcceleration(bool bHardwareAcceleration)
	{
		m_bHardwareAcceleration = bHardwareAcceleration;
	}

	inline bool GetSoftwareVertexProcessing(void) const
	{
		return m_bSoftwareVertexProcessing;
	}

	inline void SetSoftwareVertexProcessing(bool bSWVP)
	{
		m_bSoftwareVertexProcessing = bSWVP;
	}

	inline bool GetShaderDebug(void) const
	{
		return m_bShaderDebug;
	}

	inline void SetShaderDebug(bool bShaderDebug)
	{
		m_bShaderDebug = bShaderDebug;
	}

	inline bool GetPureDevice(void) const
	{
		return m_bPureDevice;
	}

	inline void SetPureDevice(bool bPureDevice)
	{
		m_bPureDevice = bPureDevice;
	}

	void ApplyGraphicsSettings(void);

	//
	// Paths
	//

	Resource* LoadResource(LPCWSTR pszPath,
		float fPersistenceTime = -1.0f);

	inline const String& GetBaseDirectory(void) const
	{
		return m_strBaseDir;
	}

	inline void SetBaseDirectory(LPCWSTR pszBaseDir)
	{
		m_strBaseDir = pszBaseDir;
	}

	inline const String& GetExeTitle(void) const
	{
		return m_strExeTitle;
	}
	
	inline const String& GetExeDirectory(void) const
	{
		return m_strExeDir;
	}

	inline const String& GetProfilePath(void) const
	{
		return m_strProfilePath;
	}

	void SetProfilePath(LPCWSTR pszProfilePath);

	inline const String& GetControlsProfilePath(void) const
	{
		return m_strControlsProfilePath;
	}

	void SetControlsProfilePath(LPCWSTR pszControlsPath);

	inline const String& GetLogFilePath(void) const
	{
		return m_strLogFilePath;
	}

	void SetLogFilePath(LPCWSTR pszLogFilePath);

	inline const String& GetThemePath(void) const
	{
		return m_strThemePath;
	}

	void SetThemePath(LPCWSTR pszThemePath);

	//
	// Command Line
	//

	inline int GetCommandShow(void) const
	{
		return m_nCmdShow;
	}

	inline const String& GetCommandArg(int nArgIndex) const
	{
		return m_arCommandArgs[nArgIndex];
	}

	inline int GetCommandArgCount(void) const
	{
		return int(m_arCommandArgs.size());
	}

	inline StringArrayIterator GetBeginCommandArg(void)
	{
		return m_arCommandArgs.begin();
	}

	inline StringArrayIterator GetEndCommandArg(void)
	{
		return m_arCommandArgs.end();
	}

	//
	// Logging
	//
	
	inline LogFile* GetLogFile(void) const
	{
		return m_pLogFile;
	}

	void SetLogMode(LogMode nLogMode);

	inline LogMode GetLogMode(void) const
	{
		return m_nLogMode;
	}
	
	//
	// Events
	//

	// General

	virtual void OnSessionStart(void);
	virtual void OnSessionSave(Stream& rStream);
	virtual void OnSessionLoad(Stream& rStream);
	virtual void OnSessionEnd(void);
	virtual void OnSessionPause(bool bPause);

	virtual void OnDeactivate(void);

	virtual void OnProgress(ProgressTypes nType, ProgressSubTypes nSubType, int nProgress, int nProgressMax);

	virtual void OnMediaNotify(MediaNotify nNotify, Resource* pMedia);

	virtual void OnThemeChange(Theme& rNewTheme);
	
	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnLogStart(void);
	virtual void OnLogEnd(void);

	virtual void OnError(const Error& rError);

	// Input

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode, bool extended, bool alt);
	virtual void OnControl(int nControlID);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	virtual void Print(LPCWSTR pszString, PrintTypes nPrintType = PRINT_MESSAGE, bool bLine = true);

	void PrintEx(LPCWSTR pszString, PrintTypes nPrintType = PRINT_MESSAGE, bool bLogDateTime = false, bool bLine = true);

	void PrintMute(bool bMute);

	inline bool IsPrintMuted(void) const
	{
		return m_bMute;
	}

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_GAME_H