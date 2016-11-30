/*------------------------------------------------------------------*\
|
| ThunderTimer.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine timer classes
| Created: 07/11/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_TIMER_H
#define THUNDER_TIMER_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Screen;			// referencing Screen
class TimerManager;		// referencing TimerManager, declared below
class Timer;			// referencing Timer, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::list<Timer*> TimerList;
typedef std::list<Timer*>::iterator TimerListIterator;
typedef std::list<Timer*>::const_iterator TimerListConstIterator;


/*----------------------------------------------------------*\
| Timer class
\*----------------------------------------------------------*/

class Timer
{
private:
	// Pointer to screen that will handle timer event
	Screen* m_pTarget;

	// ID of this timer	
	int m_nID;

	// Timer's interval
	float m_fInterval;

	// Last time this timer was fired in engine time
	float m_fLastFired;

private:

	// Private constructor because we only want TimerManager
	// to handle creating these

	Timer(Screen* pTarget, int nID, float fInterval);
public:
	//
	// Target
	//

	inline Screen* GetTarget(void) const
	{
		return m_pTarget;
	}

	inline void SetTarget(Screen* pTarget)
	{
		m_pTarget = pTarget;
	}

	//
	// ID
	//

	inline int GetID(void) const
	{
		return m_nID;
	}

	inline void SetID(int nID)
	{
		m_nID = nID;
	}

	//
	// Interval
	//

	inline float GetInterval(void) const
	{
		return m_fInterval;
	}

	inline void SetInterval(float fInterval)
	{
		m_fInterval = fInterval;
	}

	//
	// Fire
	//

	void Fire(float fTime);

	inline float GetLastFiredTime(void) const
	{
		return m_fLastFired;
	}

	//
	// Friends
	//

	friend class TimerManager;
};

/*----------------------------------------------------------*\
| TimerManager class
\*----------------------------------------------------------*/

class TimerManager
{
private:
	TimerList m_arTimers;

public:
	inline ~TimerManager(void)
	{
		Empty();
	}

public:
	//
	// Management
	//

	Timer* Add(Screen* pTarget, float fInterval, int nID = 0);
	void Remove(Screen* pTarget, int nID = 0);
	void RemoveAll(void);
	
	inline int GetCount(void) const
	{
		return int(m_arTimers.size());
	}

	Timer* Find(Screen* pTarget, int nID = 0);
	const Timer* FindConst(Screen* pTarget, int nID = 0) const;

	inline TimerListIterator GetBeginPos(void)
	{
		return m_arTimers.begin();
	}

	inline TimerListIterator GetEndPos(void)
	{
		return m_arTimers.end();
	}

	inline TimerListConstIterator GetBeginPosConst(void) const
	{
		return m_arTimers.begin();
	}

	inline TimerListConstIterator GetEndPosConst(void) const
	{
		return m_arTimers.end();
	}

	//
	// Update
	//

	void Update(float fTime);

	//
	// Deinitialization
	//

	inline void Empty(void)
	{
		RemoveAll();

		m_arTimers.clear();
	}
};

} // namespace ThunderStorm

#endif // THUNDER_TIMER_H