/*------------------------------------------------------------------*\
|
| ThunderTileLayer.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine tile layer class(es) implementation
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine, Error
#include "ThunderTileMap.h"		// using TileMap
#include "ThunderTileLayer.h"	// defining TileLayer

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| TileLayer implementation
\*----------------------------------------------------------*/

TileLayer::TileLayer(TileMap& rMap):
					 m_rMap(rMap),
					 m_nWidth(0),
					 m_nHeight(0),
					 m_ppTiles(NULL),
					 m_pppTiles(NULL),
					 m_pSpace(NULL)
{
}

TileLayer::~TileLayer(void)
{
	Empty();
}

TileMap& TileLayer::GetMap(void)
{
	return m_rMap;
}

const TileMap& TileLayer::GetMapConst(void) const
{
	return m_rMap;
}

int TileLayer::GetWidth(void) const
{
	return m_nWidth;
}

int TileLayer::GetHeight(void) const
{
	return m_nHeight;
}

Vector2 TileLayer::GetSize(void) const
{
	return Vector2(float(m_nWidth), float(m_nHeight));
}

void TileLayer::SetSize(int nWidth, int nHeight)
{
	// Allocate 1D tile array

	int nTileCount = nWidth * nHeight;

	Tile** ppTiles = NULL;
	Tile*** pppTiles = NULL;

	try
	{
		ppTiles = new Tile*[nTileCount];
	}

	catch(std::bad_alloc e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Tile*) * nTileCount);
	}

	// Allocate 2D tile array

	try
	{
		pppTiles = new Tile**[nHeight];
	}

	catch(std::bad_alloc e)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Tile**) * nHeight);
	}

	// Point elements of 2D tile array into 1D tile array

	for(int y = 0, n = 0; y < nHeight; y++, n += nWidth)
	{
		pppTiles[y] = &ppTiles[n];
	}

	if (m_ppTiles != NULL)
	{
		// Copy and release previously allocated tiles

		int nCopyHeight = min(m_nHeight, nHeight);
		int nCopyWidth = min(m_nWidth, nWidth);
		int nOldWidth = m_nWidth;

		size_t nCopySizeBytes = nOldWidth * sizeof(int);

		size_t nFillSizeBytes =
			(nWidth > nOldWidth ? nWidth - nOldWidth : 0) * sizeof(int);

		for(int y = 0; y < nCopyHeight; y++)
		{
			// Copy valid tiles

			CopyMemory(ppTiles[y * nWidth],
				   m_ppTiles[y * nOldWidth],
				   nCopySizeBytes);

			// If growing, fill new elements with NULL

			if (nFillSizeBytes != 0)
				memset(ppTiles[y * nWidth + nCopyWidth],
				NULL,
				nFillSizeBytes);
		}

		delete[] m_ppTiles;
		delete[] m_pppTiles;
	}
	else
	{
		// If no previously allocated tiles, fill with NULL

		memset(ppTiles, NULL, size_t(nTileCount * sizeof(Tile*)));
	}

	// Update pointers

	m_ppTiles = ppTiles;
	m_pppTiles = pppTiles;

	// Update spacial partitions

	if (NULL == m_pSpace)
	{
		try
		{
			m_pSpace = new SpacePartitionUniformGrid;
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
				sizeof(SpacePartitionUniformGrid));
		}
	}
	
	m_pSpace->SetSize(nWidth, nHeight, true);
}

Rect TileLayer::GetBounds(void) const
{
	return Rect(int(m_vecPos.x),
		int(m_vecPos.y),
		int(ceil(m_vecPos.x)) + m_nWidth,
		int(ceil(m_vecPos.y)) + m_nHeight);
}

SpacePartition* TileLayer::GetSpace(void) const
{
	return m_pSpace;
}

Tile* TileLayer::SetTile(int tx,
						 int ty,
						 DWORD dwFlags,
						 D3DCOLOR clrBlend,
						 int nMaterialID,
						 POINT ptTextureCoords)
{
	return m_rMap.SetTileTemplateStatic(GetTileIndex(tx, ty),
		dwFlags, clrBlend, nMaterialID, ptTextureCoords);
}

Tile* TileLayer::SetTile(int tx,
						 int ty,						 
						 DWORD dwFlags,
						 D3DCOLOR clrBlend,
						 int nAnimationID,
						 bool bLoop,
						 int nStartFrame,
						 bool bPlay,
						 bool bReverse,
						 float fSpeed)
{
	return m_rMap.SetTileTemplateAnimated(GetTileIndex(tx, ty),
		dwFlags,clrBlend, nAnimationID,bLoop,nStartFrame,
		bPlay, bReverse, fSpeed);
}

void TileLayer::SetTile(int tx, int ty, Tile* pTile)
{
	if (pTile != NULL)
		pTile->AddRef();

	Tile* pOldTile = GetTile(tx, ty);

	if (pOldTile != NULL)
		pOldTile->RemoveRef();

	m_pppTiles[ty][tx] = pTile;
}

Tile* TileLayer::GetTile(int tx, int ty)
{
	return m_pppTiles[ty][tx];
}

const Tile* TileLayer::GetTileConst(int tx, int ty) const
{
	return m_pppTiles[ty][tx];
}

int TileLayer::GetTileIndex(int tx, int ty, bool* pbOutAnimated) const
{
	const Tile* pTemplate = GetTileConst(tx, ty);

	if (pTemplate != NULL)
	{
		if (pbOutAnimated != NULL)
			*pbOutAnimated = pTemplate->IsFlagSet(Tile::ANIMATED);

		if (pTemplate->IsFlagSet(Tile::ANIMATED) == true)
			return int(pTemplate - &m_rMap.GetTileTemplateAnimatedConst(0));
		else
			return int(pTemplate - &m_rMap.GetTileTemplateStaticConst(0));
	}

	return INVALID_INDEX;
}

Tile** TileLayer::GetTiles(void)
{
	return m_ppTiles;
}

Tile*** TileLayer::GetTiles2D(void)
{
	return m_pppTiles;
}

bool TileLayer::IsValidPosition(const Vector2& rvecPos) const
{
	return ((rvecPos.x >= 0.0f && rvecPos.x < float(m_nWidth)) &&
			(rvecPos.y >= 0.0f && rvecPos.y < float(m_nHeight)));
}

bool TileLayer::IsValidPosition(float x, float y) const
{
	return ((x >= 0.0f && x < float(m_nWidth)) &&
			(y >= 0.0f && y < float(m_nHeight)));
}

bool TileLayer::IsValidPosition(int x, int y) const
{
	return ((x >= 0 && x < m_nWidth) &&
			(y >= 0 && y < m_nHeight));
}

bool TileLayer::IsValidRange(const Rect& rrc) const
{
	if (rrc.left < 0 ||
	   rrc.right >= m_nWidth) return false;
	if (rrc.top < 0 ||
	   rrc.bottom >= m_nHeight) return false;

	return true;
}

bool TileLayer::IsValidRange(const Vector2& rvecPos,
							 const Vector2& rvecSize) const
{
	if (int(rvecPos.x) < 0 ||
	   int(ceil(rvecPos.x + rvecSize.x)) > m_nWidth) return false;

	if (int(rvecPos.y) < 0 ||
	   int(ceil(rvecPos.y + rvecSize.y)) > m_nHeight) return false;

	return true;
}

void TileLayer::ValidateRange(Rect& rrc) const
{
	if (rrc.left < 0)
		rrc.left = 0;
	else if (rrc.left >= m_nWidth)
		rrc.left = m_nWidth - 1;

	if (rrc.top < 0)
		rrc.top = 0;
	else if (rrc.top >= m_nHeight)
		rrc.top = m_nHeight - 1;

	if (rrc.right < 0)
		rrc.right = 0;
	else if (rrc.right > m_nWidth)
		rrc.right = m_nWidth;

	if (rrc.bottom < 0)
		rrc.bottom = 0;
	else if (rrc.bottom > m_nHeight)
		rrc.bottom = m_nHeight;
}

Vector2& TileLayer::GetPosition(void)
{
	return m_vecPos;
}

const Vector2& TileLayer::GetPositionConst(void) const
{
	return m_vecPos;
}

void TileLayer::SetPosition(Vector2 vecPosition)
{
	m_vecPos = vecPosition;
}

void TileLayer::SetPosition(float x, float y)
{
	m_vecPos.x = x;
	m_vecPos.y = y;
}

void TileLayer::Move(float fDeltaX, float fDeltaY)
{
	m_vecPos.x += fDeltaX;
	m_vecPos.y += fDeltaY;
}

Vector2 TileLayer::LocalToWorld(Vector2 vecLocal)
{
	return (vecLocal + m_vecPos);
}

Vector2 TileLayer::WorldToLocal(Vector2 vecWorld)
{
	return (vecWorld - m_vecPos);
}

void TileLayer::Serialize(Stream& rStream) const
{
	// Write position on map

	m_vecPos.Serialize(rStream);

	// Write size

	rStream.WriteVar(&m_nWidth);
	rStream.WriteVar(&m_nHeight);

	// Write tiles

	int nTileCount = m_nWidth * m_nHeight;

	for(int n = 0; n < nTileCount; n++)
	{		
		bool bTileAnimated;
		int nTileIndex;

		if (m_ppTiles[n]->IsFlagSet(Tile::ANIMATED) == true)
		{
			bTileAnimated = true;
			nTileIndex = int(m_ppTiles[n] - &m_rMap.GetTileTemplateAnimated(0));
		}
		else
		{
			bTileAnimated = false;
			nTileIndex = int(m_ppTiles[n] - &m_rMap.GetTileTemplateStatic(0));
		}

		rStream.WriteVar(&bTileAnimated);
		rStream.WriteVar(&nTileIndex);
	}
}

void TileLayer::Deserialize(Stream& rStream)
{
	Empty();

	// Read position on map

	m_vecPos.Deserialize(rStream);

	// Read size

	rStream.ReadVar(&m_nWidth);
	rStream.ReadVar(&m_nHeight);

	// Read tile type/index pairs and convert to pointers

	int nTileCount = m_nWidth * m_nHeight;

	for(int n = 0; n < nTileCount; n++)
	{
		bool bTileAnimated;
		rStream.ReadVar(&bTileAnimated);

		int nTileIndex;		
		rStream.ReadVar(&nTileIndex);

		if (true == bTileAnimated)
			m_ppTiles[n] = (Tile*)&m_rMap.GetTileTemplateAnimated(nTileIndex);
		else
			m_ppTiles[n] = (Tile*)&m_rMap.GetTileTemplateStatic(nTileIndex);
	}
}

DWORD TileLayer::GetMemoryFootprint(void) const
{
	return sizeof(TileLayer) +
		sizeof(Tile*) * m_nWidth * m_nHeight +
		sizeof(Tile**) * m_nHeight +
		(m_pSpace != NULL ? m_pSpace->GetMemoryFootprint() : 0);
}

void TileLayer::Empty(void)
{
	// Deallocate 2D tile array

	delete[] m_pppTiles;
	m_pppTiles = NULL;

	// Deallocate 1D tile array

	delete[] m_ppTiles;
	m_ppTiles = NULL;

	// Deallocate spacial database
	
	delete m_pSpace;
	m_pSpace = NULL;
}

/*----------------------------------------------------------*\
| SpacePartitionUniformGrid implementation
\*----------------------------------------------------------*/

SpacePartitionUniformGrid::SpacePartitionUniformGrid(void):
													 m_nWidth(0),
													 m_nHeight(0),
													 m_ppsetSpace(NULL)
{
}

SpacePartitionUniformGrid::~SpacePartitionUniformGrid(void)
{
	Empty();
}

void SpacePartitionUniformGrid::SetSize(int nWidth, int nHeight, bool bInitOnUse)
{
	if (NULL == m_ppsetSpace)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;

		if (false == bInitOnUse)
			Initialize();
	}
	else
	{
		Resize(nWidth, nHeight);
	}
}

bool SpacePartitionUniformGrid::IsValidPosition(const Vector2& rvecPos) const
{
	return ((int(rvecPos.x) >= 0 && int(rvecPos.x) < m_nWidth) &&
			(int(rvecPos.y) >= 0 && int(rvecPos.y) < m_nHeight));
}

bool SpacePartitionUniformGrid::IsValidPosition(float x, float y) const
{
	return ((int(x) >= 0 && int(x) < m_nWidth) &&
			(int(y) >= 0 && int(y) < m_nHeight));
}

bool SpacePartitionUniformGrid::IsValidPosition(int x, int y) const
{
	return ((x >= 0 && x < m_nWidth) &&
			(y >= 0 && y < m_nHeight));
}

bool SpacePartitionUniformGrid::IsValidRange(const Rect& rrc) const
{
	if (rrc.left < 0 || rrc.left >= m_nWidth)
		return false;

	if (rrc.top < 0 || rrc.top >= m_nHeight)
		return false;

	if (rrc.right < 0 || rrc.right > m_nWidth)
		return false;

	if (rrc.bottom < 0 || rrc.bottom > m_nHeight)
		return false;

	return true;
}

bool SpacePartitionUniformGrid::IsValidRange(const Vector2& rvecPos,
											 const Vector2& rvecSize) const
{
	return IsValidRange(Rect(int(floor(rvecPos.x)),
							 int(floor(rvecPos.y)),
							 int(ceil(rvecPos.x + rvecSize.x)),
							 int(ceil(rvecPos.y + rvecSize.y))));
}

void SpacePartitionUniformGrid::ValidateRange(Rect& rrc) const
{
	if (rrc.left < 0)
		rrc.left = 0;
	else if (rrc.left >= m_nWidth)
		rrc.left = m_nWidth - 1;

	if (rrc.top < 0)
		rrc.top = 0;
	else if (rrc.top >= m_nHeight)
		rrc.top = m_nHeight - 1;

	if (rrc.right < 0)
		rrc.right = 0;
	else if (rrc.right > m_nWidth)
		rrc.right = m_nWidth;

	if (rrc.bottom < 0)
		rrc.bottom = 0;
	else if (rrc.bottom > m_nHeight)
		rrc.bottom = m_nHeight;
}

void SpacePartitionUniformGrid::Add(Actor* pActor)
{
	if (NULL == pActor)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	if (NULL == m_ppsetSpace)
		Initialize();

	Rect rcBounds = pActor->GetBounds();
	ValidateRange(rcBounds);

	for(int ty = rcBounds.top; ty < rcBounds.bottom; ty++)
	{
		for(int tx = rcBounds.left; tx < rcBounds.right; tx++)
		{
			m_ppsetSpace[ty][tx].insert(pActor);
		}
	}
}

void SpacePartitionUniformGrid::Remove(Actor* pActor)
{
	if (NULL == pActor)
		return;

	if (NULL == m_ppsetSpace)
		return;

	Rect rcBounds = pActor->GetBounds();
	ValidateRange(rcBounds);

	for(int ty = rcBounds.top; ty < rcBounds.bottom; ty++)
	{
		for(int tx = rcBounds.left; tx < rcBounds.right; tx++)
		{
			m_ppsetSpace[ty][tx].erase(pActor);
		}
	}
}

void SpacePartitionUniformGrid::Update(Actor* pActor,
									   const Rect& rrcOldBounds)
{
	if (NULL == pActor)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	if (NULL == m_ppsetSpace)
	{
		Add(pActor);
	}
	else
	{
		// Calculate ranges

		Rect rcOldRange = rrcOldBounds;
		Rect rcNewRange = pActor->GetBounds();

		ValidateRange(rcOldRange);
		ValidateRange(rcNewRange);

		// Add to new range

		for(int ty = rcNewRange.top; ty < rcNewRange.bottom; ty++)
		{
			for(int tx = rcNewRange.left; tx < rcNewRange.right; tx++)
			{
				m_ppsetSpace[ty][tx].insert(pActor);
			}
		}

		// Remove from old range

		Rect rcIntersect;
		
		if (rcOldRange.Intersect(rcNewRange, rcIntersect) == true)
		{
			// Remove only from non-intersected cells

			for(int ty = rcOldRange.top; ty < rcOldRange.bottom; ty++)
			{
				for(int tx = rcOldRange.left; tx < rcOldRange.right; tx++)
				{
					if (rcIntersect.PtInRect(tx, ty) == false)
						m_ppsetSpace[ty][tx].erase(pActor);
				}
			}
		}
		else
		{
			for(int ty = rcOldRange.top; ty < rcOldRange.bottom; ty++)
			{
				for(int tx = rcOldRange.left; tx < rcOldRange.right; tx++)
				{
					m_ppsetSpace[ty][tx].erase(pActor);
				}
			}
		}
	}
}

int SpacePartitionUniformGrid::Query(int tx, int ty, ActorArray* parResult)
{
	if (NULL == m_ppsetSpace)
		return 0;

	if (IsValidPosition(tx, ty) == false)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	ActorSet& rCell = m_ppsetSpace[ty][tx];

	if (parResult != NULL)
		std::copy(rCell.begin(), rCell.end(),
			std::inserter(*parResult, parResult->end()));

	return int(rCell.size());
}

int SpacePartitionUniformGrid::Query(float tx, float ty, ActorArray* parResult)
{
	return Query(int(tx), int(ty), parResult);
}

int SpacePartitionUniformGrid::Query(Rect& rrcRange, ActorArray* parResult) const
{
	if (NULL == m_ppsetSpace)
		return 0;

	// Validate range

	ValidateRange(rrcRange);

	// Query actors from cells in range	

	ActorArray arActors;

	for(int ty = rrcRange.top; ty < rrcRange.bottom; ty++)
	{
		for(int tx = rrcRange.left; tx < rrcRange.right; tx++)
		{
			std::copy(m_ppsetSpace[ty][tx].begin(),
					  m_ppsetSpace[ty][tx].end(),
					  std::inserter(arActors, arActors.end()));
		}
	}

	std::sort(arActors.begin(), arActors.end());

	ActorArrayIterator posUniqueEnd =
		std::unique(arActors.begin(), arActors.end());

	// Return actors and the count of actors

	int nUniqueCount =
		(arActors.end() == posUniqueEnd) ? 0 : int(posUniqueEnd - arActors.begin());

	if (parResult != NULL)
		std::copy(arActors.begin(), posUniqueEnd,
		std::inserter(*parResult, parResult->end()));

	return nUniqueCount;
}

void SpacePartitionUniformGrid::Initialize(void)
{
	if (m_ppsetSpace != NULL)
		return;

	try
	{
		m_ppsetSpace = new ActorSet*[m_nHeight];
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
			sizeof(ActorSet*) * m_nHeight);
	}

	try
	{
		for(int n = 0; n < m_nHeight; n++)
		{
			m_ppsetSpace[n] = new ActorSet[m_nWidth];
		}
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
			sizeof(ActorSet) * m_nWidth);
	}
}

void SpacePartitionUniformGrid::Resize(int nWidth, int nHeight)
{
	if (nHeight > m_nHeight)
	{
		// Reallocate

		ActorSet** ppsetSpace = NULL;

		try
		{
			ppsetSpace = new ActorSet*[nHeight];
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
				sizeof(ActorSet*) * nHeight);
		}

		// Re-fill

		for(int ty = 0; ty < m_nHeight; ty++)
		{
			ppsetSpace[ty] = m_ppsetSpace[ty];
		}	

		// Add new entries

		for(int ty = m_nHeight; ty < nHeight; ty++)
		{
			ppsetSpace[ty] = new ActorSet[nWidth];
		}

		// Replace

		delete[] m_ppsetSpace;
		m_ppsetSpace = ppsetSpace;
	}
	else if (nHeight < m_nHeight)
	{
		// Delete a few rows from the end

		for(int y = m_nHeight - nHeight + 1; y < m_nHeight; y++)
		{
			delete m_ppsetSpace[y];
		}

		// Reallocate

		ActorSet** ppsetSpace = NULL;

		try
		{
			ppsetSpace = new ActorSet*[nHeight];
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__,
				sizeof(ActorSet*) * nHeight);
		}

		// Re-fill

		for(int y = 0; y < nHeight; y++)
		{
			ppsetSpace[y] = m_ppsetSpace[y];
		}

		// Replace

		delete[] m_ppsetSpace;
		m_ppsetSpace = ppsetSpace;
	}

	try
	{
		if (nWidth > m_nWidth)
		{
			// Reallocate rows to contain more elements
			
			for(int ty = 0; ty < m_nHeight; ty++)
			{
				ActorSet* pOldSets = m_ppsetSpace[ty];

				m_ppsetSpace[ty] = new ActorSet[nWidth];

				for(int tx = 0; tx < m_nWidth; tx++)
				{
					std::copy(pOldSets[tx].begin(), pOldSets[tx].end(),
						std::inserter(m_ppsetSpace[ty][tx],
						m_ppsetSpace[ty][tx].end()));
				}

				delete[] pOldSets;
			}
		}
		else if (nWidth < m_nWidth)
		{
			// Reallocate rows to contain less elements

			for(int ty = 0; ty < m_nHeight; ty++)
			{
				ActorSet* pOldSets = m_ppsetSpace[ty];

				m_ppsetSpace[ty] = new ActorSet[nWidth];

				for(int tx = 0; tx < nWidth; tx++)
				{
					std::copy(pOldSets[tx].begin(),
						pOldSets[tx].end(),
						std::inserter(m_ppsetSpace[ty][tx],
							m_ppsetSpace[ty][tx].end()));
				}

				delete[] pOldSets;
			}
		}
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(ActorSet) * nWidth);
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

DWORD SpacePartitionUniformGrid::GetMemoryFootprint(void) const
{
	return sizeof(SpacePartitionUniformGrid) +
		   sizeof(ActorSet) * m_nWidth * m_nHeight +
		   sizeof(ActorSet*) * m_nHeight;
}

void SpacePartitionUniformGrid::Empty(void)
{
	if (m_ppsetSpace != NULL)
	{
		for(int n = 0; n < m_nHeight; n++)
		{
			delete m_ppsetSpace[n];
		}

		delete[] m_ppsetSpace;

		m_ppsetSpace = NULL;
	}

	m_nWidth = 0;
	m_nHeight = 0;
}