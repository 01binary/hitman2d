/*------------------------------------------------------------------*\
|
| ScreenOptions.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScreenOptions implementation
| Created: 04/21/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "ScreenImage.h"		// using ScreenImage
#include "ScreenListBox.h"		// using ScreenListBox
#include "ScreenComboBox.h"		// using ScreenComboBox
#include "ScreenTabControl.h"	// using ScreenTabControl
#include "ScreenOptions.h"		// defining ScreenOptions

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenOptions::SZ_CLASS[]						= L"overlapped::options";
const WCHAR ScreenOptions::SZ_KEYCOLOR_SELECTION[]			= L"keycolor.selection";
const WCHAR ScreenOptions::SZ_KEYCOLOR_PREVIEW[]			= L"keycolor.preview";
const WCHAR ScreenOptions::SZ_KEYCOLOR_OVERVIEW[]			= L"keycolor.overview";
const WCHAR ScreenOptions::SZ_KEYBOARDMAP_ELEM[]			= L"keyboardmap";
const WCHAR ScreenOptions::SZ_KEY_ELEM[]					= L"key";
const WCHAR ScreenOptions::SZ_KEYBOARDMAP_IMAGE_ELEM[]		= L"keyboardmap_image";
const WCHAR ScreenOptions::SZ_STYLE_WIDESCREEN[]			= L"widescreen";

const LPCWSTR ScreenOptions::SZ_FORMAT_DESCRIPTIONS[]		= {
																  L"Desktop",
																  L"True Color (32 bit)",
																  L"High Color (16 bit)",
																  L"High Color (16 bit+)"
															  };

const WCHAR ScreenOptions::SZ_FORMAT_VOLUME[]				= L"%d%%";


/*----------------------------------------------------------*\
| ScreenOptions implementation
\*----------------------------------------------------------*/

ScreenOptions::ScreenOptions(Engine& rEngine,
							 LPCWSTR pszClass,
							 Screen* pParent):
							 ScreenOverlapped(rEngine, pszClass, pParent),
							 m_bFullScreen(false),
							 m_bVSync(false),
							 m_dwResWidth(0),
							 m_dwResHeight(0),
							 m_nFormat(Graphics::FORMAT_DESKTOP),
							 m_nAALevel(D3DMULTISAMPLE_NONE),
							 m_dwRefresh(0),
							 m_nAudioDest(Audio::DESTINATION_SPEAKERS),
							 m_fEffectsVolume(0.0f),
							 m_fSpeechVolume(0.0f),
							 m_fMusicVolume(0.0f),
							 m_fMasterVolume(0.0f),
							 m_pGame(NULL),
							 m_pSelImage(NULL),
							 m_pPreviewImage(NULL),
							 m_pOverviewParent(NULL),
							 m_pImageMap(NULL),
							 m_pControlsTab(NULL),
							 m_pControlsList(NULL),
							 m_pControlsTip(NULL),
							 m_bOverviewVisible(false),
							 m_bOverviewFadeIn(false)
{
}

ScreenOptions::~ScreenOptions(void)
{
	m_rEngine.SetOption(Engine::OPTION_GAME_EVENTS, TRUE);
}

Object* ScreenOptions::CreateInstance(Engine& rEngine,
					   LPCWSTR pszClass,
					   Object* pParent)
{
	return new ScreenOptions(rEngine, pszClass,
		dynamic_cast<Screen*>(pParent));
}

void ScreenOptions::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	// Get game instance

	m_pGame = dynamic_cast<Game*>(m_rEngine.GetClientInstance());

	if (NULL == m_pGame)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR, __FUNCTIONW__, L"pGame");

	// Load keyboard map graphic and data

	LoadKeyboardMap(rRoot);

	// Load controls into list box and in-memory list

	LoadControlsList();	

	// Set full screen

	m_bFullScreen = m_pGame->GetFullScreen();
	GetControl<ScreenButton>(ID_CHECK_FS).SetToggle(m_bFullScreen);

	// Set vsync

	m_bVSync = m_pGame->GetVSync();
	GetControl<ScreenButton>(ID_CHECK_VSYNC).SetToggle(m_bVSync);

	// Set resolution

	m_dwResWidth = DWORD(m_pGame->GetDisplayWidth());
	m_dwResHeight = DWORD(m_pGame->GetDisplayHeight());

	// Set format

	m_nFormat = m_pGame->GetDeviceFormat();

	// Set anti alias level

	m_nAALevel = m_pGame->GetMultiSampleType();

	// Set refresh

	m_dwRefresh = m_pGame->GetRefreshRate();

	// Set effects volume

	m_fEffectsVolume = m_pGame->GetEffectsVolume();

	int nVolume = int(m_fEffectsVolume * 100.0f);

	String str;
	str.Format(SZ_FORMAT_VOLUME, nVolume);

	GetControl<ScreenScrollBar>(ID_SLIDER_EFFECT).SetValue(nVolume);
	GetControl<ScreenLabel>(ID_STATIC_EFFECT_PERC).SetText(str);

	// Set speech volume

	m_fSpeechVolume = m_pGame->GetSpeechVolume();

	nVolume = int(m_fSpeechVolume * 100.0f);
	str.Format(SZ_FORMAT_VOLUME, nVolume);

	GetControl<ScreenScrollBar>(ID_SLIDER_SPEECH).SetValue(nVolume);
	GetControl<ScreenLabel>(ID_STATIC_SPEECH_PERC).SetText(str);

	// Set music volume

	m_fMusicVolume = m_pGame->GetMusicVolume();

	nVolume = int(m_fMusicVolume * 100.0f);
	str.Format(SZ_FORMAT_VOLUME, nVolume);

	GetControl<ScreenScrollBar>(ID_SLIDER_MUSIC).SetValue(nVolume);
	GetControl<ScreenLabel>(ID_STATIC_MUSIC_PERC).SetText(str);

	// Set master volume (can be -1 when muted)

	m_fMasterVolume = m_rEngine.GetAudio().GetMasterVolume();

	nVolume = m_fMasterVolume > 0.0f ? int(m_fMasterVolume * 100.0f) : 0;
	str.Format(SZ_FORMAT_VOLUME, nVolume);

	GetControl<ScreenScrollBar>(ID_SLIDER_MASTER).SetValue(nVolume);
	GetControl<ScreenLabel>(ID_STATIC_MASTER_PERC).SetText(str);

	// Set master mute

	m_bMasterMute = m_rEngine.GetAudio().GetMasterVolumeMute();
	GetControl<ScreenButton>(ID_CHECK_MUTE).SetToggle(m_bMasterMute);

	// Enable/disable master volume slider based on 

	try
	{
		if (true == m_bMasterMute)
			GetControl<ScreenScrollBar>(ID_SLIDER_MASTER).SetFlag(DISABLED);
		else
			GetControl<ScreenScrollBar>(ID_SLIDER_MASTER).ClearFlag(DISABLED);
	}

	catch(Error) {}

	// Set audio destination

	m_nAudioDest = (Audio::Destinations)m_rEngine.GetOption(Engine::OPTION_AUDIO_DESTINATION);
	GetControl<ScreenComboBox>(ID_CBO_AUDIODEST).SetSelectedItem(m_nAudioDest);

	// Add supported formats for selected device type

	LoadFormats();

	// Cache tip control

	m_pControlsTip = m_lstChildren.FindByID(ID_TIP, true);
}

void ScreenOptions::LoadFormats(void)
{
	// Get format list

	ScreenComboBox& rFormats = GetControl<ScreenComboBox>(ID_CBO_FMT);

	// Clear items and extra data

	rFormats.RemoveAllItems();

	m_arFormats.clear();

	// Get current (desktop) mode

	D3DDISPLAYMODE dmDesktop = {0};
	LPDIRECT3D9 pD3D = m_rEngine.GetGraphics().GetDirect3D();

	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmDesktop);

	// Enumerate all modes

	int nIndex = -1;
	int nDefaultIndex = -1;

	for(DWORD dwDevFormat = 0;
		dwDevFormat < (sizeof(DW_THUDEVICEFORMATS) / sizeof(DWORD));
		dwDevFormat++)
	{
		FormatInfo fi = { (Graphics::DeviceFormats)dwDevFormat, false, false };

		if (0 == dwDevFormat)
		{
			if (SUCCEEDED(pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
				m_pGame->GetHardwareAcceleration() == true ?
					D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				dmDesktop.Format,
				dmDesktop.Format, false)))
				fi.bFullScreen = true;

			if (SUCCEEDED(pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
				m_pGame->GetHardwareAcceleration() == true ?
					D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				dmDesktop.Format,
				dmDesktop.Format, true)))
				fi.bWindowed = true;
		}
		else
		{
			if (SUCCEEDED(pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
				m_pGame->GetHardwareAcceleration() == true ?
					D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat],
				(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat], false)))
				fi.bFullScreen = true;

			if (SUCCEEDED(pD3D->CheckDeviceType(D3DADAPTER_DEFAULT,
				m_pGame->GetHardwareAcceleration() == true ?
					D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				(D3DFORMAT)DW_THUDEVICEFORMATS[dwDevFormat],
				dmDesktop.Format, true)))
				fi.bWindowed = true;
		}

		if (true == fi.bFullScreen || true == fi.bWindowed)
		{
			// Add format and extra data

			nIndex = rFormats.AddItem(SZ_FORMAT_DESCRIPTIONS[dwDevFormat], NULL, false);

			m_arFormats.push_back(fi);

			// See if this should be the default index

			if (m_nFormat > Graphics::FORMAT_DESKTOP)
			{
				if (DWORD(m_nFormat) == dwDevFormat)
					nDefaultIndex = nIndex;
			}
			else
			{
				if (Graphics::FORMAT_DESKTOP == dwDevFormat)
					nDefaultIndex = nIndex;
			}
		}
	}

	if (nIndex != -1)
		rFormats.SetSelectedItem(nDefaultIndex == -1 ? 0 : nDefaultIndex);

	rFormats.Update();

	if (rFormats.GetItemCount() > 1)
	{
		GetControl<ScreenScrollBar>(ID_SPIN_FMT).ClearFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_FMT).SetMax(rFormats.GetItemCount() - 1);
	}
	else
	{
		GetControl<ScreenScrollBar>(ID_SPIN_FMT).SetFlag(DISABLED);
	}

	LoadResolutions();
}

void ScreenOptions::LoadResolutions(void)
{
	// Get Resolutions combo
	
	ScreenComboBox& rResolutions = GetControl<ScreenComboBox>(ID_CBO_RES);

	// Clear resolutions and extra data

	rResolutions.RemoveAllItems();
	m_arRes.clear();

	// Get selected format

	D3DDISPLAYMODE dmDesktop = {0};

	LPDIRECT3D9 pD3D = m_rEngine.GetGraphics().GetDirect3D();
	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmDesktop);

	// List all resolutions with current format

	D3DFORMAT nFormat = (Graphics::FORMAT_DESKTOP == m_nFormat) ?
		dmDesktop.Format : D3DFORMAT(DW_THUDEVICEFORMATS[m_nFormat]);

	DWORD dwModeCount = pD3D->GetAdapterModeCount(
		D3DADAPTER_DEFAULT, nFormat);

	m_arRes.reserve(dwModeCount);

	int nIndex = INVALID_INDEX;
	int nDefaultRes = INVALID_INDEX;
	D3DDISPLAYMODE dm = {0};

	DWORD dwLastWidth = 0;
	DWORD dwLastHeight = 0;

	String str;
	ScreenButtonEx* pItem = NULL;

	// Add adapter mode for desktop

	for(DWORD dw = 0; dw < dwModeCount; dw++)
	{
		pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, nFormat, dw, &dm);

		if (dm.Width != dwLastWidth || dm.Height != dwLastHeight)
		{
			// Adapter modes repeat for each refresh rate, so filter out duplicates

			dwLastWidth = dm.Width;
			dwLastHeight = dm.Height;

			str.Format(L"%dx%d", dm.Width, dm.Height);

			// Indicate desktop resolution

			if (dm.Width == dmDesktop.Width && dm.Height == dmDesktop.Height &&
			   dm.RefreshRate == dmDesktop.RefreshRate)
				str += L"*";

			// Add resolution to list

			nIndex = rResolutions.AddItem(str, NULL, false, true, &pItem);

			// Add wide screen icon if wide screen
			// (wide is considered something other than a 4:3 ratio)

			if (abs(float(dm.Width) / float(dm.Height) - 4.0f / 3.0f) >= 0.001f)
			{
				LoadMaterialInstance(pItem->GetIcon(), SZ_SCREEN_BACKGROUND,
					NULL, m_rEngine.GetScreens().GetTheme().GetStyle(SZ_STYLE_WIDESCREEN), NULL);

				pItem->SetIconAlign(ScreenButtonEx::ICON_ALIGN_RIGHT);
			}

			// Add extra resolution info

			ResolutionInfo ri;
			ri.dwWidth = dm.Width;
			ri.dwHeight = dm.Height;

			m_arRes.push_back(ri);

			// See if this should be default

			if (nDefaultRes == -1 && dm.Height == m_dwResHeight &&
				dm.Width == m_dwResWidth)
				nDefaultRes = nIndex;
		}
	}

	// Update resolutions spinner to correspond to correct item count

	if (rResolutions.GetItemCount() > 1)
	{
		GetControl<ScreenScrollBar>(ID_SPIN_RES).ClearFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_RES).SetMax(rResolutions.GetItemCount() - 1);
	}
	else
	{
		GetControl<ScreenScrollBar>(ID_SPIN_RES).SetFlag(DISABLED);
	}

	// Get format combo

	ScreenComboBox& rFormat = GetControl<ScreenComboBox>(ID_CBO_FMT);
	int nFormatID = rFormat.GetSelectedItem();

	// Update full screen checkbox

	ScreenButton& rFullScreen = GetControl<ScreenButton>(ID_CHECK_FS);

	if (true == m_arFormats[nFormatID].bFullScreen &&
	   false == m_arFormats[nFormatID].bWindowed)
	{
		// Set full screen checkbox and disable it

		rFullScreen.SetToggle(true);
		rFullScreen.SetFlag(DISABLED);
	}
	else if (false == m_arFormats[nFormatID].bFullScreen &&
			true == m_arFormats[nFormatID].bWindowed)
	{
		// Clear full screen checkbox and disable it

		rFullScreen.SetToggle(false);
		rFullScreen.SetFlag(DISABLED);
	}
	else
	{
		// Enable full screen checkbox

		rFullScreen.ClearFlag(DISABLED);
	}

	// Select item with default index

	if (nDefaultRes != -1)
	{
		rResolutions.SetSelectedItem(nDefaultRes);
	}

	rResolutions.Update();
}

void ScreenOptions::LoadRefreshRates(void)
{
	// Clear refresh combo box and extra data array

	ScreenComboBox& rRefresh = GetControl<ScreenComboBox>(ID_CBO_REF);

	rRefresh.RemoveAllItems();
	m_arRefresh.clear();

	// Get current display mode	

	LPDIRECT3D9 pD3D = m_rEngine.GetGraphics().GetDirect3D();
	D3DDISPLAYMODE dmDesktop = {0};

	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmDesktop);

	// List refresh rates for selected format and resolution

	D3DFORMAT nFormat = (Graphics::FORMAT_DESKTOP == m_nFormat) ?
		dmDesktop.Format : D3DFORMAT(DW_THUDEVICEFORMATS[m_nFormat]);

	DWORD dwModeCount = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, nFormat);

	D3DDISPLAYMODE dm = {0};

	String str;

	int nDefaultRefresh = -1;
	int nIndex = -1;

	if (m_bFullScreen == false)
	{
		nIndex = rRefresh.AddItem(L"Desktop", NULL, false);
		m_arRefresh.push_back(0);
	}

	for(DWORD dwMode = 0; dwMode < dwModeCount; dwMode++)
	{
		pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, nFormat, dwMode, &dm);

		if (dm.Width == m_dwResWidth && dm.Height == m_dwResHeight)
		{
			// Add item to list

			str.Format(L"%d Hz", dm.RefreshRate);

			nIndex = rRefresh.AddItem(str, NULL, false);

			// Add extra data to array

			m_arRefresh.push_back(dm.RefreshRate);

			// See if this should be default

			if (dm.RefreshRate == m_dwRefresh)
				nDefaultRefresh = nIndex;
		}
	}

	// Set default refresh

	if (m_bFullScreen == false || INVALID_INDEX == nDefaultRefresh)
		rRefresh.SetSelectedItem(0);
	else
		rRefresh.SetSelectedItem(nDefaultRefresh);

	// If not full screen, disable refresh rates combo and its spinner

	if (true == m_bFullScreen)
	{
		rRefresh.ClearFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_REF).ClearFlag(DISABLED);
	}
	else
	{
		rRefresh.SetFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_REF).SetFlag(DISABLED);
	}

	// Update

	rRefresh.Update();

	if (rRefresh.GetItemCount() > 1 && true == m_bFullScreen)
	{
		GetControl<ScreenScrollBar>(ID_SPIN_REF).ClearFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_REF).SetMax(rRefresh.GetItemCount() - 1);
	}
	else
	{
		GetControl<ScreenScrollBar>(ID_SPIN_REF).SetFlag(DISABLED);
	}

	// Add supported anti-alias levels for resolution...

	// Get anti-alias combo

	ScreenComboBox& rAAList = GetControl<ScreenComboBox>(ID_CBO_AA);

	// Clear anti-alias combo

	rAAList.RemoveAllItems();

	rAAList.AddItem(L"No Anti-Alias");
	m_arAntiAlias.push_back(D3DMULTISAMPLE_NONE);

	WCHAR szItem[64] = {0};

	nIndex = 0;

	for(int nAntiAliasLevel = D3DMULTISAMPLE_2_SAMPLES;
		nAntiAliasLevel < D3DMULTISAMPLE_16_SAMPLES + 1;
		nAntiAliasLevel++)
	{
		if (SUCCEEDED(
			pD3D->CheckDeviceMultiSampleType(
				D3DADAPTER_DEFAULT,
				m_rEngine.GetClientInstance()->GetHardwareAcceleration() == true ?
				D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				nFormat,
				m_bFullScreen == false,
				D3DMULTISAMPLE_TYPE(nAntiAliasLevel),
				NULL)))
		{
			// Add anti-alias level item

			swprintf_s(szItem, L"%dx MSAA", nAntiAliasLevel);

			int nNewItemIndex = rAAList.AddItem(szItem, NULL, false);

			// Add extra info

			m_arAntiAlias.push_back(nAntiAliasLevel);

			// Should be selected item?

			if (m_nAALevel == nAntiAliasLevel)
				nIndex = nNewItemIndex;
		}							
	}

	if (nIndex != INVALID_INDEX)
		rAAList.SetSelectedItem(nIndex);
	else
		rAAList.SetSelectedItem(0);

	rAAList.Update();

	if (rAAList.GetItemCount() > 1)
	{
		GetControl<ScreenScrollBar>(ID_SPIN_AA).ClearFlag(DISABLED);
		GetControl<ScreenScrollBar>(ID_SPIN_AA).SetMax(rAAList.GetItemCount() - 1);
	}
	else
	{
		GetControl<ScreenScrollBar>(ID_SPIN_AA).SetFlag(DISABLED);
	}
}

void ScreenOptions::LoadKeyboardMap(const InfoElem& rRoot)
{
	// Find keyboardmap element within the file	

	const InfoElem* pElemKeys = rRoot.FindChildConst(SZ_KEYBOARDMAP_ELEM,
		InfoElem::TYPE_ANY, InfoElem::TYPE_UNDEFINED, true);

	if (NULL == pElemKeys)
		return;

	// Load destination and source coordinates of key graphics
	// from keyboardmap element

	m_arKeyGraphics.reserve(100);

	InfoElemConstRange range = pElemKeys->FindChildrenConst(SZ_KEY_ELEM);

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		const InfoElem& rKeyElem = *pos->second;

		// Validate

		if (rKeyElem.GetChildCount() < 7)
			continue;

		if (rKeyElem.GetChildConst(0)->GetVarType() != Variable::TYPE_STRING ||
		   rKeyElem.GetChildConst(1)->GetVarType() != Variable::TYPE_INT ||
		   rKeyElem.GetChildConst(2)->GetVarType() != Variable::TYPE_INT ||
		   rKeyElem.GetChildConst(3)->GetVarType() != Variable::TYPE_INT ||
		   rKeyElem.GetChildConst(4)->GetVarType() != Variable::TYPE_INT ||
		   rKeyElem.GetChildConst(5)->GetVarType() != Variable::TYPE_INT ||
		   rKeyElem.GetChildConst(6)->GetVarType() != Variable::TYPE_INT)
			continue;

		// Cache in array and in map

		KeyGraphic graphic;

		graphic.nVirtKey =
			ControlManager::GetKeyCode(rKeyElem.GetChildConst(0)->GetStringValue());

		rKeyElem.ToIntArray(reinterpret_cast<int*>(&graphic.rcDestCoords.left), 2, 1);
		rKeyElem.ToIntArray((int*)&graphic.rcSrcCoords.left, 2, 3);
		rKeyElem.ToIntArray((int*)&graphic.rcSrcCoords.right, 2, 5);

		graphic.rcDestCoords.right += graphic.rcSrcCoords.right +
			graphic.rcDestCoords.left;

		graphic.rcDestCoords.bottom += graphic.rcSrcCoords.bottom +
			graphic.rcDestCoords.top;

		graphic.rcSrcCoords.right += graphic.rcSrcCoords.left;
		graphic.rcSrcCoords.bottom += graphic.rcSrcCoords.top;

		m_arKeyGraphics.push_back(graphic);

		m_mapKeyGraphics.insert(std::pair<int, KeyGraphic*>(
			graphic.nVirtKey, &m_arKeyGraphics.back()));		
	}

	// Load key blend colors from the key map element

	LoadColor(m_clrSelBlend, SZ_KEYCOLOR_SELECTION, pElemKeys);
	LoadColor(m_clrPreviewBlend, SZ_KEYCOLOR_PREVIEW, pElemKeys);
	LoadColor(m_clrOverviewBlend, SZ_KEYCOLOR_OVERVIEW, pElemKeys);

	// Find image map screen

	m_pImageMap = m_lstChildren.FindByName(SZ_KEYBOARDMAP_IMAGE_ELEM, true);

	if (m_pImageMap != NULL)
	{
		// Create overview parent for showing all used keys

		m_pOverviewParent = dynamic_cast<Screen*>(ScreenImage::CreateInstance(
			m_rEngine, ScreenImage::SZ_CLASS, m_pImageMap));

		m_pOverviewParent->SetFlags(INVISIBLE | BACKGROUND);

		m_pImageMap->GetChildren().Add(m_pOverviewParent);

		// Create selection image to show selected key

		m_pSelImage = dynamic_cast<Screen*>(ScreenImage::CreateInstance(
			m_rEngine, ScreenImage::SZ_CLASS, m_pImageMap));

		m_pSelImage->SetFlags(INVISIBLE | BACKGROUND);
		m_pSelImage->GetBackground() = m_pImageMap->GetBackground();
		m_pSelImage->SetBlend(m_clrSelBlend);

		m_pImageMap->GetChildren().Add(m_pSelImage);

		// Create preview image to show hover key

		m_pPreviewImage = dynamic_cast<Screen*>(ScreenImage::CreateInstance(
			m_rEngine, ScreenImage::SZ_CLASS, m_pImageMap));

		m_pPreviewImage->SetFlags(INVISIBLE | BACKGROUND);
		m_pPreviewImage->GetBackground() = m_pImageMap->GetBackground();
		m_pPreviewImage->SetBlend(m_clrPreviewBlend);

		m_pImageMap->GetChildren().Add(m_pPreviewImage);

		// Cache pointer to controls tab

		m_pControlsTab = m_pImageMap->GetParent();
	}
}

void ScreenOptions::LoadControlsList(void)
{
	/*
		Load control settings into separate array and populate listbox.
		We keep control settings made by Options dialog in a separate
		array so they can be propagated on OK and Apply, not immediately.
		Also because we want to have header items for categories which makes
		it impossible to maintain 1 to 1 correspondence between list box items
		and control IDs.
	*/

	m_pControlsList = dynamic_cast<ScreenListBox*>(
		m_lstChildren.FindByID(ID_CONTROLS, true));

	if (NULL == m_pControlsList)
		return;	

	// Reserve array space

	m_arSettings.reserve(64);

	// Parse all setting names and save into control array	

	ControlManager& rControls = m_rEngine.GetClientInstance()->GetControls();

	WCHAR szControlTitle[128] = {0};
	WCHAR szControlCategory[128] = {0};
	WCHAR szLastControlCategory[128] = {0};

	ControlSetting setting;

	wcscpy_s(szControlTitle, sizeof(szControlTitle) / sizeof(WCHAR), L"  ");

	for(int nControlID = 0;
		nControlID < rControls.GetControlCount();
		nControlID++)
	{
		// Get control title and category

		LPCWSTR pszControlName = rControls.GetControlName(nControlID);

		LPCWSTR pszTemp = wcschr(pszControlName, L'.');

		// If not following correct format, ignore

		if (NULL == pszTemp)
			continue;

		// Parse title and category

		wcscpy_s(szControlTitle,
			sizeof(szControlTitle) / sizeof(WCHAR), pszTemp + 1);

		wcsncpy_s(szControlCategory,
			sizeof(szControlTitle) / sizeof(WCHAR),
			pszControlName, pszTemp - pszControlName);

		// Add category to settings array if not added yet

		if (wcscmp(szControlCategory, szLastControlCategory) != 0)
		{
			setting.nListID = int(m_arSettings.size());
			setting.nControlID = INVALID_INDEX;
			setting.nVirtKey = 0;

			m_arSettings.push_back(setting);

			wcscpy_s(szLastControlCategory,
				sizeof(szLastControlCategory) / sizeof(WCHAR), szControlCategory);

			// Add setting header to list box

			ScreenButtonEx* pNewHeader = m_pControlsList->AddItem(false, false);

			pNewHeader->SetText(szControlCategory);
			pNewHeader->SetFlag(Screen::DISABLED);
			pNewHeader->SetStyle(L"compactheader");
		}

		// Setup setting

		setting.nListID = int(m_arSettings.size());
		setting.nControlID = nControlID;
		setting.nVirtKey = rControls.GetControlBoundKey(nControlID);
		setting.strName = szControlTitle;

		// Add to settings array

		m_arSettings.push_back(setting);

		// Register setting under currently used key in map

		m_mapSettings[setting.nVirtKey] = &m_arSettings.back();
	
		// Add setting to list box

		UpdateControlItemText(m_pControlsList->AddItem(false), m_arSettings.back());		
	}

	// Update listbox after adding all the items

	m_pControlsList->Update();
}

void ScreenOptions::UpdateControlKey(ScreenOptions::ControlSetting& rSetting,
									 int nNewVirtKey)
{
	// See if this key is already used for another command

	ControlSettingMapIterator posFind = m_mapSettings.find(nNewVirtKey);

	if (m_mapSettings.end() != posFind)
	{
		// Clear this key from command currently using it		

		posFind->second->nVirtKey = ControlManager::KEY_UNASSIGNED;

		// Update existing list item to indicate unbinding

		UpdateControlItemText(m_pControlsList->GetItem(posFind->second->nListID),
			*posFind->second);
	}

	// Remove entry for previous key from the map

	m_mapSettings.erase(rSetting.nVirtKey);
	
	// Set this key for selected command

	rSetting.nVirtKey = nNewVirtKey;

	// Update the map to indicate this key was binded to this command

	m_mapSettings[nNewVirtKey] = &(rSetting);

	// Update this setting in list box

	UpdateControlItemText(
		m_pControlsList->GetItem(rSetting.nListID), rSetting);

	// Update the selected key image and key overview

	UpdateSelKeyImage();

	UpdateKeyOverview();
}

void ScreenOptions::UpdateControlItemText(ScreenButtonEx* pItem,
										  const ScreenOptions::ControlSetting& rSetting)
{
	static WCHAR szDestBuffer[MAX_PATH] = {0};

	// Build item text - get control name

	if (NULL == pItem)
		return;

	wcscpy_s(szDestBuffer, MAX_PATH, L"  ");

	wcscat_s(szDestBuffer, MAX_PATH, rSetting.strName);

	if (rSetting.nVirtKey != ControlManager::KEY_UNASSIGNED)
	{
		// Space out the control key name

		size_t nSpaces = size_t(float(m_pControlsList->GetSize().cx -
			m_pControlsList->GetMargin() * 2) * 3.0f / 4.3f) /
			pItem->GetFont()->GetAveCharWidth(); 

		size_t nLen = wcslen(szDestBuffer);

		for(size_t n = nLen; n < nSpaces; n++)
			szDestBuffer[n] = L' ';

		szDestBuffer[nSpaces] = L'\0';

		// Get control key name

		wcscat_s(szDestBuffer, MAX_PATH,
			ControlManager::GetKeyDescription(rSetting.nVirtKey));
	}

	// Set item properties

	pItem->SetText(szDestBuffer);
}

void ScreenOptions::OnKeyDown(int nKeyCode)
{
	// Validate

	if (NULL == m_pControlsList)
		return;

	// If the key is valid and a command is selected in the listbox,
	// set command key to key pressed

	ScreenTabControl& rTabControl = GetControl<ScreenTabControl>(ID_TAB_PAGES);

	if (true == IsValidControlKey(nKeyCode) &&
	   m_pControlsList->GetSelectedItem() != INVALID_INDEX &&
	   (rTabControl.GetActiveTab() == 2) &&
	   m_rEngine.GetScreens().GetActiveScreen() == this)
	{
		UpdateControlKey(
			m_arSettings[m_pControlsList->GetSelectedItem()], nKeyCode);
	}
	else
	{
		ScreenOverlapped::OnKeyDown(nKeyCode);
	}
}

void ScreenOptions::OnMouseLDown(POINT pt)
{
	if (NULL == m_pImageMap || NULL == m_pControlsTab)
		return;

	POINT ptAdj = {
		pt.x - (m_pControlsTab->GetParent()->GetPosition().x +
			m_pControlsTab->GetPosition().x +
			m_pImageMap->GetPosition().x),

		pt.y - (m_pControlsTab->GetParent()->GetPosition().y +
			m_pControlsTab->GetPosition().y +
			m_pImageMap->GetPosition().y)
	};

	if (m_pImageMap->GetClientRect().PtInRect(ptAdj) == false)
	{
		ScreenOverlapped::OnMouseLDown(pt);
	}
	else
	{
		// If a command is selected in the listbox, set command key to key selected

		if (m_pControlsList->GetSelectedItem() != INVALID_INDEX)
		{
			UpdateControlKey(
				m_arSettings[m_pControlsList->GetSelectedItem()], m_nSelKeyCode);
		}
	}
}

void ScreenOptions::OnMouseMove(POINT pt)
{
	// Show preview image over the key in keyboard map under mouse

	if (m_pPreviewImage != NULL)
		m_pPreviewImage->SetFlag(INVISIBLE);

	if (m_pControlsTab != NULL &&
	   m_pControlsTab->IsFlagSet(INVISIBLE) == false &&
	   m_pControlsList != NULL &&
	   m_pControlsList->GetSelectedItem() != INVALID_INDEX)
	{
		// Process mouse messages for the keyboard map

		POINT ptAdj = {
			pt.x - (m_pControlsTab->GetParent()->GetPosition().x +
				m_pControlsTab->GetPosition().x +
				m_pImageMap->GetPosition().x),

			pt.y - (m_pControlsTab->GetParent()->GetPosition().y +
				m_pControlsTab->GetPosition().y +
				m_pImageMap->GetPosition().y)
		};

		if (m_pImageMap->GetClientRect().PtInRect(ptAdj) == true)
		{
			// Search by position

			for(KeyGraphicArrayConstIterator pos = m_arKeyGraphics.begin();
				pos != m_arKeyGraphics.end();
				pos++)
			{
				if (pos->rcDestCoords.PtInRect(ptAdj) == true)
				{
					// Set selection image properties

					m_pPreviewImage->SetPosition(pos->rcDestCoords.GetPosition());
					m_pPreviewImage->SetSize(pos->rcDestCoords.GetSize());
					m_pPreviewImage->GetBackground().SetTextureCoords(pos->rcSrcCoords);

					m_pPreviewImage->ClearFlag(INVISIBLE);

					m_nSelKeyCode = pos->nVirtKey;

					break;
				}
			}
		}
	}
	
	ScreenOverlapped::OnMouseMove(pt);
}

int ScreenOptions::OnNotify(int nNotifyID, Screen* pSender, int nParam)
{
 	ScreenOverlapped::OnNotify(nNotifyID, pSender, nParam);

	if (ScreenListBox::NOTIFY_SELECT == nNotifyID)
	{
		switch(pSender->GetID())
		{
		case ID_CONTROLS:
			{
				// Controls list selection changed

				UpdateSelKeyImage();

				UpdateKeyOverview();

				// Disable firing controls when editing a key bind

				m_rEngine.SetOption(Engine::OPTION_GAME_EVENTS,
					m_pControlsList->GetSelectedItem() == INVALID_INDEX);

				// Disable keyboard processing for the list box when editing a key bind

				if (m_pControlsList->GetSelectedItem() == INVALID_INDEX)
					m_pControlsList->ClearFlag(ScreenListBox::DISABLE_KEYBOARD);
				else
					m_pControlsList->SetFlag(ScreenListBox::DISABLE_KEYBOARD);
			}
			break;
		case ID_CBO_RES:
			{
				// Resolution combo selection changed

				int nResID = 
					dynamic_cast<ScreenComboBox*>(pSender)->GetSelectedItem();

				m_dwResWidth = m_arRes[nResID].dwWidth;
				m_dwResHeight = m_arRes[nResID].dwHeight;

				// Update corresponding spinner

				GetControl<ScreenScrollBar>(pSender->GetID() + 100).SetValue(nResID);

				// Reload refresh rates and anti-alias levels

				LoadRefreshRates();
			}
			break;
		case ID_CBO_FMT:
			{
				// Format combo selection changed

				int nFormatID =
					dynamic_cast<ScreenComboBox*>(pSender)->GetSelectedItem();

				m_nFormat = m_arFormats[nFormatID].nFormat;

				// Update corresponding spinner

				GetControl<ScreenScrollBar>(pSender->GetID() + 100).SetValue(nFormatID);

				// Reload resolutions

				LoadResolutions();
			}
			break;
		case ID_CBO_REF:
			{
				// Refresh combo selection changed

				int nRefID =
					dynamic_cast<ScreenComboBox*>(pSender)->GetSelectedItem();

				m_dwRefresh = m_arRefresh[nRefID];

				// Update corresponding spinner

				GetControl<ScreenScrollBar>(pSender->GetID() + 100).SetValue(nRefID);
			}
			break;
		case ID_CBO_AA:
			{
				int nAALevelID =
					dynamic_cast<ScreenComboBox*>(pSender)->GetSelectedItem();

				m_nAALevel = D3DMULTISAMPLE_TYPE(m_arAntiAlias[nAALevelID]);

				// Update corresponding spinner

				GetControl<ScreenScrollBar>(pSender->GetID() + 100).SetValue(nAALevelID);
			}
			break;
		case ID_CBO_AUDIODEST:
			{
				m_nAudioDest =
					(Audio::Destinations)dynamic_cast<ScreenComboBox*>(pSender)->GetSelectedItem();
			}
			break;
		}
	}
	else if (ScreenScrollBar::NOTIFY_SCROLL == nNotifyID)
	{
		if (pSender->GetID() == ID_SLIDER_MASTER ||
		   pSender->GetID() == ID_SLIDER_EFFECT ||
		   pSender->GetID() == ID_SLIDER_SPEECH ||
		   pSender->GetID() == ID_SLIDER_MUSIC)
		{
			ScreenScrollBar* pSlider = dynamic_cast<ScreenScrollBar*>(pSender);

			String str;
			str.Format(SZ_FORMAT_VOLUME, pSlider->GetValue());

			try
			{
				GetControl<ScreenLabel>(pSlider->GetID() - 1).SetText(str);
			}

			catch(Error) {}

			float fVolume = float(pSlider->GetValue()) / 100.0f;

			switch(pSender->GetID())
			{
			case ID_SLIDER_MASTER:
				m_fMasterVolume = fVolume;
				break;
			case ID_SLIDER_SPEECH:
				m_fSpeechVolume = fVolume;
				break;
			case ID_SLIDER_EFFECT:
				m_fEffectsVolume = fVolume;
				break;
			case ID_SLIDER_MUSIC:
				m_fMusicVolume = fVolume;
				break;
			}
		}
		else if (pSender->GetID() == ID_SPIN_AA ||
				pSender->GetID() == ID_SPIN_FMT ||
				pSender->GetID() == ID_SPIN_REF ||
				pSender->GetID() == ID_SPIN_RES)
		{
			ScreenScrollBar* pSlider = dynamic_cast<ScreenScrollBar*>(pSender);

			try
			{
				ScreenComboBox& rCombo =
					GetControl<ScreenComboBox>(pSender->GetID() - 100);

				if (rCombo.GetSelectedItem() != pSlider->GetValue())
					rCombo.SetSelectedItem(pSlider->GetValue());
			}
			
			catch(Error) {}
		}
	}

	return TRUE;
}

void ScreenOptions::OnCommand(int nCommandID, Screen* pSender, int nParam)
{
	switch(nCommandID)
	{
	case ID_CHECK_FS:
		{
			m_bFullScreen = dynamic_cast<ScreenButton*>(pSender)->GetToggle();

			LoadRefreshRates();
		}
		break;
	case ID_CHECK_VSYNC:
		{
			m_bVSync = dynamic_cast<ScreenButton*>(pSender)->GetToggle();
		}
		break;
	case ID_CHECK_MUTE:
		{
			m_bMasterMute = dynamic_cast<ScreenButton*>(pSender)->GetToggle();

			// Enable/disable master volume based on mute

			try
			{
				if (true == m_bMasterMute)
					GetControl<ScreenScrollBar>(ID_SLIDER_MASTER).SetFlag(DISABLED);
				else
					GetControl<ScreenScrollBar>(ID_SLIDER_MASTER).ClearFlag(DISABLED);
			}

			catch(Error) {}
		}
		break;
	case ID_TAB_GRAPHICS:
	case ID_TAB_AUDIO:
	case ID_TAB_GAME:
		{
			// Allow processing of keyboard controls

			m_rEngine.SetOption(Engine::OPTION_GAME_EVENTS, TRUE);
		}
		break;
	case ID_TAB_CONTROLS:
		{
			// Block processing of keyboard controls if
			// within controls tab and an item is selected

			m_rEngine.SetOption(Engine::OPTION_GAME_EVENTS,
				m_pControlsList->GetSelectedItem() == INVALID_INDEX);
		}
		break;
	case ID_OK:
	case ID_APPLY:
		{
			// Validate

			Game* pGame = dynamic_cast<Game*>(m_rEngine.GetClientInstance());

			if (NULL == pGame)
				break;

			// Apply graphics settings

			pGame->SetFullScreen(m_bFullScreen);
			pGame->SetVSync(m_bVSync);
			pGame->SetDisplayWidth(int(m_dwResWidth));
			pGame->SetDisplayHeight(int(m_dwResHeight));
			pGame->SetDeviceFormat(m_nFormat);
			pGame->SetMultiSampleType(m_nAALevel);
			pGame->SetRefreshRate(m_dwRefresh);
			pGame->ApplyGraphicsSettings();

			// Apply sound settings

			pGame->SetEffectsVolume(m_fEffectsVolume);
			pGame->SetSpeechVolume(m_fSpeechVolume);
			pGame->SetMusicVolume(m_fMusicVolume);
			m_rEngine.GetAudio().SetMasterVolumeMute(m_bMasterMute);
			m_rEngine.GetAudio().SetMasterVolume(m_fMasterVolume);
			m_rEngine.GetAudio().SetDestination(m_nAudioDest);

			// Apply control settings

			ControlManager& rControls = m_rEngine.GetClientInstance()->GetControls();
			
			rControls.Empty();			

			for(ControlSettingArrayConstIterator pos = m_arSettings.begin();
				pos != m_arSettings.end();
				pos++)
			{
				// If this setting is not a header, bind

				if (INVALID_INDEX != pos->nControlID)
					rControls.Bind(pos->nControlID, pos->nVirtKey);
			}

			if (ID_OK == nCommandID)
				Close();
		}
		break;
	}

	ScreenOverlapped::OnCommand(nCommandID, pSender, nParam);
}

void ScreenOptions::UpdateSelKeyImage(void)
{
	// Mark selected key on keyboard map by
	// showing and moving the selection image

	if (NULL == m_pSelImage || NULL == m_pControlsList)
		return;

	if (m_pControlsList->GetSelectedItem() == INVALID_INDEX)
		return;

	if (m_rEngine.GetClientInstance() == NULL)
		return;

	int nKeySelected = m_arSettings[m_pControlsList->GetSelectedItem()].nVirtKey;

	KeyGraphicMapConstIterator posFind =
		m_mapKeyGraphics.find(nKeySelected);

	if (m_mapKeyGraphics.end() == posFind)
	{
		m_pSelImage->SetFlag(INVISIBLE);
	}
	else
	{
		const KeyGraphic& rGraphic = *posFind->second;

		m_pSelImage->SetPosition(rGraphic.rcDestCoords.GetPosition());
		m_pSelImage->SetSize(rGraphic.rcDestCoords.GetSize());
		m_pSelImage->GetBackground().SetTextureCoords(rGraphic.rcSrcCoords);

		m_pSelImage->ClearFlag(INVISIBLE);
	}
}

void ScreenOptions::UpdateKeyOverview(void)
{
	if (m_pControlsList->GetSelectedItem() == INVALID_INDEX)
	{
		// Hide overview map

		if (true == m_bOverviewVisible)
		{
			m_bOverviewVisible = false;
			m_bOverviewFadeIn = false;

			m_rEngine.GetTimers().Add(this, m_fScreenFadeInterval, TIMER_ID);
		}
	}
	else
	{
		// Show and update overview map

		if (false == m_bOverviewVisible)
		{
			m_bOverviewVisible = true;
			m_bOverviewFadeIn = true;

			m_pOverviewParent->GetBlend().SetAlpha(0);
			m_pSelImage->GetBlend().SetAlpha(0);

			m_rEngine.GetTimers().Add(this, m_fScreenFadeInterval, TIMER_ID);
		}

		m_pOverviewParent->GetChildren().Empty();

		for(int n = 0; n < int(m_arSettings.size()); n++)
		{
			// Skip if this is a header

			if (INVALID_INDEX == m_arSettings[n].nControlID)
				continue;

			// Don't show if same as selected key

			if (m_pControlsList->GetSelectedItem() == n)
				continue;					

			// Find the key graphic for the used key

			KeyGraphicMapConstIterator posFind =
				m_mapKeyGraphics.find(m_arSettings[n].nVirtKey);

			if (m_mapKeyGraphics.end() == posFind)
				continue;
			
			// Create key image

			ScreenImage* pUsedKey = dynamic_cast<ScreenImage*>(
				ScreenImage::CreateInstance(m_rEngine,
					ScreenImage::SZ_CLASS, m_pOverviewParent));	

			pUsedKey->SetPosition(
				posFind->second->rcDestCoords.GetPosition());

			pUsedKey->SetSize(
				posFind->second->rcDestCoords.GetSize());

			pUsedKey->GetBackground() = m_pSelImage->GetBackground();
			
			pUsedKey->GetBackground().SetTextureCoords(
				posFind->second->rcSrcCoords);

			pUsedKey->SetFlag(BACKGROUND);

			pUsedKey->SetBlend(m_clrOverviewBlend);			

			m_pOverviewParent->GetChildren().Add(pUsedKey);
		}
	}
}

void ScreenOptions::OnTimer(Timer& rTimer)
{
	if (rTimer.GetID() != TIMER_ID)
	{
		ScreenOverlapped::OnTimer(rTimer);
		return;
	}

	int nOverviewAlpha = m_pOverviewParent->GetBlendConst().GetAlpha();

	if (true == m_bOverviewFadeIn)
	{
		nOverviewAlpha += m_nScreenFadeStep;

		if (nOverviewAlpha >= Color::MAX_CHANNEL)
		{
			m_pOverviewParent->GetBlend().SetAlpha(Color::MAX_CHANNEL);
			m_pSelImage->GetBlend().SetAlpha(Color::MAX_CHANNEL);

			if (m_pControlsTip != NULL && !m_pControlsTip->IsFlagSet(INVISIBLE))
				m_pControlsTip->SetFlag(INVISIBLE);

			m_rEngine.GetTimers().Remove(this, TIMER_ID);

			return;
		}
		else if (m_pOverviewParent->IsFlagSet(INVISIBLE))
		{
			m_pOverviewParent->ClearFlag(INVISIBLE);
		}

		if (m_pControlsTip != NULL && !m_pControlsTip->IsFlagSet(INVISIBLE))
			m_pControlsTip->GetBlend().SetAlpha(
				m_pControlsTip->GetBlend().GetAlpha() - m_nScreenFadeStep);
	}
	else
	{
		nOverviewAlpha -= m_nScreenFadeStep;

		if (nOverviewAlpha <= 0)
		{
			m_pOverviewParent->GetBlend().SetAlpha(0);
			m_pSelImage->GetBlend().SetAlpha(0);

			m_pOverviewParent->SetFlags(INVISIBLE);
			
			m_rEngine.GetTimers().Remove(this, TIMER_ID);

			return;
		}

		if (m_pControlsTip != NULL && !m_pControlsTip->IsFlagSet(INVISIBLE))
			m_pControlsTip->GetBlend().SetAlpha(
				m_pControlsTip->GetBlend().GetAlpha() + m_nScreenFadeStep);
	}

	m_pOverviewParent->GetBlend().SetAlpha(nOverviewAlpha);
	m_pSelImage->GetBlend().SetAlpha(nOverviewAlpha);
}