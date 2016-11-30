/*------------------------------------------------------------------*\
|
| Globals.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D global constants and functions
| Created: 08/12/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef HITMAN2D_GLOBALS_H
#define HITMAN2D_GLOBALS_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Shader Constant Names

const WCHAR SZ_SHADERCONST_TEXELSIZE[] =	L"texelsize";
const WCHAR SZ_SHADERCONST_FACTOR[] =		L"factor";

// Variable Names

const WCHAR SZ_VAR_SCREENFADEINACTIVE[] =	L"fade-inactive-alpha";
const WCHAR SZ_VAR_SCREENFADEACTIVE[] =		L"fade-active-alpha";
const WCHAR SZ_VAR_SCREENFADESTEP[] =		L"fade-step-alpha";
const WCHAR SZ_VAR_SCREENFADEINTERVAL[] =	L"fade-timer-interval";

const WCHAR SZ_VAR_SNDSCREENACTIVATE[] =	L"sound-activate";
const WCHAR SZ_VAR_SNDSCREENDEACTIVATE[] =	L"sound-deactivate";
const WCHAR SZ_VAR_SNDSCREENSTARTDRAG[] =	L"sound-start-drag";
const WCHAR SZ_VAR_SNDSCREENENDDRAG[] =		L"sound-end-drag";
const WCHAR SZ_VAR_SNDSCREENSELECT[] =		L"sound-select";
const WCHAR SZ_VAR_SNDSCREENACTION[] =		L"sound-action";

// Common Element Names

const WCHAR SZ_SCREEN_FLAGS[] =				L"flags";
const WCHAR SZ_SCREEN_STYLE[] =				L"style";
const WCHAR SZ_SCREEN_FONT[] =				L"font";
const WCHAR SZ_SCREEN_BACKGROUND[] =		L"background.material";
const WCHAR SZ_SCREEN_MARGIN[] =			L"margin";
const WCHAR SZ_MATERIAL_POSTFIX[] =			L".material";

// Screen themes

const WCHAR SZ_SHARED_THEMESTYLE[]		  = L"shared";

// Loading status messages

const LPCWSTR SZ_PROGRESS_TYPES[] =		{
											L"loading",
											L"saving"
										};

const LPCWSTR SZ_PROGRESS_SUBTYPES[] =	{
											L"session",
											L"session header",
											L"session maps",
											L"map",
											L"map header",
											L"map instance",
											L"map texture sheets",
											L"map animations",
											L"map sounds",
											L"map music",
											L"map tiles",
											L"map actors",
											L"map user data"
										};
// Console/log message colors

const LPCWSTR SZ_MESSAGECOLORS[] =		{
											L"message.color",
											L"echo.color",
											L"error.color",
											L"warning.color",
											L"info.color",
											L"debug.color"
										};
// Engine options

const LPCWSTR SZ_ENGINE_OPTIONS[] =		{
											L"manage-com",
											L"manage-game-window",

											L"tile-size",
											L"render-map",
											L"render-screens",
											L"custom-cursor",
											L"show-cursor",

											L"wireframe",
											L"max-batch-primitives",
											L"effect-compile-flags",

											L"disable-sounds",
											L"disable-music",
											L"exclusive-sound",
											L"audio-destination",
											L"sound-cache-frequency",

											L"enable-resource-cache",
											L"resource-cache-duration",
											L"resource-cache-frequency",
											L"enable-stream-cache",
											L"stream-cache-duration",
											L"stream-cache-frequency",

											L"enable-networking",
											L"net-port",		
											
											L"game-events",
											L"screen-events",
											L"map-events",
											L"progress-events",				
											
											L"enable-echo"											
										};

// Audio destination values

const LPCWSTR SZ_AUDIO_DEST[] =			{
											L"speakers",
											L"headphones"
										};

// MapBase flags

const LPCWSTR SZ_MAPFLAGS[] =			{
											L"default",

											L"background",
											L"background_tile",
											L"background_stretch",
											L"background_center",
											L"background_animated",

											L"render",
											
											L"update",

											L"optimize"
										};

const DWORD DW_MAPFLAGS[] =				{
											TileMap::DEFAULT,

											TileMap::BACKGROUND,
											TileMap::BACKGROUND_TILE,
											TileMap::BACKGROUND_STRETCH,
											TileMap::BACKGROUND_CENTER,
											TileMap::BACKGROUND_ANIMATED,
											
											TileMap::RENDER,
											
											TileMap::UPDATE,

											TileMap::OPTIMIZE
										};

// Actor flags

const LPCWSTR SZ_ACTORFLAGS[] =			{
											L"default",
											L"render",
											L"update",
											L"clip"
										};

const DWORD DW_ACTORFLAGS[] =			{
											Actor::DEFAULT,
											Actor::RENDER,
											Actor::UPDATE,
											Actor::CLIP
										};

// Supported Device formats

const LPCWSTR SZ_THUDEVICEFORMATS[] =	{
											L"Desktop",
											L"True Color (X8R8G8B8 24 bit)",
											L"High Color (X1R5G5B5 16 bit)",
											L"High Color (R5G6B5 16 bit)"
										};

const DWORD DW_THUDEVICEFORMATS[] =		{
											INVALID_VALUE,
											D3DFMT_X8R8G8B8,
											D3DFMT_X1R5G5B5,
											D3DFMT_R5G6B5
										};

// All device formats

const LPCTSTR SZ_DEVICEFORMATS[] =		{
											L"D3DFMT_R8G8B8",
											L"D3DFMT_A8R8G8B8",
											L"D3DFMT_X8R8G8B8",
											L"D3DFMT_R5G6B5",
											L"D3DFMT_X1R5G5B5",
											L"D3DFMT_A1R5G5B5",
											L"D3DFMT_A4R4G4B4",
											L"D3DFMT_R3G3B2",
											L"D3DFMT_A8",
											L"D3DFMT_A8R3G3B2",
											L"D3DFMT_X4R4G4B4",
											L"D3DFMT_A2B10G10R10",
											L"D3DFMT_A8B8G8R8",
											L"D3DFMT_X8B8G8R8",
											L"D3DFMT_G16R16",
											L"D3DFMT_A2R10G10B10",
											L"D3DFMT_A16B16G16R16"
										};

const DWORD DW_DEVICEFORMATS[] =		{
											D3DFMT_R8G8B8,
											D3DFMT_A8R8G8B8,
											D3DFMT_X8R8G8B8,
											D3DFMT_R5G6B5,
											D3DFMT_X1R5G5B5,
											D3DFMT_A1R5G5B5,
											D3DFMT_A4R4G4B4,
											D3DFMT_R3G3B2,
											D3DFMT_A8R3G3B2,
											D3DFMT_X4R4G4B4,
											D3DFMT_A2B10G10R10,
											D3DFMT_A8B8G8R8,
											D3DFMT_X8B8G8R8,
											D3DFMT_G16R16,
											D3DFMT_A2R10G10B10,
											D3DFMT_A16B16G16R16
										};
// All device types

const LPCTSTR SZ_DEVICETYPE[] =			{
											L"D3DDEVTYPE_HAL",
											L"D3DDEVTYPE_REF",
											L"D3DDEVTYPE_SW",
											L"D3DDEVTYPE_NULLREF"
										};

// Swap effects

const LPCTSTR SZ_SWAPEFFECT[3] =		{
											L"D3DSWAPEFFECT_DISCARD",
											L"D3DSWAPEFFECT_FLIP",
											L"D3DSWAPEFFECT_COPY"
										};

// Device caps

const LPCWSTR SZ_DEVCAPS_1[] =			{
											L"D3DCAPS_READ_SCANLINE"
										};

const DWORD DW_DEVCAPS_1[] =			{
											D3DCAPS_READ_SCANLINE
										};

const LPCWSTR SZ_DEVCAPS_2[] =			{
											L"D3DCAPS2_CANAUTOGENMIPMAP",
											L"D3DCAPS2_CANCALIBRATEGAMMA",
											L"D3DCAPS2_CANMANAGERESOURCE",
											L"D3DCAPS2_DYNAMICTEXTURES",
											L"D3DCAPS2_FULLSCREENGAMMA"
										};

const DWORD DW_DEVCAPS_2[] =			{
											D3DCAPS2_CANAUTOGENMIPMAP,
											D3DCAPS2_CANCALIBRATEGAMMA,
											D3DCAPS2_CANMANAGERESOURCE,
											D3DCAPS2_DYNAMICTEXTURES,
											D3DCAPS2_FULLSCREENGAMMA
										};

const LPCWSTR SZ_DEVCAPS_3[] =			{
											L"D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD",
											L"D3DCAPS3_COPY_TO_VIDMEM",
											L"D3DCAPS3_COPY_TO_SYSTEMMEM",
											L"D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION"
										};

const DWORD DW_DEVCAPS_3[] =			{
											D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD,
											D3DCAPS3_COPY_TO_VIDMEM,
											D3DCAPS3_COPY_TO_SYSTEMMEM,
											D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION
										};

const LPCWSTR SZ_PRESENT[] =			{
											L"D3DPRESENT_INTERVAL_IMMEDIATE",
											L"D3DPRESENT_INTERVAL_ONE",
											L"D3DPRESENT_INTERVAL_TWO",
											L"D3DPRESENT_INTERVAL_THREE",
											L"D3DPRESENT_INTERVAL_FOUR"
										};

const DWORD DW_PRESENT[] =				{
											D3DPRESENT_INTERVAL_IMMEDIATE,
											D3DPRESENT_INTERVAL_ONE,
											D3DPRESENT_INTERVAL_TWO,
											D3DPRESENT_INTERVAL_THREE,
											D3DPRESENT_INTERVAL_FOUR
										};

const LPCWSTR SZ_CURSOR[] =				{
											L"D3DCURSORCAPS_COLOR",
											L"D3DCURSORCAPS_LOWRES"
										};

const DWORD DW_CURSOR[] =				{
											D3DCURSORCAPS_COLOR,
											D3DCURSORCAPS_LOWRES
										};

const LPCWSTR SZ_DEVCAPS_4[] =			{
											L"D3DDEVCAPS_CANBLTSYSTONONLOCAL",
											L"D3DDEVCAPS_CANRENDERAFTERFLIP",
											L"D3DDEVCAPS_DRAWPRIMITIVES2",
											L"D3DDEVCAPS_DRAWPRIMITIVES2EX",
											L"D3DDEVCAPS_DRAWPRIMTLVERTEX",
											L"D3DDEVCAPS_EXECUTESYSTEMMEMORY",
											L"D3DDEVCAPS_EXECUTEVIDEOMEMORY",
											L"D3DDEVCAPS_HWRASTERIZATION",
											L"D3DDEVCAPS_HWTRANSFORMANDLIGHT",
											L"D3DDEVCAPS_NPATCHES",
											L"D3DDEVCAPS_PUREDEVICE",
											L"D3DDEVCAPS_QUINTICRTPATCHES",
											L"D3DDEVCAPS_RTPATCHES",
											L"D3DDEVCAPS_RTPATCHHANDLEZERO",
											L"D3DDEVCAPS_SEPARATETEXTUREMEMORIES",
											L"D3DDEVCAPS_TEXTURENONLOCALVIDMEM",
											L"D3DDEVCAPS_TEXTURESYSTEMMEMORY",
											L"D3DDEVCAPS_TEXTUREVIDEOMEMORY",
											L"D3DDEVCAPS_TLVERTEXSYSTEMMEMORY",
											L"D3DDEVCAPS_TLVERTEXVIDEOMEMORY"
										};

const DWORD DW_DEVCAPS_4[] =			{
											D3DDEVCAPS_CANBLTSYSTONONLOCAL,
											D3DDEVCAPS_CANRENDERAFTERFLIP,
											D3DDEVCAPS_DRAWPRIMITIVES2,
											D3DDEVCAPS_DRAWPRIMITIVES2EX,
											D3DDEVCAPS_DRAWPRIMTLVERTEX,
											D3DDEVCAPS_EXECUTESYSTEMMEMORY,
											D3DDEVCAPS_EXECUTEVIDEOMEMORY,
											D3DDEVCAPS_HWRASTERIZATION,
											D3DDEVCAPS_HWTRANSFORMANDLIGHT,
											D3DDEVCAPS_NPATCHES,
											D3DDEVCAPS_PUREDEVICE,
											D3DDEVCAPS_QUINTICRTPATCHES,
											D3DDEVCAPS_RTPATCHES,
											D3DDEVCAPS_RTPATCHHANDLEZERO,
											D3DDEVCAPS_SEPARATETEXTUREMEMORIES,
											D3DDEVCAPS_TEXTURENONLOCALVIDMEM,
											D3DDEVCAPS_TEXTURESYSTEMMEMORY,
											D3DDEVCAPS_TEXTUREVIDEOMEMORY,
											D3DDEVCAPS_TLVERTEXSYSTEMMEMORY,
											D3DDEVCAPS_TLVERTEXVIDEOMEMORY
										};

const LPCWSTR SZ_PRIM[] =				{
											L"D3DPMISCCAPS_MASKZ",
											L"D3DPMISCCAPS_CULLNONE",
											L"D3DPMISCCAPS_CULLCW",
											L"D3DPMISCCAPS_CULLCCW",
											L"D3DPMISCCAPS_COLORWRITEENABLE",
											L"D3DPMISCCAPS_CLIPPLANESCALEDPOINTS",
											L"D3DPMISCCAPS_CLIPTLVERTS",
											L"D3DPMISCCAPS_TSSARGTEMP",
											L"D3DPMISCCAPS_BLENDOP",
											L"D3DPMISCCAPS_NULLREFERENCE",
											L"D3DPMISCCAPS_INDEPENDENTWRITEMASKS",
											L"D3DPMISCCAPS_PERSTAGECONSTANT",
											L"D3DPMISCCAPS_FOGANDSPECULARALPHA",
											L"D3DPMISCCAPS_SEPARATEALPHABLEND",
											L"D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS",
											L"D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING",
											L"D3DPMISCCAPS_FOGVERTEXCLAMPED"
										};

const DWORD DW_PRIM[] =					{ 
											D3DPMISCCAPS_MASKZ,
											D3DPMISCCAPS_CULLNONE,
											D3DPMISCCAPS_CULLCW,
											D3DPMISCCAPS_CULLCCW,
											D3DPMISCCAPS_COLORWRITEENABLE,
											D3DPMISCCAPS_CLIPPLANESCALEDPOINTS,
											D3DPMISCCAPS_CLIPTLVERTS,
											D3DPMISCCAPS_TSSARGTEMP,
											D3DPMISCCAPS_BLENDOP,
											D3DPMISCCAPS_NULLREFERENCE,
											D3DPMISCCAPS_INDEPENDENTWRITEMASKS,
											D3DPMISCCAPS_PERSTAGECONSTANT,
											D3DPMISCCAPS_FOGANDSPECULARALPHA,
											D3DPMISCCAPS_SEPARATEALPHABLEND,
											D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS,
											D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING,
											D3DPMISCCAPS_FOGVERTEXCLAMPED
										};

const LPCWSTR SZ_RAST[] =				{
											L"D3DPRASTERCAPS_ANISOTROPY",
											L"D3DPRASTERCAPS_COLORPERSPECTIVE",
											L"D3DPRASTERCAPS_DITHER",
											L"D3DPRASTERCAPS_DEPTHBIAS",
											L"D3DPRASTERCAPS_FOGRANGE",
											L"D3DPRASTERCAPS_FOGTABLE",
											L"D3DPRASTERCAPS_FOGVERTEX",
											L"D3DPRASTERCAPS_MIPMAPLODBIAS",
											L"D3DPRASTERCAPS_MULTISAMPLE_TOGGLE",
											L"D3DPRASTERCAPS_SCISSORTEST",
											L"D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS",
											L"D3DPRASTERCAPS_WBUFFER",
											L"D3DPRASTERCAPS_WFOG",
											L"D3DPRASTERCAPS_ZBUFFERLESSHSR",
											L"D3DPRASTERCAPS_ZFOG",
											L"D3DPRASTERCAPS_ZTEST"
										};

const DWORD DW_RAST[] =					{
											D3DPRASTERCAPS_ANISOTROPY,
											D3DPRASTERCAPS_COLORPERSPECTIVE,
											D3DPRASTERCAPS_DITHER,
											D3DPRASTERCAPS_DEPTHBIAS,
											D3DPRASTERCAPS_FOGRANGE,
											D3DPRASTERCAPS_FOGTABLE,
											D3DPRASTERCAPS_FOGVERTEX,
											D3DPRASTERCAPS_MIPMAPLODBIAS,
											D3DPRASTERCAPS_MULTISAMPLE_TOGGLE,
											D3DPRASTERCAPS_SCISSORTEST,
											D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS,
											D3DPRASTERCAPS_WBUFFER,
											D3DPRASTERCAPS_WFOG,
											D3DPRASTERCAPS_ZBUFFERLESSHSR,
											D3DPRASTERCAPS_ZFOG,
											D3DPRASTERCAPS_ZTEST
										};

const LPCWSTR SZ_ZCMP[] =				{
											L"D3DPCMPCAPS_ALWAYS",
											L"D3DPCMPCAPS_EQUAL",
											L"D3DPCMPCAPS_GREATER",
											L"D3DPCMPCAPS_GREATEREQUAL",
											L"D3DPCMPCAPS_LESS",
											L"D3DPCMPCAPS_LESSEQUAL",
											L"D3DPCMPCAPS_NEVER",
											L"D3DPCMPCAPS_NOTEQUAL"
										};

const DWORD DW_ZCMP[] =					{
											D3DPCMPCAPS_ALWAYS,
											D3DPCMPCAPS_EQUAL,
											D3DPCMPCAPS_GREATER,
											D3DPCMPCAPS_GREATEREQUAL,
											D3DPCMPCAPS_LESS,
											D3DPCMPCAPS_LESSEQUAL,
											D3DPCMPCAPS_NEVER,
											D3DPCMPCAPS_NOTEQUAL
										};

const LPCWSTR SZ_SRCBLEND[] =			{ 
											L"D3DPBLENDCAPS_BLENDFACTOR",
											L"D3DPBLENDCAPS_BOTHINVSRCALPHA",
											L"D3DPBLENDCAPS_BOTHSRCALPHA",
											L"D3DPBLENDCAPS_DESTALPHA",
											L"D3DPBLENDCAPS_DESTCOLOR",
											L"D3DPBLENDCAPS_INVDESTALPHA",
											L"D3DPBLENDCAPS_INVDESTCOLOR",
											L"D3DPBLENDCAPS_INVSRCALPHA",
											L"D3DPBLENDCAPS_INVSRCCOLOR",
											L"D3DPBLENDCAPS_ONE",
											L"D3DPBLENDCAPS_SRCALPHA",
											L"D3DPBLENDCAPS_SRCALPHASAT",
											L"D3DPBLENDCAPS_SRCCOLOR",
											L"D3DPBLENDCAPS_ZERO"
										};

const DWORD DW_SRCBLEND[] =				{
											D3DPBLENDCAPS_BLENDFACTOR,
											D3DPBLENDCAPS_BOTHINVSRCALPHA,
											D3DPBLENDCAPS_BOTHSRCALPHA,
											D3DPBLENDCAPS_DESTALPHA,
											D3DPBLENDCAPS_DESTCOLOR,
											D3DPBLENDCAPS_INVDESTALPHA,
											D3DPBLENDCAPS_INVDESTCOLOR,
											D3DPBLENDCAPS_INVSRCALPHA,
											D3DPBLENDCAPS_INVSRCCOLOR,
											D3DPBLENDCAPS_ONE,
											D3DPBLENDCAPS_SRCALPHA,
											D3DPBLENDCAPS_SRCALPHASAT,
											D3DPBLENDCAPS_SRCCOLOR,
											D3DPBLENDCAPS_ZERO
										};

const LPCWSTR SZ_TEX[] =				{
											L"D3DPTEXTURECAPS_ALPHA",
											L"D3DPTEXTURECAPS_ALPHAPALETTE",
											L"D3DPTEXTURECAPS_CUBEMAP",
											L"D3DPTEXTURECAPS_CUBEMAP_POW2",
											L"D3DPTEXTURECAPS_MIPCUBEMAP",
											L"D3DPTEXTURECAPS_MIPMAP",
											L"D3DPTEXTURECAPS_MIPVOLUMEMAP",
											L"D3DPTEXTURECAPS_NONPOW2CONDITIONAL",
											L"D3DPTEXTURECAPS_NOPROJECTEDBUMPENV",
											L"D3DPTEXTURECAPS_PERSPECTIVE",
											L"D3DPTEXTURECAPS_POW2",
											L"D3DPTEXTURECAPS_PROJECTED",
											L"D3DPTEXTURECAPS_SQUAREONLY",
											L"D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE",
											L"D3DPTEXTURECAPS_VOLUMEMAP",
											L"D3DPTEXTURECAPS_VOLUMEMAP_POW2"
										};

const DWORD DW_TEX[] =					{
											D3DPTEXTURECAPS_ALPHA,
											D3DPTEXTURECAPS_ALPHAPALETTE,
											D3DPTEXTURECAPS_CUBEMAP,
											D3DPTEXTURECAPS_CUBEMAP_POW2,
											D3DPTEXTURECAPS_MIPCUBEMAP,
											D3DPTEXTURECAPS_MIPMAP,
											D3DPTEXTURECAPS_MIPVOLUMEMAP,
											D3DPTEXTURECAPS_NONPOW2CONDITIONAL,
											D3DPTEXTURECAPS_NOPROJECTEDBUMPENV,
											D3DPTEXTURECAPS_PERSPECTIVE,
											D3DPTEXTURECAPS_POW2,
											D3DPTEXTURECAPS_PROJECTED,
											D3DPTEXTURECAPS_SQUAREONLY,
											D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE,
											D3DPTEXTURECAPS_VOLUMEMAP,
											D3DPTEXTURECAPS_VOLUMEMAP_POW2
										};

const LPCWSTR SZ_TEXF[] =				{ 
											L"D3DPTFILTERCAPS_MAGFPOINT",
											L"D3DPTFILTERCAPS_MAGFLINEAR",
											L"D3DPTFILTERCAPS_MAGFANISOTROPIC",
											L"D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD",
											L"D3DPTFILTERCAPS_MAGFGAUSSIANQUAD",
											L"D3DPTFILTERCAPS_MINFPOINT",
											L"D3DPTFILTERCAPS_MINFLINEAR",
											L"D3DPTFILTERCAPS_MINFANISOTROPIC",
											L"D3DPTFILTERCAPS_MINFPYRAMIDALQUAD",
											L"D3DPTFILTERCAPS_MINFGAUSSIANQUAD",
											L"D3DPTFILTERCAPS_MIPFPOINT",
											L"D3DPTFILTERCAPS_MIPFLINEAR"
										};

const DWORD DW_TEXF[] =					{
											D3DPTFILTERCAPS_MAGFPOINT,
											D3DPTFILTERCAPS_MAGFLINEAR,
											D3DPTFILTERCAPS_MAGFANISOTROPIC,
											D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD,
											D3DPTFILTERCAPS_MAGFGAUSSIANQUAD,
											D3DPTFILTERCAPS_MINFPOINT,
											D3DPTFILTERCAPS_MINFLINEAR,
											D3DPTFILTERCAPS_MINFANISOTROPIC,
											D3DPTFILTERCAPS_MINFPYRAMIDALQUAD,
											D3DPTFILTERCAPS_MINFGAUSSIANQUAD,
											D3DPTFILTERCAPS_MIPFPOINT,
											D3DPTFILTERCAPS_MIPFLINEAR
										};

// CPU flags

const DWORD DW_CPUFLAGS[] =				{
											1u << 0,
											1u << 1,
											1u << 2,
											1u << 3,
											1u << 4,
											1u << 5,
											1u << 6,
											1u << 7,
											1u << 8,
											1u << 9,
											1u << 10,
											1u << 11,
											1u << 12,
											1u << 13,
											1u << 14,
											1u << 15,
											1u << 16,
											1u << 17,
											1u << 18,
											1u << 19,
											1u << 20,
											1u << 21,
											1u << 22,
											1u << 23,
											1u << 24,
											1u << 25,
											1u << 26,
											1u << 27,
											1u << 28,
											1u << 29,
											1u << 30,
											1u << 31
										};

const LPCWSTR SZ_CPUFLAGS[] =			{
											L"FPU",
											L"VME",
											L"DE",
											L"PSE",
											L"TSC",
											L"MSR",
											L"PAE",
											L"MCE",
											L"CXCHG8",
											L"APIC",
											L"reserved",
											L"SEP",
											L"MTRR",
											L"PGE",
											L"MCA",
											L"CMOV",
											L"PAT",
											L"PSE36",
											L"PSN",
											L"CLFL",
											L"reserved",
											L"DTES",
											L"ACPI",
											L"MMX",
											L"FXSR",
											L"SSE",
											L"SSE2",
											L"SS",
											L"HTT",
											L"TM1",
											L"IA64",
											L"PBE"
										};

// DirectSound caps

const DWORD DW_DSCAPS[] =				{
											DSCAPS_PRIMARYMONO,
											DSCAPS_PRIMARYSTEREO,
											DSCAPS_PRIMARY8BIT,
											DSCAPS_PRIMARY16BIT,
											DSCAPS_CONTINUOUSRATE,
											DSCAPS_EMULDRIVER,
											DSCAPS_CERTIFIED,
											DSCAPS_SECONDARYMONO,
											DSCAPS_SECONDARYSTEREO,
											DSCAPS_SECONDARY8BIT,
											DSCAPS_SECONDARY16BIT
										};

const LPCWSTR SZ_DSCAPS[] =				{
											L"DSCAPS_PRIMARYMONO",
											L"DSCAPS_PRIMARYSTEREO",
											L"DSCAPS_PRIMARY8BIT",
											L"DSCAPS_PRIMARY16BIT",
											L"DSCAPS_CONTINUOUSRATE",
											L"DSCAPS_EMULDRIVER",
											L"DSCAPS_CERTIFIED",
											L"DSCAPS_SECONDARYMONO",
											L"DSCAPS_SECONDARYSTEREO",
											L"DSCAPS_SECONDARY8BIT",
											L"DSCAPS_SECONDARY16BIT"
										};

const DWORD DW_SPEAKERCONFIG[] =		{
											DSSPEAKER_DIRECTOUT,
											DSSPEAKER_HEADPHONE,
											DSSPEAKER_MONO,
											DSSPEAKER_QUAD,
											DSSPEAKER_STEREO,
											DSSPEAKER_SURROUND,
											DSSPEAKER_5POINT1, // old
											DSSPEAKER_7POINT1,
											DSSPEAKER_7POINT1_SURROUND,
											DSSPEAKER_7POINT1_WIDE,
											0x00000009 // new
										};

const LPCWSTR SZ_SPEAKERCONFIG[] =		{
											L"DSSPEAKER_DIRECTOUT",
											L"DSSPEAKER_HEADPHONE",
											L"DSSPEAKER_MONO",
											L"DSSPEAKER_QUAD",
											L"DSSPEAKER_STEREO",
											L"DSSPEAKER_SURROUND",
											L"DSSPEAKER_5POINT1 [old]",
											L"DSSPEAKER_7POINT1",
											L"DSSPEAKER_7POINT1_SURROUND",
											L"DSSPEAKER_7POINT1_WIDE",
											L"DSSPEAKER_5POINT1_SURROUND [new]"
										};

const DWORD DW_SPEAKERGEOMETRY[] =		{
											DSSPEAKER_GEOMETRY_MIN,
											DSSPEAKER_GEOMETRY_NARROW,
											DSSPEAKER_GEOMETRY_WIDE,
											DSSPEAKER_GEOMETRY_MAX
										};

const LPCWSTR SZ_SPEAKERGEOMETRY[] =	{
											L"DSSPEAKER_GEOMETRY_MIN",
											L"DSSPEAKER_GEOMETRY_NARROW",
											L"DSSPEAKER_GEOMETRY_WIDE",
											L"DSSPEAKER_GEOMETRY_MAX"
										};

// Window show flags

const int N_SWFLAGS[] =					{
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

const LPCWSTR SZ_SWFLAGS[] =			{
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

// Print types

const LPCWSTR SZ_PRINTTYPES[] =			{
											L"message",
											L"echo",
											L"error",
											L"warning",
											L"info",
											L"debug",
											L"clear"
										};

const int N_PRINTTYPES[] =				{
											PRINT_MESSAGE,
											PRINT_ECHO,
											PRINT_ERROR,
											PRINT_WARNING,
											PRINT_INFO,
											PRINT_DEBUG,
											PRINT_CLEAR
										};


/*----------------------------------------------------------*\
| Prototypes
\*----------------------------------------------------------*/

float FormatMemory(DWORD dwSizeInBytes, LPCWSTR* ppszUnits);
D3DXIMAGE_FILEFORMAT ImageFormatFromExtension(LPCWSTR pszExt);

} // namespace Hitman2D

#endif // HITMAN2D_GLOBALS_H