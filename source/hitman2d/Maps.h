/*------------------------------------------------------------------*\
|
| Maps.h
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D map class(es)
| Created: 12/21/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef HITMAN2D_MAPS_H
#define HITMAN2D_MAPS_H

/*----------------------------------------------------------*\
| Using Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace Hitman2D {


/*----------------------------------------------------------*\
| MapBase class
\*----------------------------------------------------------*/

class MapBase: public TileMap
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_CLASS[];

private:

public:
	MapBase(Engine& rEngine, LPCWSTR pszClass);
	~MapBase(void);

public:
	//
	// Creation
	//

	static Object* CreateInstance(Engine& rEngine, LPCWSTR pszClass, Object* pOwner);

	//
	// Update
	//

	virtual void Update(void);

	//
	// Rendering
	//

	virtual void Render();

	//
	// Events
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnUpdateCamera(void);

	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseRDown(POINT pt);
	virtual void OnMouseMDown(POINT pt);

	virtual void SerializeUserData(Stream& rStream, bool bInstance) const;
	virtual void DeserializeUserData(Stream& rStream, bool bInstance);

protected:
};

} // namespace Hitman2D

#endif // MAPS_H