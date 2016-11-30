/*------------------------------------------------------------------*\
|
| ThunderResource.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm resource classes implementation
| Created: 03/31/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderGlobals.h"			// using GetAbsolutePath
#include "ThunderEngine.h"			// using Engine, defining Resource
#include "ThunderInfoFile.h"		// using InfoFile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Resource implementation
\*----------------------------------------------------------*/

Resource::Resource(Engine& rEngine):
				   Object(rEngine),
				   m_nRefCount(0),
				   m_fDiscardTime(-1.0f),
				   m_fPersistenceTime(float(m_rEngine.GetOption(
						Engine::OPTION_RESOURCE_CACHE_DURATION)))
{
}

Resource::~Resource(void)
{
}

void Resource::SerializeInstance(Stream& rStream) const
{
	try
	{
		// Make sure the path is relative to current directory

		String strRelPath;
		strRelPath.Allocate(MAX_PATH);

		GetRelativePath(m_strName, strRelPath);

		// Write path

		strRelPath.Serialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Resource::SerializeInstance(InfoElem& rElem) const
{
	try
	{
		// Make sure the path is relative to current directory

		if (PathIsRelative(m_strName) == FALSE)
		{
			String strRelPath;
			strRelPath.Allocate(MAX_PATH);

			GetRelativePath(m_strName, strRelPath);

			// Write path

			rElem.SetStringValue(strRelPath);
		}
		else
		{
			// Write path

			rElem.SetStringValue(m_strName);
		}
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rElem.GetDocumentConst().GetPath());
	}
}

void Resource::DeserializeInstance(Stream& rStream)
{
	try
	{
		// Read name

		m_strName.Deserialize(rStream);

		// Read from relative path in the name

		Deserialize(m_strName);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Resource::DeserializeInstance(const InfoElem& rElem)
{
	try
	{
		// Read from relative path

		Deserialize(rElem.GetStringValue());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rElem.GetDocumentConst().GetPath());
	}
}

void Resource::OnLostDevice(bool bRecreate)
{
	UNREFERENCED_PARAMETER(bRecreate);
}

void Resource::OnResetDevice(bool bRecreate)
{
	UNREFERENCED_PARAMETER(bRecreate);
}

DWORD Resource::GetMemoryFootprint(void) const
{
	return Object::GetMemoryFootprint() -
			sizeof(Object) +
			sizeof(Resource);
}

int Resource::Release(void)
{
	m_nRefCount--;

	if (0 == m_nRefCount)
	{
		// When no longer referenced, remove from resource map

		Remove();

		return 0;
	}

	return m_nRefCount;
}

void Resource::Remove(void)
{
	// Must be subclassed to remove from appropriate resource manager
}

/*----------------------------------------------------------*\
| ResourceCache implementation
\*----------------------------------------------------------*/

ResourceCache::ResourceCache(void)
{
}

ResourceCache::~ResourceCache(void)
{
	Empty();
}

Resource* ResourceCache::Retrieve(LPCWSTR pszName)
{
	ResourceMapIterator pos = m_mapResources.find(pszName);

	if (pos == m_mapResources.end())
		return NULL;

	return pos->second;
}

void ResourceCache::Cache(Resource* pResource)
{
	m_mapResources[pResource->GetName()] = pResource;
}

void ResourceCache::Evict(LPCWSTR pszName, bool bDeallocate)
{
	ResourceMapIterator pos = m_mapResources.find(pszName);

	if (pos != m_mapResources.end())
	{
		if (bDeallocate) delete pos->second;
		m_mapResources.erase(pos);
	}
}

void ResourceCache::EvictAll(void)
{
	for(ResourceMapIterator pos = m_mapResources.begin();
		pos != m_mapResources.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapResources.clear();
}

void ResourceCache::Update(void)
{
	ResourceMapIterator pos = m_mapResources.begin();

	while(pos != m_mapResources.end())
	{
		if ((pos->second->GetEngine().GetRunTime() -
			pos->second->GetDiscardTime()) >
			pos->second->GetPersistenceTime())
		{
			ResourceMapIterator posNext = pos;
			posNext++;

			delete pos->second;
			m_mapResources.erase(pos);

			pos = posNext;

			continue;
		}

		pos++;
	}
}

void ResourceCache::OnLostDevice(bool bRecreate)
{
	// Doesn't make sense to reload a bunch of trash

	EvictAll();
}

void ResourceCache::OnResetDevice(bool bRecreate)
{
	UNREFERENCED_PARAMETER(bRecreate);
}

DWORD ResourceCache::GetMemoryFootprint(void) const
{
	DWORD dwSize = sizeof(ResourceCache);

	for(ResourceMapIterator::const_iterator pos = m_mapResources.begin();
		pos != m_mapResources.end();
		pos++)
	{
		dwSize += pos->second->GetMemoryFootprint();
	}

	return dwSize;
}

void ResourceCache::Empty(void)
{
	EvictAll();
}