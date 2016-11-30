/*------------------------------------------------------------------*\
|
| ThunderTexture.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine texture class
| Created: 04/09/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include <crtdbg.h>				// using ASSERT
#include "ThunderTexture.h"		// defining Texture
#include "ThunderEngine.h"		// using Engine
#include "ThunderRegion.h"		// using RegionSet
#include "ThunderInfoFile.h"	// using InfoFile/Elem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Texture implementation
\*----------------------------------------------------------*/

Texture::Texture(Engine& rEngine): Resource(rEngine),
								   m_pD3DTex(NULL)
{
}

Texture::~Texture()
{
	Empty();
}

void Texture::Allocate(int nWidth, int nHeight, D3DFORMAT nFormat)
{
	Empty();

	HRESULT hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
		UINT(nWidth), UINT(nHeight), 1, 0, nFormat, D3DPOOL_MANAGED, &m_pD3DTex);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_CREATETEXTURE,
			__FUNCTIONW__, hr);

	m_info.Width = UINT(nWidth);
	m_info.Height = UINT(nHeight);
	m_info.Depth = 1;
	m_info.ImageFileFormat = D3DXIFF_PNG;
	m_info.MipLevels = 1;
	m_info.ResourceType = D3DRTYPE_TEXTURE;
	m_info.Format = nFormat;

	//m_rEngine.PrintDebug(L"alloc texture %s 0x%x", !this->m_strName.IsEmpty() ? this->m_strName : "(null)", this);
}

void Texture::Lock(const RECT* prcLock, D3DLOCKED_RECT* pOutLockInfo, DWORD dwFlags)
{
	HRESULT hr = m_pD3DTex->LockRect(0, pOutLockInfo, prcLock, dwFlags);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_LOCKRECT,
			__FUNCTIONW__, hr);
}

void Texture::Unlock(void)
{
	HRESULT hr = m_pD3DTex->UnlockRect(0);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_UNLOCKRECT,
			__FUNCTIONW__, hr);
}

void Texture::Deserialize(LPCWSTR pszPath)
{
	Empty();
	
	HRESULT hr = 0;

	const D3DCAPS9& rCaps = m_rEngine.GetGraphics().GetDeviceCaps();

	if (rCaps.TextureCaps & D3DPTEXTURECAPS_POW2 &&
	   ~rCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
	{
		// Textures must be sized to powers of two.
		// Default D3DX behavior is to stretch the texture, but
		// we would rather have it cropped since this is a 2D engine

		D3DXIMAGE_INFO info;

		hr = D3DXGetImageInfoFromFile(pszPath, &info);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3DX_CREATETEXTUREFROMFILEEX,
				__FUNCTIONW__, pszPath, hr);

		UINT uWidth = Pow2(info.Width);
		UINT uHeight = Pow2(info.Height);

		if (uWidth != info.Width || uHeight != info.Height)
		{
			// Create a managed texture of this size and lock

			hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
					uWidth, uHeight, info.MipLevels, 0, info.Format,
					D3DPOOL_MANAGED, &m_pD3DTex);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3DX_CREATETEXTURE, __FUNCTIONW__,
					pszPath, hr);

			D3DLOCKED_RECT lrDest;
			
			hr = m_pD3DTex->LockRect(0, &lrDest, NULL, 0);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_TEXTURE_LOCKRECT, __FUNCTIONW__, hr);

			// Load texture in question and lock

			LPDIRECT3DTEXTURE9 pTempTex = NULL;

			hr = D3DXCreateTextureFromFileEx(
				m_rEngine.GetGraphics().GetDevice(),
				pszPath, info.Width, info.Height, 1, 0, D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0,
				&m_info, NULL, &pTempTex);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3DX_CREATETEXTUREFROMFILEEX,
					__FUNCTIONW__, pszPath, hr);

			D3DLOCKED_RECT lrSrc;
			
			hr = pTempTex->LockRect(0, &lrSrc, NULL, 0);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_TEXTURE_LOCKRECT, __FUNCTIONW__, hr);

			// Copy with cropping			

			LPBYTE pbDest = LPBYTE(lrDest.pBits);
			LPBYTE pbSrc = LPBYTE(lrSrc.pBits);

			UINT uZeroFill = lrDest.Pitch - lrSrc.Pitch;

			for(UINT y = 0;
				y < info.Height;
				y++, pbDest += lrDest.Pitch, pbSrc += lrSrc.Pitch)
			{
				CopyMemory(pbDest, pbSrc, lrSrc.Pitch);
				ZeroMemory(pbDest + lrSrc.Pitch, uZeroFill);
			}

			// Clean up			

			hr = m_pD3DTex->UnlockRect(0);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_TEXTURE_UNLOCKRECT, __FUNCTIONW__, hr);

			hr = pTempTex->UnlockRect(0);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3D_TEXTURE_UNLOCKRECT, __FUNCTIONW__, hr);

			pTempTex->Release();

			// Set info

			m_info.Width = uWidth;
			m_info.Height = uHeight;

			return;
		}
	}

	hr = D3DXCreateTextureFromFileEx(
		m_rEngine.GetGraphics().GetDevice(),
		pszPath, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0,
		&m_info, NULL, &m_pD3DTex);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::D3DX_CREATETEXTUREFROMFILEEX,
			__FUNCTIONW__, pszPath, hr);

	/*if (pszPath != NULL)
		m_rEngine.PrintDebug(L"load texture %s 0x%x", pszPath, this);*/
}

void Texture::Serialize(LPCWSTR pszPath) const
{
	HRESULT hr = D3DXSaveTextureToFile(pszPath, GetFormatFromPath(pszPath),
		m_pD3DTex, NULL);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_SAVESURFACETOFILE,
			__FUNCTIONW__, hr);
}

D3DXIMAGE_FILEFORMAT Texture::GetFormatFromPath(LPCWSTR pszPath)
{
	LPCWSTR pszExt = PathFindExtension(pszPath);

	if (NULL == pszExt)
		return (D3DXIMAGE_FILEFORMAT)-1;

	if (wcscmp(pszExt, L".bmp") == 0)
	{
		return D3DXIFF_BMP;
	}
	else if (wcscmp(pszExt, L".jpg") == 0)
	{
		return D3DXIFF_JPG;
	}
	else if (wcscmp(pszExt, L".tga") == 0)
	{
		return D3DXIFF_TGA;
	}
	else if (wcscmp(pszExt, L".png") == 0)
	{
		return D3DXIFF_PNG;
	}
	else if (wcscmp(pszExt, L".dds") == 0)
	{
		return D3DXIFF_DDS;
	}

	return (D3DXIMAGE_FILEFORMAT)-1;
}

void Texture::OnLostDevice(bool bRecreate)
{
	if (true == bRecreate)
	{
		Empty();
	}
	else
	{
		// Default handler does nothing (texture is in managed pool)
	}
}

void Texture::OnResetDevice(bool bRecreate)
{
	if (true == bRecreate)
	{
		if (IsFlagSet(NORELOAD) == false)
			Reload();
	}
	else
	{
		// Default handler does nothing (texture is in managed pool)
	}
}

DWORD Texture::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() -
				   sizeof(Resource) +
				   sizeof(Texture);

	DWORD dwBufSize = m_info.Width * m_info.Height;

	if (D3DFMT_X8R8G8B8 == m_info.Format ||
		D3DFMT_A8R8G8B8 == m_info.Format)
		dwBufSize *= sizeof(DWORD);
	else if (D3DFMT_R5G6B5 == m_info.Format ||
		D3DFMT_A1R5G5B5 == m_info.Format)
		dwBufSize *= sizeof(WORD);

	return dwSize + dwBufSize;
}

void Texture::Empty(void)
{
	SAFERELEASE(m_pD3DTex);
}

void Texture::Remove(void)
{
	m_rEngine.GetTextures().Remove(this);
}

/*----------------------------------------------------------*\
| TextureDynamic implementation
\*----------------------------------------------------------*/

TextureDynamic::TextureDynamic(Engine& rEngine): Texture(rEngine),
												 m_pBuffer(NULL),
												 m_pSurf(NULL),
												 m_hDC(NULL),
												 m_bLocked(true)
{
}

TextureDynamic::~TextureDynamic(void)
{
	Empty();
}

void TextureDynamic::Allocate(int nWidth, int nHeight)
{
	Empty();

	HRESULT hr = 0;

	if (m_rEngine.GetGraphics().GetDeviceCaps().Caps2 &
	   D3DCAPS2_DYNAMICTEXTURES)
	{
		// Device supports dynamic textures

		hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
			UINT(nWidth), UINT(nHeight), 1, D3DUSAGE_DYNAMIC,
			m_rEngine.GetGraphics().GetDeviceParams().BackBufferFormat,
			D3DPOOL_DEFAULT, &m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3DX_CREATETEXTUREFROMFILEEX, __FUNCTIONW__, hr);
	}
	else
	{
		// Dynamic textures not supported - have to create system memory copy

		hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
			UINT(nWidth), INT(nHeight), 1, 0,
			m_rEngine.GetGraphics().GetDeviceParams().BackBufferFormat,
			D3DPOOL_SYSTEMMEM, &m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::D3DX_CREATETEXTUREFROMFILEEX, __FUNCTIONW__, hr);

		// Then create a default pool copy

		hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
			UINT(nWidth), UINT(nHeight), 1, 0,
			m_rEngine.GetGraphics().GetDeviceParams().BackBufferFormat,
			D3DPOOL_DEFAULT, &m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_UPDATETEXTURE,
				__FUNCTIONW__, hr);
	}
}

void TextureDynamic::OnLostDevice(bool bRecreate)
{
	// Always release (we are in default pool)

	Empty();
}

void TextureDynamic::OnResetDevice(bool bRecreate)
{
	// Always recreate (we are in default pool)

	if (IsFlagSet(NORELOAD) == false)
		Reload();
}

void TextureDynamic::Lock(const RECT* prcLock, D3DLOCKED_RECT* pOutLockInfo)
{
	if (true == m_bLocked)
		return;

	// Should not lock bits while capturing DC

	if (m_hDC != NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	HRESULT hr = 0;
	
	if (m_pBuffer != NULL)
	{
		// Lock system memory texture if dynamic textures not supported

		hr = m_pBuffer->LockRect(0, pOutLockInfo, prcLock, D3DLOCK_DISCARD);
	}
	else
	{
		// Lock dynamic texture

		hr = m_pD3DTex->LockRect(0, pOutLockInfo, prcLock, D3DLOCK_DISCARD);
	}

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_SURFACE_LOCKRECT,
			__FUNCTIONW__, hr);
}

void TextureDynamic::Unlock(bool bUpdate)
{
	if (false == m_bLocked) return;

	HRESULT hr = 0;

	if (m_pBuffer != NULL)
	{
		// Unlock system memory texture and update static texture

		hr = m_pBuffer->UnlockRect(0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_SURFACE_UNLOCKRECT,
				__FUNCTIONW__, hr);

		if (true == bUpdate)
		{
			hr = m_rEngine.GetGraphics().GetDevice()->UpdateTexture(
				m_pBuffer, m_pD3DTex);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_UPDATETEXTURE,
					__FUNCTIONW__, hr);
		}
	}
	else
	{
		// Unlock dynamic texture

		hr = m_pD3DTex->UnlockRect(0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_SURFACE_UNLOCKRECT,
				__FUNCTIONW__, hr);
	}

	m_bLocked = false;
}

HDC TextureDynamic::GetDC(void)
{
	// Should not get DC when locked bits

	if (true == m_bLocked)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	HRESULT hr = 0;

	if (m_pBuffer != NULL)
	{
		// Dynamic texture not supported, get system memory surface

		hr = m_pBuffer->GetSurfaceLevel(0, &m_pSurf);
	}
	else
	{
		// Dynamic textures supported, get default memory surface

		hr = m_pD3DTex->GetSurfaceLevel(0, &m_pSurf);
	}

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_GETSURFACELEVEL,
			__FUNCTIONW__, hr);

	// Capture DC from surface

	hr = m_pSurf->GetDC(&m_hDC);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_SURFACE_GETDC,
			__FUNCTIONW__, hr);

	// Return captured DC

	return m_hDC;
}

void TextureDynamic::ReleaseDC(bool bUpdate)
{
	if (NULL == m_hDC) return;

	// Release captured DC

	HRESULT hr = m_pSurf->ReleaseDC(m_hDC);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_SURFACE_RELEASEDC,
			__FUNCTIONW__, hr);

	m_hDC = NULL;

	// Release captured surface

	m_pSurf->Release();
	m_pSurf = NULL;

	// If dynamic textures not supported, update the default memory texture

	if (m_pBuffer != NULL)
	{
		hr = m_pD3DTex->AddDirtyRect(NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_ADDDIRTYRECT,
				__FUNCTIONW__, hr);

		if (true == bUpdate)
		{
			hr = m_rEngine.GetGraphics().GetDevice()->UpdateTexture(
				m_pBuffer, m_pD3DTex);

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_UPDATETEXTURE,
					__FUNCTIONW__, hr);
		}
	}
}

void TextureDynamic::Deserialize(LPCWSTR pszPath)
{
	Empty();

	HRESULT hr = 0;

	if (m_rEngine.GetGraphics().GetDeviceCaps().Caps2 &
	   D3DCAPS2_DYNAMICTEXTURES)
	{
		// Device supports dynamic textures

		hr = D3DXCreateTextureFromFileEx(m_rEngine.GetGraphics().GetDevice(),
			pszPath, D3DX_DEFAULT, D3DX_DEFAULT, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0,
			&m_info, NULL, &m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3DX_CREATETEXTUREFROMFILEEX,
				__FUNCTIONW__, hr);
	}
	else
	{
		// Dynamic textures not supported - have to create system memory copy

		hr = D3DXCreateTextureFromFileEx(m_rEngine.GetGraphics().GetDevice(),
			pszPath, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN,
			D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT, 0,
			&m_info, NULL, &m_pBuffer);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3DX_CREATETEXTUREFROMFILEEX,
				__FUNCTIONW__, hr);

		// Then create a default pool copy

		hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
			m_info.Width, m_info.Height, m_info.MipLevels, 0,
			m_info.Format, D3DPOOL_DEFAULT, &m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_CREATETEXTURE,
			__FUNCTIONW__, hr);

		// Update default pool texture for the first time

		hr = m_pD3DTex->AddDirtyRect(NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_ADDDIRTYRECT,
				__FUNCTIONW__, hr);

		hr = m_rEngine.GetGraphics().GetDevice()->UpdateTexture(
			m_pBuffer, m_pD3DTex);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_UPDATETEXTURE,
			__FUNCTIONW__, hr);
	}
}

void TextureDynamic::Empty(void)
{
	// Unlock / release captured DC

	if (true == m_bLocked)
	{
		Unlock(false);
	}
	else if (m_hDC != NULL)
	{
		ReleaseDC(false);
	}

	// Clean up system memory copy

	if (m_pBuffer != NULL)
		m_pBuffer->Release();

	// Clean up the base texture

	Texture::Empty();
}

/*----------------------------------------------------------*\
| TextureRenderTarget implementation
\*----------------------------------------------------------*/

TextureRenderTarget::TextureRenderTarget(Engine& rEngine): Texture(rEngine),
														   m_pSurf(NULL),
														   m_pOldSurf(NULL)
{
}

TextureRenderTarget::~TextureRenderTarget(void)
{
	Empty();
}

void TextureRenderTarget::Allocate(int nWidth, int nHeight, bool bAlphaChannel)
{
	Empty();

	m_info.Format =
		m_rEngine.GetGraphics().GetDeviceParams().BackBufferFormat;

	if (true == bAlphaChannel)
	{
		if (D3DFMT_X8R8G8B8 == m_info.Format)
			m_info.Format = D3DFMT_A8R8G8B8;
		else if (D3DFMT_R5G6B5 == m_info.Format)
			m_info.Format = D3DFMT_A4R4G4B4;
	}

	m_info.Depth = 1;
	m_info.Width = UINT(nWidth);
	m_info.Height = UINT(nHeight);
	m_info.ImageFileFormat = D3DXIFF_FORCE_DWORD;
	m_info.MipLevels = 1;
	m_info.ResourceType = D3DRTYPE_TEXTURE;

	OnResetDevice(false);
}

void TextureRenderTarget::BeginScene(void)
{
	if (m_pSurf != NULL)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	// Get old surface

	HRESULT hr =
		m_rEngine.GetGraphics().GetDevice()->GetRenderTarget(0, &m_pOldSurf);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_GETRENDERTARGET,
			__FUNCTIONW__, hr);

	// Get surface

	hr = m_pD3DTex->GetSurfaceLevel(0, &m_pSurf);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_GETSURFACELEVEL,
			__FUNCTIONW__, hr);

	// Set surface as render target

	hr = m_rEngine.GetGraphics().GetDevice()->SetRenderTarget(0, m_pSurf);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETRENDERTARGET,
			__FUNCTIONW__, hr);

	// Save matrices

	m_mtxOldWorld = m_rEngine.GetGraphics().GetStates()->GetTransform(D3DTS_WORLD);
	m_mtxOldView = m_rEngine.GetGraphics().GetStates()->GetTransform(D3DTS_VIEW);
	m_mtxOldProj = m_rEngine.GetGraphics().GetStates()->GetTransform(D3DTS_PROJECTION);

	// Set new projection matrix to match render target

	m_rEngine.GetGraphics().GetStates()->SetTransformOrthoProjection(float(m_info.Width),
		float(m_info.Height));

	// Set blank world & view matrices

	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_WORLD, NULL);
	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_VIEW, NULL);

	// Begin scene
	
	m_rEngine.GetGraphics().BeginScene();
}

void TextureRenderTarget::EndScene(void)
{
	if (NULL == m_pSurf || NULL == m_pOldSurf)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	// End scene

	m_rEngine.GetGraphics().EndScene();

	// Cleanup

	m_rEngine.GetGraphics().GetDevice()->SetRenderTarget(0, m_pOldSurf);
	m_pOldSurf->Release();
	m_pOldSurf = NULL;

	m_pSurf->Release();
	m_pSurf = NULL;

	// Restore previous matrices

	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_WORLD, &m_mtxOldWorld);
	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_VIEW, &m_mtxOldView);
	m_rEngine.GetGraphics().GetStates()->SetTransform(D3DTS_PROJECTION, &m_mtxOldProj);
}

void TextureRenderTarget::OnLostDevice(bool bRecreate)
{
	Empty();
}

void TextureRenderTarget::OnResetDevice(bool bRecreate)
{
	// Always re-create, because we are in video memory pool

	SAFERELEASE(m_pD3DTex);

	HRESULT hr = D3DXCreateTexture(m_rEngine.GetGraphics().GetDevice(),
		m_info.Width, m_info.Height, 1, D3DUSAGE_RENDERTARGET,
		m_info.Format, D3DPOOL_DEFAULT, &m_pD3DTex);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_CREATETEXTURE,
			__FUNCTIONW__, hr);
}

void TextureRenderTarget::Deserialize(LPCWSTR pszPath)
{
	// Render targets do not support serialization from file

	UNREFERENCED_PARAMETER(pszPath);

	throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);
}

void TextureRenderTarget::Empty(void)
{
	if (m_pOldSurf != NULL)
	{
		if (m_rEngine.GetGraphics().GetDevice() != NULL)
			m_rEngine.GetGraphics().GetDevice()->SetRenderTarget(0, m_pOldSurf);

		m_pOldSurf->Release();
		m_pOldSurf = NULL;

		if (m_pSurf != NULL)
		{
			m_pSurf->Release();
			m_pSurf = NULL;
		}
	}

	Texture::Empty();
}

/*----------------------------------------------------------*\
| TextureCube implementation
\*----------------------------------------------------------*/

TextureCube::TextureCube(Engine& rEngine): Texture(rEngine)
{
}

void TextureCube::Deserialize(LPCWSTR pszPath)
{
	Empty();

	// Only one mip level loaded

	HRESULT hr = D3DXCreateCubeTextureFromFileEx(
		m_rEngine.GetGraphics().GetDevice(),
		pszPath, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0,
		&m_info, NULL,
		reinterpret_cast<LPDIRECT3DCUBETEXTURE9*>(&m_pD3DTex));

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_CREATECUBETEXTUREFROMFILEEX,
			__FUNCTIONW__, pszPath, hr);
}

DWORD TextureCube::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(TextureCube);

	DWORD dwBufSize = m_info.Width * m_info.Height * 6; // 6 faces of cube

	if (D3DFMT_X8R8G8B8 == m_info.Format ||
		D3DFMT_A8R8G8B8 == m_info.Format)
		dwBufSize *= sizeof(DWORD);
	else if (D3DFMT_R5G6B5 == m_info.Format ||
		D3DFMT_A1R5G5B5 == m_info.Format)
		dwBufSize *= sizeof(WORD);

	return dwSize + dwBufSize;
}

void TextureCube::Remove(void)
{
	m_rEngine.GetCubeTextures().Remove(this);
}

/*----------------------------------------------------------*\
| Color implementation
\*----------------------------------------------------------*/

Color::Color(D3DCOLOR clrInit): m_clrColor(clrInit)
{
}

Color::Color(int nR, int nG, int nB, int nA):
			 m_clrColor(D3DCOLOR_RGBA(nR, nG, nB, nA))
{
}

Color::Color(float fR, float fG, float fB, float fA):
			 m_clrColor(D3DCOLOR_COLORVALUE(fR, fG, fB, fA))
{
}

Color::Color(const Color& clrInit): m_clrColor(clrInit.m_clrColor)
{
}

Color::Color(const D3DXCOLOR& rInit)			 
{
	m_clrColor = D3DCOLOR_ARGB(BYTE(rInit.a * 255.0f),
		BYTE(rInit.r * 255.0f), BYTE(rInit.g * 255.0f),
		BYTE(rInit.b * 255.0f));
}

Color Color::ToGrayscaleAverage(void) const
{
	int nValue = (GetR() + GetG() + GetB()) / 3;

	return Color(nValue, nValue, nValue, GetAlpha());
}

Color Color::ToGrayscaleWeight(float fWeightR,
							   float fWeightG,
							   float fWeightB) const
{
	int nValue = int(GetRFloat() * fWeightR +
		GetGFloat() * fWeightG +
		GetBFloat() * fWeightB);

	return Color(nValue, nValue, nValue, GetAlpha());
}

void Color::Serialize(InfoElem& rRoot) const
{
	if (rRoot.GetElemType() == InfoElem::TYPE_VALUE)
	{
		rRoot.SetDwordValue(m_clrColor);
	}
	else if (rRoot.GetElemType() == InfoElem::TYPE_VALUELIST)
	{
		int nRGBA[] = { m_byChannels[CHANNEL_RED],
			m_byChannels[CHANNEL_GREEN],
			m_byChannels[CHANNEL_BLUE],
			m_byChannels[CHANNEL_ALPHA] };

		if (m_byChannels[CHANNEL_ALPHA] == 0xFF)
			rRoot.FromIntArray(nRGBA, 3);
		else
			rRoot.FromIntArray(nRGBA, 4);
	}
}

void Color::Deserialize(const InfoElem& rRoot)
{
	if (rRoot.GetElemType() == InfoElem::TYPE_VALUE)
	{
		if (rRoot.GetVarType() == Variable::TYPE_DWORD)
			m_clrColor = rRoot.GetDwordValue();
		else
			throw Error(Error::FILE_ELEMENTFORMAT,
				__FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
				Variable::GetVarTypeString(Variable::TYPE_DWORD));
	}
	else if (rRoot.GetElemType() == InfoElem::TYPE_VALUELIST)
	{
		if (rRoot.GetChildConst(0)->GetVarType() == InfoElem::TYPE_FLOAT)
		{
			float fRGBA[] = { 255.0f, 255.0f, 255.0f, 255.0f };

			int nCount = rRoot.ToFloatArray(fRGBA, 4);

			if (nCount > 2)
			{
				m_clrColor = D3DCOLOR_COLORVALUE(fRGBA[0],
					fRGBA[1], fRGBA[2], fRGBA[3]);
			}
			else
			{
				throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
					Variable::GetVarTypeString(Variable::TYPE_FLOAT));
			}
		}
		else if (rRoot.GetChildConst(0)->GetVarType() == InfoElem::TYPE_INT)
		{
			int nRGBA[] = { 255, 255, 255, 255 };

			int nCount = rRoot.ToIntArray(nRGBA, 4);

			if (3 == nCount) // RGB
			{
				m_clrColor = D3DCOLOR_XRGB(nRGBA[0],
					nRGBA[1], nRGBA[2]);
			}
			else if (4 == nCount) // RGBA to ARGB
			{
				m_clrColor = D3DCOLOR_ARGB(nRGBA[3],
					nRGBA[0], nRGBA[1], nRGBA[2]);
			}
			else
			{
				throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
					Variable::GetVarTypeString(Variable::TYPE_DWORD));
			}
		}
		else if (rRoot.GetChildConst(0)->GetVarType() == InfoElem::TYPE_DWORD)
		{
			DWORD dwRGBA[] = { 0xFF, 0xFF, 0xFF, 0xFF };

			int nCount = rRoot.ToDwordArray(dwRGBA, 4);

			if (3 == nCount) // RGB
			{
				m_clrColor = D3DCOLOR_XRGB(dwRGBA[0],
					dwRGBA[1], dwRGBA[2]);
			}
			else if (4 == nCount) // RGBA to ARGB
			{
				m_clrColor = D3DCOLOR_ARGB(dwRGBA[3],
					dwRGBA[0], dwRGBA[1], dwRGBA[2]);
			}
			else
			{
				throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
					Variable::GetVarTypeString(Variable::TYPE_DWORD));
			}
		}
		else
		{
			throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
				rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
				Variable::GetVarTypeString(Variable::TYPE_DWORD));
		}
	}
	else
	{
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetDocumentConst().GetPath(), rRoot.GetName(),
			Variable::GetVarTypeString(Variable::TYPE_DWORD));
	}
}

Color& Color::operator=(const Color& rAssign)
{
	m_clrColor = rAssign.m_clrColor;

	return *this;
}

Color& Color::operator=(D3DCOLOR clrAssign)
{
	m_clrColor = clrAssign;

	return *this;
}

Color& Color::operator+=(const Color& rAdd)
{
	m_byChannels[CHANNEL_ALPHA] = BYTE(min(m_byChannels[CHANNEL_ALPHA] +
		rAdd.m_byChannels[CHANNEL_ALPHA], 255));

	m_byChannels[CHANNEL_RED] = BYTE(min(m_byChannels[CHANNEL_RED] +
		rAdd.m_byChannels[CHANNEL_RED], 255));

	m_byChannels[CHANNEL_GREEN] = BYTE(min(m_byChannels[CHANNEL_GREEN] +
		rAdd.m_byChannels[CHANNEL_GREEN], 255));

	m_byChannels[CHANNEL_BLUE] = BYTE(min(m_byChannels[CHANNEL_BLUE] +
		rAdd.m_byChannels[CHANNEL_BLUE], 255));

	return *this;
};	

Color& Color::operator-=(const Color& rSub)
{
	m_byChannels[CHANNEL_ALPHA] = BYTE(max(m_byChannels[CHANNEL_ALPHA] -
		rSub.m_byChannels[CHANNEL_ALPHA], 0));

	m_byChannels[CHANNEL_RED] = BYTE(max(m_byChannels[CHANNEL_RED] -
		rSub.m_byChannels[CHANNEL_RED], 0));

	m_byChannels[CHANNEL_GREEN] = BYTE(max(m_byChannels[CHANNEL_GREEN] -
		rSub.m_byChannels[CHANNEL_GREEN], 0));

	m_byChannels[CHANNEL_BLUE] = BYTE(max(m_byChannels[CHANNEL_BLUE] -
		rSub.m_byChannels[CHANNEL_BLUE], 0));

	return *this;
}

Color& Color::operator*=(const Color& rMul)
{
	m_byChannels[CHANNEL_ALPHA] = BYTE(max(min(GetAFloat() *
		rMul.GetAFloat() * 255.0f, 255), 0));

	m_byChannels[CHANNEL_RED] = BYTE(max(min(GetRFloat() *
		rMul.GetRFloat() * 255.0f, 255), 0));

	m_byChannels[CHANNEL_GREEN] = BYTE(max(min(GetGFloat() *
		rMul.GetGFloat() * 255.0f, 255), 0));

	m_byChannels[CHANNEL_BLUE] = BYTE(max(min(GetBFloat() *
		rMul.GetBFloat() * 255.0f, 255), 0));

	return *this;
}

Color Color::operator+(const Color& rAdd) const
{
	return Color(min(GetR() + rAdd.GetR(), 255),
		min(GetG() + rAdd.GetG(), 255),
		min(GetB() + rAdd.GetB(), 255),
		min(GetAlpha() + rAdd.GetAlpha(), 255));
}

Color Color::operator-(const Color& rSub) const
{
	return Color(max(GetR() - rSub.GetR(), 0),
		max(GetG() - rSub.GetG(), 0),
		max(GetB() - rSub.GetB(), 0),
		max(GetAlpha() - rSub.GetAlpha(), 0));
}

Color Color::operator*(const Color& rMul) const
{
	return Color(max(min(int(GetRFloat() *
		rMul.GetRFloat() * 255.0f), 255), 0),
		max(min(int(GetGFloat() * 
		rMul.GetGFloat() * 255.0f), 255), 0),
		max(min(int(GetBFloat() *
		rMul.GetBFloat() * 255.0f), 255), 0),
		max(min(int(GetAFloat() *
		rMul.GetAFloat() * 255.0f), 255), 0));
}