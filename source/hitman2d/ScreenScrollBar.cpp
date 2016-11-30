/*------------------------------------------------------------------*\
|
| ScreenScrollBar.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D ScrollBar Control implementation
| Created: 03/02/2011
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "Globals.h"			// using global constants
#include "Game.h"				// using Game
#include "Error.h"				// using error constants
#include "ScreenScrollBar.h"	// defining ScreenScrollBar

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenScrollBar::SZ_MIN[] =					L"min";
const WCHAR ScreenScrollBar::SZ_MAX[] =					L"max";
const WCHAR ScreenScrollBar::SZ_POS[] =					L"value";
const WCHAR ScreenScrollBar::SZ_PAGE[] =				L"page";
const WCHAR ScreenScrollBar::SZ_WAITARROWAUTOSCROLL[] =	L"wait-arrow-auto-scroll";
const WCHAR ScreenScrollBar::SZ_ARROWAUTOSCROLLINT[] =	L"arrow-auto-scroll-interval";
const WCHAR ScreenScrollBar::SZ_WAITSHAFTAUTOSCROLL[] =	L"wait-shaft-auto-scroll";
const WCHAR ScreenScrollBar::SZ_SHAFTAUTOSCROLLINT[] =	L"shaft-auto-scroll-interval";
const WCHAR ScreenScrollBar::SZ_DISABLEDBLEND[]	=		L"disabled.color";
const WCHAR ScreenScrollBar::SZ_CLASS[] =				L"scrollbar";

const LPCWSTR ScreenScrollBar::SZ_FLAGS[] =			{
														L"horizontal",
														L"vertical",
														L"spinner",
														L"slider"
													};

const DWORD ScreenScrollBar::DW_FLAGS[] =			{
														ScreenScrollBar::HORIZONTAL,
														ScreenScrollBar::VERTICAL,
														ScreenScrollBar::SPINNER,
														ScreenScrollBar::SLIDER
													};

const LPCWSTR ScreenScrollBar::SZ_PARTS_HORZ[] =	{
														L"leftarrow",
														L"rightarrow",
														L"shaftleft",
														L"shaftcenter",
														L"shaftright",
														L"thumbleft",
														L"thumbrepeatleft",
														L"thumbcenter",
														L"thumbrepeatright",
														L"thumbright"
													};

const LPCWSTR ScreenScrollBar::SZ_PARTS_VERT[] =	{
														L"uparrow",
														L"downarrow",
														L"shafttop",
														L"shaftcenter",
														L"shaftbottom",
														L"thumbtop",
														L"thumbrepeattop",
														L"thumbcenter",
														L"thumbrepeatbottom",
														L"thumbbottom"
													};

const LPCWSTR ScreenScrollBar::SZ_PART_STATES[] =	{
														L"normal",
														L"hover",
														L"pushed",
														L"disabled"
													};


/*----------------------------------------------------------*\
| ScreenScrollBar implementation
\*----------------------------------------------------------*/

ScreenScrollBar::ScreenScrollBar(Engine& rEngine,
								 LPCWSTR pszClass,
								 Screen* pParent):

								 Screen(rEngine, pszClass, pParent),
								 m_nMin(0),
								 m_nMax(100),
								 m_nPos(0),
								 m_fPage(50.0f),
								 m_nShaftSize(0),
								 m_nThumbSize(0),
								 m_nThumbPos(0),
								 m_nMinThumbEdge(0),
								 m_nMinThumbCenter(0),
								 m_nThumbCenter(0),
								 m_nDistributeEdge(0),
								 m_nDistributeCenter(0),
								 m_fPixelsPerUnit(0.0f),
								 m_fWaitArrowAutoScroll(0.15f),
								 m_fArrowAutoScrollInterval(0.1f),
								 m_fWaitShaftAutoScroll(0.18f),
								 m_fShaftAutoScrollInterval(0.1f),
								 m_pTimerAutoScroll(NULL)
{
	ZeroMemory(m_nElemStates, sizeof(m_nElemStates));
	ZeroMemory(m_rcHitTest, sizeof(m_rcHitTest));
	
	m_nShaftPartsSize[0] = 0;
	m_nShaftPartsSize[1] = 0;

	m_ptDragOffset.x = 0;
	m_ptDragOffset.y = 0;
}

ScreenScrollBar::~ScreenScrollBar(void)
{
}

Object* ScreenScrollBar::CreateInstance(Engine& rEngine,
										LPCWSTR pszClass,
										Object* pParent)
{
	return new ScreenScrollBar(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenScrollBar::SetFlags(DWORD dwFlags)
{
	Screen::SetFlags(dwFlags);

	if (IsFlagSet(DISABLED) == true)
		m_clrBlend = m_clrDisabledBlend;
	else
		m_clrBlend = Color::BLEND_ONE;
}

void ScreenScrollBar::SetMin(int nMin)
{
	m_nMin = nMin;

	UpdateThumbSize();
}

void ScreenScrollBar::SetMax(int nMax)
{
	m_nMax = nMax;

	UpdateThumbSize();
}

void ScreenScrollBar::SetValue(int nValue)
{
	if (m_nPos == nValue)
		return;

	if (nValue < m_nMin)
	{
		nValue = m_nMin;
	}
	else if (nValue > m_nMax)
	{
		nValue = m_nMax;
	}

	m_nPos = nValue;

	UpdateThumbFromPosition();

	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(NOTIFY_SCROLL,
			this, SCROLL_USER);
	}
}

void ScreenScrollBar::SetPageSize(float fPageSize)
{
	m_fPage = fPageSize > 0.0f ? fPageSize : 1.0f;

	UpdateThumbSize();
}

void ScreenScrollBar::SetDisabledBlend(D3DCOLOR clrDisabledBlend)
{
	m_clrDisabledBlend = clrDisabledBlend;

	if (IsFlagSet(DISABLED) == true)
		SetBlend(clrDisabledBlend);
}

void ScreenScrollBar::OnRender(Graphics& rGraphics, LPCRECT prc)
{
	Vector2 vecPos = m_vecCachedPos;

	Color blend = GetFrontBufferBlend();

	if (IsFlagSet(HORIZONTAL) == true)
	{
		// If not enough size to render both arrows, exit early
		// Don't wanna make ourselves look stupid

		if (m_psSize.cx < int(
			(m_rcHitTest[HIT_LEFTARROW].right - m_rcHitTest[HIT_LEFTARROW].left) +
			 (m_rcHitTest[HIT_RIGHTARROW].right - m_rcHitTest[HIT_RIGHTARROW].left)))
			return;

		if (IsFlagSet(SPINNER) == false)
		{
			// Render shaft left

			rGraphics.RenderQuad(
				m_Parts[PART_SHAFTLEFT][m_nElemStates[HIT_SHAFTLEFT]],
				m_vecCachedPos, blend);			

			// Render shaft center #1, adjusting size

			vecPos.x += float(m_Parts[PART_SHAFTLEFT]
				[m_nElemStates[HIT_SHAFTLEFT]].GetTextureCoords().GetWidth());

			if (m_nShaftPartsSize[0] > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_SHAFTCENTER][m_nElemStates[HIT_SHAFTLEFT]],
					vecPos,
					Vector2(m_nShaftPartsSize[0],
							m_Parts[PART_SHAFTCENTER]
								[m_nElemStates[HIT_SHAFTLEFT]].GetTextureCoords().GetHeight()),
					blend);

				vecPos.x += float(m_nShaftPartsSize[0]);
			}

			// Render shaft center #2, adjusting size

			if (m_nShaftPartsSize[1] > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_SHAFTCENTER][m_nElemStates[HIT_SHAFTRIGHT]],
					vecPos,
					Vector2(m_nShaftPartsSize[1],
							m_Parts[PART_SHAFTCENTER]
								[m_nElemStates[HIT_SHAFTRIGHT]].GetTextureCoords().GetHeight()),
					blend);

				vecPos.x += float(m_nShaftPartsSize[1]);
			}

			// Render shaft right

			rGraphics.RenderQuad(m_Parts[PART_SHAFTRIGHT][m_nElemStates[HIT_SHAFTRIGHT]],
				vecPos, blend);
		}
	}
	else
	{
		// If not enough size to render both arrows, exit early
		// Don't wanna make ourselves look stupid

		if (m_psSize.cy < int(
			(m_rcHitTest[HIT_UPARROW].bottom - m_rcHitTest[HIT_UPARROW].top) +
			 (m_rcHitTest[HIT_DOWNARROW].bottom - m_rcHitTest[HIT_DOWNARROW].top)))
			return;

		if (IsFlagSet(SPINNER) == false)
		{
			// Render shaft top

			rGraphics.RenderQuad(
				m_Parts[PART_SHAFTTOP][m_nElemStates[HIT_SHAFTTOP]],
				m_vecCachedPos, blend);

			// Render shaft center #1, adjusting size

			vecPos.y += float(m_Parts[PART_SHAFTTOP]
				[m_nElemStates[HIT_SHAFTTOP]].GetTextureCoords().GetHeight());

			if (m_nShaftPartsSize[0] > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_SHAFTCENTER][m_nElemStates[HIT_SHAFTTOP]],
					vecPos,
					Vector2(m_Parts[PART_SHAFTCENTER]
								[m_nElemStates[HIT_SHAFTTOP]].GetTextureCoords().GetWidth(),
							m_nShaftPartsSize[0]),
					blend);

				vecPos.y += float(m_nShaftPartsSize[0]);
			}

			// Render shaft center #2, adjusting size	

			if (m_nShaftPartsSize[1] > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_SHAFTCENTER][m_nElemStates[HIT_SHAFTBOTTOM]],
					vecPos,
					Vector2(m_Parts[PART_SHAFTCENTER]
							[m_nElemStates[HIT_SHAFTBOTTOM]].GetTextureCoords().GetWidth(),
							m_nShaftPartsSize[1]),
					blend);

				vecPos.y += float(m_nShaftPartsSize[1]);
			}

			// Render shaft bottom

			rGraphics.RenderQuad(
				m_Parts[PART_SHAFTBOTTOM][m_nElemStates[HIT_SHAFTBOTTOM]],
				vecPos, blend);
		}
	}

	// Render arrows if used as scrollbar or spinner. Sliders don't have arrows.

	if (IsFlagSet(SLIDER) == false)
	{
		// Render up/left arrow

		vecPos.x = m_vecCachedPos.x + float(m_rcHitTest[HIT_UPARROW].left);
		vecPos.y = m_vecCachedPos.y + float(m_rcHitTest[HIT_UPARROW].top);

		rGraphics.RenderQuad(m_Parts[PART_UPARROW][m_nElemStates[HIT_UPARROW]],
			vecPos, blend);

		// Render down/right arrow

		vecPos.x = m_vecCachedPos.x + float(m_rcHitTest[HIT_DOWNARROW].left);
		vecPos.y = m_vecCachedPos.y + float(m_rcHitTest[HIT_DOWNARROW].top);

		rGraphics.RenderQuad(m_Parts[PART_DOWNARROW][m_nElemStates[HIT_DOWNARROW]],
			vecPos, blend);
	}

	// If spinner, rendering only arrows, no shaft or thumb.

	if (IsFlagSet(SPINNER) == true)
		return;

	// Render thumb elements

	if (IsFlagSet(HORIZONTAL) == true)
	{
		// Render thumb left

		vecPos.x = m_vecCachedPos.x +
			float(m_rcHitTest[HIT_LEFTARROW].right + m_nThumbPos);

		vecPos.y = m_vecCachedPos.y;

		rGraphics.RenderQuad(m_Parts[PART_THUMBLEFT]
			[m_nElemStates[HIT_THUMB]],
			vecPos, blend);

		vecPos.x += float(m_Parts[PART_THUMBLEFT]
			[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetWidth());

		if (IsFlagSet(SLIDER) == false)
		{
			// Render thumb left stretch
			
			if (m_nDistributeEdge > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_THUMBREPLEFT][m_nElemStates[HIT_THUMB]],
					vecPos,
					Vector2(m_nDistributeEdge,
							m_Parts[PART_THUMBREPLEFT]
								[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetHeight()),
					blend);

				vecPos.x += float(m_nDistributeEdge);
			}

			// Render thumb center if needed		

			if (m_nThumbCenter >= m_nMinThumbCenter)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_THUMBCENTER][m_nElemStates[HIT_THUMB]],
					vecPos, blend);

				vecPos.x = vecPos.x + float(m_Parts[PART_THUMBCENTER]
					[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetWidth());
			}

			// Render thumb center stretch

			if (m_nDistributeCenter > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_THUMBREPRIGHT][m_nElemStates[HIT_THUMB]],
					vecPos,
					Vector2(m_nDistributeCenter,
							m_Parts[PART_THUMBREPRIGHT]
								[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetHeight()),
					blend);

				vecPos.x += float(m_nDistributeCenter);
			}
		}

		// Render thumb right

		rGraphics.RenderQuad(m_Parts[PART_THUMBRIGHT]
			[m_nElemStates[HIT_THUMB]],
			vecPos, blend);
	}
	else
	{
		// Render thumb top

		vecPos.x = m_vecCachedPos.x;
		vecPos.y = m_vecCachedPos.y +
			float(m_rcHitTest[HIT_UPARROW].bottom + m_nThumbPos);

		rGraphics.RenderQuad(m_Parts[PART_THUMBTOP]
			[m_nElemStates[HIT_THUMB]],
			vecPos, blend);

		vecPos.y += float(m_Parts[PART_THUMBTOP]
			[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetHeight());

		if (IsFlagSet(SLIDER) == false)
		{
			// Render thumb top stretch
			
			if (m_nDistributeEdge > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_THUMBREPTOP][m_nElemStates[HIT_THUMB]],
					vecPos,
					Vector2(m_Parts[PART_THUMBREPTOP]
								[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetWidth(),
							m_nDistributeEdge),
					blend);

				vecPos.y += float(m_nDistributeEdge);
			}

			// Render thumb center if needed		

			if (m_nThumbCenter >= m_nMinThumbCenter)
			{
				rGraphics.RenderQuad(m_Parts[PART_THUMBCENTER]
					[m_nElemStates[HIT_THUMB]],
					vecPos, blend);

				vecPos.y += float(m_Parts[PART_THUMBCENTER]
					[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetHeight());
			}

			// Render thumb center stretch

			if (m_nDistributeCenter > 0)
			{
				rGraphics.RenderQuad(
					m_Parts[PART_THUMBREPBOTTOM][m_nElemStates[HIT_THUMB]],
					vecPos,
					Vector2(m_Parts[PART_THUMBREPBOTTOM]
								[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetWidth(),
								m_nDistributeCenter),
					blend);

				vecPos.y += float(m_nDistributeCenter);
			}
		}

		// Render thumb bottom

		rGraphics.RenderQuad(m_Parts[PART_THUMBBOTTOM]
			[m_nElemStates[HIT_THUMB]],
			vecPos, blend);
	}
}

void ScreenScrollBar::Deserialize(const InfoElem& rRoot)
{
	Screen::Deserialize(rRoot);

	// Read scrollbar flags

	const InfoElem* pElem = rRoot.FindChildConst(SZ_SCREEN_FLAGS);

	if (pElem != NULL)
		m_dwFlags |= pElem->ToFlags(SZ_FLAGS, DW_FLAGS,
			sizeof(DW_FLAGS) / sizeof(DWORD));

	// Read Min

	pElem = rRoot.FindChildConst(SZ_MIN,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
		m_nMin = pElem->GetIntValue();

	// Read Max

	pElem = rRoot.FindChildConst(SZ_MAX,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
		m_nMax = pElem->GetIntValue();

	// Read Position

	pElem = rRoot.FindChildConst(SZ_POS,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
		m_nPos = pElem->GetIntValue();

	// Read Page

	pElem = rRoot.FindChildConst(SZ_PAGE,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
	{
		m_fPage = float(pElem->GetIntValue());
	}
	else
	{
		pElem = rRoot.FindChildConst(SZ_PAGE,
			InfoElem::TYPE_VALUE, Variable::TYPE_FLOAT);

		if (pElem != NULL)
			m_fPage = pElem->GetFloatValue();
	}

	// (Re)load style

	if (m_pStyle != NULL)
		OnThemeStyleChange();

	// Send the first scroll notification

	if (m_pParent != NULL)
		m_pParent->OnNotify(NOTIFY_SCROLL, this, SCROLL_USER);
}

void ScreenScrollBar::OnThemeStyleChange(void)
{
	Screen::OnThemeStyleChange();

	// Read material instance for all elements

	WCHAR szElemName[64] = {0};

	for(int n = 0; n < PART_COUNT; n++)
	{
		wcscpy_s(szElemName, 64,
			IsFlagSet(ScreenScrollBar::HORIZONTAL) ?
			SZ_PARTS_HORZ[n] : SZ_PARTS_VERT[n]);

		wcscat_s(szElemName, 64, L".");

		LPWSTR pszElemState = szElemName + wcslen(szElemName);
		
		for(int j = 0; j < PART_STATE_COUNT; j++)
		{
			wcscpy_s(pszElemState, 64 - (pszElemState - szElemName),
				SZ_PART_STATES[j]);

			wcscat_s(pszElemState, 64 - (pszElemState - szElemName),
				SZ_MATERIAL_POSTFIX);

			if (LoadMaterialInstance(m_Parts[n][j], szElemName,
				NULL, m_pStyle) == false)
			{
				if (j > 0)
					m_Parts[n][j] = m_Parts[n][j - 1];
			}
		}

		// Resize width/height automatically

		if (IsFlagSet(HORIZONTAL) == true)
		{
			if (IsFlagSet(SLIDER) == true)
				m_psSize.cy = m_Parts[PART_THUMBLEFT]
					[0].GetTextureCoords().GetHeight();
			else
				m_psSize.cy = m_Parts[PART_LEFTARROW]
					[0].GetTextureCoords().GetHeight();
		}
		else
		{
			if (IsFlagSet(SLIDER) == true)
				m_psSize.cx = m_Parts[PART_THUMBTOP]
					[0].GetTextureCoords().GetWidth();
			else
				m_psSize.cx = m_Parts[PART_UPARROW]
					[0].GetTextureCoords().GetWidth();
		}

		// Read wait arrow auto scroll time

		const Variable* pVar = NULL;

		if (LoadVariable(&pVar, SZ_WAITARROWAUTOSCROLL,
			Variable::TYPE_FLOAT, Variable::TYPE_INT, NULL, m_pStyle) == true)
		{
			if (pVar->GetVarType() == Variable::TYPE_INT)
				m_fWaitArrowAutoScroll = float(pVar->GetIntValue());
			else
				m_fWaitArrowAutoScroll = pVar->GetFloatValue();
		}

		// Read auto scroll interval

		if (LoadVariable(&pVar, SZ_ARROWAUTOSCROLLINT,
			Variable::TYPE_FLOAT, Variable::TYPE_INT, NULL, m_pStyle) == true)
		{
			if (pVar->GetVarType() == Variable::TYPE_INT)
				m_fArrowAutoScrollInterval = float(pVar->GetIntValue());
			else
				m_fArrowAutoScrollInterval = pVar->GetFloatValue();
		}

		// Read wait shaft auto scroll time

		if (LoadVariable(&pVar, SZ_WAITSHAFTAUTOSCROLL,
			Variable::TYPE_FLOAT, Variable::TYPE_INT, NULL, m_pStyle) == true)
		{
			if (pVar->GetVarType() == Variable::TYPE_INT)
				m_fWaitShaftAutoScroll = float(pVar->GetIntValue());
			else
				m_fWaitShaftAutoScroll = pVar->GetFloatValue();
		}

		// Read shaft auto scroll interval

		if (LoadVariable(&pVar, SZ_SHAFTAUTOSCROLLINT,
			Variable::TYPE_FLOAT, Variable::TYPE_INT, NULL, m_pStyle) == true)
		{
			if (pVar->GetVarType() == Variable::TYPE_INT)
				m_fShaftAutoScrollInterval = float(pVar->GetIntValue());
			else
				m_fShaftAutoScrollInterval = pVar->GetFloatValue();
		}

		// Read disabled alpha

		if (!LoadColor(m_clrDisabledBlend, SZ_DISABLEDBLEND, NULL, m_pStyle))
		{
			m_clrDisabledBlend = Color(1.0f, 1.0f, 1.0f, 0.5f);
		}
	}

	if (IsFlagSet(HORIZONTAL) == true)
	{
		m_nMinThumbEdge = m_Parts[PART_THUMBLEFT]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth() +
			m_Parts[PART_THUMBRIGHT]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();

		m_nMinThumbCenter = m_Parts[PART_THUMBCENTER]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();
	}
	else
	{
		
		m_nMinThumbEdge = m_Parts[PART_THUMBTOP]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight() +
			m_Parts[PART_THUMBBOTTOM]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_nMinThumbCenter = m_Parts[PART_THUMBCENTER]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();
	}
}

DWORD ScreenScrollBar::GetMemoryFootprint(void) const
{
	return sizeof(ScreenScrollBar) - sizeof(Screen) * 2 +
		   Screen::GetMemoryFootprint();
}

void ScreenScrollBar::OnSize(const SIZE& rpsOldSize)
{
	if (IsFlagSet(HORIZONTAL) == true)
	{
		m_rcHitTest[HIT_LEFTARROW].left = 0;

		m_rcHitTest[HIT_LEFTARROW].top = 0;

		m_rcHitTest[HIT_LEFTARROW].right = m_Parts[PART_LEFTARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();

		m_rcHitTest[HIT_LEFTARROW].bottom = m_Parts[PART_LEFTARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_rcHitTest[HIT_RIGHTARROW].left = m_psSize.cx - m_Parts[PART_RIGHTARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();

		m_rcHitTest[HIT_RIGHTARROW].top = 0;

		m_rcHitTest[HIT_RIGHTARROW].right = m_psSize.cx;

		m_rcHitTest[HIT_RIGHTARROW].bottom = m_Parts[PART_RIGHTARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_rcHitTest[HIT_THUMB].top = 0;	

		m_rcHitTest[HIT_THUMB].bottom = m_Parts[PART_THUMBLEFT]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();
	}
	else
	{
		m_rcHitTest[HIT_UPARROW].left = 0;
		m_rcHitTest[HIT_UPARROW].top = 0;

		m_rcHitTest[HIT_UPARROW].right = m_Parts[PART_UPARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();

		m_rcHitTest[HIT_UPARROW].bottom = m_Parts[PART_UPARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_rcHitTest[HIT_DOWNARROW].left = 0;

		m_rcHitTest[HIT_DOWNARROW].top =
			m_psSize.cy - m_Parts[PART_DOWNARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_rcHitTest[HIT_DOWNARROW].right = m_Parts[PART_DOWNARROW]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();

		m_rcHitTest[HIT_DOWNARROW].bottom = m_psSize.cy;

		m_rcHitTest[HIT_THUMB].left = 0;

		m_rcHitTest[HIT_THUMB].right = m_Parts[PART_THUMBLEFT]
			[PART_STATE_NORMAL].GetTextureCoords().GetWidth();
	}

	// Resize shaft

	UpdateShaftSize();

	// Resize thumb

	UpdateThumbSize();
}

void ScreenScrollBar::OnMouseMove(POINT pt)
{
	if (PART_STATE_PUSHED == m_nElemStates[HIT_THUMB])
	{
		// Dragging thumb?

		if (IsFlagSet(HORIZONTAL))
		{
			// Calculate new relative position of thumb within the shaft

			m_nThumbPos = pt.x + m_ptDragOffset.x - m_nThumbSize -
						  m_rcHitTest[HIT_SHAFTLEFT].left;

			if (m_nThumbPos < 0)
			{
				// Prevent thumb from going through the left of the shaft

				m_nThumbPos = 0;
			}
			else if (m_rcHitTest[HIT_SHAFTLEFT].left +
					m_nThumbPos + m_nThumbSize >
					m_rcHitTest[HIT_SHAFTRIGHT].right)
			{
				// Prevent scrollbar from resetting to 0 when dragging right,
				// and reset to zero when dragged left past limit

				if (pt.x <= 65535 && pt.x >= 60000)
				{
					m_nThumbPos = 0;
				}
				else
				{
					m_nThumbPos = m_rcHitTest[HIT_SHAFTRIGHT].right -
						m_nThumbSize - m_rcHitTest[HIT_SHAFTLEFT].left;
				}
			}

			// Update thumb

			m_rcHitTest[HIT_THUMB].left =
				m_rcHitTest[HIT_SHAFTLEFT].left + m_nThumbPos;

			m_rcHitTest[HIT_THUMB].top = 0;

			m_rcHitTest[HIT_THUMB].right =
				m_rcHitTest[HIT_SHAFTLEFT].left +
				m_nThumbPos + m_nThumbSize;

			m_rcHitTest[HIT_THUMB].bottom =
				m_rcHitTest[HIT_THUMB].top +
					m_Parts[PART_THUMBLEFT]
					[PART_STATE_NORMAL].GetTextureCoords().GetSize().cy;
		}
		else
		{
			// Calculate new relative position of thumb within the shaft

			m_nThumbPos = pt.y + m_ptDragOffset.y - m_nThumbSize -
						  m_rcHitTest[HIT_SHAFTTOP].top;

			if (m_nThumbPos < 0)
			{
				// Prevent thumb from going through the top of the shaft

				m_nThumbPos = 0;
			}
			else if (m_rcHitTest[HIT_SHAFTTOP].top +
					m_nThumbPos + m_nThumbSize >
					m_rcHitTest[HIT_SHAFTBOTTOM].bottom)
			{
				// Prevent scrollbar from resetting to 0 when dragging down,
				// and reset to zero when dragged up past limit

				if (pt.y <= 65535 && pt.y >= 60000)
				{
					m_nThumbPos = 0;
				}
				else
				{
					m_nThumbPos = m_rcHitTest[HIT_SHAFTBOTTOM].bottom -
						m_nThumbSize - m_rcHitTest[HIT_SHAFTTOP].top;
				}
			}

			// Update thumb

			m_rcHitTest[HIT_THUMB].left = 0;

			m_rcHitTest[HIT_THUMB].top =
				m_rcHitTest[HIT_SHAFTTOP].top + m_nThumbPos;

			m_rcHitTest[HIT_THUMB].right =
				m_rcHitTest[HIT_SHAFTTOP].right;

			m_rcHitTest[HIT_THUMB].bottom =
				m_rcHitTest[HIT_SHAFTTOP].top +
				m_nThumbPos + m_nThumbSize;

			// Re-render if buffered

			Invalidate();
		}

		UpdatePositionFromThumb();

		// If slider, snap to possible positions

		if (IsFlagSet(SLIDER) == true)
			UpdateThumbFromPosition();

		// Re-render if buffered

		Invalidate();
	}
	else if (PART_STATE_PUSHED == m_nElemStates[HIT_SHAFTTOP] ||
			PART_STATE_PUSHED == m_nElemStates[HIT_SHAFTBOTTOM] )
	{
		// Scrolling with shaft. Update drag offset

		m_ptDragOffset.x = pt.x;
		m_ptDragOffset.y = pt.y;

		// Re-render if buffered

		Invalidate();
	}
	else
	{
		// If not dragging thumb, update states for
		// all elements in response to mouse

		PartHitTest nHit = HitTest(pt);

		for(int nHitElem = 0;
			nHitElem < HIT_COUNT;
			nHitElem++)
		{
			if (nHitElem == nHit)
			{
				// Mouse is over this element

				if (true == m_bMouseDown)
				{
					if (m_nHitOnMouseDown == nHit)
					{
						// If mouse was pushed down on this element,
						// set its state to pushed

						m_nElemStates[nHitElem] =
							PART_STATE_PUSHED;
					}
					else
					{
						// If mouse was not pushed down on this element,
						// set its state to hover

						m_nElemStates[nHitElem] =
							PART_STATE_HOVER;
					}
				}
				else
				{
					// If mouse was never pushed down,
					// set element state to hover

					m_nElemStates[nHitElem] = PART_STATE_HOVER;
				}
			}
			else
			{
				// Mouse is not over this element, set its state to normal

				m_nElemStates[nHitElem] = PART_STATE_NORMAL;
			}
		}

		// Re-render if buffered

		Invalidate();
	}
}

void ScreenScrollBar::OnMouseLDbl(POINT pt)
{
	return OnMouseLDown(pt);
}

void ScreenScrollBar::OnMouseLDown(POINT pt)
{
	Activate();

	if (IsFlagSet(SPINNER) == true || IsFlagSet(SLIDER) == true)
		m_rEngine.GetScreens().SetFocusScreen(this);

	m_bMouseDown = true;

	m_nHitOnMouseDown = HitTest(pt);

	switch(m_nHitOnMouseDown)
	{
	case HIT_LEFTARROW:
		{
			m_nElemStates[HIT_LEFTARROW] = PART_STATE_PUSHED;

			m_rEngine.GetTimers().Add(this,
				m_fWaitArrowAutoScroll, TIMER_WAITARROW);

			if (m_nPos > m_nMin)
			{
				m_nPos--;
				UpdateThumbFromPosition();

				if (m_pParent != NULL)
				{
					m_pParent->OnNotify(NOTIFY_SCROLL,
						this, SCROLL_ARROW);
				}
			}

			// Capture the mouse so we get a mouse-up event for sure

			m_rEngine.GetScreens().SetCaptureScreen(this);

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_RIGHTARROW:
		{
			m_nElemStates[HIT_RIGHTARROW] = PART_STATE_PUSHED;

			m_rEngine.GetTimers().Add(this,
				 m_fWaitArrowAutoScroll, TIMER_WAITARROW);

			if (m_nPos < m_nMax)
			{
				m_nPos++;
				UpdateThumbFromPosition();

				if (m_pParent != NULL)
				{
					m_pParent->OnNotify(NOTIFY_SCROLL,
						this, SCROLL_ARROW);
				}
			}

			// Capture the mouse so we get a mouse-up event for sure

			m_rEngine.GetScreens().SetCaptureScreen(this);

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_THUMB:
		{
			m_nElemStates[HIT_THUMB] = PART_STATE_PUSHED;

			m_ptDragOffset.x =
				m_rcHitTest[HIT_THUMB].right - pt.x;

			m_ptDragOffset.y =
				m_rcHitTest[HIT_THUMB].bottom - pt.y;

			// Capture the mouse so we get a mouse-up event for sure

			m_rEngine.GetScreens().SetCaptureScreen(this);

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_SHAFTTOP:
	case HIT_SHAFTBOTTOM:
		{
			m_nElemStates[m_nHitOnMouseDown] = PART_STATE_PUSHED;

			// Set the offset

			m_ptDragOffset.x = pt.x;
			m_ptDragOffset.y = pt.y;

			// Scroll for the first time

			ScrollShaft();

			// Set the timer to wait before shaft auto-scrolling begins

			m_rEngine.GetTimers().Add(this,
				m_fWaitShaftAutoScroll, TIMER_WAITSHAFT);

			// Capture the mouse so we get a mouse-up event for sure

			m_rEngine.GetScreens().SetCaptureScreen(this);

			// Invalidate if buffered

			Invalidate();
		}
		break;
	}	
}

void ScreenScrollBar::OnMouseLUp(POINT pt)
{
	m_bMouseDown = false;

	switch(m_nHitOnMouseDown)
	{
	case HIT_LEFTARROW:
		{
			if (HitTest(pt) == PART_LEFTARROW)
			{
				m_nElemStates[HIT_LEFTARROW] =
					PART_STATE_HOVER;
			}
			else
			{
				m_nElemStates[HIT_LEFTARROW] =
					PART_STATE_NORMAL;
			}

			m_rEngine.GetTimers().Remove(this, TIMER_WAITARROW);

			if (m_pTimerAutoScroll != NULL)
			{
				m_rEngine.GetTimers().Remove(this, TIMER_ARROW);
				m_pTimerAutoScroll = NULL;
			}

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_RIGHTARROW:
		{
			if (HitTest(pt) == HIT_RIGHTARROW)
			{
				m_nElemStates[HIT_RIGHTARROW] =
					PART_STATE_HOVER;
			}
			else
			{
				m_nElemStates[HIT_RIGHTARROW] =
					PART_STATE_NORMAL;
			}

			m_rEngine.GetTimers().Remove(this, TIMER_WAITARROW);

			if (m_pTimerAutoScroll != NULL)
			{
				m_rEngine.GetTimers().Remove(this, TIMER_ARROW);
				m_pTimerAutoScroll = NULL;
			}

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_THUMB:
		{
			if (HitTest(pt) == HIT_THUMB)
			{
				m_nElemStates[HIT_THUMB] =
					PART_STATE_HOVER;
			}
			else
			{
				m_nElemStates[HIT_THUMB] =
					PART_STATE_NORMAL;
			}

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case HIT_SHAFTTOP:
	case HIT_SHAFTBOTTOM:
		{
			if (HitTest(pt) == m_nHitOnMouseDown)
			{
				m_nElemStates[m_nHitOnMouseDown] =
					PART_STATE_HOVER;
			}
			else
			{
				m_nElemStates[m_nHitOnMouseDown] =
					PART_STATE_NORMAL;
			}

			m_rEngine.GetTimers().Remove(this, TIMER_WAITSHAFT);

			if (m_pTimerAutoScroll != NULL)
			{
				m_rEngine.GetTimers().Remove(this, TIMER_SHAFT);
				m_pTimerAutoScroll = NULL;
			}

			// Invalidate if buffered

			Invalidate();
		}
		break;
	}

	// Release mouse capture

	if (m_rEngine.GetScreens().GetCaptureScreen() == this)
		m_rEngine.GetScreens().SetCaptureScreen(NULL);
}

void ScreenScrollBar::OnMouseLeave(void)
{
	// If dragging thumb or scrolling shaft, don't do anything

	if (m_nElemStates[HIT_THUMB] == PART_STATE_PUSHED ||
	   m_nElemStates[HIT_SHAFTTOP] == PART_STATE_PUSHED ||
	   m_nElemStates[HIT_SHAFTBOTTOM] == PART_STATE_PUSHED)
		return;

	// Set all element states to normal

	for(int nHitElem = 0;
		nHitElem < HIT_COUNT;
		nHitElem++)
	{
		m_nElemStates[nHitElem] = PART_STATE_NORMAL;
	}

	// Invalidate if buffered

	Invalidate();
}

void ScreenScrollBar::OnKeyDown(int nKeyCode)
{
	switch (nKeyCode)
	{
	case VK_UP:
	case VK_LEFT:
		{
			SetValue(m_nPos - 1);
		}
		break;
	case VK_DOWN:
	case VK_RIGHT:
		{
			SetValue(m_nPos + 1);
		}
		break;
	default:
		{
			// Not handled, pass it off to parent
			if (m_pParent != NULL)
				m_pParent->OnKeyDown(nKeyCode);
		}
		break;
	}
}

void ScreenScrollBar::OnFocus(Screen* pOldFocus)
{
	m_nElemStates[HIT_UPARROW] = PART_STATE_HOVER;
	m_nElemStates[HIT_DOWNARROW] = PART_STATE_HOVER;
}

void ScreenScrollBar::OnDefocus(Screen* pNewFocus)
{
	m_nElemStates[HIT_UPARROW] = PART_STATE_NORMAL;
	m_nElemStates[HIT_DOWNARROW] = PART_STATE_NORMAL;
}

void ScreenScrollBar::OnTimer(Timer& rTimer)
{
	switch(rTimer.GetID())
	{
	case TIMER_WAITARROW:
		{
			// Waiting for arrow auto-scroll is over, start arrow auto-scroll timer

			m_rEngine.GetTimers().Remove(this, TIMER_WAITARROW);

			if (NULL == m_pTimerAutoScroll)
			{
				m_pTimerAutoScroll =
					m_rEngine.GetTimers().Add(this,
						m_fArrowAutoScrollInterval, TIMER_ARROW);
			}
		}
		break;
	case TIMER_ARROW:
		{
			// Perform auto-scrolling

			if (m_nElemStates[HIT_LEFTARROW] ==
				PART_STATE_PUSHED)
			{
				if (m_nPos > m_nMin)
				{
					m_nPos--;

					UpdateThumbFromPosition();

					if (m_pParent != NULL)
					{
						m_pParent->OnNotify(NOTIFY_SCROLL,
							this, SCROLL_ARROW);
					}
				}
			}
			else if (m_nElemStates[HIT_RIGHTARROW] ==
				PART_STATE_PUSHED)
			{
				if (m_nPos < m_nMax)
				{
					m_nPos++;

					UpdateThumbFromPosition();

					if (m_pParent != NULL)
					{
						m_pParent->OnNotify(NOTIFY_SCROLL,
							this, SCROLL_ARROW);
					}
				}
			}

			// Invalidate if buffered

			Invalidate();
		}
		break;
	case TIMER_WAITSHAFT:
		{
			// Waiting for shaft auto-scroll is over, start shaft auto-scroll timer

			m_rEngine.GetTimers().Remove(this, TIMER_WAITSHAFT);

			if (NULL == m_pTimerAutoScroll)
			{
				m_pTimerAutoScroll =
					m_rEngine.GetTimers().Add(this,
						m_fShaftAutoScrollInterval, TIMER_SHAFT);
			}
		}
		break;
	case TIMER_SHAFT:
		{
			// Perform auto-scrolling

			ScrollShaft();
		}
		break;
	}
}

void ScreenScrollBar::UpdateThumbFromPosition(void)
{
	// Update thumb placement based on value in range

	m_nThumbPos = int(m_fPixelsPerUnit * float(m_nPos - m_nMin));

	if (IsFlagSet(VERTICAL) == true)
	{
		m_rcHitTest[HIT_THUMB].top =
			m_rcHitTest[HIT_UPARROW].bottom + m_nThumbPos;

		m_rcHitTest[HIT_THUMB].bottom =
			m_rcHitTest[HIT_THUMB].top + m_nThumbSize;
	}
	else
	{
		m_rcHitTest[HIT_THUMB].left =
			m_rcHitTest[HIT_LEFTARROW].right + m_nThumbPos;

		m_rcHitTest[HIT_THUMB].right =
			m_rcHitTest[HIT_THUMB].left + m_nThumbSize;
	}

	// Update shaft part sizes

	UpdateShaftSplit();
}

void ScreenScrollBar::UpdateThumbSize(void)
{
	// Calculate thumb size based on items-per-page

	if (m_nShaftSize < m_nMinThumbEdge)
		return;

	if (IsFlagSet(SLIDER) == false)
	{
		m_nThumbSize = int(float(m_nShaftSize) * m_fPage / float(m_nMax - m_nMin + 1));
		
		// Clamp to bounds

		if (m_nThumbSize < m_nMinThumbEdge)
			m_nThumbSize = m_nMinThumbEdge;
		else if (m_nThumbSize > m_nShaftSize)
			m_nThumbSize = m_nShaftSize;
	}
	else
	{
		m_nThumbSize = m_nMinThumbEdge;
	}

	// Calculate pixels per unit, within space actually available for scrolling

	m_fPixelsPerUnit = float(m_nShaftSize - m_nThumbSize) / float(m_nMax - m_nMin);

	// Make sure thumb cannot go beyond the shaft and
	// comes close enough to the end of shaft
	// This counteracts rounding errors

	if (m_nThumbSize > m_nMinThumbEdge)
	{
		int nMaxPos = int(float(m_nMax) * m_fPixelsPerUnit);

		if (IsFlagSet(VERTICAL) == true)
		{		
			if ((m_rcHitTest[HIT_SHAFTTOP].top +
				nMaxPos + m_nThumbSize) >
				m_rcHitTest[HIT_SHAFTBOTTOM].bottom)
			{
				m_nThumbSize--;
			}
			else if ((m_rcHitTest[HIT_SHAFTTOP].top +
				nMaxPos + m_nThumbSize) <
				m_rcHitTest[HIT_SHAFTBOTTOM].bottom)
			{
				m_nThumbSize++;
			}
		}
		else
		{		
			if ((m_rcHitTest[HIT_SHAFTLEFT].left +
				nMaxPos + m_nThumbSize) >
				m_rcHitTest[HIT_SHAFTRIGHT].right)
			{
				m_nThumbSize--;
			}
			else if ((m_rcHitTest[HIT_SHAFTLEFT].left +
				nMaxPos + m_nThumbSize) <
				m_rcHitTest[HIT_SHAFTRIGHT].right)
			{
				m_nThumbSize++;
			}
		}
	}

	// Calculate size of thumb center piece

	m_nThumbCenter = m_nThumbSize - m_nMinThumbEdge;

	Rect rc;

	if (m_nThumbCenter > m_nMinThumbCenter)
	{
		// Size of left/top edge
		//(must be extended to left edge of thumb center piece)

		m_nDistributeEdge = int(float(m_nThumbSize - m_nMinThumbCenter) / 2.0f);

		if (IsFlagSet(HORIZONTAL) == true)
			m_nDistributeEdge -= m_Parts[PART_THUMBLEFT]
				[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetWidth();
		else
			m_nDistributeEdge -= m_Parts[PART_THUMBTOP]
				[m_nElemStates[HIT_THUMB]].GetTextureCoords().GetHeight();

		// Size of thumb center
		// (must be extended from left edge of thumb center to
		// left/top edge of right/bottom piece)

		m_nDistributeCenter =
			m_nThumbSize - m_nDistributeEdge - m_nMinThumbCenter - m_nMinThumbEdge;
	}
	else
	{
		m_nDistributeCenter = m_nThumbSize - m_nMinThumbEdge;
	}

	// Update thumb position, because pixels-per-unit ratio
	// depends on thumb size

	UpdateThumbFromPosition();
}

void ScreenScrollBar::UpdateShaftSize(void)
{
	if (IsFlagSet(VERTICAL) == true)
	{
		m_rcHitTest[HIT_SHAFTTOP].left = 0;

		m_rcHitTest[HIT_SHAFTTOP].top =
			m_rcHitTest[HIT_UPARROW].bottom;

		m_rcHitTest[HIT_SHAFTTOP].right =
			m_rcHitTest[HIT_UPARROW].right;

		m_rcHitTest[HIT_SHAFTBOTTOM].left = 0;

		m_rcHitTest[HIT_SHAFTBOTTOM].right =
			m_rcHitTest[HIT_SHAFTTOP].right;

		m_rcHitTest[HIT_SHAFTBOTTOM].bottom =
			m_rcHitTest[HIT_DOWNARROW].top;

		m_nShaftSize = m_rcHitTest[HIT_DOWNARROW].top -
			m_rcHitTest[HIT_UPARROW].bottom;
	}
	else
	{
		m_rcHitTest[HIT_SHAFTLEFT].left =
			m_rcHitTest[HIT_LEFTARROW].right;

		m_rcHitTest[HIT_SHAFTLEFT].top = 0;

		m_rcHitTest[HIT_SHAFTLEFT].bottom =
			m_Parts[PART_SHAFTLEFT]
			[PART_STATE_NORMAL].GetTextureCoords().GetHeight();

		m_rcHitTest[HIT_SHAFTRIGHT].top = 0;

		m_rcHitTest[HIT_SHAFTRIGHT].right = m_psSize.cx -
			(m_rcHitTest[HIT_RIGHTARROW].right -
			m_rcHitTest[HIT_RIGHTARROW].left);

		m_rcHitTest[HIT_SHAFTRIGHT].bottom =
			m_rcHitTest[HIT_SHAFTLEFT].bottom;

		m_nShaftSize = m_rcHitTest[HIT_SHAFTRIGHT].right -
			m_rcHitTest[HIT_SHAFTLEFT].left;
	}
}

void ScreenScrollBar::UpdateShaftSplit(void)
{
	// Update the two parts of shaft to always split along thumb position

	if (IsFlagSet(VERTICAL) == true)
	{
		// Update hit test rectangles

		m_rcHitTest[HIT_SHAFTTOP].bottom = m_nThumbPos +
			m_rcHitTest[HIT_UPARROW].bottom -
			m_rcHitTest[HIT_UPARROW].top;

		if (IsFlagSet(SLIDER) == false && m_nThumbSize == m_nMinThumbEdge)
			m_rcHitTest[HIT_SHAFTTOP].bottom += m_nThumbSize / 2;

		m_rcHitTest[HIT_SHAFTBOTTOM].top =
			m_rcHitTest[HIT_SHAFTTOP].bottom;

		// Update sizes

		if (IsFlagSet(SLIDER) == false)
		{
			m_nShaftPartsSize[0] =
				m_rcHitTest[HIT_SHAFTTOP].bottom -
				m_rcHitTest[HIT_SHAFTTOP].top +
				m_Parts[PART_SHAFTTOP]
				[m_nElemStates[HIT_SHAFTTOP]].GetTextureCoords().GetHeight() +
				m_nThumbCenter;

			m_nShaftPartsSize[1] =			
				m_rcHitTest[HIT_SHAFTBOTTOM].bottom -
				m_rcHitTest[HIT_SHAFTBOTTOM].top +
				m_Parts[PART_SHAFTBOTTOM]
				[m_nElemStates[HIT_SHAFTBOTTOM]].GetTextureCoords().GetHeight() -
				m_nThumbCenter;
		}
		else
		{
			m_nShaftPartsSize[0] =
				m_rcHitTest[HIT_SHAFTTOP].bottom -
				m_rcHitTest[HIT_SHAFTTOP].top -
				m_Parts[PART_SHAFTTOP]
				[m_nElemStates[HIT_SHAFTTOP]].GetTextureCoords().GetHeight() +
					m_nMinThumbEdge / 2;

			m_nShaftPartsSize[1] =			
				m_rcHitTest[HIT_SHAFTBOTTOM].bottom -
				m_rcHitTest[HIT_SHAFTBOTTOM].top -
				m_Parts[PART_SHAFTBOTTOM]
				[m_nElemStates[HIT_SHAFTBOTTOM]].GetTextureCoords().GetHeight() -
					m_nMinThumbEdge / 2;
		}
	}
	else
	{
		// Update hit test rectangles

		m_rcHitTest[HIT_SHAFTLEFT].right = m_nThumbPos +
			m_rcHitTest[HIT_LEFTARROW].right -
			m_rcHitTest[HIT_LEFTARROW].left;

		if (IsFlagSet(SLIDER) == false && m_nThumbSize == m_nMinThumbEdge)
			m_rcHitTest[HIT_SHAFTLEFT].right += m_nThumbSize / 2;

		m_rcHitTest[HIT_SHAFTRIGHT].left =
			m_rcHitTest[HIT_SHAFTLEFT].right;

		// Update sizes

		if (IsFlagSet(SLIDER) == false)
		{
			m_nShaftPartsSize[0] =
				m_rcHitTest[HIT_SHAFTLEFT].right -
				m_rcHitTest[HIT_SHAFTLEFT].left +
				m_Parts[PART_SHAFTLEFT]
				[m_nElemStates[HIT_SHAFTLEFT]].GetTextureCoords().GetWidth() +
				m_nThumbCenter;

			m_nShaftPartsSize[1] =			
				m_rcHitTest[HIT_SHAFTRIGHT].right -
				m_rcHitTest[HIT_SHAFTRIGHT].left +
				m_Parts[PART_SHAFTRIGHT]
				[m_nElemStates[HIT_SHAFTRIGHT]].GetTextureCoords().GetWidth() -
				m_nThumbCenter;
		}
		else
		{
			m_nShaftPartsSize[0] =
				m_rcHitTest[HIT_SHAFTLEFT].right -
				m_rcHitTest[HIT_SHAFTLEFT].left -
				m_Parts[PART_SHAFTLEFT]
				[m_nElemStates[HIT_SHAFTLEFT]].GetTextureCoords().GetWidth() +
					m_nMinThumbEdge / 2;

			m_nShaftPartsSize[1] =			
				m_rcHitTest[HIT_SHAFTRIGHT].right -
				m_rcHitTest[HIT_SHAFTRIGHT].left -
				m_Parts[PART_SHAFTRIGHT]
				[m_nElemStates[HIT_SHAFTRIGHT]].GetTextureCoords().GetWidth() -
					m_nMinThumbEdge / 2;
		}
	}
}

void ScreenScrollBar::UpdatePositionFromThumb(void)
{
	// Calculate relative position based on
	// relative thumb position within the shaft

	int nNewRelPos = int(float(m_nThumbPos + 1) / m_fPixelsPerUnit);

	// If position is non-zero when it should be zero
	// (due to round-off errors), force it to be zero

	if (IsFlagSet(VERTICAL) == true)
	{
		if (nNewRelPos > 0 &&
		   (m_rcHitTest[HIT_THUMB].top ==
		   m_rcHitTest[HIT_SHAFTTOP].top))
		{
			nNewRelPos = 0;
		}
	}
	else
	{
		if (nNewRelPos > 0 &&
		   (m_rcHitTest[HIT_THUMB].left ==
		   m_rcHitTest[HIT_SHAFTLEFT].left))
		{
			nNewRelPos = 0;
		}
	}

	// Clamp to relative range

	if (nNewRelPos > (m_nMax - m_nMin))
		nNewRelPos = m_nMax - m_nMin;
	else if (nNewRelPos < 0)
		nNewRelPos = 0;

	// Translate into absolute position

	m_nPos = nNewRelPos + m_nMin;

	UpdateShaftSplit();

	// Notify of position change

	if (m_pParent != NULL)
	{
		m_pParent->OnNotify(NOTIFY_SCROLL,
			this, SCROLL_THUMB);
	}
}

void ScreenScrollBar::ScrollShaft(void)
{
	// Scroll page-by-page (or less) until thumb
	// reaches desired position on shaft

	float fDelta = 0.0f;

	if (IsFlagSet(VERTICAL) == true)
	{
		if (m_rcHitTest[HIT_THUMB].top < m_ptDragOffset.y)
		{
			// Desired position is below the thumb, so move thumb down
			
			fDelta = float(m_ptDragOffset.y -
				m_rcHitTest[HIT_THUMB].top) / m_fPixelsPerUnit;

			if (fDelta > m_fPage)
				fDelta = m_fPage;
			else if (fDelta < FLT_EPSILON)
				return;

			m_nPos += int(fDelta);

			if (m_nPos > m_nMax)
				m_nPos = m_nMax;

			UpdateThumbFromPosition();

			if (m_pParent != NULL)
			{
				m_pParent->OnNotify(NOTIFY_SCROLL,
					this, SCROLL_SHAFT);
			}
		}
		else if (m_rcHitTest[HIT_THUMB].top > m_ptDragOffset.y)
		{
			// Desired position is above the thumb, so move thumb up

			fDelta = float(m_rcHitTest[HIT_THUMB].top -
				m_ptDragOffset.y) / m_fPixelsPerUnit;

			if (fDelta > m_fPage)
				fDelta = m_fPage;
			else if (fDelta < FLT_EPSILON)
				return;

			m_nPos -= int(fDelta);

			if (m_nPos < m_nMin)
				m_nPos = m_nMin;

			UpdateThumbFromPosition();

			if (m_pParent != NULL)
			{
				m_pParent->OnNotify(NOTIFY_SCROLL,
					this, SCROLL_SHAFT);
			}
		}
	}
	else
	{
		if (m_rcHitTest[HIT_THUMB].left < m_ptDragOffset.x)
		{
			// Desired position is to the right of thumb, so move thumb right

			fDelta = float(m_ptDragOffset.x -
				m_rcHitTest[HIT_THUMB].left) / m_fPixelsPerUnit;

			if (fDelta > m_fPage)
				fDelta = m_fPage;
			else if (fDelta < FLT_EPSILON)
				return;

			m_nPos += int(fDelta);

			if (m_nPos > m_nMax)
				m_nPos = m_nMax;

			UpdateThumbFromPosition();

			if (m_pParent != NULL)
			{
				m_pParent->OnNotify(NOTIFY_SCROLL,
					this, SCROLL_SHAFT);
			}
		}
		else if (m_rcHitTest[HIT_THUMB].left > m_ptDragOffset.x)
		{
			// Desired position is to the left of thumb, so move thumb left

			fDelta = float(m_rcHitTest[HIT_THUMB].left -
				m_ptDragOffset.x) / m_fPixelsPerUnit;

			if (fDelta > m_fPage)
				fDelta = m_fPage;
			else if (fDelta < FLT_EPSILON)
				return;

			m_nPos -= int(fDelta);

			if (m_nPos < m_nMin)
				m_nPos = m_nMin;

			UpdateThumbFromPosition();

			if (m_pParent != NULL)
			{
				m_pParent->OnNotify(NOTIFY_SCROLL,
					this, SCROLL_SHAFT);
			}
		}
	}

	// Invalidate if buffered

	Invalidate();
}

ScreenScrollBar::PartHitTest ScreenScrollBar::HitTest(POINT pt)
{
	for(int n = 0; n < PART_COUNT; n++)
	{
		if (PtInRect(&m_rcHitTest[n], pt))
			return PartHitTest(n);
	}
	
	return HIT_COUNT;
}