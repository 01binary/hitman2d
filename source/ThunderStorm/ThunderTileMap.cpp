/*------------------------------------------------------------------*\
|
| ThunderTileMap.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine map class(es) implementation
| Created: 04/09/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine
#include "ThunderSound.h"		// using Sound
#include "ThunderMusic.h"		// using Music
#include "ThunderStream.h"		// using Stream
#include "ThunderClient.h"		// using Client
#include "ThunderRegion.h"		// using Region
#include <algorithm>			// using std::sort, std::lower_bound, std::unique

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const BYTE TileMap::SIGNATURE[4]		= "THM";
const BYTE TileMap::FORMAT_VERSION[4]	= {2, 2, 0, 0};

const int TileMap::RESERVE_LAYERS				= 4;
const int TileMap::RESERVE_MATERIALS			= 16;
const int TileMap::RESERVE_ANIMATIONS			= 8;
const int TileMap::RESERVE_SOUNDS				= 8;
const int TileMap::RESERVE_MUSIC				= 4;
const int TileMap::RESERVE_CAMERAS				= 4;
const int TileMap::RESERVE_ACTIVECAMERAS		= 2;


/*----------------------------------------------------------*\
| TileMap implementation
\*----------------------------------------------------------*/

TileMap::TileMap(Engine& rEngine, LPCWSTR pszClass):
				 Object(rEngine),

				 m_strClass(pszClass),

				 m_pPlayer(NULL),

				 m_nBackMaterialID(INVALID_INDEX),

				 m_nBackAnimationID(INVALID_INDEX),

				 m_clrBackBlend(0xFFFFFFFF),

				 m_Variables(&rEngine.GetErrors())
{
	// Set default flags

	SetFlags(RENDER | UPDATE);

	// Set grow sizes for map arrays

	m_arLayers.reserve(RESERVE_LAYERS);

	m_arMaterials.reserve(RESERVE_MATERIALS);
	m_arAnimations.reserve(RESERVE_ANIMATIONS);
	m_arSounds.reserve(RESERVE_SOUNDS);
	m_arMusic.reserve(RESERVE_MUSIC);

	m_arCameras.reserve(RESERVE_CAMERAS);
	m_arActiveCameras.reserve(RESERVE_ACTIVECAMERAS);

	// Add default layer

	InsertLayer(CreateLayer());

	// Add default camera

	Camera* pDefCamera = CreateCamera();

	float fTileSize = float(m_rEngine.GetOption(Engine::OPTION_TILE_SIZE));

	pDefCamera->SetPosition(0.0f, 0.0f);

	pDefCamera->SetSize(
		floor(float(m_rEngine.GetGraphics().GetDeviceParams().BackBufferWidth) /
			fTileSize),
		floor(float(m_rEngine.GetGraphics().GetDeviceParams().BackBufferHeight) /
			fTileSize));

	AddCamera(pDefCamera);
}

TileMap::~TileMap(void)
{
	Empty();
}

const String& TileMap::GetClass(void) const
{
	return m_strClass;
}

TileStatic& TileMap::GetTileTemplateStatic(int nIndex)
{
	if (nIndex >= 0 && nIndex < int(m_arTilesStatic.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arTilesStatic[nIndex];
}

const TileStatic& TileMap::GetTileTemplateStaticConst(int nIndex) const
{
	if (nIndex >= 0 && nIndex < int(m_arTilesStatic.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arTilesStatic[nIndex];
}

int TileMap::GetTileTemplateStaticCount(void) const
{
	return int(m_arTilesStatic.size());
}

TileAnimated& TileMap::GetTileTemplateAnimated(int nIndex)
{
	if (nIndex >= 0 && nIndex < int(m_arTilesAnimated.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arTilesAnimated[nIndex];
}

const TileAnimated& TileMap::GetTileTemplateAnimatedConst(int nIndex) const
{
	if (nIndex >= 0 && nIndex < int(m_arTilesAnimated.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arTilesAnimated[nIndex];
}

int TileMap::GetTileTemplateAnimatedCount(void) const
{
	return int(m_arTilesAnimated.size());
}

TileStatic* TileMap::SetTileTemplateStatic(int nIndex,
										   DWORD dwFlags,
										   D3DCOLOR clrBlend,
										   int nMaterialID,
										   POINT ptTextureCoords)
{
	// Create requested template on the stack

	TileStatic tile;

	tile.SetFlags(dwFlags);
	tile.SetBlend(clrBlend);
	tile.SetMaterialID(*this, nMaterialID);

	int nTileSize = m_rEngine.GetOption(Engine::OPTION_TILE_SIZE);

	tile.GetMaterialInstance().SetTextureCoords(Rect(
		ptTextureCoords.x,
		ptTextureCoords.y,
		ptTextureCoords.x + nTileSize,
		ptTextureCoords.y + nTileSize));

	// Add or replace

	return SetTileTemplateStatic(nIndex, tile);
}

TileStatic* TileMap::SetTileTemplateStatic(int nIndex,
										   const TileStatic& rTemplate)
{
	// Add or replace

	if (INVALID_INDEX == nIndex || m_arTilesStatic[nIndex].GetRefCount() > 1)
	{
		if (IsFlagSet(OPTIMIZE) == true)
		{
			// Adding template - check if exists and
			// mark any empty places found

			TileStaticArrayIterator posEmpty = m_arTilesStatic.end();

			for(TileStaticArrayIterator pos = m_arTilesStatic.begin();
				pos != m_arTilesStatic.end();
				pos++)
			{
				if (pos->GetRefCount() == 0)
					posEmpty = pos;
				else if ((*pos) == rTemplate)
					return &(*pos);
			}

			if (posEmpty != m_arTilesStatic.end())
			{
				// Found an empty place - add there

				*posEmpty = rTemplate;
				return &(*posEmpty);
			}
		}

		// Empty places not found (or no progressive optimization)
		// Add to end

		m_arTilesStatic.push_back(rTemplate);
		return &m_arTilesStatic.back();
	}

	// Check for index out of bounds

	if (nIndex < 0 || nIndex >= int(m_arTilesStatic.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_INDEX,
			__FUNCTIONW__, L"nIndex");

	// If replacing with the same one, return

	if (m_arTilesStatic[nIndex] == rTemplate)
		return &m_arTilesStatic[nIndex];

	// Replace template at specificed position

	m_arTilesStatic[nIndex] = rTemplate;
	return &m_arTilesStatic[nIndex];
}

TileAnimated* TileMap::SetTileTemplateAnimated(int nIndex,
											   int nAnimationID,
											   DWORD dwFlags,
											   D3DCOLOR clrBlend,
											   bool bLoop,
											   int nStartFrame,
											   bool bPlay,
											   bool bReverse,
											   float fSpeed)
{
	// Create requested template on the stack

	TileAnimated tile;

	tile.SetFlags(dwFlags);
	tile.SetBlend(clrBlend);
	tile.SetAnimationID(*this, nAnimationID);

	tile.GetMaterialInstance().Play(m_rEngine.GetTime(),
									bLoop,
									nStartFrame,
									bReverse);

	if (false == bPlay)
		tile.GetMaterialInstance().Stop();

	// Add or replace
	
	return SetTileTemplateAnimated(nIndex, tile);
}

TileAnimated* TileMap::SetTileTemplateAnimated(int nIndex,
											   const TileAnimated& rTemplate)
{
	// Add or replace

	if (INVALID_INDEX == nIndex || m_arTilesAnimated[nIndex].GetRefCount() > 1)
	{
		if (IsFlagSet(OPTIMIZE) == true)
		{
			// Adding template - check if exists and mark any empty places found

			TileAnimatedArrayIterator posEmpty = m_arTilesAnimated.end();

			for(TileAnimatedArrayIterator pos = m_arTilesAnimated.begin();
				pos != m_arTilesAnimated.end();
				pos++)
			{
				if (pos->GetRefCount() == 0)
					posEmpty = pos;
				else if ((*pos) == rTemplate)
					return &(*pos);
			}

			if (posEmpty != m_arTilesAnimated.end())
			{
				// Found an empty place - add there

				*posEmpty = rTemplate;
				return &(*posEmpty);
			}
		}

		// Empty places not found (or no progressive optimization)
		// Add to end

		m_arTilesAnimated.push_back(rTemplate);
		return &m_arTilesAnimated.back();
	}

	// Check for index out of bounds

	if (nIndex < 0 || nIndex >= int(m_arTilesAnimated.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_INDEX,
			__FUNCTIONW__, L"nIndex");

	// If replacing with the same one, return

	if (m_arTilesAnimated[nIndex] == rTemplate)
		return &m_arTilesAnimated[nIndex];

	// Replace template at specified position

	m_arTilesAnimated[nIndex] = rTemplate;
	return &m_arTilesAnimated[nIndex];
}

void TileMap::Optimize(void)
{
	// TODO
	// Consolidate duplicates and collapse empty spots
}

void TileMap::RemoveAllTileTemplates(void)
{
	m_arTilesStatic.clear();

	m_arTilesAnimated.clear();
}

TileLayer* TileMap::CreateLayer(void)
{
	return new TileLayer(*this);
}

int TileMap::InsertLayer(TileLayer* pLayer, int nIndex)
{
	if (nIndex != INVALID_INDEX)
	{
		if (nIndex < 0 || nIndex >= int(m_arLayers.size()))
			throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
				__FUNCTIONW__, 1);

		m_arLayers.insert(m_arLayers.begin() + nIndex, pLayer);

		return nIndex;
	}

	m_arLayers.push_back(pLayer);

	return int(m_arLayers.size());
}

void TileMap::RemoveLayer(int nIndex)
{
	if (nIndex < 0 || nIndex >= int(m_arLayers.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	delete m_arLayers[nIndex];

	m_arLayers.erase(m_arLayers.begin() + nIndex);
}

void TileMap::RemoveAllLayers(void)
{
	for(TileLayerArrayIterator pos = m_arLayers.begin();
		pos != m_arLayers.end();
		pos++)
	{
		delete *pos;
	}

	m_arLayers.clear();
}

int TileMap::GetLayerCount(void) const
{
	return int(m_arLayers.size());
}

TileLayer* TileMap::GetLayer(int nIndex)
{
	if (nIndex < 0 || nIndex >= int(m_arLayers.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arLayers[nIndex];
}

const TileLayer* TileMap::GetLayerConst(int nIndex) const
{
	if (nIndex < 0 || nIndex >= int(m_arLayers.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arLayers[nIndex];
}

Color& TileMap::GetBackgroundColor(void)
{
	return m_clrBackColor;
}

const Color& TileMap::GetBackgroundColorConst(void) const
{
	return m_clrBackColor;
}

void TileMap::SetBackgroundColor(D3DCOLOR clrBackColor)
{
	m_clrBackColor = clrBackColor;
}

Color& TileMap::GetBackgroundBlend(void)
{
	return m_clrBackBlend;
}

const Color& TileMap::GetBackgroundBlendConst(void) const
{
	return m_clrBackBlend;
}

void TileMap::SetBackgroundBlend(D3DCOLOR clrBlend)
{
	m_clrBackBlend = clrBlend;
}

void TileMap::SetBackground(int nMaterialID, RECT rcTextureCoords)
{
	if (nMaterialID < 0 || nMaterialID >= int(m_arMaterials.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	m_nBackMaterialID = nMaterialID;

	m_BackStatic.SetMaterial(m_arMaterials[nMaterialID]);
	m_BackStatic.SetTextureCoords(rcTextureCoords);
}

void TileMap::SetBackground(int nAnimationID)
{
	if (nAnimationID < 0 || nAnimationID >= int(m_arAnimations.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	m_nBackAnimationID = nAnimationID;

	m_BackAnimated.SetAnimation(m_arAnimations[nAnimationID]);
}

int TileMap::GetBackgroundMaterialID(void) const
{
	return m_nBackMaterialID;
}

int TileMap::GetBackgroundAnimationID(void) const
{
	return m_nBackAnimationID;
}

MaterialInstance& TileMap::GetBackgroundStatic(void)
{
	return m_BackStatic;
}

const MaterialInstance& TileMap::GetBackgroundStaticConst(void) const
{
	return m_BackStatic;
}

MaterialInstance& TileMap::GetBackgroundAnimated(void)
{
	return m_BackAnimated;
}

const MaterialInstance& TileMap::GetBackgroundAnimatedConst(void) const
{
	return m_BackAnimated;
}

Actor* TileMap::CreateActor(LPCWSTR pszClass, LPCWSTR pszName)
{
	bool bSubclassed = !String::IsEmpty(pszName);

	try
	{
		Actor* pActor = (true == bSubclassed) ?
			dynamic_cast<Actor*>(m_rEngine.GetClasses().Create(pszClass, this)) :
			new Actor(*this, pszClass);

		if (NULL == pActor)
			throw m_rEngine.GetErrors().Push(Error::CLIENT_ABORT, __FUNCTIONW__);

		pActor->m_strName = pszName;

		return pActor;
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(Actor));
	}
}

Actor* TileMap::AddActor(LPCWSTR pszClass, LPCWSTR pszName)
{
	Actor* pNewActor = CreateActor(pszClass, pszName);

	if (pNewActor != NULL)
	{
		try
		{
			AddActor(pNewActor);
		}
		
		catch(Error& rError)
		{
			delete pNewActor;

			throw rError;
		}		
	}

	return pNewActor;
}

void TileMap::AddActor(Actor* pActor)
{
	if (NULL == pActor ||
		pActor->GetName().IsEmpty() || &pActor->GetMap() != this)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	m_mapActors[pActor->GetName()] = pActor;
}

void TileMap::RemoveActor(Actor* pActor)
{
	if (NULL == pActor || pActor->GetName().IsEmpty() ||
		&pActor->GetMap() != this)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	ActorMapIterator pos = m_mapActors.find(pActor->GetName());

	if (pos == m_mapActors.end()) return;

	delete pActor;

	m_mapActors.erase(pos);
}

void TileMap::RemoveAllActors(void)
{
	// Clear actors

	m_pPlayer = NULL;

	for(ActorMapIterator pos = m_mapActors.begin();
		pos != m_mapActors.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapActors.clear();

	// Clear actors scheduled for update

	m_arUpdateActors.clear();
}

int TileMap::GetActorCount(void) const
{
	return int(m_mapActors.size());
}

Actor* TileMap::GetActor(LPCWSTR pszName)
{
	ActorMapIterator pos = m_mapActors.find(pszName);

	if (pos == m_mapActors.end())
		return NULL;

	return pos->second;
}

const Actor* TileMap::GetActorConst(LPCWSTR pszName) const
{
	ActorMapConstIterator pos = m_mapActors.find(pszName);

	if (pos == m_mapActors.end())
		return NULL;

	return pos->second;
}

ActorMapIterator TileMap::GetBeginActorPos(void)
{
	return m_mapActors.begin();
}

ActorMapIterator TileMap::GetEndActorPos(void)
{
	return m_mapActors.end();
}

ActorMapConstIterator TileMap::GetBeginActorPosConst(void) const
{
	return m_mapActors.begin();
}

ActorMapConstIterator TileMap::GetEndActorPosConst(void) const
{
	return m_mapActors.end();
}

Actor* TileMap::GetPlayerActor(void)
{
	return m_pPlayer;
}

const Actor* TileMap::GetPlayerActorConst(void) const
{
	return m_pPlayer;
}

void TileMap::SetPlayerActor(Actor* pPlayer)
{
	if (NULL == pPlayer || &pPlayer->GetMap() != this)
		m_rEngine.GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	m_pPlayer = pPlayer;
}

int TileMap::GetLayersFromPosition(int x,
								   int y,
								   TileLayerArray& rarLayers) const
{
	for(TileLayerArrayConstIterator pos = m_arLayers.begin();
		pos != m_arLayers.end();
		pos++)
	{
		if ((*pos)->GetBounds().PtInRect(x, y) == true)
			rarLayers.push_back(*pos);
	}

	return int(rarLayers.size());
}

int TileMap::GetLayersFromPosition(float x,
								   float y,
								   TileLayerArray& rarLayers) const
{
	return GetLayersFromPosition(int(x), int(y), rarLayers);
}

int TileMap::GetLayersFromRange(Rect& rcTileRange, 
								TileLayerArray& rarLayers) const
{
	for(TileLayerArrayConstIterator pos = m_arLayers.begin();
		pos != m_arLayers.end();
		pos++)
	{
		if ((*pos)->GetBounds().Intersect(rcTileRange) == true)
			rarLayers.push_back(*pos);
	}

	return int(rarLayers.size());
}

int TileMap::GetActorsFromPosition(int x,
								   int y, 
								   ActorArray& rarActors) const
{
	// Get layers at that position

	TileLayerArray arLayersAt;

	if (GetLayersFromPosition(x, y, arLayersAt) == 0)
		return 0;

	// For each layer, get actors at that position

	ActorArray arActorsAt;

	for(TileLayerArrayConstIterator pos = arLayersAt.begin();
		pos != arLayersAt.end();
		pos++)
	{
		const TileLayer* pLayer = (*pos);

		pLayer->GetSpace()->Query(float(x) - pLayer->GetPositionConst().x,
			float(y) - pLayer->GetPositionConst().y, &arActorsAt);
	}

	// Remove duplicates

	std::sort(arActorsAt.begin(), arActorsAt.end());
	arActorsAt.erase(std::unique(arActorsAt.begin(), arActorsAt.end()));

	// Copy

	std::copy(arActorsAt.begin(), arActorsAt.end(),
		std::back_inserter(rarActors));

	return int(rarActors.size());
}

int TileMap::GetActorsFromPosition(float x, 
								   float y,
								   ActorArray& rarActors) const
{
	// Get layers at that position

	TileLayerArray arLayersAt;

	if (GetLayersFromPosition(x, y, arLayersAt) == 0)
		return 0;

	// For each layer, get actors at that position

	ActorArray arActorsAt;

	for(TileLayerArrayConstIterator pos = arLayersAt.begin();
		pos != arLayersAt.end();
		pos++)
	{
		const TileLayer* pLayer = (*pos);

		pLayer->GetSpace()->Query(x - pLayer->GetPositionConst().x,
			y - pLayer->GetPositionConst().y, &arActorsAt);
	}

	// Remove duplicates

	std::sort(arActorsAt.begin(), arActorsAt.end());
		arActorsAt.erase(std::unique(arActorsAt.begin(), arActorsAt.end()));

	// Copy

	std::copy(arActorsAt.begin(), arActorsAt.end(),
		std::back_inserter(rarActors));

	return int(rarActors.size());
}

int TileMap::GetActorsFromRange(Rect& rcTileRange, ActorArray& rarActors) const
{
	// Get layers at that position

	TileLayerArray arLayersAt;

	if (GetLayersFromRange(rcTileRange, arLayersAt) == 0)
		return 0;

	// For each layer, get actors at that position

	ActorArray arActorsAt;

	for(TileLayerArrayConstIterator pos = arLayersAt.begin();
		pos != arLayersAt.end();
		pos++)
	{
		const TileLayer* pLayer = (*pos);

		Rect rcRange = rcTileRange;
		rcRange.Offset(pLayer->GetPositionConst() * -1);

		pLayer->GetSpace()->Query(rcRange, &arActorsAt);
	}

	// Remove duplicates

	std::sort(arActorsAt.begin(), arActorsAt.end());
	arActorsAt.erase(std::unique(arActorsAt.begin(), arActorsAt.end()));

	// Copy

	std::copy(arActorsAt.begin(), arActorsAt.end(),
		std::back_inserter(rarActors));

	return int(rarActors.size());
}

Camera* TileMap::CreateCamera(void)
{
	return new Camera(this);
}

int TileMap::AddCamera(Camera* pCamera)
{
	m_arCameras.push_back(pCamera);

	return int(m_arCameras.size()) - 1;
}

void TileMap::RemoveCamera(int nCamera)
{
	if (nCamera < 0 || nCamera >= int(m_arCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
		__FUNCTIONW__, 0);

	Camera* pCamera = m_arCameras[nCamera];

	CameraArrayIterator pos =
		std::find(m_arActiveCameras.begin(),
			m_arActiveCameras.end(), pCamera);

	if (pos != m_arActiveCameras.end())
		m_arActiveCameras.erase(pos);	

	m_arCameras.erase(m_arCameras.begin() + nCamera);

	delete pCamera;
}

void TileMap::RemoveAllCameras(void)
{
	m_arActiveCameras.clear();

	for(CameraArrayIterator pos = m_arCameras.begin();
		pos != m_arCameras.end();
		pos++)
	{
		delete *pos;
	}

	m_arCameras.clear();
}

int TileMap::GetCameraCount(void) const
{
	return int(m_arCameras.size());
}

Camera* TileMap::GetCamera(int nCamera)
{
	if (nCamera < 0 || nCamera >= int(m_arCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arCameras[nCamera];
}

const Camera* TileMap::GetCameraConst(int nCamera) const
{
	if (nCamera < 0 || nCamera >= int(m_arCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arCameras[nCamera];
}

Camera* TileMap::GetActiveCamera(int nActiveCamera)
{
	if (nActiveCamera < 0 || nActiveCamera >= int(m_arActiveCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arActiveCameras[nActiveCamera];
}

const Camera* TileMap::GetActiveCameraConst(int nActiveCamera) const
{
	if (nActiveCamera < 0 || nActiveCamera >= int(m_arActiveCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	return m_arActiveCameras[nActiveCamera];
}

int TileMap::GetActiveCameraCount(void) const
{
	return int(m_arActiveCameras.size());
}

void TileMap::SetActiveCamera(Camera* pCamera)
{
	CameraArrayIterator posExisting =
		std::find(m_arActiveCameras.begin(),
			m_arActiveCameras.end(), pCamera);

	if (posExisting != m_arActiveCameras.end())
		return;

	m_arActiveCameras.push_back(pCamera);
}

void TileMap::RemoveActiveCamera(int nActiveCamera)
{
	if (nActiveCamera < 0 || nActiveCamera >= int(m_arActiveCameras.size()))
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM,
			__FUNCTIONW__, 0);

	m_arActiveCameras.erase(m_arActiveCameras.begin() + nActiveCamera);
}

void TileMap::RemoveActiveCamera(Camera* pCamera)
{
	CameraArrayIterator posExisting =
		std::find(m_arActiveCameras.begin(),
				  m_arActiveCameras.end(),
				  pCamera);

	if (posExisting != m_arActiveCameras.end())
		m_arActiveCameras.erase(posExisting);
}

void TileMap::RemoveAllActiveCameras(void)
{
	m_arActiveCameras.clear();
}

int TileMap::AddMaterial(Material* pMaterial)
{
	if (NULL == pMaterial)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	int nMaterialID = FindMaterial(pMaterial);

	if (INVALID_INDEX == nMaterialID)
	{
		pMaterial->AddRef();

		m_arMaterials.push_back(pMaterial);

		nMaterialID = int(m_arMaterials.size()) - 1;
	}

	return nMaterialID;
}

int TileMap::FindMaterial(Material* pMaterial) const
{
	MaterialArrayConstIterator pos =
		find(m_arMaterials.begin(), m_arMaterials.end(), pMaterial);

	if (pos == m_arMaterials.end())
		return INVALID_INDEX;

	return int(pos - m_arMaterials.begin());
}

void TileMap::RemoveMaterial(int nMaterialID)
{
	m_arMaterials[nMaterialID]->Release();

	m_arMaterials.erase(m_arMaterials.begin() + nMaterialID);
}

void TileMap::RemoveAllMaterials(void)
{
	for(MaterialArrayIterator pos = m_arMaterials.begin();
		pos != m_arMaterials.end();
		pos++)
	{
		(*pos)->Release();
	}

	m_arMaterials.clear();
}

int TileMap::GetMaterialCount(void) const
{
	return int(m_arMaterials.size());
}

Material* TileMap::GetMaterial(int nMaterialID)
{
	return m_arMaterials[nMaterialID];
}

const Material* TileMap::GetMaterialConst(int nMaterialID) const
{
	return m_arMaterials[nMaterialID];
}

int TileMap::AddAnimation(Animation* pAnimation)
{
	if (NULL == pAnimation)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	int nAnimationID = FindAnimation(pAnimation);

	if (INVALID_INDEX == nAnimationID)
	{
		pAnimation->AddRef();

		m_arAnimations.push_back(pAnimation);

		nAnimationID = int(m_arAnimations.size()) - 1;
	}

	return nAnimationID;
}

int TileMap::FindAnimation(Animation* pAnimation) const
{
	AnimationArrayConstIterator posFind =
		find(m_arAnimations.begin(), m_arAnimations.end(), pAnimation);

	if (posFind == m_arAnimations.end())
		return INVALID_INDEX;

	return int(posFind - m_arAnimations.begin());
}

void TileMap::RemoveAnimation(int nAnimationID)
{
	m_arAnimations[nAnimationID]->Release();

	m_arAnimations.erase(m_arAnimations.begin() + nAnimationID);
}

void TileMap::RemoveAllAnimations(void)
{
	for(AnimationArrayConstIterator pos = m_arAnimations.begin();
		pos != m_arAnimations.end();
		pos++)
	{
		(*pos)->Release();
	}

	m_arAnimations.clear();
}

Animation* TileMap::GetAnimation(int nAnimationID)
{
	return m_arAnimations[nAnimationID];
}

const Animation* TileMap::GetAnimationConst(int nAnimationID) const
{
	return m_arAnimations[nAnimationID];
}

int TileMap::GetAnimationCount(void) const
{
	return int(m_arAnimations.size());
}

int TileMap::AddSound(Sound* pSound)
{
	if (NULL == pSound) return INVALID_INDEX;

	int nSoundID = FindSound(pSound);

	if (INVALID_INDEX == nSoundID)
	{
		pSound->AddRef();

		m_arSounds.push_back(pSound);

		nSoundID = int(m_arSounds.size()) - 1;
	}

	return nSoundID;
}

int TileMap::FindSound(Sound* pSound) const
{
	SoundArrayConstIterator posFind =
		find(m_arSounds.begin(), m_arSounds.end(), pSound);

	if (posFind == m_arSounds.end())
		return INVALID_INDEX;

	return int(posFind - m_arSounds.begin());
}

void TileMap::RemoveSound(int nSoundID)
{
	m_arSounds[nSoundID]->Release();

	m_arSounds.erase(m_arSounds.begin() + nSoundID);
}

void TileMap::RemoveAllSounds(void)
{
	for(SoundArrayIterator pos = m_arSounds.begin();
		pos != m_arSounds.end();
		pos++)
	{
		(*pos)->Release();
	}

	m_arSounds.clear();
}

Sound* TileMap::GetSound(int nSoundID)
{
	return m_arSounds[nSoundID];
}

const Sound* TileMap::GetSoundConst(int nSoundID) const
{
	return m_arSounds[nSoundID];
}

int TileMap::GetSoundCount(void) const
{
	return int(m_arSounds.size());
}

int TileMap::AddMusic(Music* pMusic)
{
	if (NULL == pMusic) return INVALID_INDEX;

	int nMusicID = FindMusic(pMusic);

	if (INVALID_INDEX == nMusicID)
	{
		pMusic->AddRef();

		m_arMusic.push_back(pMusic);

		nMusicID = int(m_arMusic.size() - 1);
	}

	return nMusicID;
}

int TileMap::FindMusic(Music* pMusic) const
{
	MusicArrayConstIterator posFind =
		find(m_arMusic.begin(), m_arMusic.end(), pMusic);

	if (posFind == m_arMusic.end())
		return INVALID_INDEX;

	return int(posFind - m_arMusic.begin());
}

void TileMap::RemoveMusic(int nMusicID)
{
	m_arMusic[nMusicID]->Release();

	m_arMusic.erase(m_arMusic.begin() + nMusicID);
}

void TileMap::RemoveAllMusic(void)
{
	for(MusicArrayIterator pos = m_arMusic.begin();
		pos != m_arMusic.end();
		pos++)
	{
		(*pos)->Release();
	}

	m_arMusic.clear();
}

Music* TileMap::GetMusic(int nMusicID)
{
	return m_arMusic[nMusicID];
}

const Music* TileMap::GetMusicConst(int nMusicID) const
{
	return m_arMusic[nMusicID];
}

int TileMap::GetMusicCount(void) const
{
	return int(m_arMusic.size());
}

VariableManager& TileMap::GetVariables(void)
{
	return m_Variables;
}

const VariableManager& TileMap::GetVariablesConst(void) const
{
	return m_Variables;
}

void TileMap::Render(void)
{
	// Render all active cameras

	for(CameraArrayIterator pos = m_arActiveCameras.begin();
		pos != m_arActiveCameras.end();
		pos++)
	{
		(*pos)->Render();
	}
}

void TileMap::Update(void)
{
	// Update background animation

	if (IsFlagSet(BACKGROUND_ANIMATED) == true)
		m_BackAnimated.Update(m_rEngine.GetTime());

	// Update tile animations

	for(TileAnimatedArrayIterator pos = m_arTilesAnimated.begin();
		pos != m_arTilesAnimated.end();
		pos++)
	{
		pos->GetMaterialInstance().Update(m_rEngine.GetTime());
	}

	// Update actors

	for(ActorArrayIterator pos = m_arUpdateActors.begin();
		pos != m_arUpdateActors.end();
		pos++)
	{
		while(NULL == *pos)
		{
			// If this list item has been nulled out, remove it

			pos = m_arUpdateActors.erase(pos);

			if (m_arUpdateActors.end() == pos)
				return;
		}

		(*pos)->Update();
	}
}

void TileMap::Serialize(Stream& rStream, bool bInstance) const
{
	try
	{
		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) &&
			m_rEngine.GetClientInstance();

		if (true == bProgressNotify)
		{
			m_rEngine.GetClientInstance()->OnProgress(
					Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP,
					0,
					1);
		}

		// Write chunks sorted by chunk type

		int nCurChunkType = 0;
		int nCurChunkSize = 0;

		for(; nCurChunkType < TileMap::CHUNK_COUNT; nCurChunkType++)
		{
			// Check if we should skip this chunk

			switch(nCurChunkType)
			{
			case TileMap::CHUNK_MATERIALS:
				{
					// Skip chunk if there are no texture sheets

					if (m_arMaterials.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_ANIMATIONS:
				{
					// Skip chunk if there are no animations

					if (m_arAnimations.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_SOUNDS:
				{
					// Skip chunk if there are no sounds

					if (m_arSounds.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_MUSIC:
				{
					// Skip chunk if there is no music

					if (m_arMusic.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_TILES:
				{
					// Skip chunk if there are no tiles

					if (m_arTilesStatic.empty() == true ||
					   m_arTilesAnimated.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_LAYERS:
				{
					if (m_arLayers.empty() == true)
						continue;
				}
				break;
			case TileMap::CHUNK_ACTORS:
				{
					// Skip chunk if there are no actors

					if (m_mapActors.empty()) continue;
				}
				break;
			case TileMap::CHUNK_USER:
				{
					// Skip user chunk if map is not subclassed

					if (m_strClass.IsEmpty() == true)
					   continue;
				}
				break;
			}

			// Write chunk type as byte

			rStream.WriteVar((LPBYTE)&nCurChunkType);

			// Reserve space for chunk size

			rStream.WriteVar(&nCurChunkSize);

			// Track size of data written for this chunk

			rStream.ResetSizeWritten();

			// Write chunk

			switch(nCurChunkType)
			{
			case TileMap::CHUNK_HEADER:
				SerializeHeader(rStream);
				break;
			case TileMap::CHUNK_MATERIALS:
				SerializeMaterials(rStream);
				break;
			case TileMap::CHUNK_ANIMATIONS:
				SerializeAnimations(rStream);
				break;
			case TileMap::CHUNK_SOUNDS:
				SerializeSounds(rStream);
				break;
			case TileMap::CHUNK_MUSIC:
				SerializeMusic(rStream);
				break;
			case TileMap::CHUNK_TILES:
				SerializeTiles(rStream);
				break;
			case TileMap::CHUNK_LAYERS:
				SerializeLayers(rStream);
				break;
			case TileMap::CHUNK_ACTORS:
				SerializeActors(rStream);
				break;
			case TileMap::CHUNK_USER:
				SerializeUserData(rStream, bInstance);
				break;
			}

			// Write chunk size into space reserved for it

			nCurChunkSize = int(rStream.GetSizeWritten());

			rStream.SetPosition(-LONG(nCurChunkSize) - 4l,
								Stream::MOVE_CURRENT);

			rStream.WriteVar(&nCurChunkSize);

			rStream.SetPosition(LONG(nCurChunkSize),
								Stream::MOVE_CURRENT);

		} //for

		// Notify

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
												Client::PROGRESS_SAVE,
												Client::PROGRESS_MAP,
												1,
												1);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::Serialize(LPCWSTR pszPath) const
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

	try
	{
		// Write signature and version

		stream.WriteVar(TileMap::SIGNATURE, 4);
		stream.WriteVar(TileMap::FORMAT_VERSION, 4);

		// Write class key

		m_strClass.Serialize(stream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_WRITE,
			__FUNCTIONW__, pszPath);
	}

	// Write chunks
	
	Serialize(stream, false);
}

void TileMap::Deserialize(Stream& rStream, bool bInstance)
{
	if (false == bInstance)
		Empty();

	m_strName = rStream.GetPath();

	// Notify

	bool bProgressNotify =
		m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
		m_rEngine.GetClientInstance() != NULL;

	if (true == bProgressNotify)
	{
		m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_MAP, 0, 1);
	}

	try
	{
		// Read all the chunks

		int nCurChunkType = 0;
		int nCurChunkSize = 0;

		bool bReadMaterials = false;
		bool bReadAnimations = false;

		do
		{
			// Read current chunk type as byte

			rStream.ReadVar((BYTE*)&nCurChunkType);

			// Read current chunk size

			rStream.ReadVar(&nCurChunkSize);
			
			// Read the chunk data, depending on type

			switch(nCurChunkType)
			{
			case TileMap::CHUNK_HEADER:
				DeserializeHeader(rStream);
				break;
			case TileMap::CHUNK_MATERIALS:
				DeserializeMaterials(rStream);
				bReadMaterials = true;
				break;
			case TileMap::CHUNK_ANIMATIONS:
				DeserializeAnimations(rStream);
				bReadAnimations = true;
				break;
			case TileMap::CHUNK_SOUNDS:
				DeserializeSounds(rStream);
				break;
			case TileMap::CHUNK_MUSIC:
				DeserializeMusic(rStream);
				break;
			case TileMap::CHUNK_TILES:
				{
					// Materials and animations chunks must always
					// precede tile templates chunk

					if (false == bReadMaterials || false == bReadAnimations)
						throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
							__FUNCTIONW__, rStream.GetPath());

					DeserializeTiles(rStream);
				}
				break;
			case TileMap::CHUNK_LAYERS:
				DeserializeLayers(rStream);
				break;
			case TileMap::CHUNK_ACTORS:
				{
					// Skip chunk if loading instance, use instance's actor
					// information instead

					if (true == bInstance)
					{
						rStream.SetPosition(LONG(nCurChunkSize),
											Stream::MOVE_CURRENT);

						continue;
					}

					DeserializeActors(rStream);
				}
				break;
			case TileMap::CHUNK_USER:
				{
					DeserializeUserData(rStream, bInstance);
				}
				break;
			default:
				{
					// If chunk type is unknown, ignore chunk

					rStream.SetPosition(LONG(nCurChunkSize),
						Stream::MOVE_CURRENT);
				}
				break;

			} // switch

		} while(rStream.GetPosition() < rStream.GetSize());

		if (false == bInstance)
		{
			// Cache background

			if (IsFlagSet(BACKGROUND_ANIMATED) == true)
				m_BackAnimated.SetAnimation(GetAnimation(m_nBackAnimationID));
			else
				m_BackStatic.SetMaterial(GetMaterial(m_nBackMaterialID));
		}

		// Notify

		if (true == bProgressNotify)
		{
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP, 1, 1);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeInstance(Stream& rStream) const
{
	try
	{
		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		// Serialize header chunk to instance

		SerializeHeader(rStream);

		// Notify client we are starting with part 1 of instance data

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_SAVE, Client::PROGRESS_MAP_INSTANCE, 0, 1);

		// Serialize actors to instance

		SerializeActors(rStream);

		// Notify client we are done with part 1 of instance data

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_SAVE, Client::PROGRESS_MAP_INSTANCE, 1, 1);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeInstance(Stream& rStream)
{
	try
	{
		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		// Deserialize header chunk from instance

		DeserializeHeader(rStream);

		// Notify client we are starting with part 1 of instance data

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_LOAD, Client::PROGRESS_MAP_INSTANCE, 0, 1);

		// Deserialize actors from instance

		DeserializeActors(rStream);

		// Notify client we are done with part 1 of instance data

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_INSTANCE, 1, 1);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeHeader(Stream& rStream) const
{
	try
	{
		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		// Notify that we are starting map header

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_HEADER, 0, 1);

		// Write flags

		rStream.WriteVar((const LPDWORD)&m_dwFlags);

		// Write background color

		m_clrBackColor.Serialize(rStream);

		// Write background blend

		m_clrBackBlend.Serialize(rStream);

		// Write background

		if (IsFlagSet(BACKGROUND_ANIMATED) == true)
		{
			rStream.WriteVar(&m_nBackAnimationID);

			m_BackAnimated.Serialize(m_rEngine, rStream, false);
		}
		else
		{
			rStream.WriteVar(&m_nBackMaterialID);

			m_BackStatic.Serialize(m_rEngine, rStream, false);
		}		

		// Write variables

		m_Variables.Serialize(rStream);

		// Notify client that we are ending header chunk part 1

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_SAVE, Client::PROGRESS_MAP_HEADER, 1, 1);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeHeader(Stream& rStream)
{
	try
	{
		// Notify that we are starting map header

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
			Client::PROGRESS_MAP_HEADER, 0, 1);

		// Read flags

		rStream.ReadVar(&m_dwFlags);

		// Read background color

		m_clrBackColor.Deserialize(rStream);

		// Read background blend

		m_clrBackBlend.Deserialize(rStream);

		// Read background

		if (IsFlagSet(BACKGROUND_ANIMATED) == true)
		{
			rStream.ReadVar(&m_nBackAnimationID);

			m_BackAnimated.Deserialize(m_rEngine, rStream, false);
		}
		else
		{
			rStream.ReadVar(&m_nBackMaterialID);

			m_BackStatic.Deserialize(m_rEngine, rStream, false);
		}

		// Read variables

		m_Variables.Deserialize(rStream);

		// Notify client that we are ending header chunk part 1 of 1

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_HEADER, 1, 1);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeMaterials(Stream& rStream) const
{
	try
	{
		// Write the number of material instances

		int nCount = int(m_arMaterials.size());
		rStream.WriteVar(&nCount);

		// Notify client

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_MATERIALS, 0, nCount);

		// Write Material instances

		for(int n = 0; n < nCount; n++)
		{
			if (m_arMaterials[n] != NULL)
				m_arMaterials[n]->SerializeInstance(rStream);
			else
				Object::SerializeNullInstance(rStream);

			// Notify client

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_MATERIALS, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeMaterials(Stream& rStream)
{
	try
	{
		// Read the number of Material instances

		int nCount = 0;
		rStream.ReadVar(&nCount);

		if (0 == nCount) return;

		// Notify client

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_MATERIALS, 0, nCount);

		// Read Material instances

		m_arMaterials.resize(nCount);

		for(int n = 0; n < nCount; n++)
		{
			m_arMaterials[n] =
				m_rEngine.GetMaterials().LoadInstance(rStream);

			m_arMaterials[n]->AddRef();

			// Notify client

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(
					Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_MATERIALS, n + 1, nCount);
		}
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeAnimations(Stream& rStream) const
{
	try
	{
		// Write animation instance count

		int nCount = int(m_arAnimations.size());

		rStream.WriteVar(&nCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_ANIMATIONS, 0, nCount);

		// Write animation instances
		
		for(int n = 0; n < nCount; n++)
		{
			if (m_arAnimations[n] != NULL)
				m_arAnimations[n]->SerializeInstance(rStream);
			else
				Object::SerializeNullInstance(rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_ANIMATIONS, n + 1, nCount);
		}
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeAnimations(Stream& rStream)
{
	try
	{
		// Read animation instance count

		int nCount = 0;
		rStream.ReadVar(&nCount);

		if (0 == nCount) return;

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_ANIMATIONS, 0, nCount);

		// Read animation instances

		SIZE sizeTile = { m_rEngine.GetOption(Engine::OPTION_TILE_SIZE),
			m_rEngine.GetOption(Engine::OPTION_TILE_SIZE) };

		m_arAnimations.resize(nCount);

		for(int n = 0; n < nCount; n++)
		{
			// Load animation

			Animation* pAnim =
				m_rEngine.GetAnimations().LoadInstance(rStream);				

			// Since these are tile animations, set animation size to tile size]

			pAnim->SetFrameSize(sizeTile);

			// Reference this animation

			pAnim->AddRef();

			m_arAnimations[n] = pAnim;

			// Update progresss

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_ANIMATIONS, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeSounds(Stream& rStream) const
{
	try
	{
		// Write sound instances

		int nCount = int(m_arSounds.size());
		rStream.WriteVar(&nCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_SOUNDS, 0, nCount);

		// Write sound instances

		for(int n = 0; n < nCount; n++)
		{
			if (m_arSounds[n] != NULL)
				m_arSounds[n]->SerializeInstance(rStream);
			else
				Object::SerializeNullInstance(rStream);

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_SOUNDS, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeSounds(Stream& rStream)
{
	try
	{
		// Read sound instance count

		int nCount = 0;
		rStream.ReadVar(&nCount);

		if (0 == nCount) return;

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_SOUNDS, 0, nCount);

		// Read sound instances

		m_arSounds.resize(nCount);

		for(int n = 0; n < nCount; n++)
		{
			m_arSounds[n] =
				m_rEngine.GetSounds().LoadInstance(rStream);

			m_arSounds[n]->AddRef();

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(
					Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_SOUNDS, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}	

void TileMap::SerializeMusic(Stream& rStream) const
{
	try
	{
		// Write music instance count

		int nCount = int(m_arMusic.size());
		rStream.WriteVar(&nCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_MUSIC, 0, nCount);

		// Write music instances

		for(int n = 0; n < nCount; n++)
		{
			if (m_arMusic[n] != NULL)
				m_arMusic[n]->SerializeInstance(rStream);
			else
				Object::SerializeNullInstance(rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_MUSIC, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeMusic(Stream& rStream)
{
	try
	{
		// Read music instance count

		int nCount = 0;
		rStream.ReadVar(&nCount);

		if (0 == nCount) return;

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_MUSIC, 0, nCount);

		// Read music instances

		m_arMusic.resize(nCount);

		for(int n = 0; n < nCount; n++)
		{
			m_arMusic[n] =
				m_rEngine.GetMusic().LoadInstance(rStream);

			m_arMusic[n]->AddRef();

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_MUSIC, n + 1, nCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeTiles(Stream& rStream) const
{
	try
	{
		// Write static tile count

		int nTilesStatic = int(m_arTilesStatic.size());
		rStream.WriteVar(&nTilesStatic);

		// Write animated tile count

		int nTilesAnimated = int(m_arTilesAnimated.size());
		rStream.WriteVar(&nTilesAnimated);

		int nTiles = nTilesStatic + nTilesAnimated;

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
			Client::PROGRESS_MAP_TILES, 0, nTiles);

		// Write static tiles

		int nWritten = 0;

		for(TileStaticArrayConstIterator pos = m_arTilesStatic.begin();
			pos != m_arTilesStatic.end();
			pos++)
		{
			pos->Serialize(*this, rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_TILES, ++nWritten, nTiles);
		}

		// Write animated tiles

		for(TileAnimatedArrayConstIterator pos = m_arTilesAnimated.begin();
			pos != m_arTilesAnimated.end();
			pos++)
		{
			// Write tile

			pos->Serialize(*this, rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_TILES, ++nWritten, nTiles);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeTiles(Stream& rStream)
{
	try
	{
		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		int nTilesStatic = 0;
		int nTilesAnimated = 0;
		int nTiles = 0;
		int nRead = 0;

		// Read static tile count

		rStream.ReadVar(&nTilesStatic);

		// Read animated tile count

		rStream.ReadVar(&nTilesAnimated);

		nTiles = nTilesStatic + nTilesAnimated;

		// Notify begin tiles

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_TILES, 0, nTiles);

		m_arTilesStatic.resize(nTilesStatic);
		m_arTilesAnimated.resize(nTilesAnimated);

		// Read static tiles

		for(int n = 0; n < nTilesStatic; n++, nRead++)
		{
			m_arTilesStatic[n].Deserialize(*this, rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_TILES, nRead, nTiles);
		}

		// Read animated tiles

		for(int n = 0; n < nTilesAnimated; n++, nRead++)
		{
			// Read tile

			m_arTilesAnimated[n].Deserialize(*this, rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_TILES, nRead, nTiles);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeLayers(Stream& rStream) const
{
	try
	{
		// Write layer count

		int nLayerCount = int(m_arLayers.size());

		rStream.WriteVar(&nLayerCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_LAYERS, 0, nLayerCount);

		// Write layers

		int nWritten = 0;

		for(TileLayerArrayConstIterator pos = m_arLayers.begin();
			pos != m_arLayers.end();
			pos++)
		{
			// Write layer

			(*pos)->Serialize(rStream);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(
					Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_LAYERS, ++nWritten, nLayerCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeLayers(Stream& rStream)
{
	try
	{
		// Read layer count

		int nLayerCount = 0;

		rStream.ReadVar(&nLayerCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_LAYERS, 0, nLayerCount);

		// Read layers

		m_arLayers.resize(nLayerCount);

		for(int n = 0; n < nLayerCount; n++)
		{
			// Read layer

			TileLayer* pLayer = CreateLayer();
			pLayer->Deserialize(rStream);

			m_arLayers[n] = pLayer;

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(
					Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_LAYERS, n, nLayerCount);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeActors(Stream& rStream) const
{
	try
	{
		// Write actor count

		int nCount = int(m_mapActors.size());
		rStream.WriteVar(&nCount);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(
				Client::PROGRESS_SAVE,
				Client::PROGRESS_MAP_ACTORS, 0, nCount);

		// Write actors

		int n = 0;

		for(ActorMapConstIterator pos = m_mapActors.begin();
			pos != m_mapActors.end();
			pos++, n++)
		{
			if (pos->second != NULL)
			{
				// Write actor class

				pos->second->GetClass().Serialize(rStream);

				// Write actor

				pos->second->Serialize(rStream);
			}
			else
			{
				throw m_rEngine.GetErrors().Push(Error::FILE_NULLINSTANCE,
					__FUNCTIONW__, rStream.GetPath());
			}

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_MAP_ACTORS, n + 1, nCount);
		}

		// Write player actor name

		if (m_pPlayer != NULL)
		{
			m_pPlayer->GetName().Serialize(rStream);
		}
		else
		{
			Object::SerializeNullInstance(rStream);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::DeserializeActors(Stream& rStream)
{
	try
	{
		// Clear all current actors

		RemoveAllActors();

		// Read actor count

		int nCount = 0;			
		rStream.ReadVar(&nCount);

		if (0 == nCount)
			return;

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
								m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_MAP_ACTORS, 0, nCount);

		// Read Actors

		String strClass;

		for(int n = 0; n < nCount; n++)
		{
			// Read class

			strClass.Deserialize(rStream);

			// Create actor

			Actor* pActor = CreateActor(strClass, NULL);

			// Read actor

			pActor->Deserialize(rStream);

			// Add actor to actor list

			AddActor(pActor);

			// If update flag specified, add to update list

			if (pActor->IsFlagSet(Actor::UPDATE) == true)
				m_arUpdateActors.push_back(pActor);

			// Notify

			if (true == bProgressNotify)
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_MAP_ACTORS, n + 1, nCount);
		}

		// Read player actor name

		String strPlayerActor;
		strPlayerActor.Deserialize(rStream);

		if (strPlayerActor.IsEmpty() == false)
			m_pPlayer = GetActor(strPlayerActor);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMap::SerializeUserData(Stream& rStream, bool bInstance) const
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);
	UNREFERENCED_PARAMETER(bInstance);
}

void TileMap::DeserializeUserData(Stream& rStream, bool bInstance)
{
	// Default handler

	UNREFERENCED_PARAMETER(rStream);
	UNREFERENCED_PARAMETER(bInstance);
}

DWORD TileMap::GetMemoryFootprint(void) const
{
	DWORD dwSize = Object::GetMemoryFootprint() -
		sizeof(Object) + sizeof(TileMap);

	dwSize += DWORD(m_strClass.GetLengthBytes());

	dwSize += GetActorsMemoryFootprint();

	dwSize += GetLayersMemoryFootprint();

	dwSize += DWORD(m_arCameras.size() * (sizeof(int) + sizeof(Camera)));

	dwSize += DWORD(m_arActiveCameras.size() * sizeof(int));

	dwSize += m_Variables.GetMemoryFootprint();

	dwSize += DWORD(m_arMaterials.size() * sizeof(int));

	dwSize += DWORD(m_arAnimations.size() * sizeof(int));

	dwSize += DWORD(m_arSounds.size() * sizeof(int));

	dwSize += DWORD(m_arMusic.size() * sizeof(int));

	return dwSize;
}

DWORD TileMap::GetActorsMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(ActorMapConstIterator posActors = m_mapActors.begin();
		posActors != m_mapActors.end();
		posActors++)
	{
		dwSize += posActors->second->GetMemoryFootprint();
	}

	return dwSize;
}

DWORD TileMap::GetLayersMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(TileLayerArrayConstIterator posLayers = m_arLayers.begin();
		posLayers != m_arLayers.end();
		posLayers++)
	{
		dwSize += (*posLayers)->GetMemoryFootprint();
	}

	return dwSize;
}

void TileMap::Empty(void)
{
	// Unload cameras

	RemoveAllCameras();

	// Unload actors

	RemoveAllActors();

	// Unload layers

	RemoveAllLayers();

	// Unload tile templates

	RemoveAllTileTemplates();

	// Unload resources

	RemoveAllMaterials();
	RemoveAllAnimations();
	RemoveAllSounds();
	RemoveAllMusic();
}

void TileMap::OnLostDevice(bool bRecreate)
{
	// Forward to actors

	for(ActorMapIterator pos = m_mapActors.begin();
		pos != m_mapActors.end();
		pos++)
	{
		pos->second->OnLostDevice(bRecreate);
	}
}

void TileMap::OnResetDevice(bool bRecreate)
{
	// Forward to actors

	for(ActorMapIterator pos = m_mapActors.begin();
		pos != m_mapActors.end();
		pos++)
	{
		pos->second->OnResetDevice(bRecreate);
	}
}

void TileMap::OnSessionPause(bool bPause)
{
	// Forward to actors

	for(ActorMapIterator pos = m_mapActors.begin();
		pos != m_mapActors.end();
		pos++)
	{
		pos->second->OnSessionPause(bPause);
	}
}

void TileMap::OnKeyDown(int nKeyCode)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnKeyDown(nKeyCode);
}

void TileMap::OnKeyUp(int nKeyCode)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnKeyUp(nKeyCode);
}

void TileMap::OnKeyPress(int nKeyCode)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnKeyPress(nKeyCode);
}

void TileMap::OnMouseMove(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseMove(pt);
}

void TileMap::OnMouseLDown(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseLDown(pt);
}

void TileMap::OnMouseLUp(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseLUp(pt);
}

void TileMap::OnMouseLDbl(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseLDbl(pt);
}

void TileMap::OnMouseRDown(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseRDown(pt);
}

void TileMap::OnMouseRUp(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseRUp(pt);
}

void TileMap::OnMouseRDbl(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseRDbl(pt);
}

void TileMap::OnMouseMDown(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseMDown(pt);
}

void TileMap::OnMouseMUp(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseMUp(pt);
}

void TileMap::OnMouseMDbl(POINT pt)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseMDbl(pt);
}

void TileMap::OnMouseWheel(int nZDelta)
{
	// Forward to player

	if (m_pPlayer != NULL) m_pPlayer->OnMouseWheel(nZDelta);
}

/*----------------------------------------------------------*\
| TileMapManager implementation
\*----------------------------------------------------------*/

TileMapManager::TileMapManager(Engine& rEngine): m_rEngine(rEngine)
{
	ZeroMemory(m_szBasePath, sizeof(m_szBasePath));
	ZeroMemory(m_szBaseExt, sizeof(m_szBaseExt));
}

TileMapManager::~TileMapManager(void)
{
	Empty();
}

TileMap* TileMapManager::Create(LPCWSTR pszClass)
{
	TileMap* pNewMap = NULL;

	if (String::IsEmpty(pszClass) == false)
	{
		// Create from class key

		pNewMap =
			dynamic_cast<TileMap*>(m_rEngine.GetClasses().Create(pszClass));
	}
	else
	{
		// Create unsubclassed

		m_rEngine.Print(L"creating unsubclassed map.", PRINT_WARNING);

		try
		{
			pNewMap = new TileMap(m_rEngine);
		}

		catch(std::bad_alloc)
		{
			throw Error(Error::MEM_ALLOC, __FUNCTIONW__, sizeof(TileMap));
		}
	}

	return pNewMap;
}

void TileMapManager::Add(TileMap* pMap)
{
	if (NULL == pMap)
		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	m_arMaps.push_back(pMap);
}

TileMap* TileMapManager::Add(LPCWSTR pszClass)
{
	TileMap* pNewMap = Create(pszClass);

	Add(pNewMap);

	return pNewMap;
}

TileMap* TileMapManager::Find(LPCWSTR pszPath)
{
	for(TileMapArrayIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		if ((*pos)->GetName() == pszPath) return *pos;
	}

	return NULL;
}

const TileMap* TileMapManager::FindConst(LPCWSTR pszPath) const
{
	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		if ((*pos)->GetName() == pszPath) return *pos;
	}

	return NULL;
}

TileMap* TileMapManager::FindPattern(LPCWSTR pszPattern)
{
	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		if (wcsstr((*pos)->GetName(), pszPattern)) return *pos;
	}

	return NULL;
}

const TileMap* TileMapManager::FindPatternConst(LPCWSTR pszPattern) const
{
	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		if (wcsstr((*pos)->GetName(), pszPattern)) return *pos;
	}

	return NULL;
}

void TileMapManager::Remove(TileMap* pMap)
{
	if (m_rEngine.GetCurrentMapConst() == pMap)
		m_rEngine.SetCurrentMap(NULL);

	TileMapArrayIterator pos =
		std::find(m_arMaps.begin(), m_arMaps.end(), pMap);

	if (pos != m_arMaps.end())
	{
		delete *pos;
		*pos = NULL;
	}
}

void TileMapManager::RemoveAll(void)
{
	m_rEngine.SetCurrentMap(NULL);

	for(TileMapArrayIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		delete *pos;
	}

	m_arMaps.clear();
}

TileMap* TileMapManager::Load(LPCWSTR pszPath,
							  bool bInstance,
							  bool bCheckExists,
							  bool bReload)
{
	// Make sure we are loading from a full path

	WCHAR szFullPath[MAX_PATH] = {0};

	if (*(PathFindExtension(pszPath)) != L'\0')
	{
		GetAbsolutePath(pszPath, szFullPath);
	}
	else
	{
		m_rEngine.GetBaseFilePath(pszPath,
			m_szBasePath, m_szBaseExt, szFullPath);
	}

	TileMap* pMap = NULL;

	if (true == bCheckExists)
	{
		pMap = Find(szFullPath);

		if (pMap != NULL && false == bReload)
			return pMap;
	}
	else if (true == bReload)
	{
		// Cannot reload without finding first

		throw m_rEngine.GetErrors().Push(Error::INVALID_PARAM, __FUNCTIONW__, 3);
	}

	Stream stream(&m_rEngine.GetErrors());

	try
	{
		stream.Open(szFullPath, GENERIC_READ, OPEN_EXISTING);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_OPEN, __FUNCTIONW__, pszPath);
	}

	// Read map signature and version

	BYTE bySignature[4], byVersion[4];

	try
	{
		stream.ReadVar(bySignature, 4);
		stream.ReadVar(byVersion, 4);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_READ,
			__FUNCTIONW__, pszPath);
	}

	// Validate signature and version

	if (memcmp(bySignature, TileMap::SIGNATURE, sizeof(TileMap::SIGNATURE)))
		throw m_rEngine.GetErrors().Push(Error::FILE_SIGNATURE,
			__FUNCTIONW__, pszPath);

	if (memcmp(byVersion, TileMap::FORMAT_VERSION, sizeof(TileMap::FORMAT_VERSION)))
		m_rEngine.GetErrors().Push(Error::FILE_VERSION,
			__FUNCTIONW__, pszPath);

	// Set current directory to map directory

	PUSH_CURRENT_DIRECTORY(szFullPath);

	// Read class name

	String strClass;

	try
	{
		strClass.Deserialize(stream);

		// If not reloading, create from class
	
		if (false == bReload && NULL == pMap)
			pMap = Create(strClass);

		// Read the rest of it once created

		pMap->Deserialize(stream, bInstance);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (false == bReload)
			delete pMap;

		SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::FILE_READ, __FUNCTIONW__, pszPath);
	}

	// Restore current directory

	POP_CURRENT_DIRECTORY();

	// Add map

	if (false == bReload)
		m_arMaps.push_back(pMap);

	return pMap;
}

TileMap* TileMapManager::LoadInstance(Stream& rStream, bool bCheckExists)
{
	TileMap* pMap = NULL;

	try
	{
		// Read map path

		String strName;
		strName.Deserialize(rStream);

		// Make sure it's a full path

		WCHAR szFullPath[MAX_PATH] = {0};

		if (PathFindExtension(strName) != NULL)
		{
			GetAbsolutePath(strName, szFullPath);
		}
		else
		{
			m_rEngine.GetBaseFilePath(strName,
				m_szBasePath, m_szBaseExt, szFullPath);
		}

		// Load map from path

		pMap = Load(szFullPath, true, bCheckExists);

		// Read map instance data

		pMap->DeserializeInstance(rStream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		if (pMap != NULL) Remove(pMap);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}

	return pMap;
}

void TileMapManager::Save(TileMap* pMap, LPCWSTR pszPath) const
{
	try
	{
		// Get full path

		WCHAR szFullPath[MAX_PATH] = {0};
		
		if (*(PathFindExtension(pszPath)) != L'\0')
		{
			GetAbsolutePath(pszPath, szFullPath);
		}
		else
		{
			m_rEngine.GetBaseFilePath(pszPath,
				m_szBasePath, m_szBaseExt, szFullPath);
		}

		// Set current directory to map directory

		PUSH_CURRENT_DIRECTORY(szFullPath);

		// Save map

		pMap->Serialize(szFullPath);

		// Set map path

		pMap->SetName(szFullPath);

		// Restore current directory

		POP_CURRENT_DIRECTORY();
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, pszPath);
	}
}

void TileMapManager::Update(void)
{
	for(TileMapArrayIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		while(NULL == *pos)
		{
			pos = m_arMaps.erase(pos);

			if (m_arMaps.end() == pos) return;
		}

		if ((*pos)->IsFlagSet(TileMap::UPDATE) == true)
			(*pos)->Update();
	}
}

void TileMapManager::Serialize(Stream& rStream) const
{
	try
	{
		// Write the number of maps

		int nMaps = int(m_arMaps.size());
		rStream.WriteVar(&nMaps);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
				Client::PROGRESS_SESSION_MAPS, 0, nMaps);

		// Write maps

		String strRelPath;	
		strRelPath.Allocate(MAX_PATH);

		int n = 0;

		for(TileMapArrayConstIterator pos = GetBeginPosConst();
			pos != GetEndPosConst();
			pos++, n++)
		{
			if ((*pos)->GetName().GetLength() == 0)
				throw m_rEngine.GetErrors().Push(Error::FILE_NULLINSTANCE,
					__FUNCTIONW__, rStream.GetPath());

			GetRelativePath((*pos)->GetName(), strRelPath);

			strRelPath.Serialize(rStream);

			(*pos)->SerializeInstance(rStream);

			// Notify

			if (true == bProgressNotify)
			{
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_SAVE,
					Client::PROGRESS_SESSION_MAPS, n + 1, nMaps);
			}
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMapManager::Deserialize(Stream& rStream)
{
	try
	{
		// Read the number of maps

		int nMaps = 0;
		rStream.ReadVar(&nMaps);

		// Notify

		bool bProgressNotify =
			m_rEngine.GetOption(Engine::OPTION_PROGRESS_EVENTS) == TRUE &&
			m_rEngine.GetClientInstance() != NULL;

		if (true == bProgressNotify)
		{
			m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
				Client::PROGRESS_SESSION_MAPS, 0, nMaps);
		}

		// Read maps

		for(int n = 0; n < nMaps; n++)
		{
			LoadInstance(rStream, true);

			if (true == bProgressNotify)
			{
				m_rEngine.GetClientInstance()->OnProgress(Client::PROGRESS_LOAD,
					Client::PROGRESS_SESSION_MAPS, n + 1, nMaps);
			}
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void TileMapManager::OnLostDevice(bool bRecreate)
{
	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		(*pos)->OnLostDevice(bRecreate);
	}
}

void TileMapManager::OnResetDevice(bool bRecreate)
{
	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		(*pos)->OnResetDevice(bRecreate);
	}
}

DWORD TileMapManager::GetMemoryFootprint(void) const
{
	DWORD dwSize = 0;

	for(TileMapArrayConstIterator pos = m_arMaps.begin();
		pos != m_arMaps.end();
		pos++)
	{
		dwSize += (*pos)->GetMemoryFootprint();
	}
	
	return dwSize;
}