/*------------------------------------------------------------------*\
|
| Game.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D game class implementation
| Created: 06/30/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "resource.h"				// using resource constants
#include "Game.h"					// defining Game, using Theme
#include "Actors.h"					// using ActorPlayer
#include "Maps.h"					// using MapBase
#include "Error.h"					// using Errors
#include "Dialogs.h"				// using ConfigDialogProc
#include "Globals.h"				// using global constants and functions
#include "ScreenLabel.h"			// using ScreenLabel
#include "ScreenListBox.h"			// using ScreenListBox, ScreenScrollBar, ScreenFrame, ScreenButtonEx
#include "ScreenRadioButton.h"		// using ScreenButton, ScreenCheckBox, ScreenRadioButton
#include "ScreenToolbarButton.h"	// using ScreenToolbarButton
#include "ScreenMenu.h"				// using ScreenMenu
#include "ScreenMenuItem.h"			// using ScreenMenuItem
#include "ScreenComboBox.h"			// using ScreenComboBox
#include "ScreenTabControl.h"		// using ScreenTabControl
#include "ScreenProgressBar.h"		// using ScreenProgressBar
#include "ScreenImage.h"			// using ScreenImage
#include "ScreenImageScroller.h"	// using ScreenImageScroller
#include "ScreenConsole.h"			// using ScreenConsole
#include "ScreenCredits.h"			// using ScreenCredits
#include "ScreenExit.h"				// using ScreenExit
#include "ScreenFps.h"				// using ScreenFps
#include "ScreenOverlapped.h"		// using ScreenOverlapped
#include "ScreenPause.h"			// using ScreenPause
#include "ScreenStart.h"			// using ScreenStart
#include "ScreenStats.h"			// using ScreenStats
#include "ScreenProgress.h"			// using ScreenProgress
#include "ScreenOptions.h"			// using ScreenOptions
#include "typeinfo.h"				// using RTTI
#include <psapi.h>					// using GetProcessMemoryInfo
#include <shellapi.h>				// using ShellExecute

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Required Engine version

const BYTE Game::REQUIRED_ENGINE_VERSION[]			= { 2, 1, 0, 0 };

// Command line switches

const WCHAR Game::SZ_CMDSWITCH_PROFILE[]			= L"/profile";
const WCHAR Game::SZ_CMDSWITCH_CONFIGURE[]			= L"/configure";

// Settings

const WCHAR Game::SZ_PROFILE_SECTION_AUDIO[]		= L"audio";
const WCHAR Game::SZ_PROFILE_KEY_MUSICVOLUME[]		= L"musicvolume";
const WCHAR Game::SZ_PROFILE_KEY_EFFECTSVOLUME[]	= L"effectsvolume";
const WCHAR Game::SZ_PROFILE_KEY_SPEECHVOLUME[]		= L"speechvolume";
const WCHAR Game::SZ_PROFILE_SECTION_STARTUP[]		= L"startup";
const WCHAR Game::SZ_PROFILE_KEY_SCRIPT[]			= L"scriptpath";
const WCHAR Game::SZ_PROFILE_KEY_SHOWCONFIGURE[]	= L"showconfigure";

// Theme Variables

const WCHAR Game::SZ_VAR_LOGBACKGROUND[]			= L"log-background";
const WCHAR Game::SZ_COLOR_DESKTOP[]				= L"desktop.color";
const WCHAR Game::SZ_MAT_WIREFRAME[]				= L"wireframe.material";
const WCHAR Game::SZ_MAT_COLORFILL[]				= L"colorfill.material";
const WCHAR Game::SZ_MAT_DEBUG[]					= L"debug.material";

// Paths

const WCHAR Game::SZ_DEFAULT_THEMEPATH[]			= L".\\screens\\default.the";
const WCHAR Game::SZ_DEFAULT_SCRIPTPATH[]			= L".\\scripts\\startup.thc";

// Logging Modes

const LPCWSTR Game::SZ_LOGMODES[] =			{
													L"disable",
													L"text",
													L"html"
											};
// Controls

const LPCWSTR Game::SZ_CONTROLS[] =			{
													L"game.pause",
													L"game.screenshot",
													L"debug.console",
													L"debug.exit",
													L"debug.fillmode",
													L"debug.test",
													L"debug.printstats",
													L"editor.cameraup",
													L"editor.cameradown",
													L"editor.cameraleft",
													L"editor.cameraright"
											};

// Fill Modes

const LPCWSTR Game::SZ_FILLMODE[] =			{
													L"default",
													L"wireframe",
													L"color",
													L"debug"
											};


/*----------------------------------------------------------*\
| Game implementation
\*----------------------------------------------------------*/

Game::Game(void):	Client(IDI_HITMAN2D),					
					m_pConsole(NULL),
					m_pLoading(NULL),
					m_bProgressMapInstance(false),
					m_bProgressShowFailed(false),
					m_bLastProgressAborted(false),
					m_qwLastBench(0),
					m_nFillMode(FILL_DEFAULT),
					m_fMusicVolume(0.0f),
					m_fEffectsVolume(0.0f),
					m_fSpeechVolume(0.0f),
					m_nAudioDest(Audio::DESTINATION_SPEAKERS),
					m_bConfigureExternal(false),
					m_bConfigure(false)
{
#ifdef _DEBUG

	// Enable run-time memory check

    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// Enable custom allocation hook for leak tracking

	_CrtSetAllocHook(Game::DebugAllocHook);

#endif
}

Game::~Game(void)
{
	Empty();
}

void Game::ProcessCommandLine(LPCWSTR pszCmdLine)
{
	// Check Engine version

	if (Engine::GetVersion() != *(DWORD*)REQUIRED_ENGINE_VERSION)
		throw m_Engine.GetErrors().Push(ErrorGame(ErrorGame::ENGINEVERSION,
			__FUNCTIONW__, *(DWORD*)REQUIRED_ENGINE_VERSION));

	// Parse command line

	Client::ProcessCommandLine(pszCmdLine);

	// Check for switches

	bool bProfileSwitch = false;

	for(StringArrayIterator pos = m_arCommandArgs.begin();
		pos != m_arCommandArgs.end();
		pos++)
	{
		if (false == bProfileSwitch &&
			(*pos) == SZ_CMDSWITCH_PROFILE)
		{
			// The next argument must be profile path

			pos++;

			if (pos != m_arCommandArgs.end())
				SetProfilePath(*pos);

			bProfileSwitch = true;
		}
		else if ((*pos) == SZ_CMDSWITCH_CONFIGURE)
		{
			m_bConfigure = true;
			m_bConfigureExternal = true;
		}
	}
}

void Game::DeserializeSettings(const IniFile& rProfile)
{
	//
	// Read Game settings
	//

	Client::DeserializeSettings(rProfile);

	//
	// Read audio settings
	//

	// Read music volume

	m_fMusicVolume = rProfile.GetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_MUSICVOLUME, 1.0f);

	// Read effects volume

	m_fEffectsVolume = rProfile.GetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_EFFECTSVOLUME, 1.0f);

	// Read speech volume

	m_fSpeechVolume = rProfile.GetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_SPEECHVOLUME, 1.0f);

	//
	// Read Startup Settings
	//

	// Read script path

	SetScriptPath(rProfile.GetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_SCRIPT, NULL));

	// Read showconfigure

	if (false == m_bConfigure && rProfile.GetBoolKeyValue(
		SZ_PROFILE_SECTION_STARTUP, SZ_PROFILE_KEY_SHOWCONFIGURE, false))
	{
		m_bConfigure = true;
	}

	// Set default theme path if none set

	if (m_strThemePath.IsEmpty())
		m_strThemePath = SZ_DEFAULT_THEMEPATH;
}

void Game::RegisterControls(void)
{
	for(int n = 0;
		n < (sizeof(SZ_CONTROLS) / sizeof(LPCWSTR));
		n++)
	{
		m_Controls.RegisterControl(SZ_CONTROLS[n]);
	}

	Print(L"registered controls.", PRINT_INFO);
}

void Game::ResetControls(void)
{
	m_Controls.Bind(CONTROL_GAME_PAUSE, VK_PAUSE);
	m_Controls.Bind(CONTROL_GAME_SCREENSHOT, VK_F11);
	m_Controls.Bind(CONTROL_DEBUG_CONSOLE, VK_OEM_3);
	m_Controls.Bind(CONTROL_DEBUG_EXIT, VK_F12);
	m_Controls.Bind(CONTROL_DEBUG_FILLMODE, 'T');
	m_Controls.Bind(CONTROL_DEBUG_TEST, VK_F10);
	m_Controls.Bind(CONTROL_DEBUG_PRINTSTATS, VK_F11);
	m_Controls.Bind(CONTROL_GAME_SCREENSHOT, VK_F9);

	Print(L"binded default controls.", PRINT_INFO);
}

void Game::SerializeSettings(IniFile& rProfile)
{
	//
	// Save Game settings
	//

	Client::SerializeSettings(rProfile);

	//
	// Save audio settings
	//

	// Save effects volume

	rProfile.SetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_EFFECTSVOLUME, m_fEffectsVolume);

	// Save speech volume

	rProfile.SetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_SPEECHVOLUME, m_fSpeechVolume);

	// Save music volume

	rProfile.SetFloatKeyValue(SZ_PROFILE_SECTION_AUDIO,
		SZ_PROFILE_KEY_MUSICVOLUME, m_fMusicVolume);

	//
	// Write Startup Settings
	//

	// Write script path

	rProfile.SetStringKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_SCRIPT, m_strScriptPath);

	// Write show configure

	rProfile.SetBoolKeyValue(SZ_PROFILE_SECTION_STARTUP,
		SZ_PROFILE_KEY_SHOWCONFIGURE, m_bConfigure);
}

void Game::InitializeEngine(void)
{
	//
	// Display configuration dialog if needed
	//

	if (false == m_bConfigure &&
	   (FALSE == PathFileExists(m_strProfilePath) ||
	   ControlManager::IsKeyPressed(VK_SHIFT) == true))
	{
		// Profile doesn't exist - need to show configuration dialog
		// to allow user to select first-time run settings

		m_bConfigure = true;
		m_bConfigureExternal = true;
	}

	if (true == m_bConfigure)
	{
		PrintEx(L"displaying configuration dialog...", PRINT_INFO, true);

		DialogConfigure configureDialog(this);

		switch(configureDialog.Show())
		{
		case IDSAVEEXIT:
			{
				// Save settings

				IniFile profile;

				try
				{
					SerializeSettings(profile);				
					profile.Serialize(m_strProfilePath);
				}

				catch(Error& rError)
				{
					// Ignore errors if can't save settings

					UNREFERENCED_PARAMETER(rError);
				}

				// Exit game

				PrintEx(L"configuration dialog dismissed, save & exit.",
					PRINT_INFO, true);

				throw ErrorGame(ErrorGame::CONFIGUREEXIT, __FUNCTIONW__);
			}
			break;
		case IDCANCEL:
			{
				// Exit game

				PrintEx(L"configuration dialog dismissed, exit",
					PRINT_INFO, true);

				throw ErrorGame(ErrorGame::CONFIGUREEXIT, __FUNCTIONW__);
			}
			break;
		}

		PrintEx(L"configuration dialog dismissed, continue.",
			PRINT_INFO, true);
	}

	PrintEx(L"initializing engine using profile settings...",
		PRINT_INFO, true);

	Client::InitializeEngine();

	PrintEx(L"engine initialized successfully.", PRINT_INFO, true);
}

void Game::InitializeInstance(void)
{
	Client::InitializeInstance();

	//
	// Begin data initialization
	//

	PrintEx(L"initializing game data...", PRINT_INFO, true);

	//
	// Register variables
	//

	RegisterVariables(m_Engine.GetVariables());

	Print(L"added variables.", PRINT_INFO);

	//
	// Register commands
	//

	RegisterCommands(m_Engine.GetCommands());

	Print(L"registered commands.", PRINT_INFO);

	//
	// Register Classes
	//

	RegisterClasses(m_Engine.GetClasses());

	Print(L"registered classes.", PRINT_INFO);	

	//
	// Execute startup script
	//

	if (m_strScriptPath.IsEmpty() == false)
	{
		PrintEx(L"executing startup script...", PRINT_INFO, true);

		VariableArray params;
		params.resize(1);

		params[0].SetStringValue(m_strScriptPath);

		cmd_execute(m_Engine, params);
	}

	//
	// End data initialization
	//

	PrintEx(L"game data initialized successfully.", PRINT_INFO, true);

	// Unmute diagnostics

	PrintMute(false);
}

void Game::RegisterCommands(CommandManager& rCommands)
{
	rCommands.Register(L"openconsole", cmd_openconsole);
	rCommands.Register(L"print", cmd_print);
	rCommands.Register(L"clear", cmd_clear);
	rCommands.Register(L"cls", cmd_clear);
	rCommands.Register(L"execute", cmd_execute);
	rCommands.Register(L"shell", cmd_shell);
	rCommands.Register(L"vartype", cmd_vartype);
	rCommands.Register(L"mapvar", cmd_mapvar);
	rCommands.Register(L"echo", cmd_echo);
	rCommands.Register(L"dir", cmd_dir);
	rCommands.Register(L"curdir", cmd_curdir);
	rCommands.Register(L"exit", cmd_exit);
	rCommands.Register(L"restart", cmd_restart);
	rCommands.Register(L"help", cmd_help);
	rCommands.Register(L"load", cmd_load);
	rCommands.Register(L"unload", cmd_unload);
	rCommands.Register(L"reload", cmd_reload);
	rCommands.Register(L"makeregion", cmd_makeregion);
	rCommands.Register(L"regiontotexture", cmd_regiontotexture);
	rCommands.Register(L"verifyunicode", cmd_verifyunicode);
	rCommands.Register(L"showcustomcursor", cmd_customcursor);
	rCommands.Register(L"setcustomcursor", cmd_setcustomcursor);
	rCommands.Register(L"showcursor", cmd_showcursor);
	rCommands.Register(L"showscreen", cmd_showscreen);
	rCommands.Register(L"loadscreen", cmd_loadscreen);
	rCommands.Register(L"closescreen", cmd_closescreen);
	rCommands.Register(L"showfps", cmd_showfps);
	rCommands.Register(L"alignfps", cmd_alignfps);
	rCommands.Register(L"showstart", cmd_showstart);
	rCommands.Register(L"minimize", cmd_minimize);
	rCommands.Register(L"pausegame", cmd_pausegame);
	rCommands.Register(L"screenshot", cmd_screenshot);
	rCommands.Register(L"fillmode", cmd_fillmode);
	rCommands.Register(L"mastervolume", cmd_mastervolume);
	rCommands.Register(L"mastermute", cmd_mastermute);
	rCommands.Register(L"musicvolume", cmd_musicvolume);
	rCommands.Register(L"effectsvolume", cmd_effectsvolume);
	rCommands.Register(L"speechvolume", cmd_speechvolume);
	rCommands.Register(L"playsound", cmd_playsound);
	rCommands.Register(L"playmusic", cmd_playmusic);
	rCommands.Register(L"playvideo", cmd_playvideo);
	rCommands.Register(L"stopsound", cmd_stopsound);
	rCommands.Register(L"stopmusic", cmd_stopmusic);
	rCommands.Register(L"stopvideo", cmd_stopvideo);
	rCommands.Register(L"status", cmd_status);
	rCommands.Register(L"list", cmd_list);	
	rCommands.Register(L"about", cmd_about);
	rCommands.Register(L"pony", cmd_pony);
	rCommands.Register(L"settings", cmd_settings);
	rCommands.Register(L"configure", cmd_configure);
	rCommands.Register(L"logmode", cmd_logmode);
	rCommands.Register(L"wait", cmd_wait);
	rCommands.Register(L"break", cmd_break);	
	rCommands.Register(L"crash", cmd_crash);
	rCommands.Register(L"benchmark", cmd_benchmark);
	rCommands.Register(L"lasterror", cmd_lasterror);
	rCommands.Register(L"errorexit", cmd_errorexit);
	rCommands.Register(L"test", cmd_test);
	rCommands.Register(L"engine.option", cmd_engineoption);
	rCommands.Register(L"timemultiplier", cmd_timemultiplier);
	rCommands.Register(L"map", cmd_map);
	rCommands.Register(L"unloadmap", cmd_unloadmap);
	rCommands.Register(L"savemap", cmd_savemap);
	rCommands.Register(L"loadgame", cmd_loadgame);
	rCommands.Register(L"savegame", cmd_savegame);
	rCommands.Register(L"quickload", cmd_quickload);
	rCommands.Register(L"quicksave", cmd_quicksave);
	rCommands.Register(L"control", cmd_control);
	rCommands.Register(L"savetexture", cmd_savetexture);
	rCommands.Register(L"qcapture", cmd_qcapture);
}

void Game::RegisterVariables(VariableManager& rVariables)
{
	// Register variables

#ifdef _DEBUG
	rVariables.Add(L"debug_build")->SetBoolValue(true);
#endif
}

void Game::RegisterClasses(ClassManager& rClasses)
{
	// Register screen classes

	rClasses.Register(ScreenFrame::SZ_CLASS, ScreenFrame::CreateInstance);
	rClasses.Register(ScreenImage::SZ_CLASS, ScreenImage::CreateInstance);
	rClasses.Register(ScreenImageScroller::SZ_CLASS, ScreenImageScroller::CreateInstance);
	rClasses.Register(ScreenLabel::SZ_CLASS, ScreenLabel::CreateInstance);
	rClasses.Register(ScreenButton::SZ_CLASS, ScreenButton::CreateInstance);
	rClasses.Register(ScreenButtonEx::SZ_CLASS, ScreenButtonEx::CreateInstance);
	rClasses.Register(ScreenCheckBox::SZ_CLASS, ScreenCheckBox::CreateInstance);
	rClasses.Register(ScreenRadioButton::SZ_CLASS, ScreenRadioButton::CreateInstance);
	rClasses.Register(ScreenTabControl::SZ_CLASS, ScreenTabControl::CreateInstance);
	rClasses.Register(ScreenProgressBar::SZ_CLASS, ScreenProgressBar::CreateInstance);
	rClasses.Register(ScreenScrollBar::SZ_CLASS, ScreenScrollBar::CreateInstance);
	rClasses.Register(ScreenListBox::SZ_CLASS, ScreenListBox::CreateInstance);
	rClasses.Register(ScreenComboBox::SZ_CLASS, ScreenComboBox::CreateInstance);
	rClasses.Register(ScreenToolbarButton::SZ_CLASS, ScreenToolbarButton::CreateInstance);
	rClasses.Register(ScreenMenu::SZ_CLASS, ScreenMenu::CreateInstance);
	rClasses.Register(ScreenMenuItem::SZ_CLASS, ScreenMenuItem::CreateInstance);
	rClasses.Register(ScreenOverlapped::SZ_CLASS, ScreenOverlapped::CreateInstance);
	rClasses.Register(ScreenFps::SZ_CLASS, ScreenFps::CreateInstance);
	rClasses.Register(ScreenConsole::SZ_CLASS, ScreenConsole::CreateInstance);
	rClasses.Register(ScreenCredits::SZ_CLASS, ScreenCredits::CreateInstance);
	rClasses.Register(ScreenPause::SZ_CLASS, ScreenPause::CreateInstance);
	rClasses.Register(ScreenStart::SZ_CLASS, ScreenStart::CreateInstance);
	rClasses.Register(ScreenProgress::SZ_CLASS, ScreenProgress::CreateInstance);
	rClasses.Register(ScreenExit::SZ_CLASS, ScreenExit::CreateInstance);
	rClasses.Register(ScreenStats::SZ_CLASS, ScreenStats::CreateInstance);
	rClasses.Register(ScreenOptions::SZ_CLASS, ScreenOptions::CreateInstance);

	// Register map classes

	rClasses.Register(MapBase::SZ_CLASS, MapBase::CreateInstance);

	// Register actor classes

	//rClasses.Register(L"player", ActorPlayer::CreateInstance);
	//rClasses.Register(L"prop", ActorProp::CreateInstance);
}

void Game::DestroyInstance(void)
{
	PrintEx(L"destroying game data...", PRINT_INFO, true);

	PrintEx(L"game data destroyed successfully.", PRINT_INFO, true);
}

void Game::OnError(const Error& rError)
{
	UNREFERENCED_PARAMETER(rError);

	// Handle recoverable errors

	if (rError.GetCode() == ErrorGame::CONFIGUREEXIT ||
		rError.GetCode() == ErrorGame::CUSTOM)
			return;

	// If running full screen, hide game window and display
	// error on top of desktop

	bool bOnTopOfWindow = true;

	if (true == m_bFullScreen && m_Engine.GetGameWindow() != NULL)
	{
		ShowWindow(m_Engine.GetGameWindow(), SW_HIDE);

		bOnTopOfWindow = false;
	}

	// Show error message box for every error on the error stack

	if (m_pLogFile != NULL)
		m_pLogFile->EnableDateTime(true);

	DialogError dialogError;

	if (m_Engine.GetErrors().GetCount() > 0)
	{
		bool bContinue = true;

		while(m_Engine.GetErrors().GetCount() > 0)
		{
			// Print description

			Print(m_Engine.GetErrors().GetLastError().GetDescription(),
				PRINT_ERROR);

			// Display error dialog

			if (true == bContinue)
			{
				dialogError.Show(bOnTopOfWindow ? m_Engine.GetGameWindow() :
					NULL, m_Engine.GetErrors().GetLastError().GetDescription());

				bContinue = !dialogError.GetNotAgain();
			}

			// Remove it from the stack

			m_Engine.GetErrors().Pop();
		}
	}
	else
	{
		// Print description

		Print(rError.GetDescription(), PRINT_ERROR);

		// Display error dialog

		dialogError.Show(true == bOnTopOfWindow ?
			m_Engine.GetGameWindow() : NULL, rError.GetDescription());
	}

	if (m_pLogFile != NULL)
		m_pLogFile->EnableDateTime(false);

	Exit();
}

void Game::SetMusicVolume(float fMusicVolume)
{
	m_fMusicVolume = fMusicVolume;

	// Set volume of all playing music resources

	for(ResourceManager<Music>::Iterator pos = m_Engine.GetMusic().GetBeginPos();
		pos != m_Engine.GetMusic().GetEndPos();
		pos++)
	{
		Music* pMusic = pos->second;

		if (NULL == pMusic) continue;

		if (pMusic->IsFlagSet(Music::PLAYING) == true)
			pMusic->SetVolume(m_fMusicVolume);
	}
}

void Game::SetEffectsVolume(float fEffectsVolume)
{
	m_fEffectsVolume = fEffectsVolume;

	m_Engine.GetAudio().SetChannelVolume(CHANNEL_EFFECTS, fEffectsVolume);
}

void Game::SetSpeechVolume(float fSpeechVolume)
{
	m_fSpeechVolume = fSpeechVolume;

	m_Engine.GetAudio().SetChannelVolume(CHANNEL_SPEECH, fSpeechVolume);
}

void Game::SetScriptPath(LPCWSTR pszScriptPath)
{
	if (pszScriptPath != NULL)
		m_strScriptPath = pszScriptPath;
	else
		m_strScriptPath = SZ_DEFAULT_SCRIPTPATH;
}

void Game::SetConsole(ScreenConsole* pConsole)
{
	m_pConsole = pConsole;

	try
	{
		if (IsPrintMuted() == true && pConsole != NULL)
			PrintMute(false);
	}

	catch(Error&) {}
}

int Game::DebugAllocHook(int nAllocType,
						 void* pUserData,
						 size_t nAllocSize,
						 int nBlockType,
						 long nRequestNumber,
						 const unsigned char* pszFileName,
						 int nLineNumber)
{
	// Insert code here to check for a specific allocation

	//_ASSERT(nRequestNumber != 35647);

	return TRUE;
}

/*
void Game::Render(void)
{
	// Test

	Graphics& rGraphics = m_Engine.GetGraphics();

	rGraphics.Clear(m_Engine.GetBackColorConst());

	rGraphics.BeginScene();

	rGraphics.BeginBatch();

	m_Engine.Render();

	MaterialInstance testing(m_Engine.GetMaterials().Load(L"shared"),
		m_Engine.GetTextures().Load(L"screens\\cursors"),
		Rect(0, 0, 64, 4s6));

	rGraphics.RenderQuad(testing, Vector2(100, 100));

	rGraphics.EndBatch();

	rGraphics.EndScene();

	rGraphics.Present();
}
*/

void Game::OnKeyDown(int nKeyCode)
{
	Client::OnKeyDown(nKeyCode);

	try
	{
		// Stop currently playing video with certain key presses

		if (m_Engine.GetCurrentVideo() != NULL &&
		  (VK_ESCAPE == nKeyCode ||
		   VK_RETURN == nKeyCode ||
		   VK_SPACE == nKeyCode))
		{
			m_Engine.GetCurrentVideo()->Stop();
		}		
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(m_Engine);
	}
}

void Game::OnControl(int nControlID)
{
	try
	{
		switch(nControlID)
		{
		case CONTROL_DEBUG_CONSOLE:
			{
				if (m_pConsole != NULL)
					m_pConsole->Toggle();
			}
			break;
		case CONTROL_DEBUG_EXIT:
			{
				Exit();
			}
			break;
		case CONTROL_GAME_PAUSE:
			{
				if (m_Engine.GetCurrentMapConst() != NULL)
					m_Engine.GetCommands().ExecuteStatement(
						L"showscreen \"pause\" 1006");
			}
			break;
		case CONTROL_GAME_SCREENSHOT:
			{
				m_Engine.GetCommands().ExecuteStatement(L"screenshot");
			}
			break;
		case CONTROL_DEBUG_FILLMODE:
			{
				if (m_Engine.GetScreens().GetFocusScreen() == NULL ||
					m_Engine.GetScreens().GetFocusScreen()->IsFlagSet(Screen::DISABLED))
					m_Engine.GetCommands().ExecuteStatement(L"fillmode");
			}
			break;
		case CONTROL_DEBUG_TEST:
			{
				m_Engine.GetCommands().ExecuteStatement(L"test");
			}
			break;
		case CONTROL_DEBUG_PRINTSTATS:
			{
				Graphics& rGraphics = m_Engine.GetGraphics();

				String strStats;

				strStats.Format(
					L"fps:\t%d  mspf:\t%.3f\n"
					L"triangles:\t%d\n"
					L"lines:\t\t\t%d\n"
					L"points:\t\t\t%d\n\n"
					L"batches:\t\t%d\n\n"
					L"renderables per frame:\t%d\n"
					L"max prims per batch:\t%d\n\n"
					L"state changes:\t\t%d\n"
					L"filtered changes:\t%d",

					rGraphics.GetFPS(),
					m_Engine.GetFrameTime() * 1000.0f,
					rGraphics.GetTriangleCount(),
					rGraphics.GetLineCount(),
					rGraphics.GetPointCount(),
					rGraphics.GetBatchCount(),
					rGraphics.GetRenderableCount(),
					rGraphics.GetMaxPrimitivesPerBatch(),
					rGraphics.GetStates()->GetStateChangeCount(),
					rGraphics.GetStates()->GetFilteredStateChangeCount()
				);

				PrintEx(strStats, PRINT_INFO);
			}
			break;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(m_Engine);
	}
}

void Game::OnThemeChange(Theme& rNewTheme)
{
	ThemeStyle* pSharedStyle = rNewTheme.GetStyle(SZ_SHARED_THEMESTYLE);

	if (NULL == pSharedStyle)
		return;

	// Set engine clear color

	if (pSharedStyle->GetColor(SZ_COLOR_DESKTOP) != NULL)
		m_Engine.SetBackColor(*pSharedStyle->GetColor(SZ_COLOR_DESKTOP));
}

DWORD Game::GetMemoryFootprint(void) const
{
	return Client::GetMemoryFootprint() -
		sizeof(Client) +
		sizeof(Game) +
		(DWORD)m_strScriptPath.GetLengthBytes();
}

void Game::Print(LPCWSTR pszString, PrintTypes nPrintType, bool bLine)
{
	Client::Print(pszString, nPrintType, bLine);

	if (true == m_bMute)
		return;

	if (m_pConsole != NULL)
		m_pConsole->Print(pszString, nPrintType, bLine);
}

void Game::OnLogStart(void)
{
	// Set some properties

	if (LOG_HTML == m_nLogMode)
	{
		LogFileHTML* pEx = dynamic_cast<LogFileHTML*>(m_pLogFile);
		
		if (NULL == pEx)
			throw m_Engine.GetErrors().Push(Error::INVALID_PTR,
				__FUNCTIONW__, L"m_pLogFile");

		WCHAR szLogDir[MAX_PATH] = {0};
		wcscpy_s(szLogDir, MAX_PATH, m_strLogFilePath);
		PathRemoveFileSpec(szLogDir);

		if (PathIsRelative(szLogDir) == TRUE)
		{
			LPCWSTR pszDirs[] = { m_strBaseDir, NULL };
			PathResolve(szLogDir, pszDirs, PRF_FIRSTDIRDEF);
		}		

		ThemeStyle* pSharedStyle =
			m_Engine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

		if (pSharedStyle != NULL &&
		   pSharedStyle->GetVariable(SZ_VAR_LOGBACKGROUND) != NULL)
		{
			WCHAR szImagePath[MAX_PATH] = {0};
			wcscpy_s(szImagePath, MAX_PATH, m_strBaseDir);

			PathAppend(szImagePath,
				pSharedStyle->GetVariable(SZ_VAR_LOGBACKGROUND)->GetStringValue());

			WCHAR szRelPath[MAX_PATH] = {0};

			PathRelativePathTo(szRelPath, szLogDir,
				FILE_ATTRIBUTE_DIRECTORY,
				szImagePath, FILE_ATTRIBUTE_NORMAL);

			pEx->SetBackground(szRelPath);
			pEx->SetBackColor(D3DCOLOR_XRGB(149, 0, 0));
			pEx->SetBackgroundCenter(true);
			pEx->SetBackgroundFixed(true);
			pEx->SetBackgroundRepeat(false);	
		}

		if (pSharedStyle != NULL)
		{
			for(int n = 0; n < PRINT_CLEAR; n++)
			{
				const Color* pColor = pSharedStyle->GetColor(SZ_MESSAGECOLORS[n]);

				if (pColor != NULL)
					pEx->SetTextColor(static_cast<PrintTypes>(n), *pColor);
			}
		}
	}
}

void Game::OnProgress(ProgressTypes nType,
					  ProgressSubTypes nSubType,
					  int nProgress,
					  int nProgressMax)
{
	// Do not display progress for loading or saving map instance

	if (Client::PROGRESS_MAP_INSTANCE == nSubType)
		m_bProgressMapInstance = !(nProgress == nProgressMax);	

	if (true == m_bProgressMapInstance)
		return;

	// Clear last progress aborted flag if starting new sequence

	if (PROGRESS_SESSION == nType || PROGRESS_MAP == nType)
		m_bLastProgressAborted = false;

	// Display progress dialog if not displayed

	if (NULL == m_pLoading &&
	   false == m_bLastProgressAborted &&
	   false == m_bProgressShowFailed)
	{
		m_pLoading = dynamic_cast<ScreenProgress*>
			(m_Engine.GetScreens().Show(L"progress"));
	}

	// Notify

	if (NULL == m_pLoading)
	{
		m_bProgressShowFailed = true;
		return;
	}
	
	m_pLoading->SetProgress(nType, nSubType, nProgress, nProgressMax);

	if (m_pLoading->IsAborted())
	{
		m_bLastProgressAborted = true;
		m_pLoading = NULL;

		throw m_Engine.GetErrors().Push(Error::CLIENT_ABORT, __FUNCTIONW__);
	}
}

void Game::Empty(void)
{
	Client::Empty();
}

int Game::cmd_help(Engine& rEngine, VariableArray& rParams)
{
	StringTable* pHelpStrings =
		rEngine.GetStrings().Load(L".\\strings\\console.tht");

	if (NULL == pHelpStrings)
	{
		PrintLastError(rEngine);

		return FALSE;
	}

	if (rParams.empty() == true)
	{
		// Print main topic

		rEngine.Print(pHelpStrings->GetString(L"main"));
	}
	else
	{
		if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
		{
			rEngine.PrintError(L"invalid param type (1): expected enum.");

			return FALSE;
		}

		rEngine.Print(pHelpStrings->GetString(rParams[0].GetStringValue()));
	}
		
	return TRUE;
}

int Game::cmd_exit(Engine& rEngine, VariableArray& rParams)
{
	if (rEngine.GetClientInstance() != NULL)		
		rEngine.GetClientInstance()->Exit();

	return TRUE;
}

int Game::cmd_wait(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: int waittime expected.");
		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_INT &&
		rParams[0].GetVarType() != Variable::TYPE_DWORD)
	{
		rEngine.PrintError(L"invalid param type (1): expected int or dword.");

		return FALSE;
	}

	Sleep(DWORD(rParams[0].GetDwordValue()));

	return TRUE;
}

int Game::cmd_clear(Engine& rEngine, VariableArray& rParams)
{
	rEngine.PrintClear();

	return TRUE;
}

int Game::cmd_status(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: expected enum {"
			L"engine, map, memory, d3d } item.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): expected enum");

		return FALSE;
	}
	
	String str;
	int n;

	if (rParams[0].GetString() == L"engine")
	{
		// Print general Engine info

		rEngine.PrintInfo(
			L"\nBEGIN ENGINE STATUS\n\n"
			L"   run time          = %f seconds\n"
			L"   session started   = %s\n"
			L"   session paused    = %s\n"				   
			L"   session time      = %f seconds\n"
			L"   last frame time   = %f seconds (%.2f ms)\n"
			L"   frame rate        = %d frames per second\n",

			rEngine.GetRunTime(),
			Variable(rEngine.IsSessionStarted()).ToString(),
			Variable(rEngine.IsSessionPaused()).ToString(),
			rEngine.GetTime(),
			rEngine.GetFrameTime(),
			rEngine.GetFrameTime() * 1000.0f,
			rEngine.GetGraphics().GetFPS());

		// Print Engine options

		for(n = 0; n < Engine::OPTION_COUNT; n++)
		{
			switch(n)
			{
			case Engine::OPTION_TILE_SIZE:
			case Engine::OPTION_NET_PORT:
			case Engine::OPTION_STREAM_CACHE_DURATION:
			case Engine::OPTION_RESOURCE_CACHE_DURATION:
			case Engine::OPTION_STREAM_CACHE_FREQUENCY:
			case Engine::OPTION_RESOURCE_CACHE_FREQUENCY:
			case Engine::OPTION_SOUND_CACHE_FREQUENCY:
			case Engine::OPTION_MAX_BATCH_PRIM:
				str.Format(L"   %-24s = %d", SZ_ENGINE_OPTIONS[n],
					rEngine.GetOption((Engine::Options)n));
				break;
			case Engine::OPTION_EFFECT_COMPILE_FLAGS:
				str.Format(L"   %-24s = 0x%x", SZ_ENGINE_OPTIONS[n],
					rEngine.GetOption(Engine::OPTION_EFFECT_COMPILE_FLAGS));
				break;
			case Engine::OPTION_AUDIO_DESTINATION:
				str.Format(L"   %-24s = %s", SZ_ENGINE_OPTIONS[n],
				SZ_AUDIO_DEST[rEngine.GetOption(Engine::OPTION_AUDIO_DESTINATION)]);
				break;
			default:
				str.Format(L"   %-24s = %s", SZ_ENGINE_OPTIONS[n],
					rEngine.GetOption((Engine::Options)n) ? L"true" : L"false");
				break;
			}

			rEngine.PrintInfo(str);
		}

		// Print resource counts

		rEngine.PrintInfo(
			L"\n"
			L"   timers            = %d\n"
			L"   commands          = %d\n"
			L"   variables         = %d\n"
			L"   sounds playing    = %d\n"
			L"   top level screens = %d\n"
			L"\nEND ENGINE STATUS",

			rEngine.GetTimers().GetCount(),
			rEngine.GetCommandsConst().GetCount(),
			rEngine.GetVariablesConst().GetCount(),
			rEngine.GetAudio().GetSoundInstances().GetCount(),
			rEngine.GetScreensConst().GetCount());
	}
	else if (rParams[0].GetString() == L"map")
	{
		/*
		TileMap* pMap = rEngine.GetCurrentMap();

		if (NULL == pMap)
		{
			rEngine.Print(L"no current map.", PRINT_ERROR);
			return FALSE;
		}

		LPCWSTR pszUnits[4] = {0};

		float fMap = FormatMemory(pMap->GetMemoryFootprint(), &pszUnits[0]);
		float fTiles = FormatMemory(pMap->GetWidth() * pMap->GetHeight() *
			sizeof(Tile), &pszUnits[1]);
		float fCachedTiles = FormatMemory(pMap->GetCameraTileRange().GetTileCount() *
			sizeof(TileCached), &pszUnits[2]);
		float fActors = FormatMemory(pMap->GetActorsMemoryFootprint(), &pszUnits[3]);

		String strFlags;

		PrintFlags(pMap->GetFlags(), SZ_MAPFLAGS, DW_MAPFLAGS,
			sizeof(DW_MAPFLAGS) / sizeof(DWORD), strFlags,
			L"                      ", L"\n");

		str.Format(L"\nBEGIN CURRENT MAP STATUS\n\n"
			L"   name             = \"%s\"\n\n"
			L"   flags:\n%s\n\n"
			L"   est. memory used = %.3f %s\n\n"
			L"   width            = %d\n"
			L"   height           = %d\n"
			L"   tiles loaded     = %d (%.3f %s)\n"
			L"   tiles cached     = %d (%.3f %s)\n\n"
			L"   actors           = %d (%.3f %s)\n"
			L"   player actor     = \"%s\"\n"
			L"   actors cached    = %d\n\n"
			L"   camera position  = %.3f, %.3f\n"
			L"   camera size      = %.3f, %.3f\n\n"
			L"   variables        = %d\n\n"
			L"   texturesheets    = %d\n"
			L"   animations       = %d\n"
			L"   sounds           = %d\n"
			L"   music            = %d\n\n"
			L"END CURRENT MAP STATUS",

			pMap->GetName(),
			strFlags,
			fMap, pszUnits[0],
			pMap->GetWidth(),
			pMap->GetHeight(),
			pMap->GetWidth() * pMap->GetHeight(), fTiles, pszUnits[1],
			pMap->GetCameraTileRange().GetTileCount(), fCachedTiles, pszUnits[2],
			pMap->GetActorCount(), fActors, pszUnits[3],
			pMap->GetPlayerActorConst() ?
				pMap->GetPlayerActorConst()->GetName() : L"(null)",
			int(pMap->GetVisibleActors().size()),
			pMap->GetCameraPos().tx, pMap->GetCameraPos().ty,
			pMap->GetCameraSize().tx, pMap->GetCameraSize().ty,
			pMap->GetVariables().GetCount(),
			pMap->GetTextureSheetCount(),
			pMap->GetAnimationCount(),
			pMap->GetSoundCount(),
			pMap->GetMusicCount());

		rEngine.Print(str, PRINT_INFO);
		*/
	}
	else if (rParams[0].GetString() == L"memory")
	{
		// Display memory status

		MEMORYSTATUS ms = {0};
		PROCESS_MEMORY_COUNTERS mc = {0};

		GlobalMemoryStatus(&ms);

		GetProcessMemoryInfo(GetCurrentProcess(), &mc,
			sizeof(PROCESS_MEMORY_COUNTERS));

		LPCWSTR pszUnits[32] = { NULL };

		float fTotalPhysical = FormatMemory(DWORD(ms.dwTotalPhys), &pszUnits[0]);
		float fFreePhysical = FormatMemory(DWORD(ms.dwAvailPhys), &pszUnits[1]);
		float fTotalVirtual = FormatMemory(DWORD(ms.dwTotalVirtual), &pszUnits[2]);
		float fFreeVirtual = FormatMemory(DWORD(ms.dwAvailVirtual), &pszUnits[3]);
		float fTotalPagefile = FormatMemory(DWORD(ms.dwTotalPageFile), &pszUnits[4]);
		float fFreePagefile = FormatMemory(DWORD(ms.dwAvailPageFile), &pszUnits[5]);

		float fWorkingSet = FormatMemory(DWORD(mc.WorkingSetSize), &pszUnits[6]);
		float fPeakWorkingSet = FormatMemory(DWORD(mc.PeakWorkingSetSize), &pszUnits[7]);
		float fPagefileUsage = FormatMemory(DWORD(mc.PagefileUsage), &pszUnits[8]);
		float fPeakPagefileUsage = FormatMemory(DWORD(mc.PeakPagefileUsage), &pszUnits[9]);

		float fAdapterMemory = FormatMemory(rEngine.GetGraphics().GetAdapterMemory(), &pszUnits[27]);

		DWORD dwEngine = rEngine.GetMemoryFootprint();

		float fGame = FormatMemory(rEngine.GetClientInstance()->GetMemoryFootprint() -
			dwEngine, &pszUnits[10]);

		float fEngine = FormatMemory(dwEngine, &pszUnits[11]);		
		float fScreens = FormatMemory(rEngine.GetScreens().GetMemoryFootprint(),
			&pszUnits[12]);

		float fMaps = FormatMemory(rEngine.GetMaps().GetMemoryFootprint(),
			&pszUnits[13]);

		float fTextures = FormatMemory(
			rEngine.GetTextures().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Texture>(), &pszUnits[14]);

		float fMaterials = FormatMemory(
			rEngine.GetMaterials().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Material>(), &pszUnits[15]);

		float fEffects = FormatMemory(
			rEngine.GetEffects().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Effect>(), &pszUnits[16]);

		float fAnimations = FormatMemory(
			rEngine.GetAnimations().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Font>(), &pszUnits[18]);

		float fSprites = FormatMemory(
			rEngine.GetSprites().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Sprite>(), &pszUnits[19]);

		float fSounds = FormatMemory(
			rEngine.GetSounds().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Sound>(), &pszUnits[20]);

		float fMusic = FormatMemory(
			rEngine.GetMusic().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Music>(), &pszUnits[21]);

		float fVideo = FormatMemory(
			rEngine.GetVideos().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Video>(), &pszUnits[22]);

		float fRegionSets = FormatMemory(
			rEngine.GetRegions().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<RegionSet>(), &pszUnits[23]);

		float fFonts = FormatMemory(
			rEngine.GetFonts().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<Font>(), &pszUnits[24]);

		float fStrings = FormatMemory(
			rEngine.GetStrings().GetMemoryFootprint() +
			rEngine.GetResourceCache().GetMemoryFootprint<StringTable>(), &pszUnits[25]);

		float fStreams = FormatMemory(rEngine.GetStreamCache().GetMemoryFootprint(),
			&pszUnits[26]);

		rEngine.PrintInfo(
			L"\nBEGIN MEMORY STATUS\n\n"
			L"video memory\n"
			L"   total adapter memory   = %.3f %s\n\n"
			L"global memory\n"
			L"   memory in use          = %d%%\n"
			L"   total physical memory  = %.3f %s\n"
			L"   free physical memory   = %.3f %s\n"
			L"   total virtual memory   = %.3f %s\n"
			L"   free virtual memory    = %.3f %s\n"
			L"   total pagefile memory  = %.3f %s\n"
			L"   free pagefile memory   = %.3f %s\n\n"
			L"process memory\n"
			L"   working set size       = %.3f %s\n"
			L"   peak working set size  = %.3f %s\n"
			L"   pagefile usage         = %.3f %s\n"
			L"   peak pagefile usage    = %.3f %s\n\n"
			L"game memory\n"
			L"   used by game           = %.3f %s\n"
			L"   used by engine         = %.3f %s\n"
			L"   used by screens        = %.3f %s (%d loaded on top level)\n"
			L"   used by maps           = %.3f %s (%d loaded)\n"
			L"   used by textures       = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by materials      = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by effects        = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by fonts          = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by animations     = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by sprites        = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by sounds         = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by music          = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by videos         = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by region sets    = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by string tables  = %.3f %s (%d loaded) (%d cached)\n"
			L"   used by stream cache   = %.3f %s (%d streams)\n"
			L"\nEND MEMORY STATUS",

			fAdapterMemory, pszUnits[27],
			int(ms.dwMemoryLoad),
			fTotalPhysical, pszUnits[0],
			fFreePhysical, pszUnits[1],
			fTotalVirtual, pszUnits[2],
			fFreeVirtual, pszUnits[3],
			fTotalPagefile, pszUnits[4],
			fFreePagefile, pszUnits[5],
			fWorkingSet, pszUnits[6],
			fPeakWorkingSet, pszUnits[7],
			fPagefileUsage, pszUnits[8],
			fPeakPagefileUsage, pszUnits[9],
			fGame, pszUnits[10],
			fEngine, pszUnits[11],
			fScreens, pszUnits[12], rEngine.GetScreens().GetCount(),
			fMaps, pszUnits[13], rEngine.GetMaps().GetCount(),
			fTextures, pszUnits[14], rEngine.GetTextures().GetCount(),
			rEngine.GetResourceCache().GetCount<Texture>(),
			fMaterials, pszUnits[15], rEngine.GetMaterials().GetCount(),
			rEngine.GetResourceCache().GetCount<Material>(),
			fEffects, pszUnits[16], rEngine.GetEffects().GetCount(),
			rEngine.GetResourceCache().GetCount<Effect>(),
			fFonts, pszUnits[24], rEngine.GetFonts().GetCount(),
			rEngine.GetResourceCache().GetCount<Font>(),
			fAnimations, pszUnits[18], rEngine.GetAnimations().GetCount(),
			rEngine.GetResourceCache().GetCount<Animation>(),
			fSprites, pszUnits[19], rEngine.GetSprites().GetCount(),
			rEngine.GetResourceCache().GetCount<Sprite>(),
			fSounds, pszUnits[20], rEngine.GetSounds().GetCount(),
			rEngine.GetResourceCache().GetCount<Sound>(),
			fMusic, pszUnits[21], rEngine.GetMusic().GetCount(),
			rEngine.GetResourceCache().GetCount<Music>(),
			fVideo, pszUnits[22], rEngine.GetVideos().GetCount(),
			rEngine.GetResourceCache().GetCount<Video>(),
			fRegionSets, pszUnits[23], rEngine.GetRegions().GetCount(),
			rEngine.GetResourceCache().GetCount<RegionSet>(),
			fStrings, pszUnits[25], rEngine.GetStrings().GetCount(),
			rEngine.GetResourceCache().GetCount<StringTable>(),
			fStreams, pszUnits[26], rEngine.GetStreamCache().GetCount()
		);
	}
	else if (rParams[0].GetString() == L"d3d")
	{
		// Display D3D initialization parameters

		D3DDEVICE_CREATION_PARAMETERS createparams = {0};

		if (FAILED(rEngine.GetGraphics().GetDevice()->GetCreationParameters(
			&createparams)))
		{
			rEngine.PrintError(L"failed to get device creation parameters.");
			return FALSE;
		}

		D3DADAPTER_IDENTIFIER9 adapterid = {0};

		if (FAILED(rEngine.GetGraphics().GetDirect3D()->GetAdapterIdentifier(
			createparams.AdapterOrdinal, 0, &adapterid)))
		{
			rEngine.PrintError(L"failed to get current adapter information.");
			return FALSE;
		}

		const D3DPRESENT_PARAMETERS& presentparams =
			rEngine.GetGraphics().GetDeviceParams();

		str.Format(
			L"\nBEGIN D3D STATUS\n\n"
			L"   adapter                    = %s\n"
			L"   device type                = %s\n\n"
			L"   windowed                   = %s\n"
			L"   back buffer size           = %dx%d\n"
			L"   back buffer format         = %s\n"
			L"   back buffer count          = %d\n\n"
			L"   multisample type           = %d\n"
			L"   multisample quality        = %d\n"
			L"   swap effect                = %s\n"
			L"   enable auto depth stencil  = %s\n"
			L"   auto depth stencil format  = %s\n"
			L"   full screen refresh rate   = %d Hz\n"
			L"   presentation interval      = %s\n\n"
			L"   device creation flags:       ",

			String(adapterid.Description),
			SZ_DEVICETYPE[createparams.DeviceType - 1],
			Variable(TRUE == presentparams.Windowed).ToString(),
			presentparams.BackBufferWidth,
			presentparams.BackBufferHeight,
			SZ_DEVICEFORMATS[presentparams.BackBufferFormat - D3DFMT_R8G8B8],
			presentparams.BackBufferCount,
			presentparams.MultiSampleType,
			presentparams.MultiSampleQuality,
			SZ_SWAPEFFECT[presentparams.SwapEffect - 1],
			Variable(TRUE == presentparams.EnableAutoDepthStencil).ToString(),
				presentparams.EnableAutoDepthStencil ?
				(presentparams.AutoDepthStencilFormat > D3DFMT_D16 ?
				SZ_DEVICEFORMATS[presentparams.AutoDepthStencilFormat -
					D3DFMT_D32F_LOCKABLE] :
				SZ_DEVICEFORMATS[presentparams.AutoDepthStencilFormat -
					D3DFMT_D16_LOCKABLE]) :
				L"none",
			presentparams.FullScreen_RefreshRateInHz,
			presentparams.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ?
				L"D3DPRESENT_INTERVAL_IMMEDIATE" : L"D3DPRESENT_INTERVAL_ONE");

		bool bAddedFlag = false;

		if (createparams.BehaviorFlags & D3DCREATE_PUREDEVICE)
		{
			str += L"D3DCREATE_PUREDEVICE\n";
			bAddedFlag = true;
		}

		if (createparams.BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		{
			if (bAddedFlag) str += L"                                ";
			str += L"D3DCREATE_HARDWARE_VERTEXPROCESSING\n";
			bAddedFlag = true;
		}

		if (createparams.BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
		{
			if (bAddedFlag) str += L"                                ";
			str += L"D3DCREATE_SOFTWARE_VERTEXPROCESSING\n";
			bAddedFlag = true;
		}

		if (createparams.BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
		{
			if (bAddedFlag) str += L"                                ";
			str += L"D3DCREATE_MIXED_VERTEXPROCESSING\n";
			bAddedFlag = true;
		}

		str += L"\nEND D3D STATUS";

		rEngine.PrintInfo(str);
	}

	return TRUE;
}

int Game::cmd_engineoption(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(
			L"invalid syntax: expected enum engineoptions "
			L"option, [int|bool value].");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(
			L"invalid param type (1): expected enum engineoptions.");

		return FALSE;
	}

	int nOption = 0;

	for(; nOption < Engine::OPTION_COUNT; nOption++)
	{
		if (rParams[0].GetString() == SZ_ENGINE_OPTIONS[nOption])
			break;
	}

	if (Engine::OPTION_COUNT == nOption)
	{
		rEngine.PrintError(L"invalid param value (1): unknown"
			L" engine option specified.");

		return FALSE;
	}

	if (rParams.size() == 2)
	{
		// Setting an option

		if (rParams[1].GetVarType() != Variable::TYPE_INT &&
		   rParams[1].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (2): expected int or bool");

			return FALSE;
		}

		rEngine.SetOption((Engine::Options)nOption, rParams[1].GetIntValue());
	}
	else
	{
		// Getting an option

		switch(nOption)
		{
		case Engine::OPTION_TILE_SIZE:
		case Engine::OPTION_NET_PORT:
		case Engine::OPTION_RESOURCE_CACHE_DURATION:
		case Engine::OPTION_STREAM_CACHE_DURATION:
		case Engine::OPTION_RESOURCE_CACHE_FREQUENCY:
		case Engine::OPTION_STREAM_CACHE_FREQUENCY:
			rEngine.PrintMessage(L"%d", rEngine.GetOption((Engine::Options)nOption));
			break;
		default:
			rEngine.Print(Variable(
				rEngine.GetOption((Engine::Options)nOption) == TRUE).ToString());
			break;
		}
	}

	return TRUE;
}

int Game::cmd_timemultiplier(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// If no params specified, print time multiplier

		rEngine.Print(Variable(rEngine.GetTimeMultiplier()).ToString());
	}
	else
	{
		// Validate

		if (rParams[0].GetVarType() != Variable::TYPE_FLOAT)
		{
			rEngine.PrintError(L"invalid param type (1): float expected.");

			return FALSE;
		}

		// Set time multiplier

		rEngine.SetTimeMultiplier(rParams[0].GetFloatValue());
	}

	return TRUE;
}

int Game::cmd_list(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: expected enum listitems item.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): enum "
			L"listitems expected.");

		return FALSE;
	}

	String strItem = rParams[0].GetStringValue();
	String str;
	int n, nCount;

	if (strItem == L"cpucaps")
	{
		// CPU flags and vendor

		DWORD dwCPUVendor[4] = {0};
		DWORD dwCPUFlags = 0;

		__asm
		{
			pusha

			xor eax,eax

			cpuid

			mov dwCPUVendor[0], ebx
			mov dwCPUVendor[4], edx
			mov dwCPUVendor[8], ecx

			mov eax, 1h

			cpuid

			mov dwCPUFlags, edx

			popa
		}

		WCHAR szCPUVendor[16] = {0};

		mbstowcs_s(NULL, szCPUVendor, 16, (const char*)dwCPUVendor, 12);

		rEngine.PrintInfo(L"\nBEGIN CPU CAPABILITIES");

		rEngine.PrintInfo(L"   vendor = \"%s\"\n", szCPUVendor);

		rEngine.PrintInfo(L"   flags:");

		PrintFlags(dwCPUFlags, SZ_CPUFLAGS, DW_CPUFLAGS, sizeof(DW_CPUFLAGS) /
			sizeof(DWORD), str, L"         ", L"\n");

		rEngine.PrintInfo(str);

		rEngine.PrintInfo(L"END CPU CAPABILITIES");
	}
	else if (wcscmp(rParams[0].GetStringValue(), L"d3dcaps") == 0)
	{
		// D3D capabilities

		const D3DCAPS9& rCaps = rEngine.GetGraphics().GetDeviceCaps();

		// Output general caps

		rEngine.PrintInfo(
			L"\nBEGIN DIRECT3D CAPABILITIES\n"
			L"   MaxTextureWidth         = %d\n"
			L"   MaxTextureHeight        = %d\n"
			L"   MaxVolumeExtent         = %d\n"
			L"   MaxTextureRepeat        = %d\n"
			L"   MaxTextureAspectRatio   = %d\n"
			L"   MaxTextureBlendStages   = %d\n"
			L"   MaxSimultaneousTextures = %d\n"
			L"   MaxPointSize            = %f\n"
			L"   MaxPrimitiveCount       = %d\n"
			L"   MaxVertexIndex          = %d\n"
			L"   MaxStreams              = %d\n"
			L"   VertexShaderVersion     = 0x%x\n"
			L"   MaxVertexShaderConst    = %d\n"
			L"   PixelShaderVersion      = 0x%x\n"
			L"   NumSimultaneousRTs      = %d\n\n",
				   
			rCaps.MaxTextureWidth,
			rCaps.MaxTextureHeight,
			rCaps.MaxVolumeExtent,
			rCaps.MaxTextureRepeat,
			rCaps.MaxTextureAspectRatio,
			rCaps.MaxTextureBlendStages,
			rCaps.MaxSimultaneousTextures,
			rCaps.MaxPointSize,
			rCaps.MaxPrimitiveCount,
			rCaps.MaxVertexIndex,
			rCaps.MaxStreams,
			rCaps.VertexShaderVersion,
			rCaps.MaxVertexShaderConst,
			rCaps.PixelShaderVersion,
			rCaps.NumSimultaneousRTs);


		// Output caps 1 flags

		str.Empty();

		PrintFlags(rCaps.Caps, SZ_DEVCAPS_1, DW_DEVCAPS_1,
			sizeof(DW_DEVCAPS_1) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.GetLength()) rEngine.Print(str, PRINT_INFO);

		rEngine.PrintBlank();

		// Output caps 2 flags

		PrintFlags(rCaps.Caps2, SZ_DEVCAPS_2, DW_DEVCAPS_2,
			sizeof(DW_DEVCAPS_2) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output caps 3 flags

		PrintFlags(rCaps.Caps3, SZ_DEVCAPS_3, DW_DEVCAPS_3,
			sizeof(DW_DEVCAPS_3) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output present flags

		PrintFlags(rCaps.PresentationIntervals, SZ_PRESENT, DW_PRESENT,
			sizeof(DW_PRESENT) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output cursor flags

		PrintFlags(rCaps.CursorCaps, SZ_CURSOR, DW_CURSOR,
			sizeof(DW_CURSOR) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output devcaps flags

		PrintFlags(rCaps.DevCaps, SZ_DEVCAPS_4, DW_DEVCAPS_4,
			sizeof(DW_DEVCAPS_4) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output primitive flags

		PrintFlags(rCaps.PrimitiveMiscCaps, SZ_PRIM, DW_PRIM,
			sizeof(DW_PRIM) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output raster flags

		PrintFlags(rCaps.RasterCaps, SZ_RAST, DW_RAST,
			sizeof(DW_RAST) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output ZCmpCaps flags

		PrintFlags(rCaps.ZCmpCaps, SZ_ZCMP, DW_ZCMP,
			sizeof(DW_ZCMP) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output srcblend flags

		PrintFlags(rCaps.SrcBlendCaps, SZ_SRCBLEND, DW_SRCBLEND,
			sizeof(DW_SRCBLEND) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output destblend flags

		PrintFlags(rCaps.DestBlendCaps, SZ_SRCBLEND, DW_SRCBLEND,
			sizeof(DW_SRCBLEND) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output alphacmp flags

		PrintFlags(rCaps.AlphaCmpCaps, SZ_ZCMP, DW_ZCMP,
			sizeof(DW_ZCMP) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output texture flags

		PrintFlags(rCaps.TextureCaps, SZ_TEX, DW_TEX,
			sizeof(DW_TEX) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		// Output texture filter flags

		PrintFlags(rCaps.TextureFilterCaps, SZ_TEXF, DW_TEXF,
			sizeof(DW_TEXF) / sizeof(DWORD), str, L"   ", L"\n");

		if (str.IsEmpty() == false)
			rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		rEngine.PrintInfo(L"END DIRECT3D CAPABILITIES");
	}
	else if (strItem == L"dscaps")
	{
		// DirectSound capabilities

		if (rEngine.GetAudio().GetDirectSound() == NULL)
		{
			rEngine.PrintError(L"directsound not initialized.");
			return FALSE;
		}
		
		DSCAPS caps;
		caps.dwSize = sizeof(DSCAPS);

		if (FAILED(rEngine.GetAudio().GetDirectSound()->GetCaps(&caps)))
		{
			rEngine.PrintError(L"failed to get directsound capabilities.");

			PrintLastError(rEngine);

			return FALSE;
		}

		DWORD dwSpeaker = 0;

		if (FAILED(rEngine.GetAudio().GetDirectSound()->GetSpeakerConfig(&dwSpeaker)))
		{
			rEngine.PrintError(L"failed to get directsound speaker configuration.");

			PrintLastError(rEngine);

			return FALSE;
		}

		rEngine.PrintInfo(L"\nBEGIN DIRECTSOUND CAPABILITIES");

		DWORD dwSpeakerConfig = DSSPEAKER_CONFIG(dwSpeaker);
		DWORD dwSpeakerGeometry = DSSPEAKER_GEOMETRY(dwSpeaker);

		rEngine.PrintInfo(L"   speaker configuration:");

		PrintFlags(dwSpeakerConfig, SZ_SPEAKERCONFIG, DW_SPEAKERCONFIG,
			sizeof(DW_SPEAKERCONFIG) / sizeof(DWORD), str, L"   ", L"\n");

		rEngine.PrintInfo(str);

		rEngine.PrintInfo(L"\n   speaker geometry:");

		PrintFlags(dwSpeakerGeometry, SZ_SPEAKERGEOMETRY, DW_SPEAKERGEOMETRY,
			sizeof(DW_SPEAKERCONFIG) / sizeof(DWORD), str, L"   ", L"\n");

		rEngine.PrintInfo(str);

		rEngine.PrintBlank();

		LPCWSTR pszUnits[3] = { NULL };

		float fHardware = FormatMemory(
			int(caps.dwTotalHwMemBytes), pszUnits);

		float fHardwareFree = FormatMemory(
			int(caps.dwFreeHwMemBytes), pszUnits + 1);

		float fLargestBlock = FormatMemory(
			int(caps.dwMaxContigFreeHwMemBytes), pszUnits + 2);

		rEngine.PrintInfo(
			L"   min secondary sample rate = %u\n"
			L"   max secondary sample rate = %u\n"
			L"   primary buffers supported = %u\n\n"

			L"   max buffers               = %u\n"
			L"   max static buffers        = %u\n"
			L"   max streaming buffers     = %u\n"
			L"   free buffers              = %u\n"
			L"   free static buffers       = %u\n"
			L"   free streaming buffers    = %u\n\n"

			L"   max 3d buffers            = %u\n"
			L"   max static 3d buffers     = %u\n"
			L"   max streaming 3d buffers  = %u\n"
			L"   free 3d buffers           = %u\n"
			L"   free static 3d buffers    = %u\n"
			L"   free streaming 3d buffers = %u\n\n"

			L"   total hardware memory     = %.3f %s\n"
			L"   free hardware memory      = %.3f %s\n"
			L"   hardware memory used      = %g%%\n"
			L"   hardware memory free      = %g%%\n"
			L"   largest free block size   = %.3f %s\n"
			L"   hardware transfer rate    = %u KB/s\n"
			L"   software buffer overhead  = %u%% of CPU clock cycles\n",

			caps.dwMinSecondarySampleRate,
			caps.dwMaxSecondarySampleRate,
			caps.dwPrimaryBuffers,

			caps.dwMaxHwMixingAllBuffers,
			caps.dwMaxHwMixingStaticBuffers,
			caps.dwMaxHwMixingStreamingBuffers,
			caps.dwFreeHwMixingAllBuffers,
			caps.dwFreeHwMixingStaticBuffers,
			caps.dwFreeHwMixingStreamingBuffers,

			caps.dwMaxHw3DAllBuffers,
			caps.dwMaxHw3DStaticBuffers,
			caps.dwMaxHw3DStreamingBuffers,
			caps.dwFreeHw3DAllBuffers,
			caps.dwFreeHw3DStaticBuffers,
			caps.dwFreeHw3DStreamingBuffers,
				   
			fHardware, pszUnits[0],
			fHardwareFree, pszUnits[1],
			caps.dwTotalHwMemBytes ?
				float(caps.dwTotalHwMemBytes - caps.dwFreeHwMemBytes) /
				float(caps.dwTotalHwMemBytes) * 100.0f : 0.0f,
			caps.dwTotalHwMemBytes ? float(caps.dwFreeHwMemBytes) /
				float(caps.dwTotalHwMemBytes) * 100.0f : 0.0f,
				fLargestBlock, pszUnits[2],
			caps.dwUnlockTransferRateHwBuffers,
			caps.dwPlayCpuOverheadSwBuffers);

		PrintFlags(caps.dwFlags, SZ_DSCAPS, DW_DSCAPS,
			sizeof(DW_DSCAPS) / sizeof(DWORD), str, L"   ", L"\n");

		rEngine.PrintInfo(str);

		rEngine.PrintInfo(L"END DIRECTSOUND CAPABILITIES");
	}
	else if (strItem == L"textures")
	{
		rEngine.PrintInfo(L"\nBEGIN TEXTURES");

		n = 0;

		LPCWSTR pszType = NULL;

		for(ResourceManager<Texture>::ConstIterator pos =
			rEngine.GetTextures().GetBeginPosConst();
			pos != rEngine.GetTextures().GetEndPosConst();
			pos++)
		{
			const Texture* pTex = pos->second;

			if (NULL == pTex)
				continue;

			if (dynamic_cast<const TextureDynamic*>(pTex) != NULL)
				pszType = L"dynamic";
			else
				pszType = L"default";

			rEngine.PrintInfo(L"%3d %3d \"%s\" (%s) %dx%d", n,
				pTex->GetRefCount(), pTex->GetName(), pszType,
				pTex->GetInfo().Width, pTex->GetInfo().Height);

			n++;
		}

		rEngine.PrintInfo(L"END TEXTURES");
	}
	else if (strItem == L"materials")
	{
		rEngine.PrintInfo(L"\nBEGIN MATERIALS");

		n = 0;

		for(ResourceManager<Material>::ConstIterator pos =
			rEngine.GetMaterials().GetBeginPosConst();
			pos != rEngine.GetMaterials().GetEndPosConst();
			pos++)
		{
			const Material* pMat = (pos->second);

			if (NULL == pMat)
				continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pMat->GetRefCount(), pMat->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END MATERIALS");
	}
	else if (strItem == L"sharedmaterials")
	{
		rEngine.PrintInfo(L"\nBEGIN SHARED MATERIALS");

		n = 0;

		MaterialInstancePool& rPool =
			rEngine.GetGraphics().GetMaterialInstancePool();

		for(MaterialInstanceSharedMapConstIterator pos = rPool.GetBeginPosConst();
			pos != rPool.GetEndPosConst();
			pos++)
		{
			rEngine.PrintInfo(L"%3d %4d \"%s\"\n         \"%s\" %s", n,
				pos->second.GetRefCount(),
				PathFindFileName(pos->second.GetMaterialConst()->GetName()),
				PathFindFileName(pos->second.GetMaterialConst()->GetEffectConst()->GetName()),
				String(pos->second.GetTechniqueConst()->GetInfo().Name));

			// Print names of static params set

			if (pos->second.GetMaterialConst()->GetParameterCount() > 0)
			{
				rEngine.Print(L"         static params:", PRINT_INFO, false);

				for(EffectParameterArrayConstIterator posSp =
					pos->second.GetMaterialConst()->GetBeginParameterPosConst();
					posSp != pos->second.GetMaterialConst()->GetEndParameterPosConst();
					posSp++)
				{

					rEngine.Print(L" ", PRINT_INFO, false);
					rEngine.Print(posSp->GetInfo()->GetName(), PRINT_INFO, false);					

					if (posSp->GetInfo()->GetSemantic() ==
						EffectParameterInfo::SEMANTIC_BASETEXTURE)
					{
						rEngine.PrintInfo(L" \"%s\"",
							PathFindFileName(posSp->GetTextureConst()->GetName()));
					}
				}
			}

			if (pos->second.GetParameterCount() > 0)
			{
				rEngine.Print(L"         dynamic params:", PRINT_INFO, false);

				for(EffectParameterArrayConstIterator posDp =
					pos->second.GetBeginParameterPosConst();
					posDp != pos->second.GetEndParameterPosConst();
					posDp++)
				{
					rEngine.Print(L" ", PRINT_INFO, false);
					rEngine.Print(posDp->GetInfo()->GetName(), PRINT_INFO, false);					

					if (posDp->GetInfo()->GetSemantic() ==
						EffectParameterInfo::SEMANTIC_BASETEXTURE)
					{
						rEngine.PrintInfo(L" \"%s\"",
							PathFindFileName(posDp->GetTextureConst()->GetName()));
					}
				}
			}

			n++;
		}

		rEngine.PrintInfo(L"END SHARED MATERIALS");
	}
	else if (strItem == L"map.materials")
	{
		const TileMap* pMap = rEngine.GetCurrentMapConst();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"\nBEGIN CURRENT MAP MATERIALS");

		nCount = pMap->GetMaterialCount();

		for(n = 0; n < nCount; n++)
		{
			const Material* pMat = pMap->GetMaterialConst(n);
			if (NULL == pMat) continue;

			rEngine.PrintInfo(L"%3d %3d   \"%s\"", n,
				pMat->GetRefCount(), pMat->GetName());
		}

		rEngine.Print(L"END CURRENT MAP MATERIALS", PRINT_INFO);
	}
	else if (strItem == L"effects")
	{
		rEngine.PrintInfo(L"\nBEGIN EFFECTS");

		n = 0;

		for(ResourceManager<Effect>::ConstIterator pos =
			rEngine.GetEffects().GetBeginPosConst();
			pos != rEngine.GetEffects().GetEndPosConst();
			pos++)
		{
			const Effect* pEffect = pos->second;
			if (NULL == pEffect) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pEffect->GetRefCount(), pEffect->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END EFFECTS");
	}
	else if (strItem == L"animations")
	{
		rEngine.PrintInfo(L"\nBEGIN ANIMATIONS");

		n = 0;

		for(ResourceManager<Animation>::ConstIterator pos =
			rEngine.GetAnimations().GetBeginPosConst();
			pos != rEngine.GetAnimations().GetEndPosConst();
			pos++)
		{
			const Animation* pAnim = pos->second;
			if (NULL == pAnim) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pAnim->GetRefCount(), pAnim->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END ANIMATIONS");
	}
	else if (strItem == L"map.animations")
	{
		const TileMap* pMap = rEngine.GetCurrentMapConst();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"\nBEGIN CURRENT MAP ANIMATIONS");

		nCount = pMap->GetAnimationCount();

		for(n = 0; n < nCount; n++)
		{
			const Animation* pAnim = pMap->GetAnimationConst(n);
			if (NULL == pAnim) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n, pAnim->GetRefCount(),
				pAnim->GetName());
		}

		rEngine.PrintInfo(L"\nEND CURRENT MAP ANIMATIONS");
	}
	else if (strItem == L"sprites")
	{
		rEngine.PrintInfo(L"\nBEGIN SPRITES");

		n = 0;

		for(ResourceManager<Sprite>::ConstIterator pos =
			rEngine.GetSprites().GetBeginPosConst();
			pos != rEngine.GetSprites().GetEndPosConst();
			pos++)
		{
			const Sprite* pSprite = pos->second;
			if (NULL == pSprite) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pSprite->GetRefCount(), pSprite->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END SPRITES");
	}
	else if (strItem == L"fonts")
	{
		rEngine.PrintInfo(L"\nBEGIN FONTS");

		n = 0;

		for(ResourceManager<Font>::ConstIterator pos =
			rEngine.GetFonts().GetBeginPosConst();
			pos != rEngine.GetFonts().GetEndPosConst();
			pos++)
		{
			const Font* pFont = pos->second;
			if (NULL == pFont) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pFont->GetRefCount(), pFont->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END FONTS");
	}
	else if (strItem == L"stringtables")
	{
		rEngine.PrintInfo(L"\nBEGIN STRING TABLES");

		n = 0;

		for(ResourceManager<StringTable>::ConstIterator pos =
			rEngine.GetStrings().GetBeginPosConst();
			pos != rEngine.GetStrings().GetEndPosConst();
			pos++)
		{
			const StringTable* pStringTable = pos->second;
			if (NULL == pStringTable) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pStringTable->GetRefCount(), pStringTable->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END STRING TABLES");
	}
	else if (strItem == L"sounds")
	{
		rEngine.PrintInfo(L"\nBEGIN SOUNDS");

		n = 0;

		for(ResourceManager<Sound>::ConstIterator pos =
			rEngine.GetSounds().GetBeginPosConst();
			pos != rEngine.GetSounds().GetEndPosConst();
			pos++)
		{
			const Sound* pSound = pos->second;
			if (NULL == pSound) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pSound->GetRefCount(), pSound->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END SOUNDS");
	}
	else if (strItem == L"map.sounds")
	{
		const TileMap* pMap = rEngine.GetCurrentMapConst();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"\nBEGIN CURRENT MAP SOUNDS");

		nCount = pMap->GetSoundCount();

		for(n = 0; n < nCount; n++)
		{
			const Sound* pSound = pMap->GetSoundConst(n);

			if (NULL == pSound) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pSound->GetRefCount(), pSound->GetName());
		}

		rEngine.PrintInfo(L"END CURRENT MAP SOUNDS");
	}
	else if (strItem == L"music")
	{
		bool bPlayingOnly = false;

		if (rParams.size() > 1)
		{
			if (rParams[1].GetVarType() != Variable::TYPE_BOOL &&
			   rParams[1].GetVarType() != Variable::TYPE_INT)
			{
				rEngine.PrintError(L"invalid param type (1): expected int or bool.");

				return FALSE;
			}

			bPlayingOnly = rParams[1].GetBoolValue();
		}

		if (true == bPlayingOnly)
			rEngine.PrintInfo(L"\nBEGIN PLAYING MUSIC");
		else
			rEngine.PrintInfo(L"\nBEGIN MUSIC");
			

		n = 0;

		for(ResourceManager<Music>::ConstIterator pos =
			rEngine.GetMusic().GetBeginPosConst();
			pos != rEngine.GetMusic().GetEndPosConst();
			pos++)
		{
			const Music* pMusic = pos->second;

			if (NULL == pMusic) continue;

			if (true == bPlayingOnly &&
				pMusic->IsFlagSet(Music::PLAYING) == false) continue;

			str.Format(L"%3d %3d \"%s\"", n,
				pMusic->GetRefCount(), pMusic->GetName());

			if (pMusic->IsFlagSet(Music::PLAYING) == true)
				str += L" [playing]";

			if (pMusic->IsFlagSet(Music::LOOPING) == true)
				str += L" [looping]";

			rEngine.PrintInfo(str);

			n++;
		}

		rEngine.PrintInfo(L"END MUSIC");
	}
	else if (wcscmp(rParams[0].GetStringValue(), L"videos") == 0)
	{
		rEngine.PrintInfo(L"\nBEGIN VIDEOS");

		n = 0;

		for(ResourceManager<Video>::ConstIterator pos =
			rEngine.GetVideos().GetBeginPosConst();
			pos != rEngine.GetVideos().GetEndPosConst();
			pos++)
		{
			const Video* pVideo = pos->second;

			if (NULL == pVideo) continue;

			str.Format(L"%3d %3d \"%s\"", n, pVideo->GetRefCount(),
				pVideo->GetName());

			if (pVideo->IsFlagSet(Video::PLAYING) == true)
				str += L" [playing]";

			rEngine.PrintInfo(str);

			n++;
		}

		rEngine.Print(L"END VIDEOS", PRINT_INFO);
	}
	else if (strItem == L"map.music")
	{
		const TileMap* pMap = rEngine.GetCurrentMapConst();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"BEGIN CURRENT MAP MUSIC");

		nCount = pMap->GetMusicCount();

		for(n = 0; n < nCount; n++)
		{
			const Music* pMusic = pMap->GetMusicConst(n);

			if (NULL == pMusic) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n, pMusic->GetRefCount(),
				pMusic->GetName());
		}

		rEngine.PrintInfo(L"END CURRENT MAP MUSIC");
	}
	else if (strItem == L"regionsets")
	{
		rEngine.PrintInfo(L"\nBEGIN REGION SETS");

		n = 0;

		for(ResourceManager<RegionSet>::ConstIterator pos =
			rEngine.GetRegions().GetBeginPosConst();
			pos != rEngine.GetRegions().GetEndPosConst();
			pos++)
		{
			const RegionSet* pRegionSet = pos->second;

			if (NULL == pRegionSet) continue;

			rEngine.PrintInfo(L"%3d %3d \"%s\"", n,
				pRegionSet->GetRefCount(), pRegionSet->GetName());

			n++;
		}

		rEngine.PrintInfo(L"END REGION SETS");
	}
	else if (strItem == L"maps")
	{
		rEngine.PrintInfo(L"\nBEGIN MAPS");

		TileMapManager& rMaps = rEngine.GetMaps();

		n = 0;

		for(TileMapArrayConstIterator pos = rMaps.GetBeginPosConst();
			pos != rMaps.GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L" %2d \"%s\" ( \"%s\" )", n,
				(*pos)->GetName(), (*pos)->GetClass());
		}

		rEngine.PrintInfo(L"END MAPS");
	}
	else if (strItem == L"map.actors")
	{
		TileMap* pMap = rEngine.GetCurrentMap();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"BEGIN CURRENT MAP ACTORS");

		String strFlags;

		n = 0;

		for(ActorMapConstIterator pos = pMap->GetBeginActorPosConst();
			pos != pMap->GetEndActorPosConst();
			pos++, n++)
		{
			Actor* pActor = pos->second;

			if (NULL == pActor) continue;

			PrintFlags(pActor->GetFlags(), SZ_ACTORFLAGS, DW_ACTORFLAGS,
				sizeof(DW_ACTORFLAGS) / sizeof(DWORD), strFlags,
				L"", L" | ");

			rEngine.PrintInfo(L"%3d \"%s\" ( \"%s\" ) [ %g, %g, %g ] "
				L"[ %g, %g ]\n   [ %s ]", n, pActor->GetName(),
				pActor->GetClass(), pActor->GetPosition().x,
				pActor->GetPosition().y, pActor->GetZOrder(),
				pActor->GetSprite()->GetSizeInTiles().x,
				pActor->GetSprite()->GetSizeInTiles().y, strFlags.GetBuffer());
		}

		rEngine.PrintInfo(L"END CURRENT MAP ACTORS");
	}
	else if (strItem == L"variables")
	{
		rEngine.PrintInfo(L"\nBEGIN VARIABLES");

		n = 0;

		for(VariableMapConstIterator pos =
			rEngine.GetVariablesConst().GetBeginPosConst();
			pos != rEngine.GetVariablesConst().GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L"%3d %s = %s", n, pos->first,
				pos->second->ToString());
		}

		rEngine.PrintInfo(L"END VARIABLES");
	}
	else if (strItem == L"map.variables")
	{
		TileMap* pMap = rEngine.GetCurrentMap();

		if (NULL == pMap)
		{
			rEngine.PrintError(L"no current map.");
			return FALSE;
		}

		rEngine.PrintInfo(L"\nBEGIN CURRENT MAP VARIABLES");

		n = 0;

		for(VariableMapConstIterator pos =
			pMap->GetVariablesConst().GetBeginPosConst();
			pos != pMap->GetVariablesConst().GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L"%3d %s = %s", n, pos->first,
				pos->second->ToString());
		}

		rEngine.PrintInfo(L"END CURRENT MAP VARIABLES");
	}
	else if (strItem == L"commands")
	{
		rEngine.PrintInfo(L"\nBEGIN COMMANDS");

		n = 0;

		for(CommandMapConstIterator pos =
			rEngine.GetCommandsConst().GetBeginPosConst();
			pos	!= rEngine.GetCommandsConst().GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L"%3d %s [0x%x]", n,
				pos->first, pos->second->GetCallback());
		}

		rEngine.PrintInfo(L"END COMMANDS");
	}
	else if (wcscmp(rParams[0].GetStringValue(), L"soundinstances") == 0)
	{
		rEngine.PrintInfo(L"\nBEGIN SOUND INSTANCES");

		const SoundInstanceManager& rSoundInstances =
			rEngine.GetAudio().GetSoundInstancesConst();

		n = 0;

		for(SoundInstanceArrayConstIterator pos =
			rSoundInstances.GetBeginPosConst();
			pos != rSoundInstances.GetEndPosConst();
			pos++, n++)
		{
			str.Format(L"%3d \"%s\"", n, (*pos)->GetSound().GetName());

			if ((*pos)->IsFlagSet(SoundInstance::PLAYING) == true)
				str += L" [playing]";
			
			if ((*pos)->IsFlagSet(SoundInstance::LOOPING) == true)
				str += L" [looping]";

			rEngine.PrintInfo(str);
		}

		rEngine.PrintInfo(L"END SOUND INSTANCES");
	}
	else if (strItem == L"screens")
	{
		ScreenManager& rScreens = rEngine.GetScreens();

		rEngine.PrintInfo(L"\nBEGIN SCREENS");

		rEngine.PrintInfo(L"   active screen: %s",
			rScreens.GetActiveScreen() != NULL ?
			rScreens.GetActiveScreen()->GetName().GetBufferConst() : NULL);

		rEngine.PrintInfo(L"   hover screen:  %s",
			rScreens.GetHoverScreen() != NULL ?
			rScreens.GetHoverScreen()->GetName().GetBufferConst() : NULL);

		rEngine.PrintInfo(L"   focus screen:  %s\n",
			rScreens.GetFocusScreen() != NULL ?
			rScreens.GetFocusScreen()->GetName().GetBufferConst() : NULL);

		n = 0;

		for(ScreenListConstIterator pos =
			rEngine.GetScreensConst().GetBeginPosConst();
			pos != rEngine.GetScreensConst().GetEndPosConst();
			pos++, n++)
		{
			Screen* pScreen = *pos;

			if (NULL == pScreen) continue;

			rEngine.PrintInfo(L"%3d \"%s\" (\"%s\") (%d, %d) (%d, %d)", n,
				pScreen->GetName(), pScreen->GetClass(),
				pScreen->GetPosition().x, pScreen->GetPosition().y,
				pScreen->GetSize().cx, pScreen->GetSize().cy);
		}

		rEngine.PrintInfo(L"END SCREENS");
	}
	else if (strItem == L"timers")
	{
		rEngine.PrintInfo(L"\nBEGIN TIMERS");

		n = 0;

		for(TimerListConstIterator pos =
			rEngine.GetTimers().GetBeginPosConst();
			pos != rEngine.GetTimers().GetEndPosConst();
			pos++, n++)
		{
			Timer* pTimer = *pos;

			rEngine.PrintInfo(L"%3d id: %d, target: \"%s\" interval: %g seconds,"
				L"last fired: %g seconds ago",
				n, pTimer->GetID(), pTimer->GetTarget()->GetName(),
				pTimer->GetInterval(),
				rEngine.GetRunTime() - pTimer->GetLastFiredTime());
		}

		rEngine.PrintInfo(L"END TIMERS");
	}
	else if (strItem == L"classes")
	{
		rEngine.PrintInfo(L"\nBEGIN CLASSES");

		n = 0;

		for(CallbackMapConstIterator pos =
			rEngine.GetClasses().GetBeginPosConst();
			pos != rEngine.GetClasses().GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L"%3d \"%s\" [0x%x]", n,
				pos->first, pos->second);
		}

		rEngine.PrintInfo(L"END CLASSES");
	}
	else if (strItem == L"controls")
	{
		rEngine.PrintInfo(L"\nBEGIN CONTROLS");

		Client* pGame = rEngine.GetClientInstance();

		if (NULL == pGame)
			return FALSE;

		ControlManager& rControls = pGame->GetControls();

		for(int n = 0; n < CONTROL_COUNT; n++)
		{
			rEngine.PrintInfo(L"   %s: %s", SZ_CONTROLS[n],
				rControls.GetKeyDescription(rControls.GetControlBoundKey(n)));
		}

		rEngine.PrintInfo(L"END CONTROLS");
	}
	else if (strItem == L"cachedstreams")
	{
		rEngine.PrintInfo(L"\nBEGIN CACHED STREAMS");

		int n = 0;

		for(StreamDataMapConstIterator pos =
			rEngine.GetStreamCacheConst().GetBeginPosConst();
			pos != rEngine.GetStreamCacheConst().GetEndPosConst();
			pos++, n++)
		{
			rEngine.PrintInfo(L"%3d   \"%s\"\n      [0x%x:%d] "
				L"(%f seconds before eviction)",
				n, pos->first, pos->second->GetData(),
				pos->second->GetDataSize(),
				pos->second->GetLastRequestTime() +
				float(rEngine.GetOption(Engine::OPTION_STREAM_CACHE_DURATION)) -
				rEngine.GetRunTime());
		}

		rEngine.PrintInfo(L"END CACHED STREAMS");
	}
	else if (strItem == L"cachedresources")
	{
		rEngine.PrintInfo(L"\nBEGIN CACHED RESOURCES");

		int n = 0;

		for(ResourceMapConstIterator pos =
			rEngine.GetResourceCache().GetBeginPosConst();
			pos != rEngine.GetResourceCache().GetEndPosConst();
			pos++, n++)
		{
			Resource* pResource = pos->second;

			rEngine.PrintInfo(L"%3d   \"%s\"\n      (%f seconds before discard)",
				n, pResource->GetName(),
				pResource->GetDiscardTime() +
				pResource->GetPersistenceTime() -
				rEngine.GetRunTime());
		}

		rEngine.PrintInfo(L"END CACHED RESOURCES");
	}
	else
	{
		rEngine.PrintError(L"invalid list value, see help.");
		return FALSE;
	}

	return TRUE;
}

int Game::cmd_about(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame) return FALSE;

	rEngine.PrintInfo(
		L"\nBEGIN GAME INFORMATION\n"
		L"   title                = \"%s\"\n"
		L"   version              =  %d.%d.%d.%d\n"
		L"   description          = \"%s\"\n"
		L"   profile              = \"%s\"\n"
		L"   logfile              = \"%s\"\n"
		L"   script               = \"%s\"\n"
		L"   executable           = \"%s\"\n"
		L"   executable directory = \"%s\"\n"
		L"   base directory       = \"%s\""
		L"\nEND GAME INFORMATION",

		pGame->m_strTitle,
		pGame->m_nVersion[VERSION_MAJOR],
		pGame->m_nVersion[VERSION_MINOR],
		pGame->m_nVersion[VERSION_REVISION],
		pGame->m_nVersion[VERSION_BUILD],
		pGame->m_strDescription,
		pGame->m_strProfilePath,
		pGame->m_strLogFilePath,
		pGame->m_strScriptPath,
		pGame->m_strExeTitle,
		pGame->m_strExeDir,
		pGame->m_strBaseDir);

	return TRUE;
}

int Game::cmd_pony(Engine& rEngine, VariableArray& rParams)
{
	rEngine.PrintMessage(
		L"           .,,.\n"
		L"         ,;;*;;;;,\n"
		L"        .-'``;-');;.\n"
		L"       /'  .-.  /*;;\n"
		L"     .'    \\d    \\;;               .;;;,\n"
		L"    / o      `    \\;    ,__.     ,;*;;;*;,\n"
		L"    \\__, _.__,'   \\_.-') __)--.;;;;;*;;;;,\n"
		L"     `__`;;;\\       /-')_) __)  `\\' ';;;;;;\n"
		L"        ;*;;;        -') `)_)  |\\ |  ;;;;*;\n"
		L"        ;;;;|        `---`    O | | ;;*;;;\n"
		L"        *;*;\\|                 O  / ;;;;;*\n"
		L"       ;;;;;/|    .-------\\      / ;*;;;;;\n"
		L"      ;;;*;/ \\    |        '.   (`. ;;;*;;;\n"
		L"      ;;;;;'. ;   |          )   \\ | ;;;;;;\n"
		L"      ,;*;;;;\\/   |.        /   /` | ';;;*;\n"
		L"       ;;;;;;/    |/       /   /__/   ';;;\n"
		L"       '*jgs/     |       /    |      ;*;\n"
		L"            `____`        `____`     ;'\n");

	return TRUE;
}

int Game::cmd_configure(Engine& rEngine, VariableArray& rParams)
{
	// Relaunch with /configure switch

	Client* pGame = rEngine.GetClientInstance();

	if (NULL == pGame) return FALSE;

	pGame->Exit();

	WCHAR szExePath[MAX_PATH] = {0};
	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);

	WCHAR szCurDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szCurDir);

	ShellExecute(NULL, NULL, szExePath,
		SZ_CMDSWITCH_CONFIGURE, szCurDir, SW_SHOW);

	return TRUE;
}

int Game::cmd_settings(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame) return FALSE;

	String strRefresh;

	if (pGame->m_dwRefreshRate == 0xFFFFFFFF)
		strRefresh = SZ_THUDEVICEFORMATS[0];
	else
		strRefresh.Format(L"%d", pGame->m_dwRefreshRate);

	rEngine.PrintInfo(
		L"\nBEGIN GAME SETTINGS\n"
		L"video\n"
		L"   resolution      = %dx%d\n"
		L"   full screen     = %s\n"
		L"   hwaccel         = %s\n"
		L"   device format   = %s\n"
		L"   refresh rate    = %s\n"
		L"   vsync           = %s\n"
		L"   swvp            = %s\n"
		L"   pure device     = %s\n"
		L"audio\n"
		L"   exclusive sound = %s\n"
		L"   disable sounds  = %s\n"
		L"   disable music   = %s\n"
		L"   master volume   = %g\n"
		L"   master mute     = %s\n"
		L"   music volume    = %g\n"
		L"   effects volume  = %g\n"
		L"   speech volume   = %g\n"
		L"   destination     = %s\n"
		L"system\n"
		L"   log mode  = %s"
		L"\nEND GAME SETTINGS",

		pGame->m_nResolutionWidth,
		pGame->m_nResolutionHeight,
		Variable(pGame->m_bFullScreen).ToString(),
		Variable(pGame->m_bHardwareAcceleration).ToString(),
		SZ_THUDEVICEFORMATS[pGame->m_nDeviceFormat],
		strRefresh,
		Variable(pGame->m_bVSync).ToString(),
		Variable(pGame->m_bSoftwareVertexProcessing).ToString(),
		Variable(pGame->m_bPureDevice).ToString(),
		Variable(rEngine.GetOption(
			Engine::OPTION_EXCLUSIVE_SOUND) == TRUE).ToString(),
		Variable(rEngine.GetOption(
			Engine::OPTION_DISABLE_SOUNDS) == TRUE).ToString(),
		Variable(rEngine.GetOption(
		Engine::OPTION_DISABLE_MUSIC) == TRUE).ToString(),
		rEngine.GetAudio().GetMasterVolume(),
		Variable(rEngine.GetAudio().GetMasterVolumeMute()).ToString(),
		pGame->m_fMusicVolume,
		pGame->m_fEffectsVolume,
		pGame->m_fSpeechVolume,
		rEngine.GetOption(Engine::OPTION_AUDIO_DESTINATION) == Audio::DESTINATION_SPEAKERS ?
			L"speakers" : L"headphones",
		SZ_LOGMODES[pGame->m_nLogMode]);

	return TRUE;
}

int Game::cmd_logmode(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: expected enum "
			L"logmodes logmode, [ string newlogpath. ]");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): "
			L"enum logmodes expected.");

		return FALSE;
	}

	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame)
	{
		rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pGame");

		PrintLastError(rEngine);

		return FALSE;
	}

	Client::LogMode nLogMode = 
		(Client::LogMode)rParams[0].GetEnumValue(SZ_LOGMODES, LOG_COUNT, 0);

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (2): string expected.");

			return FALSE;
		}

		pGame->SetLogFilePath(rParams[1].GetStringValue());
	}

	try
	{
		pGame->SetLogMode(nLogMode);

		rEngine.PrintInfo(L"log mode set successfully.");
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_screenshot(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());
	if (NULL == pGame) return FALSE;

	WCHAR szFileName[MAX_PATH] = {0};
	WCHAR szCurTime[32] = {0};

	GetCurrentDirectory(MAX_PATH, szFileName);
	PathAddBackslash(szFileName);
	wcscat_s(szFileName, MAX_PATH, L"screenshots");

	// Create base\screenshots if does not exist

	if ((PathFileExists(szFileName) == FALSE) &&
		!CreateDirectory(szFileName, NULL))
	{
		rEngine.PrintError(L"failed to create screenshots directory.");

		return FALSE;
	}

	// Figure out filename to use (screenshot_<curtime>.jpg)

	SYSTEMTIME st;
	GetSystemTime(&st);

	swprintf_s(szCurTime, 32, L"%d-%d-%d %d.png",
		(int)st.wMonth, (int)st.wDay, (int)st.wYear,
		(int)GetTickCount());

	PathAddBackslash(szFileName);
	wcscat_s(szFileName, MAX_PATH, szCurTime);

	// Take the screenshot and save it

	try
	{
		rEngine.GetGraphics().Screenshot(szFileName, D3DXIFF_PNG);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return FALSE;
	}

	rEngine.PrintInfo(L"screenshot: \"%s\"", szFileName, PRINT_INFO);

	return TRUE;
}

int Game::cmd_fillmode(Engine& rEngine, VariableArray& rParams)
{
	// Game game instance

	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	// Get current fill mode

	int nFillMode = pGame->GetFillMode();

	if (rParams.empty() == true)
	{
		// If called without parameters, cycle fill mode

		if (FILL_DEBUG == nFillMode)
			nFillMode = FILL_DEFAULT;
		else
			nFillMode++;
	}
	else
	{
		// If called with one parameter, specified new fill mode to set

		if (rParams[0].GetVarType() == Variable::TYPE_INT &&
			rParams[0].GetVarType() == Variable::TYPE_BOOL)
		{
			nFillMode = rParams[0].GetIntValue();

			if (nFillMode < FILL_DEFAULT)
				nFillMode = FILL_DEFAULT;
			else if (nFillMode > FILL_DEBUG)
				nFillMode = FILL_DEBUG;
		}
		else if (rParams[0].GetVarType() == Variable::TYPE_ENUM)
		{
			nFillMode = rParams[0].GetEnumValue(SZ_FILLMODE,
												  FILL_DEBUG + 1, 0);
		}
		else
		{
			rEngine.PrintError(L"invalid parameter(s) specified: expected "
				L"int | bool | enum FillModes { default, wireframe, color, debug } mode");

			return FALSE;
		}
	}

	// Set appropriate settings

	pGame->SetFillMode(FillModes(nFillMode));

	if (FILL_DEFAULT == nFillMode)
	{
		rEngine.SetOption(Engine::OPTION_WIREFRAME, FALSE);
	}
	else
	{
		ThemeStyle* pShared = rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

		if (NULL == pShared)
			return FALSE;

		switch(nFillMode)
		{
		case FILL_WIREFRAME:
			rEngine.GetGraphics().SetWireframeMaterial(
				pShared->GetMaterialInstance(SZ_MAT_WIREFRAME));
			break;
		case FILL_COLOR:
			rEngine.GetGraphics().SetWireframeMaterial(
				pShared->GetMaterialInstance(SZ_MAT_COLORFILL));
			break;
		case FILL_DEBUG:
			rEngine.GetGraphics().SetWireframeMaterial(
				pShared->GetMaterialInstance(SZ_MAT_DEBUG));
			break;
		}

		rEngine.SetOption(Engine::OPTION_WIREFRAME, TRUE);
	}
	
	return TRUE;
}

int Game::cmd_qcapture(Engine& rEngine, VariableArray& rParams)
{
	rEngine.GetGraphics().DebugCaptureQueue();

	return TRUE;
}

int Game::cmd_shell(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: expected string path.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): string expected.");

		return FALSE;
	}

	LPCWSTR pszOperation = NULL;

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_STRING &&
			rParams[1].GetVarType() != Variable::TYPE_ENUM)
		{
			rEngine.PrintError(L"invalid param type (2): "
				L"string or enum expected.");

			return FALSE;
		}

		pszOperation = rParams[1].GetStringValue();
	}

	LPCWSTR pszParameters = NULL;

	if (rParams.size() > 2)
	{
		if (rParams[2].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (3): string expected.");

			return FALSE;
		}

		pszParameters = rParams[2].GetStringValue();
	}

	LPCWSTR pszDirectory = NULL;

	if (rParams.size() > 3)
	{
		if (rParams[3].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (4): string expected.");

			return FALSE;
		}

		pszDirectory = rParams[3].GetStringValue();
	}

	int nShowCmd = SW_SHOW;

	if (rParams.size() > 4)
	{
		if (rParams[4].GetVarType() != Variable::TYPE_ENUM)
		{
			rEngine.PrintError(L"invalid param type (5): "
				L"enum showtypes expected.");

			return FALSE;
		}

		nShowCmd = rParams[4].GetEnumValue(SZ_SWFLAGS, 
			N_SWFLAGS, sizeof(N_SWFLAGS) / sizeof(int), SW_SHOW);
	}

	ShellExecute(rEngine.GetGameWindow(), pszOperation,
		rParams[0].GetStringValue(), pszParameters,
		pszDirectory, nShowCmd);

	return TRUE;
}

int Game::cmd_execute(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string scriptfile expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	LPCWSTR pszLabel = NULL;

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (2): expected string.");

			return FALSE;
		}

		pszLabel = rParams[1].GetStringValue();
	}

	// Execute the script

	String strMsg = PathFindFileName(rParams[0].GetStringValue());

	try
	{
		rEngine.GetCommands().ExecuteScriptFile(rParams[0].GetStringValue(),
			pszLabel);

		strMsg += L" execution complete.";

		rEngine.PrintInfo(strMsg);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		strMsg += L" execution failed.";

		rEngine.PrintError(strMsg);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_customcursor(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(Variable(rEngine.GetOption(
			Engine::OPTION_CUSTOM_CURSOR) == TRUE).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_INT &&
			rParams[0].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (1): int or bool expected.");

			return FALSE;
		}

		rEngine.SetOption(Engine::OPTION_CUSTOM_CURSOR, rParams[0].GetIntValue());
	}

	return FALSE;
}

int Game::cmd_setcustomcursor(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.Print(L"invalid syntax: expected string material-instance-name.",
			PRINT_ERROR);

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): string expected.");

		return FALSE;
	}

	ThemeStyle* m_pStyle =
		rEngine.GetScreens().GetTheme().GetStyle(SZ_SHARED_THEMESTYLE);

	if (NULL == m_pStyle)
	{
		rEngine.PrintError(L"shared theme style not found in loaded theme.");

		return FALSE;
	}

	MaterialInstance* pInst =
		m_pStyle->GetMaterialInstance(rParams[0].GetStringValue());

	if (NULL == pInst)
	{
		rEngine.PrintError(L"failed to find specified material instance in style.");

		return FALSE;
	}

	rEngine.GetCustomCursor() = *pInst;		

	// Center custom cursor

	Rect rcCustomCursor =
		rEngine.GetCustomCursor().GetTextureCoords();

	rEngine.SetCursorPosition(
		Vector3(float(rEngine.GetClientInstance()->GetDisplayWidth() -
		(rcCustomCursor.right - rcCustomCursor.left)) / 2.0f,
		float(rEngine.GetClientInstance()->GetDisplayHeight() -
		(rcCustomCursor.bottom - rcCustomCursor.top)) / 2.0f, 0.0f));

	return TRUE;
}

int Game::cmd_showcursor(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(Variable(rEngine.GetOption(
			Engine::OPTION_SHOW_CURSOR) == TRUE).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_BOOL &&
			rParams[0].GetVarType() != Variable::TYPE_INT)
		{
			rEngine.PrintError(L"invalid param type (1): int or bool expected.");

			return FALSE;
		}

		rEngine.SetOption(Engine::OPTION_SHOW_CURSOR, rParams[0].GetIntValue());
	}

	return FALSE;
}

int Game::cmd_showscreen(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string screenpath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	Screen* pScreen = NULL;

	if (rParams.size() > 1)
	{
		switch(rParams[1].GetVarType())
		{
		case Variable::TYPE_INT:
			pScreen = rEngine.GetScreens().FindByID(
				rParams[1].GetIntValue());
			break;
		case Variable::TYPE_STRING:
			pScreen = rEngine.GetScreens().FindByName(
				rParams[1].GetStringValue());
			break;
		default:
			rEngine.PrintError(L"invalid param type (2): expected int or string.");

			return FALSE;
		}

		if (pScreen != NULL)
		{
			if (pScreen->IsFlagSet(Screen::INVISIBLE) == true)
				pScreen->ClearFlag(Screen::INVISIBLE);

			rEngine.GetScreens().SetActiveScreen(pScreen);
			rEngine.GetScreens().SetForegroundScreen(pScreen);

			return TRUE;
		}
	}

	try
	{
		pScreen = rEngine.GetScreens().Show(rParams[0].GetStringValue());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return 0;
	}

	return TRUE;
}

int Game::cmd_loadscreen(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string screenpath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	Screen* pScreen = NULL;

	if (rParams.size() > 1)
	{
		switch(rParams[1].GetVarType())
		{
		case Variable::TYPE_INT:
			pScreen = rEngine.GetScreens().FindByID(rParams[1].GetIntValue());
			break;
		case Variable::TYPE_STRING:
			pScreen = rEngine.GetScreens().FindByName(rParams[1].GetStringValue());
			break;
		default:
			rEngine.PrintError(L"invalid param type (2): expected int or string.");
			return FALSE;
		}

		if (pScreen != NULL)
			return TRUE;
	}

	// Load screen

	try
	{
		pScreen = rEngine.GetScreens().Load(rParams[0].GetStringValue());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_closescreen(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Close all screens except for the active screen

		for(ScreenListIterator pos = rEngine.GetScreens().GetBeginPos();
			pos != rEngine.GetScreens().GetEndPos();
			pos++)
		{
			if (*pos != rEngine.GetScreens().GetActiveScreen() &&
				*pos != rEngine.GetScreens().GetFocusScreen() &&
				*pos != rEngine.GetScreens().GetCaptureScreen())
			{
				rEngine.GetScreens().Remove(pos, true);
			}
		}

		return FALSE;
	}

	Screen* pScreen = NULL;

	if (rParams[0].GetVarType() == Variable::TYPE_STRING)
	{
		// Find by name

		pScreen =
			rEngine.GetScreens().FindByName(rParams[0].GetStringValue());
	}
	else if (rParams[0].GetVarType() == Variable::TYPE_INT)
	{
		// Find by id

		pScreen =
			rEngine.GetScreens().FindByID(rParams[0].GetIntValue());
	}
	else
	{
		rEngine.PrintError(L"invalid param type (1): string or int expected.");

		return FALSE;
	}

	if (NULL == pScreen)
	{
		// Not found

		String str;

		if (rParams[0].GetVarType() == Variable::TYPE_STRING)
		{
			str.Format(L"screen not found: \"%s\"",
				rParams[0].GetStringValue());
		}
		else
		{
			str.Format(L"screen not found: %d", rParams[0].GetIntValue());
		}

		return FALSE;
	}

	// If this is an overlapped screen, do graceful closure with animation

	ScreenOverlapped* pOverlapped =
		dynamic_cast<ScreenOverlapped*>(pScreen);

	if (pOverlapped != NULL)
		pOverlapped->Close();
	else
		pScreen->Release();

	return TRUE;
}

int Game::cmd_showfps(Engine& rEngine, VariableArray& rParams)
{
	ScreenFps* pScreenFps = dynamic_cast<ScreenFps*>(
		rEngine.GetScreens().FindByID(1010));

	bool bShow = (pScreenFps == NULL);
	bool bExtended = false;

	if (rParams.size())
	{
		if (rParams[0].GetVarType() != Variable::TYPE_BOOL &&
			rParams[0].GetVarType() != Variable::TYPE_INT)
		{
			rEngine.PrintError(L"invalid param type (1): expected int or bool.");

			return FALSE;
		}

		bShow = rParams[0].GetBoolValue();

		if (rParams.size() > 1)
		{
			if (rParams[1].GetVarType() != Variable::TYPE_BOOL &&
				rParams[0].GetVarType() != Variable::TYPE_INT)
			{
				rEngine.PrintError(L"invalid param type (2): expected int or bool.");

				return FALSE;
			}

			bExtended = rParams[1].GetBoolValue();
		}
	}

	if (true == bShow)
	{
		if (pScreenFps != NULL)
		{
			// Extend or retract fps screen

			if (pScreenFps->GetExtended() != bExtended)
				pScreenFps->SetExtended(bExtended);

			return TRUE;
		}
		else
		{
			// Show fps screen when not displayed

			pScreenFps = dynamic_cast<ScreenFps*>(
				rEngine.GetScreens().Show(L"fps"));

			if (pScreenFps != NULL && pScreenFps->GetExtended() != bExtended)
				pScreenFps->SetExtended(bExtended);
		}		
	}
	else
	{
		if (pScreenFps != NULL)
			pScreenFps->Close();
	}
	
	return TRUE;
}

int Game::cmd_alignfps(Engine& rEngine, VariableArray& rParams)
{
	Screen* pScreenFps =
		rEngine.GetScreens().FindByID(ScreenFps::ID);

	if (NULL == pScreenFps)
	{
		rEngine.PrintError(L"failed to align fps screen: not loaded.");
		return FALSE;
	}

	const int OFFSET_TOP = 2;
	const int OFFSET_LEFT = 2;
	const int OFFSET_RIGHT = 4;
	const int OFFSET_BOTTOM = 2;

	for(VariableArrayIterator pos = rParams.begin();
		pos != rParams.end();
		pos++)
	{
		if ((*pos).GetVarType() == Variable::TYPE_ENUM)
		{
			if (wcscmp((*pos).GetStringValue(), L"left") == 0)
			{
				// Left

				pScreenFps->SetPosition(OFFSET_LEFT,
					pScreenFps->GetPosition().y);
			}
			else if (wcscmp((*pos).GetStringValue(), L"top") == 0)
			{
				// Top

				pScreenFps->SetPosition(pScreenFps->GetPosition().x,
					OFFSET_TOP);
			}
			else if (wcscmp((*pos).GetStringValue(), L"right") == 0)
			{
				// Right

				pScreenFps->SetPosition(
					rEngine.GetClientInstance()->GetDisplayWidth() -
					pScreenFps->GetSize().cx - OFFSET_RIGHT,
					pScreenFps->GetPosition().y);
			}
			else if (wcscmp((*pos).GetStringValue(), L"bottom") == 0)
			{
				// Bottom

				pScreenFps->SetPosition(pScreenFps->GetPosition().x,
					rEngine.GetClientInstance()->GetDisplayHeight() -
					pScreenFps->GetSize().cy - OFFSET_BOTTOM);
			}
			else if (wcscmp((*pos).GetStringValue(), L"center") == 0)
			{
				// Horizontal center
				
				pScreenFps->SetPosition((
					rEngine.GetClientInstance()->GetDisplayWidth() -
					pScreenFps->GetSize().cx) / 2,
					pScreenFps->GetPosition().y);
			}
		}
	}

	return TRUE;
}

int Game::cmd_showstart(Engine& rEngine, VariableArray& rParams)
{
	// Show random start screen in the range specified

	if (rParams.size() < 2)
	{
		rEngine.PrintError(L"invalid syntax: expected int rangemin, int rangemax.");
		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_INT &&
		rParams[0].GetVarType() != Variable::TYPE_BOOL)
	{
		rEngine.PrintError(L"invalid param type (1): expected int.");
		return FALSE;
	}

	if (rParams[1].GetVarType() != Variable::TYPE_INT &&
		rParams[1].GetVarType() != Variable::TYPE_BOOL)
	{
		rEngine.PrintError(L"invalid param type (2): expected int.");
		return FALSE;
	}

	srand(GetTickCount());

	int nIndex = (rand() % rParams[1].GetIntValue()) +
		rParams[0].GetIntValue();

	String strName;
	strName.Format(L"start%.2d", nIndex);

	VariableArray arShowParams;

	arShowParams.resize(1);
	arShowParams[0].SetStringValue(strName);

	cmd_showscreen(rEngine, arShowParams);

	return TRUE;
}

int Game::cmd_minimize(Engine& rEngine, VariableArray& rParams)
{
	ShowWindow(rEngine.GetGameWindow(), SW_MINIMIZE);

	return TRUE;
}

int Game::cmd_pausegame(Engine& rEngine, VariableArray& rParams)
{
	VariableArray arShowParams;

	arShowParams.resize(1);
	arShowParams[0].SetStringValue(L"pause");

	cmd_showscreen(rEngine, arShowParams);

	return TRUE;
}

int Game::cmd_mastervolume(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(
			Variable(rEngine.GetAudio().GetMasterVolume()).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_FLOAT)
		{
			rEngine.PrintError(L"invalid param type (1): expected float.");

			return FALSE;
		}

		rEngine.GetAudio().SetMasterVolume(rParams[0].GetFloatValue());
	}

	return TRUE;
}

int Game::cmd_mastermute(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(
			Variable(rEngine.GetAudio().GetMasterVolumeMute()).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_INT ||
			rParams[0].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (1): expected int or bool.");

			return FALSE;
		}

		try
		{
			rEngine.GetAudio().SetMasterVolumeMute(rParams[0].GetBoolValue());
		}

		catch(Error& rError)
		{
			UNREFERENCED_PARAMETER(rError);

			rEngine.PrintError(L"failed to set master volume mute.");

			PrintLastError(rEngine);

			return FALSE;
		}
	}

	return TRUE;
}

int Game::cmd_musicvolume(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame)
		return FALSE;

	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(Variable(pGame->m_fMusicVolume).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_FLOAT)
		{
			rEngine.PrintError(L"invalid param type (1): expected float.");
			return FALSE;
		}

		pGame->SetMusicVolume(rParams[0].GetFloatValue());
	}

	return TRUE;
}

int Game::cmd_effectsvolume(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());
	
	if (NULL == pGame)
		return FALSE;

	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(Variable(pGame->m_fEffectsVolume).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_FLOAT)
		{
			rEngine.PrintError(L"invalid param type (1): expected float.");

			return FALSE;
		}

		pGame->SetEffectsVolume(rParams[0].GetFloatValue());
	}

	return TRUE;
}

int Game::cmd_speechvolume(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame)
		return FALSE;

	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(Variable(pGame->m_fSpeechVolume).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_FLOAT)
		{
			rEngine.PrintError(L"invalid param type (1): expected float.");

			return FALSE;
		}

		pGame->SetSpeechVolume(rParams[0].GetFloatValue());
	}

	return TRUE;
}

int Game::cmd_playsound(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame)
		return FALSE;

	if (rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
		return TRUE;
	
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string soundpath, bool loop, "
			L"enum { effect* | speech } chanel expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	// Load sound

	Sound* pSound = NULL;
	
	try
	{
		pSound = rEngine.GetSounds().Load(rParams[0].GetStringValue());
	}

	catch(std::exception)
	{
		PrintLastError(rEngine);
		return FALSE;
	}

	bool bLoop = false;

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_INT ||
			rParams[1].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (2): expected bool.");

			return FALSE;
		}

		bLoop = rParams[1].GetBoolValue();
	}

	bool bSpeech = false;

	if (rParams.size() > 2)
	{
		if (rParams[2].GetVarType() != Variable::TYPE_ENUM)
		{
			rEngine.PrintError(L"invalid param type (3): expected enum.");

			return FALSE;
		}

		bSpeech = (rParams[2].GetString() == L"speech");
	}

	// Play sound

	try
	{	
		pSound->Play(bLoop,
			true == bSpeech ? CHANNEL_SPEECH : CHANNEL_EFFECTS);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_playmusic(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());
	if (NULL == pGame) return FALSE;

	if (rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return TRUE;

	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string musicpath, "
			L"bool loop expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	try
	{
		// Load music

		Music* pMusic = rEngine.GetMusic().Load(rParams[0].GetStringValue());

		bool bLoop = false;

		if (rParams.size() > 1)
		{
			if (rParams[1].GetVarType() != Variable::TYPE_INT ||
				rParams[1].GetVarType() != Variable::TYPE_BOOL)
			{
				rEngine.PrintError(L"invalid param type (2): expected int or bool.");

				return FALSE;
			}

			bLoop = rParams[1].GetBoolValue();
		}

		pMusic->SetVolume(pGame->m_fMusicVolume);

		// Play music

		pMusic->Play(bLoop);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return FALSE;
	}

	return TRUE;
}

int Game::cmd_playvideo(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string videopath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	bool bAsync = true;

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_BOOL &&
			rParams[1].GetVarType() != Variable::TYPE_INT)
		{
			rEngine.PrintError(L"invalid param type (2): expected int or bool.");

			return FALSE;
		}

		bAsync = rParams[1].GetBoolValue();
	}

	try
	{
		// Load video (disable caching on unload)

		Video* pVideo = rEngine.GetVideos().Load(rParams[0].GetStringValue(), 0.0f);

		// Set volume

		Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

		if (pGame != NULL)
			pVideo->SetVolume(pGame->m_fEffectsVolume);

		// Play video

		pVideo->Play();

		// If not playing asynchronously, wait until it stops

		if (false == bAsync)
		{
			while(rEngine.GetCurrentVideo() == pVideo && pGame->IsRunning())
			{
				pGame->DoEvents();
			}
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return FALSE;
	}

	return TRUE;
}

int Game::cmd_stopsound(Engine& rEngine, VariableArray& rParams)
{
	int nStopped = 0;

	if (rParams.empty() == true)
	{
		// Stop all sounds
		
		nStopped = rEngine.GetAudio().GetSoundInstances().GetCount();

		rEngine.GetAudio().GetSoundInstances().RemoveAll();
	}
	else
	{
		// Stop sounds matching a specific pattern

		if (rParams[0].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");

			return FALSE;
		}

		for(SoundInstanceArrayIterator pos =
			rEngine.GetAudio().GetSoundInstances().GetBeginPos();
			pos != rEngine.GetAudio().GetSoundInstances().GetEndPos();
			pos++)
		{
			if (NULL == *pos) continue;

			if (wcsstr((*pos)->GetSound().GetName(),
				rParams[0].GetStringValue()))
			{
				rEngine.GetAudio().GetSoundInstances().Remove(pos);
				nStopped++;
			}
		}
	}

	if (nStopped > 0)
	{
		if (nStopped == 1)
			rEngine.PrintMessage(L"stopped 1 sound.");
		else
			rEngine.PrintMessage(L"stopped %d sounds.", nStopped);
	}
	else
	{
		rEngine.PrintMessage(L"no sounds stopped.");
	}

	return TRUE;
}

int Game::cmd_stopmusic(Engine& rEngine, VariableArray& rParams)
{
	Music* pMusic;
	int nStopped = 0;

	if (rParams.empty() == true)
	{
		// Stop all music

		for(ResourceManager<Music>::ConstIterator pos =
			rEngine.GetMusic().GetBeginPosConst();
			pos != rEngine.GetMusic().GetEndPosConst();
			pos++)
		{
			pMusic = pos->second;

			if (NULL == pMusic) continue;

			if (pMusic->IsFlagSet(Music::PLAYING) == true)
			{
				nStopped++;

				pMusic->Stop();
			}
		}
	}
	else
	{
		// Stop music matching a specific pattern

		if (rParams[0].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");

			return FALSE;
		}

		for(ResourceManager<Music>::ConstIterator pos =
			rEngine.GetMusic().GetBeginPosConst();
			pos != rEngine.GetMusic().GetEndPosConst();
			pos++)
		{
			pMusic = pos->second;

			if (NULL == pMusic) continue;

			if (pMusic->IsFlagSet(Music::PLAYING) == true &&
			   pMusic->GetName().Find(rParams[0].GetStringValue()))
			{
				nStopped++;

				pMusic->Stop();
			}
		}
	}

	if (nStopped > 0)
	{
		if (1 == nStopped)
			rEngine.PrintMessage(L"stopped 1 music resource.");
		else
			rEngine.PrintMessage(L"stopped %d music resources.", nStopped);
	}
	else
	{
		rEngine.PrintMessage(L"no music resources stopped.");
	}

	return TRUE;
}

int Game::cmd_stopvideo(Engine& rEngine, VariableArray& rParams)
{
	try
	{
		// Only one video can be playing at the same time,
		// so stop current video if any

		if (rEngine.GetCurrentVideo() != NULL)
			rEngine.GetCurrentVideo()->Stop();
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return FALSE;
	}

	return TRUE;
}

int Game::cmd_break(Engine& rEngine, VariableArray& rParams)
{
	DebugBreak();

	return TRUE;
}

int Game::cmd_crash(Engine& rEngine, VariableArray& rParams)
{
	__asm
	{
		xor eax, eax
		mov ecx, [eax]
	};

	return TRUE;
}

int Game::cmd_restart(Engine& rEngine, VariableArray& rParams)
{
	if (NULL == rEngine.GetClientInstance())
		return FALSE;

	rEngine.GetClientInstance()->Exit();

	WCHAR szExePath[MAX_PATH] = {0};
	GetModuleFileName(GetModuleHandle(NULL), szExePath, MAX_PATH);

	ShellExecute(GetDesktopWindow(),
		NULL, szExePath, NULL, rEngine.GetClientInstance()->GetBaseDirectory(),
		SW_SHOW);

	return TRUE;
}

int Game::cmd_openconsole(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame || NULL == pGame->GetConsole())
		return FALSE;

	bool bFullOpen = false;

	if (rParams.size() > 0)
	{
		if (rParams[0].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (1): expected bool.");

			return FALSE;
		}

		bFullOpen = rParams[0].GetBoolValue();
	}

	if (rEngine.GetScreens().GetActiveScreen() != pGame->GetConsole())
		pGame->GetConsole()->Toggle(bFullOpen);

	return TRUE;
}

int Game::cmd_print(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.Print();
	}
	else
	{
		if (rParams[0].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");

			return FALSE;
		}

		PrintTypes nType = PRINT_MESSAGE;

		if (rParams.size() > 1)
		{
			if (rParams[1].GetVarType() != Variable::TYPE_ENUM)
			{
				rEngine.PrintError(L"invalid param type (2) expected enum.");

				return FALSE;
			}

			nType = (PrintTypes)rParams[1].GetEnumValue(SZ_PRINTTYPES,
				N_PRINTTYPES, sizeof(N_PRINTTYPES) / sizeof(int),
				PRINT_MESSAGE);
		}

		rEngine.Print(rParams[0].GetStringValue(), nType);
	}

	return TRUE;
}

int Game::cmd_vartype(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: enum varname expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): expected enum.");

		return FALSE;
	}

	LPCWSTR pszVarName = rParams[0].GetStringValue();

	Variable* pVar = rEngine.GetVariables().Find(pszVarName);

	if (NULL == pVar)
	{
		// If not found in Engine variables, check current map

		if (rEngine.GetCurrentMapConst() == NULL)
		{
			rEngine.PrintError(L"variable not found, current map not searched.");

			return FALSE;
		}

		pVar = rEngine.GetCurrentMap()->GetVariables().Find(pszVarName);

		if (NULL == pVar)
		{
			rEngine.PrintError(L"variable not found, current map searched.");

			return FALSE;
		}
	}

	rEngine.PrintMessage(pVar->GetVarTypeString());

	return TRUE;
}

int Game::cmd_mapvar(Engine& rEngine, VariableArray& rParams)
{
	if (rEngine.GetCurrentMapConst() == NULL)
	{
		rEngine.PrintError(L"no current map.");
		return FALSE;
	}

	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: enum varname {...} expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): expected enum.");

		return FALSE;
	}

	LPCWSTR pszVarName = rParams[0].GetStringValue();

	Variable* pVar =
		rEngine.GetCurrentMap()->GetVariables().Find(pszVarName);

	if (rParams.size() > 1)
	{
		// Set map variable

		if (NULL == pVar)
			pVar = rEngine.GetCurrentMap()->GetVariables().Add(pszVarName,
			Variable::TYPE_UNDEFINED);

		*pVar = rParams[1];
	}
	else
	{
		// Get map variable

		if (NULL == pVar)
		{
			rEngine.PrintError(L"map variable not found.");
			return FALSE;
		}

		rEngine.PrintMessage(pVar->ToString());
	}

	return TRUE;
}

int Game::cmd_echo(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		rEngine.PrintMessage(
			Variable(rEngine.GetOption(Engine::OPTION_ENABLE_ECHO) == TRUE).ToString());
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_INT &&
			rParams[0].GetVarType() != Variable::TYPE_BOOL)
		{
			rEngine.PrintError(L"invalid param type (1): "
				L"int or bool expected.");

			return FALSE;
		}

		rEngine.SetOption(Engine::OPTION_ENABLE_ECHO,
			rParams[0].GetBoolValue());
	}

	return TRUE;
}

int Game::cmd_makeregion(Engine& rEngine, VariableArray& rParams)
{
	/*
	if (rParams.size() != 2)
	{
		rEngine.Print(L"invalid syntax: string srcrespath, "
			L"string destrespath expected.", PRINT_ERROR);

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.Print(L"invalid param type (1): string expected.",
			PRINT_ERROR);

		return FALSE;
	}

	if (rParams[1].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.Print(L"invalid param type (2): string expected.",
			PRINT_ERROR);

		return FALSE;
	}

	RegionSet* pRegionSet = NULL;

	try
	{
		// Load resource to create region set from

		Resource* pResource = rEngine.GetClientInstance()->LoadResource(
			rParams[0].GetStringValue());

		// Create a region set

		pRegionSet = rEngine.GetResources().Create<RegionSet>();

		// Load based on resource type

		if (dynamic_cast<Sprite*>(pResource) != NULL)
		{
			// Create a region set from sprite	

			pRegionSet->FromSprite(*static_cast<Sprite*>(pResource));
		}
		else if (dynamic_cast<Animation*>(pResource) != NULL)
		{
			// Create region set from animation

			pRegionSet->FromAnimation(*static_cast<Animation*>(pResource));
		}

		// Save it to destination file

		pRegionSet->Serialize(rParams[1].GetStringValue());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		delete pRegionSet;

		rEngine.Print(L"resource not found.", PRINT_ERROR);

		PrintLastError(rEngine);

		return FALSE;
	}
	*/

	return TRUE;
}

int Game::cmd_savetexture(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string name-pattern [, bool open]");
		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");
		return FALSE;
	}

	Texture* pTexture =
		rEngine.GetTextures().FindPattern(rParams[0].GetStringValue());

	if (NULL == pTexture)
	{
		rEngine.PrintError(L"texture not found.");
		return FALSE;
	}

	String strPath;

	rEngine.GetBaseFilePath(PathFindFileName(pTexture->GetName()),
		NULL, L".png", strPath.Allocate(MAX_PATH));

	try
	{
		pTexture->Serialize(strPath);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_BOOL &&
			rParams[1].GetVarType() != Variable::TYPE_INT)
		{
			rEngine.PrintError(L"invalid param type (2): bool or int expected.");
			return FALSE;
		}

		if (rParams[1].GetBoolValue() == true)
		{
			ShellExecute(rEngine.GetGameWindow(),
				L"open", strPath, NULL, NULL, SW_SHOW);
		}
	}

	return TRUE;
}

int Game::cmd_regiontotexture(Engine& rEngine, VariableArray& rParams)
{
	/*
	if (rParams.size() != 3)
	{
		rEngine.Print(L"invalid syntax: string regionsetpath, "
			L"int regionid, string destpath expected.", PRINT_ERROR);

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.Print(L"invalid param type (1): expected string.",
			PRINT_ERROR);

		return FALSE;
	}

	if (rParams[1].GetVarType() != Variable::TYPE_INT)
	{
		rEngine.Print(L"invalid param type (2): expected int.",
			PRINT_ERROR);

		return FALSE;
	}

	if (rParams[2].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.Print(L"invalid param type (3): expected string.",
			PRINT_ERROR);

		return FALSE;
	}

	RegionSet* pRgnSet =
		rEngine.GetRegions().Load(rParams[0].GetStringValue());

	if (NULL == pRgnSet)
	{
		rEngine.Print(L"failed to find or load region set.",
			PRINT_ERROR);

		PrintLastError(rEngine);

		return FALSE;
	}

	Region* pRgn = pRgnSet->GetRegion(rParams[2].GetIntValue());
	
	if (NULL == pRgn)
	{
		rEngine.Print(L"could not get region with a specified id.",
			PRINT_ERROR);

		PrintLastError(rEngine);

		return FALSE;
	}

	const SIZE& rRegionSize = pRgn->GetSize();

	// Create dynamic texture in system memory

	LPDIRECT3DTEXTURE9 pTex = NULL;

	D3DXCreateTexture(rEngine.GetGraphics().GetDevice(),
		rRegionSize.cx, rRegionSize.cy, 1, D3DUSAGE_DYNAMIC,
		D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &pTex);

	if (NULL == pTex)
	{
		rEngine.Print(L"failed to create texture for a region.",
			PRINT_ERROR);

		return FALSE;
	}

	// Get its surface

	LPDIRECT3DSURFACE9 pSurf = NULL;

	pTex->GetSurfaceLevel(0, &pSurf);

	if (NULL == pSurf)
	{
		rEngine.Print(L"failed to get new texture surface level.",
			PRINT_ERROR);

		return FALSE;
	}

	// Get surface's DC

	HDC hDC = NULL;
	pSurf->GetDC(&hDC);

	if (NULL == hDC)
	{
		pSurf->Release();
		pTex->Release();

		rEngine.Print(L"failed to get surface dc.", PRINT_ERROR);

		return FALSE;
	}

	// Clear DC with black color

	RECT rcClear;
	FillRect(hDC, &rcClear, (HBRUSH)GetStockObject(BLACK_BRUSH));

	// Set opaque pixels on DC to white

	const BYTE** pbRgnData = pRgn->GetData2DConst();

	for(int y = 0; y < rRegionSize.cy; y++)
	{
		for(int x = 0; x < rRegionSize.cx; x++)
		{
			if (pbRgnData[y][x]) SetPixel(hDC, x, y, 0xFFFFFFFF);
		}
	}

	// Cleanup

	pSurf->ReleaseDC(hDC);
	pSurf->Release();

	// Save texture to file

	D3DXIMAGE_FILEFORMAT nFormat = ImageFormatFromExtension(
		PathFindExtension(rParams[2].GetStringValue()));

	if (FAILED(D3DXSaveTextureToFile(rParams[2].GetStringValue(),
		nFormat, pTex, NULL)))
	{
		pSurf->ReleaseDC(hDC);
		pSurf->Release();
		pTex->Release();

		rEngine.Print(L"failed to save texture.", PRINT_ERROR);

		return FALSE;
	}

	pTex->Release();

	rEngine.Print(L"texture saved successfully.", PRINT_MESSAGE);
	*/

	return TRUE;
}

int Game::cmd_verifyunicode(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string dirname expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	// Convert all files in specified directory with 
	// specified filter to unicode

	LPCWSTR pszFilter = L"*";

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");

			return FALSE;
		}

		pszFilter = rParams[1].GetStringValue();
	}

	WCHAR szSearch[MAX_PATH] = {0};
	wcscpy_s(szSearch, MAX_PATH, rParams[0].GetStringValue());

	LPWSTR pszTitleStart = PathAddBackslash(szSearch);

	wcscpy_s(pszTitleStart,
		sizeof(szSearch) / sizeof(WCHAR) - (pszTitleStart - szSearch),
		pszFilter);

	WIN32_FIND_DATA wfd = {0};

	HANDLE hFind = FindFirstFile(szSearch, &wfd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		rEngine.PrintError(L"no files found.");
		return FALSE;
	}

	rEngine.PrintInfo(L"\nBEGIN FILES PROCESSED");

	Stream stream;

	try
	{
		do
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			// Get this file's full path

			wcscpy_s(pszTitleStart,
					 sizeof(szSearch) / sizeof(WCHAR) - (pszTitleStart - szSearch),
					 wfd.cFileName);

			// Process this file

			stream.Open(szSearch, GENERIC_READ, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN);

			LPSTR pszBuffer = (LPSTR)stream.CreateReadBuffer();

			if (0xFEFF == *(WORD*)pszBuffer)
			{
				// Already unicode

				rEngine.Print(wfd.cFileName, PRINT_MESSAGE);
			}
			else
			{
				// Check the first 16 bytes for a non-ascii character

				int nLen = int(stream.GetSize());

				int nLenCheck = nLen > 16 ? 16 : nLen;

				bool bBinaryFile = false;

				for(int n = 0; n < nLenCheck; n++)
				{
					if (pszBuffer[n] < 0x20)
					{
						bBinaryFile = true;

						break;
					}
				}

				// Convert to unicode

				if (true == bBinaryFile)
				{
					// This is a binary file!

					rEngine.PrintWarning(wfd.cFileName);
				}
				else
				{
					// Allocate buffer for file data,
					// including unicode signature and nullchar

					String strBuffer;
					strBuffer.Allocate(nLen + 1);

					// First char is unicode signature

					strBuffer[0] = 0xFEFF;

					// Convert from ASCII to unicode

					mbstowcs_s(NULL, strBuffer.GetBuffer() + 1,
						nLen + 1, pszBuffer, nLen);

					stream.Empty();

					stream.Open(szSearch, GENERIC_WRITE, CREATE_ALWAYS);

					// Write unicode data to file

					stream.Write((LPCVOID)strBuffer.GetBufferConst(),
						DWORD((nLen + 1) * sizeof(WCHAR)));

					rEngine.PrintInfo(wfd.cFileName);
				}
			}
		} while(FindNextFile(hFind, &wfd));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		FindClose(hFind);

		rEngine.PrintError(wfd.cFileName);

		return FALSE;
	}

	FindClose(hFind);

	rEngine.PrintInfo(L"END FILES PROCESSED");

	return TRUE;
}

int Game::cmd_load(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string respath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	// Load the resource

	if (rEngine.GetClientInstance() == NULL)
		return FALSE;

	Resource* pResource = NULL;

	try
	{
		pResource = rEngine.GetClientInstance()->LoadResource(
			rParams[0].GetStringValue());
	}

	catch(Error&)
	{
		PrintLastError(rEngine);

		return FALSE;
	}

	rEngine.PrintMessage(L"resource loaded successfully.");

	return TRUE;
}

int Game::cmd_unload(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.size() < 1)
	{
		rEngine.PrintError(L"invalid syntax: string res_path_or_title expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	LPCWSTR pszSearch = rParams[0].GetStringValue();

	bool bPattern = (PathGetDriveNumber(pszSearch) == -1);

	Resource* pRes = (true == bPattern) ?
		rEngine.GetTextures().FindPattern(pszSearch) :
	    rEngine.GetTextures().Find(pszSearch);

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetCubeTextures().FindPattern(pszSearch) :
			rEngine.GetCubeTextures().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetRegions().FindPattern(pszSearch) :
			rEngine.GetRegions().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetAnimations().FindPattern(pszSearch) :
			rEngine.GetAnimations().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetMaterials().FindPattern(pszSearch) :
			rEngine.GetMaterials().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetEffects().FindPattern(pszSearch) :
			rEngine.GetEffects().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetSprites().FindPattern(pszSearch) :
			rEngine.GetSprites().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetSounds().FindPattern(pszSearch) :
			rEngine.GetSounds().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetMusic().FindPattern(pszSearch) :
			rEngine.GetMusic().Find(pszSearch);
	}
	
	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetVideos().FindPattern(pszSearch) :
			rEngine.GetVideos().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetFonts().FindPattern(pszSearch) :
			rEngine.GetFonts().Find(pszSearch);
	}

	if (NULL == pRes)
	{
		pRes = (true == bPattern) ?
			rEngine.GetStrings().FindPattern(pszSearch) :
			rEngine.GetStrings().Find(pszSearch);
	}

	if (pRes != NULL)
	{
		while(pRes->GetRefCount() > 0)
			pRes->Release();

		rEngine.PrintInfo(L"resource unloaded.");
	}
	else
	{
		rEngine.PrintError(L"resource not found.");
	}

	return TRUE;
}

int Game::cmd_reload(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.size() != 1)
	{
		rEngine.PrintError(L"invalid syntax: string pathortitle expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}	
	
	LPCWSTR pszSearch = rParams[0].GetStringValue();

	bool bPattern = (PathGetDriveNumber(pszSearch) == -1);

	Resource* pResource = (true == bPattern) ?
		rEngine.GetTextures().FindPattern(pszSearch) :
		rEngine.GetTextures().Find(pszSearch);

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetCubeTextures().FindPattern(pszSearch) :
			rEngine.GetCubeTextures().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetRegions().FindPattern(pszSearch) :
			rEngine.GetRegions().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetAnimations().FindPattern(pszSearch) :
			rEngine.GetAnimations().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetMaterials().FindPattern(pszSearch) :
			rEngine.GetMaterials().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetEffects().FindPattern(pszSearch) :
			rEngine.GetEffects().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetSprites().FindPattern(pszSearch) :
			rEngine.GetSprites().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetSounds().FindPattern(pszSearch) :
			rEngine.GetSounds().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetMusic().FindPattern(pszSearch) :
			rEngine.GetMusic().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetVideos().FindPattern(pszSearch) :
			rEngine.GetVideos().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetFonts().FindPattern(pszSearch) :
			rEngine.GetFonts().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		pResource = (true == bPattern) ?
			rEngine.GetStrings().FindPattern(pszSearch) :
			rEngine.GetStrings().Find(pszSearch);
	}

	if (NULL == pResource)
	{
		rEngine.PrintError(L"failed to find resource.");
		return FALSE;
	}

	try
	{
		pResource->Reload();

		rEngine.PrintMessage(L"resource reloaded successfully.");
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);
		return FALSE;
	}

	return TRUE;
}

int Game::cmd_test(Engine& rEngine, VariableArray& rParams)
{

	return TRUE;
}

int Game::cmd_errorexit(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string errormessage expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());

	if (NULL == pGame)
		return FALSE;

	pGame->OnError(ErrorGame(ErrorGame::CUSTOM, __FUNCTIONW__,
		rParams[0].GetStringValue()));

	if (pGame != NULL)
		pGame->Exit();

	return TRUE;
}

int Game::cmd_lasterror(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		PrintLastError(rEngine);
	}
	else
	{
		if (rParams[0].GetVarType() != Variable::TYPE_ENUM)
		{
			rEngine.PrintError(L"invalid param type (1): expected enum { clear }.");

			return FALSE;
		}

		if (rParams[0].GetString() == L"clear")
			rEngine.GetErrors().Empty();
	}

	return TRUE;
}

int Game::cmd_benchmark(Engine& rEngine, VariableArray& rParams)
{
	Game* pGame = dynamic_cast<Game*>(rEngine.GetClientInstance());
	if (NULL == pGame) return FALSE;

	__int64	qwFreq = 0;

	if (QueryPerformanceFrequency((LARGE_INTEGER*)&qwFreq) == FALSE)
		return FALSE;

	if (rParams.empty() == true ||
	  (rParams[0].GetVarType() == Variable::TYPE_ENUM &&
	   rParams[0].GetString() == L"start"))
	{
		// Start benchmark		

		if (QueryPerformanceCounter((
			LARGE_INTEGER*)&pGame->m_qwLastBench) == FALSE)
			return FALSE;

		rEngine.PrintInfo(L"benchmark started.");
	}
	else
	{
		// End benchmark

		__int64 qwCurTime;

		if (QueryPerformanceCounter((LARGE_INTEGER*)&qwCurTime) == FALSE)
			return FALSE;

		__int64 qwDiff = qwCurTime - pGame->m_qwLastBench;

		double dElapsed = double(qwDiff) / double(qwFreq);

		rEngine.PrintInfo(L"benchmark over, %f seconds (or %f milliseconds, "
			L"or %f microseconds) passed.",
			dElapsed, dElapsed * 1000.0f, dElapsed * 1000000.0f);
	}

	return TRUE;
}

int Game::cmd_dir(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rParams.resize(1);

		WCHAR szCurDir[MAX_PATH - 1] = {0};
		GetCurrentDirectory(MAX_PATH, szCurDir);

		rParams[0].SetStringValue(szCurDir);
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	// Print directory contents

	LPCWSTR pszFilter = L"*";

	if (rParams.size() > 1)
	{
		if (rParams[1].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");
			return FALSE;
		}

		pszFilter = rParams[1].GetStringValue();
	}

	WCHAR szSearch[MAX_PATH] = {0};

	wcscpy_s(szSearch, MAX_PATH, rParams[0].GetStringValue());		
	PathAppend(szSearch, pszFilter);

	WIN32_FIND_DATA wfd = {0};

	HANDLE hFind = FindFirstFile(szSearch, &wfd);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		rEngine.PrintError(L"no files found.");
		return FALSE;
	}

	rEngine.PrintInfo(L"\nBEGIN DIRECTORY CONTENTS");

	String strFound;

	do
	{
		if (L'.' == wfd.cFileName[0])
		{
			if (L'\0' == wfd.cFileName[1]) continue;

			if (L'.' == wfd.cFileName[1] &&
				L'\0' == wfd.cFileName[2]) continue;
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			strFound.Format(L"   [%s]", wfd.cFileName);
		}
		else
		{
			strFound.Format(L"   %s", wfd.cFileName);
		}

		rEngine.Print(strFound, PRINT_INFO);

	} while(FindNextFile(hFind, &wfd));

	FindClose(hFind);

	rEngine.PrintInfo(L"END DIRECTORY CONTENTS");

	return TRUE;
}

int Game::cmd_curdir(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		// Get

		WCHAR szCurDir[MAX_PATH] = {0};

		GetCurrentDirectory(MAX_PATH, szCurDir);

		rEngine.Print(szCurDir, PRINT_MESSAGE);
	}
	else
	{
		// Set

		if (rParams[0].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (1): expected string.");

			return FALSE;
		}

		SetCurrentDirectory(rParams[0].GetStringValue());
	}

	return TRUE;
}

int Game::cmd_map(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string path expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	try
	{
		// Load specified map

		TileMap* pMap = rEngine.GetMaps().Load(rParams[0].GetStringValue());

		// Set it as current map

		rEngine.SetCurrentMap(pMap);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}	

	return TRUE;
}

int Game::cmd_unloadmap(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string path expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	LPCWSTR pszSearch = rParams[0].GetStringValue();

	// Find map

	TileMap* pMap = (PathGetDriveNumber(pszSearch) == -1) ?
		rEngine.GetMaps().FindPattern(pszSearch) :
	    rEngine.GetMaps().Find(pszSearch);

	if (pMap != NULL)
		rEngine.GetMaps().Remove(pMap);

	return TRUE;
}

int Game::cmd_savemap(Engine& rEngine, VariableArray& rParams)
{
	try
	{
		if (rParams.size() == 1)
		{
			if (rParams[0].GetVarType() != Variable::TYPE_STRING)
			{
				rEngine.PrintError(L"invalid param type (1): expected string.");
				return FALSE;
			}

			// Save current map

			rEngine.GetMaps().Save(rEngine.GetCurrentMap(), rParams[0].GetStringValue());
		}
		else
		{
			if (rParams.size() != 2)
			{
				rEngine.PrintError(L"invalid syntax: string searchpattern, "
					L"string path expected.");

				return FALSE;
			}

			if (rParams[0].GetVarType() != Variable::TYPE_STRING)
			{
				rEngine.PrintError(L"invalid param type (1): expected string.");

				return FALSE;
			}

			if (rParams[1].GetVarType() != Variable::TYPE_STRING)
			{
				rEngine.PrintError(L"invalid param type (2): expected string.");

				return FALSE;
			}

			// Save specified map

			TileMap* pMap = *rParams[1].GetStringValue() ?
				rEngine.GetMaps().Find(rParams[1].GetStringValue()) :
				rEngine.GetMaps().FindPattern(rParams[0].GetStringValue());

			rEngine.GetMaps().Save(pMap, rParams[0].GetStringValue());
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_loadgame(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string loadfilepath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	try
	{
		rEngine.DeserializeSession(rParams[0].GetStringValue());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_savegame(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: string savefilepath expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING)
	{
		rEngine.PrintError(L"invalid param type (1): expected string.");

		return FALSE;
	}

	try
	{
		rEngine.SerializeSession(rParams[0].GetStringValue());
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		PrintLastError(rEngine);

		return FALSE;
	}

	return TRUE;
}

int Game::cmd_quickload(Engine& rEngine, VariableArray& rParams)
{
	VariableArray params;

	params.resize(1);
	params[0].SetStringValue(L"quick");

	return cmd_loadgame(rEngine, params);
}

int Game::cmd_quicksave(Engine& rEngine, VariableArray& rParams)
{
	VariableArray params;

	params.resize(1);
	params[0].SetStringValue(L"quick");

	return cmd_savegame(rEngine, params);
}

int Game::cmd_control(Engine& rEngine, VariableArray& rParams)
{
	if (rParams.empty() == true)
	{
		rEngine.PrintError(L"invalid syntax: enum controls controlname, "
			L"[string virtkey] expected.");

		return FALSE;
	}

	if (rParams[0].GetVarType() != Variable::TYPE_STRING &&
		rParams[0].GetVarType() != Variable::TYPE_ENUM)
	{
		rEngine.PrintError(L"invalid param type (1): expected string or enum.");

		return FALSE;
	}

	// Determine control index

	int nControl = rParams[0].GetEnumValue(SZ_CONTROLS, CONTROL_COUNT);

	if (nControl == -1)
	{
		rEngine.PrintError(L"invalid control specified, "
			L"see help for controls.");

		return FALSE;
	}

	ControlManager& rControls =
		rEngine.GetClientInstance()->GetControls();

	if (rParams.size() == 1)
	{
		// Display key currently binded to control

		rEngine.PrintInfo(rControls.GetKeyDescription(
			rControls.GetControlBoundKey(nControl)));
	}
	else if (rParams.size() == 2)
	{
		// Bind key to control

		if (rParams[1].GetVarType() != Variable::TYPE_STRING)
		{
			rEngine.PrintError(L"invalid param type (2): expected string.");

			return FALSE;
		}

		int nKey = rControls.GetKeyCode(rParams[1].GetStringValue());

		if (INVALID_INDEX == nKey)
		{
			rEngine.PrintError(L"invalid key specified, see help "
				L"for possible key names.");

			return FALSE;
		}

		rControls.Bind(nControl, nKey);
	}

	return TRUE;
}

void Game::PrintFlags(DWORD dwFlags,
					  const LPCWSTR* pszarFlags,
					  const DWORD* dwarFlags,
					  int nFlagsCount,
					  String& strFlagsOut,
					  LPCWSTR pszSepBefore,
					  LPCWSTR pszSepAfter)
{
	strFlagsOut.Empty();

	// Write flags into a string.

	if (0 == dwFlags && 0 == dwarFlags[0])
	{
		// If the first item in flags array is 0 and
		// no flags set, use that item

		strFlagsOut = pszarFlags[0];

		return;
	}

	while(dwFlags != 0)
	{
		int n = 0;

		for(; n < nFlagsCount; n++)
		{
			if (dwFlags & dwarFlags[n])
			{
				dwFlags &= ~dwarFlags[n];

				if (*pszSepBefore != '\0')
					strFlagsOut += pszSepBefore;

				strFlagsOut += pszarFlags[n];

				if (dwFlags != 0 && *pszSepAfter != '\0')
					strFlagsOut += pszSepAfter;

				break;
			}
		}

		if (n == nFlagsCount)
		{
			break;
		}
	}
}

void Game::PrintLastError(Engine& rEngine)
{
	// Print out error stack

	String str;
	String strLine;

	WCHAR szSpaces[128] = {0};

	while(rEngine.GetErrors().GetCount())
	{
		// Get error description

		str = rEngine.GetErrors().GetLastError().GetDescription();
		rEngine.GetErrors().Pop();

		if (NULL == *szSpaces)
		{
			// Print it, if the first stack level

			rEngine.PrintError(str);
		}
		else
		{
			// Break it into lines, then print every line
			// prefixed by spaces according to stack level

			LPCWSTR pszStart = str.GetBufferConst();
			LPCWSTR pszEnd = wcschr(pszStart, L'\n');

			for(;;)
			{
				if (NULL == pszEnd)
					pszEnd = pszStart + wcslen(pszStart);
				else
					pszEnd--;

				strLine.Allocate(int(pszEnd - pszStart + 1));

				strLine.CopyToBuffer(int(pszEnd - pszStart + 1),
					pszStart, int(pszEnd - pszStart));

				rEngine.Print(szSpaces, PRINT_ERROR, false);
				rEngine.Print(strLine, PRINT_ERROR);

				if (L'\0' == *pszEnd) break;

				pszStart = pszEnd + 2;
				pszEnd = wcschr(pszStart, L'\n');
			}
		}

		// Increase number of spaces after each stack level

		wcscat_s(szSpaces, 128, L"   ");
	}
}