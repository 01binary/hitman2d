/*------------------------------------------------------------------*\
|
| ThunderVideo.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine video class
| Created: 11/24/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_VIDEO_H
#define THUNDER_VIDEO_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"		// using Resource
#include "ThunderMusic.h"			// using various constants

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {


/*----------------------------------------------------------*\
| Video class - windowless video playback
\*----------------------------------------------------------*/

class Video: public Resource
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// No flags set
		DEFAULT		= 0,

		// Playing
		PLAYING		= 1 << 0,

		// Suspended due to session pause
		SUSPENDED	= 1 << 1
	};

private:
	//
	// Members
	//

	IGraphBuilder* m_pGraphBuilder;
	IVMRWindowlessControl* m_pWindowlessControl;
	IMediaControl* m_pMediaControl;
	IMediaPosition* m_pMediaPosition;
	IMediaEventEx* m_pMediaEvent;
	IBasicAudio* m_pBasicAudio;

	RECT m_rcSrc;
	RECT m_rcDest;

public:
	Video(Engine& rEngine);
	virtual ~Video(void);

public:
	//
	// Playback
	//

	void Play(void);
	void Stop(void);
	void Pause(bool bPause = true);
	void Restart(void);

	//
	// Video Position
	//

	const RECT& GetSrcVideoPosition(void) const;
	const RECT& GetDestVideoPosition(void) const;
	void SetSrcVideoPosition(const RECT& rcSrcPos);
	void SetDestVideoPosition(const RECT& rcDestPos);
	void SetVideoPosition(const RECT& rcSrcPos, const RECT& rcDestPos);

	//
	// Volume control (0.0 to 1.0)
	//

	float GetVolume(void);
	void SetVolume(float fVolume);

	//
	// Interfaces
	//

	inline IVMRWindowlessControl* GetWindowlessControl(void)
	{
		return m_pWindowlessControl;
	}

	inline IMediaControl* GetMediaControl(void)
	{
		return m_pMediaControl;
	}

	inline IMediaEventEx* GetMediaEvent(void)
	{
		return m_pMediaEvent;
	}

	inline IMediaPosition* GetMediaPosition(void)
	{
		return m_pMediaPosition;
	}

	//
	// Serialization
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

} // namespace ThunderStorm

#endif // THUNDER_VIDEO_H