/*------------------------------------------------------------------*\
|
| ThunderMaterial.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine material class(es)
| Created: 06/10/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_MATERIAL_H
#define THUNDER_MATERIAL_H

/*----------------------------------------------------------*\
| Overrides
\*----------------------------------------------------------*/

#pragma warning (disable: 4201)

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderTexture.h"		// using Texture
#include "ThunderAnimation.h"	// using Animation
#include "ThunderStates.h"		// using StateBlock

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Effect;					// referencing Effect, declared below
class EffectTechnique;			// referencing EffectTechnique, declared below
class EffectParameterInfo;		// referencing EffectParameterInfo, declared below
class EffectParameter;			// referencing EffectParameter, declared below
class Material;					// referencing Material, declared below
class MaterialInstance;			// referencing MaterialInstance, declared below
class MaterialInstancePool;		// referencing MaterialInstancePool, declared below
class MaterialInstanceShared;	// referencing MaterialInstanceShared, declared below
class InfoElem;					// referencing InfoElem

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Effect*> EffectArray;
typedef std::vector<Effect*>::iterator EffectArrayIterator;
typedef std::vector<Effect*>::const_iterator EffectArrayConstIterator;

typedef std::vector<EffectParameterInfo*> EffectParameterInfoArray;
typedef std::vector<EffectParameterInfo*>::iterator EffectParameterInfoArrayIterator;
typedef std::vector<EffectParameterInfo*>::const_iterator EffectParameterInfoArrayConstIterator;

typedef std::vector<EffectParameter> EffectParameterArray;
typedef std::vector<EffectParameter>::iterator EffectParameterArrayIterator;
typedef std::vector<EffectParameter>::const_iterator EffectParameterArrayConstIterator;

typedef std::multimap<Material*, MaterialInstanceShared> MaterialInstanceSharedMap;
typedef std::multimap<Material*, MaterialInstanceShared>::iterator MaterialInstanceSharedMapIterator;
typedef std::multimap<Material*, MaterialInstanceShared>::const_iterator MaterialInstanceSharedMapConstIterator;
typedef std::pair<MaterialInstanceSharedMapIterator, MaterialInstanceSharedMapIterator> MaterialInstanceSharedMapRange;

typedef std::map<String, EffectParameterInfo*> EffectParameterInfoMap;
typedef std::map<String, EffectParameterInfo*>::iterator EffectParameterInfoMapIterator;
typedef std::map<String, EffectParameterInfo*>::const_iterator EffectParameterInfoMapConstIterator;

typedef std::map<String, D3DXHANDLE> EffectHandleMap;
typedef std::map<String, D3DXHANDLE>::iterator EffectHandleMapIterator;
typedef std::map<String, D3DXHANDLE>::const_iterator EffectHandleMapConstIterator;

typedef std::map<String, EffectTechnique*> EffectTechniqueMap;
typedef std::map<String, EffectTechnique*>::iterator EffectTechniqueMapIterator;
typedef std::map<String, EffectTechnique*>::const_iterator EffectTechniqueMapConstIterator;


/*----------------------------------------------------------*\
| Effect class - wrapper around D3DXEffect
\*----------------------------------------------------------*/

class Effect: public Resource
{
private:
	//
	// Members
	//

	// Wrap D3D Effect functionality
	LPD3DXEFFECT m_pD3DEffect;

	// Size of compiled effect
	DWORD m_dwSize;

	// Has automatic parameters? (cached)
	bool m_bHasAutoParams;
	
	// Automatic target texture param (cached if exists)
	EffectParameterInfo* m_pAutoTargetParam;

	// Base texture param (cached if exists)
	EffectParameterInfo* m_pBaseParam;

	// Keep track of current technique to reduce state changes
	D3DXHANDLE m_hTechnique;

	// Remember effect description
	D3DXEFFECT_DESC m_desc;

	// Remember all automatic parameters
	EffectParameterInfoArray m_arAutoParams;

	// Remember all parameters by name
	EffectParameterInfoMap m_mapParams;

	// Remember all techniques by name
	EffectTechniqueMap m_mapTechniques;

public:
	Effect(Engine& rEngine);
	virtual ~Effect(void);

public:
	//
	// Effect
	//

	LPD3DXEFFECT GetD3DXEffect(void)
	{
		return m_pD3DEffect;
	}

	const D3DXEFFECT_DESC& GetInfo(void) const
	{
		return m_desc;
	}

	void SetTechnique(D3DXHANDLE hTechnique);

	//
	// Technique Info
	//

	inline EffectTechnique* GetTechnique(LPCWSTR pszName)
	{
		EffectTechniqueMapIterator pos =
			m_mapTechniques.find(pszName);

		if (pos != m_mapTechniques.end())
			return pos->second;

		return NULL;
	}

	inline const EffectTechnique* GetTechniqueCOnst(LPCWSTR pszName)
	{
		EffectTechniqueMapConstIterator pos =
			m_mapTechniques.find(pszName);

		if (pos != m_mapTechniques.end())
			return pos->second;

		return NULL;
	}

	inline EffectTechniqueMapIterator GetBeginTechniquePos(void)
	{
		return m_mapTechniques.begin();
	}

	inline EffectTechniqueMapConstIterator GetBeginTechniquePosConst(void) const
	{
		return m_mapTechniques.begin();
	}

	inline EffectTechniqueMapIterator GetEndTechniquePos(void)
	{
		return m_mapTechniques.end();
	}

	inline EffectTechniqueMapConstIterator GetEndTechniquePosConst(void)
	{
		return m_mapTechniques.end();
	}

	//
	// Parameter Info
	//

	EffectParameterInfo* GetParameterInfo(LPCWSTR pszParameter);
	const EffectParameterInfo* GetParameterInfoConst(LPCWSTR pszParameter) const;

	inline EffectParameterInfoMapConstIterator GetBeginParameterInfoPos(void) const
	{
		return m_mapParams.begin();
	}

	inline EffectParameterInfoMapConstIterator GetEndParameterInfoPos(void) const
	{
		return m_mapParams.end();
	}

	inline EffectParameterInfoArrayIterator GetBeginAutoParameterInfoPos(void)
	{
		return m_arAutoParams.begin();
	}

	inline EffectParameterInfoArrayIterator GetEndAutoParameterInfoPos(void)
	{
		return m_arAutoParams.end();
	}

	bool HasAutoParams(void) const
	{
		return m_bHasAutoParams;
	}

	EffectParameterInfo* GetTargetParamInfo(void)
	{
		return m_pAutoTargetParam;
	}

	EffectParameterInfo* GetBaseParamInfo(void)
	{
		return m_pBaseParam;
	}

	//
	// Device Events
	//

	virtual void OnLostDevice(bool bRecreate);
	virtual void OnResetDevice(bool bRecreate);
	
	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);
	
	//
	// Friends
	//

	friend class EffectParameterInfo;

private:
	//
	// Private Functions
	//

	void Compile(LPCWSTR pszPath);
};

/*----------------------------------------------------------*\
| EffectTechnique class - technique info
\*----------------------------------------------------------*/

class EffectTechnique
{
private:
	//
	// Members
	//

	D3DXHANDLE m_hTechnique;
	UINT m_uTechniqueIndex;
	D3DXTECHNIQUE_DESC m_desc;
	StateBlock m_RestoreStates;

public:
	EffectTechnique(Effect& rEffect, UINT uTechniqueIndex);

public:
	inline D3DXHANDLE GetHandle(void) const
	{
		return m_hTechnique;
	}

	inline UINT GetIndex(void) const
	{
		return m_uTechniqueIndex;
	}

	inline void CacheHandle(D3DXHANDLE handle)
	{
		m_hTechnique = handle;
	}

	inline const D3DXTECHNIQUE_DESC& GetInfo(void) const
	{
		return m_desc;
	}

	inline StateBlock& GetRestoreStates(void)
	{
		return m_RestoreStates;
	}

public:
	inline EffectTechnique& operator=(const EffectTechnique& rAssign)
	{
		m_hTechnique = rAssign.m_hTechnique;

		CopyMemory(&m_desc, &rAssign.m_desc, sizeof(D3DXTECHNIQUE_DESC));
	}
};

/*----------------------------------------------------------*\
| EffectPool class
\*----------------------------------------------------------*/

class EffectPool
{
private:
	//
	// Members
	//

	// Shared effect parameter pool
	LPD3DXEFFECTPOOL m_pD3DXEffectPool;

	// Effect parameters in shared pool (non-owned pointers)
	EffectParameterInfoArray m_arPoolParams;

public:
	EffectPool(void);
	~EffectPool(void);

public:
	//
	// Initialization
	//

	void Initialize(void);

	//
	// D3DX Handle
	//

	LPD3DXEFFECTPOOL GetD3DXEffectPool(void);

	//
	// Shared Parameters
	//

	EffectParameterInfo* Add(EffectParameterInfo* pParam);
	void Remove(EffectParameterInfo* pParam);

	//
	// Deinitialization
	//

	void Empty(void);
};

/*----------------------------------------------------------*\
| EffectParameterInfo class
\*----------------------------------------------------------*/

class EffectParameterInfo
{
public:
	//
	// Constants
	//

	// Supported shader parameter types

	enum Types
	{
		// Unresolved
		TYPE_UNKNOWN,

		// Texture pointer
		TYPE_TEXTURE,

		// TextureCube pointer
		TYPE_TEXTURECUBE,

		// 4d float color vector
		TYPE_COLOR,

		// float scalar
		TYPE_SCALAR,

		// 2d float vector
		TYPE_VECTOR2,

		// 4d float vector
		TYPE_VECTOR4,

		// 4x4 float matrix
		TYPE_MATRIX4X4
	};

	// Supported semantics

	enum Semantics			
	{
		// None
		SEMANTIC_NONE,

		// Base texture (hint)
		SEMANTIC_BASETEXTURE,

		// Target texture (automatic) - TODO
		SEMANTIC_TARGETTEXTURE,

		// World transform (automatic)
		SEMANTIC_WORLD,

		// View transform (automatic)
		SEMANTIC_VIEW,

		// Projection transform (automatic)
		SEMANTIC_PROJ,

		// View + Projection transform (automatic)
		SEMANTIC_VIEWPROJ,

		// World + View + Projection transform (automatic)
		SEMANTIC_WORLDVIEWPROJ,

		// Session time (automatic)
		SEMANTIC_TIME,

		// Run time (automatic)
		SEMANTIC_RUNTIME,

		// Delta frame time (automatic)
		SEMANTIC_FRAMETIME,

		// Size of currently selected render target (automatic)
		SEMANTIC_TARGETSIZE,

		// Number of semantics defined
		SEMANTIC_COUNT
	};

	static const char SZ_SEMANTIC_COLOR[];
	static const LPCSTR SZ_SEMANTIC[];

private:
	//
	// Member
	//

	// Effect parameter handle
	D3DXHANDLE m_hParam;

	// Effect parameter index
	UINT m_uParamIndex;

	// Effect parameter type
	Types m_nType;

	// Effect parameter semantic
	Semantics m_nSemantic;

	// Effect parameter is shared?
	bool m_bShared;

	// Frame ID when last changed, used to limit shader param changes
	DWORD m_dwLastFrameChanged;

	// Effect parameter name
	String m_strName;

	// Effects that use this parameter, if shared
	EffectArray m_arRefs;

public:
	EffectParameterInfo(Effect* pEffect, UINT uParamIndex);
	EffectParameterInfo(const EffectParameterInfo& rInit);

public:
	//
	// Type
	//

	inline Types GetType(void) const
	{
		return m_nType;
	}

	//
	// Semantic (for auto set)
	//

	inline Semantics GetSemantic(void) const
	{
		return m_nSemantic;
	}

	inline bool IsAutomatic(void) const
	{
		return (m_nSemantic >= SEMANTIC_TARGETTEXTURE);
	}

	//
	// Name
	//

	inline const String& GetName(void) const
	{
		return m_strName;
	}

	//
	// Shared
	//

	inline bool IsShared(void) const
	{
		return m_bShared;
	}

	//
	// Change Tracking
	//

	inline DWORD GetLastFrameChanged(void) const
	{
		return m_dwLastFrameChanged;
	}

	inline void SetLastFrameChanged(DWORD dwLastFrameChanged)
	{
		m_dwLastFrameChanged = dwLastFrameChanged;
	}

	//
	// Handle
	//

	inline D3DXHANDLE GetHandle(void) const
	{
		return m_hParam;
	}

	inline UINT GetIndex(void) const
	{
		return m_uParamIndex;
	}

	inline void CacheHandle(D3DXHANDLE handle)
	{
		m_hParam = handle;
	}

	//
	// Operators
	//

	EffectParameterInfo& operator=(const EffectParameterInfo& rAssign);

	//
	// Deinitialization
	//

	void Release(Effect* pOwner);

	//
	// Friends
	//
	
	friend class EffectParameter;
};

/*----------------------------------------------------------*\
| EffectParameter class
\*----------------------------------------------------------*/

class EffectParameter
{
private:
	//
	// Members
	//

	// Type of parameter, handle, etc.
	EffectParameterInfo* m_pInfo;

	union
	{
		// Texture value
		Texture* m_pTexture;

		// Texture cube value
		TextureCube* m_pTextureCube;

		// Scalar value
		float m_fScalar;

		// 2D vector value
		float m_fVec2[2];

		// 4D vector value
		float m_fVec4[4];

		// Color value (4D vector)
		D3DCOLORVALUE m_crvColor;

		// Matrix4x4 value
		D3DMATRIX m_mtx;
	};

public:
	EffectParameter(EffectParameterInfo* pInfo);
	EffectParameter(const EffectParameter& rInit);
	~EffectParameter(void);

public:
	//
	// Info
	//

	inline const EffectParameterInfo* GetInfo(void) const
	{
		return m_pInfo;
	}

	//
	// Value
	//

	LPVOID GetValue(void);
	Texture* GetTexture(void);
	const Texture* GetTextureConst(void) const;
	TextureCube* GetTextureCube(void);
	const TextureCube* GetTextureCubeConst(void) const;

	inline D3DCOLORVALUE GetColor(void)
	{
		return m_crvColor;
	}

	inline float GetScalar(void)
	{
		return m_fScalar;
	}

	inline D3DXVECTOR2 GetVector2(void)
	{
		return D3DXVECTOR2(m_fVec2);
	}

	D3DXVECTOR4 GetVector4(void)
	{
		return D3DXVECTOR4(m_fVec4);
	}

	D3DXMATRIX GetMatrix(void)
	{
		return D3DXMATRIX(m_mtx);
	}

	void SetValue(LPVOID pvValue);
	void SetTexture(Texture* pTexture);
	void SetTextureCube(TextureCube* pTextureCube);
	void SetColor(D3DCOLORVALUE crvColor);
	void SetColor(D3DCOLOR clrColor);
	void SetScalar(float fScalar);
	void SetVector2(const D3DXVECTOR2& rVector2);
	void SetVector4(const D3DXVECTOR4& rVector4);
	void SetMatrix(const D3DXMATRIX& rMatrix);

	void Apply(void);

	//
	// Serialization
	//

	void Serialize(const Engine& rEngine, InfoElem& rRoot) const;
	void Deserialize(Engine& rEngine, const InfoElem& rRoot);

	void Serialize(const Engine& rEngine, Stream& rStream) const;
	void Deserialize(Engine& rEngine, Stream& rStream);

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	bool operator==(const EffectParameter& rCompare) const;
	EffectParameter& operator=(const EffectParameter& rAssign);
};

/*----------------------------------------------------------*\
| Material class - Effect with static parameters
\*----------------------------------------------------------*/

class Material: public Resource
{
public:
	//
	// Constants
	//

	static const WCHAR SZ_MATERIAL[];
	static const WCHAR SZ_EFFECT[];
	static const WCHAR SZ_TECHNIQUE[];
	static const WCHAR SZ_PARAMS[];
	static const WCHAR SZ_REGIONSET[];
	static const WCHAR SZ_META[];
	static const char SZ_TECHNIQUE_FIRSTVALID[];

private:
	//
	// Members
	//

	// Effect used
	Effect* m_pEffect;						

	// Technique used
	EffectTechnique* m_pTechnique;

	// Per-pixel collision regions (optional)
	RegionSet* m_pRegionSet;

	// Static effect parameters. TODO: map instead of vector
	EffectParameterArray m_arStaticParams;

	// Meta data
	VariableMap m_mapVariables;

public:
	Material(Engine& rEngine);
	~Material(void);

public:
	//
	// Effect
	//

	Effect* GetEffect(void)
	{
		return m_pEffect;
	}

	const Effect* GetEffectConst(void) const
	{
		return m_pEffect;
	}

	//
	// Technique
	//

	EffectTechnique* GetTechnique(void) const
	{
		return m_pTechnique;
	}

	const EffectTechnique* GetTechniqueConst(void) const
	{
		return m_pTechnique;
	}

	//
	// Region Set
	//

	RegionSet* GetRegionSet(void)
	{
		return m_pRegionSet;
	}

	const RegionSet* GetRegionSetConst(void) const
	{
		return m_pRegionSet;
	}

	//
	// Static Parameters
	//

	EffectParameter* GetParameter(LPCWSTR pszName);
	EffectParameter* GetParameter(const EffectParameterInfo* pInfo);

	const EffectParameter* GetParameterConst(LPCWSTR pszName) const;
	const EffectParameter* GetParameterConst(const EffectParameterInfo* pInfo) const;

	EffectParameter* GetBaseParameter(void);
	const EffectParameter* GetBaseParameterConst(void) const;

	EffectParameter* GetTargetParameter(void);
	const EffectParameter* GetTargetParameterConst(void) const;
	
	void SetParameter(EffectParameter& rCopyFrom);

	inline EffectParameterArrayIterator GetBeginParameterPos(void)
	{
		return m_arStaticParams.begin();
	}

	inline EffectParameterArrayConstIterator GetBeginParameterPosConst(void) const
	{
		return m_arStaticParams.begin();
	}

	inline EffectParameterArrayIterator GetEndParameterPos(void)
	{
		return m_arStaticParams.end();
	}

	inline EffectParameterArrayConstIterator GetEndParameterPosConst(void) const
	{
		return m_arStaticParams.end();
	}

	inline int GetParameterCount(void) const
	{
		return int(m_arStaticParams.size());
	}

	//
	// Variables
	//

	const Variable* GetVariable(LPCWSTR pszName) const;
	
	VariableMapConstIterator GetBeginVariablePos(void) const
	{
		return m_mapVariables.begin();
	}

	VariableMapConstIterator GetEndVariablePos(void) const
	{
		return m_mapVariables.end();
	}

	//
	// Serialization
	//

	virtual void Deserialize(LPCWSTR pszPath);
	virtual void Deserialize(Stream& rStream);
	virtual void Deserialize(const InfoElem& rRoot);

	virtual void Serialize(LPCWSTR pszPath) const;
	virtual void Serialize(Stream& rStream) const;
	virtual void Serialize(InfoElem& rRoot) const;

	//
	// Diagnostics
	//

	virtual DWORD GetMemoryFootprint(void) const;

	//
	// Deinitialization
	//

	virtual void Empty(void);
	virtual void Remove(void);

	//
	// Operators
	//

	Material& operator=(const Material& rCopy);
};

/*----------------------------------------------------------*\
| MaterialInstanceShared class
\*----------------------------------------------------------*/

class MaterialInstanceShared
{
private:
	//
	// Members
	//

	// Keep a reference to manager for garbage collection
	MaterialInstancePool& m_rManager;

	// Number of times referenced
	int m_nRefs;

	// Material used
	Material* m_pMaterial;

	// Parameter block with static & dynamic parameters recorded
	D3DXHANDLE m_hParams;

	// Dynamic parameters
	EffectParameterArray m_arParams;

	// Cached base texture parameter
	EffectParameter* m_pBaseParam;

	// Cache target texture parameter
	EffectParameter* m_pTargetParam;

public:
	MaterialInstanceShared(Material* pMaterial);
	MaterialInstanceShared(const MaterialInstanceShared& rInit);
	~MaterialInstanceShared(void);

public:
	//
	// Manager
	//

	inline MaterialInstancePool& GetManager(void)
	{
		return m_rManager;
	}

	//
	// Material
	//

	inline Material* GetMaterial(void)
	{
		return m_pMaterial;
	}

	inline const Material* GetMaterialConst(void) const
	{
		return m_pMaterial;
	}

	// Technique

	inline EffectTechnique* GetTechnique(void)
	{
		return m_pMaterial->GetTechnique();
	}

	inline const EffectTechnique* GetTechniqueConst(void) const
	{
		return m_pMaterial->GetTechniqueConst();
	}

	//
	// Parameters
	//

	EffectParameter* GetParameter(LPCWSTR pszParamName);
	const EffectParameter* GetParameterConst(LPCWSTR pszParamName) const;

	EffectParameter* GetParameter(const EffectParameterInfo* pInfo);
	const EffectParameter* GetParameterConst(const EffectParameterInfo* pInfo) const;

	void SetParameter(EffectParameter& rCopyFrom);

	inline EffectParameter* GetBaseParam(void)
	{
		return m_pBaseParam;
	}

	inline EffectParameter* GetTargetParam(void)
	{
		return m_pTargetParam;
	}

	inline EffectParameterArrayIterator GetBeginParameterPos(void)
	{
		return m_arParams.begin();
	}

	inline EffectParameterArrayConstIterator GetBeginParameterPosConst(void) const
	{
		return m_arParams.begin();
	}

	inline EffectParameterArrayIterator GetEndParameterPos(void)
	{
		return m_arParams.end();
	}

	inline EffectParameterArrayConstIterator GetEndParameterPosConst(void) const
	{
		return m_arParams.end();
	}

	inline int GetParameterCount(void) const
	{
		return int(m_arParams.size());
	}
	
	void SetValue(LPCWSTR pszParamName, LPVOID pvValue);
	void SetTexture(LPCWSTR pszParamName, Texture* pTexture);
	void SetTextureCube(LPCWSTR pszParamName, TextureCube* pTextureCube);
	void SetColor(LPCWSTR pszParamName, D3DCOLOR clrColor);
	void SetColor(LPCWSTR pszParamName, D3DCOLORVALUE crvColor);
	void SetScalar(LPCWSTR pszParamName, float fScalar);
	void SetVector2(LPCWSTR pszParamName, const D3DXVECTOR2& rVector2);
	void SetVector4(LPCWSTR pszParamName, const D3DXVECTOR4& rVector4);
	void SetMatrix(LPCWSTR pszParamName, const D3DXMATRIX& rMatrix);

	void RemoveParameter(LPCWSTR pszParamName);
	void RemoveParameter(const EffectParameterInfo* pInfo);

	void Apply(void);
	void ApplyChanges(void);

	inline bool IsDirty(void) const
	{
		return NULL == m_hParams;
	}
	
	//
	// Rendering
	//

	UINT Begin(void);
	void BeginPass(UINT nPass);
	void CommitChanges(void);
	void EndPass(void);
	void End(void);

	//
	// Device Reset
	//

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);

	//
	// Reference Counting
	//

	inline int GetRefCount(void) const
	{
		return m_nRefs;
	}

	int AddRef(void);

	//
	// Operators
	//

	bool operator==(const MaterialInstanceShared& rCompare) const;
	bool operator<(const MaterialInstanceShared& rCompare) const;
	MaterialInstanceShared& operator=(const MaterialInstanceShared& rAssign);

	//
	// Deinitialization
	//

	void Empty(void);

	void Release(void);

	//
	// Private Functions
	//

private:
	void CaptureParameterBlock(void);
	void ApplyParameterBlock(void);
	void ReleaseParameterBlock(void);

	void ApplyAutoParameters(void);
};

/*----------------------------------------------------------*\
| MaterialInstance class
\*----------------------------------------------------------*/

class MaterialInstance
{
public:
	//
	// Constants
	//

	enum Flags
	{
		// This instance is static, use Static members to render
		STATIC = 0,

		// This instance is animated, use Animated members to render
		ANIMATED = 1 << 0,

		// Animation is playing
		PLAYING = 1 << 1,

		// Animation should advance backward
		REVERSE = 1 << 2,

		// Animation should wrap
		LOOPING = 1 << 3,

		// Next sequence triggered, played after current is over
		TRIGGERED = 1 << 4,

		// Flag set when waiting for triggered sequence to end
		WAITSIGNALED = 1 << 5,

		// Flag set when a triggered sequence is done playing
		SIGNALED = 1 << 6
	};

	// Element names

	static const WCHAR SZ_TEXCOORDS[];
	static const WCHAR SZ_FLAGS[];
	static const WCHAR SZ_CURFRAME[];
	static const WCHAR SZ_CURSEQ[];
	static const WCHAR SZ_NEXTSEQ[];
	static const WCHAR SZ_ANIM[];
	static const WCHAR SZ_ANIM_FRAME[];
	static const WCHAR SZ_ANIM_PLAY[];
	static const WCHAR SZ_ANIM_LOOP[];
	static const WCHAR SZ_ANIM_REVERSE[];
	static const DWORD DW_FLAGS[];
	static const LPCWSTR SZ_FLAGNAMES[];

protected:
	//
	// Members
	//

	// Flags that specify if animated and animation control
	DWORD m_dwFlags;

	// Combination of material and dynamic parameters used
	MaterialInstanceShared* m_pSharedInstance;

	union
	{
		// Animated

		struct
		{
			// Animation using, if animated
			Animation* m_pAnimation;

			// Current frame index
			int m_nFrameID;

			// Current sequence index
			WORD m_wSequenceID;

			// Triggered sequence index
			WORD m_wNextSequenceID;

			// Time of last frame change
			float m_fLastUpdateTime;
		};

		// Static

		struct
		{
			// Texture coordinates used if static
			RECT m_rcTextureCoords;
		};
	};

public:
	MaterialInstance(void);
	MaterialInstance(const MaterialInstance& rInit);
	MaterialInstance(Material* pMaterial, const Rect& rrcTextureCoords);
	MaterialInstance(Material* pMaterial, Texture* pBaseTexture,
		const Rect& rrcTextureCoords);
	MaterialInstance(Material* pMaterial, Texture* pBaseTexture);
	MaterialInstance(Material* pMaterial);
	MaterialInstance(Animation* pAnimation);
	~MaterialInstance(void);

public:
	//
	// Material
	//

	inline Material* GetMaterial(void) const
	{
		return (m_pSharedInstance != NULL ? m_pSharedInstance->GetMaterial() : NULL);
	}

	inline MaterialInstanceShared* GetSharedMaterial(void) const
	{
		return m_pSharedInstance;
	}

	inline bool IsEmpty(void) const
	{
		return (NULL == m_pSharedInstance);
	}

	void SetMaterial(Material* pMaterial);

	//
	// Material Parameters
	//

	const EffectParameter* GetParameter(LPCWSTR pszParamName) const;

	Texture* GetBaseTexture(void);
	const Texture* GetBaseTextureConst(void) const;
	
	void SetValue(LPCWSTR pszParamName, LPVOID pvValue);
	void SetTexture(LPCWSTR pszParamName, Texture* pTexture);
	void SetBaseTexture(Texture* pTexture);
	void SetTextureCube(LPCWSTR pszParamName, TextureCube* pTextureCube);
	void SetColor(LPCWSTR pszParamName, const Color& rColor);
	void SetScalar(LPCWSTR pszParamName, float fScalar);
	void SetVector2(LPCWSTR pszParamName, const D3DXVECTOR2& rVector2);
	void SetVector4(LPCWSTR pszParamName, const D3DXVECTOR4& rVector4);
	void SetMatrix(LPCWSTR pszParamName, const D3DXMATRIX& rMatrix);
	
	//
	// Animation
	//

	void SetAnimation(Animation* pAnimation);

	void Play(float fTime, bool bLoop = true, int nSequence = 0, bool bReverse = false);

	void TriggerSequence(int nSequenceID, bool bLoop = false);
	void TriggerSequence(LPCWSTR pszSequence, bool bLoop = false);

	void Update(float fTime);

	inline bool IsAnimated(void) const
	{
		return (m_dwFlags & ANIMATED) != 0;
	}

	inline bool IsTriggered(void) const
	{
		return (m_dwFlags & TRIGGERED) != 0;
	}

	inline bool IsTriggeredSequenceOver(void) const
	{
		return (m_dwFlags & SIGNALED) != 0;
	}

	inline Animation* GetAnimation(void)
	{
		return m_pAnimation;
	}	

	inline void Play(void)
	{
		m_dwFlags |= PLAYING;
	}

	inline void Stop(void)
	{
		m_dwFlags &= ~PLAYING;
	}

	inline int GetCurrentFrameIndex(void) const
	{
		return m_nFrameID;
	}

	inline Frame* GetCurrentFrame(void)
	{
		return m_pAnimation != NULL ?
			&m_pAnimation->GetFrame(m_nFrameID) : NULL;
	}

	inline const Frame* GetCurrentFrameConst(void) const
	{
		return m_pAnimation != NULL ?
			&m_pAnimation->GetFrame(m_nFrameID) : NULL;
	}

	inline int GetCurrentSequenceIndex(void) const
	{
		return int(m_wSequenceID);
	}

	inline bool IsPlaying(void) const
	{
		return (m_dwFlags & PLAYING) != 0;
	}

	inline bool IsStopped(void) const
	{
		return (~m_dwFlags & PLAYING) != 0;
	}

	inline bool IsLooping(void) const
	{
		return (m_dwFlags & LOOPING) != 0;
	}

	void SetLooping(bool bLoop)
	{
		if (true == bLoop)
			m_dwFlags |= LOOPING;
		else
			m_dwFlags &= ~LOOPING;
	}

	inline bool IsReverse(void) const
	{
		return (m_dwFlags & REVERSE) != 0;
	}

	void SetReverse(bool bReverse)
	{
		if (true == bReverse)
			m_dwFlags |= REVERSE;
		else
			m_dwFlags &= ~REVERSE;
	}

	//
	// Texture Coordinates
	//

	Rect GetTextureCoords(void) const
	{
		if (m_dwFlags & ANIMATED)
		{
			return (NULL == m_pAnimation) ? Rect() : Rect(
				m_pAnimation->GetFrame(m_nFrameID).GetTextureCoordsConst(),
				m_pAnimation->GetFrameSize());
		}
		
		return m_rcTextureCoords;
	}
	
	void GetTextureCoords(float& ru1, float& rv1,
		float& ru2, float& rv2) const;

	void SetTextureCoords(const RECT& rrcTextureCoords);

	void SetTextureCoords(int x, int y, int nWidth, int nHeight);

	void SetTextureCoords(float u1, float v1, float u2, float v2);

	//
	// Serialization
	//

	void Serialize(const Engine& rEngine, Stream& rStream, bool bEmbed) const;
	void Deserialize(Engine& rEngine, Stream& rStream, bool bEmbed);

	void Serialize(Engine& rEngine, InfoElem& rRoot) const;
	void Deserialize(Engine& rEngine, const InfoElem& rRoot);

	//
	// Deinitialization
	//

	void Empty(void);

	//
	// Operators
	//

	MaterialInstance& operator=(const MaterialInstance& rAssign);

	bool operator==(const MaterialInstance& rCompare) const;	
	bool operator!=(const MaterialInstance& rCompare) const;

private:
	//
	// Private Functions
	//

	void UpdateSharedInstance(MaterialInstanceShared& rDirtyCopy);
};

/*----------------------------------------------------------*\
| MaterialInstancePool class
\*----------------------------------------------------------*/

class MaterialInstancePool
{
private:
	//
	// Members
	//

	// Shared material instances
	MaterialInstanceSharedMap m_mapInstances;

public:
	//
	// Shared Material Instances
	//

	MaterialInstanceShared* Add(MaterialInstanceShared& rInstance);
	void Remove(MaterialInstanceShared* pInstance);
	int GetCount(void) const;

	inline MaterialInstanceSharedMapConstIterator GetBeginPosConst(void) const
	{
		return m_mapInstances.begin();
	}

	inline MaterialInstanceSharedMapConstIterator GetEndPosConst(void) const
	{
		return m_mapInstances.end();
	}

	//
	// Device Reset
	//

	void OnLostDevice(bool bRecreate);
	void OnResetDevice(bool bRecreate);

	//
	// Deinitialization
	//

	void Empty(void);

private:
	//
	// Private Functions
	//

	MaterialInstanceSharedMapIterator Find(MaterialInstanceShared& rFind);
};

} // namespace ThunderStorm

#pragma warning (disable: 4201)

#endif // THUNDER_MATERIAL_H