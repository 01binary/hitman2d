/*------------------------------------------------------------------*\
|
| ThunderAnimation.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine animation class(es)
| Created: 05/06/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_ANIMATION_H
#define THUNDER_ANIMATION_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource
#include "ThunderMath.h"		// using Vector2
#include "ThunderVariable.h"	// using VariableMap

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Animation;		// referencing Animation, declared below
class Sprite;			// referencing Sprite
class Texture;			// referencing Texture
class Region;			// referencing Region
class RegionSet;		// referencing RegionSet
class Frame;			// referencing Frame, declared below
class Sequence;			// referencing Sequence, declared below
class Stream;			// referencing Stream
class InfoElem;			// referencing InfoElem
class ErrorManager;		// referencing ErrorManager

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Animation*> AnimationArray;
typedef std::vector<Animation*>::iterator AnimationArrayIterator;
typedef std::vector<Animation*>::const_iterator AnimationArrayConstIterator;

typedef std::vector<Frame> FrameArray;
typedef std::vector<Frame>::iterator FrameArrayIterator;
typedef std::vector<Frame>::const_iterator FrameArrayConstIterator;

typedef std::vector<Sequence> SequenceArray;
typedef std::vector<Sequence>::iterator SequenceArrayIterator;
typedef std::vector<Sequence>::const_iterator SequenceArrayConstIterator;

typedef std::map<String, int> StringIntMap;
typedef std::map<String, int>::iterator StringIntMapIterator;
typedef std::map<String, int>::const_iterator StringIntMapConstIterator;


/*----------------------------------------------------------*\
| Animation class
\*----------------------------------------------------------*/

class Animation: public Resource
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_ANIMATION[];
	static const WCHAR SZ_BASETEX[];
	static const WCHAR SZ_FRAMESIZE[];
	static const WCHAR SZ_FRAMERATE[];
	static const WCHAR SZ_FRAME[];
	static const WCHAR SZ_SEQUENCE[];
	static const WCHAR SZ_REGIONSET[];

protected:
	//
	// Members
	//

	Sprite* m_pOwner;					// Keep pointer to sprite, if owned by sprite

	Texture* m_pTexture;				// Base texture used for all frames

	float m_fDuration;					// Total duration in seconds

	SIZE m_frameSize;					// Size of each frame in pixels
	Vector2 m_vecSize;					// Size of each frame in tiles

	SequenceArray m_arSequences;		// Sequences defined within the animation, if any
	StringIntMap m_mapSequencesByName;	// Sequences mapped by name

	FrameArray m_arFrames;				// Frames	

public:
	// Create stand-alone
	Animation(Engine& rEngine);

	// Create embedded
	Animation(Sprite* pOwner);

	virtual ~Animation(void);

public:
	//
	// Sprite
	//

	inline Sprite* GetOwner(void)
	{
		return m_pOwner;
	}

	inline const Sprite* GetOwnerConst(void) const
	{
		return m_pOwner;
	}

	//
	// Texture
	//

	inline Texture* GetBaseTexture(void)
	{
		return m_pTexture;
	}

	inline const Texture* GetBaseTexture(void) const
	{
		return m_pTexture;
	}

	void SetBaseTexture(Texture* pTexture);

	//
	// Duration
	//

	inline float GetDuration(void) const
	{
		return m_fDuration;
	}

	//
	// Size
	//

	inline SIZE GetFrameSize(void) const
	{
		return m_frameSize;
	}

	void SetFrameSize(SIZE frameSize);

	inline const Vector2& GetFrameSizeTiles(void) const
	{
		return m_vecSize;
	}

	void SetSizeTiles(const Vector2& rvecSize);

	//
	// Sequences
	//

	int AddSequence(const Sequence& rSequence);

	inline Sequence& GetSequence(int nSequenceID)
	{
		return m_arSequences[nSequenceID];
	}

	inline Sequence& GetSequence(LPCWSTR pszSequence)
	{
		return m_arSequences[m_mapSequencesByName[pszSequence]];
	}

	inline const Sequence& GetSequenceConst(int nSequenceID) const
	{
		return m_arSequences[nSequenceID];
	}

	inline const Sequence& GetSequenceConst(LPCWSTR pszSequence) const
	{
		StringIntMapConstIterator posFind = m_mapSequencesByName.find(pszSequence);
		return m_arSequences[posFind->second];
	}

	void RemoveSequence(int nSequenceID);

	void RemoveSequence(LPCWSTR pszSequence);
	int GetSequenceID(LPCWSTR pszSequence) const;

	inline void RemoveAllSequences(void)
	{
		m_arSequences.empty();
	}

	inline int GetSequenceCount(void) const
	{
		return int(m_arSequences.size());
	}

	//
	// Frames
	//

	int AddFrame(const Frame& rFrame);

	inline Frame& GetFrame(int nFrameID)
	{
		return m_arFrames[nFrameID];
	}

	inline const Frame& GetFrameConst(int nFrameID) const
	{
		return m_arFrames[nFrameID];
	}

	void RemoveFrame(int nFrameID);
	void RemoveAllFrames(void);

	inline int GetFrameCount(void) const
	{
		return int(m_arFrames.size());
	}

	//
	// Serialization
	//

	virtual void Serialize(LPCWSTR pszPath) const;
	virtual void Serialize(Stream& rStream) const;
	void Serialize(InfoElem& rRoot) const;

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);
	void Deserialize(const InfoElem& rRoot);

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

/*----------------------------------------------------------*\
| Sequence class
\*----------------------------------------------------------*/

class Sequence
{
private:
	String m_strName;
	int m_nFirstFrameID;
	int m_nLastFrameID;
	int m_nNextSequenceID;

public:
	Sequence(void);

public:
	inline const String& GetName(void) const
	{
		return m_strName;
	}

	inline void SetName(LPCWSTR pszName)
	{
		m_strName = pszName;
	}

	inline int GetFirstFrameID(void) const
	{
		return m_nFirstFrameID;
	}

	inline void SetFirstFrame(int nFirstFrameID)
	{
		m_nFirstFrameID = nFirstFrameID;
	}

	inline int GetLastFrameID(void) const
	{
		return m_nLastFrameID;
	}

	inline void SetLastFrameID(int nLastFrameID)
	{
		m_nLastFrameID = nLastFrameID;
	}

	inline int GetNextSequenceID(void) const
	{
		return m_nNextSequenceID;
	}

	inline void SetNextSequenceID(int nNextSequenceID)
	{
		m_nNextSequenceID = nNextSequenceID;
	}

	//
	// Serialization
	//

	void Deserialize(const InfoElem& rRoot);
	void Serialize(InfoElem& rRoot) const;
};

/*----------------------------------------------------------*\
| Frame class
\*----------------------------------------------------------*/

class Frame
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_DURATION[];
	static const WCHAR SZ_DEF_DURATION[];
	static const WCHAR SZ_REGIONID[];
	static const WCHAR SZ_TEXTURECOORDS[];
	static const WCHAR SZ_META[];

protected:
	//
	// Members
	//

	// Duration in seconds
	float m_fDuration;

	// Collision region ID
	int m_nRegionID;

	// Texture coordinates
	POINT m_ptTextureCoords;

	// Meta data
	VariableMap m_mapVariables;

public:
	Frame(void);

	inline ~Frame(void)
	{
		Empty();
	}

public:
	//
	// Duration
	//

	inline float GetDuration(void) const
	{
		return m_fDuration;
	}

	inline void SetDuration(float fDuration)
	{
		m_fDuration = fDuration;
	}

	//
	// Region
	//

	inline int GetRegionID(void) const
	{
		return m_nRegionID;
	}

	inline void SetRegionID(int nRegionID)
	{
		m_nRegionID = nRegionID;
	}

	//
	// Texture coordinates
	//

	inline const POINT& GetTextureCoordsConst(void) const
	{
		return m_ptTextureCoords;
	}

	inline POINT& GetTextureCoords(void)
	{
		return m_ptTextureCoords;
	}

	inline void SetTextureCoords(POINT ptTextureCoords)
	{
		CopyMemory(&m_ptTextureCoords, &ptTextureCoords, sizeof(POINT));
	}

	//
	// Variables
	//

	const Variable* GetVariable(LPCWSTR pszName) const;
	
	inline VariableMapConstIterator GetBeginVariablePos(void) const
	{
		return m_mapVariables.begin();
	}

	inline VariableMapConstIterator GetEndVariablePos(void) const
	{
		return m_mapVariables.end();
	}

	//
	// Serialization
	//

	void Deserialize(const InfoElem& rRoot);
	void Serialize(InfoElem& rRoot) const;

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	void operator=(const Frame& rAssign);

	//
	// Friends
	//

	friend class Animation;
};

} // namespace ThunderStorm

#endif // THUNDER_ANIMATION_H