/*------------------------------------------------------------------*\
|
| ThunderObject.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm object classes header
| Created: 09/03/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_OBJECT_H
#define THUNDER_OBJECT_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderString.h"	// using String

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;				// referencing Engine
class Stream;				// referencing Stream
class InfoElem;				// referencing InfoElem


/*----------------------------------------------------------*\
| Object class - base class only
\*----------------------------------------------------------*/

class Object
{
protected:
	// Maintain a reference to the engine
	Engine& m_rEngine;

	// Flags (meaning depends on derived class)
	DWORD m_dwFlags;

	// Name
	String m_strName;

protected:
	Object(Engine& rEngine);

public:
	virtual ~Object(void);

public:
	//
	// Engine
	//

	inline Engine& GetEngine(void)
	{
		return m_rEngine;
	}

	inline const Engine& GetEngineConst(void) const
	{
		return m_rEngine;
	}

	//
	// Name
	//

	inline const String& GetName(void) const
	{
		return m_strName;
	}

	inline void SetName(LPCWSTR pszName)
	{
		m_strName = pszName;
	}

	//
	// Flags
	//

	inline DWORD GetFlags(void) const
	{
		return m_dwFlags;
	}

	inline bool IsFlagSet(DWORD dwFlag) const
	{
		return m_dwFlags & dwFlag ? true : false;
	}

	virtual void SetFlags(DWORD dwFlags);

	inline void SetFlag(DWORD dwFlag)
	{
		if (~m_dwFlags & dwFlag)
			SetFlags(m_dwFlags | dwFlag);
	}
	
	inline void ClearFlag(DWORD dwFlag)
	{
		if (m_dwFlags & dwFlag)
			SetFlags(m_dwFlags & ~dwFlag);
	}

	//
	// Serialization
	//

	virtual void Serialize(LPCWSTR pszPath) const;
	virtual void Serialize(Stream& rStream) const;

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);

	virtual void SerializeInstance(Stream& rStream) const;
	virtual void SerializeInstance(InfoElem& rElem) const;

	virtual void DeserializeInstance(Stream& rStream);
	virtual void DeserializeInstance(const InfoElem& rElem);

	static void SerializeNullInstance(Stream& rStream);
	static void SerializeNullInstance(InfoElem& rElem);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	
	int Release(void);
};

} // namespace ThunderStorm

#endif // THUNDER_OBJECT_H