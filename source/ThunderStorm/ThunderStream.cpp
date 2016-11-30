/*------------------------------------------------------------------*\
|
| ThunderStream.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm binary file stream class implementation
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "ThunderEngine.h"	// using Engine, using ErrorManager, etc
#include "ThunderGlobals.h"	// using INVALID_VALUE

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WORD UNICODE_SIGNATURE = 0xFEFF;


/*----------------------------------------------------------*\
| Stream implementation
\*----------------------------------------------------------*/

Stream::Stream(ErrorManager* pErrorContext,
			   StreamCache* pStreamCache):
			   
			   m_pErrorContext(pErrorContext),
			   m_pStreamCache(pStreamCache),

			   m_hFile(INVALID_HANDLE_VALUE),
			   m_dwAccess(0),

			   m_dwBufferSize(0),
			   m_pBuffer(NULL),
			   m_pPos(NULL),

			   m_dwSizeWritten(0)
{
}

Stream::~Stream(void)
{
	Empty();
}

void Stream::Open(LPCWSTR pszPath,
				  DWORD dwAccess,
				  DWORD dwCreationDisposition,
				  DWORD dwFlagsAttributes)
{
	// Validate path

	if (String::IsEmpty(pszPath) == true)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_PARAM, __FUNCTIONW__, 0);
		else
			throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);
	}

	// Empty current contents

	if (IsOpen() == true)
		Empty();

	// Get cached data

	StreamData* pCachedData = NULL;

	if (m_pStreamCache != NULL &&
	   (OPEN_EXISTING == dwCreationDisposition ||
	    OPEN_ALWAYS == dwCreationDisposition))
	{
		pCachedData = m_pStreamCache->GetData(pszPath);

		if (NULL == pCachedData)
		{
			if (m_pErrorContext != NULL)
				throw m_pErrorContext->Push(Error::INVALID_PTR,
					__FUNCTIONW__, L"pCachedData");
			else
				throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"pCachedData");
		}

		if (pCachedData->IsEmpty() == false)
		{
			// If already cached, use cached data

			m_pBuffer = pCachedData->GetData();
			m_dwBufferSize = pCachedData->GetDataSize();
			m_pPos = m_pBuffer;

			// Save access mode and path

			m_dwAccess = dwAccess;
			m_strPath = pszPath;

			return;
		}
	}

	// Create or open the file

	m_hFile = CreateFile(pszPath, dwAccess, FILE_SHARE_READ, NULL,
		dwCreationDisposition, dwFlagsAttributes, NULL);

	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_OPEN,
				__FUNCTIONW__, pszPath);
		else
			throw Error(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	// Save access mode and path

	m_dwAccess = dwAccess;
	m_strPath = pszPath;

	// If caching enabled, cache file data

	if (pCachedData != NULL)
	{
		DWORD dwSize = GetSize();
		CreateReadBuffer(dwSize);

		pCachedData->SetData(m_pBuffer, dwSize);
	}
}

void Stream::Empty(void)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);

		m_hFile = INVALID_HANDLE_VALUE;

		m_dwAccess = 0;

		ReleaseBuffer();
	}

	m_strPath.Empty();
}

bool Stream::IsOpen(void) const
{
	return (m_hFile != INVALID_HANDLE_VALUE);
}

bool Stream::IsReading(void) const
{
	return (m_dwAccess & GENERIC_READ) != 0;
}

bool Stream::IsWriting(void) const
{
	return (m_dwAccess & GENERIC_WRITE) != 0;
}

bool Stream::IsUnicodeTextFile(void)
{
	// Read the first WORD from the file.
	// If it's the Notepad unicode signature, return true

	WORD wSignature = 0;
	
	try
	{
		ReadVar(&wSignature);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_READ, __FUNCTIONW__, m_strPath);
		else
			throw Error(Error::FILE_READ, __FUNCTIONW__, m_strPath);
	}

	return (UNICODE_SIGNATURE == wSignature);
}

void Stream::WriteUnicodeSignature(void)
{
	try
	{
		// Write the unicode text file signature

		WriteVar((const LPWORD)&UNICODE_SIGNATURE);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_WRITE,
				__FUNCTIONW__, m_strPath);
		else
			throw Error(Error::FILE_WRITE, __FUNCTIONW__, m_strPath);
	}
}

DWORD Stream::GetAccess(void) const
{
	return m_dwAccess;
}

HANDLE Stream::GetHandle(void) const
{
	return m_hFile;
}

const String& Stream::GetPath(void) const
{
	return m_strPath;
}

String Stream::GetTitle(bool bIncludeExtension) const
{
	// String to be returned

	String strRet;

	// Find start of file title in path

	LPWSTR pszPathTitle = (LPWSTR)PathFindFileName(m_strPath.GetBufferConst());
	if (NULL == pszPathTitle) return strRet;

	int nTitleLen = int(wcslen(pszPathTitle));

	if (true == bIncludeExtension)
	{
		// Find extension in file title, if any

		LPCWSTR pszPathExt = PathFindExtension(pszPathTitle);

		nTitleLen = int(pszPathExt - pszPathTitle);
	}

	// Copy file title into string to be returned

	try
	{
		strRet.Allocate(nTitleLen);
	}

	catch(Error& rError)
	{
		if (m_pErrorContext != NULL)
			m_pErrorContext->Push(rError);
		else
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
				(nTitleLen + 1) * sizeof(WCHAR));
	}

	strRet.CopyToBuffer(nTitleLen, pszPathTitle, nTitleLen);

	return strRet;
}

void Stream::SetPath(LPCWSTR pszPath)
{
	if (String::IsEmpty(pszPath) == false)
		m_strPath = pszPath;
	else
		m_strPath.Empty();
}

DWORD Stream::GetSize(void) const
{
	if (m_pBuffer != NULL) return m_dwBufferSize;

	if (IsOpen() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	return GetFileSize(m_hFile, NULL);
}

DWORD Stream::GetPosition(void) const
{
	if (m_pBuffer != NULL)
		return DWORD(m_pPos - m_pBuffer);

	if (IsOpen() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	return SetFilePointer(m_hFile, 0, NULL, MOVE_CURRENT);
}

DWORD Stream::SetPosition(LONG lDistanceToMove, StreamMove nMoveMethod)
{
	if (IsOpen() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	if (m_pBuffer != NULL)
	{
		switch(nMoveMethod)
		{
		case MOVE_BEGIN:
			{
				// We assume that distances are in coords assuming buffered read

				m_pPos = m_pBuffer + lDistanceToMove;
			}
			break;
		case MOVE_CURRENT:
			{
				// Just move that distance

				m_pPos = m_pPos + lDistanceToMove;
			}
			break;
		case MOVE_END:
			{
				// We assume that distances are in coords assuming buffered read

				m_pPos = m_pBuffer + GetSize() - lDistanceToMove;
			}
			break;
		}

		return DWORD(m_pPos - m_pBuffer);
	}

	return SetFilePointer(m_hFile, lDistanceToMove, NULL, DWORD(nMoveMethod));
}

BYTE* Stream::CreateReadBuffer(DWORD dwSize)
{
	if (m_pStreamCache != NULL && m_pBuffer != NULL)
	{
		// If resizing a cached buffer larger than the cached size...

		StreamData* pStreamData = m_pStreamCache->GetData(m_strPath);

		if (dwSize > pStreamData->GetDataSize())
		{
			// Save current position

			DWORD dwPos = DWORD(m_pPos - m_pBuffer);

			// Reallocate buffer memory

			m_pBuffer = (LPBYTE)realloc(m_pBuffer, dwSize);
			
			if (NULL == m_pBuffer)
			{
				if (m_pErrorContext != NULL)
					throw m_pErrorContext->Push(Error::MEM_ALLOC,
						__FUNCTIONW__, dwSize);
				else
					throw Error(Error::MEM_ALLOC, __FUNCTIONW__, dwSize);
			}

			// Restore current position

			m_pPos = m_pBuffer + dwPos;

			// Pad with zeros

			ZeroMemory(m_pBuffer + pStreamData->GetDataSize(),
					   dwSize - pStreamData->GetDataSize());

			// Update cached size (but do not update stream buffer size)

			pStreamData->SetData(m_pBuffer, dwSize);
		}

		return m_pPos;
	}

	// Destroy any previously allocated buffer

	ReleaseBuffer();

	// Calculate fill size

	DWORD dwSizeLeft = GetSize() - GetPosition();

	if (0 == dwSize) dwSize = dwSizeLeft;

	DWORD dwSizeFill = dwSize > dwSizeLeft ? dwSizeLeft : dwSize;
	DWORD dwSizeDiff = dwSize - dwSizeFill;

	// Allocate new read buffer

	m_pBuffer = (LPBYTE)malloc(dwSize);

	if (NULL == m_pBuffer)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::MEM_ALLOC,
				__FUNCTIONW__, int(dwSize));
		else
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, int(dwSize));
	}

	m_pPos = m_pBuffer;

	// Fill it with data from file, starting at current position

	DWORD dwRead;

	if (ReadFile(m_hFile,
			    (LPVOID)m_pBuffer,
				dwSizeFill,
				&dwRead,
				NULL) == FALSE)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_READ,
				__FUNCTIONW__, m_strPath);
		else
			throw Error(Error::FILE_READ, __FUNCTIONW__, m_strPath);
	}

	// Fill unused space (if any) with zero bytes

	if (dwSizeDiff > 0)
		memset((LPVOID)(m_pBuffer + dwSizeFill), 0, dwSizeDiff);

	m_dwBufferSize = dwSize;

	return m_pBuffer;
}

BYTE* Stream::GetReadBuffer(void)
{
	return m_pBuffer;
}

const BYTE* Stream::GetReadBufferConst(void) const
{
	return m_pBuffer;
}

bool Stream::IsReadBuffered(void) const
{
	return (m_pBuffer != NULL);
}

void Stream::ReleaseBuffer(void)
{
	if (m_pBuffer != NULL)
	{
		// Only free if not used by stream cache

		if (NULL == m_pStreamCache)
			free((LPVOID)m_pBuffer);

		m_pBuffer = NULL;
		m_pPos = NULL;

		m_dwBufferSize = 0;
	}
}

void Stream::ResetSizeWritten(void)
{
	m_dwSizeWritten = 0;
}

DWORD Stream::GetSizeWritten(void) const
{
	return m_dwSizeWritten;
}

void Stream::Read(LPVOID pBuffer, DWORD nBytesToRead, LPDWORD pdwBytesRead)
{
	if (IsReading() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	if (m_pBuffer != NULL)
	{
		// Read from the read buffer at current position

		DWORD dwCanRead = m_dwBufferSize - GetPosition();
		if (0 == dwCanRead) return;

		if (nBytesToRead < dwCanRead) dwCanRead = nBytesToRead;

		CopyMemory(pBuffer, m_pPos, dwCanRead);

		if (pdwBytesRead != NULL) *pdwBytesRead = dwCanRead;

		// Update pointer
		
		m_pPos += dwCanRead;

		return;
	}

	DWORD dwRead = 0;

	if (ReadFile(m_hFile, pBuffer, nBytesToRead, &dwRead, NULL) == FALSE)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::FILE_READ,
				__FUNCTIONW__, m_strPath);
		else
			throw Error(Error::FILE_READ, __FUNCTIONW__, m_strPath);
	}

	if (pdwBytesRead != NULL)
		*pdwBytesRead = dwRead;

	m_dwSizeWritten += dwRead;
}

void Stream::Write(LPCVOID pBuffer, DWORD dwBytesToWrite, LPDWORD pdwBytesWritten)
{
	if (IsWriting() == false)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	DWORD dwWritten = 0;

	if (WriteFile(m_hFile, pBuffer, dwBytesToWrite, &dwWritten, NULL) == FALSE)
	{
		if (m_pErrorContext != NULL)
			throw m_pErrorContext->Push(Error::INVALID_CALL, __FUNCTIONW__);
		else
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	}

	if (pdwBytesWritten != NULL)
		*pdwBytesWritten = dwWritten;

	m_dwSizeWritten += dwWritten;
}

void Stream::ReadVar(int* pn, int nCount)
{
	Read((LPVOID)pn, sizeof(INT) * nCount);
}

void Stream::ReadVar(BYTE* pb, int nCount)
{
	Read((LPVOID)pb, sizeof(BYTE) * nCount);
}

void Stream::ReadVar(bool* pb, int nCount)
{
	Read((LPVOID)pb, sizeof(bool) * nCount);
}

void Stream::ReadVar(long* pl, int nCount)
{
	Read((LPVOID)pl, sizeof(LONG) * nCount);
}

void Stream::ReadVar(DWORD* pdw, int nCount)
{
	Read((LPVOID)pdw, sizeof(DWORD) * nCount);
}

void Stream::ReadVar(WORD* pw, int nCount)
{
	Read((LPVOID)pw, sizeof(WORD) * nCount);
}

void Stream::ReadVar(float* pf, int nCount)
{
	Read((LPVOID)pf, sizeof(FLOAT) * nCount);
}

void Stream::WriteVar(const int* pn, int nCount)
{
	Write((LPCVOID)pn, sizeof(INT) * nCount);
}

void Stream::WriteVar(const BYTE* pb, int nCount)
{
	Write((LPCVOID)pb, sizeof(BYTE) * nCount);
}

void Stream::WriteVar(const bool* pb, int nCount)
{
	Write((LPCVOID)pb, sizeof(bool) * nCount);
}

void Stream::WriteVar(const long* pl, int nCount)
{
	Write((LPCVOID)pl, sizeof(LONG) * nCount);
}

void Stream::WriteVar(const DWORD* pdw, int nCount)
{
	Write((LPCVOID)pdw, sizeof(DWORD) * nCount);
}

void Stream::WriteVar(const WORD* pw, int nCount)
{
	Write((LPCVOID)pw, sizeof(WORD) * nCount);
}

void Stream::WriteVar(const float* pf, int nCount)
{
	Write((LPCVOID)pf, sizeof(float) * nCount);
}

/*----------------------------------------------------------*\
| StreamData implementation
\*----------------------------------------------------------*/

StreamData::StreamData(float fRequestTime):  m_pData(NULL),
											 m_dwDataSize(0),
											 m_fLastRequestTime(fRequestTime)
{
}

StreamData::~StreamData(void)
{
	Empty();
}

LPBYTE StreamData::GetData(void) const
{
	return m_pData;
}

DWORD StreamData::GetDataSize(void) const
{
	return m_dwDataSize;
}

void StreamData::SetData(LPBYTE pData, DWORD dwDataSize)
{
	m_pData = pData;
	m_dwDataSize = dwDataSize;
}

bool StreamData::IsEmpty(void) const
{
	return (m_pData == NULL);
}

float StreamData::GetLastRequestTime(void) const
{
	return m_fLastRequestTime;
}

void StreamData::SetLastRequestTime(float fRequestTime)
{
	m_fLastRequestTime = fRequestTime;
}

void StreamData::Empty(void)
{
	if (m_pData != NULL)
		delete[] m_pData;
}

/*----------------------------------------------------------*\
| StreamCache implementation
\*----------------------------------------------------------*/

StreamCache::StreamCache(Engine& rEngine, float fPersistanceTime):
						m_rEngine(rEngine)
{
}

StreamCache::~StreamCache(void)
{
	Empty();
}

StreamData* StreamCache::GetData(LPCWSTR pszStreamName)
{
	StreamDataMapIterator pos = m_mapStreams.find(pszStreamName);

	if (pos == m_mapStreams.end())
	{
		StreamData* pNewStreamData = new StreamData(m_rEngine.GetRunTime());

		m_mapStreams[pszStreamName] = pNewStreamData;

		return pNewStreamData;
	}

	pos->second->SetLastRequestTime(m_rEngine.GetRunTime());
	
	return pos->second;
}

void StreamCache::Evict(LPCWSTR pszStreamName)
{
	StreamDataMapIterator pos = m_mapStreams.find(pszStreamName);

	if (pos != m_mapStreams.end())
	{
		delete pos->second;
		m_mapStreams.erase(pos);
	}
}

void StreamCache::EvictAll(void)
{
	for(StreamDataMapIterator pos = m_mapStreams.begin();
		pos != m_mapStreams.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapStreams.clear();
}

StreamDataMapIterator StreamCache::GetBeginPos(void)
{
	return m_mapStreams.begin();
}

StreamDataMapIterator StreamCache::GetEndPos(void)
{
	return m_mapStreams.end();
}

StreamDataMapConstIterator StreamCache::GetBeginPosConst(void) const
{
	return m_mapStreams.begin();
}

StreamDataMapConstIterator StreamCache::GetEndPosConst(void) const
{
	return m_mapStreams.end();
}

int StreamCache::GetCount(void) const
{
	return int(m_mapStreams.size());
}

void StreamCache::Update(void)
{
	StreamDataMapIterator pos = m_mapStreams.begin();

	while(pos != m_mapStreams.end())
	{
		if (int(m_rEngine.GetRunTime() - pos->second->GetLastRequestTime()) >
		   m_rEngine.GetOption(Engine::OPTION_STREAM_CACHE_DURATION))
		{
			StreamDataMapIterator posNext = pos;
			posNext++;

			delete pos->second;
			m_mapStreams.erase(pos);

			pos = posNext;

			continue;
		}

		pos++;
	}
}

DWORD StreamCache::GetMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(StreamDataMapConstIterator pos = m_mapStreams.begin();
		pos != m_mapStreams.end();
		pos++)
	{
		dwSize += pos->second->GetDataSize();
	}

	return dwSize;
}

void StreamCache::Empty(void)
{
	EvictAll();
}