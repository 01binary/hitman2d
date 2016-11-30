/*------------------------------------------------------------------*\
|
| ThunderActor.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine actor class(es) implementation
| Created: 11/30/2008
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine, using TileMap
#include "ThunderStream.h"		// using Stream
#include "ThunderObject.h"		// using Object
#include "ThunderCollision.h"	// using Collision, VolumeXXX
#include "ThunderSprite.h"		// using Sprite
#include "ThunderActor.h"		// defining Actor
#include "ThunderTileMap.h"		// using TileMap

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Actor implementation
\*----------------------------------------------------------*/

Actor::Actor(TileMap& rMap, LPCWSTR pszClass): 
			 Object(rMap.GetEngine()),
			 m_rMap(rMap),
			 m_strClass(pszClass),
			 m_vecPos(-1.0f, -1.0f),	
			 m_vecPrevPos(-1.0f, -1.0f),
			 m_pSprite(NULL),
			 m_nSpriteAnimationID(-1),
			 m_clrBlend(Color::BLEND_ONE)
{
	m_materialInstance.Empty();
}

Actor::~Actor(void)
{
	Empty();
}

void Actor::SetFlags(DWORD dwFlags)
{
	if (IsFlagSet(UPDATE) == false && (dwFlags & UPDATE))
	{
		// Add to map's update list

		m_rMap.m_arUpdateActors.push_back(this);
	}
	else if (IsFlagSet(UPDATE) == true && (~dwFlags & UPDATE))
	{
		// Null out the item in map's update list (will be removed on next update)

		ActorArrayIterator pos =
			std::find(m_rMap.m_arUpdateActors.begin(),
			m_rMap.m_arUpdateActors.end(), this);

		if (pos != m_rMap.m_arUpdateActors.end())
			*pos = NULL;	
	}
	
	m_dwFlags = dwFlags;
}

void Actor::SetPosition(const Vector2& vecPosition)
{
	Rect rcOldBounds = GetBounds();

	m_vecPrevPos = m_vecPos;
	m_vecPos = vecPosition;

	m_pLayer->GetSpace()->Update(this, rcOldBounds);
}

Rect Actor::GetBounds(void) const
{
	return Rect(int(floor(m_vecPos.x)), int(floor(m_vecPos.y)),
		int(ceil(m_vecPos.x + m_pSprite->GetSizeInTiles().x)),
		int(ceil(m_vecPos.y + m_pSprite->GetSizeInTiles().y)));
}

void Actor::SetSprite(Sprite* pSprite)
{
	Rect rcOldBounds = GetBounds();

	if (pSprite != NULL)
		pSprite->AddRef();

	if (m_pSprite != NULL)
		m_pSprite->Release();

	m_pSprite = pSprite;

	PlayAnimation();

	if (rcOldBounds != GetBounds())
		OnBoundsChange(rcOldBounds);
}

void Actor::PlayAnimation(int nAnimationID,
						  bool bLoop,
						  int nStartFrame,
						  bool bReverse)
{
	if (NULL == m_pSprite)
		return;

	m_nSpriteAnimationID = (INVALID_INDEX == nAnimationID ?
		m_pSprite->GetDefaultAnimation() : nAnimationID);

	m_materialInstance.SetAnimation(m_pSprite->GetAnimation(m_nSpriteAnimationID));

	m_materialInstance.Play(m_rEngine.GetTime(), bLoop, nStartFrame, bReverse);
}

void Actor::Render(void)
{
	if (NULL == m_pSprite)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
		__FUNCTIONW__);

	// Render

	m_rEngine.GetGraphics().RenderQuad(m_materialInstance, m_vecPos,
		m_clrBlend, m_fZOrder, &m_pSprite->GetPivotInTiles(), &m_mtxTransform);
}

void Actor::Update(void)
{
	// Update animation

	m_materialInstance.Update(m_rEngine.GetTime());
}

const VolumeCircle* Actor::GetCollisionBoundsCircle(void) const
{
	// Default implementation
	return NULL;
}

const VolumeAABB* Actor::GetCollisionBoundsAABB(void) const
{
	// Default implementation
	return NULL;
}

const VolumeOBB* Actor::GetCollisionBoundsOBB(void) const
{
	// Default implementation
	return NULL;
}

const VolumeHull* Actor::GetCollisionBoundsHull(void) const
{
	// Default implementation
	return NULL;
}

int Actor::Collision(CollisionInfoArray* parOutCollisions) const
{
	// Make sure actor has collision info

	if (NULL == m_pSprite)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	// Check collisions with tiles and actors near this actor

	Rect rcTiles;	

	rcTiles.left = int(m_vecPos.x - m_pSprite->GetPivotInTiles().x);
	rcTiles.top = int(m_vecPos.y - m_pSprite->GetPivotInTiles().y);

	// TODO: see if we need +1 or -1 or whatever...

	rcTiles.right = rcTiles.left + int(ceil(m_pSprite->GetSizeInTiles().x));
	rcTiles.bottom = rcTiles.top + int(ceil(m_pSprite->GetSizeInTiles().y));

	m_pLayer->ValidateRange(rcTiles);

	ActorArray arActors;
	arActors.reserve(rcTiles.GetArea() * 8);

	int nCollisions = 0;
	CollisionInfo collision;
	
	for(int y = rcTiles.top; y <= rcTiles.bottom; y++)
	{
		for(int x = rcTiles.left; x <= rcTiles.right; x++)
		{
			// Collide with tile

			if (CollisionWithTile(x, y, &collision) == true)
			{
				++nCollisions;

				if (parOutCollisions != NULL)
					parOutCollisions->push_back(collision);
			}

			// Add actors overlaying this tile to the
			// list of actors to collide against

			m_pLayer->GetSpace()->Query(x, y, &arActors);
		}
	}

	std::sort(arActors.begin(), arActors.end());
	std::unique(arActors.begin(), arActors.end());

	for(ActorArrayIterator pos = arActors.begin();
		pos != arActors.end();
		pos++)
	{
		// Collision with actor

		if (CollisionWithActor(**pos, &collision))
		{
			nCollisions++;

			if (parOutCollisions != NULL)
				parOutCollisions->push_back(collision);
		}
	}

	return nCollisions;
}

bool Actor::CollisionWithTile(int x, int y, CollisionInfo* pOutCollision) const
{
	// If either cannot collide, return no collision

	if (IsFlagSet(Actor::CLIP) == false ||
		m_pLayer->GetTileConst(x, y)->IsFlagSet(Tile::CLIP) == false)
	   return false;

	// TODO: bv test

	// Set collision information

	if (pOutCollision != NULL)
		pOutCollision->SetTile(m_pLayer->GetTile(x, y));

	return false;
}

bool Actor::CollisionWithActor(Actor& rActor, CollisionInfo* pOutCollision) const
{
	// If either actor cannot collide, return no collision

	if (IsFlagSet(Actor::CLIP) == false ||
		rActor.IsFlagSet(Actor::CLIP) == false)
	   return false;

	// TODO: bv test

	return false;
}

void Actor::Serialize(Stream& rStream) const
{
	try
	{
		// Write name

		m_strName.Serialize(rStream);

		// Write flags

		rStream.WriteVar(&m_dwFlags);

		// Write position

		m_vecPos.Serialize(rStream);

		// Write sprite instance

		if (m_pSprite != NULL)
			m_pSprite->SerializeInstance(rStream);
		else
			Object::SerializeNullInstance(rStream);

		// Write sprite animation ID

		rStream.WriteVar(&m_nSpriteAnimationID);

		// Write material instance

		m_materialInstance.Serialize(m_rEngine, rStream, false);

		// Write blend

		rStream.WriteVar(reinterpret_cast<const DWORD*>(&m_clrBlend));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Actor::Deserialize(Stream& rStream)
{
	try
	{
		Empty();

		// Read name

		m_strName.Deserialize(rStream);

		// Read flags

		rStream.ReadVar(&m_dwFlags);		

		// Read position

		m_vecPos.Deserialize(rStream);

		// Read sprite instance

		m_pSprite = m_rEngine.GetSprites().LoadInstance(rStream);

		// Read sprite animation ID

		rStream.ReadVar(&m_nSpriteAnimationID);

		if (m_nSpriteAnimationID < 0 ||
			m_nSpriteAnimationID >= m_pSprite->GetAnimationCount())
				throw m_rEngine.GetErrors().Push(Error::INVALID_INDEX,
					__FUNCTIONW__, L"m_nSpriteAnimationID");

		// Read material instance

		m_materialInstance.Deserialize(m_rEngine, rStream, false);

		// Read blend

		rStream.ReadVar(reinterpret_cast<DWORD*>(&m_clrBlend));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	// Update map's spacial grid and camera

	m_pLayer->GetSpace()->Add(this);
}

DWORD Actor::GetMemoryFootprint(void) const
{
	return Object::GetMemoryFootprint() +
		sizeof(Actor) -
		sizeof(Object) +
		(DWORD)m_strClass.GetLengthBytes();
}

void Actor::Empty(void)
{
	SetSprite(NULL);
}

void Actor::OnEnterCamera(void)
{
	// Default handler
}

void Actor::OnExitCamera(void)
{
	// Default handler
}

void Actor::OnLostDevice(bool bRecreate)
{
	// Default handler
}

void Actor::OnResetDevice(bool bRecreate)
{
	// Default handler
}

void Actor::OnSessionPause(bool bPause)
{
	// Default handler
	UNREFERENCED_PARAMETER(bPause);
}

void Actor::OnKeyDown(int nKeyCode)
{
	// Default handler
	UNREFERENCED_PARAMETER(nKeyCode);
}

void Actor::OnKeyUp(int nKeyCode)
{
	// Default handler
	UNREFERENCED_PARAMETER(nKeyCode);
}

void Actor::OnKeyPress(int nKeyCode)
{
	// Default handler
	UNREFERENCED_PARAMETER(nKeyCode);
}

void Actor::OnMouseMove(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseLDown(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseLUp(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseLDbl(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseRDown(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseRUp(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseRDbl(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseMDown(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseMUp(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseMDbl(POINT pt)
{
	// Default handler
	UNREFERENCED_PARAMETER(pt);
}

void Actor::OnMouseWheel(int nZDelta)
{
	// Default handler
	UNREFERENCED_PARAMETER(nZDelta);
}

void Actor::OnBoundsChange(const Rect& rrcOldBounds)
{
	m_pLayer->GetSpace()->Update(this, rrcOldBounds);
}