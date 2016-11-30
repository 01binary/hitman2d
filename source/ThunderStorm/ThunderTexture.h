/*------------------------------------------------------------------*\
|
| ThunderTexture.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine texture class
| Created: 04/09/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_TEXTURE_H
#define THUNDER_TEXTURE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource
#include "ThunderStream.h"		// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Texture;				// referencing Texture, declared below
class RegionSet;			// referencing RegionSet
class Region;				// referencing Region
class InfoElem;				// referencing InfoElem
class ErrorManager;			// referencing ErrorManager

/*----------------------------------------------------------*\
| Macros
\*----------------------------------------------------------*/

#define COLOR8_A1R5G5B5(a,r,g,b) ( \
		((a > 0 ? 1 : 0) << 15) | \
		((((r + 4) >> 3) > 0x1F ? 0x1F : ((r + 4) >> 3)) << 10) | \
		((((g + 4) >> 3) > 0x1F ? 0x1F : ((g + 4) >> 3)) << 5) | \
		(((b + 4) >> 3) > 0x1F ? 0x1F : ((b + 4) >> 3))

#define COLOR5_A1R5G5B5(a,r,g,b) ( \
		(a << 15) | (b << 10) | (g << 5) | r)

#define COLOR5_A4R4G4B4(a,r,g,b) ( \
		(a << 12) | (b << 8) | (g << 4) | r)

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Texture*> TextureArray;
typedef std::vector<Texture*>::iterator TextureArrayIterator;
typedef std::vector<Texture*>::const_iterator TextureArrayConstIterator;


/*----------------------------------------------------------*\
| Texture class - static texture resource
\*----------------------------------------------------------*/

class Texture: public Resource
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// Do not attempt to reload when re-creating device
		NORELOAD = 1
	};

protected:
	//
	// Members
	//

	// D3D Texture instance
	LPDIRECT3DTEXTURE9 m_pD3DTex;

	// D3D Texture descriptor
	D3DXIMAGE_INFO m_info;

public:
	Texture(Engine& rEngine);
	virtual ~Texture(void);

public:
	//
	// Info
	//

	inline LPDIRECT3DTEXTURE9 GetD3DTexture(void)
	{
		return m_pD3DTex;
	}

	inline const D3DXIMAGE_INFO& GetInfo(void) const
	{
		return m_info;
	}

	//
	// Allocation
	//

	void Allocate(int nWidth, int nHeight, D3DFORMAT nFormat);

	//
	// Access to bits
	//

	void Lock(const RECT* prcLock, D3DLOCKED_RECT* pOutLockInfo, DWORD dwFlags);
	void Unlock(void);

	//
	// Device Events
	//
	
	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Serialize(LPCWSTR pszPath) const;

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);

	//
	// Static Functions
	//

	static D3DXIMAGE_FILEFORMAT GetFormatFromPath(LPCWSTR pszPath);
};

/*----------------------------------------------------------*\
| TextureDynamic class
\*----------------------------------------------------------*/

class TextureDynamic: public Texture
{
protected:
	LPDIRECT3DTEXTURE9 m_pBuffer;	// If dynamic textures are not supported, system memory backup

	LPDIRECT3DSURFACE9 m_pSurf;		// Surface used for getting device context
	HDC m_hDC;						// Device context

	bool m_bLocked;					// Currently locked?

public:
	TextureDynamic(Engine& rEngine);
	virtual ~TextureDynamic(void);

public:
	//
	// Allocation
	//

	void Allocate(int nWidth, int nHeight);

	//
	// Access to bits
	//

	void Lock(const RECT* prcLock, D3DLOCKED_RECT* pOutLockInfo);
	void Unlock(bool bUpdate = true);

	//
	// Access to DC
	//

	HDC GetDC(void);
	void ReleaseDC(bool bUpdate = true);

	//
	// Device Events
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Remove(void) {};

	//
	// Deinitialization
	//

	virtual void Empty(void);
};

/*----------------------------------------------------------*\
| TextureRenderTarget class
\*----------------------------------------------------------*/

class TextureRenderTarget: public Texture
{
protected:
	LPDIRECT3DSURFACE9 m_pSurf;		// Render target surface, if set as render target
	LPDIRECT3DSURFACE9 m_pOldSurf;	// Old render target surface, if set as render target

	D3DXMATRIX m_mtxOldWorld;		// Previous world transform
	D3DXMATRIX m_mtxOldView;		// Previous view projection
	D3DXMATRIX m_mtxOldProj;		// Previous projection transform

public:
	TextureRenderTarget(Engine& rEngine);
	virtual ~TextureRenderTarget(void);

public:
	//
	// Allocation
	//

	void Allocate(int nWidth, int nHeight, bool bAlphaChannel = false);

	//
	// Rendering
	//

	void BeginScene(void);
	void EndScene(void);

	//
	// Device Events
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void) {};
};

/*----------------------------------------------------------*\
| TextureCube class
\*----------------------------------------------------------*/

class TextureCube: public Texture
{
public:
	TextureCube(Engine& rEngine);

public:
	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Remove(void);
};

/*----------------------------------------------------------*\
| Color class
\*----------------------------------------------------------*/

class Color
{
public:
	//
	// Constants
	//

	// Color channel byte indices

	enum Channels		
	{
		CHANNEL_BLUE,		
		CHANNEL_GREEN,
		CHANNEL_RED,
		CHANNEL_ALPHA
	};

	static const D3DCOLOR BLEND_ONE = 0xFFFFFFFF;
	static const D3DCOLOR BLEND_ZERO = 0xFF000000;

	static const int MIN_CHANNEL = 0;
	static const int MAX_CHANNEL = 0xFF;

private:
	//
	// Members
	//

	union
	{
		D3DCOLOR m_clrColor;
		BYTE m_byChannels[4];
	};

public:
	Color(D3DCOLOR clrInit = 0xFF000000);
	Color(int nR, int nG, int nB, int nA = 255);
	Color(float fR, float fG, float fB, float fA = 1.0f);
	Color(const Color& clrInit);
	Color(const D3DXCOLOR& clrInit);

public:
	//
	// Value
	//

	inline int GetR(void) const
	{
		return int((m_clrColor & 0x00FF0000) >> 16);
	}

	inline float GetRFloat(void) const
	{
		return float((m_clrColor & 0x00FF0000) >> 16) / 255.0f;
	}

	inline int GetG(void) const
	{
		return int((m_clrColor & 0x0000FF00) >> 8);
	}

	inline float GetGFloat(void) const
	{
		return float((m_clrColor & 0x0000FF00) >> 8) / 255.0f;
	}

	inline int GetB(void) const
	{
		return int(m_clrColor & 0x000000FF);
	}

	inline float GetBFloat(void) const
	{
		return float(m_clrColor & 0x000000FF) / 255.0f;
	}

	inline int GetAlpha(void) const
	{
		return int((m_clrColor & 0xFF000000) >> 24);
	}

	inline float GetAFloat(void) const
	{
		return float((m_clrColor & 0xFF000000) >> 24) / 255.0f;
	}

	inline int GetChannel(Channels nIndex) const
	{
		return int(m_byChannels[nIndex]);
	}

	inline float GetChannelFloat(Channels nIndex) const
	{
		return float(m_byChannels[nIndex]) / 255.0f;
	}

	inline void SetR(int nR)
	{
		m_byChannels[CHANNEL_RED] = BYTE(nR);
	}

	inline void SetR(float fR)
	{
		m_byChannels[CHANNEL_RED] = BYTE(fR * 255.0f);
	}

	inline void SetG(int nG)
	{
		m_byChannels[CHANNEL_GREEN] = BYTE(nG);
	}

	inline void SetG(float fG)
	{
		m_byChannels[CHANNEL_GREEN] = BYTE(fG * 255.0f);
	}

	inline void SetB(int nB)
	{
		m_byChannels[CHANNEL_BLUE] = BYTE(nB);
	}

	inline void SetB(float fB)
	{
		m_byChannels[CHANNEL_BLUE] = BYTE(fB * 255.0f);
	}

	inline void SetAlpha(int nAlpha)
	{
		m_byChannels[CHANNEL_ALPHA] = BYTE(nAlpha);
	}

	inline void SetAlpha(float fAlpha)
	{
		m_byChannels[CHANNEL_ALPHA] = BYTE(fAlpha * 255.0f);
	}

	inline void SetChannel(Channels nIndex, int nValue)
	{
		m_byChannels[nIndex] = BYTE(nValue);
	}

	inline void SetChannel(Channels nIndex, float fValue)
	{
		m_byChannels[nIndex] = BYTE(fValue * 255.0f);
	}

	void Set(int nR, int nG, int nB, int nA = 255)
	{
		m_clrColor = D3DCOLOR_RGBA(nR, nG, nB, nA);
	}

	void Set(float fR, float fG, float fB, float fA = 1.0f)
	{
		m_clrColor = D3DCOLOR_COLORVALUE(fR, fG, fB, fA);
	}

	//
	// Operations
	//

	Color ToGrayscaleAverage(void) const;
	Color ToGrayscaleWeight(float fWeightR = 0.222f,
		float fWeightG = 0.707f, float fWeightB = 0.071f) const;

	//
	// Serialization
	//

	inline void Serialize(Stream& rStream) const
	{
		rStream.WriteVar(&m_clrColor);
	}

	inline void Deserialize(Stream& rStream)
	{
		rStream.ReadVar(&m_clrColor);
	}

	void Serialize(InfoElem& rRoot) const;
	void Deserialize(const InfoElem& rRoot);

	//
	// Operators
	//

	inline int operator [] (int nIndex) const
	{
		return int(m_byChannels[nIndex]);
	}

	inline operator D3DCOLOR(void) const
	{
		return m_clrColor;
	}

	inline operator D3DXCOLOR(void) const
	{
		return D3DXCOLOR(m_clrColor);
	}

	Color& operator=(const Color& rAssign);
	Color& operator=(D3DCOLOR clrAssign);
	Color& operator+=(const Color& rAdd);
	Color& operator-=(const Color& rSub);
	Color& operator*=(const Color& rMul);

	Color operator+(const Color& rAdd) const;
	Color operator-(const Color& rSub) const;
	Color operator*(const Color& rMul) const;
};

} // namespace ThunderStorm

#endif // THUNDER_TEXTURE_H