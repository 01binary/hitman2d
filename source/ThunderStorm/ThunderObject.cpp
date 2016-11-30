/*------------------------------------------------------------------*\
|
| ThunderObject.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm object classes implementation
| Created: 11/01/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderObject.h"		// defining Object
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoElem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Object implementation
\*----------------------------------------------------------*/

Object::Object(Engine& rEngine):
				m_rEngine(rEngine),
				m_dwFlags(0)
{
}

Object::~Object(void)
{
}

void Object::SetFlags(DWORD dwFlags)
{
	m_dwFlags = dwFlags;
}

DWORD Object::GetMemoryFootprint(void) const
{
	return sizeof(Object) + (DWORD)m_strName.GetLengthBytes();
}

void Object::Serialize(LPCWSTR pszPath) const
{
	// Default handler

	UNREFERENCED_PARAMETER(pszPath);

	_ASSERT(false);
}

void Object::Deserialize(LPCWSTR pszPath)
{
	// Default handler

	UNREFERENCED_PARAMETER(pszPath);

	_ASSERT(false);
}

void Object::Serialize(Stream& rStream) const
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);

	_ASSERT(false);
}

void Object::Deserialize(Stream& rStream)
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);

	_ASSERT(false);
}

void Object::SerializeInstance(Stream& rStream) const
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);

	_ASSERT(false);
}

void Object::SerializeInstance(InfoElem& rElem) const
{
	// Default handler

	UNREFERENCED_PARAMETER(rElem);

	_ASSERT(false);
}

void Object::DeserializeInstance(Stream& rStream)
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);

	_ASSERT(false);
}

void Object::DeserializeInstance(const InfoElem &rElem)
{
	// Default handler

	UNREFERENCED_PARAMETER(rElem);

	_ASSERT(false);
}

void Object::SerializeNullInstance(Stream& rStream)
{
	// Write a zero-length string

	String str;
	str.Serialize(rStream);
}

void Object::SerializeNullInstance(InfoElem& rElem)
{
	// Write an empty string element

	rElem.SetStringValue(NULL);
}

void Object::Empty(void)
{
	// Default handler
	// Note: should never clear m_strName in any derived class implementations
}

int Object::Release(void)
{
	// Reference counting not supported by default

	delete this;
	return 0;
}