/*------------------------------------------------------------------*\
|
| ScreenCredits.cpp
|
|-------------------------------------------------------------------
|
| Content: HiTMAN: 2D Credits Screen implementation
| Created: 03/02/2011
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
#include "ScreenImage.h"	// using ScreenImage
#include "ScreenCredits.h"	// defining ScreenCredits

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace Hitman2D;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR ScreenCredits::SZ_CREDITSPATH[] =		L"credits.path";
const WCHAR ScreenCredits::SZ_SCROLLINTERVAL[] =	L"credits.scrollinterval";
const WCHAR ScreenCredits::SZ_SCROLLDISTANCE[] =	L"credits.scrolldistance";
const WCHAR ScreenCredits::SZ_CLASS[] =				L"overlapped::credits";


/*----------------------------------------------------------*\
| ScreenCredits implementation
\*----------------------------------------------------------*/

ScreenCredits::ScreenCredits(Engine& rEngine,
							 LPCWSTR pszClass,
							 Screen* pParent):

							 ScreenOverlapped(rEngine, pszClass, pParent),
							 m_nVisibleLines(0),
							 m_nFirstVisibleLine(0),
							 m_nRenderOffset(0),
							 m_nScrollDistance(1),
							 m_fScrollInterval(0.02f),
							 m_pScrollTimer(NULL),
							 m_pTextContainer(NULL)
{
	// Reserve a couple of lines

	m_arLines.reserve(64);
}

ScreenCredits::~ScreenCredits(void)
{
	Empty();
}

Object* ScreenCredits::CreateInstance(Engine& rEngine,
									  LPCWSTR pszClass,
									  Object* pParent)
{
	return new ScreenCredits(rEngine, pszClass, dynamic_cast<Screen*>(pParent));
}

void ScreenCredits::Deserialize(const InfoElem& rRoot)
{
	ScreenOverlapped::Deserialize(rRoot);

	// Read credits path

	const InfoElem* pElem = rRoot.FindChildConst(
		SZ_CREDITSPATH, InfoElem::TYPE_VALUE,
		Variable::TYPE_STRING);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			rRoot.GetDocumentConst().GetPath(), SZ_CREDITSPATH);

	// Load credits text

	Stream stream(&m_rEngine.GetErrors(),
		m_rEngine.GetOption(Engine::OPTION_ENABLE_STREAM_CACHE) ?
		&m_rEngine.GetStreamCache() : NULL);

	try
	{
		stream.Open(pElem->GetStringValue(), GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}

	if (stream.IsUnicodeTextFile() == false)
		throw m_rEngine.GetErrors().Push(Error::FILE_NOTUNICODE,
			__FUNCTIONW__, pElem->GetStringValue());

	// Parse credits text to individual lines

	try
	{
		stream.CreateReadBuffer(stream.GetSize() + 2);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}

	LPCWSTR pszStart =
		reinterpret_cast<LPCWSTR>(stream.GetReadBufferConst());

	LPCWSTR pszEnd = wcschr(pszStart, L'\n');

	for(int nLines = 0; ; nLines++)
	{
		if (NULL == pszEnd)
			pszEnd = pszStart + wcslen(pszStart);

		if (pszEnd[-1] == L'\r') pszEnd--;

		int nLen = int(pszEnd - pszStart);

		LPWSTR pszLine = new WCHAR[nLen + 1];

		if (NULL == pszLine)
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, nLen + 1);

		if (nLen > 0)
			wcsncpy_s(pszLine, nLen + 1, pszStart, nLen);

		pszLine[nLen] = L'\0';

		m_arLines.push_back(pszLine);

		if (L'\0' == *pszEnd)
			break;
		else
			pszStart = pszEnd + 1;

		pszStart++;

		pszEnd = wcschr(pszStart, L'\n');
	}

	// Read scroll interval

	pElem = rRoot.FindChildConst(SZ_SCROLLINTERVAL,
		InfoElem::TYPE_VALUE);

	if (pElem != NULL)
	{
		if (pElem->GetVarType() == Variable::TYPE_INT)
			m_fScrollInterval = float(pElem->GetIntValue());
		else if (pElem->GetVarType() == Variable::TYPE_FLOAT)
			m_fScrollInterval = pElem->GetFloatValue();
	}

	// Read scroll speed

	pElem = rRoot.FindChildConst(SZ_SCROLLDISTANCE,
		InfoElem::TYPE_VALUE, Variable::TYPE_INT);

	if (pElem != NULL)
		m_nScrollDistance = pElem->GetIntValue();

	// Cache a pointer to a child screen on which we will render text

	m_pTextContainer =
		dynamic_cast<ScreenImage*>(m_lstChildren.FindByID(101));

	// Calculate number of lines that can be visible at once

	if (m_pFont != NULL)
	{
		m_nLineHeight = m_pFont->GetLineSpacing();

		if (m_pTextContainer != NULL)
			m_nVisibleLines = m_pTextContainer->GetSize().cy / m_nLineHeight + 1;
	}

	// Start scroll timer

	m_pScrollTimer = m_rEngine.GetTimers().Add(this,
		m_fScrollInterval, TIMER_SCROLL);
}

void ScreenCredits::OnThemeStyleChange(void)
{
	ScreenOverlapped::OnThemeStyleChange();

	// Re-calculate number of lines that can be visible at once

	if (m_pFont != NULL)
	{
		m_nLineHeight = m_pFont->GetLineSpacing();

		if (m_pTextContainer != NULL)
			m_nVisibleLines = m_pTextContainer->GetSize().cy / m_nLineHeight + 1;
	}
}

void ScreenCredits::Empty(void)
{
	ScreenOverlapped::Empty();

	// Release text lines

	for(std::vector<LPWSTR>::iterator pos = m_arLines.begin();
		pos != m_arLines.end();
		pos++)
	{
		delete[] *pos;
	}

	m_arLines.clear();

	// Release timer

	if (m_pScrollTimer != NULL)
	{
		m_rEngine.GetTimers().Remove(this, TIMER_SCROLL);
		m_pScrollTimer = NULL;
	}
}

DWORD ScreenCredits::GetMemoryFootprint(void) const
{
	DWORD dwTextSize = 0;

	for(std::vector<LPWSTR>::const_iterator pos = m_arLines.begin();
		pos != m_arLines.end();
		pos++)
	{
		dwTextSize += DWORD(wcslen(*pos));
	}

	return DWORD(sizeof(ScreenCredits) - sizeof(ScreenOverlapped)) +
			ScreenOverlapped::GetMemoryFootprint() + dwTextSize;
}

void ScreenCredits::OnTimer(Timer& rTimer)
{
	if (rTimer.GetID() == TIMER_SCROLL)
	{
		if (NULL == m_pTextContainer)
			return;

		Graphics& rGraphics = m_rEngine.GetGraphics();

		try
		{
			// Render text

			m_pTextContainer->GetBuffer().BeginScene();

			rGraphics.Clear(m_pTextContainer->GetBackColor());

			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			rGraphics.BeginBatch();

			RECT rcLine = { 0, m_nRenderOffset,
				m_pTextContainer->GetSize().cx,
				m_nRenderOffset + m_nLineHeight };

			for(int nLine = m_nFirstVisibleLine;
				nLine < (m_nFirstVisibleLine + m_nVisibleLines) &&
				nLine != int(m_arLines.size());
				nLine++)
			{
				m_pFont->RenderText(rcLine, m_arLines[nLine],
					-1, D3DCOLOR_ARGB(255, 0, 0, 0));

				rcLine.top += m_nLineHeight;
				rcLine.bottom += m_nLineHeight;
			}

			rGraphics.EndBatch();

			m_pTextContainer->GetBuffer().EndScene();

			rGraphics.GetStates()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

			// Update render offset

			m_nRenderOffset -= m_nScrollDistance;

			if (-m_nRenderOffset >= m_nLineHeight)
			{
				m_nRenderOffset = 0;

				// Update first visible line

				m_nFirstVisibleLine++;

				if (m_nFirstVisibleLine >= int(m_arLines.size()))
					m_nFirstVisibleLine = 0;
			}
		}

		catch(Error& rError)
		{
			UNREFERENCED_PARAMETER(rError);

			throw m_rEngine.GetErrors().Push(Error::INTERNAL, __FUNCTIONW__);
		}
	}
	else
	{
		ScreenOverlapped::OnTimer(rTimer);
	}	
}