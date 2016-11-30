/*------------------------------------------------------------------*\
|
| ThunderCamera.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine camera class implementation
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderCamera.h"		// defining Camera
#include "ThunderEngine.h"		// using Graphics
#include "ThunderTileMap.h"		// using TileMap, TileLayer, Tile, Actor
#include "ThunderError.h"		// using Error

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const D3DCOLOR Camera::WIRECOLOR_BACKGROUND		= D3DCOLOR_XRGB(255, 255, 176);
const D3DCOLOR Camera::WIRECOLOR_ACTOR			= D3DCOLOR_XRGB(190, 0, 0);


/*----------------------------------------------------------*\
| Camera implementation
\*----------------------------------------------------------*/

Camera::Camera(TileMap* pMap): m_pMap(pMap),
							   m_fZoom(1.0f)
{
}

Camera::~Camera(void)
{
}

TileMap* Camera::GetMap(void)
{
	return m_pMap;
}

const TileMap* Camera::GetMapConst(void) const
{
	return m_pMap;
}

void Camera::SetMap(TileMap* pMap)
{
	m_pMap = pMap;

	Cache();
}

Vector2& Camera::GetPosition(void)
{
	return m_vecPos;
}

const Vector2& Camera::GetPositionConst(void) const
{
	return m_vecPos;
}

void Camera::SetPosition(Vector2 vecPosition)
{
	m_vecPos = vecPosition;

	Cache();
}

void Camera::SetPosition(float x, float y)
{
	m_vecPos.x = x;
	m_vecPos.y = y;

	Cache();
}

void Camera::Move(float fDeltaX, float fDeltaY)
{
	m_vecPos.x += fDeltaX;
	m_vecPos.y += fDeltaY;

	Cache();
}

void Camera::Focus(Vector2 vecAreaPosition, Vector2 vecAreaSize)
{
	SetPosition(-((m_vecSize.x - vecAreaSize.x) / 2.0f - vecAreaPosition.x),
		-((m_vecSize.y - vecAreaSize.y) / 2.0f - vecAreaPosition.y));
}

Vector2& Camera::GetSize(void)
{
	return m_vecSize;
}

const Vector2& Camera::GetSizeConst(void) const
{
	return m_vecSize;
}

void Camera::SetSize(Vector2 vecSize)
{
	m_vecSize = vecSize;

	Cache();
}

void Camera::SetSize(float x, float y)
{
	m_vecSize.x = x;
	m_vecSize.y = y;

	Cache();
}

float Camera::GetZoom(void) const
{
	return m_fZoom;
}

void Camera::SetZoom(float fZoom)
{
	m_fZoom = fZoom;
	
	Cache();
}

void Camera::Zoom(float fDelta)
{
	m_fZoom += fDelta;

	Cache();
}

Rect& Camera::GetDestRect(void)
{
	return m_rcDest;
}

const Rect& Camera::GetDestRectConst(void) const
{
	return m_rcDest;
}

void Camera::SetDestRect(Rect rcDest)
{
	m_rcDest = rcDest;

	Cache();
}

const Rect& Camera::GetVisibleRange(void) const
{
	return m_rcVisibleRange;
}

bool Camera::PointVisible(Vector2 vecPoint)
{
	return (vecPoint.x >= m_vecPos.x &&
		vecPoint.x <= (m_vecPos.x + m_vecSize.x) &&
		vecPoint.y >= m_vecPos.y &&
		vecPoint.y <= (m_vecPos.y + m_vecSize.y));
}

bool Camera::PointVisible(float x, float y)
{
	return (x >= m_vecPos.x && x <= (m_vecPos.x + m_vecSize.x) &&
		y >= m_vecPos.y && y <= (m_vecPos.y + m_vecSize.y));
}

bool Camera::AreaVisible(Rect rcArea)
{
	if (rcArea.right < int(m_vecPos.x))
		return false;

	if (rcArea.bottom < int(m_vecPos.y))
		return false;

	if (rcArea.left > int(ceil(m_vecPos.x + m_vecSize.x)))
		return false;

	if (rcArea.top > int(ceil(m_vecPos.y + m_vecSize.y)))
		return false;

	return true;
}

bool Camera::AreaVisible(Vector2 vecAreaPosition, Vector2 vecAreaSize)
{
	// TODO: need to use intersection code, not containment!
	// Use sep axis, that should be fine

	return true;
}

void Camera::Render(void)
{
	if (NULL == m_pMap)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Render layers

	Graphics& rGraphics = m_pMap->GetEngine().GetGraphics();

	float fTileSize =
		float(m_pMap->GetEngine().GetOption(Engine::OPTION_TILE_SIZE));

	Rect rcLayerRange;
	ActorArray arActors;

	for(int n = 0;
		n < m_pMap->GetLayerCount();
		n++)
	{
		TileLayer& rLayer = *m_pMap->GetLayer(n);

		if (rLayer.GetBounds().Intersect(m_rcVisibleRange,
			rcLayerRange) == true)
		{
			// Render tiles

			Vector2 vecTilePos = rLayer.GetPositionConst() - m_vecPos;

			for(int ty = 0; ty < rcLayerRange.bottom; ty++)
			{
				for(int tx = 0; tx < rcLayerRange.top; tx++)
				{
					Tile* pTile = rLayer.GetTile(tx, ty);

					if (pTile->IsFlagSet(Tile::ANIMATED))
					{
						TileAnimated* pTileAnimated =
							static_cast<TileAnimated*>(pTile);

						rGraphics.RenderQuad(pTileAnimated->GetMaterialInstance(),
							vecTilePos, pTileAnimated->GetBlendConst());
					}
					else
					{
						TileStatic* pTileStatic =
							static_cast<TileStatic*>(pTile);

						rGraphics.RenderQuad(pTileStatic->GetMaterialInstance(),
							vecTilePos, pTileStatic->GetBlendConst());
					}

					vecTilePos.x += fTileSize;
				}

				vecTilePos.x = rLayer.GetPositionConst().x - m_vecPos.x;
				vecTilePos.y += fTileSize;
			}

			// Render Actors

			rLayer.GetSpace()->Query(rcLayerRange, &arActors);

			for(ActorArrayIterator pos = arActors.begin();
				pos != arActors.end();
				pos++)
			{
				(*pos)->Render();
			}

			arActors.clear();
		}
	}
}

void Camera::Cache(void)
{
	if (NULL == m_pMap)
		return;

	// Calculate visible range scaled by zoom
	// using center point as reference

	Vector2 vecCenter = m_vecPos + m_vecSize * 0.5f;
	Vector2 vecScaledSize = m_vecSize * m_fZoom;
	Vector2 vecScaledPos = vecCenter - vecScaledSize * 0.5f;

	m_rcVisibleRange.left = int(floor(vecScaledPos.x));
	m_rcVisibleRange.top = int(floor(vecScaledPos.y));

	m_rcVisibleRange.right =
		int(ceil(vecScaledPos.x + vecScaledSize.x));

	m_rcVisibleRange.bottom =
		int(ceil(vecScaledPos.y + vecScaledSize.y));

	// Calculate scale required to proportionally fit
	// visible range into destination rectangle

	int nTileSize = m_pMap->GetEngineConst().GetOption(Engine::OPTION_TILE_SIZE);
	int nWidth = m_rcVisibleRange.GetWidth() * nTileSize;
	int nHeight = m_rcVisibleRange.GetHeight() * nTileSize;

	if (nWidth != m_rcDest.GetWidth() || nHeight != m_rcDest.GetHeight())
	{
		float fScale = 1.0f;

		if (nWidth > nHeight)
			fScale = float(nWidth) / float(m_rcDest.GetWidth());
		else
			fScale = float(nHeight) / float(m_rcDest.GetHeight());

		D3DXMatrixScaling(&m_mtxViewScale, fScale, fScale, fScale);

		// Calculate view translation

		D3DXMatrixTranslation(&m_mtxView, vecScaledPos.x, vecScaledPos.y, 0.0f);

		// Calculate combined view matrix

		D3DXMatrixMultiply(&m_mtxView, &m_mtxView, &m_mtxViewScale);
	}
	else
	{
		D3DXMatrixIdentity(&m_mtxViewScale);
		D3DXMatrixIdentity(&m_mtxView);
	}
}