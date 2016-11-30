/*------------------------------------------------------------------*\
|
| ThunderTileMap.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine tile map class(es)
| Created: 04/09/2004
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_TILE_MAP_H
#define THUNDER_TILE_MAP_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderTileLayer.h"	// using TileLayer, MaterialInstance
#include "ThunderActor.h"		// using Actor
#include "ThunderCamera.h"		// using Camera
#include "ThunderVariable.h"	// using VariableManager

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Sound;					// referencing Sound
class Music;					// referencing Music
class Sound;					// referencing Sound
class Stream;					// referencing Stream
class TileMap;					// referencing TileMap, declared below
class CollisionInfo;			// referencing CollisionInfo, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<TileMap*> TileMapArray;
typedef std::vector<TileMap*>::iterator TileMapArrayIterator;
typedef std::vector<TileMap*>::const_iterator TileMapArrayConstIterator;

typedef std::vector<Material*> MaterialArray;
typedef std::vector<Material*>::iterator MaterialArrayIterator;
typedef std::vector<Material*>::const_iterator MaterialArrayConstIterator;

typedef std::vector<Sound*> SoundArray;
typedef std::vector<Sound*>::iterator SoundArrayIterator;
typedef std::vector<Sound*>::const_iterator SoundArrayConstIterator;

typedef std::vector<Music*> MusicArray;
typedef std::vector<Music*>::iterator MusicArrayIterator;
typedef std::vector<Music*>::const_iterator MusicArrayConstIterator;


/*----------------------------------------------------------*\
| TileMap class
\*----------------------------------------------------------*/

class TileMap: public Object
{
public:
	//
	// Constants
	//

	// Map flags

	enum Flags
	{
		// No flags specified
		DEFAULT				= 0,

		// Use background material
		BACKGROUND			= 1 << 0,

		// Background material is tiled
		BACKGROUND_TILE		= 1 << 1,

		// Background material is stretched
		BACKGROUND_STRETCH	= 1 << 2,

		// Background material is centered
		BACKGROUND_CENTER	= 1 << 3,

		// Use animated background material
		BACKGROUND_ANIMATED = 1 << 4,

		// Render active cameras?
		RENDER				= 1 << 5,

		// Should engine call Update() on each tick for this map?
		UPDATE				= 1 << 6,

		// Optimize progressively as tiles are being set
		OPTIMIZE			= 1 << 7,

		// First value for user flagss
		USER				= 1 << 7
	};

	// Chunks used in a map file

	enum Chunks
	{
		// Map info header
		CHUNK_HEADER,

		// Material table
		CHUNK_MATERIALS,

		// Animation table
		CHUNK_ANIMATIONS,

		// Sound table
		CHUNK_SOUNDS,

		// Music table
		CHUNK_MUSIC,

		// Tile templates
		CHUNK_TILES,

		// Layers
		CHUNK_LAYERS,

		// Actors
		CHUNK_ACTORS,

		// User chunk
		CHUNK_USER,

		// Number of pre-defined chunks
		CHUNK_COUNT
	};

	static const BYTE SIGNATURE[4];
	static const BYTE FORMAT_VERSION[4];

	static const int RESERVE_LAYERS;
	static const int RESERVE_MATERIALS;
	static const int RESERVE_ANIMATIONS;
	static const int RESERVE_SOUNDS;
	static const int RESERVE_MUSIC;
	static const int RESERVE_CAMERAS;
	static const int RESERVE_ACTIVECAMERAS;

protected:
	//
	// Attributes
	//

	// Class key created from
	String m_strClass;

	//
	// Tiles
	//

	// Static tile templates
	TileStaticArray m_arTilesStatic;

	// Animated tile templates
	TileAnimatedArray m_arTilesAnimated;

	//
	// Layers
	//

	// Tile layers (each element indexes into tiles)
	TileLayerArray m_arLayers;

	//
	// Actors
	//

	// Actors mapped by name
	ActorMap m_mapActors;

	// Actors with UPDATE flag, updated on every frame
	ActorArray m_arUpdateActors;

	// Actor that receives forwarded keyboard and mouse input
	Actor* m_pPlayer;

	//
	// Cameras
	//

	// All cameras (map views)
	CameraArray m_arCameras;

	// Active cameras (get rendered)
	CameraArray m_arActiveCameras;

	//
	// Background
	//

	// Background material ID
	int m_nBackMaterialID;

	// Background animation ID
	int m_nBackAnimationID;

	// Background static material instance
	MaterialInstance m_BackStatic;

	// Background animated material instance
	MaterialInstance m_BackAnimated;
	
	// Background texture/animation blend color
	Color m_clrBackBlend;

	// Background color
	Color m_clrBackColor;

	//
	// Resources
	//

	// Material table for all materials used by tiles
	MaterialArray m_arMaterials;

	// Animation table for all animations used by tiles (uses additional materials)
	AnimationArray m_arAnimations;

	// Sound table for all ambient sounds used by the map
	SoundArray m_arSounds;

	// Music table for all music files used by the map
	MusicArray m_arMusic;

	//
	// Scripting
	//

	// Map variables
	VariableManager m_Variables;

public:
	TileMap(Engine& rEngine, LPCWSTR pszClass = NULL);
	virtual ~TileMap(void);

public:
	//
	// Class
	//

	const String& GetClass(void) const;

	//
	// Background
	//

	Color& GetBackgroundColor(void);
	const Color& GetBackgroundColorConst(void) const;
	void SetBackgroundColor(D3DCOLOR clrBackColor);

	Color& GetBackgroundBlend(void);
	const Color& GetBackgroundBlendConst(void) const;
	void SetBackgroundBlend(D3DCOLOR clrBlend);

	void SetBackground(int nMaterialID, RECT rcTextureCoords);
	void SetBackground(int nAnimationID);

	int GetBackgroundMaterialID(void) const;
	int GetBackgroundAnimationID(void) const;

	MaterialInstance& GetBackgroundStatic(void);
	const MaterialInstance& GetBackgroundStaticConst(void) const;

	MaterialInstance& GetBackgroundAnimated(void);
	const MaterialInstance& GetBackgroundAnimatedConst(void) const;

	//
	// Tile Templates
	//

	TileStatic& GetTileTemplateStatic(int nIndex);
	const TileStatic& GetTileTemplateStaticConst(int nIndex) const;

	int GetTileTemplateStaticCount(void) const;

	TileAnimated& GetTileTemplateAnimated(int nIndex);
	const TileAnimated& GetTileTemplateAnimatedConst(int nIndex) const;

	int GetTileTemplateAnimatedCount(void) const;

	TileStatic* SetTileTemplateStatic(int nIndex,
									  DWORD dwFlags,
									  D3DCOLOR clrBlend,
									  int nMaterialID,
									  POINT ptTextureCoords);

	TileStatic* SetTileTemplateStatic(int nIndex, const TileStatic& rTemplate);

	TileAnimated* SetTileTemplateAnimated(int nIndex,
										  int nAnimationID,
										  DWORD dwFlags,
										  D3DCOLOR clrBlend,
										  bool bLoop = true,
										  int nStartFrame = 0,
										  bool bPlay = true,
										  bool bReverse = false,
										  float fSpeed = 1.0f);

	TileAnimated* SetTileTemplateAnimated(int nIndex, const TileAnimated& rTemplate);

	void Optimize(void);

	void RemoveAllTileTemplates(void);

	//
	// Tile Layers
	//

	virtual TileLayer* CreateLayer(void);

	int InsertLayer(TileLayer* pLayer, int nIndex = INVALID_INDEX);
	void RemoveLayer(int nIndex);
	void RemoveAllLayers(void);
	int GetLayerCount(void) const;

	TileLayer* GetLayer(int nIndex = 0);
	const TileLayer* GetLayerConst(int nIndex = 0) const;
	
	//
	// Actors
	//

	virtual Actor* CreateActor(LPCWSTR pszClass, LPCWSTR pszName);

	void AddActor(Actor* pActor);
	Actor* AddActor(LPCWSTR pszClass, LPCWSTR pszName);
	void RemoveActor(Actor* pActor);
	void RemoveAllActors(void);
	int GetActorCount(void) const;

	Actor* GetActor(LPCWSTR pszName);
	const Actor* GetActorConst(LPCWSTR pszName) const;

	ActorMapIterator GetBeginActorPos(void);
	ActorMapIterator GetEndActorPos(void);

	ActorMapConstIterator GetBeginActorPosConst(void) const;
	ActorMapConstIterator GetEndActorPosConst(void) const;

	Actor* GetPlayerActor(void);
	const Actor* GetPlayerActorConst(void) const;
	void SetPlayerActor(Actor* pPlayer);

	//
	// Spacial Database
	//

	int GetLayersFromPosition(int x, int y, TileLayerArray& rarLayers) const;
	int GetLayersFromPosition(float x, float y, TileLayerArray& rarLayers) const;
	int GetLayersFromRange(Rect& rcTileRange, TileLayerArray& rarLayers) const;

	int GetActorsFromPosition(int x, int y, ActorArray& rarActors) const;
	int GetActorsFromPosition(float x, float y, ActorArray& rarActors) const;
	int GetActorsFromRange(Rect& rcTileRange, ActorArray& rarActors) const;

	//
	// Cameras
	//

	virtual Camera* CreateCamera(void);

	int AddCamera(Camera* pCamera);
	void RemoveCamera(int nCamera);
	void RemoveAllCameras(void);
	int GetCameraCount(void) const;

	Camera* GetCamera(int nCamera);
	const Camera* GetCameraConst(int nCamera) const;

	Camera* GetActiveCamera(int nActiveCamera = 0);
	const Camera* GetActiveCameraConst(int nActiveCamera = 0) const;
	int GetActiveCameraCount(void) const;

	void SetActiveCamera(Camera* pCamera);
	void RemoveActiveCamera(int nActiveCamera);
	void RemoveActiveCamera(Camera* pCamera);
	void RemoveAllActiveCameras(void);

	//
	// Resources
	//

	int AddMaterial(Material* pTexture);
	int FindMaterial(Material* pMaterial) const;
	void RemoveMaterial(int nMaterialID);
	void RemoveAllMaterials(void);
	Material* GetMaterial(int nMaterialID);
	const Material* GetMaterialConst(int nMaterialID) const;
	int GetMaterialCount(void) const;

	int AddAnimation(Animation* pAnimation);
	int FindAnimation(Animation* pAnimation) const;
	void RemoveAnimation(int nAnimationID);
	void RemoveAllAnimations(void);
	Animation* GetAnimation(int nAnimationID);
	const Animation* GetAnimationConst(int nAnimationID) const;
	int GetAnimationCount(void) const;

	int AddSound(Sound* pSound);
	int FindSound(Sound* pSound) const;
	void RemoveSound(int nSoundID);
	void RemoveAllSounds(void);
	Sound* GetSound(int nSoundID);
	const Sound* GetSoundConst(int nSoundID) const;
	int GetSoundCount(void) const;

	int AddMusic(Music* pMusic);
	int FindMusic(Music* pMusic) const;
	void RemoveMusic(int nMusicID);
	void RemoveAllMusic(void);
	Music* GetMusic(int nMusicID);
	const Music* GetMusicConst(int nMusicID) const;
	int GetMusicCount(void) const;

	//
	// Variables
	//

	VariableManager& GetVariables(void);
	const VariableManager& GetVariablesConst(void) const;

	//
	// Rendering
	//

	virtual void Render(void);

	//
	// Update
	//

	virtual void Update(void);

	//
	// Serialization
	//
	
	void Serialize(LPCWSTR pszPath) const;

	void Serialize(Stream& rStream, bool bInstance) const;
	void Deserialize(Stream& rStream, bool bInstance);
	
	void SerializeInstance(Stream& rStream) const;
	void DeserializeInstance(Stream& rStream);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	DWORD GetActorsMemoryFootprint(void) const;
	DWORD GetLayersMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);

	//
	// Handlers
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);

	virtual void OnSessionPause(bool bPause);

	virtual void OnKeyDown(int nKeyCode);
	virtual void OnKeyUp(int nKeyCode);
	virtual void OnKeyPress(int nAsciiCode);

	virtual void OnMouseMove(POINT pt);
	virtual void OnMouseLDown(POINT pt);
	virtual void OnMouseLUp(POINT pt);
	virtual void OnMouseLDbl(POINT pt);
	virtual void OnMouseRDown(POINT pt);
	virtual void OnMouseRUp(POINT pt);
	virtual void OnMouseRDbl(POINT pt);
	virtual void OnMouseMDown(POINT pt);
	virtual void OnMouseMUp(POINT pt);
	virtual void OnMouseMDbl(POINT pt);
	virtual void OnMouseWheel(int nZDelta);

protected:
	//
	// Serialization
	//

	void SerializeHeader(Stream& rStream) const;
	void DeserializeHeader(Stream& rStream);

	void SerializeMaterials(Stream& rStream) const;
	void DeserializeMaterials(Stream& rStream);

	void SerializeAnimations(Stream& rStream) const;
	void DeserializeAnimations(Stream& rStream);

	void SerializeSounds(Stream& rStream) const;
	void DeserializeSounds(Stream& rStream);

	void SerializeMusic(Stream& rStream) const;
	void DeserializeMusic(Stream& rStream);

	void SerializeTiles(Stream& rStream) const;
	void DeserializeTiles(Stream& rStream);

	void SerializeLayers(Stream& rStream) const;
	void DeserializeLayers(Stream& rStream);

	void SerializeActors(Stream& rStream) const;
	void DeserializeActors(Stream& rStream);

	virtual void SerializeUserData(Stream& rStream, bool bInstance) const;
	virtual void DeserializeUserData(Stream& rStream, bool bInstance);

	//
	// Friends
	//

	friend class Actor;
};

/*----------------------------------------------------------*\
| TileMapManager class
\*----------------------------------------------------------*/

class TileMapManager
{
private:
	Engine& m_rEngine;
	TileMapArray m_arMaps;

	WCHAR m_szBasePath[128];
	WCHAR m_szBaseExt[8];

public:
	TileMapManager(Engine& rEngine);
	~TileMapManager(void);

public:
	//
	// Base Path and Extension
	//

	inline LPCWSTR GetBasePath(void) const
	{
		return m_szBasePath;
	}

	inline void SetBasePath(LPCWSTR pszBasePath)
	{
		wcscpy_s(m_szBasePath,
			sizeof(m_szBasePath) / sizeof(WCHAR), pszBasePath);
	}

	inline LPCWSTR GetBaseExtension(void) const
	{
		return m_szBaseExt;
	}

	inline void SetBaseExtension(LPCWSTR pszBaseExt)
	{
		wcscpy_s(m_szBaseExt,
			sizeof(m_szBaseExt) / sizeof(WCHAR), pszBaseExt);
	}

	//
	// Management
	//

	TileMap* Load(LPCWSTR pszPath, bool bInstance = false, bool bCheckExists = true, bool bReload = false);
	TileMap* LoadInstance(Stream& rStream, bool bCheckExists = true);

	void Save(TileMap* pMap, LPCWSTR pszPath) const;

	TileMap* Create(LPCWSTR pszClass = NULL);

	void Add(TileMap* pMap);
	TileMap* Add(LPCWSTR pszClass = NULL);

	TileMap* Find(LPCWSTR pszPath);
	const TileMap* FindConst(LPCWSTR pszPath) const;

	TileMap* FindPattern(LPCWSTR pszPattern);
	const TileMap* FindPatternConst(LPCWSTR pszPattern) const;

	inline int GetCount(void) const
	{
		return int(m_arMaps.size());
	}

	inline bool IsEmpty(void) const
	{
		return m_arMaps.empty();
	}

	void Remove(TileMap* pMap);
	void RemoveAll(void);

	//
	// Iteration
	//

	inline TileMapArrayIterator GetBeginPos(void)
	{
		return m_arMaps.begin();
	}

	inline TileMapArrayIterator GetEndPos(void)
	{
		return m_arMaps.end();
	}

	inline TileMapArrayConstIterator GetBeginPosConst(void) const
	{
		return m_arMaps.begin();
	}

	inline TileMapArrayConstIterator GetEndPosConst(void) const
	{
		return m_arMaps.end();
	}

	//
	// Update
	//

	void Update(void);

	//
	// Serialization
	//

	void Serialize(Stream& rStream) const;
	void Deserialize(Stream& rStream);

	//
	// Device Events
	//

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);
	
	//
	// Diagnostics
	//

	DWORD GetMemoryFootprint(void) const;
	
	//
	// Deinitialization
	//

	inline void Empty(void)
	{
		RemoveAll();
	}
};

} // namespace ThunderStorm

#endif // THUNDER_TILE_MAP_H