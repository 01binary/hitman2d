/*------------------------------------------------------------------*\
|
| ThunderAudio.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine audio class(es) implementation
| Created: 04/30/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine, Error, DEFAULT_VALUE

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Audio implementation
\*----------------------------------------------------------*/

Audio::Audio(Engine& rEngine):
			 m_rEngine(rEngine),
			 m_pDSound(NULL),
			 m_bInitialized(false),
			 m_hMasterMixer(NULL),
			 m_dwMasterLine(DEFAULT_VALUE),
			 m_dwMasterVolumeCtl(DEFAULT_VALUE),
			 m_dwMasterMuteCtl(DEFAULT_VALUE)
{
	m_arChannelVolumes.reserve(4);
}

Audio::~Audio(void)
{
	Empty();
}

void Audio::Initialize(void)
{
	// Create DirectSound8 instance

	HRESULT hr = DirectSoundCreate8(NULL, &m_pDSound, NULL);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSOUND_CREATE,
			__FUNCTIONW__, hr);

	hr = m_pDSound->SetCooperativeLevel(m_rEngine.GetGameWindow(),
		m_rEngine.GetOption(Engine::OPTION_EXCLUSIVE_SOUND) == TRUE ?
		DSSCL_EXCLUSIVE : DSSCL_NORMAL);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSOUND_SETCOOPERATIVELEVEL,
			__FUNCTIONW__, hr);

	// Set audio destination for the first time
	
	SetDestination(Audio::Destinations(
		m_rEngine.GetOption(Engine::OPTION_AUDIO_DESTINATION)));

	m_bInitialized = true;
}

void Audio::CacheMixerControlHandles(void)
{
	// Open the specified mixer

	MMRESULT mmr = mixerOpen(&m_hMasterMixer, DEFAULT_MIXER,
		NULL, 0, MIXER_OBJECTF_MIXER);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_OPEN,
			__FUNCTIONW__, mmr);

	// Find a speakers/headphones destination line

	MIXERLINE mxl;

	mxl.cbStruct = sizeof(MIXERLINE);

	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

	mmr = mixerGetLineInfo(HMIXEROBJ(m_hMasterMixer), &mxl,
		MIXER_GETLINEINFOF_COMPONENTTYPE);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_GETLINEINFO,
			__FUNCTIONW__, mmr);

	// Find volume control
	
	MIXERLINECONTROLS mxlcs = {0};

	MIXERCONTROL mxlcVolume = {0};

	mxlcs.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlcs.dwLineID = mxl.dwLineID;
	mxlcs.cControls = 1;
	mxlcs.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlcs.pamxctrl = &mxlcVolume;
	mxlcs.cbmxctrl = sizeof(MIXERCONTROL);

	mmr = mixerGetLineControls(HMIXEROBJ(m_hMasterMixer), &mxlcs,
		MIXER_GETLINECONTROLSF_ONEBYTYPE);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_GETLINECONTROLS,
			__FUNCTIONW__, mmr);

	m_dwMasterVolumeCtl = mxlcVolume.dwControlID;

	// Find mute control

	MIXERCONTROL mxlcMute = {0};

	mxlcs.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlcs.pamxctrl = &mxlcMute;	

	mmr = mixerGetLineControls(HMIXEROBJ(m_hMasterMixer),
		&mxlcs, MIXER_GETLINECONTROLSF_ONEBYTYPE);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_GETLINECONTROLS,
			__FUNCTIONW__, mmr);

	m_dwMasterMuteCtl = mxlcMute.dwControlID;
}

float Audio::GetMasterVolume(void) const
{
	if (NULL == m_hMasterMixer)
	{
		if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
			return -1.0f;
		else
			throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}

	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdval = {0};

	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.dwControlID = m_dwMasterVolumeCtl;
	mcd.cChannels = 1;
	mcd.paDetails = &mcdval;

	MMRESULT mmr = mixerGetControlDetails((HMIXEROBJ)m_hMasterMixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_GETCONTROLDETAILS,
			__FUNCTIONW__, mmr);

	return float(LOWORD(mcdval.dwValue)) / float(0xFFFF);
}

void Audio::SetMasterVolume(float fVolume)
{
	if (NULL == m_hMasterMixer)
	{
		if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
			return;
		else
			throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}

	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_UNSIGNED mcdval = {0};

	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mcd.dwControlID = m_dwMasterVolumeCtl;
	mcd.cChannels = 1;
	mcd.paDetails = &mcdval;

	mcdval.dwValue = DWORD(float(0xFFFF) * fVolume);

	MMRESULT mmr = mixerSetControlDetails((HMIXEROBJ)m_hMasterMixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);

	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_SETCONTROLDETAILS,
			__FUNCTIONW__, mmr);
}

bool Audio::GetMasterVolumeMute(void) const
{
	if (NULL == m_hMasterMixer)
	{
		if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
			return true;
		else
			throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}

	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_BOOLEAN mcdval = {0};

	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.dwControlID = m_dwMasterMuteCtl;
	mcd.cChannels = 1;
	mcd.paDetails = &mcdval;

	MMRESULT mmr = mixerGetControlDetails((HMIXEROBJ)m_hMasterMixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
	
	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_GETCONTROLDETAILS,
			__FUNCTIONW__, mmr);

	return mcdval.fValue != 0;
}

void Audio::SetMasterVolumeMute(bool bMute)
{
	if (NULL == m_hMasterMixer)
	{
		if (m_rEngine.GetOption(Engine::OPTION_DISABLE_SOUNDS) == TRUE)
			return;
		else
			throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}

	MIXERCONTROLDETAILS mcd = {0};
	MIXERCONTROLDETAILS_BOOLEAN mcdval = {0};

	mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mcd.dwControlID = m_dwMasterMuteCtl;
	mcd.cChannels = 1;
	mcd.paDetails = &mcdval;

	mcdval.fValue = (LONG)bMute;

	MMRESULT mmr = mixerSetControlDetails((HMIXEROBJ)m_hMasterMixer, &mcd,
		MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
	
	if (mmr != MMSYSERR_NOERROR)
		throw m_rEngine.GetErrors().Push(Error::WIN_MIXER_SETCONTROLDETAILS,
			__FUNCTIONW__, mmr);
}

void Audio::SetDestination(Destinations nDest)
{
	// Update engine option if not the same (this calls SetDestination again so we return)

	if (m_rEngine.GetOption(Engine::OPTION_AUDIO_DESTINATION) != nDest)
	{
		m_rEngine.SetOption(Engine::OPTION_AUDIO_DESTINATION, nDest);
		return;
	}

	// Set DirectSound audio destination

	if (m_pDSound != NULL)
	{
		DWORD dwSpeakerConfig = 0;
		
		HRESULT hr = m_pDSound->GetSpeakerConfig(&dwSpeakerConfig);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSOUND_GETSPEAKERCONFIG,
				__FUNCTIONW__, hr);

		BYTE byGeometry = BYTE(HIWORD(dwSpeakerConfig));

		if (DESTINATION_SPEAKERS == nDest)
			dwSpeakerConfig = DSSPEAKER_COMBINED(DSSPEAKER_STEREO, byGeometry);
		else if (DESTINATION_HEADPHONES == nDest)
			dwSpeakerConfig = DSSPEAKER_HEADPHONE;
		else
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__);

		hr = m_pDSound->SetSpeakerConfig(dwSpeakerConfig);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSOUND_SETSPEAKERCONFIG,
				__FUNCTIONW__, hr);
	}

	// Save volume settings

	bool bSave = (m_hMasterMixer != NULL);

	float fMaster = (true == bSave) ? GetMasterVolume() : -1.0f;
	bool bMute = (true == bSave) ? GetMasterVolumeMute() : true;

	// Re-discover the mixer lines

	CacheMixerControlHandles();

	// Restore volume settings

	if (true == bSave)
	{
		SetMasterVolume(fMaster);
		SetMasterVolumeMute(bMute);
	}
}

float Audio::GetChannelVolume(int nLogicalChannel) const
{
	if (nLogicalChannel < 0 ||
		nLogicalChannel >= int(m_arChannelVolumes.size()))
		return -1.0f;

	return m_arChannelVolumes[nLogicalChannel];
}

void Audio::SetChannelVolume(int nLogicalChannel, float fVolume)
{
	if (nLogicalChannel < 0 || fVolume < 0.0f || fVolume > 1.0f)
		return;

	// If out of upper bound, set number of channels to match upper bound

	if (nLogicalChannel >= int(m_arChannelVolumes.size()))
		m_arChannelVolumes.resize(size_t(nLogicalChannel + 1));

	m_arChannelVolumes[nLogicalChannel] = fVolume;
}

SoundInstanceManager& Audio::GetSoundInstances(void)
{
	return m_SoundInstances;
}

const SoundInstanceManager& Audio::GetSoundInstancesConst(void) const
{
	return m_SoundInstances;
}

void Audio::Update(float fTime)
{
	m_SoundInstances.Update(fTime);
}

void Audio::Empty(void)
{
	m_bInitialized = false;

	// Unload all sound instances

	m_SoundInstances.Empty();

	// Clean up DirectSound

	SAFERELEASE(m_pDSound);

	// Clean up master volume mixer

	if (m_hMasterMixer != NULL)
	{
		mixerClose(m_hMasterMixer);

		m_hMasterMixer = NULL;
		m_dwMasterLine = DEFAULT_VALUE;
		m_dwMasterVolumeCtl = DEFAULT_VALUE;
		m_dwMasterMuteCtl = DEFAULT_VALUE;
	}
}