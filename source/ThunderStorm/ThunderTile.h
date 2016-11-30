/*------------------------------------------------------------------*\
|
| ThunderTile.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine tile class(es)
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_TILE_H
#define THUNDER_TILE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderMaterial.h"	// using Material, Animation, Color

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class TileStatic;				// referencing TileStatic, declared below
class TileAnimated;				// referencing TileAnimated, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<TileStatic> TileStaticArray;
typedef std::vector<TileStatic>::iterator TileStaticArrayIterator;
typedef std::vector<TileStatic>::const_iterator TileStaticArrayConstIterator;

typedef std::vector<TileAnimated> TileAnimatedArray;
typedef std::vector<TileAnimated>::iterator TileAnimatedArrayIterator;
typedef std::vector<TileAnimated>::const_iterator TileAnimatedArrayConstIterator;


/*----------------------------------------------------------*\
| Tile class
\*----------------------------------------------------------*/

class Tile
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// No flags specified
		DEFAULT		= 0,

		// Disable rendering
		INVISIBLE	= 1 << 0,

		// Enable collision detection
		CLIP		= 1 << 1,

		// TileStatic or TileAnimated?
		ANIMATED	= 1 << 2,

		// Start value for user flags
		USER		= 1 << 3
	};

protected:
	//
	// Member Variables
	//

	DWORD m_dwFlags;			// Flags

	Color m_clrBlend;			// Blend color

	int m_nRefs;				// References (how many times used)

public:
	Tile(void);
	Tile(DWORD dwFlags, D3DCOLOR clrBlend);

public:
	//
	// Flags
	//

	DWORD GetFlags(void) const;
	bool IsFlagSet(DWORD dwFlag) const;
	void SetFlags(DWORD dwFlags);
	void SetFlag(DWORD dwFlag);
	void ClearFlag(DWORD dwFlag);

	//
	// Blend color
	//

	Color& GetBlend(void);
	const Color& GetBlendConst(void) const;
	void SetBlend(D3DCOLOR clrBlend);

	//
	// References
	//

	int AddRef(void);
	void RemoveRef(void);
	int GetRefCount(void) const;

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Friends
	//

	friend class TileMap;
};

/*----------------------------------------------------------*\
| TileStatic class
\*----------------------------------------------------------*/

class TileStatic: public Tile
{
protected:
	int m_nMaterialID;
	MaterialInstance m_MaterialInst;

public:
	TileStatic(void);
	TileStatic(const TileStatic& rInit);

public:
	//
	// Material
	//

	int GetMaterialID(void) const;
	MaterialInstance& GetMaterialInstance(void);

	//
	// Operators
	//

	TileStatic& operator=(const TileStatic& rAssign);
	bool operator==(const TileStatic& rCompare) const;

	//
	// Serialization
	//

	void Serialize(const TileMap& rMap, Stream& rStream) const;
	void Deserialize(TileMap& rMap, Stream& rStream);

	//
	// Friends
	//

	friend class TileMap;

protected:
	//
	// Private Functions
	//

	void SetMaterialID(TileMap& rMap, int nMaterialID);
};

/*----------------------------------------------------------*\
| TileAnimated class
\*----------------------------------------------------------*/

class TileAnimated: public Tile
{
protected:
	int m_nAnimationID;
	MaterialInstance m_MaterialInst;

public:
	TileAnimated(void);
	TileAnimated(const TileAnimated& rInit);

public:
	//
	// Material
	//

	int GetAnimationID(void) const;
	MaterialInstance& GetMaterialInstance(void);

	//
	// Operators
	//

	TileAnimated& operator=(const TileAnimated& rAssign);
	bool operator==(const TileAnimated& rCompare) const;

	//
	// Serialization
	//

	void Serialize(const TileMap& rMap, Stream& rStream) const;
	void Deserialize(TileMap& rMap, Stream& rStream);

	//
	// Friends
	//

	friend class TileMap;

protected:
	//
	// Private Functions
	//

	void SetAnimationID(TileMap& rMap, int nAnimationID);
};

} // namespace ThunderStorm

#endif // THUNDER_TILE_H