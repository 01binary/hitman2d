/*------------------------------------------------------------------*\
|
| ThunderVertex.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm vertex class(es)
| Created: 10/04/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_VERTEX_H
#define THUNDER_VERTEX_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderMath.h"		// using Vector
#include "ThunderTexture.h"		// using Color

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Graphics;					// using Graphics

/*----------------------------------------------------------*\
| VertexTriangle class - vertex used in triangles
\*----------------------------------------------------------*/

class VertexTriangle
{
public:
	enum
	{
		PRIM_INDEX_COUNT = 3
	};

public:
	float x, y;
	D3DCOLOR clrBlend;
	float u, v;
};

/*----------------------------------------------------------*\
| VertexLine class - vertex used in lines and line strips
\*----------------------------------------------------------*/

class VertexLine
{
public:
	enum
	{
		PRIM_INDEX_COUNT = 2
	};

public:
	float x, y;
	D3DCOLOR clrBlend;
};

/*----------------------------------------------------------*\
| VertexPoint class - vertex used for points
\*----------------------------------------------------------*/

class VertexPoint
{
public:
	float x, y;
	D3DCOLOR clrBlend;
};

/*----------------------------------------------------------*\
| VertexParticle class - vertex used for point sprites
\*----------------------------------------------------------*/

class VertexParticle
{
public:
	float x, y, z;
	D3DCOLOR clrBlend;
	float fSize;
};

/*----------------------------------------------------------*\
| VertexCache class
\*----------------------------------------------------------*/

class VertexCache
{
private:
	UINT m_uSize;
	UINT m_uSizeFree;
	
	LPBYTE m_pVC;
	LPBYTE m_pVCPos;

public:
	VertexCache(void);
	~VertexCache(void);

public:
	inline UINT GetSize(void) const
	{
		return m_uSize;
	}

	inline UINT GetSizeFree(void) const
	{
		return m_uSizeFree;
	}
	
	inline LPBYTE GetCache(void) const
	{
		return m_pVC;
	}

	inline LPBYTE GetCurrentPos(void) const
	{
		return m_pVCPos;
	}

	void Create(UINT uSize);

	void Write(LPBYTE pbData, UINT uSize);

	void Reset(void);

	void Empty(void);
};

/*----------------------------------------------------------*\
| VertexBuffer class
\*----------------------------------------------------------*/

class VertexBuffer
{
private:
	UINT m_uVertexSize;
	UINT m_uBufferSize;
	UINT m_uUsedSize;
	UINT m_uUsedCount;

	DWORD m_dwUsage;

	LPDIRECT3DVERTEXBUFFER9 m_pBuffer;

public:
	VertexBuffer(UINT uVertexSize, DWORD dwUsage);
	~VertexBuffer(void);

public:
	inline LPDIRECT3DVERTEXBUFFER9 GetBuffer(void) const
	{
		return m_pBuffer;
	}

	inline UINT GetVertexSize(void) const
	{
		return m_uVertexSize;
	}

	inline UINT GetBufferSize(void) const
	{
		return m_uBufferSize;
	}

	inline UINT GetUsedSize(void) const
	{
		return m_uUsedSize;
	}

	inline UINT GetFreeSize(void) const
	{
		return m_uBufferSize - m_uUsedSize;
	}

	inline UINT GetUsedCount(void) const
	{
		return m_uUsedCount;
	}

	void Lock(UINT uSize, void** ppData, DWORD dwFlags);
	void Unlock(void);

	void Reset(void);

	void Create(Graphics& rGraphics, UINT uVertexCount);

	void Empty(void);
};

/*----------------------------------------------------------*\
| IndexBuffer class
\*----------------------------------------------------------*/

class IndexBuffer
{
private:
	UINT m_uIndexCount;	// Indices allocated
	UINT m_uUsedCount;	// Next available index for DIP call
	UINT m_uMinVertex;	// MinVertexIndex used in DIP call

	DWORD m_dwUsage;	// Usage saved for re-creation

	LPDIRECT3DINDEXBUFFER9 m_pBuffer;

public:
	IndexBuffer(DWORD dwUsage);
	~IndexBuffer(void);

public:
	inline LPDIRECT3DINDEXBUFFER9 GetBuffer(void)
	{
		return m_pBuffer;
	}

	inline UINT GetIndexCount(void) const
	{
		return m_uIndexCount;
	}

	inline UINT GetUsedCount(void) const
	{
		return m_uUsedCount;
	}

	inline UINT GetMinVertex(void) const
	{
		return m_uMinVertex;
	}

	void Lock(UINT uIndexCount, LPWORD* pwData, DWORD dwFlags);
	void Unlock(void);

	void Reserve(UINT uIndexCount, UINT uVertexCount)
	{
		m_uUsedCount += uIndexCount;
		m_uMinVertex += uVertexCount;
	}

	void Reset(void);

	void Create(Graphics& rGraphics, UINT uIndexCount);

	void Empty(void);
};

} // namespace ThunderStorm

#endif // THUNDER_VERTEX_H