/*------------------------------------------------------------------*\
|
| ScreenProgress.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Progress Screen class
| Created: 03/03/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_PROGRESS_H
#define SCREEN_PROGRESS_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ScreenOverlapped.h"	// using ScreenOverlapped
#include "ScreenProgressBar.h"	// using ScreenProgressBar
#include "ScreenLabel.h"		// using ScreenLabel

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenProgress class
\*----------------------------------------------------------*/

class ScreenProgress: public ScreenOverlapped
{
public:
	//
	// Constants
	//

	// Control IDs

	enum
	{
		ID = 1011,

		ID_STATUS = 101,
		ID_PROGRESS = 102,
		ID_CANCEL = 103,
		ID_ANIM = 104
	};

	// Class Name
	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Loading/saving session?
	bool m_bSession;

	// Cancel button was clicked?
	bool m_bCancel;

	// Progress items (TODO: multiple maps)
	float m_fProgress[Client::PROGRESS_COUNT];

	// Cached pointer to progress bar child screen
	ScreenProgressBar* m_pScreenProgress;

	// Cached pointer to status label child screen
	ScreenLabel* m_pScreenStatus;

public:
	ScreenProgress(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	~ScreenProgress(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Progress
	//

	void SetProgress(Client::ProgressTypes nType,
		Client::ProgressSubTypes nSubType,
		int nProgress, int nProgressMax);

	bool IsAborted(void) const;

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnCommand(int nCommandID,
		Screen* pSender = NULL, int nParam = 0);
};

} // namespace Hitman2D

#endif // SCREEN_PROGRESS_H