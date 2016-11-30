/*------------------------------------------------------------------*\
|
| ThunderMusic.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine music class
| Created: 09/03/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_MUSIC_H
#define THUNDER_MUSIC_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

#define WM_DSHOWNOTIFY		WM_APP + 1

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;				// referencing Engine


/*----------------------------------------------------------*\
| Music class - DirectShow/DirectSound
\*----------------------------------------------------------*/

class Music: public Resource
{
public:
	//
	// Constants
	//

	// Music flags

	enum Flags
	{
		// No flags set
		DEFAULT		= 0,

		// Playing
		PLAYING		= 1 << 0,

		// Looping
		LOOPING		= 1 << 1,

		// Suspended due to session pause
		SUSPENDED	= 1 << 2
	};

	// Min amplification of DShow filter graph
	static const long DSHOW_VOLUMEMIN;

	// Max amplification of DShow filter graph
	static const long DSHOW_VOLUMEMAX;

private:
	//
	// Members
	//

	IGraphBuilder* m_pGraphBuilder;
	IMediaControl* m_pMediaControl;
	IMediaPosition* m_pMediaPosition;
	IMediaEventEx* m_pMediaEvent;
	IBasicAudio* m_pBasicAudio;

public:
	Music(Engine& rEngine);
	virtual ~Music(void);

public:
	//
	// Playback
	//

	void Play(bool bLoop = false);
	void Stop(void);
	void Pause(bool bPause = true);
	void Restart(void);

	//
	// Volume control (0.0 to 1.0)
	//

	float GetVolume(void);
	void SetVolume(float fVolume);

	//
	// Interfaces
	//

	inline IMediaEventEx* GetMediaEvent(void)
	{
		return m_pMediaEvent;
	}

	//
	// Serialization (can only load)
	//

	virtual void Deserialize(LPCWSTR pszPath);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);
};

/*----------------------------------------------------------*\
| MusicManager class - specialized resource manager
\*----------------------------------------------------------*/

class MusicManager: public ResourceManager<Music>
{
public:
	MusicManager(Engine& rEngine);

public:
	Music* Load(LPCWSTR pszPath, float fPersistenceTime = -1.0f);
	Music* LoadInstance(Stream& rStream, float fPersistenceTime = -1.0f);
	Music* LoadInstance(const InfoElem& rElem, float fPersistenceTime = -1.0f);
};

} // namespace ThunderStorm

#endif // THUNDER_MUSIC_H