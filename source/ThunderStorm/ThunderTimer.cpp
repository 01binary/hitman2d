/*------------------------------------------------------------------*\
|
| ThunderTimer.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine timer classes implementation
| Created: 07/11/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderTimer.h"		// defining Timer, TimerManager
#include "ThunderScreen.h"		// using Screen
#include "ThunderEngine.h"		// using Engine

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Timer implementation
\*----------------------------------------------------------*/

Timer::Timer(Screen* pTarget, int nID, float fInterval):
			 m_pTarget(pTarget),							
			 m_nID(nID),
			 m_fInterval(fInterval),
			 m_fLastFired(pTarget ? pTarget->GetEngine().GetRunTime() : 0.0f)
{
}

void Timer::Fire(float fTime)
{
	m_fLastFired = fTime;

	if (m_pTarget != NULL)
		m_pTarget->OnTimer(*this);
}

/*----------------------------------------------------------*\
| TimerManager implementation
\*----------------------------------------------------------*/

Timer* TimerManager::Add(Screen* pTarget, float fInterval, int nID)
{
	if (NULL == pTarget)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	try
	{
		Timer* pNewTimer = new Timer(pTarget, nID, fInterval);

		m_arTimers.push_back(pNewTimer);

		return pNewTimer;
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Timer));
	}	
}

void TimerManager::Remove(Screen* pTarget, int nID)
{
	// Null out the timer, it will be removed on next Update()

	for(TimerListIterator pos = m_arTimers.begin();
		pos != m_arTimers.end();
		pos++)
	{
		if (NULL == *pos) continue;

		if ((*pos)->m_pTarget == pTarget &&
		   (*pos)->m_nID == nID)
		{
			delete *pos;
			*pos = NULL;

			return;
		}
	}
}

void TimerManager::RemoveAll(void)
{
	// Null out all timers, to be removed on next Update()

	for(TimerListIterator pos = m_arTimers.begin();
		pos != m_arTimers.end();
		pos++)
	{
		delete *pos;
		*pos = NULL;
	}
}

Timer* TimerManager::Find(Screen* pTarget, int nID)
{
	for(TimerListIterator pos = m_arTimers.begin();
		pos != m_arTimers.end();
		pos++)
	{
		if ((*pos)->m_pTarget == pTarget &&
		   (*pos)->m_nID == nID)
			return *pos;
	}

	return NULL;
}

const Timer* TimerManager::FindConst(Screen* pTarget, int nID) const
{
	for(TimerListConstIterator pos = m_arTimers.begin();
		pos != m_arTimers.end();
		pos++)
	{
		if ((*pos)->m_pTarget == pTarget &&
		   (*pos)->m_nID == nID)
			return *pos;
	}

	return NULL;
}

void TimerManager::Update(float fTime)
{
	for(TimerListIterator pos = m_arTimers.begin();
		pos != m_arTimers.end();
		pos++)
	{
		while(NULL == *pos)
		{
			pos = m_arTimers.erase(pos);

			if (m_arTimers.end() == pos) return;
		}

		if ((fTime - (*pos)->m_fLastFired) > (*pos)->GetInterval())
			(*pos)->Fire(fTime);
	}
}