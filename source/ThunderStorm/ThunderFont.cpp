/*------------------------------------------------------------------*\
|
| ThunderFont.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine font class implementation
| Created: 11/19/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderFont.h"		// defining Font
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoFile/Elem

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Font::SZ_ROOT[]				= L"font";
const WCHAR Font::SZ_GENERATE[]			= L"generate";
const WCHAR Font::SZ_MATERIAL[]			= L"material";
const WCHAR Font::SZ_MAX_CHAR_WIDTH[]	= L"max-char-width";
const WCHAR Font::SZ_AVE_CHAR_WIDTH[]	= L"ave-char-width";
const WCHAR Font::SZ_CHAR_HEIGHT[]		= L"char-height";
const WCHAR Font::SZ_ASCENT[]			= L"ascent";
const WCHAR Font::SZ_LINE_SPACING[]		= L"line-spacing";
const WCHAR Font::SZ_GLYPH[]			= L"glyph";
const WCHAR Font::SZ_KERNINGPAIR[]		= L"kerning";
const WCHAR FontGlyph::SZ_ABC[]			= L"spacing";
const WCHAR FontGlyph::SZ_ORIGIN[]		= L"origin";
const MAT2 Font::MTX2_IDENTITY			= { 0, 1,  0, 0,  0, 0,  0, 1 };


/*----------------------------------------------------------*\
| Font implementation
\*----------------------------------------------------------*/

Font::Font(Engine& rEngine): Resource(rEngine),
							 m_nMaxCharWidth(0),
							 m_nAveCharWidth(0),
							 m_nCharHeight(0),
							 m_nAscent(0),
							 m_nLineSpacing(0),
							 m_wDefaultChar(0)
{
}

Font::~Font(void)
{
	Empty();
}

FontGlyph* Font::GetGlyph(WCHAR ch)
{
	FontGlyphMapIterator pos = m_mapGlyphs.find(ch);

	if (pos != m_mapGlyphs.end())
		return &pos->second;

	pos = m_mapGlyphs.find(m_wDefaultChar);

	if (pos != m_mapGlyphs.end())
		return &pos->second;

	return NULL;
}

const FontGlyph* Font::GetGlyphConst(WCHAR ch) const
{
	FontGlyphMapConstIterator pos = m_mapGlyphs.find(ch);

	if (pos != m_mapGlyphs.end())
		return &pos->second;

	pos = m_mapGlyphs.find(m_wDefaultChar);

	if (pos != m_mapGlyphs.end())
		return &pos->second;

	return NULL;
}

FontGlyph& Font::InsertGlyph(WCHAR ch, const FontGlyph& rGlyph)
{
	FontGlyphMapIterator pos = m_mapGlyphs.find(ch);

	if (pos != m_mapGlyphs.end())
	{
		pos->second = rGlyph;

		return pos->second;
	}
	else
	{
		m_mapGlyphs[ch] = rGlyph;

		return m_mapGlyphs[ch];
	}
}

void Font::RemoveGlyph(WCHAR ch)
{
	FontGlyphMapIterator pos = m_mapGlyphs.find(ch);

	if (pos != m_mapGlyphs.end())
		m_mapGlyphs.erase(pos);
}

int Font::GetKerningPair(WCHAR ch1, WCHAR ch2) const
{
	KerningMapConstIterator pos =
		m_mapKerning.find(KerningKey(ch1, ch2));

	if (pos != m_mapKerning.end())
		return pos->second;

	return 0;
}

void Font::RemoveKerningPair(WCHAR ch1, WCHAR ch2)
{
	KerningMapIterator pos =
		m_mapKerning.find(KerningKey(ch1, ch2));

	if (pos != m_mapKerning.end())
		m_mapKerning.erase(pos);
}

void Font::RenderGlyph(const Vector2& rvecPos, WCHAR ch, D3DCOLOR clrBlend)
{
	const FontGlyph* pGlyph = GetGlyphConst(ch);

	if (pGlyph != NULL)
		m_rEngine.GetGraphics().RenderQuad(GetGlyph(ch)->GetMaterialInstance(),
			rvecPos, clrBlend);
}

SIZE Font::GetTextExtent(LPCWSTR psz, int nCount, DWORD dwFlags, int nMaxWidth, int* pnLineCount) const
{
	SIZE size = {0, 0};

	// Validate

	if (String::IsEmpty(psz))
		return size;

	if (-1 == nCount)
		nCount = int(wcslen(psz));
	else if (0 == nCount)
		return size;

	LPCWSTR pszEof = psz + nCount;

	// Get tab width and space width

	const FontGlyph* pGlyphSpace = GetGlyphConst(L' ');

	int nTabWidth = m_nMaxCharWidth * 2;
	int nSpaceWidth = pGlyphSpace != NULL ?
		pGlyphSpace->GetTotalSpacing() : 0;

	// Start parsing & rendering

	LPCWSTR pszBeginLine = psz, pszEndLine = NULL;
	int nSpacing = 0, nLastSpacing = 0, nLinesSkipped = 0, nLineCount = 0;

	for(LPCWSTR pszStart = psz; pszStart < pszEof; )
	{
		switch(*pszStart)
		{
		case L'\r':
		case L'\n':

			// Start a new line if found line break character.

			if (pszStart > psz && L'\n' == pszStart[-1])
				nLinesSkipped++;
			else
				nLastSpacing = nSpacing;

			nSpacing = 0;

			pszEndLine = pszStart;

			if (L'\r' == *pszStart)
				pszStart += 2;
			else
				pszStart++;

			continue;

		case L'\t':

			// Advance by a tab stop if found tab character

			nSpacing = (nSpacing / nTabWidth + 1) * nTabWidth;

			pszStart++;

			continue;
		case L' ':

			nSpacing += nSpaceWidth;

			pszStart++;

			continue;
		}

		// Find the end of current word in text and its extent

		int nWordSpacing = 0;
		LPCWSTR pszEnd = pszStart;

		while(IsWhiteSpace(*pszEnd) == FALSE && *pszEnd != L'\0')
		{
			// Don't include mnemonics in spacing calculations

			if (dwFlags & USE_MNEMONICS && L'&' == *pszEnd)
			{
				if (L'&' != pszEnd[-1])
				{
					pszEnd++;
					continue;
				}
			}

			const FontGlyph* pGlyph = GetGlyphConst(*pszEnd);

			if (pGlyph != NULL)
				nWordSpacing += pGlyph->GetTotalSpacing();

			pszEnd++;
		}

		// If doesn't fit in rect, wrap to next line

		if ((nSpacing + nWordSpacing) >= nMaxWidth)
		{
			nLastSpacing = nSpacing;
			nSpacing = 0;

			pszEndLine = pszStart;
		}

		nSpacing += nWordSpacing;

		// If at the end of string, process last part

		if (pszEnd == pszEof)
		{
			pszEndLine = pszEnd;

			nLastSpacing = nSpacing;
		}

		// Finish processing line

		if (pszEndLine != NULL)
		{
			if (L' ' == pszEndLine[-1])
			{
				// If line ended with spaces, exclude them from
				// alignment calculations

				for(LPCWSTR pszTemp = pszEndLine - 2;
					pszTemp > psz && L' ' == *pszTemp;
					pszTemp--, nLastSpacing -= nSpaceWidth);

				nLastSpacing -= nSpaceWidth;
			}

			size.cy += (m_nLineSpacing * (nLinesSkipped + 1));
			size.cx += nLastSpacing;

			nLineCount += nLinesSkipped + 1;

			nLinesSkipped = 0;

			pszEndLine = NULL;
			pszBeginLine = pszStart;
		}

		pszStart = pszEnd;
	}

	// Return line count if requested

	if (pnLineCount != NULL)
		*pnLineCount = nLineCount;

	return size;
}

void Font::RenderText(const Rect& rrcText,
					  LPCWSTR psz,
					  int nCount,
					  D3DCOLOR clrBlend,
					  DWORD dwFlags,
					  TextBlock* pCache)
{
	// Validate

	if (String::IsEmpty(psz))
		return;	

	if (-1 == nCount)
		nCount = int(wcslen(psz));
	else if (0 == nCount)
		return;

	LPCWSTR pszEof = psz + nCount;

	// If caching, pre-cache

	if (pCache != NULL)
		pCache->PreCache(nCount);

	// Set starting position

	POINT ptRenderPos = { rrcText.left, rrcText.top };

	// Get tab width and space width

	const FontGlyph* pGlyphSpace = GetGlyphConst(L' ');

	int nTabWidth = m_nMaxCharWidth * 2;
	int nSpaceWidth = pGlyphSpace != NULL ?
		pGlyphSpace->GetTotalSpacing() : m_nAveCharWidth;

	// Get underline character to use for mnemonic

	const FontGlyph* pUnderline = (dwFlags & USE_MNEMONICS) ?
		GetGlyphConst(L'_') : NULL;

	// Calculate color for the mnemonic

	Color clrMnemonic = clrBlend;
	clrMnemonic.SetAlpha(clrMnemonic.GetAFloat() * 0.6f);

	// Calculate vertical offset from vertical alignment

	if (dwFlags & ALIGN_VCENTER)
		ptRenderPos.y += (rrcText.GetHeight() -
			GetTextExtent(psz, nCount, dwFlags, rrcText.GetWidth()).cy) / 2;

	// Start parsing & rendering

	LPCWSTR pszBeginLine = psz, pszEndLine = NULL;
	int nSpacing = 0, nLastSpacing = 0, nLinesSkipped = 0;

	for(LPCWSTR pszStart = psz; pszStart < pszEof; )
	{
		switch(*pszStart)
		{
		case L'\r':
		case L'\n':
			{
				// Start a new line if found line break character.
				// Because we only render a line of text after we find the
				// start of the next line, contiguous line breaks will not be
				// processed unless we keep track explicitly (nLinesSkipped++)

				if (pszStart > psz && L'\n' == pszStart[-1])
					nLinesSkipped++;
				else
					nLastSpacing = nSpacing;

				nSpacing = 0;

				pszEndLine = pszStart;

				if (L'\r' == *pszStart)
					pszStart += 2;
				else
					pszStart++;

				if (pszStart != pszEof)
					continue;
			}
			break;
		case L'\t':
			{
				// Advance by a tab stop if found tab character

				nSpacing = (nSpacing / nTabWidth + 1) * nTabWidth;

				if ((pszStart + 1) != pszEof)
				{
					pszStart++;
					continue;
				}
			}
			break;
		case L' ':
			{
				nSpacing += nSpaceWidth;

				pszStart++;

				if (pszStart != pszEof)
					continue;
			}
			break;
		}

		// Find the end of current word in text and its extent

		int nWordSpacing = 0;
		LPCWSTR pszEnd = pszStart;

		while(IsWhiteSpace(*pszEnd) == FALSE && *pszEnd != L'\0')
		{
			// Don't include mnemonics in spacing calculations

			if (dwFlags & USE_MNEMONICS && L'&' == *pszEnd)
			{
				if (L'&' != pszEnd[-1])
				{
					pszEnd++;
					continue;
				}
			}

			const FontGlyph* pGlyph = GetGlyphConst(*pszEnd);

			if (pGlyph != NULL)
				nWordSpacing += pGlyph->GetTotalSpacing();

			pszEnd++;
		}

		// If doesn't fit in rect, wrap to next line

		if ((nSpacing + nWordSpacing) >= rrcText.GetWidth())
		{
			nLastSpacing = nSpacing;
			nSpacing = 0;

			pszEndLine = pszStart;
		}

		nSpacing += nWordSpacing;

		// If at the end of string, render last part

		if (pszEnd == pszEof)
		{
			pszEndLine = pszEnd;

			nLastSpacing = nSpacing;
		}

		// Render glyphs for the characters of this word when line is processed

		if (pszEndLine != NULL)
		{
			while((L'\n' == pszEndLine[-1] || L' ' == pszEndLine[-1]) &&
				  pszEndLine > psz)
			{
				if (L' ' == pszEndLine[-1])
					nLastSpacing -= nSpaceWidth;

				pszEndLine--;
			}

			// Calculate horizontal offset from horizontal alignment

			int nOffset = 0;

			if (dwFlags & ALIGN_RIGHT)
				nOffset = rrcText.GetWidth() - nLastSpacing;
			else if (dwFlags & ALIGN_CENTER)
				nOffset = (rrcText.GetWidth() - nLastSpacing) / 2;

			bool bMnemonic = false;

			for(LPCWSTR pszRender = pszBeginLine;
				pszRender < pszEndLine;
				pszRender++)
			{
				// Watch for space and tab

				if (L' ' == *pszRender)
				{
					ptRenderPos.x += nSpaceWidth;
				}
				else if (L'\t' == *pszRender)
				{
					ptRenderPos.x = rrcText.left +
						(((ptRenderPos.x - rrcText.left) / nTabWidth + 1) *
						nTabWidth);
				}
				else if ((dwFlags & USE_MNEMONICS) &&
						L'&' == *pszRender && L'&' != pszRender[-1])
				{
					if (L'&' != pszRender[1])
						bMnemonic = true;
				}
				else
				{
					// Get glyph info

					const FontGlyph* pGlyph = GetGlyphConst(*pszRender);
					if (NULL == pGlyph) continue;
					
					// Add kerning

					if (pszRender > psz)
						ptRenderPos.x += GetKerningPair(pszRender[-1], *pszRender);

					// Add pre-render spacing

					ptRenderPos.x += pGlyph->GetSpacingConst().abcA;

					if (pCache != NULL)
					{
						// Cache glyph

						pCache->PreCacheGlyph(
							pGlyph->GetMaterialInstanceConst(),
							ptRenderPos.x + nOffset,
							ptRenderPos.y + m_nAscent - pGlyph->GetOrigin().y,
							-1, -1,
							clrBlend);

						// If marked as mnemonic, cache underline

						if (true == bMnemonic)
						{
							pCache->PreCacheGlyph(
								pUnderline->GetMaterialInstanceConst(),

								ptRenderPos.x + nOffset,
								ptRenderPos.y + m_nAscent + 2,

								pGlyph->GetMaterialInstanceConst().GetTextureCoords().GetWidth(),								
								1,

								clrMnemonic);
						}

					}
					else
					{
						// Render glyph

						m_rEngine.GetGraphics().RenderQuad(
							pGlyph->GetMaterialInstanceConst(),
							Vector2(ptRenderPos.x + nOffset,
									ptRenderPos.y + m_nAscent -
										pGlyph->GetOrigin().y),
							clrBlend);

						// If marked as mnemonic, render underline

						if (true == bMnemonic)
						{
							m_rEngine.GetGraphics().RenderQuad(
								pUnderline->GetMaterialInstanceConst(),

								Vector2(ptRenderPos.x + nOffset,
										ptRenderPos.y + m_nAscent + 2),

								Vector2(pGlyph->GetMaterialInstanceConst().
									GetTextureCoords().GetWidth(), 1),

								clrMnemonic);
						}
					}

					// Add post-render spacing

					ptRenderPos.x += pGlyph->GetSpacingConst().abcB +
						pGlyph->GetSpacingConst().abcC;

					bMnemonic = false;
				}
			}

			ptRenderPos.y += (m_nLineSpacing * (nLinesSkipped + 1));
			ptRenderPos.x = rrcText.left;

			nLinesSkipped = 0;

			pszEndLine = NULL;
			pszBeginLine = pszStart;
		}

		pszStart = pszEnd;
	}
}

void Font::Generate(LPCWSTR pszFaceName,
					int nSize,
					bool bBold,
					bool bItalic,
					LPCWSTR pszCharRangePairs,
					float fSpacingScale,
					Material* pReferenceMaterial)
{
	// Validate

	if (NULL == pReferenceMaterial)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 6);

	// Get game window device context

	HWND hWnd = m_rEngine.GetGameWindow();
	HDC hDC = GetDC(hWnd);

	// Create a memory DC compatible with it

	HDC hFontDC = CreateCompatibleDC(hDC);

	ReleaseDC(hWnd, hDC);

	if (NULL == hFontDC)
	{
		throw m_rEngine.GetErrors().Push(Error::WIN_GDI_CREATECOMPATIBLEDC,
			__FUNCTIONW__);
	}

	// Create a GDI object instance of specified font, set as current

	HFONT hFont = NULL;
	LPCWSTR pszNameSep = wcschr(pszFaceName, L'#');

	if (pszNameSep != NULL)
	{
		// Create by face name

		int nLen = pszNameSep - pszFaceName + 1;

		String strFaceName;
		strFaceName.Allocate(nLen);
		strFaceName.CopyToBuffer(nLen, pszFaceName, nLen - 1);
		
		// If doesn't exist, install

		if (FontExists(strFaceName) == false)
		{
			WCHAR szFontPath[MAX_PATH] = {0};

			m_rEngine.GetBaseFilePath(pszNameSep + 1,
				m_rEngine.GetFonts().GetBasePath(), L".ttf", szFontPath);

			if (GetFileAttributes(szFontPath) == INVALID_VALUE)
				return;

			if (AddFontResourceEx(szFontPath, FR_PRIVATE, 0) == FALSE)
				return;
			
			hFont = CreateFont(-MulDiv(nSize, GetDeviceCaps(hFontDC, LOGPIXELSY), 72),
				0, 0, 0, true == bBold ? FW_BOLD : FW_NORMAL,
				true == bItalic ? 1 : 0, 0, 0, DEFAULT_CHARSET,
				OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
				DEFAULT_PITCH | FF_SWISS, strFaceName);
		}
		else
		{
			hFont = CreateFont(-MulDiv(nSize, GetDeviceCaps(hFontDC, LOGPIXELSY), 72),
			0, 0, 0, true == bBold ? FW_BOLD : FW_NORMAL,
			true == bItalic ? 1 : 0, 0, 0, DEFAULT_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
			DEFAULT_PITCH | FF_SWISS, strFaceName);
		}
	}
	else
	{
		// Create the font or one closest to it

		hFont = CreateFont(-MulDiv(nSize, GetDeviceCaps(hFontDC, LOGPIXELSY), 72),
			0, 0, 0, true == bBold ? FW_BOLD : FW_NORMAL,
			true == bItalic ? 1 : 0, 0, 0, DEFAULT_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
			DEFAULT_PITCH | FF_SWISS, pszFaceName);
	}

	if (NULL == hFont)
	{
		DeleteDC(hFontDC);

		throw m_rEngine.GetErrors().Push(Error::WIN_GDI_CREATEFONT,
			__FUNCTIONW__);
	}

	HGDIOBJ hOldFont = SelectObject(hFontDC, hFont);

	// Get font metrics

	TEXTMETRIC tm;
	GetTextMetrics(hFontDC, &tm);

	m_nAveCharWidth = tm.tmAveCharWidth;
	m_nMaxCharWidth = tm.tmMaxCharWidth;
	m_nCharHeight = tm.tmHeight;
	m_nAscent = tm.tmAscent;
	m_nLineSpacing = tm.tmHeight + tm.tmExternalLeading;
	m_wDefaultChar = tm.tmDefaultChar;

	try
	{
		GetKerningInfo(hFontDC, fSpacingScale);
	}

	catch(std::exception& rError)
	{
		DeleteObject(SelectObject(hFontDC, hFont));
		DeleteDC(hFontDC);

		throw rError;
	}

	// Allocate glyphs from range pairs

	WCHAR szRange[512] = {0};
	LPCWSTR pszEndRange = NULL;

	if (NULL == pszCharRangePairs || L'\0' == *pszCharRangePairs)
	{
		// If range is left blank, figure out which
		// ranges to render by glyphs that have been defined for the font.

		try
		{
			GetValidGlyphRanges(hFontDC, tm, szRange, (LPWSTR*)&pszEndRange,
				sizeof(szRange) / sizeof(WCHAR) / 2);
		}

		catch(std::exception& rError)
		{
			DeleteObject(SelectObject(hFontDC, hFont));
			DeleteDC(hFontDC);

			throw rError;
		}

		pszCharRangePairs = szRange;
	}
	else
	{
		pszEndRange = pszCharRangePairs + 2;

		if (L'\0' != *pszEndRange)
		{
			while(pszEndRange[0] != L'\0' && pszEndRange[1] != L'\0')
				pszEndRange += 2;
		}
	}

	int nEstimatedArea = 0;
	SIZE glyphSize = { 0, 0 };
	GLYPHMETRICS gm;

	for(LPCWSTR pszCur = pszCharRangePairs,
		pszLast = pszEndRange;
		pszCur != pszLast;
		pszCur += 2)
	{
		WCHAR ch = pszCur[0];
		WCHAR chEnd = pszCur[1];

		if (chEnd < ch)
		{
			if (L'\0' == chEnd)
				chEnd = ch;
			else
				break;
		}

		LPABC pAbcWidths = NULL;
		int* pWidths = NULL;

		if (tm.tmPitchAndFamily & TMPF_TRUETYPE)
		{
			// Get abc spacing if true-type font

			try
			{
				pAbcWidths = new ABC[chEnd - ch + 1];
			}

			catch(std::bad_alloc)
			{
				DeleteObject(SelectObject(hFontDC, hOldFont));
				DeleteDC(hFontDC);

				throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
					__FUNCTIONW__, sizeof(ABC) * (chEnd - ch + 1));
			}

			if (GetCharABCWidths(hFontDC, ch, chEnd, pAbcWidths) == FALSE)
			{
				delete[] pAbcWidths;

				DeleteObject(SelectObject(hFontDC, hOldFont));
				DeleteDC(hFontDC);

				throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETCHARABCWIDTHS,
					__FUNCTIONW__);
			}			
		}
		else
		{
			// Get simple spacing if raster font

			try
			{
				pWidths = new int[chEnd - ch + 1];
			}

			catch(std::bad_alloc)
			{
				throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
					__FUNCTIONW__, sizeof(int) * (chEnd - ch + 1));
			}

			if (GetCharWidth32(hFontDC, ch, chEnd, pWidths) == FALSE)
			{
				delete[] pWidths;

				DeleteObject(SelectObject(hFontDC, hOldFont));
				DeleteDC(hFontDC);

				throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETCHARWIDTH32,
					__FUNCTIONW__);
			}
		}

		for(int n = 0;
			ch <= chEnd;
			ch++, n++)
		{			
			// Copy spacing info and insert glyph into map

			FontGlyph glyph;

			if (pAbcWidths != NULL)
			{
				// Copy TrueType ABC spacing

				pAbcWidths[n].abcA = int(float(pAbcWidths[n].abcA) * fSpacingScale);
				pAbcWidths[n].abcB = UINT(float(pAbcWidths[n].abcB) * fSpacingScale);
				pAbcWidths[n].abcC = int(float(pAbcWidths[n].abcC) * fSpacingScale);

				glyph.SetSpacing(pAbcWidths[n]);

				// Set glyph size to be used when determining wrapping

				if (GetGlyphOutline(hFontDC, ch, GGO_METRICS, &gm,
					0, NULL, &MTX2_IDENTITY) == GDI_ERROR)
					throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETGLYPHOUTLINE,
						__FUNCTIONW__, GDI_ERROR);

				glyph.GetMaterialInstance().SetTextureCoords(0, 0,
					int(gm.gmBlackBoxX), int(gm.gmBlackBoxY));

				nEstimatedArea += int(gm.gmBlackBoxX * tm.tmHeight);
			}
			else
			{
				// Copy raster font spacing

				glyph.SetSpacing(0, UINT(float(pWidths[n]) * fSpacingScale), 0);

				// Set glyph size to be used when determining wrapping

				if (GetTextExtentPoint32(hFontDC, &ch, 1, &glyphSize) != TRUE)
					throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETTEXTEXTENTPOINT32,
						__FUNCTIONW__);

				glyph.GetMaterialInstance().SetTextureCoords(0, 0,
					glyphSize.cx, glyphSize.cy);

				nEstimatedArea += glyphSize.cx * tm.tmHeight;
			}

			m_mapGlyphs[ch] = glyph;
		}

		if (pAbcWidths != NULL)
			delete[] pAbcWidths;
		else
			delete[] pWidths;
	}

	// If raster font, prepare a temp bitmap for retrieving glyphs.

	D3DFORMAT nTexFormat =
		D3DFMT_X8R8G8B8 == m_rEngine.GetGraphics().GetDeviceParams().BackBufferFormat ?
		D3DFMT_A8R8G8B8 : D3DFMT_A4R4G4B4;

	HBITMAP hbmTempGlyph = NULL;
	HGDIOBJ hbmOldTempGlyph = NULL;
	LPBYTE pbTempGlyph = NULL;
	BITMAPINFO bmiTempGlyph = {0};
	UINT uTempGlyphPitch = 0;

	if (~tm.tmPitchAndFamily & TMPF_TRUETYPE)
	{
		bmiTempGlyph.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiTempGlyph.bmiHeader.biCompression = BI_RGB;

		UINT uPixelSize = 0;
		
		if (D3DFMT_A8R8G8B8 == nTexFormat)
		{
			bmiTempGlyph.bmiHeader.biBitCount = 32;
			uPixelSize = sizeof(DWORD);
		}
		else
		{
			bmiTempGlyph.bmiHeader.biBitCount = 16;
			uPixelSize = sizeof(WORD);
		}

		bmiTempGlyph.bmiHeader.biPlanes = 1;
		bmiTempGlyph.bmiHeader.biWidth = tm.tmMaxCharWidth;
		bmiTempGlyph.bmiHeader.biHeight = -tm.tmHeight;

		hbmTempGlyph = CreateDIBSection(hFontDC,
			&bmiTempGlyph, DIB_RGB_COLORS, (void**)&pbTempGlyph,
			NULL, 0);

		if (NULL == hbmTempGlyph)
			throw m_rEngine.GetErrors().Push(Error::WIN_GDI_CREATEDIBSECTION,
				__FUNCTIONW__);

		UINT uScanSize = tm.tmMaxCharWidth * uPixelSize;

		uTempGlyphPitch = ((uScanSize >> 2) + (uScanSize & 1)) << 2;

		bmiTempGlyph.bmiHeader.biSizeImage = uTempGlyphPitch * tm.tmHeight;

		hbmOldTempGlyph = SelectObject(hFontDC, hbmTempGlyph);
	}
	
	// Process glyphs - retrieve images and more metrics

	int nGlyphX = 0, nGlyphY = 0, nGlyphLineMax = 0;
	int nGlyphWidth = 0, nGlyphHeight = 0;
	int nGlyphsProcessed = 0;
	int nGlyphTexWidth = 0, nGlyphTexHeight = 0;

	Texture* pCurGlyphTex = NULL;
	Material* pCurGlyphMat = NULL;
	int nGlyphTexID = 0;

	D3DLOCKED_RECT lr;

	for(FontGlyphMapIterator pos = m_mapGlyphs.begin();
		pos != m_mapGlyphs.end();
		pos++, nGlyphsProcessed++)
	{
		// Check if allocation of new font texture page is required

		if (NULL == pCurGlyphTex)
		{
			// Allocate new glyph texture, max size 512x512

			nGlyphTexHeight = 16;

			nGlyphTexID++;

			String strBaseName = PathFindFileName(m_strName);

			PathRemoveExtension(strBaseName.GetBuffer());

			String strTexName, strMatName;

			if (nGlyphTexID > 1)
			{
				strTexName.Format(L"%s-font%d.png", strBaseName, nGlyphTexID);
				strMatName.Format(L"%s-font%d.thl", strBaseName, nGlyphTexID);
			}
			else
			{
				strTexName.Format(L"%s-font.png", strBaseName);
				strMatName.Format(L"%s-font.thl", strBaseName);
			}

			for(; nGlyphTexHeight < 512;
				nGlyphTexHeight <<= 1)
			{
				for(nGlyphTexWidth = 16;
					nGlyphTexWidth < 512;
					nGlyphTexWidth <<= 1)
				{
					if (256 == nGlyphTexWidth && nGlyphTexHeight < 256)
						break;

					if ((nGlyphTexWidth * nGlyphTexHeight) >= nEstimatedArea)
						break;		
				}

				if ((nGlyphTexWidth * nGlyphTexHeight) >= nEstimatedArea)
					break;
			}

			pCurGlyphTex = m_rEngine.GetTextures().Create();
			pCurGlyphTex->SetName(strTexName);

			// Indicate to the engine this is a dynamic resource that will not be reloaded

			pCurGlyphTex->SetFlag(Texture::NORELOAD);
			pCurGlyphTex->SetPersistenceTime(0.0f);

			// Add to font textures and register with resource manager

			m_rEngine.GetTextures().Add(pCurGlyphTex);
			m_arGenTextures.push_back(pCurGlyphTex);
			pCurGlyphTex->AddRef();

			// Allocate the new font texture dynamically

			pCurGlyphTex->Allocate(nGlyphTexWidth, nGlyphTexHeight, nTexFormat);

			// Create glyph material based on reference material

			pCurGlyphMat = m_rEngine.GetMaterials().Create();
			pCurGlyphMat->SetName(strMatName);

			// Indicate to the engine this is a dynamic resource that will not be reloaded
			//pCurGlyphMat->SetPersistenceTime(0.0f);

			// Add to font materials and register with resource manager
			strMatName.Empty();
			*pCurGlyphMat = *pReferenceMaterial;
			m_rEngine.GetMaterials().Add(pCurGlyphMat);
			m_arGenMaterials.push_back(pCurGlyphMat);
			pCurGlyphMat->AddRef();

			// Change new material's base texture to glyph texture

			if (pCurGlyphMat->GetBaseParameter() == NULL)
			{
				// Add base texture parameter if it has never been set

				if (pCurGlyphMat->GetEffect() == NULL)
					throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
						__FUNCTIONW__, L"pGlyphMaterial->GetEffect()");

				EffectParameterInfo* pBaseParamInfo =
					pCurGlyphMat->GetEffect()->GetBaseParamInfo();

				if (NULL == pBaseParamInfo)
					throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
						__FUNCTIONW__, L"pBaseParamInfo");

				EffectParameter paramBase(pBaseParamInfo);
				paramBase.SetTexture(pCurGlyphTex);

				pCurGlyphMat->SetParameter(paramBase);
			}
			else
			{
				pCurGlyphMat->GetBaseParameter()->SetTexture(pCurGlyphTex);
			}

			//m_rEngine.PrintDebug(L"font set mat texture %x (%s), param %x", pCurGlyphTex, pCurGlyphTex->GetName(), pCurGlyphMat->GetBaseParameter());

			pCurGlyphTex->Lock(NULL, &lr, D3DLOCK_NO_DIRTY_UPDATE);			
		}

		// Set glyph image and metrics

		if (tm.tmPitchAndFamily & TMPF_TRUETYPE)
		{
			GetGlyphImageTrueType(hFontDC, pos->first, lr, nTexFormat,
				nGlyphX, nGlyphY, gm);

			pos->second.SetOrigin(gm.gmptGlyphOrigin.x,
				gm.gmptGlyphOrigin.y);

			nGlyphWidth = int(gm.gmBlackBoxX);
			nGlyphHeight = int(gm.gmBlackBoxY);
		}
		else
		{
			glyphSize =
				pos->second.GetMaterialInstanceConst().GetTextureCoords().GetSize();

			GetGlyphImageRaster(hFontDC, pos->first,
				pbTempGlyph, lr, nTexFormat, nGlyphX, nGlyphY,
				tm.tmHeight, uTempGlyphPitch, glyphSize);

			pos->second.SetOrigin(0, m_nAscent - tm.tmOverhang);

			nGlyphWidth = glyphSize.cx;
			nGlyphHeight = glyphSize.cy;
		}
		
		// Set glyph's material instance

		MaterialInstance& rInst = pos->second.GetMaterialInstance();

		rInst.SetMaterial(pCurGlyphMat);
		rInst.SetTextureCoords(nGlyphX, nGlyphY,
			nGlyphWidth, nGlyphHeight);

		nGlyphX += nGlyphWidth;

		nEstimatedArea -= nGlyphWidth * nGlyphHeight;

		if (nEstimatedArea < 0)
			nEstimatedArea = 0;

		// Check if we need to wrap to another row in font texture

		FontGlyphMapIterator posNext = pos;
		SIZE sizeNextGlyph = { 0, 0 };
		
		if (m_mapGlyphs.end() != ++posNext)
		{
			sizeNextGlyph =
				posNext->second.GetMaterialInstanceConst().
				GetTextureCoords().GetSize();
		}

		if ((nGlyphX + sizeNextGlyph.cx) >= nGlyphTexWidth)
		{
			nGlyphY += nGlyphLineMax;

			nGlyphX = 0;

			nGlyphLineMax = 0;
		}
		
		if (nGlyphLineMax < nGlyphHeight)
			nGlyphLineMax = nGlyphHeight;

		// Continue to make another glyph texture if out of space on this one

		if (nGlyphY + sizeNextGlyph.cy >= nGlyphTexHeight)
		{
			nGlyphY = 0;
			nGlyphX = 0;
			nGlyphLineMax = 0;

			pCurGlyphTex->Unlock();

			pCurGlyphTex = NULL;
			pCurGlyphMat = NULL;
		}
	}

	if (pCurGlyphTex != NULL)
		pCurGlyphTex->Unlock();

	// Clean up

	DeleteObject(SelectObject(hFontDC, hOldFont));
	DeleteObject(SelectObject(hFontDC, hbmOldTempGlyph));
	DeleteDC(hFontDC);
}

int CALLBACK Font::FontEnumProc(const LOGFONT *lpelfe,
								const TEXTMETRIC *lpntme,
								DWORD FontType,
								LPARAM lParam)
{
	(*((int*)lParam))++;

	return 1;
}

bool Font::FontExists(LPCWSTR pszFaceName)
{
	HWND hWndDesktop = GetDesktopWindow();
	HDC hDisplayDC = GetDC(hWndDesktop);

	LOGFONT lf = {0};
	wcscpy_s(lf.lfFaceName, LF_FACESIZE, pszFaceName);
	lf.lfCharSet = ANSI_CHARSET;

	int nCounter = 0;	

	EnumFontFamiliesEx(hDisplayDC, &lf, FontEnumProc, (LONG)&nCounter, 0);

	ReleaseDC(hWndDesktop, hDisplayDC);

	return (nCounter > 0);
}

void Font::GetKerningInfo(HDC hDC, float fSpacingScale)
{
	DWORD dwKernPairCount = GetKerningPairs(hDC, 0, NULL);

	if (0 == dwKernPairCount)
		return;

	LPKERNINGPAIR pKernPairs = NULL;

	try
	{
		pKernPairs = new KERNINGPAIR[dwKernPairCount];
	}
	
	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(KERNINGPAIR));
	}

	if (GetKerningPairs(hDC, dwKernPairCount, pKernPairs) == 0)
	{
		delete pKernPairs;

		throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETKERNINGPAIRS,
			__FUNCTIONW__);
	}

	for(LPKERNINGPAIR pCurPair = pKernPairs,
		pEndPair = pKernPairs + dwKernPairCount;
		pCurPair != pEndPair;
		pCurPair++)
	{
		InsertKerningPair(pCurPair->wFirst, pCurPair->wSecond,
			int(float(pCurPair->iKernAmount) * fSpacingScale));
	}

	delete[] pKernPairs;
}

void Font::GetValidGlyphRanges(HDC hDC, const TEXTMETRIC& rtm,
	LPWSTR pszRange, LPWSTR* ppszEndRange, WCHAR wRangeMax)
{
	// Get unicode ranges for the font

	DWORD dwRangesSize = GetFontUnicodeRanges(hDC, NULL);

	if (0 == dwRangesSize)
		throw Error(Error::WIN_GDI_GETFONTUNICODERANGES, __FUNCTIONW__);

	LPGLYPHSET pRanges = NULL;

	try
	{
		pRanges = (LPGLYPHSET)new BYTE[dwRangesSize];
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, dwRangesSize);
	}

	ZeroMemory(pRanges, dwRangesSize);

	pRanges->cbThis = dwRangesSize;

	if (GetFontUnicodeRanges(hDC, pRanges) == 0)
		throw Error(Error::WIN_GDI_GETFONTUNICODERANGES, __FUNCTIONW__);

	// Copy range start/end to passed string

	if (pRanges->cRanges > wRangeMax)
		pRanges->cRanges = DWORD(wRangeMax);

	for(DWORD dwRange = 0;
		dwRange < pRanges->cRanges;
		dwRange++, pszRange += 2)
	{
		pszRange[0] = pRanges->ranges[dwRange].wcLow;
		pszRange[1] = pRanges->ranges[dwRange].wcLow +
			pRanges->ranges[dwRange].cGlyphs - 1;
	}

	*ppszEndRange = pszRange;

	// Clean up

	delete[] (LPBYTE)pRanges;
}

void Font::GetGlyphImageTrueType(HDC hDC,
						 UINT uGlyph,
						 const D3DLOCKED_RECT& rDestInfo,
						 D3DFORMAT nDestFormat,
						 int nDestX,
						 int nDestY,
						 GLYPHMETRICS& rMetrics)
{
	// Get required size for glyph bitmap

	GLYPHMETRICS gm;

	UINT uGlyphBitmapSize = GetGlyphOutline(hDC, uGlyph,
		GGO_GRAY8_BITMAP, &gm, 0, NULL, &MTX2_IDENTITY);

	if (GDI_ERROR == uGlyphBitmapSize)
	{
		// If couldn't get image of commonly undefined glyph, ignore error.

		if (uGlyph <= L' ' || uGlyph > L'~')
			return;

		throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETGLYPHOUTLINE,
			__FUNCTIONW__);
	}

	// Calculate pitch based on size

	UINT uGlyphBitmapPitch = uGlyphBitmapSize / gm.gmBlackBoxY;

	// Allocate temporary place for glyph bitmap

	LPBYTE pbGlyphBitmap = new BYTE[uGlyphBitmapSize];

	// Retrieve glyph bitmap with 65 levels of gray

	if (GetGlyphOutline(hDC, uGlyph, GGO_GRAY8_BITMAP, &gm,
		uGlyphBitmapSize, (LPVOID)pbGlyphBitmap, &MTX2_IDENTITY) == GDI_ERROR)
	{
		delete[] pbGlyphBitmap;

		throw m_rEngine.GetErrors().Push(Error::WIN_GDI_GETGLYPHOUTLINE,
			__FUNCTIONW__);
	}

	if (D3DFMT_A4R4G4B4 == nDestFormat)
	{
		// High Color mode

		const double D_COLOR_MUL4 = 15.0f / 64.0f;

		LPBYTE pbDest = ((LPBYTE)rDestInfo.pBits) +
			nDestY * rDestInfo.Pitch + nDestX * sizeof(WORD);

		LPBYTE pbSource = pbGlyphBitmap;

		for(LPBYTE pbSourceEnd = pbSource + uGlyphBitmapPitch * gm.gmBlackBoxY;
			pbSource != pbSourceEnd;
			pbSource += uGlyphBitmapPitch, pbDest += UINT(rDestInfo.Pitch))
		{			
			// Convert scanline

			for(UINT x = 0, xDest = 0;
				x < gm.gmBlackBoxX;
				x++, xDest += sizeof(WORD))
			{
				BYTE byShade = BYTE(double(pbSource[x]) * D_COLOR_MUL4);

				*((WORD*)&pbDest[xDest]) =
					COLOR5_A4R4G4B4(byShade, 0xF, 0xF, 0xF);
			}
		}
	}
	else
	{
		// True Color mode

		const double D_COLOR_MUL8 = 255.0f / 64.0f;

		LPBYTE pbDest = ((LPBYTE)rDestInfo.pBits) +
			nDestY * rDestInfo.Pitch + nDestX * sizeof(DWORD);

		LPBYTE pbSource = pbGlyphBitmap;

		for(LPBYTE pbSourceEnd = pbSource + uGlyphBitmapPitch * gm.gmBlackBoxY;
			pbSource != pbSourceEnd;
			pbSource += uGlyphBitmapPitch, pbDest += UINT(rDestInfo.Pitch))
		{
			// Convert scanline

			for(UINT x = 0, xDest = 0;
				x < gm.gmBlackBoxX;
				x++, xDest += sizeof(DWORD))
			{
				BYTE byShade = BYTE(double(pbSource[x]) * D_COLOR_MUL8);

				*((D3DCOLOR*)&pbDest[xDest]) =
					D3DCOLOR_ARGB(byShade, 255, 255, 255 );
			}
		}
	}

	// Clean up glyph bitmap

	delete[] pbGlyphBitmap;

	// Copy metrics

	CopyMemory(&rMetrics, &gm, sizeof(GLYPHMETRICS));
}

void Font::GetGlyphImageRaster(HDC hDC, UINT uGlyph,
	LPBYTE pbSrcBitmap, const D3DLOCKED_RECT& rDestInfo,
	D3DFORMAT nDestFormat, int nDestX, int nDestY,
	UINT uSrcHeight, UINT uSrcPitch, SIZE sizeGlyph)
{
	// Draw glyph on DIB section

	TextOut(hDC, 0, 0, (LPCWSTR)&uGlyph, 1);

	// Commit GDI render batch before accessing pixels directly

	GdiFlush();

	// Copy glyph DIB pixels to destination font texture

	LPBYTE pbSource = pbSrcBitmap;
	LPBYTE pbSourceEnd = pbSource + uSrcPitch * uSrcHeight;

	if (D3DFMT_A8R8G8B8 == nDestFormat)
	{
		LPBYTE pbDest = ((LPBYTE)rDestInfo.pBits) +
			nDestY * rDestInfo.Pitch + nDestX * sizeof(DWORD);

		for(;pbSource != pbSourceEnd;
			pbSource += uSrcPitch, pbDest += UINT(rDestInfo.Pitch))
		{
			for(int x = 0, xDest = 0;
				x < sizeGlyph.cx;
				x++, xDest += sizeof(DWORD))
			{
				*((D3DCOLOR*)&pbDest[xDest]) =
					D3DCOLOR_ARGB(0xFF - pbSource[x * sizeof(DWORD) + 1],
						0xFF, 0xFF, 0xFF );
			}
		}
	}
	else
	{
		LPBYTE pbDest = ((LPBYTE)rDestInfo.pBits) +
			nDestY * rDestInfo.Pitch + nDestX * sizeof(WORD);

		for(;pbSource != pbSourceEnd;
			pbSource += uSrcPitch, pbDest += UINT(rDestInfo.Pitch))
		{
			for(int x = 0, xDest = 0;
				x < sizeGlyph.cx;
				x++, xDest += sizeof(WORD))
			{
				*((WORD*)&pbDest[xDest]) =
					COLOR5_A4R4G4B4((0xF - pbSource[x * sizeof(WORD)]),
						0xF, 0xF, 0xF);
			}
		}
	}
}

void Font::OnLostDevice(bool bRecreate)
{
	if (true == bRecreate)
		Empty();
}

void Font::OnResetDevice(bool bRecreate)
{
	if (true == bRecreate)
		Reload();
}

void Font::Serialize(LPCWSTR pszPath) const
{
	if (NULL == pszPath || L'\0' == *pszPath)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(pszPath, GENERIC_WRITE, CREATE_ALWAYS);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	Serialize(stream);
}

void Font::Deserialize(LPCWSTR pszPath)
{
	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(pszPath, GENERIC_READ,
			OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, pszPath);
	}

	m_strName = pszPath;

	Deserialize(stream);
}

void Font::Deserialize(Stream& rStream)
{
	InfoFile doc(&m_rEngine.GetErrors());

	try
	{
		doc.Deserialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	if (NULL == doc.GetRoot())
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());

	Deserialize(*doc.GetRoot());
}

void Font::Deserialize(const InfoElem& rRoot)
{
	Empty();

	// Read generate

	const InfoElem* pElem = rRoot.FindChildConst(SZ_GENERATE);

	if (pElem != NULL)
	{
		// Read reference material

		const InfoElem* pMatElem = rRoot.FindChildConst(SZ_MATERIAL);

		if (NULL == pMatElem)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
				SZ_MATERIAL, rRoot.GetDocumentConst().GetPath());

		Material* pMaterial = m_rEngine.GetMaterials().LoadInstance(*pMatElem);

		// Read options

		LPCWSTR pszFaceName = L"Arial", pszRange = NULL;
		int nSize = 10;
		bool bBold = false, bItalic = false;
		float fSpacingScale = 1.0f;

		if (pElem->GetChildCount() > 0)
		{
			// Read face name

			if (pElem->GetChildConst(0)->GetVarType() != Variable::TYPE_STRING)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			pszFaceName = pElem->GetChildConst(0)->GetStringValue();
		}

		if (pElem->GetChildCount() > 1)
		{
			// Read size

			if (pElem->GetChildConst(1)->GetVarType() != Variable::TYPE_INT)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			nSize = pElem->GetChildConst(1)->GetIntValue();
		}

		if (pElem->GetChildCount() > 2)
		{
			// Read bold

			if (pElem->GetChildConst(2)->GetVarType() != Variable::TYPE_BOOL)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			bBold = pElem->GetChildConst(2)->GetBoolValue();
		}

		if (pElem->GetChildCount() > 3)
		{
			// Read italic

			if (pElem->GetChildConst(3)->GetVarType() != Variable::TYPE_BOOL)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			bItalic = pElem->GetChildConst(3)->GetBoolValue();
		}

		if (pElem->GetChildCount() > 4)
		{
			// Read range

			if (pElem->GetChildConst(4)->GetVarType() != Variable::TYPE_STRING)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			pszRange = pElem->GetChildConst(4)->GetStringValue();
		}

		if (pElem->GetChildCount() > 5)
		{
			// Read spacing scale

			if (pElem->GetChildConst(5)->GetVarType() != Variable::TYPE_FLOAT)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
					SZ_GENERATE, rRoot.GetDocumentConst().GetPath());

			fSpacingScale = pElem->GetChildConst(5)->GetFloatValue();
		}

		// Generate

		Generate(pszFaceName, nSize, bBold, bItalic, pszRange, fSpacingScale, pMaterial);

		return;
	}

	// Read max char width

	pElem = rRoot.FindChildConst(SZ_MAX_CHAR_WIDTH);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_MAX_CHAR_WIDTH, rRoot.GetDocumentConst().GetPath());

	if (pElem->GetVarType() != InfoElem::TYPE_INT &&
	   pElem->GetVarType() != InfoElem::TYPE_DWORD)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
			__FUNCTIONW__, SZ_MAX_CHAR_WIDTH,
			rRoot.GetDocumentConst().GetPath());


	m_nMaxCharWidth = pElem->GetIntValue();

	// Read average char width

	pElem = rRoot.FindChildConst(SZ_AVE_CHAR_WIDTH);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_AVE_CHAR_WIDTH, rRoot.GetDocumentConst().GetPath());

	if (pElem->GetVarType() != InfoElem::TYPE_INT &&
	   pElem->GetVarType() != InfoElem::TYPE_DWORD)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
			__FUNCTIONW__, SZ_AVE_CHAR_WIDTH,
			rRoot.GetDocumentConst().GetPath());

	m_nAveCharWidth = pElem->GetIntValue();

	// Read char height

	pElem = rRoot.FindChildConst(SZ_CHAR_HEIGHT);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_CHAR_HEIGHT, rRoot.GetDocumentConst().GetPath());

	if (pElem->GetVarType() != InfoElem::TYPE_INT &&
		pElem->GetVarType() != InfoElem::TYPE_DWORD)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
			__FUNCTIONW__, SZ_CHAR_HEIGHT,
			rRoot.GetDocumentConst().GetPath());

	// Read ascent

	pElem = rRoot.FindChildConst(SZ_ASCENT);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_ASCENT, rRoot.GetDocumentConst().GetPath());

	if (pElem->GetVarType() != InfoElem::TYPE_INT &&
		pElem->GetVarType() != InfoElem::TYPE_DWORD)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
			__FUNCTIONW__, SZ_ASCENT,
			rRoot.GetDocumentConst().GetPath());

	// Read line spacing

	pElem = rRoot.FindChildConst(SZ_LINE_SPACING);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_LINE_SPACING, rRoot.GetDocumentConst().GetPath());

	if (pElem->GetVarType() != InfoElem::TYPE_INT &&
		pElem->GetVarType() != InfoElem::TYPE_DWORD)
	   throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
			__FUNCTIONW__, SZ_LINE_SPACING, rRoot.GetDocumentConst().GetPath());

	m_nLineSpacing = pElem->GetIntValue();

	// Read Glyphs

	InfoElemConstRange range = rRoot.FindChildrenConst(SZ_GLYPH);

	const InfoElem* pChild = NULL;

	FontGlyph cur;

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		cur.Deserialize(m_rEngine, *pos->second);

		InsertGlyph(*pos->second->GetStringValue(), cur);
	}

	// Read Kerning

	range = rRoot.FindChildrenConst(SZ_KERNINGPAIR);

	for(InfoElemConstRangeIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		pElem = pos->second;

		if (pElem->GetElemType() != InfoElem::TYPE_VALUELIST &&
		   pElem->GetChildCount() != 3)
		   throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
				__FUNCTIONW__, SZ_KERNINGPAIR,
				pElem->GetDocumentConst().GetPath());

		pChild = pElem->GetChildConst(0);

		WCHAR chFirst = (pChild->GetVarType() == InfoElem::TYPE_STRING ||
			pChild->GetVarType() == InfoElem::TYPE_ENUM) ?
				*pChild->GetStringValue() : WCHAR(pChild->GetIntValue());

		pChild = pElem->GetChildConst(1);

		WCHAR chSecond = (pChild->GetVarType() == InfoElem::TYPE_STRING ||
			pChild->GetVarType() == InfoElem::TYPE_ENUM) ?
			*pChild->GetStringValue() : WCHAR(pChild->GetIntValue());

		m_mapKerning[KerningKey(chFirst, chSecond)] =
			pElem->GetChildConst(3)->GetIntValue();
	}
}

void Font::Serialize(Stream& rStream) const
{
	InfoFile doc(&m_rEngine.GetErrors());

	Serialize(*doc.CreateSetRoot(SZ_ROOT));

	try
	{
		doc.Serialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE, __FUNCTIONW__,
			rStream.GetPath());
	}
}

void Font::Serialize(InfoElem& rRoot) const
{
	// Write max char width

	rRoot.AddChild(SZ_MAX_CHAR_WIDTH,
		InfoElem::TYPE_VALUE,
		InfoElem::TYPE_INT)->SetIntValue(m_nMaxCharWidth);

	// Write ave char width

	rRoot.AddChild(SZ_AVE_CHAR_WIDTH,
		InfoElem::TYPE_VALUE,
		InfoElem::TYPE_INT)->SetIntValue(m_nAveCharWidth);

	// Write char height

	rRoot.AddChild(SZ_CHAR_HEIGHT,
		InfoElem::TYPE_VALUE,
		InfoElem::TYPE_INT)->SetIntValue(m_nCharHeight);

	// Write ascent

	rRoot.AddChild(SZ_ASCENT,
		InfoElem::TYPE_VALUE,
		InfoElem::TYPE_INT)->SetIntValue(m_nAscent);

	// Write line spacing

	rRoot.AddChild(SZ_LINE_SPACING,
		InfoElem::TYPE_VALUE,
		InfoElem::TYPE_INT)->SetIntValue(m_nLineSpacing);

	// Write glyphs

	for(FontGlyphMapConstIterator pos = m_mapGlyphs.begin();
		pos != m_mapGlyphs.end();
		pos++)
	{
		pos->second.Serialize(m_rEngine, *rRoot.AddChild(SZ_GLYPH,
			InfoElem::TYPE_VALUEBLOCK));
	}

	// Write kerning

	WCHAR sz[2] = { L'\0', L'\0' };

	for(KerningMapConstIterator pos = m_mapKerning.begin();
		pos != m_mapKerning.end();
		pos++)
	{
		if (0 == pos->second)
			continue;

		InfoElem* pChild = rRoot.AddChild(SZ_KERNINGPAIR,
			InfoElem::TYPE_VALUELIST);		

		// First character in pair

		sz[0] = pos->first.first;
		pChild->AddChild()->SetEnumValue(sz);

		// Second character in pair

		sz[0] = pos->first.second;
		pChild->AddChild()->SetEnumValue(sz);

		// Distance
		
		pChild->AddChild(NULL, InfoElem::TYPE_VALUE, InfoElem::TYPE_INT)->
			SetIntValue(pos->second);
	}
}

DWORD Font::GetMemoryFootprint(void) const
{
	return Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(Font) +
		m_mapGlyphs.size() * sizeof(DWORD) * 2 +
		m_mapGlyphs.size() * sizeof(FontGlyph) +
		m_mapKerning.size() * sizeof(DWORD) * 4;
}

void Font::Empty(void)
{
	m_nMaxCharWidth = 0;
	m_nAveCharWidth = 0;
	m_nLineSpacing = 0;
	m_mapGlyphs.clear();
	m_mapKerning.clear();

	for(std::vector<Material*>::iterator pos = m_arGenMaterials.begin();
		pos != m_arGenMaterials.end();
		pos++)
	{
		/*String strName = (*pos)->GetName();
		long posAddr = (long)(*pos);
		int refCount = (*pos)->Release();

		m_rEngine.PrintDebug(L"font release attached mat %s %x, %d refs (after)", strName,  posAddr	,refCount);*/

		(*pos)->Release();
	}

	m_arGenMaterials.clear();

	for(std::vector<Texture*>::iterator pos = m_arGenTextures.begin();
		pos != m_arGenTextures.end();
		pos++)
	{
		/*String strName = (*pos)->GetName();
		long posAddr = (long)(*pos);
		int refCount = (*pos)->Release();

		m_rEngine.PrintDebug(L"font release attached tex %s %x, %d refs (after)", strName, posAddr, refCount);*/

		(*pos)->Release();
	}

	m_arGenTextures.clear();
}

void Font::Remove(void)
{
	m_rEngine.GetFonts().Remove(this);
}

/*----------------------------------------------------------*\
| FontGlyph implementation
\*----------------------------------------------------------*/

FontGlyph::FontGlyph(void)
{
	ZeroMemory(&m_abcSpacing, sizeof(ABC));
	ZeroMemory(&m_ptOrigin, sizeof(POINT));
}

FontGlyph::FontGlyph(const FontGlyph& rInit):
					 m_materialInst(rInit.m_materialInst)
{
	CopyMemory(&m_abcSpacing, &rInit.m_abcSpacing, sizeof(ABC));
	CopyMemory(&m_ptOrigin, &rInit.m_ptOrigin, sizeof(POINT));
}

void FontGlyph::SetSpacing(int a, UINT b, int c)
{
	m_abcSpacing.abcA = a;
	m_abcSpacing.abcB = b;
	m_abcSpacing.abcC = c;
}

void FontGlyph::SetOrigin(int x, int y)
{
	m_ptOrigin.x = x;
	m_ptOrigin.y = y;
}

void FontGlyph::Serialize(Engine& rEngine, InfoElem& rElem) const
{
	rElem.AddChild(SZ_ABC)->FromIntArray(
		reinterpret_cast<const int*>(&m_abcSpacing), 3);

	rElem.AddChild(SZ_ORIGIN)->FromIntArray(
		reinterpret_cast<const int*>(&m_ptOrigin), 2);

	m_materialInst.Serialize(rEngine,
		*rElem.AddChild(Font::SZ_MATERIAL));
}

void FontGlyph::Deserialize(Engine& rEngine, const InfoElem& rElem)
{
	const InfoElem* pElem = rElem.FindChildConst(SZ_ABC,
		InfoElem::TYPE_VALUELIST);

	if (NULL == pElem)
		throw rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, SZ_ABC,
			rElem.GetDocumentConst().GetPath());

	pElem->ToIntArray(reinterpret_cast<int*>(&m_abcSpacing), 3);

	pElem = rElem.FindChildConst(SZ_ORIGIN,
		InfoElem::TYPE_VALUELIST);

	if (pElem != NULL)
		pElem->ToIntArray(reinterpret_cast<int*>(&m_ptOrigin), 2);

	pElem = rElem.FindChildConst(Font::SZ_MATERIAL);

	if (NULL == pElem)
		throw rEngine.GetErrors().Push(Error::FILE_ELEMENT,
			__FUNCTIONW__, Font::SZ_MATERIAL,
			rElem.GetDocumentConst().GetPath());

	m_materialInst.Deserialize(rEngine, *pElem);
}

void FontGlyph::Empty(void)
{
	m_materialInst.Empty();

	ZeroMemory(&m_abcSpacing, sizeof(ABC));
	ZeroMemory(&m_ptOrigin, sizeof(POINT));
}

/*----------------------------------------------------------*\
| TextBlock implementation
\*----------------------------------------------------------*/

void TextBlock::Render(void)
{
	if (m_arPackedEntries.empty() == true)
		return;

	// Get reference to graphics renderer

	bool bWireframe = (m_arPackedEntries[0].pMaterial->
		GetMaterial()->GetEngine().GetOption(Engine::OPTION_WIREFRAME) == TRUE);

	Graphics& rGraphics = m_arPackedEntries[0].pMaterial->
		GetMaterial()->GetEngine().GetGraphics();

	// Flush batch if batching

	rGraphics.FlushBatch();

	// Set vertex declaration

	HRESULT hr = rGraphics.GetDevice()->SetVertexDeclaration(
		rGraphics.GetTriangleVD());

	if (FAILED(hr))
		throw rGraphics.GetEngine().GetErrors().Push(
			Error::D3D_DEVICE_SETVERTEXDECLARATION,
			__FUNCTIONW__, hr);

	// Set vertex buffer

	hr = rGraphics.GetDevice()->SetStreamSource(
		0, m_VB.GetBuffer(), 0, m_VB.GetVertexSize());

	if (FAILED(hr))
		throw rGraphics.GetEngine().GetErrors().Push(
			Error::D3D_DEVICE_SETSTREAMSOURCE,
			__FUNCTIONW__, hr);

	// Set index buffer

	hr = rGraphics.GetDevice()->SetIndices(
		rGraphics.GetTriangleIB().GetBuffer());

	if (FAILED(hr))
		rGraphics.GetEngine().GetErrors().Push(
			Error::D3D_DEVICE_SETINDICES,
			__FUNCTIONW__, hr);

	// Render all entries

	MaterialInstanceShared* pLastMat = NULL;
	const EffectTechnique* pLastTech = NULL;
	UINT uPasses = 0;
	UINT uMinVertexIndex = 0;
	UINT uStartIndex = 0;

	for(std::vector<EntryPacked>::iterator pos =
		m_arPackedEntries.begin();
		pos != m_arPackedEntries.end();
		pos++)
	{
		MaterialInstanceShared* pCurMat = (true == bWireframe) ?
			rGraphics.GetWireframeMaterial() :
			pos->pMaterial;			

		// Technique changed or material parameter block needs caching?

		if (pLastTech != pCurMat->GetTechniqueConst() ||
			pCurMat->IsDirty() == true)
		{
			// End previous material if any

			if (pLastMat != NULL)
				pLastMat->End();

			// Apply material

			pCurMat->Apply();

			// Begin a new one

			uPasses = pCurMat->Begin();

			pLastMat = pCurMat;
			pLastTech = pCurMat->GetTechniqueConst();
		}
		else
		{
			// Apply material (internals take care of not setting technique if set)

			if (pLastMat != pCurMat)
			{
				pCurMat->Apply();

				pLastMat = pCurMat;
			}
		}		

		// Render quads

		for(UINT n = 0; n < uPasses; n++)
		{
			pCurMat->BeginPass(n);

			hr = rGraphics.GetDevice()->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				uMinVertexIndex,
				pos->uVertexCount,
				uStartIndex,
				pos->uPrimitiveCount);

			pCurMat->EndPass();
		}

		if (FAILED(hr))
			throw rGraphics.GetEngine().GetErrors().Push(
				Error::D3D_DEVICE_DRAWINDEXEDPRIMITIVE,
				__FUNCTIONW__, hr);
				

		uMinVertexIndex += pos->uVertexCount;
		uStartIndex += pos->uPrimitiveCount * 3;

		// Update statistics
		
		rGraphics.UpdateBatchCount(uPasses);
		rGraphics.UpdateMaxPrimitivesPerBatch(pos->uPrimitiveCount);
		rGraphics.UpdateTriangleCount(pos->uPrimitiveCount * uPasses);
	}

	// End last material

	if (pLastMat != NULL)
		pLastMat->End();
}

void TextBlock::PreCache(int nGlyphCount)
{
	m_arEntries.reserve(nGlyphCount);
}

void TextBlock::PreCacheGlyph(const MaterialInstance& rMaterialInst,
							  int x,
							  int y,
							  int cx,
							  int cy,
							  D3DCOLOR clrBlend)
{
	Entry entry;

	entry.pGlyphInst = &rMaterialInst;
	entry.x = x;
	entry.y = y;

	entry.cx = BYTE((-1 == cx) ?
		rMaterialInst.GetTextureCoords().GetWidth() :
		cx);

	entry.cy = BYTE((-1 == cy) ?
		rMaterialInst.GetTextureCoords().GetHeight() :
		cy);

	entry.blend = clrBlend;

	m_arEntries.push_back(entry);
}

void TextBlock::Cache(void)
{
	if (m_arEntries.empty() == true)
		return;

	// Preallocate packed entries to avoid multiple allocations

	m_VB.Empty();
	m_arPackedEntries.clear();

	m_arPackedEntries.reserve(m_arEntries.size());

	// Sort unpacked entries by material

	std::sort(m_arEntries.begin(),
		m_arEntries.end(), Entry::compare);

	// Allocate vertex buffer

	m_VB.Create(
		m_arEntries[0].pGlyphInst->GetMaterial()->GetEngine().GetGraphics(),
		UINT(m_arEntries.size() * 4));

	// Lock vertex buffer

	LPBYTE pbData = NULL;
	m_VB.Lock(0, (void**)&pbData, 0);

	// Pack entries

	MaterialInstanceShared* pMat = NULL;
	int nBatch = -1;

	for(std::vector<Entry>::iterator pos =
		m_arEntries.begin();
		pos != m_arEntries.end();
		pos++)
	{
		// Generate a quad for this glyph

		float u1, v1, u2, v2;
		pos->pGlyphInst->GetTextureCoords(u1, v1, u2, v2);

		Vector2 vecSize(float(pos->cx), float(pos->cy));

		VertexTriangle vertices[4] = {

			// Top Left

			float(pos->x), float(pos->y),
			pos->blend,
			u1, v1,

			// Top Right

			float(pos->x + vecSize.x),
			float(pos->y),
			pos->blend,
			u2,	v1,

			// Bottom Right

			float(pos->x + vecSize.x),
			float(pos->y + vecSize.y),
			pos->blend,
			u2,	v2,

			// Bottom Left

			float(pos->x),
			float(pos->y + vecSize.y),
			pos->blend,
			u1, v2
		};

		if (pos->pGlyphInst->GetSharedMaterial() != pMat)
		{
			// Material changed - start new batch

			EntryPacked packed;
			packed.pMaterial = pos->pGlyphInst->GetSharedMaterial();
			packed.uVertexCount = 4;
			packed.uPrimitiveCount = 2;

			m_arPackedEntries.push_back(packed);

			pMat = pos->pGlyphInst->GetSharedMaterial();

			nBatch++;
		}
		else
		{
			// Material stays the same, continue previous batch

			m_arPackedEntries.back().uVertexCount += 4;
			m_arPackedEntries.back().uPrimitiveCount += 2;
		}

		CopyMemory(pbData, vertices, sizeof(vertices));
		pbData += sizeof(vertices);
	}

	// Unlock vertex buffer

	m_VB.Unlock();

	// Remove temporary unpacked structures

	m_arEntries.clear();

	// Mark as cached

	m_bDirty = false;
}