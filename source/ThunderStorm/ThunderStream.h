/*------------------------------------------------------------------*\
|
| ThunderStream.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm binary file stream class
| Created: 06/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STREAM_H
#define THUNDER_STREAM_H

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

class Engine;			// referencing Engine
class ErrorManager;		// referencing ErrorManager
class StreamCache;		// referencing StreamCache (declared below)
class StreamData;		// referencing StreamData (declared below)

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, StreamData*> StreamDataMap;
typedef std::map<String, StreamData*>::iterator StreamDataMapIterator;
typedef std::map<String, StreamData*>::const_iterator StreamDataMapConstIterator;


/*----------------------------------------------------------*\
| Stream class
\*----------------------------------------------------------*/

class Stream
{
public:
	//
	// Constants
	//

	// Stream SetPosition move methods

	enum StreamMove
	{
		MOVE_BEGIN,
		MOVE_CURRENT,
		MOVE_END
	};

private:
	//
	// Members
	//

	// Context for reporting errors (optional)
	ErrorManager* m_pErrorContext;

	// Stream cache used
	StreamCache* m_pStreamCache;

	// File path
	String m_strPath;

	// Handle to file this stream controls
	HANDLE m_hFile;

	// Access mode this file was opened with (see ::CreateFile)	
	DWORD m_dwAccess;

	// Size of buffer, if buffering is enabled (see Stream::CreateReadBuffer)
	DWORD m_dwBufferSize;

	// Pointer to buffer data
	LPBYTE m_pBuffer;

	// Pointer to current position in buffer
	LPBYTE m_pPos;

	// Operation size tracking through Stream::GetSizeWritten/ResetSizeWritten
	DWORD m_dwSizeWritten;

public:
	Stream(ErrorManager* pErrorContext = NULL, StreamCache* pStreamCache = NULL);
	~Stream(void);

public:
	//
	// Operations
	//

	void Open(LPCWSTR pszPath, DWORD dwAccess, DWORD dwCreationDisposition, DWORD dwFlagsAttributes = 0);

	bool IsOpen(void) const;
	bool IsReading(void) const;
	bool IsWriting(void) const;

	bool IsUnicodeTextFile(void);

	void WriteUnicodeSignature(void);

	//
	// Access mode
	//

	DWORD GetAccess(void) const;

	//
	// Handle
	//

	HANDLE GetHandle(void) const;

	//
	// Path
	//

	const String& GetPath(void) const;
	void SetPath(LPCWSTR pszPath);

	String GetTitle(bool nIncludeExtension = true) const;

	//
	// Size
	//

	DWORD GetSize(void) const;

	//
	// Position
	//

	DWORD GetPosition(void) const;
	DWORD SetPosition(LONG lDistanceToMove, StreamMove nMoveMethod);

	//
	// Buffering
	//

	// Create read buffer at current position w/ current size
	BYTE* CreateReadBuffer(DWORD dwSize = 0);

	BYTE* GetReadBuffer(void);
	const BYTE* GetReadBufferConst(void) const;
	bool IsReadBuffered(void) const;
	void ReleaseBuffer(void);

	//
	// Bytes written tracking
	//

	void ResetSizeWritten(void);
	DWORD GetSizeWritten(void) const;

	//
	// Reading and writing
	//

	void Read(LPVOID pBuffer, DWORD dwBytesToRead, LPDWORD pdwBytesRead = NULL);
	void Write(LPCVOID pBuffer, DWORD dwBytesToWrite, LPDWORD pdwBytesWritten = NULL);

	void ReadVar(int* pn, int nCount = 1);
	void ReadVar(BYTE* pb, int nCount = 1);
	void ReadVar(bool* pb, int nCount = 1);
	void ReadVar(long* pl, int nCount = 1);
	void ReadVar(WORD* pdw, int nCount = 1);
	void ReadVar(DWORD* pw, int nCount = 1);
	void ReadVar(float* pf, int nCount = 1);

	void WriteVar(const int* pn, int nCount = 1);
	void WriteVar(const BYTE* pb, int nCount = 1);
	void WriteVar(const bool* pb, int nCount = 1);
	void WriteVar(const long* pl, int nCount = 1);
	void WriteVar(const DWORD* pdw, int nCount = 1);
	void WriteVar(const WORD* pw, int nCount = 1);
	void WriteVar(const float* pf, int nCount = 1);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| StreamData class
\*----------------------------------------------------------*/

class StreamData
{
private:
	LPBYTE m_pData;
	DWORD m_dwDataSize;
	float m_fLastRequestTime;

public:
	StreamData(float fRequestTime);
	~StreamData(void);

public:
	//
	// Data
	//

	LPBYTE GetData(void) const;
	DWORD GetDataSize(void) const;
	void SetData(LPBYTE pData, DWORD dwDataSize);
	bool IsEmpty(void) const;

	//
	// Last Request Time
	//

	float GetLastRequestTime(void) const;
	void SetLastRequestTime(float fRequestTime);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| StreamCache class
\*----------------------------------------------------------*/

class StreamCache
{
private:
	Engine& m_rEngine;
	StreamDataMap m_mapStreams;

public:
	StreamCache(Engine& rEngine, float fPersistanceTime);
	~StreamCache(void);

public:
	//
	// Streams
	//

	StreamData* GetData(LPCWSTR pszStreamName);
	
	void Evict(LPCWSTR pszStreamName);
	void EvictAll(void);

	StreamDataMapIterator GetBeginPos(void);
	StreamDataMapIterator GetEndPos(void);

	StreamDataMapConstIterator GetBeginPosConst(void) const;
	StreamDataMapConstIterator GetEndPosConst(void) const;

	int GetCount(void) const;

	//
	// Update
	//

	void Update(void);

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

} // using namespace ThunderStorm

#endif