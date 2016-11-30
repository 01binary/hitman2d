/*------------------------------------------------------------------*\
|
| ThunderGraphics.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm rendering class(es) implementation
| Created: 08/10/2008
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

#define _WIN32_DCOM

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine, using INVALID_VALUE, defining Graphics
#include "ThunderClient.h"		// using Client
#include "ThunderTexture.h"		// using Texture
#include <comdef.h>				// using WMI
#include <Wbemidl.h>			// using WMI

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

// Supported device formats - definitions

const DWORD Graphics::DW_DEVICE_FORMATS[] =	{
												INVALID_VALUE,
												D3DFMT_X8R8G8B8,
												D3DFMT_X1R5G5B5,
												D3DFMT_R5G6B5
											};

// Triangle vertex declaration (2D)

const D3DVERTEXELEMENT9 Graphics::VD_TRI[] = {
												{0, 0, D3DDECLTYPE_FLOAT2,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_POSITION, 0},

												{0, 8, D3DDECLTYPE_D3DCOLOR,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_COLOR, 0},

												{0, 12, D3DDECLTYPE_FLOAT2,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_TEXCOORD, 0},

												D3DDECL_END()
											};

// Line vertex declaration (2D)

const D3DVERTEXELEMENT9 Graphics::VD_LINE[] = {
												{0, 0, D3DDECLTYPE_FLOAT2,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_POSITION, 0},

												{0, 8, D3DDECLTYPE_D3DCOLOR,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_COLOR, 0},

												D3DDECL_END()
											};

// Point vertex declaration (2D)

const D3DVERTEXELEMENT9 Graphics::VD_POINT[] =	{
												{0, 0, D3DDECLTYPE_FLOAT2,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_POSITION, 0},

												{0, 8, D3DDECLTYPE_D3DCOLOR,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_COLOR, 0},

												D3DDECL_END()
											};

// Point Sprite vertex declaration (3D)

const D3DVERTEXELEMENT9 Graphics::VD_PARTICLE[] =	{
												{0, 0, D3DDECLTYPE_FLOAT3,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_POSITION, 0},

												{0, 12, D3DDECLTYPE_D3DCOLOR,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_COLOR, 0},

												{0, 16, D3DDECLTYPE_FLOAT1,
												D3DDECLMETHOD_DEFAULT,
												D3DDECLUSAGE_PSIZE, 0},

												D3DDECL_END()
											};


/*----------------------------------------------------------*\
| Graphics implementation
\*----------------------------------------------------------*/

Graphics::Graphics(Engine& rEngine): m_rEngine(rEngine),

									 m_pD3D(NULL),
									 m_pD3DDevice(NULL),
									 m_pStateManager(NULL),

									 m_nD3DDeviceType(D3DDEVTYPE_HAL),
									 m_dwD3DDeviceFlags(0),

									 m_nRendering(0),
									 m_nBatching(0),
									 m_bClipping(false),

									 m_fFpsTime(0.0f),
									 m_nFpsFrames(0),
									 m_nFps(0),

									 m_dwFrame(0),

									 m_bSortMaterial(true),
									 m_bSortDepth(true),
									 m_bDebugCaptureQueue(false),

									 m_uBatches(0),
									 m_uMaxPrimsBatch(0),
									 m_uRenderables(0),
									 m_uTriangles(0),
									 m_uLines(0),
									 m_uPoints(0),

									 m_uLastBatches(0),
									 m_uLastMaxPrimsBatch(0),
									 m_uLastTriangles(0),
									 m_uLastLines(0),
									 m_uLastPoints(0),

									 m_pTriVD(NULL),
									 m_pLineVD(NULL),
									 m_pPointVD(NULL),
									 m_pParticleVD(NULL),

									 m_TriVB(sizeof(VertexTriangle),
									 D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY),

									 m_LinePtVB(sizeof(VertexLine),
									 D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY),

									 m_ParVB(sizeof(VertexParticle),
									 D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY),
									
									 m_TriIB(D3DUSAGE_WRITEONLY),
									 m_LineIB(D3DUSAGE_WRITEONLY),

									 m_pWireframeMaterial(NULL)

{
	ZeroMemory(&m_D3DDeviceCaps, sizeof(D3DCAPS9));
	ZeroMemory(&m_D3DDeviceMode, sizeof(D3DDISPLAYMODE));
	ZeroMemory(&m_D3DDeviceParams, sizeof(D3DPRESENT_PARAMETERS));
	ZeroMemory(&m_LastD3DDeviceParams, sizeof(D3DPRESENT_PARAMETERS));

	EmptyTransforms();
}

Graphics::~Graphics(void)
{
	Empty();
}

void Graphics::Initialize(HWND hDeviceWindow,
						  bool bFullScreen,
						  int nResolutionWidth,
						  int nResolutionHeight,
						  DeviceFormats nFormat,
						  D3DMULTISAMPLE_TYPE nMultiSampleType,
						  DWORD dwMultiSampleQuality,
						  DWORD dwRefreshRate,
						  bool bVSync,
						  bool bUseHardwareAcceleration,
						  bool bUseSoftwareVertexProcessing,
						  bool bPureDevice)
{
	// Create D3D9 instance

	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (NULL == m_pD3D)
		throw m_rEngine.GetErrors().Push(Error::D3D_CREATE,
			__FUNCTIONW__, D3D_SDK_VERSION);

	if (NULL == hDeviceWindow)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	// Get default adapter device capabilities

	m_dwD3DDeviceFlags = 0;

	ZeroMemory(&m_D3DDeviceCaps, sizeof(D3DCAPS9));
	ZeroMemory(&m_D3DDeviceMode, sizeof(D3DDISPLAYMODE));
	ZeroMemory(&m_D3DDeviceParams, sizeof(D3DPRESENT_PARAMETERS));

	m_nD3DDeviceType = (true == bUseHardwareAcceleration) ?
		D3DDEVTYPE_HAL : D3DDEVTYPE_REF;

	HRESULT hr = m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
		m_nD3DDeviceType, &m_D3DDeviceCaps);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_GETDEVICECAPS,
			__FUNCTIONW__, hr);

	// Validate device capabilities

	if (true == bUseHardwareAcceleration &&
	   m_rEngine.GetClientInstance() != NULL)
	{
		if (m_rEngine.GetClientInstance()->CheckDeviceCaps(
			m_D3DDeviceCaps) == false)
			   throw m_rEngine.GetErrors().Push(
			   Error::D3D_ADAPTERCAPS, __FUNCTIONW__);
	}

	// Set creation flags based on caps and passed options

	if (false == bUseHardwareAcceleration ||
	   true == bUseSoftwareVertexProcessing)
	{
		m_dwD3DDeviceFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		if (m_D3DDeviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		{
			m_dwD3DDeviceFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

			if (true == bPureDevice &&
				m_D3DDeviceCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
				m_dwD3DDeviceFlags |= D3DCREATE_PUREDEVICE;
		}
		else
		{
			m_dwD3DDeviceFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}
	
	if (true == bVSync)
	{
		// If v-sync is requested, see if it's available and use it

		if (m_D3DDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE)
			m_D3DDeviceParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			m_D3DDeviceParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		// If only v-sync is available, use it. Otherwise, use immediate

		if (~m_D3DDeviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
			m_D3DDeviceParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			m_D3DDeviceParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	if (true == bFullScreen)
	{
		D3DFORMAT nD3DDevFormat = D3DFMT_UNKNOWN;

		if (FORMAT_DESKTOP == nFormat)
		{
			// Select desktop format if specified

			m_pD3D->GetAdapterDisplayMode(
				D3DADAPTER_DEFAULT, &m_D3DDeviceMode);

			nD3DDevFormat = m_D3DDeviceMode.Format;
		}
		else
		{
			// Otherwise, select the exact format

			nD3DDevFormat = D3DFORMAT(DW_DEVICE_FORMATS[nFormat]);
		}		

		// Make sure at least one adapter mode with that format exists

		DWORD dwModeCount =
			m_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, nD3DDevFormat);

		if (0 == dwModeCount)
			throw m_rEngine.GetErrors().Push(Error::D3D_ADAPTERMODE,
				__FUNCTIONW__);

		// If so, find the one with the needed resolution and frequency

		while(dwModeCount-- > 0)
		{
			hr = m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,
				nD3DDevFormat, dwModeCount, &m_D3DDeviceMode);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3D_ENUMADAPTERMODES,
					__FUNCTIONW__, hr);

			if (int(m_D3DDeviceMode.Width) == nResolutionWidth &&
			   int(m_D3DDeviceMode.Height) == nResolutionHeight)
			{
				// If we have a specific refresh rate specified and
				// this mode does not have it, skip this mode

				if (dwRefreshRate != DEFAULT_VALUE &&
				   m_D3DDeviceMode.RefreshRate != dwRefreshRate) continue;

				// We found a device mode that has size, format, and refresh rate

				break;
			}
		}

		// If mode was not found, exit

		if (INVALID_VALUE == dwModeCount)
			throw m_rEngine.GetErrors().Push(Error::D3D_ADAPTERMODE,
				__FUNCTIONW__);

		// Otherwise, use it

		m_D3DDeviceParams.BackBufferFormat = m_D3DDeviceMode.Format;
		m_D3DDeviceParams.BackBufferWidth = m_D3DDeviceMode.Width;
		m_D3DDeviceParams.BackBufferHeight = m_D3DDeviceMode.Height;
		m_D3DDeviceParams.FullScreen_RefreshRateInHz = m_D3DDeviceMode.RefreshRate;
	}
	else
	{
		// Set back buffer format to desktop

		hr = m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_D3DDeviceMode);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_GETADAPTERDISPLAYMODE,
				__FUNCTIONW__, D3DADAPTER_DEFAULT, hr);

		m_D3DDeviceParams.BackBufferFormat = m_D3DDeviceMode.Format;
		m_D3DDeviceParams.BackBufferWidth = DWORD(nResolutionWidth);
		m_D3DDeviceParams.BackBufferHeight = DWORD(nResolutionHeight);
	}

	// Set multi-sampling parameters

	DWORD dwQualityStops = 0;

	hr = m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
		m_nD3DDeviceType, m_D3DDeviceParams.BackBufferFormat, !bFullScreen,
		nMultiSampleType, &dwQualityStops);

	while(FAILED(hr) && (nMultiSampleType - 1) > D3DMULTISAMPLE_NONE)
	{
		nMultiSampleType = (D3DMULTISAMPLE_TYPE)(nMultiSampleType - 1);

		hr = m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
			m_nD3DDeviceType, m_D3DDeviceParams.BackBufferFormat, !bFullScreen,
			nMultiSampleType, &dwQualityStops);
	}

	m_D3DDeviceParams.MultiSampleType = nMultiSampleType;

	if (dwMultiSampleQuality > dwQualityStops - 1)
		dwMultiSampleQuality = dwQualityStops - 1;

	m_D3DDeviceParams.MultiSampleQuality = dwMultiSampleQuality;

	// Set additional creation parameters

	m_D3DDeviceParams.BackBufferCount = 1;
	m_D3DDeviceParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_D3DDeviceParams.EnableAutoDepthStencil = FALSE;
	m_D3DDeviceParams.Windowed = !bFullScreen;
	m_D3DDeviceParams.hDeviceWindow = hDeviceWindow;

	// Last parameters as current

	CopyMemory(&m_LastD3DDeviceParams, &m_D3DDeviceParams, sizeof(D3DPRESENT_PARAMETERS));

	// Show the game window

	ShowWindow(hDeviceWindow, m_rEngine.GetClientInstance() ?
		m_rEngine.GetClientInstance()->GetCommandShow() : SW_SHOW);

	UpdateWindow(hDeviceWindow);

	// Create the device

	hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, m_nD3DDeviceType,
		hDeviceWindow, m_dwD3DDeviceFlags, &m_D3DDeviceParams, &m_pD3DDevice);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICECREATE,
			__FUNCTIONW__, hr);

	StateManager* pStates = NULL;

	try
	{
		// Create state manager	

		if (true == bPureDevice)
			pStates = new StateManagerPure(*this);
		else
			pStates = new StateManager(*this);
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, true == bPureDevice ?
			sizeof(StateManagerPure) : sizeof(StateManager));
	}

	m_pStateManager = pStates;

	pStates->ResetStates();	

	// Initialize effect pool

	m_effectPool.Initialize();

	// Initialize vertex and index cache

	InitializeCache(false);
}

void Graphics::InitializeCache(bool bVideoMemoryOnly)
{
	// Calculate buffer sizes based on max batch size

	UINT uVertexCacheCount =
		m_rEngine.GetOptionEx(Engine::OPTION_MAX_BATCH_PRIM) * 3;

	UINT uVertexCacheSize = sizeof(VertexTriangle) * uVertexCacheCount;

	// Allocate vertex cache

	if (false == bVideoMemoryOnly)
		m_VC.Create(uVertexCacheSize);

	// Create triangle vertex buffer

	m_TriVB.Create(*this, uVertexCacheCount);

	// Create line vertex buffer

	m_LinePtVB.Create(*this,
		m_rEngine.GetOptionEx(Engine::OPTION_MAX_BATCH_PRIM) * 2);

	// Create particle vertex buffer

	m_ParVB.Create(*this,
		m_rEngine.GetOptionEx(Engine::OPTION_MAX_BATCH_PRIM));

	// Create triangle index buffer

	m_TriIB.Create(*this,
		m_rEngine.GetOptionEx(Engine::OPTION_MAX_BATCH_PRIM) * 6);

	// Fill triangle index buffer with pre-calculated indexes

	const WORD W_TRI_INDICES[] = { 0, 1, 2, 2, 3, 0 };
	const DWORD DW_INDICES_COUNT = sizeof(W_TRI_INDICES) / sizeof(WORD);

	LPWORD pwIndices = NULL;
	m_TriIB.Lock(m_TriIB.GetIndexCount(), &pwIndices, 0);

	for(UINT n = 0, j = 0;
		n < m_TriIB.GetIndexCount();
		n += DW_INDICES_COUNT, j += 4)
	{
		pwIndices[n + 0] = W_TRI_INDICES[0] + WORD(j);
		pwIndices[n + 1] = W_TRI_INDICES[1] + WORD(j);
		pwIndices[n + 2] = W_TRI_INDICES[2] + WORD(j);
		pwIndices[n + 3] = W_TRI_INDICES[3] + WORD(j);
		pwIndices[n + 4] = W_TRI_INDICES[4] + WORD(j);
		pwIndices[n + 5] = W_TRI_INDICES[5] + WORD(j);
	}

	m_TriIB.Unlock();
	m_TriIB.Reset();

	// Create line index buffer

	m_LineIB.Create(*this,
		m_rEngine.GetOptionEx(Engine::OPTION_MAX_BATCH_PRIM) * 2);

	// Fill line index buffer with pre-calculated indexes

	m_LineIB.Lock(m_LineIB.GetIndexCount(), &pwIndices, 0);

	for(UINT n = 0, j = 0;
		n < m_LineIB.GetIndexCount();
		n += VertexLine::PRIM_INDEX_COUNT, j++)
	{
		pwIndices[n] = WORD(j);
		pwIndices[n + 1] = WORD(j + 1);
	}

	m_LineIB.Unlock();
	m_LineIB.Reset();

	// Create vertex declarations

	HRESULT hr = m_pD3DDevice->CreateVertexDeclaration(VD_TRI, &m_pTriVD);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CREATEVERTEXDECLARATION,
			__FUNCTIONW__, hr);

	hr = m_pD3DDevice->CreateVertexDeclaration(VD_LINE, &m_pLineVD);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CREATEVERTEXDECLARATION,
			__FUNCTIONW__, hr);

	hr = m_pD3DDevice->CreateVertexDeclaration(VD_POINT, &m_pPointVD);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CREATEVERTEXDECLARATION,
			__FUNCTIONW__, hr);

	hr = m_pD3DDevice->CreateVertexDeclaration(VD_PARTICLE, &m_pParticleVD);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CREATEVERTEXDECLARATION,
			__FUNCTIONW__, hr);
}

void Graphics::EmptyCache(bool bVideoMemoryOnly)
{
	if (false == bVideoMemoryOnly)
	{
		m_VC.Empty();

		EmptyRenderQueue();
	}

	// Reset active buffers

	if (m_pD3DDevice != NULL)
	{
		HRESULT hr = m_pD3DDevice->SetStreamSource(0, NULL, 0, 0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETSTREAMSOURCE, __FUNCTIONW__, hr);

		hr = m_pD3DDevice->SetIndices(NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETINDICES, __FUNCTIONW__, hr);

		hr = m_pD3DDevice->SetVertexDeclaration(NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETVERTEXDECLARATION, __FUNCTIONW__, hr);
	}

	SAFERELEASE(m_pTriVD);
	SAFERELEASE(m_pLineVD);
	SAFERELEASE(m_pPointVD);
	SAFERELEASE(m_pParticleVD);

	m_TriVB.Empty();
	m_LinePtVB.Empty();
	m_ParVB.Empty();

	m_TriIB.Empty();
	m_LineIB.Empty();
}

void Graphics::EmptyRenderQueue(void)
{
	// Reset render queue

	m_arRenderQueue.clear();

	// Reset cache tracking

	m_VC.Reset();

	// Reset buffer tracking

	m_TriVB.Reset();
	m_LinePtVB.Reset();
	m_ParVB.Reset();
	
	m_TriIB.Reset();
	m_LineIB.Reset();
}

UINT Graphics::AddTransform(const D3DXMATRIX& rTransform)
{
	for(UINT n = 0; n < m_arTransforms.size(); n++)
	{
		if (m_arTransforms[n] == rTransform)
			return n;
	}

	m_arTransforms.push_back(rTransform);

	return UINT(m_arTransforms.size() - 1);
}

void Graphics::EmptyTransforms(void)
{
	m_arTransforms.clear();

	D3DXMATRIX mtxIdentity;
	D3DXMatrixIdentity(&mtxIdentity);

	m_arTransforms.push_back(mtxIdentity);
}

void Graphics::Reset(ResetTypes nReset)
{
	HRESULT hr = 0;

	if (nReset != RECREATE_DEVICE)
	{
		// Check for required action

		hr = m_pD3DDevice->TestCooperativeLevel();

		// If no reset required, exit

		if (SUCCEEDED(hr) && RESET_DEFAULT == nReset)
			return;

		// If there was an internal driver error, exit

		if (D3DERR_DRIVERINTERNALERROR == hr)
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_TESTCOOPERATIVELEVEL,
				__FUNCTIONW__, hr);
		
		// If device has been lost, check periodically until it can be reset

		MSG msg;

		while(m_pD3DDevice->TestCooperativeLevel() != D3DERR_DEVICENOTRESET)
		{
			if (PeekMessage(&msg, m_D3DDeviceParams.hDeviceWindow,
				0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (nReset != RESET_DEFAULT)
			{
				break;
			}
			else
			{
				Sleep(100);
			}
		}
	}

	// Notify engine

	m_rEngine.OnLostDevice(RECREATE_DEVICE == nReset);

	// Release buffers

	EmptyCache(nReset != RECREATE_DEVICE);

	// Notify states

	m_pStateManager->OnLostDevice(RECREATE_DEVICE == nReset);

	// Notify material instances

	m_materialInstances.OnLostDevice(RECREATE_DEVICE == nReset);

	if (RECREATE_DEVICE == nReset)
	{
		// Release D3D device

		if (m_pD3DDevice != NULL)
		{
			for(int n = 0; n < 8; n++)
				m_pD3DDevice->SetTexture(n, NULL);

			SAFERELEASE(m_pD3DDevice);
		}

		// Re-create the device

		hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, m_nD3DDeviceType,
			m_rEngine.GetGameWindow(), m_dwD3DDeviceFlags, &m_D3DDeviceParams, &m_pD3DDevice);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICECREATE,
				__FUNCTIONW__, hr);	
	}
	else
	{
		// Reset the device

		hr = m_pD3DDevice->Reset(&m_D3DDeviceParams);

		if (FAILED(hr))
		{
			SAFERELEASE(m_pD3DDevice);

			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_RESET,
				__FUNCTIONW__, hr);
		}
	}

	// Update device mode

	m_pD3DDevice->GetDisplayMode(0, &m_D3DDeviceMode);

	// Re-create buffers

	InitializeCache(nReset != RECREATE_DEVICE);

	// Reset states

	m_pStateManager->OnResetDevice(RECREATE_DEVICE == nReset);

	// Notify engine

	m_rEngine.OnResetDevice(RECREATE_DEVICE == nReset);

	// Reset material instances
	// Must be done after resetting engine because we'll be re-capturing parameter blocks which require
	// all effects to have been created. Those are reloaded by Engine::OnResetDevice

	m_materialInstances.OnResetDevice(RECREATE_DEVICE == nReset);
}

MaterialInstanceShared* Graphics::GetWireframeMaterial(void)
{
	if (NULL == m_pWireframeMaterial)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR, __FUNCTIONW__, L"m_pWireframeMaterial");

	return m_pWireframeMaterial->GetSharedMaterial();
}

void Graphics::Screenshot(LPDIRECT3DSURFACE9* pOutSurface)
{
	LPDIRECT3DSURFACE9 pSurf = NULL;
	HRESULT hr = 0;

	if (NULL == pOutSurface)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pOutSurface");

	try
	{
		// Create screenshot surface

		DWORD dwWidth = (m_D3DDeviceParams.Windowed ?
			DWORD(GetSystemMetrics(SM_CXSCREEN)) :
			m_D3DDeviceParams.BackBufferWidth);

		DWORD dwHeight = (m_D3DDeviceParams.Windowed ?
			WORD(GetSystemMetrics(SM_CYSCREEN)) :
			m_D3DDeviceParams.BackBufferHeight);

		// Fixed format

		hr = m_pD3DDevice->CreateOffscreenPlainSurface(
			dwWidth, dwHeight, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM,
			&pSurf, NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_CREATEOFFSCREENPLAINSURFACE,
				__FUNCTIONW__, hr);

		// Get front buffer data

		hr = m_pD3DDevice->GetFrontBufferData(0, pSurf);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_GETFRONTBUFFERDATA, __FUNCTIONW__, hr);

		// Make sure alpha channel is opaque

		D3DLOCKED_RECT lr;

		hr = pSurf->LockRect(&lr, NULL, 0);

		if (FAILED(hr) || DWORD(lr.Pitch) != (dwWidth * 4))
		{
			pSurf->UnlockRect();

			throw m_rEngine.GetErrors().Push(
				Error::D3D_SURFACE_LOCKRECT, __FUNCTIONW__, hr);
		}

		for(BYTE *pbPixels = (BYTE*)lr.pBits,
			*pbEndPixels = pbPixels + dwWidth * dwHeight * sizeof(DWORD);
			pbPixels < pbEndPixels;
			pbPixels += 4)
		{
			pbPixels[3] = 255;
		}

		pSurf->UnlockRect();

		// If taking a screenshot in windowed mode, crop it to the window

		if (TRUE == m_D3DDeviceParams.Windowed)
		{
			RECT rcSrc = {0};

			GetClientRect(m_D3DDeviceParams.hDeviceWindow, &rcSrc);
			ClientToScreen(m_D3DDeviceParams.hDeviceWindow, (LPPOINT)&rcSrc);
			ClientToScreen(m_D3DDeviceParams.hDeviceWindow, (LPPOINT)&rcSrc.right);

			LPDIRECT3DSURFACE9 pCropSurf = NULL;

			hr = m_pD3DDevice->CreateOffscreenPlainSurface(rcSrc.right - rcSrc.left,
				rcSrc.bottom - rcSrc.top, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
				&pCropSurf, NULL);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_CREATEOFFSCREENPLAINSURFACE,
					__FUNCTIONW__, hr);
			
			hr = m_pD3DDevice->UpdateSurface(pSurf, &rcSrc, pCropSurf, NULL);

			pSurf->Release();

			pSurf = pCropSurf;

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_UPDATESURFACE, __FUNCTIONW__, hr);
		}

		// Return screenshot surface

		*pOutSurface = pSurf;
	}

	catch(Error& rError)
	{
		if (pSurf) pSurf->Release();

		throw rError;
	}
}

void Graphics::Screenshot(LPCWSTR pszPath, D3DXIMAGE_FILEFORMAT nFormat)
{
	LPDIRECT3DSURFACE9 pSurf = NULL;
	
	Screenshot(&pSurf);

	HRESULT hr = D3DXSaveSurfaceToFile(pszPath, nFormat, pSurf, NULL, NULL);

	if (FAILED(hr))
	{
		pSurf->Release();

		throw m_rEngine.GetErrors().Push(Error::D3DX_SAVESURFACETOFILE,
			__FUNCTIONW__, hr);
	}

	pSurf->Release();
}

void Graphics::BeginScene(void)
{
	if (0 == m_nRendering++)
	{
		// Began for the first time on this frame

		HRESULT hr = m_pD3DDevice->BeginScene();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_BEGINSCENE,
				__FUNCTIONW__, hr);

		// Clear scene statistics

		m_uLastBatches = m_uBatches;
		m_uLastMaxPrimsBatch = m_uMaxPrimsBatch;
		m_uLastRenderables = m_uRenderables;
		m_uLastTriangles = m_uTriangles;
		m_uLastLines = m_uLines;
		m_uLastPoints = m_uPoints;

		m_uBatches = 0;
		m_uMaxPrimsBatch = 0;
		m_uRenderables = 0;
		m_uTriangles = 0;
		m_uLines = 0;
		m_uPoints = 0;

		// Clear scene cache

		EmptyTransforms();

		// Prepare state manager for next frame

		m_pStateManager->BeginScene();
	}
}

void Graphics::EndScene(void)
{
	if (0 == m_nRendering)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	if (0 == --m_nRendering)
	{
		HRESULT hr = m_pD3DDevice->EndScene();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_ENDSCENE,
				__FUNCTIONW__, hr);

		// Finished frame

		m_dwFrame++;

		// Stop one-time queue snapshot if taken

		m_bDebugCaptureQueue = false;

		// Accumulate frame counters

		m_fFpsTime += m_rEngine.GetFrameTime();
		m_nFpsFrames++;

		if (m_fFpsTime > 0.999f)
		{
			// Update frame rate

			m_nFps = m_nFpsFrames;

			// Clamp frame rate value to refresh rate, if vsync is enabled

			if (m_D3DDeviceParams.PresentationInterval !=
				D3DPRESENT_INTERVAL_IMMEDIATE)
			{
				if (TRUE == m_D3DDeviceParams.Windowed)
				{
					if (m_nFps > int(m_D3DDeviceMode.RefreshRate))
						m_nFps = int(m_D3DDeviceMode.RefreshRate);
				}
				else
				{
					if (m_nFps > int(m_D3DDeviceParams.FullScreen_RefreshRateInHz))
						m_nFps = int(m_D3DDeviceParams.FullScreen_RefreshRateInHz);
				}
			}

			// Restart frame counter

			m_fFpsTime = 0.0f;
			m_nFpsFrames = 0;
		}
	}
}

void Graphics::BeginClipping(const RECT& rcClip)
{
	// Commit any open rendering batches before changing clipping options

	try
	{
		FlushBatch();
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::INTERNAL, __FUNCTIONW__);
	}	

	// Enable scissor test (hardware support required)

	HRESULT hr = m_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETRENDERSTATE,
			__FUNCTIONW__, hr);

	// Get current render target size

	LPDIRECT3DSURFACE9 pSurf = NULL;
	
	hr = m_pD3DDevice->GetRenderTarget(0, &pSurf);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_GETRENDERTARGET,
			__FUNCTIONW__, hr);

	D3DSURFACE_DESC desc;

	pSurf->GetDesc(&desc);
	pSurf->Release();

	RECT rcFinalClip;
	CopyRect(&rcFinalClip, &rcClip);

	// Make sure clip rect is within the bounds of render target

	if (rcClip.left < 0)
		rcFinalClip.left = 0;
	else if (rcClip.right >= int(desc.Width))
		rcFinalClip.right = int(desc.Width) - 1;

	if (rcClip.top < 0)
		rcFinalClip.top = 0;
	else if (rcClip.bottom >= int(desc.Height))
		rcFinalClip.bottom = int(desc.Height) - 1;

	// Set scissor rectangle

	hr = m_pD3DDevice->SetScissorRect(&rcFinalClip);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETSCISSORRECT,
			__FUNCTIONW__, hr);

	m_bClipping = true;
}

void Graphics::EndClipping(void)
{
	if (false == m_bClipping) return;

	try
	{
		FlushBatch();
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::INTERNAL, __FUNCTIONW__);
	}

	HRESULT hr =
		m_pD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETRENDERSTATE,
			__FUNCTIONW__, hr);

	m_bClipping = false;
}

void Graphics::BeginBatch(bool bSortDepth, bool bSortMaterial)
{
	if (m_nBatching > 0)
	{
		m_nBatching++;
		return;
	}

	m_bSortDepth = bSortDepth;
	m_bSortMaterial = bSortMaterial;

	m_nBatching++;

	m_VC.Reset();
}

void Graphics::EndBatch(void)
{
	if (0 == m_nBatching)
	{
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);
	}
	else if (1 == m_nBatching)
	{
		FlushBatch();
	}

	m_nBatching--;
}

void Graphics::FlushBatch(void)
{
	if (0 == m_nBatching || m_arRenderQueue.empty() == true)
		return;

	// Sort render queue

	if (m_arRenderQueue.size() > 1)
	{
		if (true == m_bSortDepth)
		{
			if (true == m_bSortMaterial)
			{
				std::sort(m_arRenderQueue.begin(),
					m_arRenderQueue.end(),
					Renderable::CompareDepthMaterial);
			}
			else
			{
				std::sort(m_arRenderQueue.begin(),
					m_arRenderQueue.end(),
					  Renderable::CompareDepth);
			}
		}
		else if (true == m_bSortMaterial)
		{
			std::sort(m_arRenderQueue.begin(),
				m_arRenderQueue.end(),
				Renderable::CompareMaterial);
		}
	}

	// Track renderables submitted for statistics

	m_uRenderables += UINT(m_arRenderQueue.size());
	UINT* puPrimTracking = NULL;

	// Track current vertex declaration to avoid setting when already set
	
	LPDIRECT3DVERTEXDECLARATION9 pVD = NULL;
	LPDIRECT3DVERTEXDECLARATION9 pLastVD = NULL;

	// Track current buffers to avoid setting them when already set

	VertexBuffer* pVB = NULL;
	IndexBuffer* pIB = NULL;
	IndexBuffer* pLastIB = NULL;

	// Track current material/technique to avoid setting when already set

	const EffectTechnique* pLastTechnique = NULL;
	MaterialInstanceShared* pLastMaterialInst = NULL;
	UINT uPasses = 0;

	// Track current transform to avoid setting when already set

	UINT uLastTransform = 0;

	// Traverse render queue, rendering items in batches

	HRESULT hr = 0;

	for(RenderableArrayIterator posFirst = m_arRenderQueue.begin();
		posFirst != m_arRenderQueue.end();)
	{
		// Determine vertex & primitive count in this batch

		RenderableArrayIterator posLast = posFirst;
		UINT uVertexCount = 0;
		UINT uPrimCount = 0;

		for(;
			posLast != m_arRenderQueue.end() &&
			posLast->nType == posFirst->nType &&
			posLast->pMaterial == posFirst->pMaterial &&
			posLast->nTransform == posFirst->nTransform;
			posLast++)
		{
			uVertexCount += posLast->uVertexCount;
			uPrimCount += posLast->uPrimitiveCount;
		}

		// Determine batch primitive type

		D3DPRIMITIVETYPE nPrimType = D3DPT_FORCE_DWORD;
		UINT uPrimIndexCount = 0;

		pLastVD = pVD;

		switch(posFirst->nType)
		{
		case Renderable::TYPE_TRIANGLELIST:
			nPrimType = D3DPT_TRIANGLELIST;
			uPrimIndexCount = VertexTriangle::PRIM_INDEX_COUNT;
			pVD = m_pTriVD;
			pVB = &m_TriVB;
			pIB = &m_TriIB;
			puPrimTracking = &m_uTriangles;
			break;
		case Renderable::TYPE_LINELIST:
			nPrimType = D3DPT_LINELIST;
			uPrimIndexCount = VertexLine::PRIM_INDEX_COUNT;
			pVD = m_pLineVD;
			pVB = &m_LinePtVB;
			pIB = &m_LineIB;
			puPrimTracking = &m_uLines;
			break;
		case Renderable::TYPE_LINESTRIP:
			nPrimType = D3DPT_LINESTRIP;
			uPrimIndexCount = VertexLine::PRIM_INDEX_COUNT;
			pVD = m_pLineVD;
			pVB = &m_LinePtVB;
			pIB = NULL;
			puPrimTracking = &m_uLines;
			break;
		case Renderable::TYPE_POINTLIST:
			nPrimType = D3DPT_POINTLIST;
			uPrimIndexCount = 0;
			pVD = m_pPointVD;
			pVB = &m_LinePtVB;
			pIB = NULL;
			puPrimTracking = &m_uPoints;
			break;
		case Renderable::TYPE_PARTICLELIST:
			nPrimType = D3DPT_POINTLIST;
			uPrimIndexCount = 0;
			pVD = m_pParticleVD;
			pVB = &m_ParVB;
			pIB = NULL;
			puPrimTracking = &m_uPoints;
			break;
		default:
			throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
			break;
		}

		// Lock vertex buffer (incremental for each submission)

		LPBYTE pbVB = NULL;

		pVB->Lock(uVertexCount * pVB->GetVertexSize(), (LPVOID*)&pbVB,
			pVB->GetUsedSize() > 0 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);

		// Fill vertex buffer

		for(RenderableArrayIterator pos = posFirst;
			pos != posLast;
			pos++)
		{
			CopyMemory(pbVB, pos->pbVertices,
				pos->uVertexCount * pVB->GetVertexSize());

			pbVB += pos->uVertexCount * pVB->GetVertexSize();
		}

		// Unlock vertex buffer

		pVB->Unlock();

		// Vertex declaration changed?

		if (pVD != pLastVD)
		{
			// Set vertex declaration

			hr = m_pD3DDevice->SetVertexDeclaration(pVD);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_SETVERTEXDECLARATION,
					__FUNCTIONW__, hr);

			// Set vertex buffer

			hr = m_pD3DDevice->SetStreamSource(0, pVB->GetBuffer(), 0, pVB->GetVertexSize());

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_SETSTREAMSOURCE, __FUNCTIONW__, hr);

			// Set index buffer

			if (pIB != pLastIB)
			{
				hr = m_pD3DDevice->SetIndices(NULL == pIB ? NULL : pIB->GetBuffer());

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3D_DEVICE_SETINDICES, __FUNCTIONW__, hr);

				pLastIB = pIB;
			}
		}

		// Transform changed?

		if (posFirst->nTransform != uLastTransform)
		{
			// Set transform

			m_pStateManager->SetTransform(D3DTS_WORLD,
				&m_arTransforms[posFirst->nTransform]);

			uLastTransform = posFirst->nTransform;
		}

		// Technique changed or material parameter block needs caching?

		if (pLastTechnique != posFirst->pMaterial->GetTechniqueConst() ||
			posFirst->pMaterial->IsDirty() == true)
		{
			// End previous material if any

			if (pLastMaterialInst != NULL)
				pLastMaterialInst->End();

			// Apply material

			posFirst->pMaterial->Apply();

			// Begin a new one

			uPasses = posFirst->pMaterial->Begin();

			pLastMaterialInst = posFirst->pMaterial;
			pLastTechnique = posFirst->pMaterial->GetTechniqueConst();
		}
		else
		{
			// Apply material (internals take care of not setting technique if set)

			if (pLastMaterialInst != posFirst->pMaterial)
			{
				posFirst->pMaterial->Apply();

				pLastMaterialInst = posFirst->pMaterial;
			}
		}		

		// Render vertices

		if (uPrimIndexCount != 0)
		{
			for(UINT uPass = 0; uPass < uPasses; uPass++)
			{
				posFirst->pMaterial->BeginPass(uPass);

				hr = m_pD3DDevice->DrawIndexedPrimitive(
					nPrimType, 0,
					pIB->GetMinVertex(), uVertexCount,
					pIB->GetUsedCount(), uPrimCount);

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
						__FUNCTIONW__, hr);

				posFirst->pMaterial->EndPass();
			}

			pIB->Reserve(uPrimCount * uPrimIndexCount, uVertexCount);

			if (true == m_bDebugCaptureQueue)
				m_rEngine.PrintDebug(
					L"DIP: z = %g\txf = %d\tvc = %4d\tpc = %4d\t\tmat = (%x) \"%s\"",
					posFirst->fZOrder,
					posFirst->nTransform, uVertexCount, uPrimCount,
					posFirst->pMaterial,
					PathFindFileName(posFirst->pMaterial->GetMaterial()->GetName()));
		}
		else
		{
			for(UINT uPass = 0; uPass < uPasses; uPass++)
			{
				posFirst->pMaterial->BeginPass(uPass);

				hr = m_pD3DDevice->DrawPrimitive(nPrimType,
					pVB->GetUsedCount() - uVertexCount, uPrimCount);

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
						__FUNCTIONW__, hr);

				posFirst->pMaterial->EndPass();
			}

			if (true == m_bDebugCaptureQueue)
				m_rEngine.PrintDebug(
					L"DP: z = %g\txf = %d\tvc = %4d\tpc = %4d\t\tmat = (%x) \"%s\"",
					posFirst->fZOrder,
					posFirst->nTransform, uVertexCount, uPrimCount,
					posFirst->pMaterial,
					PathFindFileName(posFirst->pMaterial->GetMaterial()->GetName()));
		}

		// Track batches submitted

		m_uBatches += uPasses;

		// Track primitives drawn

		*puPrimTracking += uPrimCount * uPasses;

		// Track average primitives per batch

		if (m_uMaxPrimsBatch < (uPrimCount * uPasses))
			m_uMaxPrimsBatch = (uPrimCount * uPasses);

		// Prepare for next batch

		posFirst = posLast;
	}

	// End last material

	if (pLastMaterialInst != NULL)
		pLastMaterialInst->End();

	// Clear queue

	EmptyRenderQueue();
}

void Graphics::Clear(D3DCOLOR clrClear)
{
	// If there is a video playing, don't clear

	if (m_rEngine.GetCurrentVideo() != NULL) return;

	HRESULT hr = m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, clrClear, 1.0f, 0);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CLEAR, __FUNCTIONW__, hr);
}

void Graphics::Clear(D3DCOLOR clrClear, LPCRECT prcClear)
{
	// If there is a full screen video playing, don't clear

	if (m_rEngine.GetCurrentVideo() != NULL) return;

	HRESULT hr = m_pD3DDevice->Clear(1, (const D3DRECT*)prcClear,
		D3DCLEAR_TARGET, clrClear, 1.0f, 0);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CLEAR,
			__FUNCTIONW__, hr);
}

void Graphics::RenderQuad(const MaterialInstance& rMaterialInst,
						  const Vector2& rvecPosition,
						  D3DCOLOR clrBlend,
						  float fZOrder,
						  const Vector2* pvecPivot,
						  const D3DXMATRIX* pmtxTransform)
{
	// Determine material to use

	MaterialInstanceShared* pMat =
		m_rEngine.GetOption(Engine::OPTION_WIREFRAME) ?
		GetWireframeMaterial() :
		rMaterialInst.GetSharedMaterial();

	// Validate

	if (pMat == NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pMat");

	// Get Texture coordinates

	float u1, v1, u2, v2;
	rMaterialInst.GetTextureCoords(u1, v1, u2, v2);

	// Generate vertices

	Vector2 vecSize(rMaterialInst.GetTextureCoords().GetSize());

	VertexTriangle vertices[4] = {

		// Top Left

		rvecPosition.x, rvecPosition.y,
		clrBlend,
		u1, v1,

		// Top Right

		rvecPosition.x + vecSize.x,
		rvecPosition.y,
		clrBlend,
		u2,	v1,

		// Bottom Right

		rvecPosition.x + vecSize.x,
		rvecPosition.y + vecSize.y,
		clrBlend,
		u2,	v2,

		// Bottom Left

		rvecPosition.x,
		rvecPosition.y + vecSize.y,
		clrBlend,
		u1, v2
	};

	if (IsBatching() == true)
	{
		// Flush batch if ran out of space

		if (m_VC.GetSizeFree() < sizeof(vertices))
			FlushBatch();

		Renderable r;

		// Calculate transform		
		
		if (pmtxTransform != NULL)
		{
			if (pvecPivot != NULL)
			{
				D3DXMATRIX mtxFinal, mtx;

				D3DXMatrixTranslation(&mtx,
					-(pvecPivot->x + rvecPosition.x),
					-(pvecPivot->y + rvecPosition.y), 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtx, pmtxTransform);

				D3DXMatrixTranslation(&mtx,
					rvecPosition.x,
					rvecPosition.y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtxFinal, &mtx);	

				r.nTransform = AddTransform(mtxFinal);
			}
			else
			{
				r.nTransform = AddTransform(*pmtxTransform);
			}
		}
		else
		{
			r.nTransform = 0;
		}

		// Add to queue

		if (m_arRenderQueue.empty() == true)
		{
			// Add new item

			r.nType = Renderable::TYPE_TRIANGLELIST;
			r.fZOrder = fZOrder;
			r.pMaterial = pMat;
			r.pbVertices = m_VC.GetCurrentPos();
			r.uVertexCount = 4;
			r.uPrimitiveCount = 2;

			m_arRenderQueue.push_back(r);
		}
		else
		{
			Renderable& rLast = m_arRenderQueue.back();

			if (Renderable::TYPE_TRIANGLELIST == rLast.nType &&
			   rLast.pMaterial == pMat &&
			   rLast.fZOrder == fZOrder &&
			   rLast.nTransform == r.nTransform)
			{
				// Attach to last item

				rLast.uPrimitiveCount += 2;
				rLast.uVertexCount += 4;
			}
			else
			{	
				// Add new item

				r.nType = Renderable::TYPE_TRIANGLELIST;
				r.fZOrder = fZOrder;
				r.pMaterial = pMat;
				r.pbVertices = m_VC.GetCurrentPos();
				r.uVertexCount = 4;
				r.uPrimitiveCount = 2;

				m_arRenderQueue.push_back(r);
			}
		}

		// Copy vertices to cache

		m_VC.Write((LPBYTE)vertices, sizeof(vertices));	
	}
	else
	{
		// Fill vertex buffer

		if (m_TriVB.GetFreeSize() < sizeof(vertices))
			return;

		LPBYTE pbData = NULL;
		m_TriVB.Lock(sizeof(vertices), (void**)&pbData, D3DLOCK_DISCARD);

		CopyMemory(pbData, vertices, sizeof(vertices));

		m_TriVB.Unlock();
		m_TriVB.Reset();

		// Set vertex declaration

		HRESULT hr = m_pD3DDevice->SetVertexDeclaration(m_pTriVD);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETVERTEXDECLARATION,
				__FUNCTIONW__, hr);

		// Set index buffer

		hr = m_pD3DDevice->SetIndices(m_TriIB.GetBuffer());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETINDICES,
				__FUNCTIONW__, hr);

		// Set transform

		if (pmtxTransform != NULL)
		{
			if (pvecPivot != NULL)
			{
				D3DXMATRIX mtxFinal, mtx;

				D3DXMatrixTranslation(&mtx,
					-(pvecPivot->x + rvecPosition.x),
					-(pvecPivot->y + rvecPosition.y), 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtx, pmtxTransform);

				D3DXMatrixTranslation(&mtx,
					rvecPosition.x,
					rvecPosition.y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtxFinal, &mtx);	

				m_pStateManager->SetTransform(D3DTS_WORLD, &mtxFinal);
			}
			else
			{
				m_pStateManager->SetTransform(D3DTS_WORLD, pmtxTransform);
			}
		}
		else
		{
			m_pStateManager->SetTransform(D3DTS_WORLD, &m_arTransforms[0]);
		}

		// Set material

		pMat->Apply();

		UINT uPasses = pMat->Begin();

		for(UINT u = 0; u < uPasses; u++)
		{
			pMat->BeginPass(u);

			hr = m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				0, 0, 4, 0, 2);

			if (FAILED(hr))
				m_rEngine.GetErrors().Push(Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
					__FUNCTIONW__, hr);

			pMat->EndPass();
		}

		pMat->End();

		m_uTriangles += 2 * uPasses;
	}
}

void Graphics::RenderQuad(const MaterialInstance& rMaterialInst,
						  const Vector2& rvecPosition,
						  const Vector2& rvecSize,
						  D3DCOLOR clrBlend)
{
	// Determine material to use

	MaterialInstanceShared* pMat =
		m_rEngine.GetOption(Engine::OPTION_WIREFRAME) ?
		GetWireframeMaterial() :
		rMaterialInst.GetSharedMaterial();

	// Validate

	if (pMat == NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pMat");

	// Get Texture coordinates

	float u1, v1, u2, v2;
	rMaterialInst.GetTextureCoords(u1, v1, u2, v2);

	// Generate vertices

	VertexTriangle vertices[4] = {

		// Top Left

		rvecPosition.x, rvecPosition.y,
		clrBlend,
		u1, v1,

		// Top Right

		rvecPosition.x + rvecSize.x,
		rvecPosition.y,
		clrBlend,
		u2,	v1,

		// Bottom Right

		rvecPosition.x + rvecSize.x,
		rvecPosition.y + rvecSize.y,
		clrBlend,
		u2,	v2,

		// Bottom Left

		rvecPosition.x,
		rvecPosition.y + rvecSize.y,
		clrBlend,
		u1, v2
	};

	if (IsBatching() == true)
	{
		// Flush batch if ran out of space

		if (m_VC.GetSizeFree() < sizeof(vertices))
			FlushBatch();

		// Add to queue

		Renderable r;

		if (m_arRenderQueue.empty() == true)
		{
			// Add new item			

			r.nType = Renderable::TYPE_TRIANGLELIST;
			r.nTransform = 0;
			r.fZOrder = 0.0f;
			r.pMaterial = pMat;
			r.pbVertices = m_VC.GetCurrentPos();
			r.uVertexCount = 4;
			r.uPrimitiveCount = 2;

			m_arRenderQueue.push_back(r);
		}
		else
		{
			Renderable& rLast = m_arRenderQueue.back();

			if (Renderable::TYPE_TRIANGLELIST == rLast.nType &&
			   rLast.pMaterial == pMat &&
			   rLast.fZOrder <= FLT_EPSILON &&
			   0 == rLast.nTransform)
			{
				// Attach to last item

				rLast.uPrimitiveCount += 2;
				rLast.uVertexCount += 4;
			}
			else
			{	
				// Add new item

				r.nType = Renderable::TYPE_TRIANGLELIST;
				r.fZOrder = 0.0f;
				r.nTransform = 0;
				r.pMaterial = pMat;
				r.pbVertices = m_VC.GetCurrentPos();
				r.uVertexCount = 4;
				r.uPrimitiveCount = 2;

				m_arRenderQueue.push_back(r);
			}
		}

		// Copy vertices to cache

		m_VC.Write((LPBYTE)vertices, sizeof(vertices));	
	}
	else
	{
		// Fill vertex buffer

		if (m_TriVB.GetFreeSize() < sizeof(vertices))
			return;

		LPBYTE pbData = NULL;
		m_TriVB.Lock(sizeof(vertices), (void**)&pbData, D3DLOCK_DISCARD);

		CopyMemory(pbData, vertices, sizeof(vertices));

		m_TriVB.Unlock();
		m_TriVB.Reset();

		// Set vertex declaration

		HRESULT hr = m_pD3DDevice->SetVertexDeclaration(m_pTriVD);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETVERTEXDECLARATION,
				__FUNCTIONW__, hr);

		// Set index buffer

		hr = m_pD3DDevice->SetIndices(m_TriIB.GetBuffer());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETINDICES,
				__FUNCTIONW__, hr);

		// Set transform

		m_pStateManager->SetTransform(D3DTS_WORLD, &m_arTransforms[0]);

		// Set material

		pMat->Apply();

		UINT uPasses = pMat->Begin();

		for(UINT u = 0; u < uPasses; u++)
		{
			pMat->BeginPass(u);

			hr = m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
				0, 0, 4, 0, 2);

			if (FAILED(hr))
				m_rEngine.GetErrors().Push(Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
					__FUNCTIONW__, hr);

			pMat->EndPass();
		}

		pMat->End();

		m_uTriangles += 2 * uPasses;
	}
}

void Graphics::RenderLines(const MaterialInstance& rMaterialInst,
						   const VertexLine* pVertices,
						   UINT uVertexCount,
						   UINT uPrimCount,
						   float fZOrder,
						   const Vector2* pvecPivot,
						   const D3DXMATRIX* pmtxTransform)
{
	// Validate

	MaterialInstanceShared* pMat =
		m_rEngine.GetOption(Engine::OPTION_WIREFRAME) ?
		GetWireframeMaterial() :
		rMaterialInst.GetSharedMaterial();

	if (pMat == NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pMat");

	UINT uSize = sizeof(VertexLine) * uVertexCount;

	if (IsBatching() == true)
	{
		if (m_VC.GetSizeFree() < uSize)
			FlushBatch();		
		
		// Add item to render queue

		Renderable r;

		r.nType = Renderable::TYPE_LINELIST;
		r.fZOrder = fZOrder;
		r.pMaterial = pMat;
		r.pbVertices = m_VC.GetCurrentPos();
		r.uVertexCount = uVertexCount;
		r.uPrimitiveCount = uPrimCount;

		// Cache vertices

		m_VC.Write(LPBYTE(pVertices), uSize);

		// Set transform
		
		if (pmtxTransform != NULL)
		{
			if (pvecPivot != NULL)
			{
				D3DXMATRIX mtxFinal, mtx;

				D3DXMatrixTranslation(&mtx,
					-pvecPivot->x, -pvecPivot->y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtx, pmtxTransform);

				D3DXMatrixTranslation(&mtx,
					pvecPivot->x, pvecPivot->y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtxFinal, &mtx);	

				r.nTransform = AddTransform(mtxFinal);
			}
			else
			{
				r.nTransform = AddTransform(*pmtxTransform);
			}
		}
		else
		{
			r.nTransform = 0;
		}

		m_arRenderQueue.push_back(r);
	}
	else
	{
		// Fill vertex and index buffers

		if (m_LinePtVB.GetFreeSize() < uSize)
			return;

		LPBYTE pbData = NULL;
		m_LinePtVB.Lock(uSize, (void**)&pbData, D3DLOCK_DISCARD);

		CopyMemory(pbData, pVertices, uSize);

		m_LinePtVB.Unlock();
		m_LinePtVB.Reset();
		
		// Set vertex declaration

		HRESULT hr = m_pD3DDevice->SetVertexDeclaration(m_pLineVD);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETVERTEXDECLARATION,
				__FUNCTIONW__, hr);

		// Set vertex buffer

		hr = m_pD3DDevice->SetStreamSource(0, m_LinePtVB.GetBuffer(),
			0, m_LinePtVB.GetVertexSize());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETSTREAMSOURCE,
				__FUNCTIONW__, hr);

		// Set index buffer

		hr = m_pD3DDevice->SetIndices(m_LineIB.GetBuffer());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETINDICES, 
				__FUNCTIONW__, hr);

		// Set transform

		if (pmtxTransform != NULL)
		{
			if (pvecPivot != NULL)
			{
				D3DXMATRIX mtxFinal, mtx;

				D3DXMatrixTranslation(&mtx,
					-pvecPivot->x, -pvecPivot->y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtx, pmtxTransform);

				D3DXMatrixTranslation(&mtx,
					pvecPivot->x, pvecPivot->y, 0.0f);

				D3DXMatrixMultiply(&mtxFinal, &mtxFinal, &mtx);	

				m_pStateManager->SetTransform(D3DTS_WORLD, &mtxFinal);
			}
			else
			{
				m_pStateManager->SetTransform(D3DTS_WORLD, pmtxTransform);
			}
		}
		else
		{
			m_pStateManager->SetTransform(D3DTS_WORLD, &m_arTransforms[0]);
		}

		// Set material

		pMat->Apply();

		UINT uPasses = pMat->Begin();

		for(UINT u = 0; u < uPasses; u++)
		{
			pMat->BeginPass(u);

			// Render primitive

			hr = m_pD3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST,
				0, 0, uVertexCount, 0, uPrimCount);			

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
					__FUNCTIONW__, hr);

			pMat->EndPass();
		}

		pMat->End();

		m_uLines += uPrimCount * uPasses;
	}	
}

void Graphics::RenderRectangle(const MaterialInstance& rMaterialInst,
		const Rect& rrc, D3DCOLOR clrBlend, float fZOrder,
		const Vector2* pvecPivot,
		const D3DXMATRIX* pmtxTransform)
{
	VertexLine vertices[] = {
		{ float(rrc.left), float(rrc.top - 1), clrBlend },
		{ float(rrc.right), float(rrc.top), clrBlend },
		{ float(rrc.right), float(rrc.bottom), clrBlend },
		{ float(rrc.left), float(rrc.bottom), clrBlend },
		{ float(rrc.left), float(rrc.top - 1), clrBlend }
	};

	RenderLines(rMaterialInst, vertices, 5, 4,
		fZOrder, pvecPivot, pmtxTransform);
}

void Graphics::RenderLine(const MaterialInstance& rMaterialInst,
		const Vector2& rvecStart, const Vector2& rvecEnd,
		D3DCOLOR clrBlend, float fZOrder)
{	
	VertexLine vertices[] = {
		{ rvecStart.x, rvecStart.y, clrBlend },
		{ rvecEnd.x, rvecEnd.y, clrBlend }
	};

	if (IsBatching() == true &&
		m_arRenderQueue.empty() == false &&
		m_VC.GetSizeFree() >= sizeof(VertexLine) * 2)
	{
		// Validate

		MaterialInstanceShared* pMat =
			m_rEngine.GetOption(Engine::OPTION_WIREFRAME) ?
			GetWireframeMaterial() :
			rMaterialInst.GetSharedMaterial();

		if (pMat == NULL)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
				__FUNCTIONW__, L"pMat");

		// Add to last item in queue if same options

		Renderable& rLast = m_arRenderQueue.back();

		if (rLast.nType == Renderable::TYPE_LINELIST &&
		   rLast.pMaterial == pMat &&
		   rLast.nTransform == 0)
		{
			m_VC.Write(LPBYTE(vertices), sizeof(vertices));

			rLast.uVertexCount += 2;
			rLast.uPrimitiveCount++;

			return;
		}
	}

	RenderLines(rMaterialInst, vertices,
		2, 1, fZOrder);
}

void Graphics::RenderPoints(const MaterialInstance& rMaterialInst,
		const VertexPoint* pPoints, UINT uCount, float fZOrder)
{
	// Validate

	MaterialInstanceShared* pMat =
		m_rEngine.GetOption(Engine::OPTION_WIREFRAME) ?
		GetWireframeMaterial() :
		rMaterialInst.GetSharedMaterial();

	if (pMat == NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"pMat");

	UINT uSize = sizeof(VertexPoint) * uCount;

	if (IsBatching() == true)
	{
		if (m_VC.GetSizeFree() < uSize)
		   FlushBatch();		
		
		// Add item to render queue

		Renderable r;

		r.nType = Renderable::TYPE_POINTLIST;
		r.fZOrder = fZOrder;
		r.pMaterial = pMat;
		r.pbVertices = m_VC.GetCurrentPos();
		r.uVertexCount = uCount;
		r.uPrimitiveCount = uCount;
		r.nTransform = 0;

		m_arRenderQueue.push_back(r);

		// Copy vertices to cache	

		m_VC.Write(LPBYTE(pPoints), uSize);
	}
	else
	{
		// Fill vertex buffer

		if (m_LinePtVB.GetFreeSize() < uSize)
			return;

		LPBYTE pbData = NULL;

		m_LinePtVB.Lock(uSize, (void**)&pbData, D3DLOCK_DISCARD);

		CopyMemory(pbData, pPoints, uSize);

		m_LinePtVB.Unlock();
		m_LinePtVB.Reset();

		// Set vertex declaration

		HRESULT hr = m_pD3DDevice->SetVertexDeclaration(m_pPointVD);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETVERTEXDECLARATION,
				__FUNCTIONW__, hr);

		// Set vertex buffer

		hr = m_pD3DDevice->SetStreamSource(0, m_LinePtVB.GetBuffer(), 0, sizeof(VertexPoint));

		// Set transform

		m_pStateManager->SetTransform(D3DTS_WORLD, &m_arTransforms[0]);

		// Set material

		pMat->Apply();

		UINT uPasses = pMat->Begin();

		for(UINT u = 0; u < uPasses; u++)
		{
			pMat->BeginPass(u);

			// Render primitive

			hr = m_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, uCount);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_DRAWPRIMITIVEUP,
					__FUNCTIONW__, hr);

			pMat->EndPass();
		}

		pMat->End();

		m_uPoints += uCount * uPasses;
	}
}

void Graphics::RenderParticles(const MaterialInstance& rMaterialInst,
	const VertexParticle* pParticles, UINT uCount, float fZOrder)
{
	// Validate

	if (rMaterialInst.GetSharedMaterial() == NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"GetSharedMaterial()");

	UINT uSize = sizeof(VertexParticle) * uCount;

	if (IsBatching() == true)
	{
		if (m_VC.GetSizeFree() < uSize)
		   FlushBatch();
		
		// Add item to render queue

		Renderable r;

		r.nType = Renderable::TYPE_PARTICLELIST;
		r.fZOrder = fZOrder;
		r.pMaterial = rMaterialInst.GetSharedMaterial();
		r.pbVertices = m_VC.GetCurrentPos();
		r.uVertexCount = uCount;
		r.uPrimitiveCount = uCount;
		r.nTransform = 0;

		m_arRenderQueue.push_back(r);

		// Copy vertices to cache

		m_VC.Write(LPBYTE(pParticles), uSize);
	}
	else
	{
		// Fill vertex buffer

		if (m_ParVB.GetFreeSize() < uSize)
			return;

		LPBYTE pbData = NULL;

		m_ParVB.Lock(uSize, (void**)&pbData, D3DLOCK_DISCARD);

		CopyMemory(pbData, pParticles, uSize);

		m_ParVB.Unlock();
		m_ParVB.Reset();

		// Set vertex declaration

		HRESULT hr = m_pD3DDevice->SetVertexDeclaration(m_pParticleVD);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETVERTEXDECLARATION,
				__FUNCTIONW__, hr);

		// Set vertex buffer

		hr = m_pD3DDevice->SetStreamSource(0, m_ParVB.GetBuffer(), 0, m_ParVB.GetVertexSize());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3D_DEVICE_SETSTREAMSOURCE,
				__FUNCTIONW__, hr);

		// Set material

		MaterialInstanceShared* pSharedMaterial =
			rMaterialInst.GetSharedMaterial();

		pSharedMaterial->Apply();

		UINT uPasses = pSharedMaterial->Begin();

		for(UINT u = 0; u < uPasses; u++)
		{
			pSharedMaterial->BeginPass(u);

			// Render primitive

			hr = m_pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, 0, uCount);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_DEVICE_DRAWPRIMITIVEUP,
					__FUNCTIONW__, hr);

			pSharedMaterial->EndPass();
		}

		pSharedMaterial->End();

		m_uPoints += uCount * uPasses;
	}
}

void Graphics::Present(void)
{
	// If there is a full screen video playing, don't Present

	if (m_rEngine.GetCurrentVideo() != NULL) return;

	if (NULL == m_pD3DDevice)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	HRESULT hr = m_pD3DDevice->Present(NULL, NULL, NULL, NULL);

	if (FAILED(hr))
	{
		if (hr == D3DERR_DEVICELOST)
			Reset();
		else
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_PRESENT,
				__FUNCTIONW__, hr);
	}
}

void Graphics::Empty(void)
{
	// Release state manager

	SAFERELEASE(m_pStateManager);

	// Empty effect pool

	m_effectPool.Empty();

	// Empty material instances

	m_pWireframeMaterial = NULL;

	m_materialInstances.Empty();

	// Empty vertex and index cache

	EmptyCache(false);

	// Release D3D device

	if (m_pD3DDevice != NULL)
	{
		for(int n = 0; n < 8; n++)
			m_pD3DDevice->SetTexture(n, NULL);

		m_pD3DDevice->Release();
		m_pD3DDevice = NULL;
	}

	// Release D3D

	if (m_pD3D != NULL)
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}
}

DWORD Graphics::GetAdapterMemory(void)
{
	// Stolen from
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa390423%28v=vs.85%29.aspx

	HRESULT hr = S_OK;
	DWORD dwMem = 0;

    // Initialize COM security

    hr = CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

    if (FAILED(hr) && hr != RPC_E_TOO_LATE)
		return 0;
    
    // Create WMI locator

    IWbemLocator *pLoc = NULL;

    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hr))
		return 0;

    // Connect to WMI

    IWbemServices *pSvc = NULL;

    hr = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pSvc                    // pointer to IWbemServices proxy
         );
    
    if (FAILED(hr))
    {
        pLoc->Release();     
        return 0;
    }
    
	// Set proxy security levels

    hr = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hr))
    {
        pSvc->Release();
        pLoc->Release();     
        
		return 0;
    }

    // Run the WIQL query

    IEnumWbemClassObject* pEnumerator = NULL;

    hr = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hr))
    {
        pSvc->Release();
        pLoc->Release();
        
		return 0;
    }

    // Get the data from the query
 
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
   
    while (pEnumerator)
    {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;

        // Get adapter memory

        hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);

        dwMem += DWORD(vtProp.uintVal);

        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();

	return dwMem;
}

/*----------------------------------------------------------*\
| Renderable implementation
\*----------------------------------------------------------*/

bool Renderable::CompareDepth(const Renderable& r1,
							  const Renderable& r2)
{
	if (r1.fZOrder >= r2.fZOrder)
		return false;

	if (r1.nType >= r2.nType)
		return false;

	return true;
}

bool Renderable::CompareMaterial(const Renderable& r1,
								 const Renderable& r2)
{
	if (r1.pMaterial >= r2.pMaterial)
		return false;

	if (r1.nType >= r2.nType)
		return false;

	return true;
}

bool Renderable::CompareDepthMaterial(const Renderable& r1,
									  const Renderable& r2)
{
	if (r1.fZOrder >= r2.fZOrder)
		return false;

	if (r1.pMaterial >= r2.pMaterial)
		return false;

	if (r1.nType >= r2.nType)
		return false;

	return true;
}