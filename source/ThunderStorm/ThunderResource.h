/*------------------------------------------------------------------*\
|
| ThunderResource.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm resource classes header
| Created: 03/31/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_RESOURCE_H
#define THUNDER_RESOURCE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderObject.h"	// using Object, String

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;		// referencing Engine
class Resource;		// referencing Resource, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<String, Resource*> ResourceMap;
typedef std::map<String, Resource*>::iterator ResourceMapIterator;
typedef std::map<String, Resource*>::const_iterator ResourceMapConstIterator;


/*----------------------------------------------------------*\
| Resource class - base for all resources
\*----------------------------------------------------------*/

class Resource: public Object
{
protected:
	//
	// Members
	//

	// Number of times this resource was requested, for garbage collection
	int m_nRefCount;

	// Time when discarded (in run time, seconds) for retired garbage
	float m_fDiscardTime;

	// Persistence time after discard
	float m_fPersistenceTime;

public:
	Resource(Engine& rEngine);
	virtual ~Resource(void);

public:
	//
	// Reference counting
	//

	inline int AddRef(void)
	{
		return ++m_nRefCount;
	}

	inline int GetRefCount(void) const
	{
		return m_nRefCount;
	}

	//
	// Discard Time
	//

	inline float GetDiscardTime(void) const
	{
		return m_fDiscardTime;
	}

	inline void SetDiscardTime(float fDiscardTime)
	{
		m_fDiscardTime = fDiscardTime;
	}

	//
	// Persist Time
	//

	inline float GetPersistenceTime(void) const
	{
		return m_fPersistenceTime;
	}

	inline void SetPersistenceTime(float fPersistenceTime)
	{
		m_fPersistenceTime = fPersistenceTime;
	}

	//
	// Serialization
	//

	virtual void SerializeInstance(Stream& rStream) const;
	virtual void SerializeInstance(InfoElem& rElem) const;

	virtual void DeserializeInstance(Stream& rStream);
	virtual void DeserializeInstance(const InfoElem& rElem);

	inline void Reload(void)
	{
		// Deserialize from the same path

		Deserialize(m_strName);
	}

	//
	// Device Events
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	int Release(void);

	virtual void Remove(void);
};

/*----------------------------------------------------------*\
| ResourceCache class
\*----------------------------------------------------------*/

class ResourceCache
{
private:
	//
	// Members
	//

	ResourceMap m_mapResources;

public:
	ResourceCache(void);
	~ResourceCache(void);

public:
	//
	// Resources
	//

	Resource* Retrieve(LPCWSTR pszName);
	void Cache(Resource* pResource);
	void Evict(LPCWSTR pszName, bool bDeallocate = false);
	void EvictAll(void);

	inline ResourceMapIterator GetBeginPos(void)
	{
		return m_mapResources.begin();
	}

	inline ResourceMapIterator GetEndPos(void)
	{
		return m_mapResources.end();
	}

	inline ResourceMapConstIterator GetBeginPosConst(void) const
	{
		return m_mapResources.begin();
	}

	inline ResourceMapConstIterator GetEndPosConst(void) const
	{
		return m_mapResources.end();
	}

	template<class TResource> int GetCount(void) const
	{
		int nCount = 0;

		for(ResourceMapConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			if (dynamic_cast<TResource*>(pos->second) != NULL)
				nCount++;
		}

		return nCount;
	}

	inline int GetCount(void) const
	{
		return int(m_mapResources.size());
	}

	//
	// Update
	//

	void Update(void);

	//
	// Lost Device
	//

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);

	//
	// Diagnostics
	//

	template<class TResource> DWORD GetMemoryFootprint(void) const
	{
		DWORD dwSize = sizeof(ResourceCache);

		for(ResourceMapConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			if (dynamic_cast<TResource*>(pos->second) != NULL)
			{
				dwSize += DWORD(pos->first.GetLengthBytes());
				dwSize += pos->second->GetMemoryFootprint();
			}
		}

		return dwSize;
	}

	DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| ResourceManager template class
\*----------------------------------------------------------*/

template<class TResource> class ResourceManager
{
public:
	//
	// Definitions
	//

	typedef typename std::map<String, TResource*>::iterator Iterator;
	typedef typename std::map<String, TResource*>::const_iterator ConstIterator;

protected:
	Engine& m_rEngine;

	std::map<String, typename TResource*> m_mapResources;

	WCHAR m_szBasePath[128];
	WCHAR m_szBaseExt[8];

public:
	ResourceManager(Engine& rEngine): m_rEngine(rEngine)
	{
		ZeroMemory(m_szBasePath, sizeof(m_szBasePath));
		ZeroMemory(m_szBaseExt, sizeof(m_szBaseExt));
	}

	~ResourceManager(void)
	{
		Empty();
	}

public:
	//
	// Resource management
	//

	TResource* Load(LPCWSTR pszPath, float fPersistenceTime = -1.0f)
	{
		// Validate

		if (String::IsEmpty(pszPath) == true)
		{
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__, 0);
			return NULL;
		}

		// Make sure the path is a full path

		WCHAR szFullPath[MAX_PATH] = {0};

		if (*(PathFindExtension(pszPath)) != L'\0')
		{
			GetAbsolutePath(pszPath, szFullPath);
		}
		else
		{
			m_rEngine.GetBaseFilePath(pszPath,
				m_szBasePath, m_szBaseExt, szFullPath);
		}
		
		// Check if this resource is already loaded

		Iterator pos = m_mapResources.find(szFullPath);

		if (m_mapResources.end() == pos)
		{
			// Not loaded

			if (TRUE == m_rEngine.GetOption(Engine::OPTION_ENABLE_RESOURCE_CACHE))
			{
				// If caching enabled, check if cached

				TResource* pDiscardable =
					dynamic_cast<TResource*>(
					m_rEngine.GetResourceCache().Retrieve(szFullPath));

				if (pDiscardable	!= NULL)
				{
					// If was cached, move back to main list

					m_rEngine.GetResourceCache().Evict(szFullPath, false);

					Add(pDiscardable);

					pDiscardable->SetDiscardTime(-1.0f);

					return pDiscardable;
				}
			}

			// Load this resource

			TResource* pNewResource = NULL;

			try
			{

				// Load this resource

				pNewResource = Create();

				pNewResource->SetName(szFullPath);

				pNewResource->Deserialize(szFullPath);

				Add(pNewResource);

				// If non-default persistence time specified, set it
				// Note: default persistence time is set in Resource ctor

				if (fPersistenceTime >= 0.0f)
					pNewResource->SetPersistenceTime(fPersistenceTime);

				return pNewResource;
			}

			catch(Error& rError)
			{
				UNREFERENCED_PARAMETER(rError);

				if (pNewResource != NULL)
					delete pNewResource;
				
				throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
					__FUNCTIONW__, L"pNewResource");
			}
		}
			
		// If already loaded, update persistence time and return a pointer

		if (fPersistenceTime >= 0.0f)
			pos->second->SetPersistenceTime(fPersistenceTime);		

		return pos->second;
	}

	TResource* LoadInstance(Stream& rStream, float fPersistenceTime = -1.0f)
	{
		// Read path to load from

		String strPath;
		strPath.Deserialize(rStream);

		// Throw error if null instance encountered

		if (strPath.IsEmpty() == true)
			throw m_rEngine.GetErrors().Push(Error::FILE_NULLINSTANCE,
				__FUNCTIONW__, rStream.GetPath());

		// Load from that path

		return Load(strPath, fPersistenceTime);
	}

	TResource* LoadInstance(const InfoElem& rElem, float fPersistenceTime = -1.0f)
	{
		if (rElem.GetVarType() != InfoElem::TYPE_STRING)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
				__FUNCTIONW__, rElem.GetName(), rElem.GetVarTypeString());

		String strPath = rElem.GetStringValue();

		if (strPath.IsEmpty() == true)
			throw m_rEngine.GetErrors().Push(Error::FILE_NULLINSTANCE,
				__FUNCTIONW__, rElem.GetDocumentConst().GetPath());
		
		return Load(strPath, fPersistenceTime);		
	}

	TResource* Create(void)
	{
		TResource* pNewRes = NULL;

		try
		{
			pNewRes = new TResource(m_rEngine);
		}

		catch(std::bad_alloc e)
		{
			throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
				__FUNCTIONW__, sizeof(TResource));
		}

		return pNewRes;
	}

	TResource* Find(LPCWSTR pszPath)
	{
		if (NULL == pszPath || L'\0' == *pszPath)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);

		WCHAR szFullPath[MAX_PATH] = {0};

		if (*(PathFindExtension(pszPath)) != L'\0')
		{
			GetAbsolutePath(pszPath, szFullPath);
		}
		else
		{
			m_rEngine.GetBaseFilePath(pszPath, m_szBasePath,
				m_szBaseExt, szFullPath);
		}

		Iterator pos = m_mapResources.find(pszPath);

		if (pos != m_mapResources.end())
			return pos->second;

		return NULL;
	}

	TResource* FindPattern(LPCWSTR pszSearchPattern)
	{
		if (NULL == pszSearchPattern || L'\0' == *pszSearchPattern)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);

		for(ConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			if (PathMatchSpec(pos->first, pszSearchPattern) == TRUE)
				return static_cast<TResource*>(pos->second);
		}

		return NULL;
	}

	void Add(TResource* pResource)
	{
		if (NULL == pResource)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);

		Iterator posRes =
			m_mapResources.find(pResource->GetName());

		if (posRes != m_mapResources.end())
			return;

		m_mapResources[pResource->GetName()] = pResource;
	}

	void Remove(TResource* pResource)
	{
		if (NULL == pResource)
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 0);

		Iterator pos =
			m_mapResources.find(pResource->GetName());

		if (pos != m_mapResources.end())
		{
			if (m_rEngine.GetOption(Engine::OPTION_ENABLE_RESOURCE_CACHE) &&
			   pResource->GetPersistenceTime() > 0.0f)
			{
				// If resource caching is enabled, cache resource instead of deleting it

				pResource->SetDiscardTime(m_rEngine.GetRunTime());

				m_rEngine.GetResourceCache().Cache(pResource);
			}
			else
			{
				// Deallocate resource if no references - otherwise release.

				if (pResource->GetRefCount() == 0)
					delete pResource;
				else
					pResource->Release();
			}

			// Remove from map

			m_mapResources.erase(pos);
		}
	}

	void RemoveAll(void)
	{
		// Deallocate all resources without regard for reference count

		for(Iterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			delete pos->second;

			pos->second = NULL;
		}

		m_mapResources.clear();
	}

	Iterator GetBeginPos(void)
	{
		return m_mapResources.begin();
	}

	Iterator GetEndPos(void)
	{
		return m_mapResources.end();
	}

	ConstIterator GetBeginPosConst(void) const
	{
		return m_mapResources.begin();
	}

	ConstIterator GetEndPosConst(void) const
	{
		return m_mapResources.end();
	}

	int GetCount(void) const
	{
		return int(m_mapResources.size());
	}

	LPCWSTR GetBasePath(void) const
	{
		return m_szBasePath;
	}

	LPCWSTR GetBaseExtension(void) const
	{
		return m_szBaseExt;
	}

	void SetBasePath(LPCWSTR pszBasePath)
	{
		wcscpy_s(m_szBasePath, sizeof(m_szBasePath) / sizeof(WCHAR), pszBasePath);
	}

	void SetBaseExtension(LPCWSTR pszBaseExt)
	{
		wcscpy_s(m_szBaseExt, sizeof(m_szBaseExt) / sizeof(WCHAR), pszBaseExt);
	}

	void OnLostDevice(bool bRecreate)
	{
		for(ConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			if (pos->second != NULL)
				pos->second->OnLostDevice(bRecreate);
		}
	}

	void OnResetDevice(bool bRecreate)
	{
		for(ConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			if (pos->second != NULL)
				pos->second->OnResetDevice(bRecreate);
		}
	}

	DWORD GetMemoryFootprint(void) const
	{
		DWORD dwSize = 0;
	
		for(ConstIterator pos = m_mapResources.begin();
			pos != m_mapResources.end();
			pos++)
		{
			dwSize += DWORD(pos->first.GetLengthBytes());
			dwSize += pos->second->GetMemoryFootprint();
		}

		return dwSize;
	}

	//
	// Deinitialization
	//

	void Empty(void)
	{
		RemoveAll();
	}
};

} // namespace ThunderStorm

#endif