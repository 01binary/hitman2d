/*------------------------------------------------------------------*\
|
| ThunderSprite.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine sprite class(es) implementation
| Created: 04/19/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"					// precompiled header
#include "ThunderEngine.h"			// using Engine
#include "ThunderSprite.h"			// defining Sprite
#include "ThunderTexture.h"			// using Texture
#include "ThunderAnimation.h"		// using Animation
#include "ThunderStream.h"			// using Stream
#include "ThunderInfoFile.h"		// using InfoFile

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Sprite::SZ_SPRITE[]		= L"sprite";
const WCHAR Sprite::SZ_DEFANIM[]	= L"defanimid";
const WCHAR Sprite::SZ_TEXTURE[]	= L"texture";
const WCHAR Sprite::SZ_SIZE[]		= L"size";
const WCHAR Sprite::SZ_PIVOT[]		= L"pivot";
const WCHAR Sprite::SZ_ANIMATION[]	= L"animation";


/*----------------------------------------------------------*\
| Sprite implementation
\*----------------------------------------------------------*/

Sprite::Sprite(Engine& m_rEngine):	Resource(m_rEngine),
									m_nDefaultAnimationID(INVALID_INDEX)
{
	m_psSize.cx = m_psSize.cy =
		m_rEngine.GetOption(Engine::OPTION_TILE_SIZE);

	m_ptCenter.x = m_ptCenter.y =
		m_rEngine.GetOption(Engine::OPTION_TILE_SIZE) / 2;

	m_ptPivot.x = m_ptPivot.y = 0;
}

Sprite::~Sprite(void)
{
	Empty();
}

SIZE Sprite::GetSize(void) const
{
	return m_psSize;
}

void Sprite::SetSize(SIZE psSize)
{
	m_psSize.cx = psSize.cx;
	m_psSize.cy = psSize.cy;

	m_ptCenter.x = psSize.cx / 2;
	m_ptCenter.y = psSize.cy / 2;

	m_vecCenter.x = float(m_ptCenter.x) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecCenter.y = float(m_ptCenter.y) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecSize.x = float(psSize.cx) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecSize.y = float(psSize.cy) / float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	float fHalfWidth = m_vecSize.x / 2.0f;
	float fHalfHeight = m_vecSize.y / 2.0f;

	m_fRadius = sqrtf(fHalfWidth * fHalfWidth + fHalfHeight * fHalfHeight);

	// Set this size to all animations

	for(AnimationArrayIterator pos = m_arAnimations.begin();
		pos != m_arAnimations.end();
		pos++)
	{
		(*pos)->SetFrameSize(psSize);
	}
}

const Vector2& Sprite::GetSizeInTiles(void) const
{
	return m_vecSize;
}

void Sprite::SetSizeInTiles(const Vector2& rvecSize)
{
	m_vecSize = rvecSize;

	m_psSize.cx = int(m_vecSize.x *
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)));

	m_psSize.cy = int(m_vecSize.y *
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)));
}

float Sprite::GetRadius(void) const
{
	return m_fRadius;
}

const POINT& Sprite::GetCenter(void) const
{
	return m_ptCenter;
}

const Vector2& Sprite::GetCenterInTiles(void) const
{
	return m_vecCenter;
}

const POINT& Sprite::GetPivot(void) const
{
	return m_ptPivot;
}

void Sprite::SetPivot(const POINT& rptPivot)
{
	m_ptPivot.x = rptPivot.x;
	m_ptPivot.y = rptPivot.y;

	m_vecPivot.x = float(m_ptPivot.x) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecPivot.y = float(m_ptPivot.y) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));
}

const Vector2& Sprite::GetPivotInTiles(void) const
{
	return m_vecPivot;
}

int Sprite::GetDefaultAnimation(void)
{
	return m_nDefaultAnimationID;
}

void Sprite::SetDefaultAnimation(int nAnimationID)
{
	m_nDefaultAnimationID = nAnimationID;
}

Animation* Sprite::CreateAnimation(void)
{
	try
	{
		return (new Animation(this));
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Animation));
	}
}

int Sprite::AddAnimation(Animation* pAnimation)
{
	m_arAnimations.push_back(pAnimation);

	int nAnimationID = int(m_arAnimations.size() - 1);

	pAnimation->AddRef();

	return nAnimationID;
}

int Sprite::GetAnimationCount(void)
{
	return int(m_arAnimations.size());
}

int Sprite::FindAnimation(LPCWSTR pszName)
{
	for(AnimationArrayConstIterator pos = m_arAnimations.begin();
		pos != m_arAnimations.end();
		pos++)
	{
		if ((*pos)->GetName() == pszName)
			return int(pos - m_arAnimations.begin());
	}

	return INVALID_INDEX;
}

Animation* Sprite::GetAnimation(int nAnimationID)
{
	_ASSERT(nAnimationID >= 0 || nAnimationID < int(m_arAnimations.size()));

	return m_arAnimations[nAnimationID];
}

void Sprite::RemoveAnimation(int nAnimationID)
{
	_ASSERT(nAnimationID >= 0 || nAnimationID < int(m_arAnimations.size()));

	delete m_arAnimations[nAnimationID];

	m_arAnimations.erase(m_arAnimations.begin() + nAnimationID);
}

void Sprite::RemoveAllAnimations(void)
{
	for(std::vector<Animation*>::iterator pos = m_arAnimations.begin();
		pos != m_arAnimations.end();
		pos++)
	{
		delete *pos;
	}

	m_arAnimations.clear();
}

void Sprite::Serialize(LPCWSTR pszPath) const
{
	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(pszPath, GENERIC_WRITE, CREATE_ALWAYS);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	Serialize(stream);
}

void Sprite::Deserialize(LPCWSTR pszPath)
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

	// Set current directory to sprite directory

	WCHAR szCurDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH, szCurDir);

	WCHAR szSpriteDir[MAX_PATH] = {0};
	GetAbsolutePath(pszPath, szSpriteDir);
	PathRemoveFileSpec(szSpriteDir);

	SetCurrentDirectory(szSpriteDir);

	try
	{
		Deserialize(stream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, pszPath);
	}

	// Restore current directory

	SetCurrentDirectory(szCurDir);
}

void Sprite::Serialize(Stream& rStream) const
{
	try
	{
		InfoFile doc(&m_rEngine.GetErrors());

		doc.CreateSetRoot(SZ_SPRITE);

		InfoElem* pElem = doc.GetRoot();

		doc.GetRoot()->AddChild(SZ_SIZE)->
			FromIntArray((int*)&m_psSize, 2);

		doc.GetRoot()->AddChild(SZ_PIVOT)->
			FromIntArray((int*)&m_ptPivot, 2);

		doc.GetRoot()->AddChild(SZ_DEFANIM)->
			SetIntValue(m_nDefaultAnimationID);

		for(AnimationArrayConstIterator posAnims = m_arAnimations.begin();
			posAnims != m_arAnimations.end();
			posAnims++)
		{
			pElem = doc.GetRoot()->AddChild(SZ_ANIMATION,
				InfoElem::TYPE_VALUEBLOCK);

			pElem->SetStringValue((*posAnims)->GetName());
			
			(*posAnims)->Serialize(*pElem);
		}

		doc.Serialize(rStream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Sprite::Deserialize(Stream& rStream)
{
	Empty();

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

	if (doc.GetRoot() == NULL || doc.GetRoot()->GetName() != SZ_SPRITE)
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());

	// Read size (not optional)

	const InfoElem* pElem =
		doc.GetRoot()->FindChildConst(SZ_SIZE, InfoElem::TYPE_VALUELIST);

	if (pElem != NULL)
	{
		if (pElem->ToIntArray(reinterpret_cast<int*>(&m_psSize), 2) != 2)
			throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
				__FUNCTIONW__, rStream.GetPath());
	}
	else
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());
	}

	m_ptCenter.x = m_psSize.cx / 2;
	m_ptCenter.y = m_psSize.cy / 2;

	m_vecCenter.x = float(m_ptCenter.x) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecCenter.y = float(m_ptCenter.y) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecSize.x = float(m_psSize.cx) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	m_vecSize.y = float(m_psSize.cy) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	float fHalfWidth = m_vecSize.x / 2.0f;
	float fHalfHeight = m_vecSize.y / 2.0f;

	m_fRadius = sqrtf(fHalfWidth * fHalfWidth + fHalfHeight * fHalfHeight);

	// Read pivot (optional)

	pElem = doc.GetRoot()->FindChildConst(SZ_PIVOT,
		InfoElem::TYPE_VALUELIST);

	if (pElem != NULL)
	{
		if (pElem->ToIntArray(reinterpret_cast<int*>(&m_ptPivot), 2) != 2)
			throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
				__FUNCTIONW__, rStream.GetPath());

		m_vecPivot.x = float(m_ptPivot.x) /
			float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

		m_vecPivot.y = float(m_ptPivot.y) /
			float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));		
	}
	else
	{
		// Use center point as pivot point

		CopyMemory(&m_ptPivot, &m_ptCenter, sizeof(POINT));
		CopyMemory(&m_vecPivot, &m_vecCenter, sizeof(Vector2));
	}

	try
	{
		// Read embedded animations (at least 1 must be present)

		InfoElemConstRange range =
			doc.GetRoot()->FindChildrenConst(SZ_ANIMATION);

		for(InfoElemConstRangeIterator pos = range.first;
			pos != range.second;
			pos++)
		{
			// Allocate and read animation

			Animation* pAnim = CreateAnimation();
			pAnim->Deserialize(*pos->second);

			// Set name for the animation of named

			if (pos->second->GetElemType() == InfoElem::TYPE_VALUEBLOCK)
				pAnim->SetName(pos->second->GetName());

			// Set size for the animation

			pAnim->SetFrameSize(m_psSize);

			// Add to list of animations

			AddAnimation(pAnim);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
	
	// Must have at least one animation

	if (m_arAnimations.size() < 1)
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
		__FUNCTIONW__, rStream.GetPath());

	// Read default animation (0 by default)

	pElem = doc.GetRoot()->FindChildConst(SZ_DEFANIM,
		InfoElem::TYPE_VALUE);

	if (pElem != NULL)
	{
		if (pElem->GetVarType() == Variable::TYPE_INT)
		{
			// Default animation ID

			m_nDefaultAnimationID = pElem->GetIntValue();
		}
		else if (pElem->GetVarType() == Variable::TYPE_STRING)
		{
			// Default animation name

			for(int n = 0; n < int(m_arAnimations.size()); n++)
			{
				if (m_arAnimations[n]->GetName() == pElem->GetStringValue())
				{
					m_nDefaultAnimationID = n;
					break;
				}
			}

			if (m_nDefaultAnimationID == int(m_arAnimations.size()))
			{
				throw m_rEngine.GetErrors().Push(Error::INVALID_INDEX,
					__FUNCTIONW__, L"m_nDefaultAnimationID");
			}
		}
	}
}

DWORD Sprite::GetMemoryFootprint(void) const
{
	DWORD dwSize = Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(Sprite) +
		m_arAnimations.size() * sizeof(DWORD);

	for(AnimationArrayConstIterator pos = m_arAnimations.begin();
		pos != m_arAnimations.end();
		pos++)
	{
		dwSize += (*pos)->GetMemoryFootprint();
	}

	return dwSize;
}

void Sprite::Empty(void)
{
	RemoveAllAnimations();
}

void Sprite::Remove(void)
{
	m_rEngine.GetSprites().Remove(this);
}