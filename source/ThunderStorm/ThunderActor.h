/*------------------------------------------------------------------*\
|
| ThunderActor.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine actor class(es)
| Created: 11/30/2008
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_ACTOR_H
#define THUNDER_ACTOR_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderGlobals.h"		// using INVALID_INDEX
#include "ThunderMath.h"		// using Vector
#include "ThunderObject.h"		// using Object
#include "ThunderMaterial.h"	// using MaterialInstance

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Actor;			// referencing Actor, declared below
class Sprite;			// referencing Sprite
class TileLayer;		// referencing TileLayer
class TileMap;			// referencing TileMap
class CollisionInfo;	// referencing CollisionInfo
class VolumeCircle;		// referencing VolumeCircle
class VolumeAABB;		// referencing VolumeAABB
class VolumeOBB;		// referencing VolumeOBB
class VolumeHull;		// referencing VolumeHull

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Actor*> ActorArray;
typedef std::vector<Actor*>::iterator ActorArrayIterator;
typedef std::vector<Actor*>::const_iterator ActorArrayConstIterator;

typedef std::set<Actor*> ActorSet;
typedef std::set<Actor*>::iterator ActorSetIterator;
typedef std::set<Actor*>::const_iterator ActorSetConstIterator;

typedef std::map<String, Actor*> ActorMap;
typedef std::map<String, Actor*>::iterator ActorMapIterator;
typedef std::map<String, Actor*>::const_iterator ActorMapConstIterator;

typedef std::vector<CollisionInfo> CollisionInfoArray;
typedef std::vector<CollisionInfo>::iterator CollisionInfoArrayIterator;
typedef std::vector<CollisionInfo>::const_iterator CollisionInfoArrayConstIterator;


/*----------------------------------------------------------*\
| Actor class - tile map entity
\*----------------------------------------------------------*/

class Actor: public Object
{
public:
	//
	// Constants
	//

	// Actor flags

	enum Flags					
	{
		// No flags specified
		DEFAULT	= 0,

		// Enable rendering
		RENDER	= 1 << 0,

		// Enable updates
		UPDATE	= 1 << 1,

		// Enable collision detection
		CLIP	= 1 << 2,

		// First value for user flags
		USER	= 1 << 3
	};

protected:
	//
	// Members
	//

	// Keep a reference to the map	
	TileMap& m_rMap;

	// Map layer attached to (only one allowed)
	TileLayer* m_pLayer;

	// Class name created from
	String m_strClass;

	// Position on the map expressed in tiles
	Vector2 m_vecPos;

	// Previous position (used for dynamic collision detection)
	Vector2 m_vecPrevPos;

	// Transform
	D3DXMATRIX m_mtxTransform;

	// Z order
	float m_fZOrder;

	// Sprite used
	Sprite* m_pSprite;

	// ID of currently playing sprite animation
	int m_nSpriteAnimationID;

	// Material instance with animation controller
	MaterialInstance m_materialInstance;

	// Blending ARGB color
	Color m_clrBlend;
	
public:
	//
	// Construction/Destruction
	//

	Actor(TileMap& rMap, LPCWSTR pszClass);
	virtual ~Actor(void);

public:
	//
	// Map
	//

	inline TileMap& GetMap(void)
	{
		return m_rMap;
	}

	inline const TileMap& GetMapConst(void) const
	{
		return m_rMap;
	}

	//
	// Layer
	//

	TileLayer* GetLayer(void);
	const TileLayer* GetLayerConst(void) const;
	void SetLayer(TileLayer* pLayer, bool bAdjustCoords = true);

	//
	// Class
	//

	inline const String& GetClass(void) const
	{
		return m_strClass;
	}

	//
	// Flags
	//

	virtual void SetFlags(DWORD dwFlags);

	//
	// Position
	//

	inline const Vector2& GetPosition(void) const
	{
		return m_vecPos;
	}

	inline const Vector2& GetPreviousPosition(void) const
	{
		return m_vecPrevPos;
	}

	void SetPosition(const Vector2& vecPosition);

	inline void SetPosition(float tx, float ty)
	{
		SetPosition(Vector2(tx, ty));
	}

	Rect GetBounds(void) const;

	//
	// Transform
	//

	inline D3DXMATRIX& GetTransform(void)
	{
		return m_mtxTransform;
	}

	inline const D3DXMATRIX& GetTransformConst(void) const
	{
		return m_mtxTransform;
	}

	inline void SetTransform(D3DXMATRIX& rmtxTransform)
	{
		m_mtxTransform = rmtxTransform;
	}

	//
	// Z-Order
	//

	inline float GetZOrder(void) const
	{
		return m_fZOrder;
	}

	inline void SetZOrder(float fZOrder)
	{
		m_fZOrder = fZOrder;
	}

	//
	// Sprite
	//

	void SetSprite(Sprite* pSprite);

	inline Sprite* GetSprite(void)
	{
		return m_pSprite;
	}

	inline const Sprite* GetSpriteConst(void) const
	{
		return m_pSprite;
	}

	//
	// Material Instance
	//

	inline MaterialInstance& GetMaterialInstance(void)
	{
		return m_materialInstance;
	}

	const MaterialInstance& GetMaterialInstanceConst(void) const
	{
		return m_materialInstance;
	}

	void PlayAnimation(int nAnimationID = INVALID_INDEX,
					   bool bLoop = true,
					   int nStartFrame = 0,
					   bool bReverse = false);

	inline int GetAnimationID(void) const
	{
		return m_nSpriteAnimationID;
	}

	//
	// Blend
	//

	inline Color& GetBlend(void)
	{
		return m_clrBlend;
	}

	inline const Color& GetBlendConst(void) const
	{
		return m_clrBlend;
	}

	inline void SetBlend(D3DCOLOR clrBlend)
	{
		m_clrBlend = clrBlend;
	}

	//
	// Rendering
	//

	virtual void Render(void);

	//
	// Update
	//

	virtual void Update(void);

	//
	// Collision
	//

	virtual const VolumeCircle* GetCollisionBoundsCircle(void) const;
	virtual const VolumeAABB* GetCollisionBoundsAABB(void) const;
	virtual const VolumeOBB* GetCollisionBoundsOBB(void) const;
	virtual const VolumeHull* GetCollisionBoundsHull(void) const;

	virtual int Collision(CollisionInfoArray* parOutCollisions = NULL) const;
	virtual bool CollisionWithActor(Actor& rActor, CollisionInfo* pOutCollision = NULL) const;
	virtual bool CollisionWithTile(int x, int y, CollisionInfo* pOutCollision = NULL) const;

	//
	// Serialization
	//

	virtual void Serialize(Stream& rStream) const;
	virtual void Deserialize(Stream& rStream);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);

	//
	// Events
	//

	virtual void OnBoundsChange(const Rect& rrcOldBounds);

	virtual void OnEnterCamera(void);
	virtual void OnExitCamera(void);

	virtual void OnSessionPause(bool bPause);

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseLDbl(POINT pt);
	virtual void OnMouseRDown(POINT pt);
	virtual void OnMouseRUp(POINT pt);
	virtual void OnMouseRDbl(POINT pt);
	virtual void OnMouseMDown(POINT pt);
	virtual void OnMouseMUp(POINT pt);
	virtual void OnMouseMDbl(POINT pt);
	virtual void OnMouseWheel(int nZDelta);

protected:
	//
	// Friends
	//

	friend class TileMap;
};

} // namespace ThunderStorm

#endif // THUNDER_ACTOR_H