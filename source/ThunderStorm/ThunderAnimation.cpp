/*------------------------------------------------------------------*\
|
| ThunderAnimation.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine animation class(es) implementation
| Created: 05/06/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderGlobals.h"		// using INVALID_INDEX
#include "ThunderAnimation.h"	// defining Animation
#include "ThunderEngine.h"		// using Engine
#include "ThunderStream.h"		// using Stream
#include "ThunderInfoFile.h"	// using InfoFile/Elem
#include "ThunderMaterial.h"	// using Texture
#include "ThunderSprite.h"		// using Sprite
#include "ThunderRegion.h"		// using Region/Set

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Animation::SZ_ANIMATION[]	= L"animation";
const WCHAR Animation::SZ_BASETEX[]		= L"basetexture";
const WCHAR Animation::SZ_FRAMESIZE[]	= L"framesize";
const WCHAR Animation::SZ_FRAMERATE[]	= L"framerate";
const WCHAR Animation::SZ_FRAME[]		= L"frame";
const WCHAR Animation::SZ_SEQUENCE[]	= L"sequence";
const WCHAR Animation::SZ_REGIONSET[]	= L"regionset";

const WCHAR Frame::SZ_DURATION[]		= L"duration";
const WCHAR Frame::SZ_DEF_DURATION[]	= L"defaultduration";
const WCHAR Frame::SZ_REGIONID[]		= L"regionid";
const WCHAR Frame::SZ_TEXTURECOORDS[]	= L"texturecoords";
const WCHAR Frame::SZ_META[]			= L"meta";


/*----------------------------------------------------------*\
| Animation implementation
\*----------------------------------------------------------*/

Animation::Animation(Engine& rEngine):
					 Resource(rEngine),
					 m_pOwner(NULL),
					 m_pTexture(NULL),
					 m_fDuration(0.0f)
{
	m_arFrames.reserve(16);
}

Animation::Animation(Sprite* pOwner):
					 Resource(pOwner->GetEngine()),
					 m_pOwner(pOwner),
					 m_pTexture(NULL),
					 m_fDuration(0.0f)
{
	m_arFrames.reserve(16);
}

Animation::~Animation(void)
{
	Empty();
}

void Animation::SetBaseTexture(Texture* pTexture)
{
	pTexture->AddRef();
	m_pTexture->Release();

	m_pTexture = pTexture;
}

void Animation::SetFrameSize(SIZE frameSize)
{
	CopyMemory(&m_frameSize, &frameSize, sizeof(SIZE));
	
	// Cache size converted to tiles

	m_vecSize.Set(
		float(frameSize.cx) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)),
		float(frameSize.cy) /
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)));
}

void Animation::SetSizeTiles(const Vector2& rvecSize)
{
	m_vecSize = rvecSize;

	// Cache size converted to pixels

	m_frameSize.cx = int(rvecSize.x *
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)));

	m_frameSize.cy = int(rvecSize.y *
		float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE)));
}

int Animation::AddSequence(const Sequence& rSequence)
{
	m_arSequences.push_back(rSequence);

	m_mapSequencesByName[rSequence.GetName()] =
		int(m_arSequences.size() - 1);

	return int(m_arSequences.size() - 1);
}

void Animation::RemoveSequence(LPCWSTR pszSequence)
{
	// Validate

	StringIntMapIterator posFind =
		m_mapSequencesByName.find(pszSequence);

	if (m_mapSequencesByName.end() == posFind)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	// Reset all references to this sequence

	for(SequenceArrayIterator pos = m_arSequences.begin();
		pos != m_arSequences.end();
		pos++)
	{
		if (pos->GetNextSequenceID() == posFind->second)
			pos->SetNextSequenceID(INVALID_INDEX);
	}

	// Erase from array

	m_arSequences.erase(m_arSequences.begin() + posFind->second);

	// Erase from names

	m_mapSequencesByName.erase(posFind);
}

void Animation::RemoveSequence(int nSequenceID)
{
	// Validate
	
	if (nSequenceID < 0 || nSequenceID >= int(m_arSequences.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	// Reset all references to this sequence

	for(SequenceArrayIterator pos = m_arSequences.begin();
		pos != m_arSequences.end();
		pos++)
	{
		if (pos->GetNextSequenceID() == nSequenceID)
			pos->SetNextSequenceID(INVALID_INDEX);
	}

	// Erase from names

	m_mapSequencesByName.erase(m_arSequences[nSequenceID].GetName());

	// Erase from array

	m_arSequences.erase(m_arSequences.begin() + nSequenceID);
}

int Animation::GetSequenceID(LPCWSTR pszSequence) const
{
	StringIntMapConstIterator pos =
		m_mapSequencesByName.find(pszSequence);

	if (pos != m_mapSequencesByName.end())
		return pos->second;
	else
		return INVALID_INDEX;
}

int Animation::AddFrame(const Frame& rFrame)
{
	int nIndex = int(m_arFrames.size());

	m_arFrames.push_back(rFrame);

	m_fDuration += rFrame.GetDuration();

	return nIndex;
}

void Animation::RemoveFrame(int nFrameID)
{
	if (nFrameID < 0 || nFrameID >= int(m_arFrames.size()))
		return;

	m_fDuration -= m_arFrames[nFrameID].GetDuration();

	m_arFrames.erase(m_arFrames.begin() + nFrameID);
}

void Animation::RemoveAllFrames(void)
{
	m_arFrames.clear();

	m_fDuration = 0.0f;
}

void Animation::Serialize(LPCWSTR pszPath) const
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

void Animation::Deserialize(LPCWSTR pszPath)
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

	PUSH_CURRENT_DIRECTORY(pszPath);

	try
	{
		Deserialize(stream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::FILE_READ,
			__FUNCTIONW__, pszPath);
	}

	POP_CURRENT_DIRECTORY();
}

void Animation::Serialize(Stream& rStream) const
{
	InfoFile doc(&m_rEngine.GetErrors());

	try
	{
		Serialize(*doc.CreateSetRoot(SZ_ANIMATION));

		doc.Serialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_WRITE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Animation::Deserialize(Stream& rStream)
{
	InfoFile doc(&m_rEngine.GetErrors());

	try
	{
		doc.Deserialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN,
			__FUNCTIONW__, rStream.GetPath());
	}

	if (doc.GetRoot() == NULL || doc.GetRoot()->GetName() != SZ_ANIMATION)
	{
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());
	}

	try
	{
		Deserialize(*doc.GetRoot());
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Animation::Deserialize(const InfoElem& rRoot)
{
	Empty();

	const InfoElem* pElem = NULL;

	// Read base texture

	pElem = rRoot.FindChildConst(SZ_BASETEX,
		InfoElem::TYPE_VALUE, Variable::TYPE_STRING);

	if (NULL == pElem)
		throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());

	try
	{
		m_pTexture = m_rEngine.GetTextures().LoadInstance(*pElem);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pTexture");
	}		

	m_pTexture->AddRef();

	// Read region set

	/*

	pElem = rRoot.FindChildConst(SZ_REGIONSET, InfoElem::TYPE_VALUE, Variable::TYPE_STRING);
	
	if (pElem)
	{
		try
		{
			m_pRegionSet = m_rEngine.GetRegions().Load(pElem->GetStringValue());
		}
		
		catch(Error& rError)
		{
			UNREFERENCED_PARAMETER(rError);

			throw m_rEngine.GetErrors().Push(Error::INVALID_PTR, __FUNCTIONW__, L"m_pRegionSet");
		}
	}
	*/

	if (m_pOwner != NULL)
	{
		// Copy frame size from owner

		m_frameSize.cx = m_pOwner->GetSize().cx;
		m_frameSize.cy = m_pOwner->GetSize().cy;
	}
	else
	{
		// Read frame size

		pElem = rRoot.FindChildConst(SZ_FRAMESIZE, InfoElem::TYPE_VALUELIST);

		if (NULL == pElem)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT, __FUNCTIONW__, rRoot.GetDocumentConst().GetPath(), SZ_FRAMESIZE);

		pElem->ToIntArray(reinterpret_cast<int*>(&m_frameSize), 2);
	}
	

	// Read default frame duration

	float fDefaultFrameDuration = 0.0f;

	pElem = rRoot.FindChildConst(Frame::SZ_DEF_DURATION, InfoElem::TYPE_VALUE);

	if (pElem != NULL)
	{
		if (pElem->GetVarType() == InfoElem::TYPE_FLOAT)
			fDefaultFrameDuration = pElem->GetFloatValue();
		else if (pElem->GetVarType() == InfoElem::TYPE_INT)
			fDefaultFrameDuration = float(pElem->GetIntValue());
	}

	// Read frame(s)

	InfoElemConstRange range = rRoot.FindChildrenConst(SZ_FRAME);

	m_fDuration = 0.0f;

	try
	{
		size_t nFrame = 0;

		for(InfoElemConstRangeIterator pos = range.first;
			pos != range.second;
			pos++, nFrame++)
		{
			m_arFrames.resize(nFrame + 1, Frame());

			m_arFrames[nFrame].Deserialize(*pos->second);

			if (m_arFrames[nFrame].GetDuration() < 0.0f)
				m_arFrames[nFrame].SetDuration(fDefaultFrameDuration);

			m_fDuration += m_arFrames[nFrame].GetDuration();
		}
	}

	catch(Error& rError)
	{
		throw m_rEngine.GetErrors().Push(rError);
	}

	// Read sequence(s)

	range = rRoot.FindChildrenConst(SZ_SEQUENCE);

	try
	{
		size_t nSequence = 0;

		StringArray arNextNames;

		for(InfoElemConstRangeIterator pos = range.first;
			pos != range.second;
			pos++, nSequence++)
		{
			// Load sequence

			m_arSequences.resize(nSequence + 1, Sequence());
			m_arSequences[nSequence].Deserialize(*pos->second);

			// Save the name temporarily in the map

			m_mapSequencesByName[m_arSequences[nSequence].GetName()] = nSequence;

			// Remember the next sequence for this tem

			if (pos->second->GetChildCount() > 3 &&
			   pos->second->GetChildConst(3)->GetVarType() == Variable::TYPE_STRING)
				arNextNames.push_back(pos->second->GetChildConst(3)->GetString());
			else
				arNextNames.push_back(String());
		}

		// Cache pointers to next sequence

		StringArrayIterator posString = arNextNames.begin();

		for(SequenceArrayIterator pos = m_arSequences.begin();
			pos != m_arSequences.end();
			pos++, posString++)
		{
			if (posString->IsEmpty() == false)
				pos->SetNextSequenceID(m_mapSequencesByName[*posString]);
		}
	}

	catch(Error& rError)
	{
		m_rEngine.GetErrors().Push(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE, __FUNCTIONW__,
			rRoot.GetDocumentConst().GetPath());
	}
}

void Animation::Serialize(InfoElem& rRoot) const
{
	try
	{
		// Texture path

		if (m_pTexture != NULL)
		{
			m_pTexture->SerializeInstance(
				*rRoot.AddChild(SZ_BASETEX, InfoElem::TYPE_VALUE,
					Variable::TYPE_STRING));
		}

		// Region file

		/*
		if (m_pRegionSet != NULL)
			rRoot.AddChild(SZ_REGIONSET,
						   InfoElem::TYPE_VALUE,
						   Variable::TYPE_STRING)->SetStringValue(m_pRegionSet->GetName());
						   */

		// Frame size

		rRoot.AddChild(SZ_FRAMESIZE, InfoElem::TYPE_VALUELIST,
			Variable::TYPE_STRING)->FromIntArray(
				reinterpret_cast<const int*>(&m_frameSize), 2);		

		try
		{
			// Sequences

			for(SequenceArrayConstIterator pos = m_arSequences.begin();
				pos != m_arSequences.end();
				pos++)
			{
				InfoElem& rElem = *rRoot.AddChild();

				(*pos).Serialize(rElem);

				if (pos->GetNextSequenceID() != INVALID_INDEX)
					rElem.AddChild()->SetStringValue(
						m_arSequences[pos->GetNextSequenceID()].GetName());
			}

			// Frames

			for(FrameArrayConstIterator pos = m_arFrames.begin();
				pos != m_arFrames.end();
				pos++)
			{
				(*pos).Serialize(*rRoot.AddChild(SZ_FRAME,
					InfoElem::TYPE_BLOCK));
			}
		}

		catch(Error& rError)
		{
			throw m_rEngine.GetErrors().Push(rError);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE, __FUNCTIONW__,
			rRoot.GetDocumentConst().GetPath());
	}
}

DWORD Animation::GetMemoryFootprint(void) const
{
	return Resource::GetMemoryFootprint() -
		sizeof(Resource) +
		sizeof(Animation) +
		m_arFrames.size() * sizeof(Frame);
}

void Animation::Empty(void)
{
	RemoveAllFrames();
}

void Animation::Remove(void)
{
	m_rEngine.GetAnimations().Remove(this);
}

/*----------------------------------------------------------*\
| Sequence implementation
\*----------------------------------------------------------*/

Sequence::Sequence(void): m_nFirstFrameID(0),
						  m_nLastFrameID(0),
						  m_nNextSequenceID(INVALID_INDEX)
{
}

void Sequence::Deserialize(const InfoElem& rRoot)
{
	// Validate

	if (rRoot.GetChildCount() < 3)
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	// Read name

	if (rRoot.GetChildConst(0)->GetVarType() != Variable::TYPE_STRING)
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	m_strName = rRoot.GetChildConst(0)->GetStringValue();

	// Read first frame

	if (rRoot.GetChildConst(1)->GetVarType() != Variable::TYPE_INT)
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	m_nFirstFrameID = rRoot.GetChildConst(1)->GetIntValue();

	// Read last frame

	if (rRoot.GetChildConst(2)->GetVarType() != Variable::TYPE_INT)
		throw Error(Error::FILE_ELEMENTFORMAT, __FUNCTIONW__,
			rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

	m_nLastFrameID = rRoot.GetChildConst(2)->GetIntValue();

	// Next sequence ID will be cached by parent from string
}

void Sequence::Serialize(InfoElem& rRoot) const
{
	// Write name

	rRoot.SetElemType(InfoElem::TYPE_VALUELIST);

	rRoot.AddChild()->SetStringValue(m_strName);

	// Write first frame

	rRoot.AddChild()->SetIntValue(m_nFirstFrameID);

	// Write last frame

	rRoot.AddChild()->SetIntValue(m_nLastFrameID);

	// Next sequence will be written as string by parent
}

/*----------------------------------------------------------*\
| Frame implementation
\*----------------------------------------------------------*/

Frame::Frame(void): m_fDuration(0.5f),
					m_nRegionID(INVALID_INDEX)
{
}

const Variable* Frame::GetVariable(LPCWSTR pszName) const
{
	VariableMapConstIterator posFind = m_mapVariables.find(pszName);

	if (m_mapVariables.end() == posFind)
		return NULL;

	return posFind->second;
}

void Frame::Deserialize(const InfoElem& rRoot)
{
	// Read duration

	const InfoElem* pElem = rRoot.FindChildConst(SZ_DURATION, InfoElem::TYPE_VALUE);

	if (pElem != NULL)
	{
		if (pElem->GetVarType() == Variable::TYPE_FLOAT)
		{
			m_fDuration = pElem->GetFloatValue();
		}
		else if (pElem->GetVarType() == Variable::TYPE_INT)
		{
			m_fDuration = float(pElem->GetIntValue());
		}
		else
		{
			m_fDuration = -1.0f;
		}
	}
	else
	{
		m_fDuration = -1.0f;
	}

	// Read region ID (none by default)

	pElem = rRoot.FindChildConst(SZ_REGIONID,
		InfoElem::TYPE_VALUE,Variable::TYPE_INT);

	if (pElem != NULL)
		m_nRegionID = pElem->GetIntValue();

	// Read texture coordinates

	pElem = rRoot.FindChildConst(SZ_TEXTURECOORDS,
		InfoElem::TYPE_VALUELIST);

	if (NULL == pElem)
		throw Error(Error::FILE_ELEMENT, __FUNCTIONW__,
			SZ_TEXTURECOORDS, rRoot.GetDocumentConst().GetPath());

	pElem->ToIntArray(reinterpret_cast<int*>(&m_ptTextureCoords), 2);

	// Read meta data

	pElem = rRoot.FindChildConst(SZ_META, InfoElem::TYPE_BLOCK);

	if (pElem != NULL)
	{
		try
		{
			for(InfoElemConstIterator pos = pElem->GetBeginChildPosConst();
				pos != pElem->GetEndChildPosConst();
				pos++)
			{
				m_mapVariables[(*pos)->GetName()] =
					new Variable((*pos)->GetValue());
			}
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Variable));
		}
	}
}

void Frame::Serialize(InfoElem& rRoot) const
{
	// Write duration

	rRoot.AddChild(SZ_DURATION)->SetFloatValue(m_fDuration);

	// Write region ID

	if (m_nRegionID != INVALID_INDEX)
		rRoot.AddChild(SZ_REGIONID)->SetIntValue(m_nRegionID);

	// Write texture coordinates

	rRoot.AddChild(SZ_TEXTURECOORDS,
		InfoElem::TYPE_VALUEBLOCK)->FromIntArray(
		reinterpret_cast<const int*>(&m_ptTextureCoords), 2);

	// Write meta data

	if (m_mapVariables.empty() == false)
	{
		InfoElem* pElem = rRoot.AddChild(SZ_META, InfoElem::TYPE_BLOCK);

		for(VariableMapConstIterator pos = m_mapVariables.begin();
			pos != m_mapVariables.end();
			pos++)
		{
			pos->second->Serialize(*pElem->AddChild(pos->first));
		}
	}
}

void Frame::Empty(void)
{
	m_nRegionID = INVALID_INDEX;
	m_fDuration = 0.5f;

	ZeroMemory(&m_ptTextureCoords, sizeof(POINT));

	m_mapVariables.clear();
}

void Frame::operator=(const Frame& rAssign)
{
	m_nRegionID = rAssign.m_nRegionID;
	m_fDuration = 0.5f;

	CopyMemory(&m_ptTextureCoords, &rAssign.m_ptTextureCoords, sizeof(POINT));

	m_mapVariables.clear();

	std::copy(rAssign.m_mapVariables.begin(), rAssign.m_mapVariables.end(),
		std::inserter(m_mapVariables, m_mapVariables.end()));
}