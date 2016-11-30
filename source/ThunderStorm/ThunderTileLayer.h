/*------------------------------------------------------------------*\
|
| ThunderTileLayer.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine tile layer class(es)
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_TILE_LAYER_H
#define THUNDER_TILE_LAYER_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderMath.h"		// using Vector2
#include "ThunderTile.h"		// using TileMap, Tile
#include "ThunderActor.h"		// using Actor, ActorArray

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class TileLayer;				// referencing TileLayer, declared below
class Actor;					// referencing Actor
class SpacePartition;			// referencing SpacePartition, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<TileLayer*> TileLayerArray;
typedef std::vector<TileLayer*>::iterator TileLayerArrayIterator;
typedef std::vector<TileLayer*>::const_iterator TileLayerArrayConstIterator;


/*----------------------------------------------------------*\
| TileLayer class
\*----------------------------------------------------------*/

class TileLayer
{
private:
	// Maintain reference to parent map
	TileMap& m_rMap;

	// Width in tiles
	int m_nWidth;

	// Height in tiles
	int m_nHeight;

	// 1D tile array, each element points to map's tile template
	Tile** m_ppTiles;

	// 2D tile array that refers into the 1D array for easier addressing
	Tile*** m_pppTiles;

	// Spacial partition database
	SpacePartition* m_pSpace;

	// Position on the map
	Vector2 m_vecPos;

public:
	TileLayer(TileMap& m_rMap);
	~TileLayer(void);

public:
	//
	// Map
	//

	TileMap& GetMap(void);
	const TileMap& GetMapConst(void) const;

	//
	// Size
	//

	int GetWidth(void) const;
	int GetHeight(void) const;

	Vector2 GetSize(void) const;
	void SetSize(int tcx, int tcy);

	Rect GetBounds(void) const;

	SpacePartition* GetSpace(void) const;

	//
	// Tiles
	//	

	Tile* SetTile(int tx,
				  int ty,
				  DWORD dwFlags,
				  D3DCOLOR clrBlend,
				  int nMaterialID,
				  POINT ptTextureCoords);

	Tile* SetTile(int tx,
				  int ty,				  
				  DWORD dwFlags,
				  D3DCOLOR clrBlend,
				  int nAnimationID,
				  bool bLoop,
				  int nStartFrame,
				  bool bPlay,
				  bool bReverse,
				  float fSpeed);

	void SetTile(int tx, int ty, Tile* pTile);

	Tile* GetTile(int tx, int ty);
	const Tile* GetTileConst(int tx, int ty) const;

	int GetTileIndex(int tx, int ty, bool* pbOutAnimated = NULL) const;

	Tile** GetTiles(void);
	Tile*** GetTiles2D(void);

	bool IsValidPosition(const Vector2& rvecPos) const;
	bool IsValidPosition(float x, float y) const;
	bool IsValidPosition(int x, int y) const;

	bool IsValidRange(const Rect& rrc) const;
	bool IsValidRange(const Vector2& rvecPos, const Vector2& rvecSize) const;
	void ValidateRange(Rect& rrc) const;

	//
	// Position
	//

	Vector2& GetPosition(void);
	const Vector2& GetPositionConst(void) const;

	void SetPosition(Vector2 vecPosition);
	void SetPosition(float x, float y);

	void Move(float fDeltaX, float fDeltaY);

	Vector2 LocalToWorld(Vector2 vecLocal);
	Vector2 WorldToLocal(Vector2 vecWorld);

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;
	
	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| SpacePartition interface class
\*----------------------------------------------------------*/

class SpacePartition
{
public:
	virtual ~SpacePartition(void)
	{
	};

public:
	//
	// Size
	//

	virtual void SetSize(int nWidth, int nHeight, bool bInitOnUse) = 0;

	virtual bool IsValidPosition(const Vector2& rvecPos) const = 0;
	virtual bool IsValidPosition(float tx, float ty) const = 0;
	virtual bool IsValidPosition(int tx, int ty) const = 0;

	virtual bool IsValidRange(const Rect& rrc) const = 0;
	virtual bool IsValidRange(const Vector2& rvecPos, const Vector2& rvecSize) const = 0;
	virtual void ValidateRange(Rect& rrc) const = 0;

	//
	// Update
	//

	virtual void Add(Actor* pActor) = 0;
	virtual void Remove(Actor* pActor) = 0;
	virtual void Update(Actor* pActor, const Rect& rrcOldBounds) = 0;

	//
	// Query
	//

	virtual int Query(int tx, int ty, ActorArray* parResult) = 0;
	virtual int Query(float tx, float ty, ActorArray* parResult) = 0;
	virtual int Query(Rect& rrcRange, ActorArray* parResult) const = 0;

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const = 0;

	//
	// Deinitialization
	//

	virtual void Empty(void) = 0;
};

/*----------------------------------------------------------*\
| SpacePartitionUniformGrid class
\*----------------------------------------------------------*/

class SpacePartitionUniformGrid: public SpacePartition
{
private:
	int m_nWidth;
	int m_nHeight;

	ActorSet** m_ppsetSpace;

public:
	SpacePartitionUniformGrid(void);
	virtual ~SpacePartitionUniformGrid(void);

public:
	//
	// Size
	//

	virtual void SetSize(int nWidth, int nHeight, bool bInitOnUse);

	virtual bool IsValidPosition(const Vector2& rvecPos) const;
	virtual bool IsValidPosition(float x, float y) const;
	virtual bool IsValidPosition(int x, int y) const;

	virtual bool IsValidRange(const Rect& rrc) const;
	virtual bool IsValidRange(const Vector2& rvecPos, const Vector2& rvecSize) const;
	virtual void ValidateRange(Rect& rrc) const;

	//
	// Update
	//

	virtual void Add(Actor* pActor);
	virtual void Remove(Actor* pActor);
	virtual void Update(Actor* pActor, const Rect& rrcOldBounds);

	//
	// Query
	//

	virtual int Query(int tx, int ty, ActorArray* parResult);
	virtual int Query(float tx, float ty, ActorArray* parResult);
	virtual int Query(Rect& rrcRange, ActorArray* parResult) const;

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);

protected:
	//
	// Private Functions
	//

	void Initialize(void);
	void Resize(int nWidth, int nHeight);
};

} // namespace ThunderStorm

#endif // THUNDER_TILE_LAYER_H