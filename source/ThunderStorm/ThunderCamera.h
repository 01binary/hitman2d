/*------------------------------------------------------------------*\
|
| ThunderCamera.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine camera class
| Created: 08/26/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_CAMERA_H
#define THUNDER_CAMERA_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderMath.h"	// using Vector2, Rect

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Camera;				// referencing Camera, declared below
class TileMap;				// referencing TileMap

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Camera*> CameraArray;
typedef std::vector<Camera*>::iterator CameraArrayIterator;
typedef std::vector<Camera*>::const_iterator CameraArrayConstIterator;


/*----------------------------------------------------------*\
| Camera class
\*----------------------------------------------------------*/

class Camera
{
private:
	//
	// Constants
	//

	static const D3DCOLOR WIRECOLOR_BACKGROUND;
	static const D3DCOLOR WIRECOLOR_ACTOR;

protected:
	//
	// Members
	//

	// Map currently viewing
	TileMap* m_pMap;

	// Camera position within tile map (in fraction of tile size)
	Vector2 m_vecPos;

	// Camera size (in fraction of tile size)
	Vector2 m_vecSize;

	// Camera zoom factor
	float m_fZoom;

	// Dest rectangle
	Rect m_rcDest;

	// Range visible in camera
	Rect m_rcVisibleRange;

	// View
	D3DXMATRIX m_mtxView;

	// Scaling of the view to fit in destination rect
	D3DXMATRIX m_mtxViewScale;

public:
	Camera(TileMap* pMap = NULL);
	virtual ~Camera(void);

public:
	//
	// Map
	//

	TileMap* GetMap(void);
	const TileMap* GetMapConst(void) const;

	void SetMap(TileMap* pMap);

	//
	// Position
	//

	Vector2& GetPosition(void);
	const Vector2& GetPositionConst(void) const;

	virtual void SetPosition(Vector2 vecPosition);
	virtual void SetPosition(float x, float y);

	virtual void Move(float fDeltaX, float fDeltaY);

	virtual void Focus(Vector2 vecAreaPosition, Vector2 vecAreaSize);

	//
	// Size
	//

	Vector2& GetSize(void);
	const Vector2& GetSizeConst(void) const;

	virtual void SetSize(Vector2 vecSize);
	virtual void SetSize(float x, float y);

	//
	// Zoom
	//

	float GetZoom(void) const;
	virtual void SetZoom(float fZoom);

	virtual void Zoom(float fDelta);

	//
	// Dest Rectangle
	//

	Rect& GetDestRect(void);
	const Rect& GetDestRectConst(void) const;
	virtual void SetDestRect(Rect rcDest);

	//
	// Visibility
	//

	const Rect& GetVisibleRange(void) const;

	bool PointVisible(Vector2 vecPoint);
	bool PointVisible(float x, float y);

	bool AreaVisible(Rect rcArea);
	bool AreaVisible(Vector2 vecAreaPosition, Vector2 vecAreaSize);

	//
	// Rendering
	//

	virtual void Render(void);

protected:
	//
	// Private Functions
	//

	virtual void Cache(void);
};

} // namespace ThunderStorm

#endif