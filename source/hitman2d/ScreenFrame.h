/*------------------------------------------------------------------*\
|
| ScreenFrame.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Frame control class
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef SCREEN_FRAME_H
#define SCREEN_FRAME_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| ScreenFrame class - skinned rectangle/line
\*----------------------------------------------------------*/

class ScreenFrame: public Screen
{
public:
	//
	// Constants
	//

	// Flags

	enum Flags
	{
		// Enable automatic sizing to fill container
		AUTOSIZE	= Screen::USERFLAG << 0,

		// Render as rectangle (4 corners + 4 edges + center)
		RECTANGLE	= Screen::USERFLAG << 1,

		// Render as horizontal line (2 corners + top edge)
		HORIZONTAL	= Screen::USERFLAG << 2,

		// Render as vertical line (2 corners + left edge)
		VERTICAL	= Screen::USERFLAG << 3,
	};

	// Parts

	enum Parts
	{
		// Top Left corner texture
		PART_TOPLEFT,

		// Top Right corner texture
		PART_TOPRIGHT,

		// Bottom left corner texture
		PART_BOTTOMLEFT,

		// Bottom right corner texture
		PART_BOTTOMRIGHT,

		// Top edge repeated texture
		PART_TOP,

		// Bottom edge repeated texture
		PART_BOTTOM,

		// Left edge repeated texture
		PART_LEFT,

		// Right edge repeated texture
		PART_RIGHT,

		// Center repeated texture
		PART_CENTER,

		PART_COUNT
	};

	// Elements

	static const LPCWSTR SZ_PARTS[];
	static const LPCWSTR SZ_FLAGS[];
	static const DWORD DW_FLAGS[];

	// Class Name

	static const WCHAR SZ_CLASS[];

private:
	//
	// Members
	//

	// Graphic parts of the frame
	MaterialInstance m_Elements[PART_COUNT];

	// Cached position of top right corner
	Vector2 m_vecTopRight;

	// Cached position of bottom right corner
	Vector2 m_vecBottomRight;

	// Cached position of center (top left corner of center repeat)
	Vector2 m_vecCenter;

public:
	ScreenFrame(Engine& rEngine,
		LPCWSTR pszClass, Screen* pParent);

	virtual ~ScreenFrame(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine,
		LPCWSTR pszClass, Object* pParent);

	//
	// Background
	//

	inline MaterialInstance& GetBackground(Parts nPart)
	{
		return m_Elements[nPart];
	}

	inline const MaterialInstance& GetBackgroundConst(Parts nPart) const
	{
		return m_Elements[nPart];
	}

	//
	// Serialization
	//

	virtual void Deserialize(const InfoElem& rRoot);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Events
	//

	virtual void OnMove(const POINT& ptOldPosition);
	virtual void OnSize(const SIZE& psOldSize);

	virtual void OnRender(Graphics& rGraphics, LPCRECT prc);

	virtual void OnCommand(int nCommandID, Screen* pSender = NULL, int nParam = 0);
	virtual int OnNotify(int nNotifyID, Screen* pSender = NULL, int nParam = 0);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseMove(POINT pt);

	virtual void OnThemeStyleChange(void);
};

} // namespace Hitman2D

#endif // SCREEN_FRAME_H