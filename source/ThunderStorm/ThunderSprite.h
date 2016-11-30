/*------------------------------------------------------------------*\
|
| ThunderSprite.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine sprite class(es)
| Created: 04/19/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_SPRITE_H
#define THUNDER_SPRITE_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"		// using Resource
#include "ThunderMath.h"			// using Vector

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Texture;				// referencing Texture
class Animation;			// referencing Animation
class Stream;				// referencing Stream


/*----------------------------------------------------------*\
| Sprite class - a collection of animations
\*----------------------------------------------------------*/

class Sprite: public Resource
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_SPRITE[];
	static const WCHAR SZ_DEFANIM[];
	static const WCHAR SZ_TEXTURE[];
	static const WCHAR SZ_SIZE[];
	static const WCHAR SZ_PIVOT[];
	static const WCHAR SZ_ANIMATION[];

private:
	//
	// Members
	//

	// Embedded animations
	AnimationArray m_arAnimations;

	// ID of default animation
	int m_nDefaultAnimationID;

	// Size in pixels. All animations are assumed to have this size
	SIZE m_psSize;

	// Size in tiles calculated from pixel size
	Vector2 m_vecSize;

	// Radius in tiles calculated from size
	float m_fRadius;

	// Center offset in pixels, calculated from size
	POINT m_ptCenter;

	// Center offset in tiles, calculated from size
	Vector2 m_vecCenter;

	// Pivot point in pixels
	POINT m_ptPivot;

	// Pivot point in tiles
	Vector2 m_vecPivot;

public:
	Sprite(Engine& rEngine);
	virtual ~Sprite(void);

public:
	//
	// Size and Radius
	//

	SIZE GetSize(void) const;
	void SetSize(SIZE psSize);

	const Vector2& GetSizeInTiles(void) const;
	void SetSizeInTiles(const Vector2& rvecSize);

	float GetRadius(void) const;

	//
	// Center and Pivot Point
	//

	const POINT& GetCenter(void) const;
	const Vector2& GetCenterInTiles(void) const;

	const POINT& GetPivot(void) const;
	const Vector2& GetPivotInTiles(void) const;
	void SetPivot(const POINT& rptPivot);

	//
	// Embedded Animations
	//

	int GetDefaultAnimation(void);
	void SetDefaultAnimation(int nAnimationID);

	Animation* CreateAnimation(void);
	int AddAnimation(Animation* pAnimation);
	Animation* GetAnimation(int nAnimationID);
	int FindAnimation(LPCWSTR pszName);
	int GetAnimationCount(void);
	void RemoveAnimation(int nAnimationID);
	void RemoveAllAnimations(void);

	//
	// Serialization
	//

	virtual void Serialize(LPCWSTR pszPath) const;
	virtual void Serialize(Stream& rStream) const;

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);
	
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

} // namespace ThunderStorm

#endif // THUNDER_SPRITE_H