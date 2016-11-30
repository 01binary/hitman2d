/*------------------------------------------------------------------*\
|
| ThunderAudio.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine audio class(es)
| Created: 04/30/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_AUDIO_H
#define THUNDER_AUDIO_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderSound.h"		// using SoundInstanceManager

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;	// referencing Engine


/*----------------------------------------------------------*\
| Audio class
\*----------------------------------------------------------*/

class Audio
{
public:
	//
	// Constants
	//

	enum Destinations
	{
		DESTINATION_SPEAKERS,
		DESTINATION_HEADPHONES
	};

	enum
	{
		DEFAULT_MIXER = 0
	};

private:
	//
	// Members
	//

	// Used for DirectSound and error reporting
	Engine& m_rEngine;

	// DirectSound8 instance
	LPDIRECTSOUND8 m_pDSound;

	// DirectSound initialized?
	bool m_bInitialized;
	
	// Name of the master mixer found for master volume adjustment
	String m_strMasterMixer;

	// The first mixer found that contains master line
	HMIXER m_hMasterMixer;

	// The first mixer line found Headphones/Speakers destination type and WaveOut target type
	DWORD m_dwMasterLine;

	// The first control found with Volume control type
	DWORD m_dwMasterVolumeCtl;

	// The first control found with Mute control type
	DWORD m_dwMasterMuteCtl;

	// Volumes for all logical sound channels
	std::vector<float> m_arChannelVolumes;

	// Instances of currently playing sounds (DirectSound)
	SoundInstanceManager m_SoundInstances;

public:
	Audio(Engine& rEngine);
	~Audio(void);

public:
	//
	// Initialization
	//

	void Initialize(void);

	inline bool IsInitialized(void) const
	{
		return m_bInitialized;
	}

	//
	// DirectSound
	//

	inline LPDIRECTSOUND8 GetDirectSound(void)
	{
		return m_pDSound;
	}

	//
	// Master Volume
	//

	inline const String& GetOutputMixerName(void) const
	{
		return m_strMasterMixer;
	}

	float GetMasterVolume(void) const;
	void SetMasterVolume(float fVolume);

	bool GetMasterVolumeMute(void) const;
	void SetMasterVolumeMute(bool bMute);

	void SetDestination(Destinations nDest);

	//
	// Logical Channel Volume
	//

	float GetChannelVolume(int nLogicalChannel) const;
	void SetChannelVolume(int nLogicalChannel, float fVolume);

	//
	// Sound Instances
	//

	SoundInstanceManager& GetSoundInstances(void);
	const SoundInstanceManager& GetSoundInstancesConst(void) const;

	void Update(float fTime);

	//
	// Deinitialization
	//

	void Empty(void);

private:
	void CacheMixerControlHandles(void);
};

} // namespace ThunderStorm

#endif // THUNDER_AUDIO_H