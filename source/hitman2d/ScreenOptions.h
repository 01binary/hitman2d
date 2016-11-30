/*------------------------------------------------------------------*\
|
| ScreenOptions.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Options Screen class
| Created: 04/21/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_OPTIONS_H
#define SCREEN_OPTIONS_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenOverlapped.h"	// using ScreenOverlapped

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenOptions class
\*----------------------------------------------------------*/

class ScreenOptions: public ScreenOverlapped
{
public:
	//
	// Structures
	//

	// Holds info on keyboard map graphics

	struct KeyGraphic		
	{
		// Virtual key represented by the graphic
		int nVirtKey;

		// Position on keyboard map
		Rect rcDestCoords;

		// Texture coordinates of graphic on atlas
		Rect rcSrcCoords;
	};

	// Holds control settings before they are propagated

	struct ControlSetting
	{
		// Name of this control or header
		String strName;

		// Index within controls list box
		int nListID;

		// ControlID of this setting, INVALID_INDEX if header
		int nControlID;

		// Virtual key set, to be propagated on Apply or OK.
		int nVirtKey;
	};

	// Holds extra information about format support

	struct FormatInfo
	{
		Graphics::DeviceFormats nFormat;
		bool bWindowed;
		bool bFullScreen;
	};

	// Holds extra information about resolution

	struct ResolutionInfo
	{
		DWORD dwWidth;
		DWORD dwHeight;
	};

	//
	// Definitions
	//

	typedef std::vector<ScreenOptions::ControlSetting>
		ControlSettingArray;
	typedef std::vector<ScreenOptions::ControlSetting>::iterator
		ControlSettingArrayIterator;
	typedef std::vector<ScreenOptions::ControlSetting>::const_iterator
		ControlSettingArrayConstIterator;

	typedef std::map<int, ScreenOptions::ControlSetting*>
		ControlSettingMap;
	typedef std::map<int, ScreenOptions::ControlSetting*>::iterator
		ControlSettingMapIterator;
	typedef std::map<int, ScreenOptions::ControlSetting*>::const_iterator
		ControlSettingMapConstIterator;

	typedef std::vector<ScreenOptions::KeyGraphic>
		KeyGraphicArray;
	typedef std::vector<ScreenOptions::KeyGraphic>::iterator
		KeyGraphicArrayIterator;
	typedef std::vector<ScreenOptions::KeyGraphic>::const_iterator
		KeyGraphicArrayConstIterator;

	typedef std::map<int, ScreenOptions::KeyGraphic*>
		KeyGraphicMap;
	typedef std::map<int, ScreenOptions::KeyGraphic*>::iterator
		KeyGraphicMapIterator;
	typedef std::map<int, ScreenOptions::KeyGraphic*>::const_iterator
		KeyGraphicMapConstIterator;

	typedef std::vector<FormatInfo> FormatInfoArray;
	typedef std::vector<FormatInfo>::iterator FormatInfoArrayIterator;

	typedef std::vector<ResolutionInfo> ResolutionInfoArray;
	typedef std::vector<ResolutionInfo>::iterator ResolutionInfoArrayIterator;

	typedef std::vector<DWORD> DwordArray;
	typedef std::vector<DWORD>::iterator DwordArrayIterator;

	//
	// Constants
	//

	// Class Name

	static const WCHAR SZ_CLASS[];

	// Element Names

	static const WCHAR SZ_KEYCOLOR_SELECTION[];
	static const WCHAR SZ_KEYCOLOR_PREVIEW[];
	static const WCHAR SZ_KEYCOLOR_OVERVIEW[];

	static const WCHAR SZ_KEYBOARDMAP_ELEM[];
	static const WCHAR SZ_KEY_ELEM[];
	static const WCHAR SZ_KEYBOARDMAP_IMAGE_ELEM[];
	static const WCHAR SZ_LISTHEADER_STYLE_ELEM[];

	static const WCHAR SZ_STYLE_WIDESCREEN[];

	// Strings

	static const LPCWSTR SZ_FORMAT_DESCRIPTIONS[];
	static const WCHAR SZ_FORMAT_VOLUME[];

	// Control IDs

	enum
	{
		ID = 1004,

		ID_CBO_RES = 1004,
		ID_SPIN_RES = 1104,
		ID_CHECK_FS = 1005,
		ID_CBO_FMT = 1007,
		ID_SPIN_FMT = 1107,
		ID_CBO_AA = 1009,
		ID_SPIN_AA = 1109,
		ID_CBO_REF = 1013,
		ID_SPIN_REF = 1113,
		ID_CHECK_VSYNC = 1014,
		ID_CBO_AUDIODEST = 1016,
		ID_STATIC_MASTER_PERC = 1018,
		ID_SLIDER_MASTER = 1019,
		ID_CHECK_MUTE = 1020,
		ID_STATIC_EFFECT_PERC = 1022,
		ID_SLIDER_EFFECT = 1023,
		ID_STATIC_SPEECH_PERC = 1025,
		ID_SLIDER_SPEECH = 1026,
		ID_STATIC_MUSIC_PERC = 1028,
		ID_SLIDER_MUSIC = 1029,

		ID_CONTROLS = 1031,

		ID_TIP = 4000,

		ID_TAB_PAGES = 101,
		ID_TAB_GRAPHICS = 201,
		ID_TAB_AUDIO = 202,
		ID_TAB_CONTROLS = 203,
		ID_TAB_GAME = 204,

		ID_OK = 102,
		ID_APPLY = 103
	};

	// Timers

	enum
	{
		TIMER_ID = 500
	};

private:
	//
	// Members
	//

	// Cached setting values before propagation (on OK or Apply)

	// Full Screen setting
	bool m_bFullScreen;

	// VSync setting
	bool m_bVSync;

	// Resolution Width setting
	DWORD m_dwResWidth;

	// Resolution Height setting
	DWORD m_dwResHeight;

	// Format setting
	Graphics::DeviceFormats m_nFormat;

	// Anti-Alias Level setting
	D3DMULTISAMPLE_TYPE m_nAALevel;

	// Refresh Rate setting
	DWORD m_dwRefresh;

	// Audio Destination setting
	Audio::Destinations m_nAudioDest;

	// Effects Volume setting
	float m_fEffectsVolume;

	// Speech Volume setting
	float m_fSpeechVolume;

	// Music Volume setting
	float m_fMusicVolume;

	// Master Volume setting
	float m_fMasterVolume;

	// Master Mute setting
	bool m_bMasterMute;

	// Misc State Variables

	// Cached game instance
	Game* m_pGame;

	// Image set/positioned to selected key
	Screen* m_pSelImage;

	// Image set/positioned to preview key
	Screen* m_pPreviewImage;

	// Image set/positioned for overview keys
	Screen* m_pOverviewParent;

	// Keyboard image
	Screen* m_pImageMap;

	// Cached pointer to controls tab
	Screen* m_pControlsTab;

	// Cached pointer to list of controls
	ScreenListBox* m_pControlsList;

	// Cached pointer to controls tip balloon
	Screen* m_pControlsTip;

	// Blend color of selected key on keyboard map
	Color m_clrSelBlend;

	// Blend color of preview key on keyboard map
	Color m_clrPreviewBlend;

	// Blend color of overview keyboard map
	Color m_clrOverviewBlend;

	// VK code of preview key on keyboard map (with mouse hover)
	int m_nSelKeyCode;

	// Key overview currently visible?
	bool m_bOverviewVisible;

	// Fading in key overview (if fade timer is enabled)?
	bool m_bOverviewFadeIn;

	// Map of key graphics by virt key (contains pointers to items in vector below)
	KeyGraphicMap m_mapKeyGraphics;

	// Key graphics by index
	KeyGraphicArray m_arKeyGraphics;

	// Control settings set by user, in effect on OK or Apply
	ControlSettingArray m_arSettings;

	// Control settings IDs mapped by virtual key code for fast lookup
	ControlSettingMap m_mapSettings;

	// Extra information for formats added to combo box
	FormatInfoArray m_arFormats;

	// Extra information for resolutions added to combo box
	ResolutionInfoArray m_arRes;

	// Extra information for refresh rates
	DwordArray m_arRefresh;

	// Extra info for alias levels
	IntArray m_arAntiAlias;

public:
	ScreenOptions(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenOptions(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Events
	//

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseMove(POINT pt);

	virtual int OnNotify(int nNotifyID, Screen* pSender = NULL, int nParam = 0);
	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);

	virtual void OnTimer(Timer& rTimer);

private:
	//
	// Private Functions
	//

	inline static bool IsValidControlKey(int nKeyCode)
	{
		return (nKeyCode != VK_LMENU &&
			   nKeyCode != VK_RMENU &&
			   nKeyCode != VK_CAPITAL &&
			   nKeyCode != VK_NUMLOCK &&
			   nKeyCode != VK_SCROLL &&
			   nKeyCode != VK_LWIN &&
			   nKeyCode != VK_RWIN &&
			   nKeyCode != VK_SNAPSHOT); 
	}

	void LoadKeyboardMap(const InfoElem& rRoot);
	void LoadControlsList(void);
	void LoadFormats(void);
	void LoadResolutions(void);
	void LoadRefreshRates(void);

	void UpdateControlKey(ScreenOptions::ControlSetting& rSetting, int nNewVirtKey);
	void UpdateControlItemText(ScreenButtonEx* pItem,
		const ScreenOptions::ControlSetting& rSetting);

	void UpdateSelKeyImage(void);
	void UpdateKeyOverview(void);
};

} // namespace Hitman2D

#endif // SCREEN_OPTIONS_H