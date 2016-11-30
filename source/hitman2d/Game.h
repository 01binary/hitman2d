/*------------------------------------------------------------------*\
|
| Game.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D game class
| Created: 06/30/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef GAME_H
#define GAME_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class ScreenConsole;	// referencing ScreenConsole
class ScreenProgress;	// referencing ScreenProgress


/*----------------------------------------------------------*\
| Game class
\*----------------------------------------------------------*/

class Game: public Client
{
public:
	//
	// Constants
	//

	// Actions mapped to controls

	enum Controls
	{
		// Game

		CONTROL_GAME_PAUSE,				// Pause menu
		CONTROL_GAME_SCREENSHOT,		// Take screenshot

		// Debug

		CONTROL_DEBUG_CONSOLE,			// Debug - show console
		CONTROL_DEBUG_EXIT,				// Debug - instant exit
		CONTROL_DEBUG_FILLMODE,			// Debug - toggle fill mode
		CONTROL_DEBUG_TEST,				// Debug - execute test command
		CONTROL_DEBUG_PRINTSTATS,		// Debug - print render statistics

		// Editor

		CONTROL_EDITOR_CAMERAUP,		// Editor - move camera up
		CONTROL_EDITOR_CAMERADOWN,		// Editor - move camera down
		CONTROL_EDITOR_CAMERALEFT,		// Editor - move camera left
		CONTROL_EDITOR_CAMERARIGHT,		// Editor - move camera right

		CONTROL_COUNT					// Number of actions defined
	};

	// Custom sound channels for this game

	enum SoundChannels
	{
		CHANNEL_EFFECTS,				// Channel for sound effects
		CHANNEL_SPEECH					// Channel for speach
	};

	// Debug feature to cycle through fill modes

	enum FillModes
	{
		FILL_DEFAULT,
		FILL_WIREFRAME,
		FILL_COLOR,
		FILL_DEBUG
	};

	// Run-time check for correct engine version
	// Would be a little more useful if engine was a DLL

	static const BYTE REQUIRED_ENGINE_VERSION[];

	// Command line switches

	static const WCHAR SZ_CMDSWITCH_PROFILE[];
	static const WCHAR SZ_CMDSWITCH_CONFIGURE[];

	// Settings

	static const WCHAR SZ_PROFILE_SECTION_AUDIO[];
	static const WCHAR SZ_PROFILE_KEY_MUSICVOLUME[];
	static const WCHAR SZ_PROFILE_KEY_EFFECTSVOLUME[];
	static const WCHAR SZ_PROFILE_KEY_SPEECHVOLUME[];
	static const WCHAR SZ_PROFILE_SECTION_STARTUP[];
	static const WCHAR SZ_PROFILE_KEY_SCRIPT[];
	static const WCHAR SZ_PROFILE_KEY_SHOWCONFIGURE[];

	// Theme Variables

	static const WCHAR SZ_VAR_LOGBACKGROUND[];
	static const WCHAR SZ_COLOR_DESKTOP[];
	static const WCHAR SZ_MAT_WIREFRAME[];
	static const WCHAR SZ_MAT_COLORFILL[];
	static const WCHAR SZ_MAT_DEBUG[];

	// Paths

	static const WCHAR SZ_DEFAULT_THEMEPATH[];
	static const WCHAR SZ_DEFAULT_SCRIPTPATH[];

	// Logging Modes
	static const LPCWSTR SZ_LOGMODES[];

	// Controls
	static const LPCWSTR SZ_CONTROLS[];

	// Fill Modes
	static const LPCWSTR SZ_FILLMODE[];

private:
	//
	// Game state
	//

	// Pointer to console screen, if loaded
	ScreenConsole* m_pConsole;

	// Pointer to loading screen, when used
	ScreenProgress* m_pLoading;

	// Loading or saving map instance?
	bool m_bProgressMapInstance;

	// Last progress show failed?
	bool m_bProgressShowFailed;

	// Last progress aborted?
	bool m_bLastProgressAborted;

	// Performance counter of last benchmark (used by cmd_benchmark)
	INT64 m_qwLastBench;

	// Current fill mode
	FillModes m_nFillMode;

	//
	// Game settings
	//

	// Audio

	// Volume of all music resources playing
	float m_fMusicVolume;

	// Volume used by effects sound instances
	float m_fEffectsVolume;

	// Volume used by speech sound instances
	float m_fSpeechVolume;

	// Audio path destination (useful for 3D sound - speakers/headphones)
	Audio::Destinations m_nAudioDest;

	// Startup

	// Startup script path
	String m_strScriptPath;

	// Display a configuration dialog on InitializeEngine?
	bool m_bConfigure;

	// Used to display configure dialog on first-time game launch
	// and then turn it back off. Unless it was set through cmd line param
	bool m_bConfigureExternal;

public:
	Game(void);
	~Game(void);

public:
	//
	// Execution
	//

	virtual void ProcessCommandLine(LPCWSTR pszCmdLine);
	virtual void DeserializeSettings(const IniFile& rProfile);
	virtual void InitializeEngine(void);
	virtual void InitializeInstance(void);

	void RegisterVariables(VariableManager& rVariables);
	void RegisterCommands(CommandManager& rCommands);
	void RegisterClasses(ClassManager& rClasses);

	virtual void RegisterControls(void);
	virtual void ResetControls(void);

	virtual void SerializeSettings(IniFile& rProfile);
	virtual void DestroyInstance(void);

	//
	// Game State
	//

	inline ScreenConsole* GetConsole(void)
	{
		return m_pConsole;
	}

	void SetConsole(ScreenConsole* pConsole);

	inline FillModes GetFillMode(void) const
	{
		return m_nFillMode;
	}

	inline void SetFillMode(FillModes nFillMode)
	{
		m_nFillMode = nFillMode;
	}

	//
	// Game Settings
	//

	// Audio

	inline float GetMusicVolume(void) const
	{
		return m_fMusicVolume;
	}

	void SetMusicVolume(float fMusicVolume);

	inline float GetEffectsVolume(void) const
	{
		return m_fEffectsVolume;
	}

	void SetEffectsVolume(float fEffectsVolume);

	inline float GetSpeechVolume(void) const
	{
		return m_fSpeechVolume;
	}

	void SetSpeechVolume(float fSpeechVolume);

	// Startup

	inline const String& GetScriptPath(void) const
	{
		return m_strScriptPath;
	}

	void SetScriptPath(LPCWSTR pszScriptPath);

	inline bool GetShowConfigure(void) const
	{
		return m_bConfigure;
	}

	inline void SetShowConfigure(bool bShowConfigure)
	{
		m_bConfigure = bShowConfigure;
	}

	inline bool GetConfigureExternal(void) const
	{
		return m_bConfigureExternal;
	}

	//
	// Events
	//

	//virtual void Render(void); // Used for testing & debugging

	virtual void OnKeyDown(int nKeyCode);

	virtual void OnControl(int nControlID);

	virtual void OnThemeChange(Theme& rNewTheme);

	virtual void OnLogStart(void);

	virtual void OnProgress(ProgressTypes nType,
		ProgressSubTypes nSubType,
		int nProgress, int nProgressMax);

	virtual void OnError(const Error& rError);

	//
	// Diagnostics
	//

	virtual void Print(LPCWSTR pszString, PrintTypes nPrintType,
		bool bLine = true);

	virtual DWORD GetMemoryFootprint(void) const;

	static int DebugAllocHook(int nAllocType, void* pUserData,
		size_t nAllocSize, int nBlockType, long nRequestNumber,
		const unsigned char* pszFileName, int nLineNumber);

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Commands
	//

	// Console

	static int cmd_openconsole(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_print(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_clear(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_execute(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_shell(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_vartype(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_mapvar(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_echo(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_dir(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_curdir(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_exit(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_help(Engine& rEngine,
		VariableArray& rParams);

	// Resources
	
	static int cmd_load(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_unload(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_reload(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_makeregion(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_regiontotexture(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_savetexture(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_verifyunicode(Engine& rEngine,
		VariableArray& rParams);

	// Graphics

	static int cmd_customcursor(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_setcustomcursor(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_showcursor(Engine& rEngine,
		VariableArray& rParams);	

	static int cmd_showscreen(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_loadscreen(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_closescreen(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_showfps(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_alignfps(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_showstart(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_minimize(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_pausegame(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_screenshot(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_fillmode(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_qcapture(Engine& rEngine,
		VariableArray& rParams);

	// Audio

	static int cmd_mastervolume(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_mastermute(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_musicvolume(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_effectsvolume(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_speechvolume(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_playsound(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_playmusic(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_stopsound(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_stopmusic(Engine& rEngine,
		VariableArray& rParams);

	// Video

	static int cmd_playvideo(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_stopvideo(Engine& rEngine,
		VariableArray& rParams);

	// Diagnostics

	static int cmd_status(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_list(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_about(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_settings(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_configure(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_logmode(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_wait(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_break(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_crash(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_restart(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_benchmark(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_lasterror(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_errorexit(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_pony(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_test(Engine& rEngine,
		VariableArray& rParams);

	// Game

	static int cmd_map(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_unloadmap(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_savemap(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_loadgame(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_savegame(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_quickload(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_quicksave(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_control(Engine& rEngine,
		VariableArray& rParams);

	// Engine

	static int cmd_engineoption(Engine& rEngine,
		VariableArray& rParams);

	static int cmd_timemultiplier(Engine& rEngine,
		VariableArray& rParams);

	//
	// Helper Functions
	//
	
	static void PrintFlags(DWORD dwFlags, const LPCWSTR* pszarFlags,
		const DWORD* dwarFlags, int nFlagsCount,
		String& strFlagsOut, LPCWSTR pszSepBefore,
		LPCWSTR pszSepAfter);

	static void PrintLastError(Engine& rEngine);
};

} // namespace Hitman2D

#endif // GAME_H