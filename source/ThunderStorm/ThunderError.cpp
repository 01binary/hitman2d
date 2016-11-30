/*------------------------------------------------------------------*\
|
| ThunderError.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm error class implementation
| Created: 01/25/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderString.h"		// using String
#include "ThunderError.h"		// defining Error, ErrorManager
#include <stdarg.h>				// using va_list, va_start etc

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Error::SZ_ERRORHEADER[] =		L"Error %d occurred in %s\r\n";
const WCHAR Error::SZ_ERRORFOOTER_COM[] =	L"\r\nExtended Code 0x%X: ";
const WCHAR Error::SZ_ERRORFOOTER_MM[] =	L"\r\nMultimedia Code 0x%X: ";

const LPCWSTR Error::SZ_ERRORS[] =	{
										L"Success.",
										L"Internal error.",

										L"Failed to create instance of \"%s\": class not registered.",
										L"Failed to create instance of \"%s\": creation callback returned NULL.",

										L"Failed to load user interface theme from \"%s\"",
										L"Failed to find specified theme style: \"%s\"",
										L"Failed to find specified theme style element: \"%s\", \"%s\"",
										
										L"Invalid pointer: %s",
										L"Invalid index: %s",
										L"Invalid parameter [%d].",
										L"Invalid call.",

										L"Failed to allocate %d bytes from process heap.",

										L"Failed to open \"%s\" with requested access.",
										L"Failed to create \"%s\" with requested access.",
										L"Failed to read \"%s\".",
										L"Failed to write \"%s\".",
										L"Invalid file signature: \"%s\".",
										L"Invalid file version: \"%s\".",
										L"Text file is not unicode: \"%s\".",
										L"Invalid file format: \"%s\".",
										L"Failed to parse \"%s\"\r\nLine %d, column %d, expected: %s.",
										L"Failed to deserialize file: \"%s\".",
										L"Failed to serialize file: \"%s\".",
										L"Failed to find file element: \"%s\" in \"%s\".",
										L"Invalid file element format: \"%s\" in \"%s\".",
										L"Null object instance encountered while reading \"%s\".",

										L"#CoInitialize failed.",
										L"#LoadLibrary failed: could not load \"%s\".",
										L"#FreeLibrary failed: could not free \"%s\".",
										L"#QueryPerformanceFrequency failed.",
										L"#QueryPerformanceCounter failed.",
										L"#GetFileVersionInfo failed.",

										L"#RegisterClassEx failed.",
										L"#CreateWindowEx failed.",
										L"#SetWindowPos failed.",

										L"#CreateCompatibleDC failed.",
										L"#CreateFont failed.",
										L"#GetKerningPairs failed.",
										L"#GetGlyphOutline failed.",
										L"#GetGlyphIndices failed.",
										L"#GetCharABCWidths failed.",
										L"#GetCharWidth32 failed.",
										L"#GetTextExtentPoint32 failed.",
										L"#CreateDIBSection failed.",
										L"#GetFontUnicodeRanges failed.",

										L"@mixerSetControlDetails failed.",
										L"@mixerGetControlDetails failed.",
										L"@mixerGetDevCaps failed.",
										L"@mixerGetLineInfo failed.",
										L"@mixerOpen failed.",
										L"@mixerGetLineControls failed.",

										L"Direct3DCreate9 failed (SDK version 0x%x).",
										L"#IDirect3D9::CreateDevice failed.",
										L"#IDirect3D9::GetDeviceCaps failed.",
										L"Adapter does not support required capabilities.",
										L"Adapter does not support requested mode.",
										L"#IDirect3D9::EnumAdapterModes failed.",
										L"#IDirect3D9::GetAdapterDisplayMode on adapter %d failed.",
										L"#Invalid Direct3D9 surface format.",

										L"#IDirect3DDevice9::ValidateDevice failed.",
										L"#IDirect3DDevice9::Clear failed.",
										L"#IDirect3DDevice9::BeginScene failed.",
										L"#IDirect3DDevice9::EndScene failed.",
										L"#IDirect3DDevice9::SetTexture failed.",
										L"#IDirect3DDevice9::SetTransform failed.",
										L"#IDirect3DDevice9::GetRenderState failed.",
										L"#IDirect3DDevice9::SetRenderState failed.",
										L"#IDirect3DDevice9::GetTextureStageState failed.",
										L"#IDirect3DDevice9::SetTextureStageState failed.",
										L"#IDirect3DDevice9::Reset failed.",
										L"#IDirect3DDevice9::Present failed.",
										L"#IDirect3DDevice9::TestCooperativeLevel failed.",
										L"#IDirect3DDevice9::CreateTexture failed.",
										L"#IDirect3DDevice9::UpdateTexture failed.",
										L"#IDirect3DDevice9::CreateOffscreenPlainSurface failed.",
										L"#IDirect3DDevice9::UpdateSurface failed.",
										L"#IDirect3DDevice9::GetBackBuffer failed.",
										L"#IDirect3DDevice9::GetFrontBufferData failed.",
										L"#IDirect3DDevice9::GetRenderTarget failed.",
										L"#IDirect3DDevice9::SetRenderTarget failed.",
										L"#IDirect3DDevice9::SetFVF failed.",
										L"#IDirect3DDevice9::ColorFill failed.",
										L"#IDirect3DDevice9::StretchRect failed.",
										L"#IDirect3DDevice9::SetScissorRect failed.",
										L"#IDirect3DDevice9::BeginStateBlock failed.",
										L"#IDirect3DDevice9::EndStateBlock failed.",
										L"#IDirect3DDevice9::SetSamplerState failed.",
										L"#IDirect3DDevice9::CreateVertexBuffer failed.",
										L"#IDirect3DDevice9::CreateVertexDeclaration failed.",
										L"#IDirect3DDevice9::CreateIndexBuffer failed.",
										L"#IDirect3DDevice9::SetVertexDeclaration failed.",
										L"#IDirect3DDevice9::SetStreamSource failed.",
										L"#IDirect3DDevice9::SetIndices failed.",
										L"#IDirect3DDevice9::DrawIndexedPrimitive failed.",
										L"#IDirect3DDevice9::DrawIndexedPrimitiveUP failed.",
										L"#IDirect3DDevice9::DrawPrimitive failed.",
										L"#IDirect3DDevice9::DrawPrimitiveUP failed.",
										L"#IDirect3DDevice9::CreateStateBlock failed.",

										L"#IDirect3DTexture9::GetSurfaceLevel failed.",
										L"#IDirect3DTexture9::GetLevelDesc failed.",
										L"#IDirect3DTexture9::AddDirtyRect failed.",
										L"#IDirect3DTexture9::LockRect failed.",
										L"#IDirect3DTexture9::UnlockRect failed.",

										L"#IDirect3DSurface9::GetDesc failed.",
										L"#IDirect3DSurface9::GetDC failed.",
										L"#IDirect3DSurface9::ReleaseDC failed.",
										L"#IDirect3DSurface9::LockRect failed.",
										L"#IDirect3DSurface9::UnlockRect failed.",

										L"#IDirect3DVertexBuffer9::Lock failed with 0x%x flag.",
										L"#IDirect3DVertexBuffer9::Unlock failed.",

										L"#IDirect3DIndexBuffer9::Lock failed with 0x%x flag.",
										L"#IDirect3DIndexBuffer9::Unlock failed.",

										L"#IDirect3DStateBlock9::Capture failed.",
										L"#IDirect3DStateBlock9::Apply failed.",

										L"#D3DXCreateTexture failed.",
										L"#D3DXCreateTextureFromFileEx failed on \"%s\".",
										L"#D3DXCreateCubeTextureFromFileEx failed on \"%s\".",
										L"#D3DXSaveSurfaceToFile failed.",
										L"#D3DXSaveTextureToFile failed.",
										L"#D3DXCreateEffect failed.",
										L"#D3DXCreateEffectCompilerFromFile failed on \"%s\":\r\n%s.",
										L"#D3DXCreateEffectPool failed.",
										L"#D3DXCreateFont failed.",
										L"#D3DXGetImageInfoFromFile failed on \"%s\".",

										L"#D3DXEffectCompiler::CompileEffect failed:\n%s.",

										L"#ID3DXEffect::GetDesc failed.",
										L"#ID3DXEffect::GetTechniqueDesc failed.",
										L"#ID3DXEffect::GetParameterDesc failed.",
										L"#ID3DXEffect::SetStateManager failed.",
										L"#ID3DXEffect::BeginParameterBlock failed.",
										L"#ID3DXEffect::EndParameterBlock failed.",
										L"#ID3DXEffect::ApplyParameterBlock failed.",
										L"#ID3DXEffect::DeleteParameterBlock failed.",
										L"#ID3DXEffect::SetTechnique failed.",
										L"#ID3DXEffect::ValidateTechnique failed.",
										L"#ID3DXEffect::Begin failed.",
										L"#ID3DXEffect::CommitChanges failed.",
										L"#ID3DXEffect::End failed.",
										L"#ID3DXEffect::BeginPass failed.",
										L"#ID3DXEffect::EndPass failed.",
										L"#ID3DXEffect::OnLostDevice failed.",
										L"#ID3DXEffect::OnResetDevice failed.",

										L"#DSoundCreate8 failed.",
										L"#IDirectSound8::SetCooperativeLevel failed.",
										L"#IDirectSound8::CreateSoundBuffer failed.",
										L"#IDirectSound8::SetSpeakerConfig failed.",
										L"#IDirectSound8::GetSpeakerConfig failed.",

										L"#IDSoundBuffer8::Lock failed.",
										L"#IDSoundBuffer8::Unlock failed.",
										L"#IDSoundBuffer8::Stop failed.",
										L"#IDSoundBuffer8::Play failed.",
										L"#IDSoundBuffer8::GetVolume failed.",
										L"#IDSoundBuffer8::SetVolume failed.",
										L"#IDSoundBuffer8::Duplicate failed.",

										L"#CoCreateInstance(IGraphBuilder) failed.",
										L"#CoCreateInstance(IBaseFilter) failed.",

										L"#IGraphBuilder::AddFilter failed.",
										L"#IGraphBuilder::QueryInterface(IMediaControl) failed.",
										L"#IGraphBuilder::QueryInterface(IMediaEvent) failed.",
										L"#IGraphBuilder::QueryInterface(IMediaPosition) failed.",
										L"#IGraphBuilder::QueryInterface(IBasicAudio) failed.",
										L"#IGraphBuilder::RenderFile failed.",

										L"#IMediaControl::Pause failed.",
										L"#IMediaControl::Run failed.",
										L"#IMediaControl::Stop failed.",

										L"#IMediaPosition::put_CurrentPosition failed.",

										L"#IBasicAudio::put_Volume failed.",
										L"#IBasicAudio::get_Volume failed.",

										L"#IMediaEvent::SetNotifyWindow failed.",
										L"#IMediaEvent::SetNotifyFlags failed.",

										L"#QueryInterface(IVMRFilterConfig) failed.",

										L"#IVMRFilterConfig::SetRenderingMode failed.",

										L"#IBaseFilter::QueryInterface(IVMRWindowlessControl) failed.",

										L"#IWindowlessControl::SetClippingWindow failed.",
										L"#IWindowlessControl::GetNativeVideoSize failed.",
										L"#IWindowlessControl::SetVideoPosition failed."
									};

const LPCWSTR SZ_MM_ERRORS[] =		{
										L"No error.",
										L"Internal error.",
										L"Device ID out of range.",
										L"Driver failed enable.",
										L"Device already allocated.",
										L"Device handle is invalid.",
										L"No device driver present.",
										L"Memory allocation failed.",
										L"Function not supported.",
										L"Error code out of range.",
										L"Invalid flag passed.",
										L"Invalid parameter passed.",
										L"Handle is busy.",
										L"Invalid alias.",
										L"Bad registry database.",
										L"Registry key not found.",
										L"Registry read error.",
										L"Registry write error.",
										L"Registry delete error.",
										L"Registry value not found.",
										L"Driver does not call DriverCallback.",
										L"More data to be returned."
									};

/*----------------------------------------------------------*\
| Error implementation
\*----------------------------------------------------------*/

Error::Error(int nCode, LPCWSTR pszFunctionName, ...):
			m_nCode(nCode)
{
	// Ignore any exceptions in the constructor

	try
	{
		va_list pArgList;
		va_start(pArgList, pszFunctionName);

		BuildDescription(SZ_ERRORS[m_nCode - SUCCESS],
			pszFunctionName, pArgList);

		va_end(pArgList);
	}

	catch(std::exception e) {}
}
			
Error::Error(va_list pArgList, int nCode, LPCWSTR pszFunctionName):
			m_nCode(nCode)
{
	// Ignore any exceptions in the constructor

	try
	{
		BuildDescription(SZ_ERRORS[m_nCode - SUCCESS],
			pszFunctionName, pArgList);
	}

	catch(std::exception e) {}
}

Error::Error(const Error& rInit)
{
	// Ignore any exceptions in the constructor

	try
	{
		m_nCode = rInit.m_nCode;
		m_strDescription = rInit.m_strDescription;
	}

	catch(std::exception e) {}
}

Error::Error(void): m_nCode(0),
					m_bIsOnStack(false)
					
{
}

int Error::GetCode(void) const
{
	return m_nCode;
}

const String& Error::GetDescription(void) const
{
	return m_strDescription;
}

const char* Error::what(void) const
{
	return ("ASCII error descriptions not supported.");
}

bool Error::IsOnStack(void) const
{
	return m_bIsOnStack;
}

Error& Error::operator=(const Error& rAssign)
{
	m_nCode = rAssign.m_nCode;
	m_strDescription = rAssign.m_strDescription;

	return *this;
}

void Error::BuildDescription(LPCWSTR pszUnformattedDescription,
							 LPCWSTR pszFunctionName,
							 va_list pArgList)
{
	// Get special flags if any

	bool bCOMError = false;
	bool bMMError = false;

	if (L'#' == *pszUnformattedDescription)
	{
		// If string starts with '#' character, add COM description footer

		bCOMError = true;
		++pszUnformattedDescription;
	}
	else if (L'@' == *pszUnformattedDescription)
	{
		// If string starts with '@' character, add multimedia description footer

		bMMError = true;
		++pszUnformattedDescription;
	}

	// Build header

	String strHeader;
	strHeader.Format(SZ_ERRORHEADER, m_nCode, pszFunctionName);

	// Build body

	String strBody;
	strBody.Format(pszUnformattedDescription, pArgList);

	// Build footer if required

	if (true == bCOMError)
	{
		// Get COM error code and description

		HRESULT hr = va_arg(pArgList, HRESULT);

		String strCOMFooter;
		strCOMFooter.Format(SZ_ERRORFOOTER_COM, hr);

		switch(HRESULT_FACILITY(hr))
		{
		case _FACD3D:
		case _FACDS:
			{				
				strCOMFooter += DXGetErrorDescription(hr);
			}
			break;
		default:
			{
				LPTSTR pszAutoBuffer = NULL;

				if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_ALLOCATE_BUFFER,
					NULL,
					hr,
					0,
					pszAutoBuffer,
					512,
					NULL) == 0)
				{
					strCOMFooter += L"Failed to get extended error description.";
				}
				else
				{
					strCOMFooter += pszAutoBuffer;
				}

				LocalFree(pszAutoBuffer);
			}
			break;
		}

		strBody += strCOMFooter;
	}

	if (true == bMMError)
	{
		// Get MM error code and description

		MMRESULT mmr = va_arg(pArgList, MMRESULT);

		String strMMFooter;
		strMMFooter.Format(SZ_ERRORFOOTER_MM, mmr);

		switch(mmr)
		{
		case MMSYSERR_NOERROR:
			strMMFooter += SZ_MM_ERRORS[0];
			break;
		default:
			strMMFooter += SZ_MM_ERRORS[mmr - MMSYSERR_BASE];
			break;
		}

		strBody += strMMFooter;
	}

	m_strDescription = strHeader + strBody;
}

/*----------------------------------------------------------*\
| ErrorManager implementation
\*----------------------------------------------------------*/

ErrorManager::ErrorManager(void):
	m_errSuccess(Error::SUCCESS, NULL, NULL)
{
}

ErrorManager::~ErrorManager(void)
{
	Empty();
}

const Error& ErrorManager::GetLastError(void) const
{
	return m_stackErrors.empty() == false ?
		m_stackErrors.top() : m_errSuccess;
}

Error& ErrorManager::Push(int nCode, LPCWSTR pszFunctionName, ...)
{
	va_list pArgList;
	va_start(pArgList, pszFunctionName);

	m_stackErrors.push(Error(pArgList, nCode, pszFunctionName));

	va_end(pArgList);

	m_stackErrors.top().m_bIsOnStack = true;

	return m_stackErrors.top();
}

Error& ErrorManager::Push(Error& rError)
{
	// Don't re-push if already in the stack

	if (true == rError.m_bIsOnStack)
		return rError;

	m_stackErrors.push(rError);

	m_stackErrors.top().m_bIsOnStack = true;

	return m_stackErrors.top();
}

void ErrorManager::Pop(void)
{
	if (m_stackErrors.empty() == false)
		m_stackErrors.pop();
}

int ErrorManager::GetCount(void) const
{
	return int(m_stackErrors.size());
}

void ErrorManager::Empty(void)
{
	while(m_stackErrors.size()) Pop();
}