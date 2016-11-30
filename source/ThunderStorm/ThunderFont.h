/*------------------------------------------------------------------*\
|
| ThunderFont.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine font class(es)
| Created: 11/19/2005
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_FONT_H
#define THUNDER_FONT_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderResource.h"	// using Resource
#include "ThunderMaterial.h"	// using MaterialInstance

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;		// referencing Engine
class InfoElem;		// referencing InfoElem
class FontGlyph;	// referencing FontGlyph, declared below
class TextBlock;	// referencing TextBlock, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<WCHAR, FontGlyph> FontGlyphMap;
typedef std::map<WCHAR, FontGlyph>::iterator FontGlyphMapIterator;
typedef std::map<WCHAR, FontGlyph>::const_iterator FontGlyphMapConstIterator;

typedef std::pair<WCHAR, WCHAR> KerningKey;

typedef std::map<KerningKey, int> KerningMap;
typedef std::map<KerningKey, int>::iterator KerningMapIterator;
typedef std::map<KerningKey, int>::const_iterator KerningMapConstIterator;


/*----------------------------------------------------------*\
| Font class
\*----------------------------------------------------------*/

class Font: public Resource
{
public:
	//
	// Constants
	//

	// Flags used for rendering and getting text extent

	enum RenderFlags
	{
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1 << 0,
		ALIGN_RIGHT = 1 << 1,
		ALIGN_VCENTER = 1 << 2,
		USE_MNEMONICS = 1 << 3
	};
	
	// Elements

	static const WCHAR SZ_ROOT[];

	static const WCHAR SZ_GENERATE[];
	static const WCHAR SZ_MATERIAL[];
	static const WCHAR SZ_MAX_CHAR_WIDTH[];
	static const WCHAR SZ_AVE_CHAR_WIDTH[];
	static const WCHAR SZ_CHAR_HEIGHT[];
	static const WCHAR SZ_ASCENT[];
	static const WCHAR SZ_LINE_SPACING[];
	static const WCHAR SZ_GLYPH[];
	static const WCHAR SZ_KERNINGPAIR[];
	static const MAT2 MTX2_IDENTITY;

private:
	//
	// Members
	//

	// Max char width text metric
	int m_nMaxCharWidth;

	// Average char width text metric
	int m_nAveCharWidth;

	// Char height
	int m_nCharHeight;

	// Ascent (distance from top of char cell to baseline)
	int m_nAscent;

	// Max char height text metric, from height + ext leading
	int m_nLineSpacing;

	// Glyph to render for undefined glyphs
	WCHAR m_wDefaultChar;

	// Padding to prevent unaligned access for m_mapGlyphs
	BYTE _padding[8];
	
	// Defined glyphs
	FontGlyphMap m_mapGlyphs;

	// Kerning pairs
	KerningMap m_mapKerning;

	// Generated materials, keep track to reload on device recreate
	std::vector<Texture*> m_arGenTextures;
	std::vector<Material*> m_arGenMaterials;

public:
	Font(Engine& rEngine);
	virtual ~Font(void);

public:
	//
	// Metrics
	//

	inline int GetMaxCharWidth(void) const
	{
		return m_nMaxCharWidth;
	}

	inline int GetAveCharWidth(void) const
	{
		return m_nAveCharWidth;
	}

	inline int GetCharHeight(void) const
	{
		return m_nCharHeight;
	}

	inline int GetAscent(void) const
	{
		return m_nAscent;
	}

	inline int GetLineSpacing(void) const
	{
		return m_nLineSpacing;
	}

	inline WCHAR GetDefaultChar(void) const
	{
		return m_wDefaultChar;
	}

	SIZE GetTextExtent(LPCWSTR psz, int nCount, DWORD dwFlags,
		int nMaxWidth, int* pnLineCount = NULL) const;
	
	//
	// Glyphs
	//

	FontGlyph* GetGlyph(WCHAR ch);
	const FontGlyph* GetGlyphConst(WCHAR ch) const;

	FontGlyph& InsertGlyph(WCHAR ch, const FontGlyph& rGlyph);

	void RemoveGlyph(WCHAR ch);

	inline void RemoveAllGlyphs(void)
	{
		m_mapGlyphs.clear();
	}

	inline int GetGlyphCount(void) const
	{
		return int(m_mapGlyphs.size());
	}

	//
	// Kerning
	//

	int GetKerningPair(WCHAR ch1, WCHAR ch2) const;

	inline void InsertKerningPair(WCHAR ch1, WCHAR ch2, int nKernAmount)
	{
		m_mapKerning[KerningKey(ch1, ch2)] = nKernAmount;
	}

	void RemoveKerningPair(WCHAR ch1, WCHAR ch2);

	inline void RemoveAllKerningPairs(void)
	{
		m_mapKerning.clear();
	}

	//
	// Rendering
	//

	void RenderGlyph(const Vector2& rvecPos,
		WCHAR ch, D3DCOLOR clrBlend = 0xFFFFFFFF);

	void RenderText(const Rect& rrcText,
		LPCWSTR psz, int nCount = -1,
		D3DCOLOR clrBlend = 0xFFFFFFFF,
		DWORD dwFlags = ALIGN_LEFT,
		TextBlock* pCache = NULL);

	//
	// Generation
	//

	void Generate(LPCWSTR pszFaceName, int nSize,
		bool bBold, bool bItalic, LPCWSTR pszCharRangePairs,
		float fSpacingScale,
		Material* pReferenceMaterial);

	//
	// Device Events
	//
	
	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	//
	// Serialization
	//

	virtual void Serialize(LPCWSTR pszPath) const;
	virtual void Serialize(Stream& rStream) const;

	void Serialize(InfoElem& rRoot) const;

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);

	void Deserialize(const InfoElem& rRoot);
	
	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);

private:
	//
	// Private Functions
	//

	void GetKerningInfo(HDC hDC, float fSpacingScale);

	void GetValidGlyphRanges(HDC hDC, const TEXTMETRIC& rtm,
		LPWSTR pszRange, LPWSTR* ppszEndRange, WCHAR wRangeMax);

	void GetGlyphImageTrueType(HDC hDC, UINT uGlyph,
		const D3DLOCKED_RECT& rDestInfo, D3DFORMAT nDestFormat,
			int nDestX, int nDestY, GLYPHMETRICS& rMetrics);

	void GetGlyphImageRaster(HDC hDC, UINT uGlyph,
		LPBYTE pbSrcBitmap, const D3DLOCKED_RECT& rDestInfo,
		D3DFORMAT nDestFormat, int nDestX, int nDestY,
		UINT uSrcHeight, UINT uSrcPitch, SIZE sizeGlyph);

	static bool FontExists(LPCWSTR pszFaceName);

	static int CALLBACK Font::FontEnumProc(const LOGFONT *lpelfe,
		const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam);
};

/*----------------------------------------------------------*\
| FontGlyph class
\*----------------------------------------------------------*/

class FontGlyph
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_ABC[];
	static const WCHAR SZ_ORIGIN[];

private:
	//
	// Members
	//

	// Spacing and width
	ABC m_abcSpacing;

	// Origin of black box
	POINT m_ptOrigin;

	// Location on font texture and black box size
	MaterialInstance m_materialInst;

public:
	FontGlyph(void);
	FontGlyph(const FontGlyph& rInit);

public:
	//
	// Spacing and Origin
	//

	inline const ABC& GetSpacingConst(void) const
	{
		return m_abcSpacing;
	}

	inline int GetTotalSpacing(void) const
	{
		return m_abcSpacing.abcA + m_abcSpacing.abcB + m_abcSpacing.abcC;
	}

	inline ABC& GetSpacing(void)
	{
		return m_abcSpacing;
	}

	inline void SetSpacing(const ABC& rABC)
	{
		CopyMemory(&m_abcSpacing, &rABC, sizeof(ABC));
	}

	void SetSpacing(int a, UINT b, int c);

	inline POINT GetOrigin(void) const
	{
		return m_ptOrigin;
	}

	inline void SetOrigin(POINT ptOrigin)
	{
		CopyMemory(&m_ptOrigin, &ptOrigin, sizeof(POINT));
	}

	void SetOrigin(int x, int y);

	//
	// Material
	//

	MaterialInstance& GetMaterialInstance(void)
	{
		return m_materialInst;
	}

	const MaterialInstance& GetMaterialInstanceConst(void) const
	{
		return m_materialInst;
	}

	//
	// Serialization
	//

	void Serialize(Engine& rEngine, InfoElem& rElem) const;
	void Deserialize(Engine& rEngine, const InfoElem& rElem);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| TextBlock class - pre-cached text
\*----------------------------------------------------------*/

class TextBlock
{
private:
	//
	// Structures
	//

	class Entry
	{
	public:
		const MaterialInstance* pGlyphInst;
		int x;
		int y;
		BYTE cx;
		BYTE cy;
		D3DCOLOR blend;

		static bool compare(const Entry& rFirst, const Entry& rSecond)
		{
			return (rFirst.pGlyphInst->GetSharedMaterial() <
					rSecond.pGlyphInst->GetSharedMaterial());
		}
	};

	class EntryPacked
	{
	public:
		MaterialInstanceShared* pMaterial;
		UINT uVertexCount;
		UINT uPrimitiveCount;
	};

private:
	bool m_bDirty;

	VertexBuffer m_VB;
	
	std::vector<Entry> m_arEntries;
	std::vector<EntryPacked> m_arPackedEntries;

public:
	inline TextBlock(void):
		m_VB(sizeof(VertexTriangle), D3DUSAGE_WRITEONLY),
		m_bDirty(true)
	{
	}

	inline ~TextBlock(void)
	{
		Empty();
	}

public:
	void Cache(void);

	void Render(void);

	inline bool IsDirty(void) const
	{
		return m_bDirty;
	}

	inline void Empty(void)
	{
		m_VB.Empty();
		m_arEntries.clear();
		m_arPackedEntries.clear();
		m_bDirty = true;
	}

public:
	//
	// Friends
	//

	friend class Font;

private:
	//
	// Private Functions
	//

	void PreCacheGlyph(const MaterialInstance& rMaterialInst,
		int x, int y, int cx = -1, int cy = -1, D3DCOLOR clrColor = Color::BLEND_ONE);

	void PreCache(int nGlyphCount);
};

} // namespace ThunderStorm

#endif // THUNDER_FONT_H