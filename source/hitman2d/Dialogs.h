/*------------------------------------------------------------------*\
|
| Dialogs.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D dialog box procedures header
| Created: 08/03/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef DIALOGS_H
#define DIALOGS_H

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

class Game;				// referencing Game
class DialogConfigure;	// referencing DialogConfigure


/*----------------------------------------------------------*\
| DialogGraphics
\*----------------------------------------------------------*/

class DialogGraphics
{
private:
	LPDIRECT3D9 m_pD3D;

	bool m_bHWAccel;
	bool m_bSWVP;
	bool m_bShaderDebug;
	bool m_bPure;

public:
	DialogGraphics(Game* pGame);
	~DialogGraphics(void);

public:
	inline bool GetHWAccel(void) const
	{
		return m_bHWAccel;
	}

	inline bool GetSWVP(void) const
	{
		return m_bSWVP;
	}

	inline bool GetPure(void) const
	{
		return m_bPure;
	}

	inline bool GetShaderDebug(void) const
	{
		return m_bShaderDebug;
	}

	int Show(HWND hWndParent, LPDIRECT3D9 pD3D);

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/*----------------------------------------------------------*\
| DialogAudio
\*----------------------------------------------------------*/

class DialogAudio
{
private:
	bool m_bExclusive;
	bool m_bDisableSounds;
	bool m_bDisableMusic;

public:
	DialogAudio(Game* pGame);
	~DialogAudio(void);

public:
	bool GetExclusive(void) const;
	bool GetDisableSound(void) const;
	bool GetDisableMusic(void) const;

	int Show(HWND hWndParent);

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/*----------------------------------------------------------*\
| DialogStartup
\*----------------------------------------------------------*/

class DialogStartup
{
private:
	String m_strLogFilePath;
	String m_strScriptPath;
	String m_strControlsPath;
	Client::LogMode m_nLogMode;

public:
	DialogStartup(Game* pGame);
	~DialogStartup(void);

public:
	const String& GetLogFilePath(void) const;
	const String& GetScriptPath(void) const;
	const String& GetControlsPath(void) const;
	Client::LogMode GetLogMode(void) const;

	int Show(HWND hWndParent);

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/*----------------------------------------------------------*\
| DialogConfigure
\*----------------------------------------------------------*/

class DialogConfigure
{
private:
	friend class DialogGraphics;

	Game* m_pGame;								// Saves game instance (engine not yet initialized)
	LPDIRECT3D9 m_pD3D;							// Temporary D3D instance for information retrieval

	Graphics::DeviceFormats m_nDevFormat;
	DWORD m_dwResWidth;
	DWORD m_dwResHeight;
	int m_nMultiSampleType;
	DWORD m_dwMultiSampleQuality;
	DWORD m_dwRefresh;

	DialogGraphics m_DialogGraphics;
	DialogAudio m_dialogAudio;
	DialogStartup m_dialogStartup;

public:
	DialogConfigure(Game* pGame);
	~DialogConfigure(void);

public:
	int Show(void);

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

/*----------------------------------------------------------*\
| DialogError
\*----------------------------------------------------------*/

class DialogError
{
private:
	HICON m_hErrorIcon;
	LPCWSTR m_pszText;
	bool m_bNotAgain;

public:
	DialogError(void);
	~DialogError(void);

public:
	bool GetNotAgain(void) const;

	int Show(HWND hWndParent, LPCWSTR pszText);

	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

} // namespace Hitman2D

#endif // DIALOGS_H