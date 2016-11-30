/*------------------------------------------------------------------*\
|
| ThunderSound.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine sound class
| Created: 04/18/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_SOUND_H
#define THUNDER_SOUND_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;					// referencing Engine
class Stream;					// referencing Stream
class SoundInstance;			// referencing SoundInstance, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<SoundInstance*> SoundInstanceArray;
typedef std::vector<SoundInstance*>::iterator SoundInstanceArrayIterator;
typedef std::vector<SoundInstance*>::const_iterator SoundInstanceArrayConstIterator;


/*----------------------------------------------------------*\
| Sound class - non-streamed sound resource
\*----------------------------------------------------------*/

class Sound: public Resource
{
	//
	// Constants
	//

	// WAV RIFF format constants

	static const char SZ_RIFF_SIG[];
	static const char SZ_WAVE_SIG[];
	static const char SZ_FMT_SIG[];
	static const char SZ_DATA_SIG[];

private:
	//
	// Members
	//

	// Base sound buffer (gets duplicated for other instances)
	LPDIRECTSOUNDBUFFER m_pSoundBuffer;

	// Format of this buffer as read from WAV PCM file
	WAVEFORMATEX m_format;

	// Estimated duration in seconds (must be liberate)
	float m_fDuration;

	// Size of the sound buffer in bytes
	DWORD m_dwSize;

	// Number of times this sound is currently instanced
	int m_nInstanceCount;

public:
	Sound(Engine& rEngine);
	virtual ~Sound(void);

public:
	//
	// Sound Buffer
	//

	const WAVEFORMATEX& GetFormat(void) const;
	float GetDuration(void) const;
	DWORD GetBufferSize(void) const;

	//
	// Playback
	//

	SoundInstance* Play(bool bLoop = false, int nChannel = 0);

	int GetInstanceCount(void) const;

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);

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
	// Friends
	//

	friend class SoundInstance;
};

/*----------------------------------------------------------*\
| SoundInstance class - playing instance of a sound
\*----------------------------------------------------------*/

class SoundInstance
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// No flags set
		DEFAULT			= 0,

		// Playing
		PLAYING			= 1 << 0,

		// Looping
		LOOPING			= 1 << 1,

		// Suspended due to session pause
		SUSPENDED		= 1 << 2,

		// Uses a duplicated sound buffer (for playing multiple instances of same sound)
		DUPLICATEBUFFER	= 1 << 3
	};

private:
	// Pointer to sound object originated from
	Sound& m_rSound;

	// Pointer to sound buffer in use (duplicate of sound's buffer)
	LPDIRECTSOUNDBUFFER m_pSoundBuffer;

	// Flags for this sound instance (see Flags enum)
	DWORD m_dwFlags;

	// Logical channel to play in (used to group sounds for manipulation)
	int m_nChannel;

	// Next time to check if removal is required (ignored for looping sounds)
	float m_fEndingTime;

	// If currently paused, stores time when was paused
	float m_fPausedTime;

public:
	SoundInstance(Sound& rSound, DWORD dwFlags, int nChannel);
	~SoundInstance(void);

public:
	//
	// Sound
	//

	Sound& GetSound(void);
	const Sound& GetSoundConst(void) const;

	//
	// Sound buffer
	//

	LPDIRECTSOUNDBUFFER GetDSoundBuffer(void);

	//
	// Time checking
	//

	void SetEndingTime(float fEndingTime);
	float GetEndingTime(void) const;

	//
	// Flags
	//

	DWORD GetFlags(void) const;
	void SetFlags(DWORD dwFlags);
	bool IsFlagSet(DWORD dwFlag) const;
	void SetFlag(DWORD dwFlag);
	void ClearFlag(DWORD dwFlag);

	//
	// Channel
	//

	int GetChannel(void) const;
	void SetChannel(int nChannel);

	//
	// Playback
	//

	void Pause(bool bPause);

	//
	// Volume (0.0 to 1.0)
	//

	float GetVolume(void);
	void SetVolume(float fVolume);
};

/*----------------------------------------------------------*\
| SoundInstanceManager class
\*----------------------------------------------------------*/

class SoundInstanceManager
{
private:
	// Sound instances in this channel
	SoundInstanceArray m_arInstances;

public:
	SoundInstanceManager(void);
	~SoundInstanceManager(void);

public:
	//
	// Sound Instances
	//

	SoundInstanceArrayIterator GetBeginPos(void);
	SoundInstanceArrayIterator GetEndPos(void);

	SoundInstanceArrayConstIterator GetBeginPosConst(void) const;
	SoundInstanceArrayConstIterator GetEndPosConst(void) const;

	void Remove(SoundInstance* pRemove);
	void Remove(SoundInstanceArrayIterator posRemove);
	void RemoveAll(void);

	int GetCount(void) const;

	//
	// Update
	//

	void Update(float fTime);

	//
	// Deinitialization
	//

	void Empty(void);

private:
	//
	// Private Functions
	//

	void Add(SoundInstance* pAdd);

	//
	// Friends
	//

	friend class SoundInstance;
	friend class Sound;
};

/*----------------------------------------------------------*\
| SoundManager class - specialized resource manager
\*----------------------------------------------------------*/

class SoundManager: public ResourceManager<Sound>
{
public:
	SoundManager(Engine& rEngine);

public:
	Sound* Load(LPCWSTR pszPath, float fPersistenceTime = -1.0f);
	Sound* LoadInstance(Stream& rStream, float fPersistenceTime = -1.0f);
	Sound* LoadInstance(const InfoElem& rElem, float fPersistenceTime = -1.0f);
};

} // namespace ThunderStorm

#endif // THUNDER_SOUND_H