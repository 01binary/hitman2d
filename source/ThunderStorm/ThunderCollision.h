/*------------------------------------------------------------------*\
|
| ThunderCollision.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine collision detection class(es)
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_COLLISION_H
#define THUNDER_COLLISION_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Actor;			// referencing Actor
class Tile;				// referencing Tile
class VolumeCircle;		// referencing VolumeCircle, declared below
class VolumeAABB;		// referencing VolumeAABB, declared below
class VolumeOBB;		// referencing VolumeOBB, declared below
class VolumeHull;		// referencing VolumeHull, declared below


/*----------------------------------------------------------*\
| Collision class - collision information
\*----------------------------------------------------------*/

class CollisionInfo
{
private:
	// Pointer to tile, if collided with a tile
	Tile* m_pTile;

	// Pointer to actor, if collided with an actor
	Actor* m_pActor;

	// Separation vector from collision
	Vector2 m_vecSeparation;

	// Contact point from collision
	Vector2 m_vecContact;

public:
	CollisionInfo(void);

public:
	//
	// Tile Collision
	//

	Tile* GetTile(void);
	void SetTile(Tile* pTile);

	bool IsTileCollision(void) const;

	//
	// Actor Collision
	//

	Actor* GetActor(void);
	void SetActor(Actor* pActor);

	bool IsActorCollision(void) const;

	//
	// Collision Info
	//

	const Vector2& GetSeparation(void) const;
	const Vector2& GetContactPoint(void) const;
};

/*----------------------------------------------------------*\
| VolumeCircle class
\*----------------------------------------------------------*/

class VolumeCircle
{
private:
	Vector2 m_vecCenterPoint;
	float m_fRadius;

public:
	VolumeCircle(void);

	VolumeCircle(const Vector2& rvecCenterPoint,
								 float fRadius);

	VolumeCircle(const VolumeOBB& rbvOBB);
	VolumeCircle(const VolumeHull& rvbHull);
	VolumeCircle(const VolumeCircle& rInit);

public:
	//
	// Accessors
	//

	const Vector2& GetCenterPoint(void) const;
	float GetRadius(void) const;

	void Set(const Vector2& rvecCenterPoint, float fRadius);
	void Set(const VolumeOBB& rbvOBB);
	void Set(const VolumeHull& rbvHull);

	//
	// Intersection
	//

	bool Intersect(const VolumeCircle& rbvOther,
				   const Vector2* pvecVelocity,
				   Vector2* pvecOutContact,
				   Vector2* pvecOutSeparation) const;

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	VolumeCircle& operator=(const VolumeCircle& rAssign);
};

/*----------------------------------------------------------*\
| VolumeAABB class
\*----------------------------------------------------------*/

class VolumeAABB
{
private:
	Vector2 m_vecMin;
	Vector2 m_vecMax;

public:
	VolumeAABB(void);

	VolumeAABB(const Vector2& rvecMin,
							   const Vector2& rvecMax);

	VolumeAABB(float x1, float y1, float x2, float y2);
	VolumeAABB(const VolumeAABB& rInit);
	VolumeAABB(const VolumeOBB& rbvOBB);
	VolumeAABB(const VolumeHull& rbvHull);

public:
	//
	// Accessors
	//

	const Vector2& GetMin(void) const;
	const Vector2& GetMax(void) const;

	void Set(const Vector2& rvecMin, const Vector2& rvecMax);
	void Set(const VolumeOBB& rbvOBB);
	void Set(const VolumeHull& rbvHull);

	//
	// Intersection
	//

	bool Intersect(const VolumeAABB& rbvOther,
				   const Vector2* pvecVelocity,
				   Vector2* pvecOutContact,
				   Vector2* pvecOutSeparation) const;

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	VolumeAABB& operator=(const VolumeAABB& rAssign);
};

/*----------------------------------------------------------*\
| VolumeOBB class
\*----------------------------------------------------------*/

class VolumeOBB
{
private:
	Vector2 m_vecCenterPoint;
	Vector2 m_vecAxes[2];
	Vector2 m_vecExtents;

public:
	VolumeOBB(void);

	VolumeOBB(const Vector2& rvecCenterPoint,
							  const Vector2& rvecPivotPoint,
							  const Vector2& rvecSize,
							  float fRotation,
							  float fScale);

	VolumeOBB(const VolumeOBB& rInit);
	VolumeOBB(const VolumeHull& rbvHull);

public:
	//
	// Accessors
	//

	const Vector2& GetCenterPoint(void) const;
	const Vector2* GetAxes(void) const;
	const Vector2& GetExtents(void) const;

	void Set(const Vector2& rvecCenterPoint,
			 const Vector2& rvecPivotPoint,
			 const Vector2& rvecSize,
			 float fRotation,
			 float fScale);

	void Set(const VolumeHull& rbvHull);

	//
	// Intersection
	//

	bool Intersect(const VolumeOBB& rbvOther,
				   const Vector2* pvecVelocity,
				   Vector2* pvecOutContact,
				   Vector2* pvecOutSeparation) const;

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	VolumeOBB& operator=(const VolumeOBB& rAssign);
};

/*----------------------------------------------------------*\
| VolumeHull class
\*----------------------------------------------------------*/

class VolumeHull
{
private:
	Vector2 m_vecCenterPoint;
	Vector2Array m_arEdges;

public:
	VolumeHull(void);

	VolumeHull(const Vector2& rvecCenterPoint,
									 const Vector2& rvecPivotPoint,
									 const Vector2Array& rarLocalSpacePoints,
									 float fRotation,
									 float fScale);

public:
	//
	// Accessors
	//

	const Vector2& GetCenterPoint(void) const;
	const Vector2Array& GetEdges(void) const;

	void Set(const Vector2& rvecCenterPoint,
			 const Vector2& rvecPivotPoint,
			 const Vector2Array& rarLocalSpacePoints,
			 float fRotation,
			 float fScale);

	//
	// Intersection
	//

	bool Intersect(const VolumeHull& rbvOther,
				   const Vector2* pvecVelocity,
				   Vector2* pvecOutContact,
				   Vector2* pvecOutSeparation) const;

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	VolumeHull& operator=(const VolumeHull& rAssign);
};

} // namespace ThunderStorm

#endif // THUNDER_COLLISION_H