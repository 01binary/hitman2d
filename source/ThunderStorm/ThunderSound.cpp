/*------------------------------------------------------------------*\
|
| ThunderSound.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine sound class implementation
| Created: 04/18/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderSound.h"		// defining Sound
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoFile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const char Sound::SZ_RIFF_SIG[]		= { 'R', 'I', 'F', 'F' };
const char Sound::SZ_WAVE_SIG[]		= { 'W', 'A', 'V', 'E' };
const char Sound::SZ_FMT_SIG[]		= { 'f', 'm', 't', ' ' };
const char Sound::SZ_DATA_SIG[]		= { 'd', 'a', 't', 'a' };


/*----------------------------------------------------------*\
| Sound implementation
\*----------------------------------------------------------*/

Sound::Sound(Engine& rEngine): Resource(rEngine),
							   m_pSoundBuffer(NULL),
							   m_fDuration(0.0f),
							   m_dwSize(0),
							   m_nInstanceCount(0)
{
	ZeroMemory(&m_format, sizeof(WAVEFORMATEX));
}

Sound::~Sound(void)
{
	Empty();
}

const WAVEFORMATEX& Sound::GetFormat(void) const
{
	return m_format;
}

float Sound::GetDuration(void) const
{
	return m_fDuration;
}

DWORD Sound::GetBufferSize(void) const
{
	return m_dwSize;
}

int Sound::GetInstanceCount(void) const
{
	return m_nInstanceCount;
}

SoundInstance* Sound::Play(bool bLoop, int nChannel)
{
	// If sounds are disabled, return without throw

	if (TRUE == m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS))
		return NULL;

	// Create a sound instance

	SoundInstance* pInst = NULL;

	try
	{
		pInst = new SoundInstance(*this,
			SoundInstance::PLAYING | (true == bLoop ? SoundInstance::LOOPING : 0),
			nChannel);
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(SoundInstance));
	}

	if (false == bLoop)
		pInst->SetEndingTime(m_rEngine.GetTime() + m_fDuration);

	// Add it to engine's sound instance list

	m_rEngine.GetAudio().GetSoundInstances().Add(pInst);

	return pInst;
}

void Sound::Deserialize(LPCWSTR pszPath)
{
	Stream stream(&m_rEngine.GetErrors());	

	// Open the file

	try
	{
		stream.Open(pszPath, GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	m_strName = pszPath;

	Deserialize(stream);
}

void Sound::Deserialize(Stream& rStream)
{
	Empty();

	// If sounds are disabled, exit with success

	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
		return;

	// Make sure DSound was initialized

	if (m_rEngine.GetAudio().IsInitialized() == false)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	// Read RIFF signature

	char szSig[4] = {0};

	try
	{
		rStream.Read(szSig, sizeof(szSig));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Validate RIFF signature

	if (strncmp(szSig, SZ_RIFF_SIG, sizeof(szSig)) != 0)
		throw m_rEngine.GetErrors().Push(Error::FILE_SIGNATURE,
			__FUNCTIONW__, rStream.GetPath());

	// Loading uncompressed WAV PCM		

	// Read total size

	DWORD dw = 0;
	rStream.ReadVar(&dw);

	// Read and validate WAVE signature

	try
	{
		rStream.Read(szSig, sizeof(szSig));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	if (strncmp(szSig, SZ_WAVE_SIG, sizeof(szSig)))
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());

	// Read and validate FORMAT chunk signature

	try
	{
		rStream.Read(szSig, sizeof(szSig));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	if (strncmp(szSig, SZ_FMT_SIG, sizeof(szSig)))
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());

	try
	{
		// Read format chunk size

		rStream.ReadVar(&dw);

		// Read WAVEFORMATEX

		rStream.Read((LPVOID)&m_format, sizeof(WAVEFORMATEX) - sizeof(WORD));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}	

	m_format.cbSize = sizeof(WAVEFORMATEX);

	if ((m_format.wFormatTag != 1 &&
		m_format.wFormatTag != 85) ||
	    m_format.nChannels > 2 ||
	   (m_format.wBitsPerSample != 8 &&
	   m_format.wBitsPerSample != 16))
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());
	}

	try
	{
		// Read DATA chunk signature signature

		rStream.Read(szSig, sizeof(szSig));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	if (strncmp(szSig, SZ_DATA_SIG, sizeof(szSig)) != 0)
		throw m_rEngine.GetErrors().Push(Error::FILE_SIGNATURE,
			__FUNCTIONW__, rStream.GetPath());

	try
	{
		// Read DATA chunk size

		rStream.ReadVar(&dw);
		m_dwSize = dw;
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Estimate duration conservatively
	// We use duration to estimate when the sound will stop playing,
	// because hardware DirectSound notifications cannot be relied upon.

	m_fDuration = (float(m_dwSize)) /
				   float(m_format.nSamplesPerSec *
						 m_format.nChannels *
						 (m_format.wBitsPerSample >> 4));

	// Create the base sound buffer (static, with volume capabilities)

	DSBUFFERDESC desc = {0};

	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
	desc.lpwfxFormat = &m_format;
	desc.dwBufferBytes = m_dwSize;
	desc.guid3DAlgorithm = DS3DALG_DEFAULT;

	HRESULT hr = m_rEngine.GetAudio().GetDirectSound()->
		CreateSoundBuffer(&desc, &m_pSoundBuffer, NULL);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSOUND_CREATESOUNDBUFFER,
			__FUNCTIONW__, hr);

	// Lock the buffer

	LPVOID pvBuffer, pvDummy;
	DWORD dwBuffer, dwDummy;

	hr = m_pSoundBuffer->Lock(0, 0, &pvBuffer, &dwBuffer,
		&pvDummy, &dwDummy, DSBLOCK_ENTIREBUFFER);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSOUND_BUFFER_LOCK,
			__FUNCTIONW__, hr);

	try
	{
		// Read sound data

		rStream.Read(pvBuffer, m_dwSize);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		// If failed reading, unlock ignoring error code, and error out

		m_pSoundBuffer->Unlock(pvBuffer, dwBuffer, pvDummy, dwDummy);
		
		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Unlock buffer

	hr = m_pSoundBuffer->Unlock(pvBuffer, dwBuffer, pvDummy, dwDummy);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSOUND_BUFFER_UNLOCK,
			__FUNCTIONW__, hr);
}

DWORD Sound::GetMemoryFootprint(void) const
{
	return Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(Sound) +
		m_dwSize;
}

void Sound::Empty(void)
{
	if (m_pSoundBuffer != NULL)
	{
		m_pSoundBuffer->Release();
		m_pSoundBuffer = NULL;
	}
}

void Sound::Remove(void)
{
	m_rEngine.GetSounds().Remove(this);
}

/*----------------------------------------------------------*\
| SoundInstance implementation
\*----------------------------------------------------------*/

SoundInstance::SoundInstance(Sound& rSound, DWORD dwFlags, int nChannel):
							 m_rSound(rSound),
							 m_dwFlags(dwFlags),
							 m_nChannel(nChannel),
							 m_fEndingTime(0.0f),
							 m_fPausedTime(0.0f)
{
	if (m_rSound.GetInstanceCount() > 0)
	{
		// If this is not the first instance, duplicate the sound's buffer

		HRESULT hr = rSound.GetEngine().GetAudio().GetDirectSound()->
			DuplicateSoundBuffer(rSound.m_pSoundBuffer, &m_pSoundBuffer);

		if (FAILED(hr))
			throw rSound.GetEngine().GetErrors().Push(Error::DSOUND_BUFFER_DUPLICATE,
				__FUNCTIONW__, hr);
	}
	else
	{
		// If this is the first instance, use sound's buffer

		m_pSoundBuffer = rSound.m_pSoundBuffer;

		m_pSoundBuffer->AddRef();
	}

	// Increment sound's instance count

	m_rSound.m_nInstanceCount++;

	// Set logical channel volume immediately
	// (see http://msdn2.microsoft.com/en-us/library/bb206037.aspx, "Remarks")

	SetVolume(rSound.GetEngine().GetAudio().GetChannelVolume(nChannel));

	// Start playing if so specified

	if (IsFlagSet(PLAYING) == true)
	{
		HRESULT hr = m_pSoundBuffer->Play(
			0, 0, IsFlagSet(LOOPING) ? DSBPLAY_LOOPING : 0);

		if (FAILED(hr))
			throw m_rSound.m_rEngine.GetErrors().Push(Error::DSOUND_BUFFER_PLAY,
				__FUNCTIONW__, hr);
	}
}

SoundInstance::~SoundInstance(void)
{	
	if (m_pSoundBuffer != NULL)
	{
		if (IsFlagSet(PLAYING) == true)
			m_pSoundBuffer->Stop();

		m_pSoundBuffer->Release();
		m_pSoundBuffer = NULL;

		m_rSound.m_nInstanceCount--;
	}
}

Sound& SoundInstance::GetSound(void)
{
	return m_rSound;
}

const Sound& SoundInstance::GetSoundConst(void) const
{
	return m_rSound;
}

LPDIRECTSOUNDBUFFER SoundInstance::GetDSoundBuffer(void)
{
	return m_pSoundBuffer;
}

void SoundInstance::SetEndingTime(float fEndingTime)
{
	m_fEndingTime = fEndingTime;
}

float SoundInstance::GetEndingTime(void) const
{
	return m_fEndingTime;
}

DWORD SoundInstance::GetFlags(void) const
{
	return m_dwFlags;
}

void SoundInstance::SetFlags(DWORD dwFlags)
{
	m_dwFlags = dwFlags;
}

bool SoundInstance::IsFlagSet(DWORD dwFlag) const
{
	return (m_dwFlags & dwFlag) != 0;
}

void SoundInstance::SetFlag(DWORD dwFlag)
{
	m_dwFlags |= dwFlag;
}

void SoundInstance::ClearFlag(DWORD dwFlag)
{
	m_dwFlags &= ~dwFlag;
}

int SoundInstance::GetChannel(void) const
{
	return m_nChannel;
}

void SoundInstance::SetChannel(int nChannel)
{
	m_nChannel = nChannel;

	SetVolume(m_rSound.GetEngine().GetAudio().GetChannelVolume(nChannel));
}

void SoundInstance::Pause(bool bPause)
{
	if (true == bPause)
	{
		HRESULT hr = m_pSoundBuffer->Stop();

		if (FAILED(hr))
			throw m_rSound.GetEngine().GetErrors().Push(Error::DSOUND_BUFFER_STOP,
				__FUNCTIONW__, hr);

		m_fPausedTime = m_rSound.GetEngine().GetTime();
	}
	else
	{
		HRESULT hr = m_pSoundBuffer->Play(0, 0,
			IsFlagSet(LOOPING) ? DSBPLAY_LOOPING : 0);

		if (FAILED(hr))
			throw m_rSound.GetEngine().GetErrors().Push(Error::DSOUND_BUFFER_PLAY,
				__FUNCTIONW__, hr);

		// Update ending time, since sound may have been paused for quite some time

		float fElapsedSincePause =
			m_rSound.GetEngine().GetTime() - m_fPausedTime;

		m_fEndingTime = m_fEndingTime + fElapsedSincePause;
	}
}

float SoundInstance::GetVolume(void)
{
	long lVolume = 0;

	HRESULT hr = m_pSoundBuffer->GetVolume(&lVolume);

	if (FAILED(hr))
		throw m_rSound.m_rEngine.GetErrors().Push(Error::DSOUND_BUFFER_GETVOLUME,
			__FUNCTIONW__, hr);

	// Convert volume into scale from 0.0 to 1.0

	return (float(lVolume - DSBVOLUME_MIN) / float(DSBVOLUME_MAX - DSBVOLUME_MIN));
}

void SoundInstance::SetVolume(float fVolume)
{
	if (fVolume < 0.0f) fVolume = -fVolume;

	long lVolume = long(DSBVOLUME_MIN + fVolume * (DSBVOLUME_MAX - DSBVOLUME_MIN));

	HRESULT hr = m_pSoundBuffer->SetVolume(lVolume);

	if (FAILED(hr))
		throw m_rSound.m_rEngine.GetErrors().Push(Error::DSOUND_BUFFER_SETVOLUME,
			__FUNCTIONW__, hr);
}

/*----------------------------------------------------------*\
| SoundInstanceManager class
\*----------------------------------------------------------*/

SoundInstanceManager::SoundInstanceManager(void)
{
}

SoundInstanceManager::~SoundInstanceManager(void)
{
	Empty();
}

SoundInstanceArrayIterator SoundInstanceManager::GetBeginPos(void)
{
	return m_arInstances.begin();
}

SoundInstanceArrayIterator SoundInstanceManager::GetEndPos(void)
{
	return m_arInstances.end();
}

SoundInstanceArrayConstIterator SoundInstanceManager::GetBeginPosConst(void) const
{
	return m_arInstances.begin();
}

SoundInstanceArrayConstIterator SoundInstanceManager::GetEndPosConst(void) const
{
	return m_arInstances.end();
}

void SoundInstanceManager::Add(SoundInstance* pAdd)
{
	m_arInstances.push_back(pAdd);
}

void SoundInstanceManager::Remove(SoundInstance* pRemove)
{
	SoundInstanceArrayIterator pos = find(m_arInstances.begin(), 
		m_arInstances.end(), pRemove);
	
	if (pos != m_arInstances.end())
		*pos = NULL;

	delete pRemove;
}

void SoundInstanceManager::Remove(SoundInstanceArrayIterator posRemove)
{
	delete *posRemove;
	*posRemove = NULL;
}

void SoundInstanceManager::RemoveAll(void)
{
	for(SoundInstanceArrayIterator pos = m_arInstances.begin();
		pos != m_arInstances.end();
		pos++)
	{
		delete *pos;
		*pos = NULL;
	}
}

int SoundInstanceManager::GetCount(void) const
{
	return int(m_arInstances.size());
}

void SoundInstanceManager::Update(float fTime)
{
	for(SoundInstanceArrayIterator pos = m_arInstances.begin();
		pos != m_arInstances.end();)
	{
		if (NULL == *pos ||
		  ((*pos)->IsFlagSet(SoundInstance::LOOPING) == false &&
		  fTime > (*pos)->GetEndingTime()))
		{
			// Time to remove this sound instance

			delete *pos;

			pos = m_arInstances.erase(pos);
		}
		else
		{
			pos++;
		}
	}
}

void SoundInstanceManager::Empty(void)
{
	RemoveAll();
}

/*----------------------------------------------------------*\
| SoundManager implementation
\*----------------------------------------------------------*/

SoundManager::SoundManager(Engine& rEngine):
						   ResourceManager<Sound>(rEngine)
{
}

Sound* SoundManager::Load(LPCWSTR pszPath, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
		return NULL;
	
	return ResourceManager<Sound>::Load(pszPath, fPersistenceTime);
}

Sound* SoundManager::LoadInstance(Stream& rStream, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
		return NULL;

	return ResourceManager<Sound>::LoadInstance(rStream, fPersistenceTime);
}

Sound* SoundManager::LoadInstance(const InfoElem& rElem, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
		return NULL;

	return ResourceManager<Sound>::LoadInstance(rElem, fPersistenceTime);
}