/*------------------------------------------------------------------*\
|
| ThunderVideo.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine video class implementation
| Created: 11/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderVideo.h"		// defining Video

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Video implementation
\*----------------------------------------------------------*/

Video::Video(Engine& rEngine): Resource(rEngine),
							   m_pGraphBuilder(NULL),
							   m_pWindowlessControl(NULL),
							   m_pMediaControl(NULL),
							   m_pMediaPosition(NULL),
							   m_pMediaEvent(NULL),
							   m_pBasicAudio(NULL)
{
}

Video::~Video(void)
{
	Empty();
}

void Video::Deserialize(LPCWSTR pszPath)
{
	Empty();

	m_strName = pszPath;

	IBaseFilter* pVMRFilter = NULL;
	IVMRFilterConfig* pVMRFilterConfig = NULL;

	try
	{
		// Create the filter graph

		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			IID_IGraphBuilder, (LPVOID*)&m_pGraphBuilder);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPHCREATE,
				__FUNCTIONW__, hr);

		// Query media control, media event, and media position interfaces

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl,
			(void**)&m_pMediaControl);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_FILTERGRAPH_MEDIACONTROLQUERY, __FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaEvent, (void**)&m_pMediaEvent);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
			Error::DSHOW_FILTERGRAPH_MEDIAEVENTQUERY, __FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IMediaPosition,
			(void**)&m_pMediaPosition);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_FILTERGRAPH_MEDIAPOSITIONQUERY, __FUNCTIONW__, hr);

		hr = m_pGraphBuilder->QueryInterface(IID_IBasicAudio,
			(void**)&m_pBasicAudio);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_FILTERGRAPH_BASICAUDIOQUERY, __FUNCTIONW__, hr);

		// Create an instance VMR7 filter that can play video		

		hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
			CLSCTX_INPROC, IID_IBaseFilter, (LPVOID*)&pVMRFilter);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_VMRCREATE,
				__FUNCTIONW__, hr);

		// Add VMR7 filter to the filter graph

		hr = m_pGraphBuilder->AddFilter(pVMRFilter, L"Video Mixing Renderer");

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_ADDFILTER,
				__FUNCTIONW__, hr);

		// Set VMR filter's rendering mode

		hr = pVMRFilter->QueryInterface(IID_IVMRFilterConfig, (LPVOID*)&pVMRFilterConfig);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_VMR_QUERYFILTERCONFIG,
				__FUNCTIONW__, hr);

		hr = pVMRFilterConfig->SetRenderingMode(VMRMode_Windowless);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_VMR_FILTERCONFIG_SETRENDERINGMODE,
					__FUNCTIONW__, hr);

		pVMRFilterConfig->Release();
		pVMRFilterConfig = NULL;

		// Set VMR's clipping window

		hr = pVMRFilter->QueryInterface(IID_IVMRWindowlessControl,
			(LPVOID*)&m_pWindowlessControl);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_VMR_QUERYWINDOWLESSCONTROL,
				__FUNCTIONW__, hr);

		hr = m_pWindowlessControl->SetVideoClippingWindow(
			m_rEngine.GetGameWindow());

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_VMR_WINDOWLESSCONTROL_SETCLIPPINGWINDOW,
				__FUNCTIONW__, hr);

		pVMRFilter->Release();
		pVMRFilter = NULL;

		// Build the rest of filter graph automatically

		hr = m_pGraphBuilder->RenderFile(pszPath, NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_FILTERGRAPH_RENDERFILE,
				__FUNCTIONW__, hr);

		// Set window for notifications

		hr = m_pMediaEvent->SetNotifyWindow((OAHWND)m_rEngine.GetGameWindow(),
			WM_DSHOWNOTIFY, 0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_MEDIAEVENT_SETNOTIFYWINDOW, __FUNCTIONW__, hr);

		// Turn notifications on

		hr = m_pMediaEvent->SetNotifyFlags(0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_MEDIAEVENT_SETNOTIFYFLAGS, __FUNCTIONW__, hr);

		// Set default source and destination rectangles (best fit by default)

		long lWidth = 0;
		long lHeight = 0;

		hr = m_pWindowlessControl->GetNativeVideoSize(&lWidth, &lHeight,
			NULL, NULL);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_VMR_WINDOWLESSCONTROL_GETNATIVEVIDEOSIZE,
				__FUNCTIONW__, hr);

		SetRect(&m_rcSrc, 0, 0, lWidth, lHeight);
		SetRectEmpty(&m_rcDest);

		long lClientWidth =
			long(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth);

		long lClientHeight =
			long(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight);

		if (lWidth > lHeight)
		{
			// Best fit by height

			m_rcDest.bottom = lClientHeight;
			m_rcDest.right = long((float(lWidth) / float(lHeight)) *
				float(lClientHeight));

			// Center by width

			m_rcDest.left = (lClientWidth - m_rcDest.right) / 2;
			m_rcDest.right += m_rcDest.left;
		}
		else
		{
			// Best fit by width

			m_rcDest.right = lClientWidth;
			m_rcDest.bottom = long((float(lHeight) / float(lWidth)) *
				float(lClientWidth));

			// Center by height

			m_rcDest.top = (lClientHeight - m_rcDest.bottom) / 2;
			m_rcDest.bottom += m_rcDest.top;
		}

		hr = m_pWindowlessControl->SetVideoPosition(&m_rcSrc, &m_rcDest);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(
				Error::DSHOW_VMR_WINDOWLESSCONTROL_SETVIDEOPOSITION,
				__FUNCTIONW__, hr);
	}

	catch(Error& rError)
	{
		if (m_pWindowlessControl != NULL)
		{
			m_pWindowlessControl->Release();
			m_pWindowlessControl = NULL;
		}

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

		if (m_pBasicAudio != NULL)
		{
			m_pBasicAudio->Release();
			m_pBasicAudio = NULL;
		}

		if (pVMRFilterConfig != NULL)
		{
			pVMRFilterConfig->Release();
			pVMRFilterConfig = NULL;
		}

		if (pVMRFilter != NULL)
		{
			pVMRFilter->Release();
			pVMRFilter = NULL;
		}	

		if (m_pGraphBuilder != NULL)
		{
			m_pGraphBuilder->Release();
			m_pGraphBuilder = NULL;
		}

		throw rError;
	}
}

DWORD Video::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() - sizeof(Video);

	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(m_strName, GENERIC_READ, OPEN_EXISTING, 0);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, m_strName);
	}

	dwSize += stream.GetSize();

	return dwSize;
}

void Video::Play(void)
{
	// Clear device

 	if (FALSE == m_rEngine.GetGraphics().GetDeviceParams().Windowed)
		m_rEngine.GetGraphics().Clear(m_rEngine.GetBackColor());

	// Repaint the video to initiate video updates

	RECT rcClient;
	GetClientRect(m_rEngine.GetGameWindow(), &rcClient);
	InvalidateRect(m_rEngine.GetGameWindow(), &rcClient, true);

	// Set as currently playing video

	m_rEngine.SetCurrentVideo(this);

	// Start playing

	SetFlag(PLAYING);

	HRESULT hr = m_pMediaControl->Run();

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_RUN,
			__FUNCTIONW__, hr);
}

void Video::Stop(void)
{
	if (IsFlagSet(PLAYING) == true)
	{
		HRESULT hr = m_pMediaControl->Stop();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIACONTROL_STOP,
				__FUNCTIONW__, hr);

		hr = m_pMediaPosition->put_CurrentPosition(0.0);

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::DSHOW_MEDIAPOSITION_PUTCURRENT,
				__FUNCTIONW__, hr);

		ClearFlag(PLAYING);

		m_rEngine.SetCurrentVideo(NULL);
	}
	else
	{
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);
	}
}

void Video::Pause(bool bPause)
{
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
		Play();
	}
}

void Video::Restart(void)
{
	HRESULT hr = m_pMediaPosition->put_CurrentPosition(0.0);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::DSHOW_MEDIAPOSITION_PUTCURRENT, __FUNCTIONW__, hr);

	hr = m_pMediaControl->Run();

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::DSHOW_MEDIACONTROL_RUN, __FUNCTIONW__, hr);
}

const RECT& Video::GetSrcVideoPosition(void) const
{
	return m_rcSrc;
}

const RECT& Video::GetDestVideoPosition(void) const
{
	return m_rcDest;
}

void Video::SetSrcVideoPosition(const RECT& rcSrcPos)
{
	CopyRect(&m_rcSrc, &rcSrcPos);

	HRESULT hr = m_pWindowlessControl->SetVideoPosition(&m_rcSrc, &m_rcDest);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::DSHOW_VMR_WINDOWLESSCONTROL_SETVIDEOPOSITION,
			__FUNCTIONW__, hr);
}

void Video::SetDestVideoPosition(const RECT& rcDestPos)
{
	CopyRect(&m_rcDest, &rcDestPos);

	HRESULT hr = m_pWindowlessControl->SetVideoPosition(&m_rcSrc, &m_rcDest);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::DSHOW_VMR_WINDOWLESSCONTROL_SETVIDEOPOSITION, __FUNCTIONW__, hr);
}

void Video::SetVideoPosition(const RECT& rcSrcPos, const RECT& rcDestPos)
{
	CopyRect(&m_rcSrc, &rcSrcPos);
	CopyRect(&m_rcDest, &rcDestPos);

	HRESULT hr = m_pWindowlessControl->SetVideoPosition(&m_rcSrc, &m_rcDest);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(
			Error::DSHOW_VMR_WINDOWLESSCONTROL_SETVIDEOPOSITION, __FUNCTIONW__, hr);
}

void Video::SetVolume(float fVolume)
{
	if (fVolume < 0.0f)
		fVolume = -fVolume;

	long lVolume = Music::DSHOW_VOLUMEMIN +
		long(fVolume * (Music::DSHOW_VOLUMEMAX - Music::DSHOW_VOLUMEMIN));

	HRESULT hr = m_pBasicAudio->put_Volume(lVolume);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_BASICAUDIO_PUTVOLUME,
			__FUNCTIONW__, hr);
}

float Video::GetVolume(void)
{
	long lVolume = 0;

	HRESULT hr = m_pBasicAudio->get_Volume(&lVolume);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::DSHOW_BASICAUDIO_GETVOLUME,
			__FUNCTIONW__, hr);

	return (float(lVolume - Music::DSHOW_VOLUMEMIN) /
		float(Music::DSHOW_VOLUMEMAX - Music::DSHOW_VOLUMEMIN));
}

void Video::Empty(void)
{
	if (IsFlagSet(PLAYING) == true)
		Stop();

	if (m_pWindowlessControl != NULL)
	{
		m_pWindowlessControl->Release();
		m_pWindowlessControl = NULL;
	}

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

	if (m_pBasicAudio != NULL)
	{
		m_pBasicAudio->Release();
		m_pBasicAudio = NULL;
	}

	if (m_pGraphBuilder != NULL)
	{
		m_pGraphBuilder->Release();
		m_pGraphBuilder = NULL;
	}
}

void Video::Remove(void)
{
	m_rEngine.GetVideos().Remove(this);
}