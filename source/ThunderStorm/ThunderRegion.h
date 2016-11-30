/*------------------------------------------------------------------*\
|
| ThunderRegion.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine region classes
| Created: 07/29/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_REGION_H
#define THUNDER_REGION_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Sprite;					// referencing Sprite
class Animation;				// referencing Animation
class Texture;					// referencing Texture
class RegionSet;				// referencing RegionSet, declared below
class Region;					// referencing Region, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Region*> RegionArray;
typedef std::vector<Region*>::iterator RegionArrayIterator;
typedef std::vector<Region*>::const_iterator RegionArrayConstIterator;


/*----------------------------------------------------------*\
| Region class - per-pixel collision map.
\*----------------------------------------------------------*/

class Region
{
private:
	//
	// Members
	//

	// RegionSet this region is contained by
	RegionSet* m_pRegionSet;

	// Pixel size
	SIZE m_psSize;

	// Data stored as a 1-d array
	BYTE* m_pData;

	// Points into m_pData, used to refer to it as a 2-d array
	BYTE** m_ppData;

public:
	Region(RegionSet* pRegionSet = NULL);
	~Region(void);

public:
	//
	// Parent
	//

	RegionSet* GetRegionSet(void);

	//
	// Size
	//

	const SIZE& GetSize(void) const;
	void SetSize(const SIZE& rpsSize);

	//
	// Data
	//

	BYTE* GetData1D(void);

	BYTE** GetData2D(void);
	const BYTE** GetData2DConst(void) const;

	//
	// Creation
	//

	void FromSurface(LPDIRECT3DSURFACE9 pSurf, const RECT& rrcSrcRect);

	//
	// Collision
	//

	bool TestPoint(const POINT& rpt) const;
	bool TestRect(const RECT& rrc) const;
	bool TestRegion(const Region& rRegion, const POINT& rptOffset) const;

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

	//
	// Operators
	//

	void operator=(const Region& rAssign);
};

/*----------------------------------------------------------*\
| RegionSet class - region container
\*----------------------------------------------------------*/

class RegionSet: public Resource
{
public:
	//
	// Constants
	//

	static const BYTE RGN_SIGNATURE[];
	static const BYTE RGN_FORMAT_VERSION[];

private:
	//
	// Members
	//

	RegionArray m_arRegions;

public:
	RegionSet(Engine& rEngine);
	virtual ~RegionSet(void);

public:
	//
	// Regions
	//

	Region* CreateRegion(void);
	int AddRegion(Region* pRegion);
	Region* GetRegion(int nRegion);
	int GetRegionCount(void) const;
	void RemoveRegion(int nRegion);
	void RemoveAllRegions(void);

	//
	// Creation
	//

	void FromAnimation(Animation& rAnim);
	void FromSprite(Sprite& rSprite);
	void FromTexture(Texture& rTexture, const RECT& rrcSrcRect);

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
};

} // namespace ThunderStorm

#endif