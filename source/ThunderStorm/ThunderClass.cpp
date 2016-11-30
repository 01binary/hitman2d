/*------------------------------------------------------------------*\
|
| ThunderClass.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine class factory implementation
| Created: 10/17/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "ThunderClass.h"	// defining Class, using String
#include "ThunderEngine.h"	// using Object, using Engine

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| ClassManager class implementation
\*----------------------------------------------------------*/

ClassManager::ClassManager(Engine& rEngine): m_rEngine(rEngine)
{
}

ClassManager::~ClassManager(void)
{
	Empty();
}

void ClassManager::Register(LPCWSTR pszClass, PCREATECLASSCALLBACK pCreateCallback)
{
	m_mapClasses[pszClass] = pCreateCallback;
}

void ClassManager::Unregister(LPCWSTR pszClass)
{
	CallbackMapIterator posFind = m_mapClasses.find(pszClass);

	if (posFind != m_mapClasses.end())
		m_mapClasses.erase(posFind);
}

Object* ClassManager::Create(LPCWSTR pszClass, Object* pOwner)
{
	if (NULL == pszClass || L'\0' == *pszClass)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	CallbackMapIterator posFind = m_mapClasses.find(pszClass);

	if (posFind == m_mapClasses.end())
		throw m_rEngine.GetErrors().Push(Error::CLASS_NOTREGISTERED,
			__FUNCTIONW__, pszClass);

	return m_mapClasses[pszClass](m_rEngine, pszClass, pOwner);
}

CallbackMapIterator ClassManager::GetBeginPos(void)
{
	return m_mapClasses.begin();
}

CallbackMapIterator ClassManager::GetEndPos(void)
{
	return m_mapClasses.end();
}

CallbackMapConstIterator ClassManager::GetBeginPosConst(void) const
{
	return m_mapClasses.begin();
}

CallbackMapConstIterator ClassManager::GetEndPosConst(void) const
{
	return m_mapClasses.end();
}

int ClassManager::GetCount(void) const
{
	return int(m_mapClasses.size());
}

DWORD ClassManager::GetMemoryFootprint(void) const
{
	DWORD dwSize = sizeof(ClassManager);

	for(CallbackMapConstIterator pos = m_mapClasses.begin();
		pos != m_mapClasses.end();
		pos++)
	{
		dwSize += DWORD(pos->first.GetLengthBytes());
	}

	return dwSize;
}

void ClassManager::Empty(void)
{
	m_mapClasses.clear();
}