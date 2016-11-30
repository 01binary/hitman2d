/*------------------------------------------------------------------*\
|
| ThunderTile.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine map class(es) implementation
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "ThunderTileMap.h"	// using TileMap
#include "ThunderStream.h"	// using Stream
#include "ThunderTile.h"	// defining ThunderTile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;


/*----------------------------------------------------------*\
| Tile implementation
\*----------------------------------------------------------*/

Tile::Tile(void): m_dwFlags(DEFAULT),
				  m_nRefs(0)
{
}

Tile::Tile(DWORD dwFlags, D3DCOLOR clrBlend):
		   m_dwFlags(dwFlags),
		   m_clrBlend(clrBlend),
		   m_nRefs(0)
{
}

DWORD Tile::GetFlags(void) const
{
	return m_dwFlags;
}

bool Tile::IsFlagSet(DWORD dwFlag) const
{
	return (m_dwFlags & dwFlag) != 0;
}

void Tile::SetFlags(DWORD dwFlags)
{
	m_dwFlags = dwFlags;
}

void Tile::SetFlag(DWORD dwFlag)
{
	m_dwFlags |= dwFlag;
}

void Tile::ClearFlag(DWORD dwFlag)
{
	m_dwFlags &= dwFlag;
}

Color& Tile::GetBlend(void)
{
	return m_clrBlend;
}

const Color& Tile::GetBlendConst(void) const
{
	return m_clrBlend;
}

void Tile::SetBlend(D3DCOLOR clrBlend)
{
	m_clrBlend = clrBlend;
}

int Tile::AddRef(void)
{
	return ++m_nRefs;
}

void Tile::RemoveRef(void)
{
	if (0 == m_nRefs) return;

	m_nRefs--;
}

int Tile::GetRefCount(void) const
{
	return m_nRefs;
}

void Tile::Serialize(Stream& rStream) const
{
	// Write flags

	rStream.WriteVar(&m_dwFlags);

	// Write blend

	m_clrBlend.Serialize(rStream);
}

void Tile::Deserialize(Stream& rStream)
{
	// Read flags

	rStream.ReadVar(&m_dwFlags);

	// Read blend

	m_clrBlend.Deserialize(rStream);
}

/*----------------------------------------------------------*\
| TileStatic implementation
\*----------------------------------------------------------*/

TileStatic::TileStatic(void): m_nMaterialID(INVALID_INDEX)
{
}

TileStatic::TileStatic(const TileStatic& rInit):
						Tile(rInit.m_dwFlags, rInit.m_clrBlend),
						m_nMaterialID(rInit.m_nMaterialID),
						m_MaterialInst(rInit.m_MaterialInst)
{
}

int TileStatic::GetMaterialID(void) const
{
	return m_nMaterialID;
}

void TileStatic::SetMaterialID(TileMap& rMap, int nMaterialID)
{
	m_nMaterialID = nMaterialID;

	m_MaterialInst.SetMaterial(rMap.GetMaterial(nMaterialID));
}

MaterialInstance& TileStatic::GetMaterialInstance(void)
{
	return m_MaterialInst;
}

TileStatic& TileStatic::operator=(const TileStatic& rAssign)
{
	m_dwFlags = rAssign.m_dwFlags;
	
	m_MaterialInst = rAssign.m_MaterialInst;

	return *this;
}

bool TileStatic::operator==(const TileStatic& rCompare) const
{
	if (m_dwFlags != rCompare.m_dwFlags)
		return false;

	if (m_MaterialInst != rCompare.m_MaterialInst)
		return false;

	return true;
}

void TileStatic::Serialize(const TileMap& rMap, Stream& rStream) const
{
	// Write parent class

	Tile::Serialize(rStream);

	// Write material ID

	rStream.WriteVar(&m_nMaterialID);

	// Write material instance

	m_MaterialInst.Serialize(rMap.GetEngineConst(), rStream, false);
}

void TileStatic::Deserialize(TileMap& rMap, Stream& rStream)
{
	// Read parent class

	Tile::Deserialize(rStream);

	// Read material ID

	rStream.ReadVar(&m_nMaterialID);

	// Cache material

	m_MaterialInst.SetMaterial(rMap.GetMaterial(m_nMaterialID));

	// Read material instance

	m_MaterialInst.Deserialize(rMap.GetEngine(), rStream, false);
}

/*----------------------------------------------------------*\
| TileAnimated implementation
\*----------------------------------------------------------*/

TileAnimated::TileAnimated(void): m_nAnimationID(INVALID_INDEX)
{
}

TileAnimated::TileAnimated(const TileAnimated& rInit):
						   Tile(rInit.m_dwFlags, rInit.m_clrBlend),
						   m_nAnimationID(rInit.m_nAnimationID)
{
}

int TileAnimated::GetAnimationID(void) const
{
	return m_nAnimationID;
}

void TileAnimated::SetAnimationID(TileMap& rMap, int nAnimationID)
{
	m_nAnimationID = nAnimationID;

	m_MaterialInst.SetAnimation(rMap.GetAnimation(nAnimationID));
}

MaterialInstance& TileAnimated::GetMaterialInstance(void)
{
	return m_MaterialInst;
}

TileAnimated& TileAnimated::operator=(const TileAnimated& rAssign)
{
	m_dwFlags = rAssign.m_dwFlags;
	m_clrBlend = rAssign.m_clrBlend;
	m_MaterialInst = rAssign.m_MaterialInst;

	return *this;
}

bool TileAnimated::operator==(const TileAnimated& rCompare) const
{
	if (m_nAnimationID != rCompare.m_nAnimationID)
		return false;

	if (m_dwFlags != rCompare.m_dwFlags)
		return false;

	if (m_clrBlend != rCompare.m_clrBlend)
		return false;	

	if (m_MaterialInst != rCompare.m_MaterialInst)
		return false;

	return true;
}

void TileAnimated::Serialize(const TileMap& rMap, Stream& rStream) const
{
	// Write parent class

	Tile::Serialize(rStream);

	// Write animation ID

	rStream.WriteVar(&m_nAnimationID);

	// Write material instance

	m_MaterialInst.Serialize(rMap.GetEngineConst(), rStream, false);
}

void TileAnimated::Deserialize(TileMap& rMap, Stream& rStream)
{
	// Read parent class

	Tile::Deserialize(rStream);

	// Read animation ID

	rStream.ReadVar(&m_nAnimationID);

	// Cache animation

	m_MaterialInst.SetAnimation(rMap.GetAnimation(m_nAnimationID));

	// Read material instance

	m_MaterialInst.Deserialize(rMap.GetEngine(), rStream, false);
}