/*------------------------------------------------------------------*\
|
| ThunderEngine.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine class(es)
| Created: 04/08/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_ENGINE_H
#define THUNDER_ENGINE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderGlobals.h"		// using INVALID_VALUE
#include "ThunderLogFile.h"		// using PRINT_TYPES (using StreamCache)
#include "ThunderGraphics.h"	// using Graphics
#include "ThunderClass.h"		// using ClassManager
#include "ThunderTexture.h"		// using Texture for templates
#include "ThunderMaterial.h"	// using Material for templates
#include "ThunderRegion.h"		// using RegionSet for templates
#include "ThunderAnimation.h"	// using Animation for templates
#include "ThunderSprite.h"		// using Sprite for templates
#include "ThunderMusic.h"		// using Music for templates
#include "ThunderVideo.h"		// using Video for templates
#include "ThunderFont.h"		// using Font for templates
#include "ThunderStringTable.h"	// using StringTable for templates
#include "ThunderTileMap.h"		// using TileMapManager
#include "ThunderAudio.h"		// using Audio
#include "ThunderScreen.h"		// using ScreenManager
#include "ThunderTimer.h"		// using TimerManager
#include "ThunderCommand.h"		// using CommandManager
#include "ThunderVariable.h"	// using VariableManager
#include "ThunderError.h"		// using ErrorManager

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Client;					// referencing Client
class Timer;					// referencing Timer


/*----------------------------------------------------------*\
| Engine class
\*----------------------------------------------------------*/

class Engine
{
public:
	//
	// Constants
	//

	enum Options
	{
		// Initialization

		// Manage COM with CoInitialize and CoUninitialize? (pre-initialization)
		OPTION_MANAGE_COM,

		// Destroy game window when being destroyed? (pre-initialization)
		OPTION_MANAGE_GAME_WINDOW,

		// Graphics

		// Tile size used
		OPTION_TILE_SIZE,

		// Render map?
		OPTION_RENDER_MAP,

		// Render screens?
		OPTION_RENDER_SCREENS,

		// Render custom cursor instead of hardware cursor?
		OPTION_CUSTOM_CURSOR,

		// Display hardware or custom cursor?
		OPTION_SHOW_CURSOR,

		// Use wireframe material for rendering (Graphics::SetWireframeMaterial)?
		OPTION_WIREFRAME,

		// Max number of primitives batched
		OPTION_MAX_BATCH_PRIM,

		// Effect compiler flags
		OPTION_EFFECT_COMPILE_FLAGS,

		// Audio

		// Disable loading and playing of sounds? (pre-initialization)
		OPTION_DISABLE_SOUNDS,

		// Disable loading and playing of music? (pre-initialization)
		OPTION_DISABLE_MUSIC,

		// Set DirectSound cooperative level to DSSCL_EXCLUSIVE on creation (pre-initialization)
		OPTION_EXCLUSIVE_SOUND,

		// Audio destination: speakers (0) or headphones (1)
		OPTION_AUDIO_DESTINATION,

		// How frequently we check if a sound instance should be removed because it stopped playing?
		OPTION_SOUND_CACHE_FREQUENCY,

		// Resources

		// Enable resource caching for released resources?
		OPTION_ENABLE_RESOURCE_CACHE,

		// Persistence time for the next resource (after release).
		// Important: set this to 0 for "dynamic" resources which are always re-allocated on reset to avoid memory leaks
		OPTION_RESOURCE_CACHE_DURATION,

		// How frequently we check if resources have to be evicted from cache
		OPTION_RESOURCE_CACHE_FREQUENCY,

		// Enable stream caching for select reading operations?
		OPTION_ENABLE_STREAM_CACHE,

		// Time before a cached stream is evicted (reset on every request of that stream)
		OPTION_STREAM_CACHE_DURATION,

		// How frequently we check if streams have to be evicted form cache
		OPTION_STREAM_CACHE_FREQUENCY,

		// Networking

		// Initialize the networking components to start/connect to server (pre-initialization)
		OPTION_ENABLE_NETWORKING,

		// Port number to use when hosting session/connecting to a session
		OPTION_NET_PORT,

		// Events

		// Send input/session events to game?
		OPTION_GAME_EVENTS,

		// Send input events to screens?
		OPTION_SCREEN_EVENTS,

		// Send input events to map?
		OPTION_MAP_EVENTS,

		// Enable progress notification (when loading/saving map or session)	
		OPTION_PROGRESS_EVENTS,

		// Diagnostics

		// Enable command echo? Print will be called for each line.
		OPTION_ENABLE_ECHO,

		// Number of defined engine options
		OPTION_COUNT
	};

	// Engine version
	static const BYTE VERSION[4];

	// Session file signature & format version
	static const BYTE SAV_SIGNATURE[4];
	static const BYTE SAV_FORMAT_VERSION[4];

	// Default tile size
	static const int DEFAULT_TILE_SIZE;

	// Time epsilon (smallest recognizable time value)
	static const float TIME_EPSILON;

	// Default stream persistence time in cache (2 minutes)
	static const float DEFAULT_STREAMCACHEDURATION;
	static const int DEFAULT_STREAMCACHEFREQUENCY;
	static const int DEFAULT_RESOURCECACHEFREQUENCY;
	static const int DEFAULT_SOUNDCACHEFREQUENCY;

	// If game window is sub classed, this prop is set to engine pointer
	static const WCHAR SZ_WNDPROP[];

	// If game window is managed by the engine, this is the class name
	static const WCHAR SZ_GAMEWNDCLASS[];

protected:
	//
	// Members
	//

	//
	// Client Instance
	//

	// Client instance for receiving notifications (optional)
	Client* m_pClientInstance;

	//
	// Client Window
	//

	// Rendering and event handling window (can be created automatically)
	HWND m_hGameWindow;

	// If this window was created manually and then subclassed, old window procedure is saved here
	WNDPROC m_nOldGameWindowProc;

	//
	// Session
	//

	// Is session started?
	bool m_bSessionStarted;

	// Is session time update paused?
	bool m_bSessionPaused;

	//
	// Options
	//

	// Engine options
	int m_nOptions[OPTION_COUNT];

	//
	// Time
	//

	// Current performance counter value (64 bit)
	INT64 m_qwCurTick;

	// Last counter value (64 bit)
	INT64 m_qwLastTick;

	// Performance counter's frequency converted from 64 bit into float
	float m_fFreq;

	// ( curtick - lasttick ) / freq
	float m_fFrameTime;

	// Session time elapsed since session start, preserved during saves/loads, has multiplier applied to it
	float m_fTime;

	// Time elapsed since the engine was initialized
	float m_fRunTime;

	// Time multiplier, can be used to slow down or speed up session time
	float m_fTimeMultiplier;

	// Accumulates time until resource cache needs to be checked
	float m_fResourceCacheAccum;

	// Accumulates time until stream cache needs to be checked
	float m_fStreamCacheAccum;

	// Accumulates time until sound instances need to be checked
	float m_fSoundCacheAccum;

	//
	// Video
	//

	// Currently playing video (only one at a time is allowed)
	Video* m_pVideo;

	//
	// Graphics
	//

	// Graphics system
	Graphics m_Graphics;

	//
	// Resources
	//

	// One manager for every type
	ResourceManager<Texture> m_Textures;
	ResourceManager<TextureCube> m_CubeTextures;
	ResourceManager<RegionSet> m_Regions;
	ResourceManager<Animation> m_Animations;
	ResourceManager<Material> m_Materials;
	ResourceManager<Effect> m_Effects;
	ResourceManager<Sprite> m_Sprites;
	SoundManager m_Sounds;
	MusicManager m_Music;
	ResourceManager<Video> m_Videos;
	ResourceManager<Font> m_Fonts;
	ResourceManager<StringTable> m_Strings;

	// Resource garbage collection
	ResourceCache m_ResourceCache;

	//
	// Maps
	//

	// Loaded maps
	TileMapManager m_Maps;

	// Current map that is rendered and receives input events
	TileMap* m_pMap;

	//
	// GUI
	//

	// Background color used when there is no current map, otherwise map background color is used
	Color m_clrBackColor;

	// Custom Cursor

	// Current on-screen position of the mouse cursor (updated by EngineWndProc)
	Vector2 m_vecCursorPos;

	// Custom cursor material instance (optionally animated)
	MaterialInstance m_CustomCursor;

	// Screens

	// Top-level screens
	ScreenManager m_Screens;

	// Timers

	// Each screen can have one or more timers sending timing events to it, at a set interval
	TimerManager m_Timers;

	//
	// Sound
	//

	// Audio system
	Audio m_Audio;

	//
	// Scripting
	//

	// Classes for internally scripted objects. Enables client sub-classing of certain objects
	ClassManager m_Classes;

	// Script commands registered for external scripting
	CommandManager m_Commands;

	// Global script variables for external scripting
	VariableManager m_Variables;

	//
	// Stream Cache
	//

	// Stream cache for storing frequently accessed file data
	StreamCache m_StreamCache;

	//
	// Base Save Path and Extension
	//

	WCHAR m_szBaseSavePath[128];
	WCHAR m_szBaseSaveExt[8];

	//
	// Diagnostics
	//

	// Trail of errors that could help in debugging
	mutable ErrorManager m_ErrorStack;
	
public:
	Engine(void);

	~Engine(void)
	{
		Empty();
	}

public:
	//
	// Initialization
	//

	void Initialize(HWND hGameWindow,
		bool bSubclassWindow = true,
		bool bFullScreen = false,
		int nResolutionWidth = 640,
		int nResolutionHeight = 480,
		Graphics::DeviceFormats nFormat = Graphics::FORMAT_DESKTOP,
		D3DMULTISAMPLE_TYPE nMultiSampleType = D3DMULTISAMPLE_NONE,
		DWORD dwMultiSampleQuality = 0,
		DWORD dwBufferRefresh = DEFAULT_VALUE,
		bool bVSync = false,
		bool bHardwareAcceleration = true,
		bool bSoftwareVertexProcessing = false,
		bool bPureDevice = true,
		bool bStartSession = true);

	void Initialize(LPCWSTR pszWindowTitle,
		DWORD dwWindowIconResourceID = DEFAULT_VALUE,
		bool bFullScreen = false,
		int nResolutionWidth = 640,
		int nResolutionHeight = 480,
		Graphics::DeviceFormats nFormat = Graphics::FORMAT_DESKTOP,
		D3DMULTISAMPLE_TYPE nMultiSampleType = D3DMULTISAMPLE_NONE,
		DWORD dwMultiSampleQuality = 0,
		DWORD dwBufferRefresh = DEFAULT_VALUE,
		bool bVSync = false,
		bool bHardwareAcceleration = true,
		bool bSoftwareVertexProcessing = false,
		bool bPureDevice = true,
		bool bStartSession = true);

	//
	// Client Instance
	//

	inline Client* GetClientInstance(void) const
	{
		return m_pClientInstance;
	}

	inline void SetClientInstance(Client* pClientInstance)
	{
		m_pClientInstance = pClientInstance;
	}

	void GetBaseFilePath(LPCWSTR pszName,
		LPCWSTR pszDefaultSubDir,
		LPCWSTR pszDefaultExtensionWithDot,
		LPWSTR pszOutBaseFilePath) const;

	inline LPCWSTR GetBaseSavePath(void) const
	{
		return m_szBaseSavePath;
	}

	inline void SetBaseSavePath(LPCWSTR pszBaseSavePath)
	{
		wcscpy_s(m_szBaseSavePath, sizeof(m_szBaseSavePath) / sizeof(WCHAR),
			pszBaseSavePath);
	}

	inline LPCWSTR GetBaseSaveExtension(void) const
	{
		return m_szBaseSaveExt;
	}

	inline void SetBaseSaveExtension(LPCWSTR pszBaseSaveExt)
	{
		wcscpy_s(m_szBaseSaveExt, sizeof(m_szBaseSaveExt) / sizeof(WCHAR),
			pszBaseSaveExt);
	}

	//
	// Client Window
	//

	inline HWND GetGameWindow(void) const
	{
		return m_hGameWindow;
	}

	void SetGameWindow(HWND hWnd, bool bSubclass = true);

	void SubclassGameWindow(void);
	void UnsubclassGameWindow(void);

	inline bool IsGameWindowSubclassed(void) const
	{
		return (m_nOldGameWindowProc != 0);
	}

	static LRESULT CALLBACK EngineWndProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);

	//
	// Session
	//

	void StartSession(void);

	inline bool IsSessionStarted(void) const
	{
		return m_bSessionStarted;
	}

	void JoinSession(LPCWSTR pszHostName);

	void SerializeSession(LPCWSTR pszPath) const;
	void SerializeSession(Stream& rStream) const;

	void DeserializeSession(LPCWSTR pszPath);
	void DeserializeSession(Stream& rStream);

	void PauseSession(bool bPause);

	inline bool IsSessionPaused(void) const
	{
		return m_bSessionPaused;
	}

	void EndSession(void);

	//
	// Options
	//

	inline int GetOption(Options nIndex) const
	{
		return m_nOptions[nIndex];
	}

	inline DWORD GetOptionEx(Options nIndex) const
	{
		return *(reinterpret_cast<const DWORD*>(&m_nOptions[nIndex]));
	}

	void SetOption(Options nIndex, int nValue);

	inline void SetOptionEx(Options nIndex, DWORD dwValue)
	{
		SetOption(nIndex, *(reinterpret_cast<int*>(&dwValue)));
	}

	void ResetOptions(void);

	//
	// Time
	//

	// Update engine time and perform time-related tasks

	void Update(void);

	// Get time passed since session start

	inline float GetTime(void) const
	{
		return m_fTime;
	}

	// Get time passed since engine was initialized

	inline float GetRunTime(void) const
	{
		return m_fRunTime;
	}

	// Get time it took to render last frame, in seconds

	float GetFrameTime(void) const
	{
		return m_fFrameTime;
	}

	// Get multiplier applied to session time (time since session start)

	inline float GetTimeMultiplier(void) const
	{
		return m_fTimeMultiplier;
	}

	inline void SetTimeMultiplier(float fTimeMultiplier)
	{
		m_fTimeMultiplier = fTimeMultiplier;
	}

	// Get time passed since session start to specified time, in seconds

	inline float GetTimeDelta(float fStart) const
	{
		return (fStart > m_fTime) ?
			fStart - m_fTime : m_fTime - fStart;
	}

	// Get time passed since engine was started to specified time, in seconds

	inline float GetRunTimeDelta(float fStart) const
	{
		return (fStart > m_fRunTime) ?
			fStart - m_fRunTime : m_fRunTime - fStart;
	}

	//
	// Graphics
	//

	inline Graphics& GetGraphics(void)
	{
		return m_Graphics;
	}

	void Render(void);
	void RenderCustomCursor(void);

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);

	//
	// Video
	//

	Video* GetCurrentVideo(void)
	{
		return m_pVideo;
	}

	void SetCurrentVideo(Video* pVideo);

	//
	// Resources
	//

	inline ResourceManager<Texture>& GetTextures(void)
	{
		return m_Textures;
	}

	inline ResourceManager<TextureCube>& GetCubeTextures(void)
	{
		return m_CubeTextures;
	}

	inline ResourceManager<RegionSet>& GetRegions(void)
	{
		return m_Regions;
	}

	inline ResourceManager<Animation>& GetAnimations(void)
	{
		return m_Animations;
	}

	inline ResourceManager<Material>& GetMaterials(void)
	{
		return m_Materials;
	}

	inline ResourceManager<Effect>& GetEffects(void)
	{
		return m_Effects;
	}

	inline ResourceManager<Sprite>& GetSprites(void)
	{
		return m_Sprites;
	}

	inline SoundManager& GetSounds(void)
	{
		return m_Sounds;
	}

	inline MusicManager& GetMusic(void)
	{
		return m_Music;
	}

	inline ResourceManager<Video>& GetVideos(void)
	{
		return m_Videos;
	}

	inline ResourceManager<Font>& GetFonts(void)
	{
		return m_Fonts;
	}

	inline ResourceManager<StringTable>& GetStrings(void)
	{
		return m_Strings;
	}

	inline ResourceCache& GetResourceCache(void)
	{
		return m_ResourceCache;
	}

	//
	// Maps
	//

	inline TileMapManager& GetMaps(void)
	{
		return m_Maps;
	}

	inline void SetCurrentMap(TileMap* pMap)
	{
		m_pMap = pMap;
	}

	inline TileMap* GetCurrentMap(void)
	{
		return m_pMap;
	}

	inline const TileMap* GetCurrentMapConst(void) const
	{
		return m_pMap;
	}
	
	//
	// GUI
	//

	// Back Color

	inline Color& GetBackColor(void)
	{
		return m_clrBackColor;
	}

	inline const Color& GetBackColorConst(void) const
	{
		return m_clrBackColor;
	}

	inline void SetBackColor(D3DCOLOR clrBackColor)
	{
		m_clrBackColor = clrBackColor;
	}

	// Custom cursor

	inline MaterialInstance& GetCustomCursor(void)
	{
		return m_CustomCursor;
	}

	inline void SetCustomCursor(MaterialInstance& rCustomCursor)
	{
		m_CustomCursor = rCustomCursor;
	}

	inline const Vector2& GetCursorPosition(void) const
	{
		return m_vecCursorPos;
	}

	void SetCursorPosition(const Vector2& vecCursorPosition);
	void ResetCursorPosition(void);

	// Screens

	inline ScreenManager& GetScreens(void)
	{
		return m_Screens;
	}

	inline const ScreenManager& GetScreensConst(void) const
	{
		return m_Screens;
	}

	// Timers

	inline TimerManager& GetTimers(void)
	{
		return m_Timers;
	}

	inline const TimerManager& GetTimersConst(void) const
	{
		return m_Timers;
	}

	//
	// Audio
	//

	inline Audio& GetAudio(void)
	{
		return m_Audio;
	}

	//
	// Scripting
	//

	// Class Keys

	inline ClassManager& GetClasses(void)
	{
		return m_Classes;
	}

	inline const ClassManager& GetClassesConst(void) const
	{
		return m_Classes;
	}

	// Variables

	inline VariableManager& GetVariables(void)
	{
		return m_Variables;
	}

	inline const VariableManager& GetVariablesConst(void) const
	{
		return m_Variables;
	}

	// Commands

	inline CommandManager& GetCommands(void)
	{
		return m_Commands;
	}

	inline const CommandManager& GetCommandsConst(void) const
	{
		return m_Commands;
	}

	//
	// Stream Cache
	//

	inline StreamCache& GetStreamCache(void)
	{
		return m_StreamCache;
	}

	inline const StreamCache& GetStreamCacheConst(void) const
	{
		return m_StreamCache;
	}
	
	//
	// Version
	//

	static DWORD GetVersion(void);

	//
	// Error handling
	//

	inline ErrorManager& GetErrors(void) const
	{
		return m_ErrorStack;
	}

	//
	// Output
	//

	void Print(LPCWSTR pszText = NULL,
			   PrintTypes nType = PRINT_MESSAGE,
			   bool bLine = true);

	void PrintMessage(LPCWSTR pszText, ...);
	void PrintError(LPCWSTR pszText, ...);
	void PrintWarning(LPCWSTR pszText, ...);
	void PrintInfo(LPCWSTR pszText, ...);
	void PrintDebug(LPCWSTR pszText, ...);
	void PrintBlank(void);
	void PrintClear(void);

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_ENGINE_H