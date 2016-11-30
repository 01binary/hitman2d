/*------------------------------------------------------------------*\
|
| ThunderVertex.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm vertex class(es) implementation
| Created: 10/04/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderGraphics.h"	// using Graphics
#include "ThunderError.h"		// using Error
#include "ThunderGlobals.h"		// using SAFERELEASE
#include "ThunderVertex.h"		// defining classes

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| VertexBuffer implementation
\*----------------------------------------------------------*/

VertexBuffer::VertexBuffer(UINT uVertexSize, DWORD dwUsage):
							m_uVertexSize(uVertexSize),
							m_uBufferSize(0),
							m_uUsedSize(0),
							m_uUsedCount(0),
							m_dwUsage(dwUsage),
							m_pBuffer(NULL)
{
}

VertexBuffer::~VertexBuffer(void)
{
	Empty();
}

void VertexBuffer::Lock(UINT uSize, void** ppData, DWORD dwFlags)
{
	if (NULL == m_pBuffer)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	HRESULT hr = m_pBuffer->Lock(m_uUsedSize, uSize, ppData, dwFlags);

	if (FAILED(hr))
		throw Error(Error::D3D_VERTEXBUFFER_LOCK, __FUNCTIONW__, hr);

	m_uUsedSize += uSize;
	m_uUsedCount += uSize / m_uVertexSize;
}

void VertexBuffer::Unlock(void)
{
	HRESULT hr = m_pBuffer->Unlock();

	if (FAILED(hr))
		throw Error(Error::D3D_VERTEXBUFFER_UNLOCK, __FUNCTIONW__, hr);
}

void VertexBuffer::Reset(void)
{
	m_uUsedSize = 0;
	m_uUsedCount = 0;
}

void VertexBuffer::Create(Graphics& rGraphics, UINT uVertexCount)
{
	m_uBufferSize = uVertexCount * m_uVertexSize;

	HRESULT hr = rGraphics.GetDevice()->CreateVertexBuffer(m_uBufferSize,
		m_dwUsage, 0, D3DPOOL_DEFAULT, &m_pBuffer, NULL);

	if (FAILED(hr))
		throw Error(Error::D3D_DEVICE_CREATEVERTEXBUFFER, __FUNCTIONW__, hr);
}

void VertexBuffer::Empty(void)
{
	SAFERELEASE(m_pBuffer);

	m_uUsedCount = 0;
	m_uUsedSize = 0;
}

/*----------------------------------------------------------*\
| IndexBuffer implementation
\*----------------------------------------------------------*/

IndexBuffer::IndexBuffer(DWORD dwUsage):
						 m_uIndexCount(0),
						 m_uUsedCount(0),
						 m_uMinVertex(0),
						 m_dwUsage(dwUsage),
						 m_pBuffer(NULL)
{
}

IndexBuffer::~IndexBuffer(void)
{
	Empty();
}

void IndexBuffer::Lock(UINT uIndexCount, LPWORD* pwData, DWORD dwFlags)
{
	if (NULL == m_pBuffer)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	HRESULT hr = m_pBuffer->Lock(m_uUsedCount * sizeof(WORD),
		D3DLOCK_DISCARD == dwFlags ? 0 : uIndexCount * sizeof(WORD), (void**)pwData, dwFlags);

	if (FAILED(hr))
		throw Error(Error::D3D_INDEXBUFFER_LOCK, __FUNCTIONW__, hr);

	m_uUsedCount += uIndexCount;
}

void IndexBuffer::Unlock(void)
{
	HRESULT hr = m_pBuffer->Unlock();

	if (FAILED(hr))
		throw Error(Error::D3D_INDEXBUFFER_UNLOCK, __FUNCTIONW__, hr);
}

void IndexBuffer::Reset(void)
{
	m_uUsedCount = 0;
	m_uMinVertex = 0;
}

void IndexBuffer::Create(Graphics& rGraphics, UINT uIndexCount)
{
	HRESULT hr = rGraphics.GetDevice()->CreateIndexBuffer(
		uIndexCount * sizeof(WORD), m_dwUsage, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &m_pBuffer, NULL);

	if (FAILED(hr))
		throw Error(Error::D3D_DEVICE_CREATEINDEXBUFFER, __FUNCTIONW__, hr);

	m_uIndexCount = uIndexCount;
}

void IndexBuffer::Empty(void)
{
	SAFERELEASE(m_pBuffer);
}

/*----------------------------------------------------------*\
| VertexCache implementation
\*----------------------------------------------------------*/

VertexCache::VertexCache(void): m_uSize(0),
								m_uSizeFree(0),
								m_pVC(NULL),
								m_pVCPos(NULL)
{
}

VertexCache::~VertexCache(void)
{
	Empty();
}

void VertexCache::Create(UINT uSize)
{
	m_pVC = (LPBYTE)malloc(uSize);

	if (NULL == m_pVC)
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, uSize);

	m_pVCPos = m_pVC;

	m_uSize = uSize;
	m_uSizeFree = uSize;
}

void VertexCache::Write(LPBYTE pbData, UINT uSize)
{
	if (NULL == m_pVCPos || uSize > m_uSizeFree)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(m_pVCPos, pbData, uSize);

	m_pVCPos += uSize;
	m_uSizeFree -= uSize;
}

void VertexCache::Reset(void)
{
	m_uSizeFree = m_uSize;
	m_pVCPos = m_pVC;
}

void VertexCache::Empty(void)
{
	if (m_pVC != NULL)
		free(m_pVC);

	m_pVC = NULL;
	m_pVCPos = NULL;

	m_uSize = 0;
	m_uSizeFree = 0;
}