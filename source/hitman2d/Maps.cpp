/*------------------------------------------------------------------*\
|
| Maps.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D map class(es) implementation
| Created: 12/21/2007
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"		// precompiled header
#include "Maps.h"		// defining map class(es)

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR MapBase::SZ_CLASS[] = L"base";


/*----------------------------------------------------------*\
| MapBase implementation
\*----------------------------------------------------------*/

MapBase::MapBase(Engine& rEngine, LPCWSTR pszClass):
				 TileMap(rEngine, pszClass)
{

}

MapBase::~MapBase(void)
{
}

Object* MapBase::CreateInstance(Engine& rEngine,
								LPCWSTR pszClass,
								Object* pOwner)
{
	UNREFERENCED_PARAMETER(pOwner);

	return new MapBase(rEngine, pszClass);
}

void MapBase::Update(void)
{
	TileMap::Update();
}

void MapBase::Render(void)
{
	TileMap::Render();
}

void MapBase::OnLostDevice(bool bRecreate)
{

}

void MapBase::OnResetDevice(bool bRecreate)
{

}

void MapBase::OnUpdateCamera(void)
{

}

void MapBase::OnMouseLDown(POINT pt)
{
	TileMap::OnMouseLDown(pt);

	if (m_rEngine.GetScreensConst().GetActiveScreen())
		m_rEngine.GetScreens().SetActiveScreen(NULL);

	if (m_rEngine.GetScreensConst().GetFocusScreen())
		m_rEngine.GetScreens().SetFocusScreen(NULL);
}

void MapBase::OnMouseRDown(POINT pt)
{
	TileMap::OnMouseRDown(pt);

	if (m_rEngine.GetScreensConst().GetActiveScreen())
		m_rEngine.GetScreens().SetActiveScreen(NULL);

	if (m_rEngine.GetScreensConst().GetFocusScreen())
		m_rEngine.GetScreens().SetFocusScreen(NULL);
}

void MapBase::OnMouseMDown(POINT pt)
{
	TileMap::OnMouseMDown(pt);

	if (m_rEngine.GetScreensConst().GetActiveScreen())
		m_rEngine.GetScreens().SetActiveScreen(NULL);

	if (m_rEngine.GetScreensConst().GetFocusScreen())
		m_rEngine.GetScreens().SetFocusScreen(NULL);
}

void MapBase::SerializeUserData(Stream& rStream, bool bInstance) const
{
	if (true == bInstance)
	{
		//
		// Write map instance data
		//

	}
	else
	{
		//
		// Write map data
		//

	}
}

void MapBase::DeserializeUserData(Stream& rStream, bool bInstance)
{
	if (true == bInstance)
	{
		//
		// Read map instance data
		//

	}
	else
	{
		//
		// Read map data
		//

	
	}
}