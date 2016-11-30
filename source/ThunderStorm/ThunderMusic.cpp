/*------------------------------------------------------------------*\
|
| ThunderMusic.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine music class implementation
| Created: 09/03/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderInfoFile.h"	// using InfoFile
#include "ThunderMusic.h"		// defining Music

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const long Music::DSHOW_VOLUMEMIN	= -10000l;
const long Music::DSHOW_VOLUMEMAX	= 0l;


/*----------------------------------------------------------*\
| Music implementation
\*----------------------------------------------------------*/

Music::Music(Engine& rEngine):	Resource(rEngine),
								m_pGraphBuilder(NULL),
								m_pMediaControl(NULL),
								m_pMediaPosition(NULL),
								m_pMediaEvent(NULL)
														
{
}

Music::~Music(void)
{
	Empty();
}

void Music::SetVolume(float fVolume)
{
	if (fVolume < 0.0f)
		fVolume = -fVolume;

	long lVolume =
		DSHOW_VOLUMEMIN + long(fVolume * (DSHOW_VOLUMEMAX - DSHOW_VOLUMEMIN));

	HRESULT hr = m_pBasicAudio->put_Volume(lVolume);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_BASICAUDIO_PUTVOLUME,
			__FUNCTIONW__, hr);
}

float Music::GetVolume(void)
{
	long lVolume = 0;

	HRESULT hr = m_pBasicAudio->get_Volume(&lVolume);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_BASICAUDIO_GETVOLUME,
			__FUNCTIONW__, hr);

	return (float(lVolume - DSHOW_VOLUMEMIN) /
		float(DSHOW_VOLUMEMAX - DSHOW_VOLUMEMIN));
}

void Music::Play(bool bLoop)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return;

	// Start playing

	HRESULT hr = m_pMediaControl->Run();

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_RUN,
			__FUNCTIONW__, hr);

	SetFlag(PLAYING);

	if (true == bLoop)
		SetFlag(LOOPING);
}

void Music::Stop(void)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return;

	if (IsFlagSet(PLAYING) == true)
	{
		HRESULT hr = m_pMediaControl->Stop();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_STOP,
				__FUNCTIONW__, hr);

		ClearFlag(PLAYING);
	}
	else
	{
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}
}

void Music::Pause(bool bPause)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return;

	if (true == bPause)
	{
		HRESULT hr = m_pMediaControl->Pause();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_PAUSE,
				__FUNCTIONW__, hr);

		ClearFlag(PLAYING);
	}
	else
	{
		Play(IsFlagSet(LOOPING));
	}
}

void Music::Restart(void)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return;

	HRESULT hr = m_pMediaPosition->put_CurrentPosition(0.0);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIAPOSITION_PUTCURRENT,
		__FUNCTIONW__, hr);

	hr = m_pMediaControl->Run();

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_RUN,
			__FUNCTIONW__, hr);
}

void Music::Deserialize(LPCWSTR pszPath)
{
	Empty();

	m_strName = pszPath;

	// If music is disabled, just return

	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return;

	try
	{
		// Create the filter graph

		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
			CLSCTX_INPROC, IID_IGraphBuilder, (void**)&m_pGraphBuilder);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPHCREATE,
				__FUNCTIONW__, hr);

		// Query media control, media event, and media position interfaces

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl,
			(void**)&m_pMediaControl);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_MEDIACONTROLQUERY,
				__FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaEvent,
			(void**)&m_pMediaEvent);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_MEDIAEVENTQUERY,
				__FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaPosition,
			(void**)&m_pMediaPosition);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_MEDIAPOSITIONQUERY,
				__FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void**)&m_pBasicAudio);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_BASICAUDIOQUERY,
				__FUNCTIONW__, hr);

		hr = m_pGraphBuilder->RenderFile(pszPath, NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_RENDERFILE,
				__FUNCTIONW__, hr);

		// Set window for notifications

		hr = m_pMediaEvent->SetNotifyWindow(reinterpret_cast<OAHWND>(
			m_rEngine.GetGameWindow()), WM_DSHOWNOTIFY, 0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIAEVENT_SETNOTIFYWINDOW,
				__FUNCTIONW__, hr);

		// Turn notifications on

		hr = m_pMediaEvent->SetNotifyFlags(0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIAEVENT_SETNOTIFYFLAGS,
				__FUNCTIONW__, hr);
	}

	catch(Error& rError)
	{	
		if (m_pMediaControl != NULL)
		{
			m_pMediaControl->Release();
			m_pMediaControl = NULL;
		}

		if (m_pMediaPosition != NULL)
		{
			m_pMediaPosition->Release();
			m_pMediaPosition = NULL;
		}

		if (m_pMediaEvent != NULL)
		{
			m_pMediaEvent->Release();
			m_pMediaEvent = NULL;
		}

		if (m_pGraphBuilder != NULL)
		{
			m_pGraphBuilder->Release();
			m_pGraphBuilder = NULL;
		}

		throw rError;
	}
}

DWORD Music::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() - sizeof(Music);

	Stream stream;

	try
	{
		stream.Open(m_strName, GENERIC_READ, OPEN_EXISTING, 0);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		return dwSize;
	}

	dwSize += stream.GetSize();

	return dwSize;
}

void Music::Empty(void)
{
	if (IsFlagSet(PLAYING) == true)
		Stop();

	if (m_pMediaPosition != NULL)
	{
		m_pMediaPosition->Release();
		m_pMediaPosition = NULL;
	}

	if (m_pMediaEvent != NULL)
	{
		m_pMediaEvent->Release();
		m_pMediaEvent = NULL;
	}

	if (m_pMediaControl != NULL)
	{
		m_pMediaControl->Release();	
		m_pMediaControl = NULL;
	}

	if (m_pGraphBuilder != NULL)
	{
		m_pGraphBuilder->Release();
		m_pGraphBuilder = NULL;
	}
}

void Music::Remove(void)
{
	m_rEngine.GetMusic().Remove(this);
}

/*----------------------------------------------------------*\
| MusicManager implementation
\*----------------------------------------------------------*/

MusicManager::MusicManager(Engine& rEngine):
						   ResourceManager<Music>(rEngine)
{
}

Music* MusicManager::Load(LPCWSTR pszPath, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return NULL;
	
	return ResourceManager<Music>::Load(pszPath, fPersistenceTime);
}

Music* MusicManager::LoadInstance(Stream& rStream, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return NULL;

	return ResourceManager<Music>::LoadInstance(rStream, fPersistenceTime);
}

Music* MusicManager::LoadInstance(const InfoElem& rElem, float fPersistenceTime)
{
	if (m_rEngine.GetOption(Engine::OPTION_DISABLE_MUSIC) == TRUE)
		return NULL;

	return ResourceManager<Music>::LoadInstance(rElem, fPersistenceTime);
}