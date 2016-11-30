/*------------------------------------------------------------------*\
|
| ThunderRegion.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine region classes implementation
| Created: 07/29/2006
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderRegion.h"		// defining Region/File
#include "ThunderTexture.h"		// using Texture
#include "ThunderAnimation.h"	// using Animation
#include "ThunderSprite.h"		// using Sprite
#include "ThunderStream.h"		// using Stream

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const BYTE RegionSet::RGN_SIGNATURE[]		= "THR";
const BYTE RegionSet::RGN_FORMAT_VERSION[] = { 2, 1, 0, 0 };


/*----------------------------------------------------------*\
| RegionSet class implementation
\*----------------------------------------------------------*/

RegionSet::RegionSet(Engine& rEngine): Resource(rEngine)
{
}

RegionSet::~RegionSet(void)
{
	Empty();
}

Region* RegionSet::CreateRegion(void)
{
	try
	{
		Region* pNewRegion = new Region(this);

		return pNewRegion;
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(Region));
	}
}

int RegionSet::AddRegion(Region* pRegion)
{
	_ASSERT(pRegion->GetRegionSet() == this);

	m_arRegions.push_back(pRegion);

	return int(m_arRegions.size() - 1);
}

int RegionSet::GetRegionCount(void) const
{
	return int(m_arRegions.size());
}

Region* RegionSet::GetRegion(int nRegion)
{
	_ASSERT(nRegion >= 0 && nRegion < int(m_arRegions.size()));

	return m_arRegions[nRegion];
}

void RegionSet::RemoveRegion(int nRegion)
{
	_ASSERT(nRegion >= 0 && nRegion < int(m_arRegions.size()));

	delete m_arRegions[nRegion];

	m_arRegions.erase(m_arRegions.begin() + nRegion);
}

void RegionSet::RemoveAllRegions(void)
{
	for(RegionArrayIterator pos = m_arRegions.begin();
		pos != m_arRegions.end();
		pos++)
	{
		delete *pos;
	}

	m_arRegions.clear();
}

void RegionSet::FromAnimation(Animation& rAnim)
{
	/*
	for(int n = 0; n < rAnim.GetFrameCount(); n++)
	{
		FromTexture(*rAnim.GetMaterial()->GetEffect()->GetBaseParam(),
			rAnim.GetFrameConst(n).GetSrcRect());
	}
	*/
}

void RegionSet::FromSprite(Sprite& rSprite)
{
	/*
	for(int n = 0; n < rSprite.GetAnimationCount(); n++)
	{
		FromAnimation(*rSprite.GetAnimation(n));
	}
	*/
}

void RegionSet::FromTexture(Texture& rTexture, const RECT& rrcSrcRect)
{
	LPDIRECT3DSURFACE9 pSurf = NULL;

	HRESULT hr = rTexture.GetD3DTexture()->GetSurfaceLevel(0, &pSurf);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3D_TEXTURE_GETSURFACELEVEL,
			__FUNCTIONW__, hr);

	Region* pNewRegion = CreateRegion();

	pNewRegion->FromSurface(pSurf, rrcSrcRect);

	AddRegion(pNewRegion);
}

void RegionSet::Serialize(LPCWSTR pszPath) const
{
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

void RegionSet::Deserialize(LPCWSTR pszPath)
{
	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(pszPath, GENERIC_READ, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN);
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

void RegionSet::Serialize(Stream& rStream) const
{
	try
	{
		// Write signature

		rStream.WriteVar((const LPBYTE)RGN_SIGNATURE,
			sizeof(RGN_SIGNATURE));

		// Write version

		rStream.WriteVar((const LPBYTE)RGN_FORMAT_VERSION,
			sizeof(RGN_FORMAT_VERSION));

		// Write the number of regions

		int nRegions = int(m_arRegions.size());	
		rStream.WriteVar(&nRegions);

		// Write regions

		for(int n = 0; n < nRegions; n++)
		{
			m_arRegions[n]->Serialize(rStream);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void RegionSet::Deserialize(Stream& rStream)
{
	Empty();

	try
	{
		// Read signature

		BYTE signature[4] = {0};

		rStream.ReadVar(signature, sizeof(RGN_SIGNATURE));

		// Read format version

		BYTE version[4];

		rStream.ReadVar(version, sizeof(RGN_FORMAT_VERSION));

		// Validate signature

		if (strncmp((char*)signature, (const char*)RGN_SIGNATURE,
			sizeof(RGN_SIGNATURE)))
				throw m_rEngine.GetErrors().Push(Error::FILE_SIGNATURE,
					__FUNCTIONW__, rStream.GetPath());

		// Validate format version

		if (strncmp((char*)version, (const char*)RGN_FORMAT_VERSION,
			sizeof(RGN_FORMAT_VERSION)))
				throw m_rEngine.GetErrors().Push(Error::FILE_VERSION,
					__FUNCTIONW__, rStream.GetPath());

		// Read the number of regions included

		int nRegions = 0;
		rStream.ReadVar(&nRegions);

		// Read the regions

		m_arRegions.resize(nRegions);

		for(int n = 0; n < nRegions; n++)
		{	
			Region* pNewRegion = CreateRegion();

			pNewRegion->Deserialize(rStream);

			m_arRegions[n] = pNewRegion;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

DWORD RegionSet::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() -
				   sizeof(Resource) +
				   sizeof(RegionSet);

	for(std::vector<Region*>::const_iterator pos = m_arRegions.begin();
		pos != m_arRegions.end();
		pos++)
	{
		dwSize += (*pos)->GetMemoryFootprint();
	}

	return dwSize;
}

void RegionSet::Empty(void)
{
	RemoveAllRegions();
}

/*----------------------------------------------------------*\
| Region class implementation
\*----------------------------------------------------------*/

Region::Region(RegionSet* pRegionSet): m_pRegionSet(pRegionSet),
									   m_pData(NULL),
									   m_ppData(NULL)
{
	m_psSize.cx = 0;
	m_psSize.cy = 0;
}

Region::~Region(void)
{
	Empty();
}

RegionSet* Region::GetRegionSet(void)
{
	return m_pRegionSet;
}

const SIZE& Region::GetSize(void) const
{
	return m_psSize;
}

void Region::SetSize(const SIZE& rpsSize)
{
	Empty();

	m_psSize.cx = rpsSize.cx;
	m_psSize.cy = rpsSize.cy;

	int nSize = rpsSize.cx * rpsSize.cy;

	try
	{
		m_pData = new BYTE[nSize];
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, nSize);
	}

	ZeroMemory(m_pData, nSize);

	try
	{
		m_ppData = new BYTE*[rpsSize.cy];
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, rpsSize.cy);
	}

	for(int n = 0; n < rpsSize.cy; n++)
	{
		m_ppData[n] = m_pData + n * rpsSize.cx;
	}
}

BYTE* Region::GetData1D(void)
{
	return m_pData;
}

BYTE** Region::GetData2D(void)
{
	return m_ppData;
}

const BYTE** Region::GetData2DConst(void) const
{
	return (const BYTE**)m_ppData;
}

void Region::FromSurface(LPDIRECT3DSURFACE9 pSurf, const RECT& rrcSrcRect)
{
	if (NULL == pSurf)
		throw m_pRegionSet->GetEngine().GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	Empty();

	// Get surface size and type

	D3DSURFACE_DESC desc;	
	HRESULT hr = pSurf->GetDesc(&desc);

	if (FAILED(hr))
		throw m_pRegionSet->GetEngine().GetErrors().Push(
			Error::D3D_TEXTURE_GETLEVELDESC, __FUNCTIONW__, hr);

	// Surface format must be A8R8G8B8 or X8R8G8B8
	// TODO: if we keep this class, also need code for 16-bit textures

	if (desc.Format != D3DFMT_A8R8G8B8 &&
	   desc.Format != D3DFMT_X8R8G8B8)
	{
		throw m_pRegionSet->GetEngine().GetErrors().Push(
			Error::D3D_INVALIDSURFACEFORMAT, __FUNCTIONW__);
	}

	// Get surface data

	D3DLOCKED_RECT lock;
	hr = pSurf->LockRect(&lock, &rrcSrcRect, D3DLOCK_READONLY);

	if (FAILED(hr))
		throw m_pRegionSet->GetEngine().GetErrors().Push(
			Error::D3D_SURFACE_LOCKRECT, __FUNCTIONW__, hr);

	// Set size to source rect (also clears the data)

	SIZE psSize = { rrcSrcRect.right - rrcSrcRect.left,
		rrcSrcRect.bottom - rrcSrcRect.top };
	
	SetSize(psSize);

	// Loop through source rect pixels, and set corresponding region cells

	int nRectWidth = rrcSrcRect.right - rrcSrcRect.left;
	int nExtra = lock.Pitch / 4 - nRectWidth;

	DWORD* pdwCurPixel = (DWORD*)lock.pBits;
	BYTE* pCurCell = m_pData;
	
	for(int y = rrcSrcRect.top;
		y < rrcSrcRect.bottom;
		y++, pdwCurPixel += nExtra)
	{
		for(int x = rrcSrcRect.left;
			x < rrcSrcRect.right;
			x++, pdwCurPixel++, pCurCell++)
		{
			if (*pdwCurPixel >> 24)
			{
				// Opaque pixel

				*pCurCell = BYTE(1);
			}
		}
	}

	pSurf->UnlockRect();
}

bool Region::TestPoint(const POINT& rpt) const
{
	if (rpt.x < 0 || rpt.y < 0 ||
		rpt.x >= m_psSize.cx || rpt.y >= m_psSize.cy)
		return false;

	if (0 == m_ppData[rpt.y][rpt.x])
		return false;

	return true;
}

bool Region::TestRect(const RECT& rrc) const
{
	RECT rcTest = { rrc.left, rrc.top, rrc.right, rrc.bottom };

	// Clamp the rectangle to region bounds

	if (rcTest.left < 0)
		rcTest.left = 0;
	else if (rcTest.left >= m_psSize.cx)
		rcTest.left = m_psSize.cx - 1;

	if (rcTest.top < 0)
		rcTest.top = 0;
	else if (rcTest.top >= m_psSize.cy)
		rcTest.top = m_psSize.cy - 1;

	if (rcTest.right < 0)
		rcTest.right = 0;
	else if (rcTest.right >= m_psSize.cx)
		rcTest.right = m_psSize.cx - 1;

	if (rcTest.bottom < 0)
		rcTest.bottom = 0;
	else if (rcTest.bottom >= m_psSize.cy)
		rcTest.bottom = m_psSize.cy - 1;

	// For every cell that lies in rrcTest, see if it's non-zero
	// If yes, we flag collision

	for(int y = rcTest.top; y < (rcTest.bottom + 1); y++)
	{
		for(int x = rcTest.left; x < (rcTest.right + 1); x++)
		{
			if (m_ppData[y][x] != 0) return true;
		}
	}

	return false;
}

bool Region::TestRegion(const Region& rRegion, const POINT& rptOffset) const
{
	// Calculate other region's bounding rectangle
	
	RECT rcTest = { rptOffset.x, rptOffset.y,
		rptOffset.x + rRegion.GetSize().cx,
		rptOffset.y + rRegion.GetSize().cy };

	// Clamp it to this region's bounds

	if (rcTest.left < 0)
		rcTest.left = 0;
	else if (rcTest.left >= m_psSize.cx)
		rcTest.left = m_psSize.cx - 1;

	if (rcTest.top < 0)
		rcTest.top = 0;
	else if (rcTest.top >= m_psSize.cy)
		rcTest.top = m_psSize.cy - 1;

	if (rcTest.right < 0)
		rcTest.right = 0;
	else if (rcTest.right >= m_psSize.cx)
		rcTest.right = m_psSize.cx - 1;

	if (rcTest.bottom < 0)
		rcTest.bottom = 0;
	else if (rcTest.bottom >= m_psSize.cy)
		rcTest.bottom = m_psSize.cy - 1;

	// Make sure the other region's rect is not
	// out of bounds after clamping

	if ((rcTest.right - rcTest.left) > rRegion.GetSize().cx ||
	   (rcTest.bottom - rcTest.top) > rRegion.GetSize().cy) return false;

	// For every cell that lies in rrcTest in this region, see if it's non-zero
	// Also see if that region's corresponding cell is non-zero
	// If so, we flag a collision

	const BYTE** ppCheckWith = rRegion.GetData2DConst();

	for(int nThisY = rcTest.top, nThatY = 0;
		nThisY < (rcTest.bottom + 1);
		nThisY++, nThatY++)
	{
		for(int nThisX = rcTest.left, nThatX = 0;
			nThisX < (rcTest.right + 1);
			nThisX++, nThatX++)
		{
			if (m_ppData[nThisY][nThisX] != 0 &&
				ppCheckWith[nThatY][nThatX] != 0)
				return true;
		}
	}

	return false;
}

void Region::Serialize(Stream& rStream) const
{
	try
	{
		// Write size

		rStream.WriteVar((int*)&m_psSize, 2);

		// Write data

		rStream.WriteVar(m_pData, UINT(m_psSize.cx * m_psSize.cy));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_pRegionSet->GetEngine().GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Region::Deserialize(Stream& rStream)
{
	try
	{
		// Read size and resize

		SIZE psSize;

		rStream.ReadVar((int*)&psSize, 2);

		SetSize(psSize);

		// Read data

		rStream.ReadVar(m_pData, UINT(psSize.cx * psSize.cy));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_pRegionSet->GetEngine().GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

DWORD Region::GetMemoryFootprint(void) const
{
	return sizeof(Region) +
		m_psSize.cx *
		m_psSize.cy +
		m_psSize.cy *
		sizeof(BYTE*);
}

void Region::Empty(void)
{
	if (m_pData != NULL)
	{
		delete[] m_pData;
		m_pData = NULL;

		delete[] m_ppData;
		m_ppData = NULL;
	}

	m_psSize.cx = 0;
	m_psSize.cy = 0;
}

void Region::operator=(const Region& rAssign)
{
	m_pRegionSet = rAssign.m_pRegionSet;

	m_pData = rAssign.m_pData;

	m_ppData = rAssign.m_ppData;

	m_psSize.cx = rAssign.m_psSize.cx;
	m_psSize.cy = rAssign.m_psSize.cy;
}