/*------------------------------------------------------------------*\
|
| ScreenStats.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Statistics Screen implementation
| Created: 03/03/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"			// precompiled header
#include "Globals.h"		// using global constants
#include "Game.h"			// using Game
#include "Error.h"			// using error constants
#include "ScreenStats.h"	// defining ScreenStats

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenStats::SZ_CLASS[] = L"overlapped::stats";


/*----------------------------------------------------------*\
| ScreenStats implementation
\*----------------------------------------------------------*/

ScreenStats::ScreenStats(Engine& rEngine, LPCWSTR pszClass, Screen* pParent):
						 ScreenOverlapped(rEngine, pszClass, pParent)
{
}

ScreenStats::~ScreenStats(void)
{
}

Object* ScreenStats::CreateInstance(Engine& rEngine, LPCWSTR pszClass, Object* pParent)
{
	return new ScreenStats(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenStats::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	Screen::OnRender(rGraphics);

	// Render statistics

	String strStats;

	strStats.Format(
		L"renderables.......%d\n"
		L"triangles.........%d\n"
		L"lines.............%d\n"
		L"points............%d\n"
		L"batches...........%d\n\n"
		L"max prims/batch...%d\n\n"
		L"state changes.....%d\n"
		L"filtered changes..%d\n",

		rGraphics.GetRenderableCount(),
		rGraphics.GetTriangleCount(),
		rGraphics.GetLineCount(),
		rGraphics.GetPointCount(),
		rGraphics.GetBatchCount(),
		rGraphics.GetMaxPrimitivesPerBatch(),
		rGraphics.GetStates()->GetStateChangeCount(),
		rGraphics.GetStates()->GetFilteredStateChangeCount()
	);

	Rect rcText = GetBufferRect();

	rcText.Inflate(-20, -20);

	if (m_pFont != NULL)
	{
		m_pFont->RenderText(rcText, strStats, -1, m_clrBackColor);

		SIZE extent = m_pFont->GetTextExtent(strStats, -1, 0, rcText.GetWidth());

		if (m_psSize.cy != extent.cy + 60)
			SetSize(m_psSize.cx, extent.cy + 60);
	}
}