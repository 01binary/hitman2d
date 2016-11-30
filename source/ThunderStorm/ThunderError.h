/*------------------------------------------------------------------*\
|
| ThunderError.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm error class
| Created: 01/25/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_ERROR_H
#define THUNDER_ERROR_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"		// using String

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Error;		// referencing Error, defined below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::stack<Error> ErrorStack;


/*----------------------------------------------------------*\
| Error class
\*----------------------------------------------------------*/

class Error: public std::exception
{
public:
	//
	// Constants
	//

	// ThunderStorm engine errors

	enum Codes
	{
		// General errors

		SUCCESS = 1000,
		INTERNAL,

		// Class factory errors

		CLASS_NOTREGISTERED,
		CLASS_CREATE,

		// Screen theme errors

		THEME_INVALID,
		THEME_INVALIDSTYLE,
		THEME_INVALIDSTYLEELEMENT,

		// Validation errors

		INVALID_PTR,
		INVALID_INDEX,
		INVALID_PARAM,
		INVALID_CALL,

		// Memory errors

		MEM_ALLOC,

		// File system errors

		FILE_OPEN,
		FILE_CREATE,
		FILE_READ,
		FILE_WRITE,
		FILE_SIGNATURE,
		FILE_VERSION,
		FILE_NOTUNICODE,
		FILE_FORMAT,
		FILE_PARSE,
		FILE_DESERIALIZE,
		FILE_SERIALIZE,
		FILE_ELEMENT,
		FILE_ELEMENTFORMAT,
		FILE_NULLINSTANCE,

		// Win32 system errors

		WIN_SYS_COINITIALIZE,
		WIN_SYS_LOADLIBRARY,
		WIN_SYS_FREELIBRARY,
		WIN_SYS_QUERYPERFORMANCEFREQUENCY,
		WIN_SYS_QUERYPERFORMANCECOUNTER,
		WIN_SYS_GETFILEVERSIONINFO,

		// Win32 user interface errors

		WIN_GUI_REGISTERCLASSEX,
		WIN_GUI_CREATEWINDOWEX,
		WIN_GUI_SETWINDOWPOS,
		
		// Win32 graphics device errors

		WIN_GDI_CREATECOMPATIBLEDC,
		WIN_GDI_CREATEFONT,
		WIN_GDI_GETKERNINGPAIRS,
		WIN_GDI_GETGLYPHOUTLINE,
		WIN_GDI_GETGLYPHINDICES,
		WIN_GDI_GETCHARABCWIDTHS,
		WIN_GDI_GETCHARWIDTH32,
		WIN_GDI_GETTEXTEXTENTPOINT32,
		WIN_GDI_CREATEDIBSECTION,
		WIN_GDI_GETFONTUNICODERANGES,

		// Win32 multimedia mixer errors

		WIN_MIXER_SETCONTROLDETAILS,
		WIN_MIXER_GETCONTROLDETAILS,
		WIN_MIXER_GETDEVCAPS,
		WIN_MIXER_GETLINEINFO,
		WIN_MIXER_OPEN,
		WIN_MIXER_GETLINECONTROLS,

		// Direct3D errors

		D3D_CREATE,
		D3D_DEVICECREATE,
		D3D_GETDEVICECAPS,
		D3D_ADAPTERCAPS,
		D3D_ADAPTERMODE,
		D3D_ENUMADAPTERMODES,
		D3D_GETADAPTERDISPLAYMODE,
		D3D_INVALIDSURFACEFORMAT,

		// Direct3D device errors

		D3D_DEVICE_VALIDATE,
		D3D_DEVICE_CLEAR,
		D3D_DEVICE_BEGINSCENE,
		D3D_DEVICE_ENDSCENE,
		D3D_DEVICE_SETTEXTURE,
		D3D_DEVICE_SETTRANSFORM,
		D3D_DEVICE_GETRENDERSTATE,
		D3D_DEVICE_SETRENDERSTATE,
		D3D_DEVICE_GETTEXTURESTAGESTATE,
		D3D_DEVICE_SETTEXTURESTAGESTATE,
		D3D_DEVICE_RESET,
		D3D_DEVICE_PRESENT,
		D3D_DEVICE_TESTCOOPERATIVELEVEL,
		D3D_DEVICE_CREATETEXTURE,
		D3D_DEVICE_UPDATETEXTURE,
		D3D_DEVICE_CREATEOFFSCREENPLAINSURFACE,
		D3D_DEVICE_UPDATESURFACE,
		D3D_DEVICE_GETBACKBUFFER,
		D3D_DEVICE_GETFRONTBUFFERDATA,
		D3D_DEVICE_GETRENDERTARGET,
		D3D_DEVICE_SETRENDERTARGET,
		D3D_DEVICE_SETFVF,
		D3D_DEVICE_COLORFILL,
		D3D_DEVICE_STRETCHRECT,
		D3D_DEVICE_SETSCISSORRECT,
		D3D_DEVICE_BEGINSTATEBLOCK,
		D3D_DEVICE_ENDSTATEBLOCK,
		D3D_DEVICE_SETSAMPLERSTATE,
		D3D_DEVICE_CREATEVERTEXBUFFER,
		D3D_DEVICE_CREATEVERTEXDECLARATION,
		D3D_DEVICE_CREATEINDEXBUFFER,
		D3D_DEVICE_SETVERTEXDECLARATION,
		D3D_DEVICE_SETSTREAMSOURCE,
		D3D_DEVICE_SETINDICES,
		D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
		D3D_DEVICE_DRAWINDEXEDPRIMITIVEUP,
		D3D_DEVICE_DRAWPRIMITIVE,
		D3D_DEVICE_DRAWPRIMITIVEUP,
		D3D_DEVICE_CREATESTATEBLOCK,

		// Direct3D texture errors

		D3D_TEXTURE_GETSURFACELEVEL,
		D3D_TEXTURE_GETLEVELDESC,
		D3D_TEXTURE_ADDDIRTYRECT,
		D3D_TEXTURE_LOCKRECT,
		D3D_TEXTURE_UNLOCKRECT,

		// Direct3D surface errors

		D3D_SURFACE_GETDESC,
		D3D_SURFACE_GETDC,
		D3D_SURFACE_RELEASEDC,
		D3D_SURFACE_LOCKRECT,
		D3D_SURFACE_UNLOCKRECT,

		// Direct3D vertex buffer errors

		D3D_VERTEXBUFFER_LOCK,
		D3D_VERTEXBUFFER_UNLOCK,

		// Direct3D index buffer errors

		D3D_INDEXBUFFER_LOCK,
		D3D_INDEXBUFFER_UNLOCK,

		// Direct3D state block errors

		D3D_STATEBLOCK_CAPTURE,
		D3D_STATEBLOCK_APPLY,

		// D3DX errors

		D3DX_CREATETEXTURE,
		D3DX_CREATETEXTUREFROMFILEEX,
		D3DX_CREATECUBETEXTUREFROMFILEEX,
		D3DX_SAVESURFACETOFILE,
		D3DX_SAVETEXTURETOFILE,
		D3DX_CREATEEFFECT,
		D3DX_CREATEEFFECTCOMPILERFROMFILE,
		D3DX_CREATEEFFECTPOOL,
		D3DX_CREATEFONT,
		D3DX_GETIMAGEINFOFROMFILE,

		// D3DXEffectCompiler errors

		D3DX_EFFECTCOMPILER_COMPILEEFFECT,

		// D3DXEffect errors

		D3DX_EFFECT_GETDESC,
		D3DX_EFFECT_GETTECHNIQUEDESC,
		D3DX_EFFECT_GETPARAMETERDESC,
		D3DX_EFFECT_SETSTATEMANAGER,
		D3DX_EFFECT_BEGINPARAMETERBLOCK,
		D3DX_EFFECT_ENDPARAMETERBLOCK,
		D3DX_EFFECT_APPLYPARAMETERBLOCK,
		D3DX_EFFECT_DELETEPARAMETERBLOCK,
		D3DX_EFFECT_SETTECHNIQUE,
		D3DX_EFFECT_VALIDATETECHNIQUE,
		D3DX_EFFECT_BEGIN,
		D3DX_EFFECT_COMMITCHANGES,
		D3DX_EFFECT_END,
		D3DX_EFFECT_BEGINPASS,
		D3DX_EFFECT_ENDPASS,
		D3DX_EFFECT_ONLOSTDEVICE,
		D3DX_EFFECT_ONRESETDEVICE,

		// DirectSound errors

		DSOUND_CREATE,
		DSOUND_SETCOOPERATIVELEVEL,
		DSOUND_CREATESOUNDBUFFER,
		DSOUND_SETSPEAKERCONFIG,
		DSOUND_GETSPEAKERCONFIG,

		// DirectSound buffer errors

		DSOUND_BUFFER_LOCK,
		DSOUND_BUFFER_UNLOCK,
		DSOUND_BUFFER_STOP,
		DSOUND_BUFFER_PLAY,
		DSOUND_BUFFER_GETVOLUME,
		DSOUND_BUFFER_SETVOLUME,
		DSOUND_BUFFER_DUPLICATE,

		// DirectShow errors

		DSHOW_FILTERGRAPHCREATE,
		DSHOW_VMRCREATE,

		// DirectShow filter graph errors

		DSHOW_FILTERGRAPH_ADDFILTER,
		DSHOW_FILTERGRAPH_MEDIACONTROLQUERY,
		DSHOW_FILTERGRAPH_MEDIAEVENTQUERY,
		DSHOW_FILTERGRAPH_MEDIAPOSITIONQUERY,
		DSHOW_FILTERGRAPH_BASICAUDIOQUERY,
		DSHOW_FILTERGRAPH_RENDERFILE,

		// DirectShow media control errors

		DSHOW_MEDIACONTROL_PAUSE,
		DSHOW_MEDIACONTROL_RUN,
		DSHOW_MEDIACONTROL_STOP,

		// DirectShow media position errors

		DSHOW_MEDIAPOSITION_PUTCURRENT,

		// DirectShow basic audio errors

		DSHOW_BASICAUDIO_PUTVOLUME,
		DSHOW_BASICAUDIO_GETVOLUME,

		// DirectShow media event errors

		DSHOW_MEDIAEVENT_SETNOTIFYWINDOW,
		DSHOW_MEDIAEVENT_SETNOTIFYFLAGS,		

		// DirectShow video mixing renderer errors

		DSHOW_VMR_QUERYFILTERCONFIG,

		// DirectShow filter config errors

		DSHOW_VMR_FILTERCONFIG_SETRENDERINGMODE,

		// DirectShow VMR filter errors

		DSHOW_VMR_QUERYWINDOWLESSCONTROL,

		// DirectShow windowless control errors

		DSHOW_VMR_WINDOWLESSCONTROL_SETCLIPPINGWINDOW,
		DSHOW_VMR_WINDOWLESSCONTROL_GETNATIVEVIDEOSIZE,
		DSHOW_VMR_WINDOWLESSCONTROL_SETVIDEOPOSITION,

		// Client errors

		CLIENT_ABORT,

		// User errors

		USER = 2000
	};

	static const WCHAR SZ_ERRORHEADER[];
	static const WCHAR SZ_ERRORFOOTER_COM[];
	static const WCHAR SZ_ERRORFOOTER_MM[];
	static const LPCWSTR SZ_ERRORS[];

protected:
	// Refers to enum above, client extensible
	int m_nCode;

	// Has been added to error stack? (set by ErrorManager)
	bool m_bIsOnStack;

	// Description based on error code
	String m_strDescription;

public:
	Error(int nCode, LPCWSTR pszFunctionName, ...);
	Error(va_list pArgList, int nCode, LPCWSTR pszFunctionName);
	Error(const Error& rInit);

protected:
	Error(void);

public:
	//
	// Code
	//

	int GetCode(void) const;

	//
	// Description
	//

	const String& GetDescription(void) const;
	virtual const char* what(void) const;

	//
	// Misc
	//

	bool IsOnStack(void) const;

	//
	// Operations
	//

	Error& operator=(const Error& rAssign);

protected:
	//
	// Private Functions
	//

	void BuildDescription(LPCWSTR pszUnformattedDescription,
		LPCWSTR pszFunctionName, va_list pArgList);

	//
	// Friends
	//

	friend class ErrorManager;
};

/*----------------------------------------------------------*\
| ErrorManager class
\*----------------------------------------------------------*/

class ErrorManager
{
private:
	// Gets returned when no error
	Error m_errSuccess;

	// Error hierarchy
	ErrorStack m_stackErrors;

public:
	ErrorManager(void);
	virtual ~ErrorManager(void);

public:
	//
	// Errors
	//

	const Error& GetLastError(void) const;

	Error& Push(int nCode, LPCWSTR pszFunctionName, ...);
	Error& Push(Error& rError);

	void Pop(void);

	int GetCount(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

} // namespace ThunderStorm

#endif