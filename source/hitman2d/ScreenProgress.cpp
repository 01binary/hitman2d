/*------------------------------------------------------------------*\
|
| ScreenProgress.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Progress Screen implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "Error.h"				// using error constants
#include "ScreenProgress.h"		// defining ScreenProgress

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenProgress::SZ_CLASS[] = L"overlapped::progress";


/*----------------------------------------------------------*\
| ScreenProgress implementation
\*----------------------------------------------------------*/

ScreenProgress::ScreenProgress(Engine& rEngine,
							   LPCWSTR pszClass,
							   Screen* pParent):

							   ScreenOverlapped(rEngine, pszClass, pParent),
							   m_bSession(false),
							   m_bCancel(false),
							   m_pScreenProgress(NULL),
							   m_pScreenStatus(NULL)
{
	for(int n = 0; n < Client::PROGRESS_COUNT; n++)
	{
		m_fProgress[n] = 0.0f;
	}
}

ScreenProgress::~ScreenProgress(void)
{
}

Object* ScreenProgress::CreateInstance(Engine& rEngine,
									   LPCWSTR pszClass,
									   Object* pParent)
{
	return new ScreenProgress(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenProgress::SetProgress(Client::ProgressTypes nType,
								 Client::ProgressSubTypes nSubType,
								 int nProgress,
								 int nProgressMax)
{
	if (true == m_bCancel) return;

	float fTotalProgress = 0.0f;

	if (Client::PROGRESS_SESSION == nSubType)
	{
		// Set loading session flag if not set

		m_bSession = true;

		if (nProgress == nProgressMax)
		{
			// If done loading session, close

			fTotalProgress = 1.0f;

			SetFlag(DISABLED);

			Close();
		}
	}

	if (Client::PROGRESS_MAP == nSubType && false == m_bSession)
	{
		// If done loading map and not loading session, close

		if (nProgress == nProgressMax)
		{
			fTotalProgress = 1.0f;

			SetFlag(DISABLED);

			Close();
		}
	}
	else
	{
		// Update progress subitem

		m_fProgress[nSubType] = float(nProgress) / float(nProgressMax);
		
		// Add up everything loaded so far

		for(int n = 0; n < Client::PROGRESS_COUNT; n++)
		{
			fTotalProgress += m_fProgress[n];
		}

		if (true == m_bSession)
			fTotalProgress /= float(Client::PROGRESS_COUNT);
		else
			fTotalProgress /= float(Client::PROGRESS_COUNT - 3);
	}

	// Set progress

	m_pScreenProgress->SetProgress(int(fTotalProgress * 100.0f));

	// Set status text

	WCHAR szMsg[MAX_PATH] = {0};

	StringCchPrintf(szMsg, MAX_PATH, L"%s %s (%d%%)",
		SZ_PROGRESS_TYPES[nType], SZ_PROGRESS_SUBTYPES[nSubType],
		int(m_fProgress[nSubType] * 100.0f));

	m_pScreenStatus->SetText(szMsg);

	// Wait so we can see the progress screen (temporary of course)

	m_rEngine.GetClientInstance()->Wait(0.1f);
}

bool ScreenProgress::IsAborted(void) const
{
	return m_bCancel;
}

void ScreenProgress::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	m_pScreenProgress =
		dynamic_cast<ScreenProgressBar*>(m_lstChildren.FindByID(ID_PROGRESS));

	if (NULL == m_pScreenProgress)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pScreenProgress");

	m_pScreenStatus =
		dynamic_cast<ScreenLabel*>(m_lstChildren.FindByID(ID_STATUS));

	if (NULL == m_pScreenStatus)
		m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pScreenStatus");
}

DWORD ScreenProgress::GetMemoryFootprint(void) const
{
	return sizeof(ScreenProgress) -
		sizeof(ScreenOverlapped) * 2 +
		ScreenOverlapped::GetMemoryFootprint();
}

void ScreenProgress::OnCommand(int nCommandID,
							   Screen* pSender,
							   int nParam)
{
	if (ID_CANCEL == nCommandID)
	{
		// Cancel button clicked

		m_bCancel = true;

		SetFlag(DISABLED);

		Screen* pAnimChild = m_lstChildren.FindByID(ID_ANIM);

		if (pAnimChild != NULL)
			pAnimChild->GetBackground().TriggerSequence(L"end", false);

		Close();
	}
}