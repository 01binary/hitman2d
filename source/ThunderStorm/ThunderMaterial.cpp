/*------------------------------------------------------------------*\
|
| ThunderMaterial.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm engine material class(es) implementation
| Created: 06/10/2009
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderInfoFile.h"	// using InfoElem
#include "ThunderRegion.h"		// using RegionSet
#include "ThunderEngine.h"		// using Engine, Graphics, TileMap, Globals
#include "ThunderMaterial.h"	// defining classes
#include "ThunderClient.h"

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| Constants
\*----------------------------------------------------------*/

const WCHAR Material::SZ_MATERIAL[]					= L"material";
const WCHAR Material::SZ_EFFECT[]					= L"effect";
const WCHAR Material::SZ_TECHNIQUE[]				= L"technique";
const WCHAR Material::SZ_PARAMS[]					= L"parameters";
const WCHAR Material::SZ_REGIONSET[]				= L"regionset";
const WCHAR Material::SZ_META[]						= L"meta";
const char Material::SZ_TECHNIQUE_FIRSTVALID[]		= "firstvalid";

const WCHAR MaterialInstance::SZ_TEXCOORDS[]		= L"texcoords";
const WCHAR MaterialInstance::SZ_FLAGS[]			= L"flags";
const WCHAR MaterialInstance::SZ_CURFRAME[]			= L"currentframe";
const WCHAR MaterialInstance::SZ_CURSEQ[]			= L"currentsequence";
const WCHAR MaterialInstance::SZ_NEXTSEQ[]			= L"nextsequence";
const WCHAR MaterialInstance::SZ_ANIM[]				= L"animation";
const WCHAR MaterialInstance::SZ_ANIM_FRAME[]		= L"frame";
const WCHAR MaterialInstance::SZ_ANIM_PLAY[]		= L"play";
const WCHAR MaterialInstance::SZ_ANIM_LOOP[]		= L"loop";
const WCHAR MaterialInstance::SZ_ANIM_REVERSE[]		= L"reverse";

const DWORD MaterialInstance::DW_FLAGS[] =
												{
													  MaterialInstance::STATIC,
													  MaterialInstance::ANIMATED,
													  MaterialInstance::PLAYING,
													  MaterialInstance::REVERSE,
													  MaterialInstance::LOOPING,
													  MaterialInstance::TRIGGERED,
													  MaterialInstance::WAITSIGNALED,
													  MaterialInstance::SIGNALED
												};

const LPCWSTR MaterialInstance::SZ_FLAGNAMES[] =
												{
													  L"static",
													  L"animated",
													  L"playing",
													  L"reverse",
													  L"looping",
													  L"triggered",
													  L"waitsignaled",
													  L"signaled"
												};

const char EffectParameterInfo::SZ_SEMANTIC_COLOR[]	= "COLOR";

const LPCSTR EffectParameterInfo::SZ_SEMANTIC[] =
												{
													  "BASETEXTURE",
													  "TARGETTEXTURE",
													  "WORLD",
													  "VIEW",
													  "PROJ",
													  "VIEWPROJ",
													  "WORLDVIEWPROJ",
													  "TIME",
													  "RUNTIME",
													  "FRAMETIME",
													  "TARGETSIZE"
												};


/*----------------------------------------------------------*\
| Effect implementation
\*----------------------------------------------------------*/

Effect::Effect(Engine& rEngine): Resource(rEngine),
								 m_pD3DEffect(NULL),
								 m_dwSize(0),
								 m_bHasAutoParams(false),
								 m_pAutoTargetParam(NULL),
								 m_pBaseParam(NULL),
								 m_hTechnique(NULL)
{
	ZeroMemory(&m_desc, sizeof(D3DXEFFECT_DESC));
}

Effect::~Effect(void)
{
	Empty();
}

void Effect::SetTechnique(D3DXHANDLE hTechnique)
{
	if (hTechnique == m_hTechnique)
		return;

	m_hTechnique = hTechnique;

	HRESULT hr = m_pD3DEffect->SetTechnique(hTechnique);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_SETTECHNIQUE,
			__FUNCTIONW__, hr);
}

const EffectParameterInfo* Effect::GetParameterInfoConst(
	LPCWSTR pszParameter) const
{
	EffectParameterInfoMapConstIterator pos = 
		m_mapParams.find(pszParameter);

	if (m_mapParams.end() == pos)
		return NULL;

	return pos->second;
}

EffectParameterInfo* Effect::GetParameterInfo(LPCWSTR pszParameter)
{
	EffectParameterInfoMapConstIterator pos =
		m_mapParams.find(pszParameter);

	if (m_mapParams.end() == pos)
		return NULL;

	return pos->second;
}

void Effect::OnLostDevice(bool bRecreate)
{
	m_hTechnique = D3DXHANDLE(NULL);

	if (true == bRecreate)
	{
		//m_rEngine.PrintDebug(L"release (lostdev) d3dxeffect 0x%x", m_pD3DEffect);

		SAFERELEASE(m_pD3DEffect);
		m_dwSize = 0;

		// Unbind parameter handles until effect is re-created

		for(EffectParameterInfoMapIterator pos = m_mapParams.begin();
			pos != m_mapParams.end();
			pos++)
		{
			pos->second->CacheHandle(D3DXHANDLE(NULL));
		}

		// Unbind technique handles until effect is re-created

		for(EffectTechniqueMapIterator pos = m_mapTechniques.begin();
			pos != m_mapTechniques.end();
			pos++)
		{
			pos->second->CacheHandle(D3DXHANDLE(NULL));
		}
	}
	else
	{
		HRESULT hr = m_pD3DEffect->OnLostDevice();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_ONLOSTDEVICE,
				__FUNCTIONW__, hr);
	}

	// Release state blocks for all techniques

	for(EffectTechniqueMapIterator pos = m_mapTechniques.begin();
		pos != m_mapTechniques.end();
		pos++)
	{
		pos->second->GetRestoreStates().Empty();
	}
}

void Effect::OnResetDevice(bool bRecreate)
{
	HRESULT hr = 0;

	if (true == bRecreate)
	{
		// Re-compile the effect

		Compile(m_strName);

		// Re-bind technique handles

		for(EffectTechniqueMapIterator pos = m_mapTechniques.begin();
			pos != m_mapTechniques.end();
			pos++)
		{
			pos->second->CacheHandle(
				m_pD3DEffect->GetTechnique(pos->second->GetIndex()));
		}

		// Re-bind parameter handles

		for(EffectParameterInfoMapIterator pos = m_mapParams.begin();
			pos != m_mapParams.end();
			pos++)
		{
			pos->second->CacheHandle(
				m_pD3DEffect->GetParameter(NULL, pos->second->GetIndex()));
		}
	}
	else
	{
		hr = m_pD3DEffect->OnResetDevice();

		if (FAILED(hr))
			throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_ONRESETDEVICE,
				__FUNCTIONW__, hr);
	}

	// Recreate state blocks for all techniques

	try
	{
		for(EffectTechniqueMapIterator pos = m_mapTechniques.begin();
			pos != m_mapTechniques.end();
			pos++)
		{
			EffectTechnique* pTech = pos->second;

			hr = m_pD3DEffect->SetTechnique(pTech->GetHandle());

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_SETTECHNIQUE,
					__FUNCTIONW__, hr);

			UINT uPasses = 0;

			pTech->GetRestoreStates().Begin();

			hr = m_pD3DEffect->Begin(&uPasses,
				D3DXFX_DONOTSAVESTATE);			

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_BEGIN,
					__FUNCTIONW__, hr);

			for(UINT j = 0; j < uPasses; j++)
			{
				hr = m_pD3DEffect->BeginPass(j);

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3DX_EFFECT_BEGINPASS, __FUNCTIONW__, hr);

				hr = m_pD3DEffect->EndPass();

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3DX_EFFECT_ENDPASS, __FUNCTIONW__, hr);
			}

			hr = m_pD3DEffect->End();

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_END,
					__FUNCTIONW__, hr);

			pTech->GetRestoreStates().End();

			pTech->GetRestoreStates().Capture();
		}
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(EffectTechnique));
	}
}

void Effect::Deserialize(LPCWSTR pszPath)
{
	Empty();

	// Compile effect from file

	Compile(pszPath);

	// Load information about the effect

	HRESULT hr = m_pD3DEffect->GetDesc(&m_desc);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_GETDESC,
			__FUNCTIONW__, hr);

	// Load techniques

	try
	{
		for(UINT n = 0; n < m_desc.Techniques; n++)
		{
			// Create technique and retrieve info

			EffectTechnique* pTech = new EffectTechnique(*this, n);
			m_mapTechniques[pTech->GetInfo().Name] = pTech;

			// Generate restore states

			hr = m_pD3DEffect->SetTechnique(pTech->GetHandle());

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_SETTECHNIQUE,
					__FUNCTIONW__, hr);

			UINT uPasses = 0;

			pTech->GetRestoreStates().Begin();

			hr = m_pD3DEffect->Begin(&uPasses,
				D3DXFX_DONOTSAVESTATE);			

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_BEGIN,
					__FUNCTIONW__, hr);

			for(UINT j = 0; j < uPasses; j++)
			{
				hr = m_pD3DEffect->BeginPass(j);

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3DX_EFFECT_BEGINPASS, __FUNCTIONW__, hr);

				hr = m_pD3DEffect->EndPass();

				if (FAILED(hr))
					throw m_rEngine.GetErrors().Push(
						Error::D3DX_EFFECT_ENDPASS, __FUNCTIONW__, hr);
			}

			hr = m_pD3DEffect->End();

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_END,
					__FUNCTIONW__, hr);

			pTech->GetRestoreStates().End();

			pTech->GetRestoreStates().Capture();
		}
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(EffectTechnique));
	}

	// Load parameter definitions

	try
	{
		for(UINT n = 0; n < m_desc.Parameters; n++)
		{
			EffectParameterInfo paramInfo(this, n);

			if (paramInfo.GetType() == EffectParameterInfo::TYPE_UNKNOWN)
				continue;

			EffectParameterInfo* pAdded = NULL;

			// If shared, create in parameter info pool, don't allow to have value

			if (paramInfo.IsShared() == true)
				pAdded = m_rEngine.GetGraphics().GetEffectPool().Add(&paramInfo);
			else
				pAdded = new EffectParameterInfo(paramInfo);

			m_mapParams[paramInfo.GetName()] = pAdded;

			// If automatic, cache any pointers

			if (pAdded->IsAutomatic() == true)
			{
				m_bHasAutoParams = true;

				if (paramInfo.GetSemantic() ==
					EffectParameterInfo::SEMANTIC_TARGETTEXTURE)
					m_pAutoTargetParam = pAdded;

				m_arAutoParams.push_back(pAdded);
			}

			if (pAdded->GetSemantic() == EffectParameterInfo::SEMANTIC_BASETEXTURE)
				m_pBaseParam = pAdded;
		}
	}

	catch(std::bad_alloc)
	{
		throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(EffectParameterInfo));
	}
}

void Effect::Compile(LPCWSTR pszPath)
{
	LPD3DXEFFECTCOMPILER pEffectCompiler = NULL;
	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pCompiledEffect = NULL;

	// Create effect compiler

	HRESULT hr = D3DXCreateEffectCompilerFromFile(pszPath,
		NULL, NULL,
		m_rEngine.GetOptionEx(Engine::OPTION_EFFECT_COMPILE_FLAGS), 
		&pEffectCompiler, &pErrors);

	if (FAILED(hr))
	{
		String strErrors;
		
		if (pErrors != NULL)
		{
			LPCSTR psz =
				reinterpret_cast<LPCSTR>(pErrors->GetBufferPointer());

			strErrors = psz;

			if (strErrors[strErrors.GetLength() - 1] == 10)
				strErrors[strErrors.GetLength() - 1] = L'\0';

			pErrors->Release();
		}

		throw m_rEngine.GetErrors().Push(Error::D3DX_CREATEEFFECTCOMPILERFROMFILE,
			__FUNCTIONW__, pszPath, strErrors, hr);
	}

	// Compile effect

	hr = pEffectCompiler->CompileEffect(
		m_rEngine.GetOptionEx(Engine::OPTION_EFFECT_COMPILE_FLAGS),
		&pCompiledEffect, &pErrors);

	if (FAILED(hr))
	{
		// Dump errors if failed

		String strErrors;
		
		if (pErrors != NULL)
		{
			LPCSTR psz =
				reinterpret_cast<LPCSTR>(pErrors->GetBufferPointer());

			strErrors = psz;

			for(int n = 0; n < strErrors.GetLength(); n++)
			{
				if (10 == strErrors[n])
					strErrors[n] = L'\n';
			}

			pErrors->Release();
		}

		throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECTCOMPILER_COMPILEEFFECT,
			__FUNCTIONW__, strErrors, hr);
	}

	// Remember size of compiled effect for memory statistics

	m_dwSize = pCompiledEffect->GetBufferSize();

	// Load compiled effect

	hr = D3DXCreateEffect(m_rEngine.GetGraphics().GetDevice(),
		pCompiledEffect->GetBufferPointer(),
		pCompiledEffect->GetBufferSize(), NULL,  NULL, D3DXFX_NOT_CLONEABLE,
		m_rEngine.GetGraphics().GetEffectPool().GetD3DXEffectPool(),
		&m_pD3DEffect, NULL);

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_CREATEEFFECT,
			__FUNCTIONW__, hr);

	// Set state manager

	hr = m_pD3DEffect->SetStateManager(m_rEngine.GetGraphics().GetStates());

	if (FAILED(hr))
		throw m_rEngine.GetErrors().Push(Error::D3DX_EFFECT_SETSTATEMANAGER,
			__FUNCTIONW__, hr);
}

DWORD Effect::GetMemoryFootprint(void) const
{
	DWORD dwSizeParamInfo = 0;
	DWORD dwSizeTechName = 0;

	for(EffectTechniqueMapConstIterator pos = m_mapTechniques.begin();
		pos != m_mapTechniques.end();
		pos++)
	{
		dwSizeTechName += (pos->first.GetLengthBytes() +
			sizeof(EffectTechnique));
	}

	for(EffectParameterInfoMapConstIterator pos = m_mapParams.begin();
		pos != m_mapParams.end();
		pos++)
	{
		dwSizeParamInfo += (pos->first.GetLengthBytes() +
			sizeof(EffectParameterInfo));
	}

	return sizeof(Effect) + m_dwSize +
		dwSizeParamInfo + dwSizeTechName;
}

void Effect::Empty(void)
{
	// Release D3D effect

	SAFERELEASE(m_pD3DEffect);

	// Release parameter infos and their mapping

	for(EffectParameterInfoMapIterator pos = m_mapParams.begin();
		pos != m_mapParams.end();
		pos++)
	{
		pos->second->Release(this);
	}

	m_mapParams.clear();

	m_arAutoParams.clear();

	// Release techniques and clear technique mapping

	for(EffectTechniqueMapIterator pos = m_mapTechniques.begin();
		pos != m_mapTechniques.end();
		pos++)
	{
		delete pos->second;
	}

	m_mapTechniques.clear();

	m_hTechnique = 0;

	// Clear data

	m_dwSize = 0;

	ZeroMemory(&m_desc, sizeof(D3DXEFFECT_DESC));
}

void Effect::Remove(void)
{
	m_rEngine.GetEffects().Remove(this);
}

/*----------------------------------------------------------*\
| EffectTechnique implementation
\*----------------------------------------------------------*/

EffectTechnique::EffectTechnique(Effect& rEffect, UINT uTechniqueIndex):
	m_uTechniqueIndex(uTechniqueIndex),
	m_RestoreStates(rEffect.GetEngine().GetGraphics())
{
	m_hTechnique = rEffect.GetD3DXEffect()->GetTechnique(uTechniqueIndex);

	HRESULT hr = rEffect.GetD3DXEffect()->GetTechniqueDesc(
		m_hTechnique, &m_desc);

	if (FAILED(hr))
		throw rEffect.GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_GETTECHNIQUEDESC, 
			__FUNCTIONW__, hr);	
}

/*----------------------------------------------------------*\
| EffectParameterInfo implementation
\*----------------------------------------------------------*/

EffectParameterInfo::EffectParameterInfo(Effect* pEffect, UINT uParamIndex):
										 m_hParam(NULL),
										 m_uParamIndex(uParamIndex),
										 m_nType(TYPE_UNKNOWN),
										 m_nSemantic(SEMANTIC_NONE),
										 m_bShared(false),
										 m_dwLastFrameChanged(INVALID_VALUE)
{
	_ASSERT(pEffect != NULL);

	D3DXPARAMETER_DESC paramdesc = {0};

	// Retrieve handle

	m_hParam = pEffect->m_pD3DEffect->GetParameter(NULL, uParamIndex);

	// Retrieve description

	HRESULT hr =
		pEffect->m_pD3DEffect->GetParameterDesc(m_hParam, &paramdesc);

	if (FAILED(hr))
		throw pEffect->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_GETPARAMETERDESC, __FUNCTIONW__, hr);

	// Retrieve name

	m_strName = paramdesc.Name;	

	// Retrieve type

	switch(paramdesc.Type)
	{
	case D3DXPT_TEXTURE:
	case D3DXPT_TEXTURE1D:
	case D3DXPT_TEXTURE2D:
		m_nType = TYPE_TEXTURE;
		break;
	case D3DXPT_TEXTURECUBE:
		m_nType = TYPE_TEXTURECUBE;
		break;
	case D3DXPT_FLOAT:
		{
			if (D3DXPC_SCALAR == paramdesc.Class)
			{
				m_nType = TYPE_SCALAR;
			}
			else if (D3DXPC_VECTOR == paramdesc.Class)
			{
				if (paramdesc.Semantic != NULL &&
					strncmp(paramdesc.Semantic, SZ_SEMANTIC_COLOR,
					strlen(SZ_SEMANTIC_COLOR)) == 0)
				{
					m_nType = TYPE_COLOR;
				}
				else
				{
					if (2 == paramdesc.Columns)
						m_nType = TYPE_VECTOR2;
					else if (4 == paramdesc.Columns)
						m_nType = TYPE_VECTOR4;
				}
			}
			else if (D3DXPC_MATRIX_ROWS == paramdesc.Class)
			{
				m_nType = TYPE_MATRIX4X4;
			}
		}
		break;
	}

	// Retrieve shared

	if (paramdesc.Flags & D3DX_PARAMETER_SHARED)
		m_bShared = true;

	// Retrieve semantic

	if (paramdesc.Semantic != NULL)
	{
		for(int nSemantic = 1; nSemantic < SEMANTIC_COUNT; nSemantic++)
		{
			if (strcmp(paramdesc.Semantic, SZ_SEMANTIC[nSemantic - 1]) == 0)
			{
				m_nSemantic = static_cast<Semantics>(nSemantic);
				break;
			}
		}
	}

	// Add reference

	m_arRefs.push_back(pEffect);
}

EffectParameterInfo::EffectParameterInfo(const EffectParameterInfo& rInit)
{
	m_hParam = rInit.m_hParam;
	m_nType = rInit.m_nType;
	m_uParamIndex = rInit.m_uParamIndex;
	m_nSemantic = rInit.m_nSemantic;
	m_bShared = rInit.m_bShared;
	m_strName = rInit.m_strName;
	m_dwLastFrameChanged = rInit.m_dwLastFrameChanged;

	std::copy(rInit.m_arRefs.begin(),
		rInit.m_arRefs.end(),
		std::inserter(m_arRefs, m_arRefs.end()));
}

EffectParameterInfo& EffectParameterInfo::operator=(
	const EffectParameterInfo& rAssign)
{
	m_hParam = rAssign.m_hParam;
	m_nType = rAssign.m_nType;
	m_uParamIndex = rAssign.m_uParamIndex;
	m_bShared = rAssign.m_bShared;
	m_strName = rAssign.m_strName;
	m_dwLastFrameChanged = rAssign.m_dwLastFrameChanged;

	std::copy(rAssign.m_arRefs.begin(),
		rAssign.m_arRefs.end(),
		std::inserter(m_arRefs, m_arRefs.end()));

	return *this;
}

void EffectParameterInfo::Release(Effect* pOwner)
{
	if (true == m_bShared)
	{
		// Find that owner

		EffectArrayIterator pos = std::find(m_arRefs.begin(),
			m_arRefs.end(), pOwner);

		if (m_arRefs.end() == pos)
			return;

		// Remove it from list of references

		m_arRefs.erase(pos);

		// If no references left, remove from pool

		if (m_arRefs.empty() == true)
			pOwner->GetEngine().GetGraphics().GetEffectPool().Remove(this);
		else
			return;
	}

	delete this;
}

/*----------------------------------------------------------*\
| EffectParameter implementation
\*----------------------------------------------------------*/

EffectParameter::EffectParameter(EffectParameterInfo* pInfo): m_pInfo(pInfo)
{
	ZeroMemory(&m_mtx, sizeof(D3DMATRIX));
}

EffectParameter::EffectParameter(const EffectParameter& rInit)
{
	m_pInfo = rInit.m_pInfo;
	CopyMemory(&m_mtx, &rInit.m_mtx, sizeof(m_mtx));

	if (m_pInfo != NULL &&
		(m_pInfo->GetType() == EffectParameterInfo::TYPE_TEXTURE ||
		m_pInfo->GetType() == EffectParameterInfo::TYPE_TEXTURECUBE))
	{
		m_pTexture->AddRef();
	}
}

EffectParameter::~EffectParameter(void)
{
	Empty();
}

LPVOID EffectParameter::GetValue(void)
{
	if (NULL == m_pInfo)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	switch(m_pInfo->GetType())
	{
	case EffectParameterInfo::TYPE_TEXTURE:
		return m_pTexture;
		break;
	case EffectParameterInfo::TYPE_TEXTURECUBE:
		return m_pTextureCube;
		break;
	case EffectParameterInfo::TYPE_COLOR:
		return &m_fVec4;
		break;
	case EffectParameterInfo::TYPE_SCALAR:
		return &m_fScalar;
		break;
	case EffectParameterInfo::TYPE_VECTOR2:
		return m_fVec2;
		break;
	case EffectParameterInfo::TYPE_VECTOR4:
		return m_fVec4;
		break;
	case EffectParameterInfo::TYPE_MATRIX4X4:
		return &m_mtx;
		break;
	}

	return NULL;
}

Texture* EffectParameter::GetTexture(void)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	return m_pTexture;
}

const Texture* EffectParameter::GetTextureConst(void) const
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	return m_pTexture;
}

TextureCube* EffectParameter::GetTextureCube(void)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURECUBE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	return m_pTextureCube;
}

const TextureCube* EffectParameter::GetTextureCubeConst(void) const
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURECUBE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	return m_pTextureCube;
}

void EffectParameter::SetValue(LPVOID pvValue)
{
	if (NULL == m_pInfo)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	switch(m_pInfo->GetType())
	{
	case EffectParameterInfo::TYPE_TEXTURE:
		m_pTexture = reinterpret_cast<Texture*>(pvValue);
		break;
	case EffectParameterInfo::TYPE_TEXTURECUBE:
		m_pTextureCube = reinterpret_cast<TextureCube*>(pvValue);
		break;
	case EffectParameterInfo::TYPE_COLOR:
		CopyMemory(m_fVec4, pvValue, sizeof(float) * 4);
		break;
	case EffectParameterInfo::TYPE_SCALAR:
		m_fScalar = *reinterpret_cast<float*>(pvValue);
		break;
	case EffectParameterInfo::TYPE_VECTOR2:
		CopyMemory(m_fVec2, pvValue, sizeof(float) * 2);
		break;
	case EffectParameterInfo::TYPE_VECTOR4:
		CopyMemory(m_fVec4, pvValue, sizeof(float) * 4);
		break;
	case EffectParameterInfo::TYPE_MATRIX4X4:
		CopyMemory(&m_mtx, pvValue, sizeof(D3DMATRIX));
		break;
	default:
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);
		break;
	}
}

void EffectParameter::SetTexture(Texture* pTexture)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	if (NULL == pTexture)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__);

	pTexture->AddRef();

	if (m_pTexture != NULL)
		m_pTexture->Release();

	m_pTexture = pTexture;
}

void EffectParameter::SetTextureCube(TextureCube* pTextureCube)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_TEXTURECUBE)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	if (NULL == pTextureCube)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__);

	pTextureCube->AddRef();

	if (m_pTextureCube != NULL)
		m_pTextureCube->Release();

	m_pTextureCube = pTextureCube;
}

void EffectParameter::SetColor(D3DCOLORVALUE crvColor)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_COLOR)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(&m_crvColor, &crvColor, sizeof(D3DCOLORVALUE));
}

void EffectParameter::SetColor(D3DCOLOR clrColor)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_COLOR)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	D3DXCOLOR clrConv(clrColor);

	CopyMemory(&m_crvColor, &clrConv, sizeof(D3DXCOLOR));
}

void EffectParameter::SetScalar(float fScalar)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_SCALAR)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	m_fScalar = fScalar;
}

void EffectParameter::SetVector2(const D3DXVECTOR2& rVector2)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_VECTOR2)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(m_fVec2, &rVector2, sizeof(D3DXVECTOR2));
}

void EffectParameter::SetVector4(const D3DXVECTOR4& rVector4)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_VECTOR4)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(m_fVec4, &rVector4, sizeof(D3DXVECTOR4));
}

void EffectParameter::SetMatrix(const D3DXMATRIX& rMatrix)
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() != EffectParameterInfo::TYPE_MATRIX4X4)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(&m_mtx, &rMatrix, sizeof(D3DXMATRIX));
}

void EffectParameter::Apply(void)
{
	if (NULL == m_pInfo || m_pInfo->m_arRefs.empty() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	LPD3DXEFFECT pEffect = m_pInfo->m_arRefs.front()->GetD3DXEffect();

	if (NULL == pEffect)
		throw Error(Error::INVALID_PTR,
			__FUNCTIONW__, L"LPD3DXEFFECT pEffect");

	switch(m_pInfo->GetType())
	{
	case EffectParameterInfo::TYPE_TEXTURE:
		pEffect->SetTexture(m_pInfo->GetHandle(),
			m_pTexture->GetD3DTexture());
		break;
	case EffectParameterInfo::TYPE_TEXTURECUBE:
		pEffect->SetTexture(m_pInfo->GetHandle(),
			m_pTextureCube->GetD3DTexture());
		break;
	case EffectParameterInfo::TYPE_COLOR:
	case EffectParameterInfo::TYPE_VECTOR4:
		pEffect->SetVector(m_pInfo->GetHandle(),
			reinterpret_cast<D3DXVECTOR4*>(m_fVec4));
		break;
	case EffectParameterInfo::TYPE_VECTOR2:
		pEffect->SetFloatArray(m_pInfo->GetHandle(), m_fVec2, 2);
		break;
	case EffectParameterInfo::TYPE_SCALAR:
		pEffect->SetFloat(m_pInfo->GetHandle(), m_fScalar);
		break;
	case EffectParameterInfo::TYPE_MATRIX4X4:
		pEffect->SetMatrix(m_pInfo->GetHandle(),
			static_cast<D3DXMATRIX*>(&m_mtx));
		break;
	}
}

void EffectParameter::Serialize(const Engine& rEngine, InfoElem& rRoot) const
{
	if (NULL == m_pInfo ||
		m_pInfo->GetType() == EffectParameterInfo::TYPE_UNKNOWN)
			throw rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	try
	{
		switch(m_pInfo->GetType())
		{
		case EffectParameterInfo::TYPE_TEXTURE:
			{
				WCHAR szRelPath[MAX_PATH] = {0};
				GetRelativePath(m_pTexture->GetName(), szRelPath);

				rRoot.SetStringValue(szRelPath);
			}
			break;
		case EffectParameterInfo::TYPE_TEXTURECUBE:
			{
				WCHAR szRelPath[MAX_PATH] = {0};
				GetRelativePath(m_pTextureCube->GetName(), szRelPath);

				rRoot.SetStringValue(szRelPath);
			}
			break;
		case EffectParameterInfo::TYPE_COLOR:
		case EffectParameterInfo::TYPE_VECTOR4:
			rRoot.FromFloatArray(m_fVec4, 4);
			break;
		case EffectParameterInfo::TYPE_SCALAR:
			rRoot.SetFloatValue(m_fScalar);
			break;
		case EffectParameterInfo::TYPE_VECTOR2:
			rRoot.FromFloatArray(m_fVec2, 2);
			break;
		case EffectParameterInfo::TYPE_MATRIX4X4:
			rRoot.FromFloatArray(reinterpret_cast<const float*>(&m_mtx), 4 * 4);
			break;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void EffectParameter::Deserialize(Engine& rEngine, const InfoElem& rRoot)
{
	if (m_pInfo->GetType() == EffectParameterInfo::TYPE_UNKNOWN)
		throw rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	try
	{
		switch(m_pInfo->GetType())
		{
		case EffectParameterInfo::TYPE_TEXTURE:
			{
				if (rRoot.GetVarType()!= Variable::TYPE_STRING)
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

				SetTexture(rEngine.GetTextures().Load(rRoot.GetStringValue()));
			}
			break;
		case EffectParameterInfo::TYPE_TEXTURECUBE:
			{
				if (rRoot.GetVarType() != Variable::TYPE_STRING)
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

				SetTextureCube(rEngine.GetCubeTextures().Load(rRoot.GetStringValue()));
			}
			break;
		case EffectParameterInfo::TYPE_COLOR:
		case EffectParameterInfo::TYPE_VECTOR4:
			{
				if (rRoot.GetChildCount() != 4)
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

				rRoot.ToFloatArray(m_fVec4, 4);
			}
			break;
		case EffectParameterInfo::TYPE_SCALAR:
			{
				if (rRoot.GetVarType() == Variable::TYPE_INT)
					m_fScalar = static_cast<float>(rRoot.GetIntValue());
				else if (rRoot.GetVarType() == Variable::TYPE_FLOAT)
					m_fScalar = rRoot.GetFloatValue();
				else
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());
			}
			break;
		case EffectParameterInfo::TYPE_VECTOR2:
			{
				if (rRoot.GetChildCount() != 2)
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

				rRoot.ToFloatArray(m_fVec2, 2);
			}
			break;
		case EffectParameterInfo::TYPE_MATRIX4X4:
			{
				if (rRoot.GetChildCount() != 4 * 4)
					throw rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
						__FUNCTIONW__, rRoot.GetName(), rRoot.GetDocumentConst().GetPath());

				rRoot.ToFloatArray(reinterpret_cast<float*>(&m_mtx), 4 * 4);
			}
			break;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void EffectParameter::Serialize(const Engine& rEngine, Stream& rStream) const
{
	if (m_pInfo->GetType() == EffectParameterInfo::TYPE_UNKNOWN)
		throw rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	try
	{
		switch(m_pInfo->GetType())
		{
		case EffectParameterInfo::TYPE_TEXTURE:
			{
				String strRelPath;
				strRelPath.Allocate(MAX_PATH);

				GetRelativePath(m_pTexture->GetName(),
					strRelPath.GetBuffer());

				strRelPath.Serialize(rStream);
			}
			break;
		case EffectParameterInfo::TYPE_TEXTURECUBE:
			{
				String strRelPath;
				strRelPath.Allocate(MAX_PATH);

				GetRelativePath(m_pTextureCube->GetName(),
					strRelPath.GetBuffer());

				strRelPath.Serialize(rStream);
			}
			break;
		case EffectParameterInfo::TYPE_COLOR:
		case EffectParameterInfo::TYPE_VECTOR4:
			rStream.WriteVar(m_fVec4, 4);
			break;
		case EffectParameterInfo::TYPE_SCALAR:
			rStream.WriteVar(&m_fScalar);
			break;
		case EffectParameterInfo::TYPE_VECTOR2:
			rStream.WriteVar(m_fVec2, 2);
			break;
		case EffectParameterInfo::TYPE_MATRIX4X4:
			rStream.WriteVar(&m_mtx._11, 16);
			break;
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void EffectParameter::Deserialize(Engine& rEngine, Stream& rStream)
{
	if (m_pInfo->GetType() == EffectParameterInfo::TYPE_UNKNOWN)
		throw rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	try
	{
		switch(m_pInfo->GetType())
		{
		case EffectParameterInfo::TYPE_TEXTURE:
			{
				String strPath;
				strPath.Deserialize(rStream);

				SetTexture(rEngine.GetTextures().Load(strPath));
			}
			break;
		case EffectParameterInfo::TYPE_TEXTURECUBE:
			{
				String strPath;
				strPath.Deserialize(rStream);

				SetTextureCube(rEngine.GetCubeTextures().Load(strPath));
			}
			break;
		case EffectParameterInfo::TYPE_COLOR:
		case EffectParameterInfo::TYPE_VECTOR4:
			rStream.ReadVar(m_fVec4, 4);
			break;
		case EffectParameterInfo::TYPE_SCALAR:
			rStream.ReadVar(&m_fScalar);
			break;
		case EffectParameterInfo::TYPE_VECTOR2:
			rStream.ReadVar(m_fVec2, 2);
			break;		
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void EffectParameter::Empty(void)
{
	if (m_pInfo != NULL &&
		(m_pInfo->GetType() == EffectParameterInfo::TYPE_TEXTURE ||
		m_pInfo->GetType() == EffectParameterInfo::TYPE_TEXTURECUBE))
	{
		/*if (m_pTexture->GetName().GetBufferConst()[1] != ':')
		{
			m_pTexture->GetInfo();

			Engine& rEngine = m_pTexture->GetEngine();

			int refs = m_pTexture->Release();

			rEngine.PrintDebug(L"effect param release, tex = %x (%s), refs = %d, param = %x", m_pTexture, m_pTexture->GetName(), refs, this);

			m_pTexture = NULL;
		}*/

		SAFERELEASE(m_pTexture);
	}
}

bool EffectParameter::operator==(const EffectParameter& rCompare) const
{
	if (m_pInfo->GetType() != rCompare.m_pInfo->GetType())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	switch(m_pInfo->GetType())
	{
	case EffectParameterInfo::TYPE_TEXTURE:
		return (m_pTexture == rCompare.m_pTexture);
		break;
	case EffectParameterInfo::TYPE_TEXTURECUBE:
		return (m_pTextureCube == rCompare.m_pTextureCube);
		break;
	case EffectParameterInfo::TYPE_COLOR:
		return (memcmp(m_fVec4, rCompare.m_fVec4, sizeof(float) * 4) == 0);
		break;
	case EffectParameterInfo::TYPE_SCALAR:
		return (m_fScalar == rCompare.m_fScalar);
		break;
	case EffectParameterInfo::TYPE_VECTOR2:
		return (memcmp(m_fVec2, rCompare.m_fVec2, sizeof(float) * 2) == 0);
		break;
	case EffectParameterInfo::TYPE_VECTOR4:
		return (memcmp(m_fVec4, rCompare.m_fVec4, sizeof(float) * 4) == 0);
		break;
	case EffectParameterInfo::TYPE_MATRIX4X4:
		return (memcmp(&m_mtx, &rCompare.m_mtx, sizeof(D3DMATRIX)) == 0);
		break;
	default:
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);
		break;
	}
}

EffectParameter& EffectParameter::operator=(const EffectParameter& rAssign)
{
	Empty();

	// Assign type information

	m_pInfo = rAssign.m_pInfo;

	// Assign value

	switch(m_pInfo->GetType())
	{
	case EffectParameterInfo::TYPE_TEXTURE:
		m_pTexture = rAssign.m_pTexture;
		m_pTexture->AddRef();
		break;
	case EffectParameterInfo::TYPE_TEXTURECUBE:
		m_pTextureCube = rAssign.m_pTextureCube;
		m_pTextureCube->AddRef();
		break;
	case EffectParameterInfo::TYPE_COLOR:
		CopyMemory(m_fVec4, rAssign.m_fVec4, sizeof(float) * 4);
		break;
	case EffectParameterInfo::TYPE_SCALAR:
		m_fScalar = rAssign.m_fScalar;
		break;
	case EffectParameterInfo::TYPE_VECTOR2:
		CopyMemory(m_fVec2, rAssign.m_fVec2, sizeof(float) * 2);
		break;
	case EffectParameterInfo::TYPE_VECTOR4:
		CopyMemory(m_fVec4, rAssign.m_fVec4, sizeof(float) * 4);
		break;
	case EffectParameterInfo::TYPE_MATRIX4X4:
		CopyMemory(&m_mtx, &rAssign.m_mtx, sizeof(D3DMATRIX));
		break;
	default:
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);
		break;
	}

	return *this;
}

/*----------------------------------------------------------*\
| Material implementation
\*----------------------------------------------------------*/

Material::Material(Engine& rEngine): Resource(rEngine),
									 m_pEffect(NULL),
									 m_pTechnique(NULL),
									 m_pRegionSet(NULL)
{
}

Material::~Material(void)
{
	Empty();
}

EffectParameter* Material::GetParameter(LPCWSTR pszName)
{
	for(EffectParameterArray::iterator pos =
		m_arStaticParams.begin();
		pos != m_arStaticParams.end();
		pos++)
	{
		if (pos->GetInfo()->GetName() == pszName)
			return &(*pos);
	}

	return NULL;
}

EffectParameter* Material::GetParameter(const EffectParameterInfo* pInfo)
{
	for(EffectParameterArray::iterator pos =
		m_arStaticParams.begin();
		pos != m_arStaticParams.end();
		pos++)
	{
		if (pos->GetInfo() == pInfo)
			return &(*pos);
	}

	return NULL;
}

const EffectParameter* Material::GetParameterConst(LPCWSTR pszName) const
{
	for(EffectParameterArray::const_iterator pos =
		m_arStaticParams.begin();
		pos != m_arStaticParams.end();
		pos++)
	{
		if (pos->GetInfo()->GetName() == pszName)
			return &(*pos);
	}

	return NULL;
}

const EffectParameter* Material::GetParameterConst(const EffectParameterInfo* pInfo) const
{
	for(EffectParameterArray::const_iterator pos =
		m_arStaticParams.begin();
		pos != m_arStaticParams.end();
		pos++)
	{
		if (pos->GetInfo() == pInfo)
			return &(*pos);
	}

	return NULL;
}

EffectParameter* Material::GetBaseParameter(void)
{
	if (NULL == m_pEffect)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameterInfo* pBaseParamInfo = 
		m_pEffect->GetBaseParamInfo();

	if (NULL == pBaseParamInfo)
		return NULL;

	return GetParameter(pBaseParamInfo);
}

const EffectParameter* Material::GetBaseParameterConst(void) const
{
	if (NULL == m_pEffect)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameterInfo* pBaseParamInfo = 
		m_pEffect->GetBaseParamInfo();

	if (NULL == pBaseParamInfo)
		return NULL;

	return GetParameterConst(pBaseParamInfo);
}

EffectParameter* Material::GetTargetParameter(void)
{
	if (NULL == m_pEffect)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameterInfo* pTargetParamInfo = 
		m_pEffect->GetTargetParamInfo();

	if (NULL == pTargetParamInfo)
		return NULL;

	return GetParameter(pTargetParamInfo);
}

const EffectParameter* Material::GetTargetParameterConst(void) const
{
	if (NULL == m_pEffect)
		throw m_rEngine.GetErrors().Push(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameterInfo* pTargetParamInfo = 
		m_pEffect->GetTargetParamInfo();

	if (NULL == pTargetParamInfo)
		return NULL;

	return GetParameterConst(pTargetParamInfo);
}

void Material::SetParameter(EffectParameter& rCopyFrom)
{
	for(EffectParameterArray::iterator pos =
		m_arStaticParams.begin();
		pos != m_arStaticParams.end();
		pos++)
	{
		if (pos->GetInfo() == rCopyFrom.GetInfo())
		{
			(*pos) = rCopyFrom;
			return;
		}
	}

	m_arStaticParams.push_back(rCopyFrom);
}

const Variable* Material::GetVariable(LPCWSTR pszName) const
{
	VariableMapConstIterator posFind = m_mapVariables.find(pszName);

	if (m_mapVariables.end() == posFind)
		return NULL;

	return posFind->second;
}

void Material::Deserialize(LPCWSTR pszPath)
{
	try
	{
		Stream stream(&m_rEngine.GetErrors());

		stream.Open(pszPath, GENERIC_READ, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN);

		Deserialize(stream);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, pszPath);
	}
}

void Material::Deserialize(Stream& rStream)
{
	try
	{
		InfoFile doc(&m_rEngine.GetErrors());

		doc.Deserialize(rStream);

		if (doc.GetRoot() == NULL ||
			doc.GetRoot()->GetName() != SZ_MATERIAL)
				throw m_rEngine.GetErrors().Push(Error::FILE_FORMAT,
					__FUNCTIONW__, rStream.GetPath());

		Deserialize(*doc.GetRoot());
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Material::Deserialize(const InfoElem& rRoot)
{
	PUSH_CURRENT_DIRECTORY(rRoot.GetDocumentConst().GetPath());

	try
	{
		// Read effect name

		const InfoElem* pElem = rRoot.FindChildConst(SZ_EFFECT,
			InfoElem::TYPE_VALUE);

		if (NULL == pElem)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENT,
				__FUNCTIONW__, SZ_EFFECT,
				rRoot.GetDocumentConst().GetPath());

		if (pElem->GetVarType() != Variable::TYPE_STRING)
			throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
				__FUNCTIONW__, SZ_EFFECT,
				rRoot.GetDocumentConst().GetPath());

		// Load effect

		m_pEffect = m_rEngine.GetEffects().Load(pElem->GetStringValue());
		m_pEffect->AddRef();

		// Read technique

		pElem = rRoot.FindChildConst(SZ_TECHNIQUE,
			InfoElem::TYPE_VALUE);

		if (NULL == pElem ||
			(pElem->GetVarType() == Variable::TYPE_ENUM &&
			 pElem->GetString() == SZ_TECHNIQUE_FIRSTVALID))
		{
			// Choose first valid technique

			for(EffectTechniqueMapIterator pos =
				m_pEffect->GetBeginTechniquePos();
				pos != m_pEffect->GetEndTechniquePos();
				pos++)
			{
				m_pTechnique = pos->second;

				HRESULT hr = m_pEffect->GetD3DXEffect()->ValidateTechnique(
					m_pTechnique->GetHandle());

				if (SUCCEEDED(hr))
					break;
			}
		}
		else
		{
			// Choose technique by name

			if (pElem->GetVarType() != Variable::TYPE_STRING)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
					__FUNCTIONW__, SZ_TECHNIQUE,
					rRoot.GetDocumentConst().GetPath());

			m_pTechnique = m_pEffect->GetTechnique(pElem->GetStringValue());

			if (NULL == m_pTechnique)
				throw m_rEngine.GetErrors().Push(Error::INVALID_PTR,
					__FUNCTIONW__, L"m_pTechnique");

			// Validate chosen technique

			HRESULT hr = m_pEffect->GetD3DXEffect()->ValidateTechnique(
				m_pTechnique->GetHandle());

			if (FAILED(hr))
				throw m_rEngine.GetErrors().Push(
					Error::D3DX_EFFECT_VALIDATETECHNIQUE,
					__FUNCTIONW__, hr);
		}

		// Read region set

		pElem = rRoot.FindChildConst(SZ_REGIONSET, InfoElem::TYPE_VALUE);

		if (pElem != NULL)
		{
			if (pElem->GetVarType() != Variable::TYPE_STRING)
				throw m_rEngine.GetErrors().Push(Error::FILE_ELEMENTFORMAT,
					__FUNCTIONW__, pElem->GetName(), rRoot.GetDocumentConst().GetPath());

			m_pRegionSet = m_rEngine.GetRegions().Load(pElem->GetStringValue());
			m_pRegionSet->AddRef();
		}

		// Read static parameters (optional)

		pElem = rRoot.FindChildConst(SZ_PARAMS, InfoElem::TYPE_BLOCK);

		if (pElem != NULL)
		{
			for(InfoElemConstIterator posElem = pElem->GetBeginChildPosConst();
				posElem != pElem->GetEndChildPosConst();
				posElem++)
			{
				const InfoElem* pChildElem = *posElem;

				// Map this parameter to one of effect's parameter definitions by name

				EffectParameterInfo* pParamInfo =
					m_pEffect->GetParameterInfo(pChildElem->GetName());

				if (NULL == pParamInfo)
					throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
						__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());

				// Parameter values cannot be defined if they are shared,
				// or are placeholders for automatic data

				if (pParamInfo->IsAutomatic() == true || pParamInfo->IsShared() == true)
					throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
						__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());

				// Create this parameter with given info and deserialize it

				EffectParameter param(pParamInfo);
				param.Deserialize(m_rEngine, *pChildElem);

				// Add to list of parameters

				m_arStaticParams.push_back(param);
			}
		}

		// Read variables

		pElem = rRoot.FindChildConst(SZ_META, InfoElem::TYPE_BLOCK);

		if (pElem != NULL)
		{
			try
			{
				for(InfoElemConstIterator posElem = pElem->GetBeginChildPosConst();
					posElem != pElem->GetEndChildPosConst();
					posElem++)
				{
					m_mapVariables[(*posElem)->GetName()] =
						new Variable(*static_cast<Variable*>(*posElem));
				}
			}

			catch(std::bad_alloc)
			{
				throw m_rEngine.GetErrors().Push(Error::MEM_ALLOC,
					__FUNCTIONW__, sizeof(Variable));
			}
		}
		
		POP_CURRENT_DIRECTORY();
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		POP_CURRENT_DIRECTORY();

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void Material::Serialize(LPCWSTR pszPath) const
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

void Material::Serialize(Stream& rStream) const
{
	InfoFile doc(&m_rEngine.GetErrors());

	try
	{
		Serialize(*doc.CreateSetRoot(SZ_MATERIAL));

		doc.Serialize(rStream);
	}
	
	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw m_rEngine.GetErrors().Push(Error::FILE_WRITE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void Material::Serialize(InfoElem& rRoot) const
{
	WCHAR szCurDir[MAX_PATH] = {0};
	GetCurrentDirectory(MAX_PATH - 1, szCurDir);

	try
	{
		// Set current directory to material directory

		WCHAR szMaterialDir[MAX_PATH] = {0};

		if (rRoot.GetDocumentConst().GetPath().IsEmpty() == false)
		{
			wcscpy_s(szMaterialDir, MAX_PATH - 1, rRoot.GetDocumentConst().GetPath());
			PathRemoveFileSpec(szMaterialDir);
			SetCurrentDirectory(szMaterialDir);
		}

		// Write effect name

		if (m_pEffect != NULL)
			rRoot.AddChild(SZ_EFFECT)->SetStringValue(m_pEffect->GetName());		

		// Write technique

		rRoot.AddChild(SZ_TECHNIQUE)->SetStringValue(
			String(m_pTechnique->GetInfo().Name));			

		// Write region set

		if (m_pRegionSet != NULL)
			rRoot.AddChild(SZ_REGIONSET)->SetStringValue(m_pRegionSet->GetName());

		// Write static parameters

		if (m_arStaticParams.empty() == false)
		{
			InfoElem& rParams = *rRoot.AddChild(SZ_PARAMS, InfoElem::TYPE_BLOCK);

			for(EffectParameterArrayConstIterator pos = m_arStaticParams.begin();
				pos != m_arStaticParams.end();
				pos++)
			{
				pos->Serialize(m_rEngine,
					*rParams.AddChild(pos->GetInfo()->GetName()));
			}
		}

		// Write variables

		if (m_mapVariables.empty() == false)
		{
			InfoElem& rVars = *rRoot.AddChild(SZ_META, InfoElem::TYPE_BLOCK);

			for(VariableMapConstIterator pos = m_mapVariables.begin();
				pos != m_mapVariables.end();
				pos++)
			{
				pos->second->Serialize(*rVars.AddChild(pos->first));
			}
		}

		// Restore current directory
		
		SetCurrentDirectory(szCurDir);
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		// Restore current directory if needed

		if (String::IsEmpty(szCurDir) == false)
			SetCurrentDirectory(szCurDir);

		throw m_rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

DWORD Material::GetMemoryFootprint(void) const
{
	return DWORD(sizeof(Material) +
		sizeof(EffectParameter) *
		m_arStaticParams.size());
}

void Material::Empty(void)
{
	if (m_pEffect != NULL)
	{
		m_pEffect->Release();
		m_pEffect = NULL;
	}

	if (m_pRegionSet != NULL)
	{
		m_pRegionSet->Release();
		m_pRegionSet = NULL;
	}

	m_arStaticParams.clear();

	m_mapVariables.clear();
}

void Material::Remove(void)
{
	m_rEngine.GetMaterials().Remove(this);
}

Material& Material::operator=(const Material& rCopy)
{
	// Copy effect instance

	if (rCopy.m_pEffect != NULL)
		rCopy.m_pEffect->AddRef();

	if (m_pEffect != NULL)
		m_pEffect->Release();

	m_pEffect = rCopy.m_pEffect;

	m_pTechnique = rCopy.m_pTechnique;

	// Copy region set instance

	if (rCopy.m_pRegionSet != NULL)
		rCopy.m_pRegionSet->AddRef();

	if (m_pRegionSet != NULL)
		m_pRegionSet->Release();

	m_pRegionSet = rCopy.m_pRegionSet;

	// Copy static parameters

	m_arStaticParams.clear();

	if (rCopy.m_arStaticParams.empty() == false)
	{
		m_arStaticParams.assign(
			rCopy.m_arStaticParams.begin(),
			rCopy.m_arStaticParams.end());
	}

	// Copy variables

	m_mapVariables.clear();

	if (rCopy.m_mapVariables.empty() == false)
	{
		m_mapVariables.insert(
			rCopy.m_mapVariables.begin(),
			rCopy.m_mapVariables.end());
	}

	return *this;
}

/*----------------------------------------------------------*\
| MaterialInstanceShared implementation
\*----------------------------------------------------------*/

MaterialInstanceShared::MaterialInstanceShared(Material* pMaterial):
											   m_rManager(pMaterial->GetEngine().
												   GetGraphics().GetMaterialInstancePool()),
											   m_pMaterial(pMaterial),
											   m_nRefs(0),
											   m_hParams(NULL),
											   m_pBaseParam(NULL),
											   m_pTargetParam(NULL)
{
	// Cache base and target parameters

	m_pBaseParam = m_pMaterial->GetBaseParameter();
	m_pTargetParam = m_pMaterial->GetTargetParameter();
}

MaterialInstanceShared::MaterialInstanceShared(const MaterialInstanceShared& rInit):
											   m_rManager(rInit.m_rManager),
											   m_pMaterial(rInit.m_pMaterial),
											   m_nRefs(0),
											   m_hParams(NULL),											   
											   m_pBaseParam(NULL),
											   m_pTargetParam(NULL)
{
	// Copy parameters

	std::copy(rInit.m_arParams.begin(),
		rInit.m_arParams.end(), std::back_inserter(m_arParams));

	// Cache base parameter and target parameters

	for(EffectParameterArrayIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		if (pos->GetInfo()->GetSemantic() ==
			EffectParameterInfo::SEMANTIC_BASETEXTURE)
		{
			m_pBaseParam = &(*pos);
			
			if (m_pTargetParam != NULL)
				break;
		}
		else if (pos->GetInfo()->GetSemantic() ==
			EffectParameterInfo::SEMANTIC_TARGETTEXTURE)
		{
			m_pTargetParam = &(*pos);

			if (m_pBaseParam != NULL)
				break;
		}
	}

	if (NULL == m_pBaseParam)
		m_pBaseParam = rInit.m_pMaterial->GetBaseParameter();

	// Cache target parameter

	if (NULL == m_pTargetParam)
		m_pTargetParam = rInit.m_pMaterial->GetTargetParameter();
}

MaterialInstanceShared& MaterialInstanceShared::operator=(const MaterialInstanceShared& rAssign)
{	
	m_pMaterial = rAssign.m_pMaterial;

	m_arParams.clear();
	
	std::copy(rAssign.m_arParams.begin(),
		rAssign.m_arParams.end(), std::back_inserter(m_arParams));

	if (rAssign.m_pBaseParam != NULL)
	{
		m_pBaseParam = GetParameter(rAssign.m_pBaseParam->GetInfo());

		if (NULL == m_pBaseParam)
			m_pBaseParam = rAssign.m_pMaterial->GetBaseParameter();
	}

	if (rAssign.m_pTargetParam != NULL)
	{
		m_pTargetParam = GetParameter(rAssign.m_pTargetParam->GetInfo());

		if (NULL == m_pTargetParam)
			m_pTargetParam = rAssign.m_pMaterial->GetTargetParameter();
	}

	ReleaseParameterBlock();

	return *this;
}

MaterialInstanceShared::~MaterialInstanceShared(void)
{
	Empty();
}

EffectParameter* MaterialInstanceShared::GetParameter(LPCWSTR pszParamName)
{
	for(EffectParameterArrayIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		const EffectParameterInfo* pInfo = (*pos).GetInfo();

		if (NULL == pInfo)
			throw Error(Error::INVALID_PTR,
				__FUNCTIONW__, L"EffectParameterInfo* pInfo");

		if (pInfo->GetName() == pszParamName)
			return &(*pos);
	}

	return NULL;
}

const EffectParameter* MaterialInstanceShared::GetParameterConst(LPCWSTR pszParamName) const
{
	for(EffectParameterArrayConstIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		const EffectParameterInfo* pInfo = (*pos).GetInfo();

		if (NULL == pInfo)
			throw Error(Error::INVALID_PTR,
				__FUNCTIONW__, L"EffectParameterInfo* pInfo");

		if (pInfo->GetName() == pszParamName)
			return &(*pos);
	}

	return NULL;
}

EffectParameter* MaterialInstanceShared::GetParameter(const EffectParameterInfo* pInfo)
{
	for(EffectParameterArrayIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		if ((*pos).GetInfo() == pInfo)
			return &(*pos);
	}

	return NULL;
}

const EffectParameter* MaterialInstanceShared::GetParameterConst(const EffectParameterInfo* pInfo) const
{
	for(EffectParameterArrayConstIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		if ((*pos).GetInfo() == pInfo)
			return &(*pos);
	}

	return NULL;
}

void MaterialInstanceShared::SetParameter(EffectParameter& rCopyFrom)
{
	// Cannot set automatic or shared parameters

	if (rCopyFrom.GetInfo()->IsAutomatic() == true ||
		rCopyFrom.GetInfo()->IsShared() == true)
			throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameter* pExisting = GetParameter(rCopyFrom.GetInfo()->GetName());

	if (NULL == pExisting)
	{
		m_arParams.push_back(rCopyFrom);

		if (rCopyFrom.GetInfo()->GetSemantic() ==
			EffectParameterInfo::SEMANTIC_BASETEXTURE)
		{
			m_pBaseParam = GetParameter(rCopyFrom.GetInfo());
		}
		else if (rCopyFrom.GetInfo()->GetSemantic() ==
			EffectParameterInfo::SEMANTIC_TARGETTEXTURE)
		{
			m_pTargetParam = GetParameter(rCopyFrom.GetInfo());
		}
	}
	else
	{
		*pExisting = rCopyFrom;
	}
}

void MaterialInstanceShared::SetValue(LPCWSTR pszParamName, LPVOID pvValue)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetValue(pvValue);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetValue(pvValue);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetTexture(LPCWSTR pszParamName, Texture* pTexture)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetTexture(pTexture);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetTexture(pTexture);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetTextureCube(LPCWSTR pszParamName,
											TextureCube* pTextureCube)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetTextureCube(pTextureCube);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetTextureCube(pTextureCube);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetColor(LPCWSTR pszParamName, D3DCOLOR clrColor)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);
	
	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetColor(clrColor);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetColor(clrColor);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetColor(LPCWSTR pszParamName,
									  D3DCOLORVALUE crvColor)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetColor(crvColor);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetColor(crvColor);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetScalar(LPCWSTR pszParamName, float fScalar)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (NULL == pInfo)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetScalar(fScalar);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetScalar(fScalar);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetVector2(LPCWSTR pszParamName,
										const D3DXVECTOR2& rVector2)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (NULL == pInfo)
		throw Error(Error::INVALID_PTR,
			__FUNCTIONW__, L"EffectParameterInfo* pInfo");

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetVector2(rVector2);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetVector2(rVector2);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetVector4(LPCWSTR pszParamName,
										const D3DXVECTOR4& rVector4)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetVector4(rVector4);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetVector4(rVector4);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::SetMatrix(LPCWSTR pszParamName,
									   const D3DXMATRIX& rMatrix)
{
	if (NULL == m_pMaterial || NULL == m_pMaterial->GetEffect())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Cannot set automatic or shared parameters

	EffectParameterInfo* pInfo =
		m_pMaterial->GetEffect()->GetParameterInfo(pszParamName);

	if (pInfo->IsAutomatic() == true || pInfo->IsShared() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Find parameter

	EffectParameter* pParam = GetParameter(pszParamName);

	if (NULL == pParam)
	{
		// Add this parameter and set its value

		EffectParameter param(pInfo);
		param.SetMatrix(rMatrix);

		m_arParams.push_back(param);
	}
	else
	{
		// Set parameter value

		pParam->SetMatrix(rMatrix);
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::RemoveParameter(LPCWSTR pszParamName)
{
	// Find and remove parameter by name

	for(EffectParameterArrayIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		if (pos->GetInfo()->GetName() == pszParamName)
		{
			m_arParams.erase(pos);
			return;
		}
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::RemoveParameter(const EffectParameterInfo* pInfo)
{
	// Find and remove parameter by handle

	for(EffectParameterArrayIterator pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		if (pos->GetInfo() == pInfo)
		{
			m_arParams.erase(pos);
			return;
		}
	}

	// Dirty parameter block

	if (m_hParams != NULL)
		ReleaseParameterBlock();
}

void MaterialInstanceShared::Apply(void)
{
	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Capture parameter block if dirty

	if (NULL == m_hParams)
		CaptureParameterBlock();

	// Set technique

	if (m_pMaterial->GetEffect() == NULL)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial->GetEffect()");

	m_pMaterial->GetEffect()->SetTechnique(GetTechniqueConst()->GetHandle());

	// Apply parameter block

	ApplyParameterBlock();

	// Apply automatic parameters in material

	if (m_pMaterial->GetEffectConst()->HasAutoParams() == true)
		ApplyAutoParameters();
}

void MaterialInstanceShared::ApplyChanges(void)
{
	ReleaseParameterBlock();
	CaptureParameterBlock();
}

UINT MaterialInstanceShared::Begin(void)
{
	// Validate

	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial");

	if (NULL == m_pMaterial->GetEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()");

	if (NULL == m_pMaterial->GetEffect()->GetD3DXEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()->GetD3DXEffect()");	

	// Do not save state because we are using our own save/restore code

	UINT uPasses = 0;

	HRESULT hr = m_pMaterial->GetEffect()->GetD3DXEffect()->Begin(
		&uPasses, D3DXFX_DONOTSAVESTATE);

	if (FAILED(hr))
	{
		throw m_pMaterial->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_BEGIN, __FUNCTIONW__, hr);
	}

	return uPasses;
}

void MaterialInstanceShared::BeginPass(UINT nPass)
{
	// Validate

	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial");

	if (NULL == m_pMaterial->GetEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()");

	if (NULL == m_pMaterial->GetEffect()->GetD3DXEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()->GetD3DXEffect()");

	// Begin a new pass, which prepares device for rendering

	HRESULT hr =
		m_pMaterial->GetEffect()->GetD3DXEffect()->BeginPass(nPass);

	if (FAILED(hr))
		throw m_pMaterial->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_BEGINPASS, __FUNCTIONW__, hr);
}

void MaterialInstanceShared::CommitChanges(void)
{
	// Validate

	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial");

	if (NULL == m_pMaterial->GetEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()");

	if (NULL == m_pMaterial->GetEffect()->GetD3DXEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()->GetD3DXEffect()");

	// Propagate shader constant changes to the shader

	HRESULT hr = m_pMaterial->GetEffect()->GetD3DXEffect()->CommitChanges();

	if (FAILED(hr))
		throw m_pMaterial->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_COMMITCHANGES, __FUNCTIONW__, hr);
}

void MaterialInstanceShared::EndPass(void)
{
	// Validate

	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial");

	if (NULL == m_pMaterial->GetEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()");

	if (NULL == m_pMaterial->GetEffect()->GetD3DXEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()->GetD3DXEffect()");

	// End current pass

	HRESULT hr =
		m_pMaterial->GetEffect()->GetD3DXEffect()->EndPass();

	if (FAILED(hr))
		throw m_pMaterial->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_ENDPASS, __FUNCTIONW__, hr);
}

void MaterialInstanceShared::End(void)
{
	// Validate

	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pMaterial");

	if (NULL == m_pMaterial->GetEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()");

	if (NULL == m_pMaterial->GetEffect()->GetD3DXEffect())
		throw m_pMaterial->GetEngine().GetErrors().Push(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pMaterial->GetEffect()->GetD3DXEffect()");

	// End effect rendering

	HRESULT hr =
		m_pMaterial->GetEffect()->GetD3DXEffect()->End();

	if (FAILED(hr))
		throw m_pMaterial->GetEngine().GetErrors().Push(
			Error::D3DX_EFFECT_END, __FUNCTIONW__, hr);

	// Restore states

	m_pMaterial->GetTechnique()->GetRestoreStates().Apply();
}

void MaterialInstanceShared::OnLostDevice(bool bRecreate)
{
	ReleaseParameterBlock();
}

void MaterialInstanceShared::OnResetDevice(bool bRecreate)
{
	CaptureParameterBlock();
}

int MaterialInstanceShared::AddRef(void)
{
	return ++m_nRefs;
}

bool MaterialInstanceShared::operator==(const MaterialInstanceShared& rCompare) const
{
	if (m_pMaterial != rCompare.m_pMaterial)
		return false;

	if (m_arParams.size() != rCompare.m_arParams.size())
		return false;

	if (std::equal(m_arParams.begin(),
	   m_arParams.end(),
	   rCompare.m_arParams.begin()) == false)
		return false;

	return true;
}

bool MaterialInstanceShared::operator<(const MaterialInstanceShared& rCompare) const
{
	return (m_pMaterial < rCompare.m_pMaterial);
}

void MaterialInstanceShared::Empty(void)
{
	// If releasing when shared instance is still used elsewhere, assert.
	// Assert fail here is better than nearly untraceable crash elsewhere

	//_ASSERT(m_nRefs < 1);

	ReleaseParameterBlock();
	m_arParams.clear();
}

void MaterialInstanceShared::Release(void)
{
	if (--m_nRefs <= 0)
		m_rManager.Remove(this);
}

void MaterialInstanceShared::CaptureParameterBlock(void)
{
	if (m_pMaterial->GetParameterCount() == 0 &&
	   m_arParams.empty() == true)
	{
		m_hParams = D3DXHANDLE(INVALID_PTR);
		return;
	}

	LPD3DXEFFECT pD3DEffect = m_pMaterial->GetEffect()->GetD3DXEffect();

	HRESULT hr = pD3DEffect->BeginParameterBlock();

	if (FAILED(hr))
		throw Error(Error::D3DX_EFFECT_BEGINPARAMETERBLOCK, __FUNCTIONW__, hr);

	EffectParameterArrayIterator pos;

	for(pos = m_pMaterial->GetBeginParameterPos();
		pos != m_pMaterial->GetEndParameterPos();
		pos++)
	{
		(*pos).Apply();
	}

	for(pos = m_arParams.begin();
		pos != m_arParams.end();
		pos++)
	{
		(*pos).Apply();
	}

	m_hParams = pD3DEffect->EndParameterBlock();
}

void MaterialInstanceShared::ApplyParameterBlock(void)
{
	// If marked with INVALID_VALUE, no params to set

	if (D3DXHANDLE(INVALID_PTR) == m_hParams)
		return;

	HRESULT hr = m_pMaterial->GetEffect()->
		GetD3DXEffect()->ApplyParameterBlock(m_hParams);

	if (FAILED(hr))
		throw Error(Error::D3DX_EFFECT_APPLYPARAMETERBLOCK,
			__FUNCTIONW__, hr);
}

void MaterialInstanceShared::ReleaseParameterBlock(void)
{
	if (m_hParams != NULL &&
	   m_hParams != D3DXHANDLE(INVALID_VALUE) &&
	   m_pMaterial != NULL &&
	   m_pMaterial->GetEffect() != NULL &&
	   m_pMaterial->GetEffect()->GetD3DXEffect() != NULL)
	{
		HRESULT hr = m_pMaterial->GetEffect()->
			GetD3DXEffect()->DeleteParameterBlock(m_hParams);

		if (FAILED(hr))
			throw Error(Error::D3DX_EFFECT_DELETEPARAMETERBLOCK,
				__FUNCTIONW__, hr);
	}

	m_hParams = NULL;
}

void MaterialInstanceShared::ApplyAutoParameters(void)
{
	if (NULL == m_pMaterial)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	Effect* pEffect = m_pMaterial->GetEffect();

	if (NULL == pEffect)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"Effect* pEffect");

	LPD3DXEFFECT pD3DEffect = pEffect->GetD3DXEffect();

	if (NULL == pD3DEffect)
		throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"LPD3DXEFFECT* pD3DEffect");

	StateManager* pStates =
		m_pMaterial->GetEngine().GetGraphics().GetStates();

	for(EffectParameterInfoArrayIterator pos =
		pEffect->GetBeginAutoParameterInfoPos();
		pos != pEffect->GetEndAutoParameterInfoPos();
		pos++)
	{
		switch((*pos)->GetSemantic())
		{
		case EffectParameterInfo::SEMANTIC_WORLD:
			{
				pD3DEffect->SetMatrix((*pos)->GetHandle(),
					static_cast<const D3DXMATRIX*>
					(&pStates->GetTransform(D3DTS_WORLD)));
			}
			break;
		case EffectParameterInfo::SEMANTIC_VIEW:
			{
				pD3DEffect->SetMatrix((*pos)->GetHandle(),
					static_cast<const D3DXMATRIX*>
					(&pStates->GetTransform(D3DTS_VIEW)));
			}
			break;
		case EffectParameterInfo::SEMANTIC_PROJ:
			{
				pD3DEffect->SetMatrix((*pos)->GetHandle(),
					static_cast<const D3DXMATRIX*>
					(&pStates->GetTransform(D3DTS_PROJECTION)));
			}
			break;
		case EffectParameterInfo::SEMANTIC_VIEWPROJ:
			{
				if ((*pos)->GetLastFrameChanged() !=
					pStates->GetLastVPChangeFrame())
				{
					pD3DEffect->SetMatrix((*pos)->GetHandle(),
						static_cast<const D3DXMATRIX*>
						(&pStates->GetTransformCombination(
							StateManager::TRANSFORM_VIEWPROJ)));

					(*pos)->SetLastFrameChanged(
						m_pMaterial->GetEngine().GetGraphics().GetFrameID());
				}
			}
			break;
		case EffectParameterInfo::SEMANTIC_WORLDVIEWPROJ:
			{
				if ((*pos)->GetLastFrameChanged() !=
					pStates->GetLastWVPChangeFrame())
				{
					pD3DEffect->SetMatrix((*pos)->GetHandle(),
						static_cast<const D3DXMATRIX*>
						(&pStates->GetTransformCombination(
							StateManager::TRANSFORM_WORLDVIEWPROJ)));

					(*pos)->SetLastFrameChanged(
						m_pMaterial->GetEngine().GetGraphics().GetFrameID());
				}
			}
			break;
		case EffectParameterInfo::SEMANTIC_TIME:
			{
				if ((*pos)->GetLastFrameChanged() !=
					m_pMaterial->GetEngine().GetGraphics().GetFrameID())
				{
					pD3DEffect->SetFloat((*pos)->GetHandle(),
						pEffect->GetEngine().GetTime());

					(*pos)->SetLastFrameChanged(
						m_pMaterial->GetEngine().GetGraphics().GetFrameID());
				}
			}
			break;
		case EffectParameterInfo::SEMANTIC_RUNTIME:
			{
				if ((*pos)->GetLastFrameChanged() !=
					m_pMaterial->GetEngine().GetGraphics().GetFrameID())
				{
					pD3DEffect->SetFloat((*pos)->GetHandle(),
						pEffect->GetEngineConst().GetRunTime());

					(*pos)->SetLastFrameChanged(
						m_pMaterial->GetEngine().GetGraphics().GetFrameID());
				}
			}
			break;
		case EffectParameterInfo::SEMANTIC_FRAMETIME:
			{
				pD3DEffect->SetFloat((*pos)->GetHandle(),
					pEffect->GetEngineConst().GetFrameTime());
			}
			break;
		case EffectParameterInfo::SEMANTIC_TARGETSIZE:
			{
				// Will probably need to optimize this at some point

				LPDIRECT3DDEVICE9 pDevice =
					pEffect->GetEngine().GetGraphics().GetDevice();

				LPDIRECT3DSURFACE9 pRT = NULL;
				D3DSURFACE_DESC desc;

				HRESULT hr = pDevice->GetRenderTarget(0, &pRT);

				if (FAILED(hr))
					throw Error(Error::D3D_DEVICE_GETRENDERTARGET,
						__FUNCTIONW__, hr);

				hr = pRT->GetDesc(&desc);

				if (FAILED(hr))
					throw Error(Error::D3D_SURFACE_GETDESC,
						__FUNCTIONW__, hr);

				pRT->Release();

				float fSize[] = { static_cast<float>(desc.Width),
								  static_cast<float>(desc.Height) };

				pD3DEffect->SetFloatArray((*pos)->GetHandle(), fSize, 2);
			}
			break;
		}
	}
}

/*----------------------------------------------------------*\
| EffectPool implementation
\*----------------------------------------------------------*/

EffectPool::EffectPool(void): m_pD3DXEffectPool(NULL)
{
}

EffectPool::~EffectPool(void)
{
	Empty();
}

void EffectPool::Initialize(void)
{
	HRESULT hr = D3DXCreateEffectPool(&m_pD3DXEffectPool);

	if (FAILED(hr))
		throw Error(Error::D3DX_CREATEEFFECTPOOL, __FUNCTIONW__, hr);
}

LPD3DXEFFECTPOOL EffectPool::GetD3DXEffectPool(void)
{
	return m_pD3DXEffectPool;
}

EffectParameterInfo* EffectPool::Add(EffectParameterInfo* pParam)
{
	// Check if exists

	EffectParameterInfoArrayIterator posFind =
		std::find(m_arPoolParams.begin(),
				  m_arPoolParams.end(),
				  pParam);

	if (m_arPoolParams.end() != posFind)
		return *posFind;

	// If doesn't exist, allocate in heap memory, copy, and return pointer
	// Deallocation will happen when Release() is called on param info

	EffectParameterInfo* pNewShared = NULL;

	try
	{
		pNewShared = new EffectParameterInfo(*pParam);
	}

	catch(std::bad_alloc)
	{
		throw Error(Error::MEM_ALLOC,
			__FUNCTIONW__, sizeof(EffectParameterInfo));
	}

	m_arPoolParams.push_back(pNewShared);

	return pNewShared;
}

void EffectPool::Remove(EffectParameterInfo* pParam)
{
	EffectParameterInfoArrayIterator posFind =
		std::find(m_arPoolParams.begin(),
				  m_arPoolParams.end(),
				  pParam);

	if (m_arPoolParams.end() != posFind)
		m_arPoolParams.erase(posFind);
}

void EffectPool::Empty(void)
{
	if (m_pD3DXEffectPool != NULL)
	{
		m_pD3DXEffectPool->Release();
		m_pD3DXEffectPool = NULL;
	}
}

/*----------------------------------------------------------*\
| MaterialInstance implementation
\*----------------------------------------------------------*/

MaterialInstance::MaterialInstance(void): m_pSharedInstance(NULL),
										  m_dwFlags(STATIC)
{
	ZeroMemory(&m_rcTextureCoords, sizeof(RECT));
}

MaterialInstance::MaterialInstance(const MaterialInstance& rInit):
								   m_pSharedInstance(rInit.m_pSharedInstance),
								   m_dwFlags(rInit.m_dwFlags)
{
	CopyMemory(&m_rcTextureCoords, &rInit.m_rcTextureCoords, sizeof(RECT));
}

MaterialInstance::MaterialInstance(Material* pMaterial, const Rect& rrcTextureCoords):
									m_pSharedInstance(NULL),
									m_dwFlags(STATIC)
{
	CopyMemory(&m_rcTextureCoords, &rrcTextureCoords, sizeof(RECT));
	SetMaterial(pMaterial);
}

MaterialInstance::MaterialInstance(Material* pMaterial, Texture* pBaseTexture,
								   const Rect& rrcTextureCoords):
									m_pSharedInstance(NULL),
									m_dwFlags(STATIC)
{
	CopyMemory(&m_rcTextureCoords, &rrcTextureCoords, sizeof(RECT));
	SetMaterial(pMaterial);
	SetBaseTexture(pBaseTexture);
}

MaterialInstance::MaterialInstance(Material* pMaterial, Texture* pBaseTexture):
									m_pSharedInstance(NULL),
									m_dwFlags(STATIC)
{
	SetMaterial(pMaterial);

	if (pBaseTexture != NULL)
	{
		SetBaseTexture(pBaseTexture);

		m_rcTextureCoords.left = 0;
		m_rcTextureCoords.top = 0;
		m_rcTextureCoords.right = int(pBaseTexture->GetInfo().Width);
		m_rcTextureCoords.bottom = int(pBaseTexture->GetInfo().Height);
	}
}

MaterialInstance::MaterialInstance(Material* pMaterial):
									m_pSharedInstance(NULL),
									m_dwFlags(STATIC)
{
	SetMaterial(pMaterial);

	// Set texture coords from base texture if one is found

	if (m_pSharedInstance != NULL)
	{
		Texture* pBase = GetBaseTexture();

		if (pBase != NULL)
		{
			m_rcTextureCoords.left = 0;
			m_rcTextureCoords.top = 0;
			m_rcTextureCoords.right = int(pBase->GetInfo().Width);
			m_rcTextureCoords.bottom = int(pBase->GetInfo().Height);
		}
	}
}

MaterialInstance::MaterialInstance(Animation* pAnimation):
									m_pSharedInstance(NULL),
									m_dwFlags(ANIMATED)
{
	SetAnimation(pAnimation);
}

MaterialInstance::~MaterialInstance(void)
{
	Empty();
}

Texture* MaterialInstance::GetBaseTexture(void)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameter* pBaseParam = m_pSharedInstance->GetBaseParam();

	if (pBaseParam != NULL)
		return pBaseParam->GetTexture();

	return NULL;
}

const Texture* MaterialInstance::GetBaseTextureConst(void) const
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	EffectParameter* pBaseParam = m_pSharedInstance->GetBaseParam();

	if (pBaseParam != NULL)
		return pBaseParam->GetTextureConst();

	return NULL;
}

const EffectParameter* MaterialInstance::GetParameter(LPCWSTR pszParamName) const
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	return m_pSharedInstance->GetParameter(pszParamName);
}

void MaterialInstance::SetValue(LPCWSTR pszParamName, LPVOID pvValue)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetValue(pszParamName, pvValue);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetTexture(LPCWSTR pszParamName,
								  Texture* pTexture)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetTexture(pszParamName, pTexture);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetBaseTexture(Texture* pTexture)
{
	if (NULL == m_pSharedInstance ||
		NULL == m_pSharedInstance->GetMaterial() ||
		NULL == m_pSharedInstance->GetMaterial()->GetEffect() ||
		NULL == m_pSharedInstance->GetMaterial()->GetEffect()->GetBaseParamInfo())
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);

	if (pTexture != NULL)
	{
		EffectParameter paramBaseTex(
			m_pSharedInstance->GetMaterial()->GetEffect()->GetBaseParamInfo());

		paramBaseTex.SetTexture(pTexture);
		copy.SetParameter(paramBaseTex);
	}
	else
	{
		copy.RemoveParameter(
			m_pSharedInstance->GetMaterial()->GetEffect()->GetBaseParamInfo());
	}

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetTextureCube(LPCWSTR pszParamName,
									  TextureCube* pTextureCube)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetTextureCube(pszParamName, pTextureCube);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetColor(LPCWSTR pszParamName,
								const Color& rColor)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetColor(pszParamName, rColor);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetScalar(LPCWSTR pszParamName, float fScalar)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetScalar(pszParamName, fScalar);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetVector2(LPCWSTR pszParamName,
								  const D3DXVECTOR2& rVector2)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetVector2(pszParamName, rVector2);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetVector4(LPCWSTR pszParamName,
								  const D3DXVECTOR4& rVector4)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetVector4(pszParamName, rVector4);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetMatrix(LPCWSTR pszParamName,
								 const D3DXMATRIX& rMatrix)
{
	if (NULL == m_pSharedInstance)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Update shared instance

	MaterialInstanceShared copy(*m_pSharedInstance);
	copy.SetMatrix(pszParamName, rMatrix);

	UpdateSharedInstance(copy);
}

void MaterialInstance::SetAnimation(Animation* pAnimation)
{
	SAFERELEASE(m_pAnimation);

	m_pAnimation = pAnimation;

	if (m_pAnimation != NULL)
	{
		m_pAnimation->AddRef();

		m_dwFlags |= ANIMATED;

		SetBaseTexture(m_pAnimation->GetBaseTexture());
	}
	else
	{
		m_dwFlags &= ~ANIMATED;
	}
}

void MaterialInstance::Play(float fTime,
							bool bLoop,
							int nSequence,
							bool bReverse)
{
	m_fLastUpdateTime = fTime;

	if (true == bLoop)
		m_dwFlags |= LOOPING;
	else
		m_dwFlags &= ~LOOPING;

	if (nSequence < 0 ||
	   nSequence >= m_pAnimation->GetSequenceCount())
		m_wSequenceID = 0;
	else
		m_wSequenceID = LOWORD(nSequence);

	m_wNextSequenceID = 0;

	if (true == bReverse)
		m_dwFlags |= REVERSE;
	else
		m_dwFlags &= ~REVERSE;

	m_dwFlags |= PLAYING;
}

void MaterialInstance::Update(float fTime)
{
	// If stopped, do not update

	if (~m_dwFlags & PLAYING)
		return;

	// Validate

	if (NULL == m_pAnimation)
		throw Error(Error::INVALID_PTR,
			__FUNCTIONW__, L"m_pAnimation");

	if (m_pAnimation->GetSequenceCount() != 0 &&
		m_wSequenceID >= LOWORD(m_pAnimation->GetSequenceCount()))
		throw Error(Error::INVALID_INDEX,
			__FUNCTIONW__, L"m_wSequenceID");

	// Calculate time elapsed since last frame change

	float fElapsed = fTime - m_fLastUpdateTime;

	// Normalize elapsed time to animation duration

	if (fElapsed >= m_pAnimation->GetDuration())
	{
		float fRepeat = fElapsed /
			m_pAnimation->GetDuration();

		fElapsed = (fRepeat - floor(fRepeat))
			* m_pAnimation->GetDuration();
	}

	// Remember current frame

	int nLastFrameID = m_nFrameID;

	// Advance current frame

	for(;;)
	{
		// Get current frame info

		const Frame& rFrame = m_pAnimation->GetFrameConst(m_nFrameID);

		// If current frame still not over, exit

		if (rFrame.GetDuration() > fElapsed)
			break;
		else
			fElapsed = fElapsed - rFrame.GetDuration();

		// Advance frame

		if (m_pAnimation->GetSequenceCount() != 0)
		{
			// Advance taking sequences into account

			const Sequence& rSequence =
				m_pAnimation->GetSequenceConst(int(m_wSequenceID));

			if (m_dwFlags & REVERSE)
			{
				if (rSequence.GetFirstFrameID() == m_nFrameID)
				{
					if (~m_dwFlags & LOOPING)
					{
						m_dwFlags &= ~PLAYING;
					}
					else
					{
						if (m_dwFlags & TRIGGERED)
						{
							// If triggered, switch to next sequence's last frame

							m_wSequenceID = m_wNextSequenceID;
							m_dwFlags = (m_dwFlags & ~TRIGGERED) | WAITSIGNALED;

							m_nFrameID = m_pAnimation->GetSequenceConst(
								int(m_wNextSequenceID)).GetLastFrameID();
						}
						else if (rSequence.GetNextSequenceID() != NULL)
						{
							// Wrap to this sequence's next sequence last frame

							m_wSequenceID = LOWORD(rSequence.GetNextSequenceID());

							m_nFrameID = m_pAnimation->GetSequenceConst(
								int(m_wSequenceID)).GetLastFrameID();
						}
						else
						{
							// Wrap to last frame

							m_nFrameID = rSequence.GetLastFrameID();
						}
					}

					// If triggered sequence is over, set signaled flag

					if (m_dwFlags & WAITSIGNALED)
						m_dwFlags = (m_dwFlags & ~WAITSIGNALED) | SIGNALED;

					break;
				}
			}
			else
			{
				if (rSequence.GetLastFrameID() == m_nFrameID)
				{
					if (~m_dwFlags & LOOPING)
					{
						m_dwFlags &= ~PLAYING;
					}
					else
					{
						if (m_dwFlags & TRIGGERED)
						{
							// If triggered, switch to next sequence's first frame

							m_wSequenceID = m_wNextSequenceID;
							m_dwFlags = (m_dwFlags & ~TRIGGERED) | WAITSIGNALED;

							m_nFrameID = m_pAnimation->GetSequenceConst(
								int(m_wNextSequenceID)).GetFirstFrameID();
						}
						else if (rSequence.GetNextSequenceID() != INVALID_INDEX)
						{
							// Wrap to next sequence's first frame

							m_wSequenceID = LOWORD(rSequence.GetNextSequenceID());

							m_nFrameID = m_pAnimation->GetSequenceConst(
									int(m_wSequenceID)).GetFirstFrameID();
						}
						else
						{
							// Wrap to first frame

							m_nFrameID = rSequence.GetFirstFrameID();
						}
					}

					// If triggered sequence is over, set signaled flag

					if (m_dwFlags & WAITSIGNALED)
						m_dwFlags = (m_dwFlags & ~WAITSIGNALED) | SIGNALED;

					break;
				}
			}

			if (m_dwFlags & REVERSE)
				m_nFrameID--;
			else
				m_nFrameID++;
		}
		else
		{
			// Advance assuming one inclusive sequence

			if (m_dwFlags & REVERSE)
			{
				if (0 == m_nFrameID)
				{
					if (~m_dwFlags & LOOPING)
					{
						m_dwFlags &= ~PLAYING;					
						break;
					}

					m_nFrameID = (m_pAnimation->GetFrameCount() - 1);
					break;
				}
			}
			else
			{
				if ((m_pAnimation->GetFrameCount() - 1) == m_nFrameID)
				{
					if (~m_dwFlags & LOOPING)
					{
						m_dwFlags &= ~PLAYING;					
						break;
					}

					m_nFrameID = 0;
					break;
				}
			}

			if (m_dwFlags & REVERSE)
				m_nFrameID--;
			else
				m_nFrameID++;
		}
	} // for

	if (nLastFrameID != m_nFrameID)
	{
		// Validate frame

		if (m_nFrameID >= m_pAnimation->GetFrameCount() ||
		   m_nFrameID < 0)
			throw Error(Error::INVALID_INDEX,
				__FUNCTIONW__, L"m_nFrameID");

		// Remember time last changed frame

		m_fLastUpdateTime = fTime;
	}
}

void MaterialInstance::TriggerSequence(int nSequenceID, bool bLoop)
{
	if (IsAnimated() == false)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Trigger specified sequence as next sequence

	m_wNextSequenceID = LOWORD(nSequenceID);

	m_dwFlags |= TRIGGERED;

	// Update the loop flag

	if (true == bLoop)
	{
		if (~m_dwFlags & LOOPING)
			m_dwFlags |= LOOPING;
	}
	else
	{
		if (m_dwFlags & LOOPING)
			m_dwFlags &= ~LOOPING;
	}
}

void MaterialInstance::TriggerSequence(LPCWSTR pszSequence, bool bLoop)
{
	if (IsAnimated() == false)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	// Trigger specified sequence as next sequence

	int nID = m_pAnimation->GetSequenceID(pszSequence);

	if (INVALID_INDEX == nID)
		throw Error(Error::INVALID_PARAM, __FUNCTIONW__, 0);

	m_wNextSequenceID = LOWORD(nID);

	m_dwFlags |= TRIGGERED;

	// Update the loop flag

	if (true == bLoop)
	{
		if (~m_dwFlags & LOOPING)
			m_dwFlags |= LOOPING;
	}
	else
	{
		if (m_dwFlags & LOOPING)
			m_dwFlags &= ~LOOPING;
	}
}

void MaterialInstance::SetTextureCoords(const RECT& rrcTextureCoords)
{
	if (IsAnimated() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	CopyMemory(&m_rcTextureCoords, &rrcTextureCoords, sizeof(RECT));
}

void MaterialInstance::SetTextureCoords(int x,
										int y,
										int nWidth,
										int nHeight)
{
	if (IsAnimated() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	m_rcTextureCoords.left = x;
	m_rcTextureCoords.top = y;
	m_rcTextureCoords.right = x + nWidth;
	m_rcTextureCoords.bottom = y + nHeight;
}

void MaterialInstance::GetTextureCoords(float& ru1,
										float& rv1,
										float& ru2,
										float& rv2) const
{
	const Texture* pTex = GetBaseTextureConst();

	if (NULL == pTex)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	const D3DXIMAGE_INFO& rInfo = pTex->GetInfo();

	if (IsAnimated() == true)
	{
		if (NULL == m_pAnimation)
			throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"m_pAnimation");

		const Frame* pCurFrame = GetCurrentFrameConst();

		if (NULL == pCurFrame)
			throw Error(Error::INVALID_PTR, __FUNCTIONW__, L"pCurFrame");

		ru1 = float(pCurFrame->GetTextureCoordsConst().x) / float(rInfo.Width);
		rv1 = float(pCurFrame->GetTextureCoordsConst().y) / float(rInfo.Height);
		ru2 = float(pCurFrame->GetTextureCoordsConst().x +
			m_pAnimation->GetFrameSize().cx) / float(rInfo.Width);
		rv2 = float(pCurFrame->GetTextureCoordsConst().y +
			m_pAnimation->GetFrameSize().cy) / float(rInfo.Height);
	}
	else
	{
		ru1 = float(m_rcTextureCoords.left) / float(rInfo.Width);
		rv1 = float(m_rcTextureCoords.top) / float(rInfo.Height);
		ru2 = float(m_rcTextureCoords.right) / float(rInfo.Width);
		rv2 = float(m_rcTextureCoords.bottom) / float(rInfo.Height);
	}
}

void MaterialInstance::SetTextureCoords(float u1,
										float v1,
										float u2,
										float v2)
{
	if (IsAnimated() == true)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	Texture* pTex = GetBaseTexture();

	if (NULL == pTex)
		throw Error(Error::INVALID_CALL, __FUNCTIONW__);

	const D3DXIMAGE_INFO& rInfo = pTex->GetInfo();

	m_rcTextureCoords.left = int(u1 * float(rInfo.Width));
	m_rcTextureCoords.top = int(v1 * float(rInfo.Height));
	m_rcTextureCoords.right = int(u2 * float(rInfo.Width));
	m_rcTextureCoords.bottom = int(v2 * float(rInfo.Height));
}

void MaterialInstance::Serialize(const Engine& rEngine,
								 Stream& rStream,
								 bool bEmbed) const
{
	// Write flags

	rStream.WriteVar(&m_dwFlags);

	// Write material or animation instance

	if (true == bEmbed)
	{
		if (m_dwFlags & ANIMATED)
		{
			if (NULL == m_pAnimation)
				Object::SerializeNullInstance(rStream);
			else
				m_pAnimation->SerializeInstance(rStream);
		}
		else
		{
			if (NULL == m_pSharedInstance)
				Object::SerializeNullInstance(rStream);
			else
				GetMaterial()->SerializeInstance(rStream);
		}
	}

	// Write parameters

	int nParams = m_pSharedInstance->GetParameterCount();
	rStream.WriteVar(&nParams);

	for(EffectParameterArrayIterator pos =
		m_pSharedInstance->GetBeginParameterPos();
		pos != m_pSharedInstance->GetEndParameterPos();
		pos++)
	{
		(*pos).GetInfo()->GetName().Serialize(rStream);
		(*pos).Serialize(rEngine, rStream);
	}

	// Write data

	rStream.Write(&m_rcTextureCoords, sizeof(RECT));
}

void MaterialInstance::Deserialize(Engine& rEngine,
								   Stream& rStream,
								   bool bEmbed)
{
	try
	{
		// Read flags

		rStream.ReadVar(&m_dwFlags);

		Material* pMaterial = NULL;

		if (true == bEmbed)
		{
			// Read material instance to use

			pMaterial = rEngine.GetMaterials().LoadInstance(rStream);

			SetMaterial(pMaterial);
		}
		else
		{
			// Already set

			pMaterial = GetMaterial();
		}

		// Read animation instance inline if embedded
		// If not embedded, animation pointer will be cached by parent from an index

		if ((m_dwFlags & ANIMATED) && (true == bEmbed))
		{
			m_pAnimation = rEngine.GetAnimations().LoadInstance(rStream);
			m_pAnimation->AddRef();
		}		

		// Read parameters

		int nParams = 0;
		rStream.ReadVar(&nParams);

		String strName;

		while(nParams-- > 0)
		{
			strName.Deserialize(rStream);

			EffectParameter param(pMaterial->GetEffect()->
				GetParameterInfo(strName));

			param.Deserialize(rEngine, rStream);

			m_pSharedInstance->SetParameter(param);
		}

		// Read data
		// If static, this reads texture coordinates
		// If animated, this reads animation values

		rStream.Read(&m_rcTextureCoords, sizeof(RECT));
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rStream.GetPath());
	}
}

void MaterialInstance::Serialize(Engine& rEngine, InfoElem& rRoot) const
{
	try
	{
		// Write flags

		if (m_dwFlags != STATIC)
			rRoot.AddChild(SZ_FLAGS)->FromFlags(m_dwFlags,
				SZ_FLAGNAMES,
				DW_FLAGS,
				sizeof(DW_FLAGS) / sizeof(DWORD));

		// Write material or animation instance

		if (m_dwFlags & ANIMATED)
		{
			if (m_pAnimation != NULL)
				m_pAnimation->SerializeInstance(rRoot);
			else
				Object::SerializeNullInstance(rRoot);
		}
		else
		{
			if (m_pSharedInstance != NULL)
				GetMaterial()->SerializeInstance(rRoot);
			else
				Object::SerializeNullInstance(rRoot);
		}

		// Write parameters

		if (m_pSharedInstance != NULL &&
			m_pSharedInstance->GetParameterCount() > 0)
		{
			rRoot.SetElemType(InfoElem::TYPE_VALUEBLOCK);

			for(EffectParameterArrayIterator pos =
				m_pSharedInstance->GetBeginParameterPos();
				pos != m_pSharedInstance->GetEndParameterPos();
				pos++)
			{
				(*pos).Serialize(rEngine,
					*rRoot.AddChild((*pos).GetInfo()->GetName()));
			}
		}

		if (rRoot.GetElemType() != InfoElem::TYPE_VALUEBLOCK)
		{
			rRoot.AddChild()->SetStringValue(rRoot.GetStringValue());
			rRoot.SetIntValue(0);
			rRoot.SetElemType(InfoElem::TYPE_VALUELIST);
		}

		// Write data

		if (m_dwFlags & ANIMATED)
		{
			// Write current frame

			rRoot.AddChild(SZ_CURFRAME)->
				SetIntValue(m_nFrameID);

			// Write current sequence

			rRoot.AddChild(SZ_CURSEQ)->
				SetIntValue(int(m_wSequenceID));

			// Write next sequence

			rRoot.AddChild(SZ_NEXTSEQ)->
				SetIntValue(int(m_wNextSequenceID));
		}
		else
		{
			// Write texture coordinates

			Rect(m_rcTextureCoords).Serialize(rRoot);
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		rEngine.GetErrors().Push(Error::FILE_SERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void MaterialInstance::Deserialize(Engine& rEngine, const InfoElem& rRoot)
{
	try
	{
		int nTexCoordsStart = 1;

		if (rRoot.GetElemType() == InfoElem::TYPE_VALUELIST)
		{
			// If inline value list, set material from first child
			
			SetMaterial(rEngine.GetMaterials().LoadInstance(
				*rRoot.GetChildConst(0)));

			// No more values specified, done reading

			if (rRoot.GetChildCount() < 2)
				return;

			// Determine what second child contains...

			switch(rRoot.GetChildConst(1)->GetVarType())
			{
			case Variable::TYPE_ENUM:
				{
					// If second child is enum, it's flags
					// This is probably an animation in that case

					m_dwFlags = rRoot.ToFlags(
						SZ_FLAGNAMES,
						DW_FLAGS,
						sizeof(DW_FLAGS) / sizeof(DWORD),
						STATIC, 1);

					nTexCoordsStart++;

					const InfoElem* pNextElem = rRoot.GetChildConst(nTexCoordsStart);

					for(;; nTexCoordsStart++)
					{
						if (NULL == pNextElem ||
						   pNextElem->GetVarType() != Variable::TYPE_ENUM)
						   break;

						pNextElem = rRoot.GetChildConst(nTexCoordsStart);
					}

					if (m_dwFlags & ANIMATED)
					{
						// Read animation from next child if animated

						if (pNextElem != NULL)
							SetAnimation(rEngine.GetAnimations().LoadInstance(
								*pNextElem));

						if (++nTexCoordsStart < rRoot.GetChildCount())
						{
							// Read sequence ID from next child if animated

							pNextElem = rRoot.GetChildConst(nTexCoordsStart);

							if (pNextElem != NULL &&
							   pNextElem->GetVarType() == Variable::TYPE_INT)
								m_wSequenceID = LOWORD(pNextElem->GetIntValue());
						}
						else
						{
							break;
						}

						if (++nTexCoordsStart < rRoot.GetChildCount())
						{
							// Read next sequence ID from next child if animated

							pNextElem = rRoot.GetChildConst(++nTexCoordsStart);

							if (pNextElem != NULL &&
							   pNextElem->GetVarType() == Variable::TYPE_INT)
								m_wNextSequenceID = LOWORD(pNextElem->GetIntValue());
						}
					}
				}
				break;
			case Variable::TYPE_STRING:
				{
					// If second element is a string, assume inline texture

					SetBaseTexture(rEngine.GetTextures().LoadInstance(
						*rRoot.GetChildConst(1)));

					m_dwFlags = STATIC;

					nTexCoordsStart++;
				}
				break;
			}
		}
		else if (rRoot.GetElemType() == InfoElem::TYPE_VALUE)
		{
			// If only one value, we assume it's material

			SetMaterial(rEngine.GetMaterials().LoadInstance(rRoot));

			return;
		}
		else
		{
			const InfoElem* pElem = NULL;

			if (rRoot.GetElemType() == InfoElem::TYPE_VALUEBLOCK)
			{
				// Read material from block value

				SetMaterial(rEngine.GetMaterials().LoadInstance(rRoot));
			}
			else
			{
				// Read material as separate element

				pElem = rRoot.FindChildConst(Material::SZ_MATERIAL,
					InfoElem::TYPE_VALUE, InfoElem::TYPE_STRING);

				if (pElem != NULL)
					SetMaterial(rEngine.GetMaterials().LoadInstance(*pElem));
			}

			// Read flags

			pElem = rRoot.FindChildConst(SZ_FLAGS);

			if (pElem != NULL)
				m_dwFlags = pElem->ToFlags(SZ_FLAGNAMES, DW_FLAGS,
					sizeof(DW_FLAGS) / sizeof(DWORD), STATIC);

			// Read animation and animation properties if animated

			if (m_dwFlags & ANIMATED)
			{
				pElem = rRoot.FindChildConst(SZ_ANIM);

				if (pElem != NULL)
					SetAnimation(rEngine.GetAnimations().LoadInstance(*pElem));

				pElem = rRoot.FindChildConst(SZ_CURFRAME,
					InfoElem::TYPE_VALUE, InfoElem::TYPE_INT);

				if (pElem != NULL)
					m_nFrameID = pElem->GetIntValue();

				pElem = rRoot.FindChildConst(SZ_CURSEQ,
					InfoElem::TYPE_VALUE, InfoElem::TYPE_INT);

				if (pElem != NULL)
					m_wSequenceID = LOWORD(pElem->GetIntValue());

				pElem = rRoot.FindChildConst(SZ_NEXTSEQ,
					InfoElem::TYPE_VALUE, InfoElem::TYPE_INT);

				if (pElem != NULL)
					m_wNextSequenceID = LOWORD(pElem->GetIntValue());
			}

			// Read shader parameters

			pElem = rRoot.FindChildConst(Material::SZ_PARAMS);

			if (pElem != NULL && pElem->GetElemType() == InfoElem::TYPE_BLOCK)
			{
				Material* pMaterial = GetMaterial();

				for(InfoElemConstIterator pos = pElem->GetBeginChildPosConst();
					pos != pElem->GetEndChildPosConst();
					pos++)
				{
					EffectParameter param(pMaterial->GetEffect()->
						GetParameterInfo((*pos)->GetName()));

					param.Deserialize(rEngine, rRoot);

					m_pSharedInstance->SetParameter(param);
				}
			}
		}		

		// Read texture coordinates

		if (~m_dwFlags & ANIMATED)
		{
			Rect rcTextureCoords;

			if (rRoot.GetElemType() == InfoElem::TYPE_VALUELIST)
			{
				if ((rRoot.GetChildCount() - nTexCoordsStart) == 0)
				{
					// Texture coordinates not specified, use whole texture

					Texture* pBase = GetBaseTexture();

					if (pBase != NULL)
						rcTextureCoords.SetSize(
							int(pBase->GetInfo().Width), int(pBase->GetInfo().Height));
				}
				else if ((rRoot.GetChildCount() - nTexCoordsStart) == 4)
				{
					rcTextureCoords.Deserialize(rRoot, nTexCoordsStart);
				}
			}
			else
			{
				const InfoElem* pElem = rRoot.FindChildConst(SZ_TEXCOORDS,
					InfoElem::TYPE_VALUELIST);

				if (pElem != NULL)
					rcTextureCoords.Deserialize(*pElem);
			}

			CopyMemory(&m_rcTextureCoords, &rcTextureCoords, sizeof(RECT));
		}
	}

	catch(Error& rError)
	{
		UNREFERENCED_PARAMETER(rError);

		throw rEngine.GetErrors().Push(Error::FILE_DESERIALIZE,
			__FUNCTIONW__, rRoot.GetDocumentConst().GetPath());
	}
}

void MaterialInstance::Empty(void)
{
	SAFERELEASE(m_pSharedInstance);

	if ((m_dwFlags & ANIMATED) && m_pAnimation != NULL)
		m_pAnimation->Release();
}

void MaterialInstance::SetMaterial(Material* pMaterial)
{
	if (NULL == pMaterial)
		return;

	MaterialInstanceShared init(pMaterial);
	UpdateSharedInstance(init);
}

MaterialInstance& MaterialInstance::operator=(const MaterialInstance& rAssign)
{
	// Copy shared instance

	if (rAssign.m_pSharedInstance != NULL)
		rAssign.m_pSharedInstance->AddRef();

	if (m_pSharedInstance != NULL)
		m_pSharedInstance->Release();

	m_pSharedInstance = rAssign.m_pSharedInstance;

	// Track animation pointer references

	if (rAssign.m_dwFlags & ANIMATED)
	{
		if (rAssign.m_pAnimation != NULL)
			rAssign.m_pAnimation->AddRef();

		if (m_dwFlags & ANIMATED && m_pAnimation != NULL)
			m_pAnimation->Release();
	}

	// Copy data (copies animation pointer value if animated)

	CopyMemory(&m_rcTextureCoords,
		&rAssign.m_rcTextureCoords, sizeof(RECT));

	// Copy flags

	m_dwFlags = rAssign.m_dwFlags;

	return *this;
}

bool MaterialInstance::operator==(const MaterialInstance& rCompare) const
{
	// Compare flags

	if (m_dwFlags != rCompare.m_dwFlags)
		return false;

	// Compare shared instance

	if (m_pSharedInstance != rCompare.m_pSharedInstance)
		return false;

	// Compare data

	if (memcmp(&m_rcTextureCoords,
		&rCompare.m_rcTextureCoords, sizeof(RECT)) != 0)
			return false;

	return true;
}

bool MaterialInstance::operator!=(const MaterialInstance& rCompare) const
{
	// Compare flags

	if (m_dwFlags != rCompare.m_dwFlags)
		return true;

	// Compare shared instance

	if (m_pSharedInstance != rCompare.m_pSharedInstance)
		return true;

	// Compare data

	if (memcmp(&m_rcTextureCoords,
		&rCompare.m_rcTextureCoords, sizeof(RECT)) != 0)
			return true;

	return false;
}

void MaterialInstance::UpdateSharedInstance(MaterialInstanceShared& rDirtyCopy)
{
	MaterialInstanceShared* pReference = rDirtyCopy.GetManager().Add(rDirtyCopy);
	pReference->AddRef();

	if (m_pSharedInstance != NULL)
		m_pSharedInstance->Release();

	m_pSharedInstance = pReference;
}

/*----------------------------------------------------------*\
| MaterialInstancePool implementation
\*----------------------------------------------------------*/

MaterialInstanceShared* MaterialInstancePool::Add(MaterialInstanceShared& rInstance)
{
	MaterialInstanceSharedMapIterator posFind = Find(rInstance);

	if (posFind != m_mapInstances.end())
		return &posFind->second;

	// If not found, add one

	MaterialInstanceSharedMapIterator posInsert =
		m_mapInstances.insert(std::pair<Material*,
		MaterialInstanceShared>(rInstance.GetMaterial(), rInstance));

	return &(posInsert->second);
}

MaterialInstanceSharedMapIterator MaterialInstancePool::Find(MaterialInstanceShared& rFind)
{
	MaterialInstanceSharedMapRange range =
		m_mapInstances.equal_range(rFind.GetMaterial());

	for(MaterialInstanceSharedMapIterator pos = range.first;
		pos != range.second;
		pos++)
	{
		if (pos->second == rFind)
			return pos;
	}

	return m_mapInstances.end();
}

void MaterialInstancePool::Remove(MaterialInstanceShared* pInstance)
{
	MaterialInstanceSharedMapIterator posFind = Find(*pInstance);

	if (posFind != m_mapInstances.end())
		m_mapInstances.erase(posFind);
}

int MaterialInstancePool::GetCount(void) const
{
	return int(m_mapInstances.size());
}

void MaterialInstancePool::OnLostDevice(bool bRecreate)
{
	for(MaterialInstanceSharedMapIterator pos = m_mapInstances.begin();
		pos != m_mapInstances.end();
		pos++)
	{
		pos->second.OnLostDevice(bRecreate);
	}
}

void MaterialInstancePool::OnResetDevice(bool bRecreate)
{
	for(MaterialInstanceSharedMapIterator pos = m_mapInstances.begin();
		pos != m_mapInstances.end();
		pos++)
	{
		pos->second.OnResetDevice(bRecreate);
	}
}

void MaterialInstancePool::Empty(void)
{
	m_mapInstances.clear();
}