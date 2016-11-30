/*------------------------------------------------------------------*\
|
| ThunderStates.cpp
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm state class(es) implementation
| Created: 10/04/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "stdafx.h"				// precompiled header
#include "ThunderEngine.h"		// using Engine, Graphics, etc

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

using namespace ThunderStorm;

/*----------------------------------------------------------*\
| StateManager implementation
\*----------------------------------------------------------*/

StateManager::StateManager(Graphics &rGraphics):
						   m_rGraphics(rGraphics),
						   m_pRecStates(NULL),
						   m_nStateChanges(0),
						   m_nLastStateChanges(0),
						   m_nRefs(1),
						   m_dwFrameWVPChanged(0),
						   m_dwFrameVPChanged(0),
						   m_DefaultStates(rGraphics)
{
	D3DXMatrixIdentity(&m_mtxWorld);
	D3DXMatrixIdentity(&m_mtxView);
	D3DXMatrixIdentity(&m_mtxProj);
	D3DXMatrixIdentity(&m_mtxViewProj);
	D3DXMatrixIdentity(&m_mtxWorldViewProj);
}

StateManager::~StateManager(void)
{
}

void StateManager::ResetStates(void)
{
	// Reset World Transform to default

	SetTransform(D3DTS_WORLD, NULL);

	// Reset View Transform to default

	SetTransform(D3DTS_VIEW, NULL);

	// Reset Projection Transform to default

	SetTransformOrthoProjection(
		float(m_rGraphics.m_D3DDeviceParams.BackBufferWidth),
		float(m_rGraphics.m_D3DDeviceParams.BackBufferHeight));

	// Record state block with default states

	m_DefaultStates.Begin();

	SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetRenderState(D3DRS_ZENABLE, FALSE);
	SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	SetRenderState(D3DRS_ALPHAREF, 0);
	SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRenderState(D3DRS_CLIPPING, TRUE);
	SetRenderState(D3DRS_CLIPPLANEENABLE, FALSE);
	SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	SetRenderState(D3DRS_FOGENABLE, FALSE);
	SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
	SetRenderState(D3DRS_STENCILENABLE, FALSE);
	SetRenderState(D3DRS_VERTEXBLEND, FALSE);
	SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	SetRenderState(D3DRS_LIGHTING, FALSE);
	SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	SetRenderState(D3DRS_WRAP0, 0);
	SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);

	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0);
	SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, 0);

	// Save state block with default states

	m_DefaultStates.End();

	m_DefaultStates.Apply();
}

int StateManager::GetFilteredStateChangeCount(void) const
{
	return 0;
}

void StateManager::SetTransformOrthoProjection(float fWidth, float fHeight)
{
	// Orthographic matrix for 2D rendering with 0,0 at top left,
	// adjusted for pixel perfect rendering (bias 0.5f)

	D3DXMATRIX mtx;

	D3DXMatrixOrthoOffCenterLH(&mtx,
		0.5f,
		fWidth + 0.5f,
		fHeight + 0.5f,
		0.5f,
		0.0f, 1000.0f);

	SetTransform(D3DTS_PROJECTION, &mtx);
}

void StateManager::BeginScene(void)
{
	m_nLastStateChanges = m_nStateChanges;
	m_nStateChanges = 0;
}

HRESULT CALLBACK StateManager::SetFVF(DWORD dwFVF)
{
	HRESULT hr = m_rGraphics.m_pD3DDevice->SetFVF(dwFVF);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETFVF,
			__FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManager::SetRenderState(D3DRENDERSTATETYPE dwState,
											  DWORD dwValue)
{
	_ASSERT(dwState >= 0 && dwState <= D3DRS_BLENDOPALPHA);

	HRESULT hr = m_rGraphics.m_pD3DDevice->SetRenderState(dwState, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETRENDERSTATE,
			__FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManager::SetTextureStageState(DWORD dwStage,
													D3DTEXTURESTAGESTATETYPE dwType,
													DWORD dwValue)
{
	_ASSERT(dwStage >= 0 && dwStage < MANAGE_STAGES);
	_ASSERT(dwType >= 0 && dwType <= D3DTSS_CONSTANT);

	HRESULT hr =
		m_rGraphics.m_pD3DDevice->SetTextureStageState(dwStage, dwType, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(
			Error::D3D_DEVICE_SETTEXTURESTAGESTATE, __FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManager::SetSamplerState(DWORD dwSampler,
											   D3DSAMPLERSTATETYPE dwType,
											   DWORD dwValue)
{
	_ASSERT(dwSampler >= 0 && dwSampler < MANAGE_STAGES);
	_ASSERT(dwType >= 0 && dwType <= D3DSAMP_DMAPOFFSET);

	HRESULT hr =
		m_rGraphics.m_pD3DDevice->SetSamplerState(dwSampler, dwType, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(
			Error::D3D_DEVICE_SETSAMPLERSTATE, __FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManager::SetTexture(DWORD dwStage,
										  LPDIRECT3DBASETEXTURE9 pTexture)
{
	_ASSERT(dwStage >= 0 && dwStage < MANAGE_STAGES);

	HRESULT hr =
		m_rGraphics.m_pD3DDevice->SetTexture(dwStage, pTexture);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETTEXTURE,
		__FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

const D3DMATRIX& StateManager::GetTransform(D3DTRANSFORMSTATETYPE dwType)
{
	switch(DWORD(dwType))
	{
	case D3DTS_WORLD:
		return m_mtxWorld;
		break;
	case D3DTS_VIEW:
		return m_mtxView;
		break;
	case D3DTS_PROJECTION:
		return m_mtxProj;
		break;
	}

	return m_mtxWorld;
}

const D3DMATRIX& StateManager::GetTransformCombination(TransformCombines nType)
{
	if (TRANSFORM_VIEWPROJ == nType)
		return m_mtxViewProj;
	else
		return m_mtxWorldViewProj;
}

HRESULT CALLBACK StateManager::SetTransform(D3DTRANSFORMSTATETYPE dwType,
											const D3DMATRIX* pMatrix)
{
	// Update cached transforms

	switch(DWORD(dwType))
	{
	case D3DTS_WORLD:
		{
			if (NULL == pMatrix)
			{
				D3DXMatrixIdentity(&m_mtxWorld);
				pMatrix = &m_mtxWorld;
			}
			else
			{
				m_mtxWorld = *pMatrix;		
			}
		}
		break;
	case D3DTS_VIEW:
		{
			if (NULL == pMatrix)
			{
				D3DXMatrixIdentity(&m_mtxView);
				pMatrix = &m_mtxView;
			}
			else
			{
				m_mtxView = *pMatrix;
			}

			D3DXMatrixMultiply(&m_mtxViewProj, &m_mtxView, &m_mtxProj);

			m_dwFrameVPChanged = m_rGraphics.GetFrameID();
		}
		break;
	case D3DTS_PROJECTION:
		{
			if (NULL == pMatrix)
			{
				D3DXMatrixIdentity(&m_mtxProj);
				pMatrix = &m_mtxProj;
			}
			else
			{
				m_mtxProj = *pMatrix;
			}
			
			D3DXMatrixMultiply(&m_mtxViewProj, &m_mtxView, &m_mtxProj);

			m_dwFrameVPChanged = m_rGraphics.GetFrameID();
		}
		break;
	}

	// Recalculate wvp matrix

	D3DXMatrixMultiply(&m_mtxWorldViewProj, &m_mtxWorld, &m_mtxViewProj);

	m_dwFrameWVPChanged = m_rGraphics.GetFrameID();

	return 0;
}

HRESULT CALLBACK StateManager::LightEnable(DWORD dwIndex, BOOL bEnable)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->LightEnable(dwIndex, bEnable);
}

HRESULT CALLBACK StateManager::SetLight(DWORD dwIndex,
										const D3DLIGHT9* pLight)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetLight(dwIndex, pLight);
}

HRESULT CALLBACK StateManager::SetMaterial(const D3DMATERIAL9* pMaterial)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetMaterial(pMaterial);
}

HRESULT CALLBACK StateManager::SetNPatchMode(float fSegments)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetNPatchMode(fSegments);
}

HRESULT CALLBACK StateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetPixelShader(pShader);
}

HRESULT CALLBACK StateManager::SetPixelShaderConstantB(UINT nStartRegister,
													   const BOOL* pConstantData,
													   UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetPixelShaderConstantB(nStartRegister,
		pConstantData,
		nRegisterCount);
}

HRESULT CALLBACK StateManager::SetPixelShaderConstantF(UINT nStartRegister,
													   const float* pConstantData,
													   UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetPixelShaderConstantF(nStartRegister,
		pConstantData,
		nRegisterCount);
}

HRESULT CALLBACK StateManager::SetPixelShaderConstantI(UINT nStartRegister,
													   const int* pConstantData,
													   UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetPixelShaderConstantI(nStartRegister,
		pConstantData,
		nRegisterCount);
}

HRESULT CALLBACK StateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetVertexShader(pShader);
}

HRESULT CALLBACK StateManager::SetVertexShaderConstantB(UINT nStartRegister,
														const BOOL* pConstantData,
														UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetVertexShaderConstantB(nStartRegister,
		pConstantData,
		nRegisterCount);
}

HRESULT CALLBACK StateManager::SetVertexShaderConstantF(UINT nStartRegister,
														const float* pConstantData,
														UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetVertexShaderConstantF(nStartRegister,
		pConstantData,
		nRegisterCount);
}

HRESULT CALLBACK StateManager::SetVertexShaderConstantI(UINT nStartRegister,
														const int* pConstantData,
														UINT nRegisterCount)
{
	m_nStateChanges++;

	return m_rGraphics.m_pD3DDevice->SetVertexShaderConstantI(nStartRegister,
															  pConstantData,
															  nRegisterCount);
}

HRESULT CALLBACK StateManager::QueryInterface(REFIID iid, LPVOID* ppv)
{
	if (IID_IUnknown == iid || IID_ID3DXEffectStateManager == iid)
	{
		*ppv = static_cast<ID3DXEffectStateManager*>(this);
	}
	else
	{
		*ppv = NULL;

		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(this)->AddRef();

	return S_OK;
}

ULONG CALLBACK StateManager::AddRef(void)
{
	return ++m_nRefs;
}

ULONG CALLBACK StateManager::Release(void)
{
	if (--m_nRefs <= 0)
	{
		delete this;

		return 0;
	}

	return m_nRefs;
}

/*----------------------------------------------------------*\
| StateManagerPure implementation
\*----------------------------------------------------------*/

StateManagerPure::StateManagerPure(Graphics &rGraphics):
								   StateManager(rGraphics),
								   m_nFilteredStateChanges(0)
{
}

StateManagerPure::~StateManagerPure(void)
{
}

int StateManagerPure::GetFilteredStateChangeCount(void) const
{
	return m_nLastFiltered;
}

void StateManagerPure::BeginScene(void)
{
	StateManager::BeginScene();

	m_nLastFiltered = m_nFilteredStateChanges;
	m_nFilteredStateChanges = 0;
}

void StateManagerPure::ResetStates(void)
{
	m_mapRenderStates.clear();

	for(int n = 0; n < MANAGE_STAGES; n++)
	{
		m_armapSamplerStates[n].clear();
		m_armapTextureStageStates[n].clear();
	}

	StateManager::ResetStates();
}

HRESULT CALLBACK StateManagerPure::SetRenderState(D3DRENDERSTATETYPE dwState,
												  DWORD dwValue)
{
	_ASSERT(dwState >= 0 && dwState <= D3DRS_BLENDOPALPHA);

	// Record change in state block

	if (m_pRecStates != NULL)
	{
		m_pRecStates->CacheRenderState(dwState, dwValue);
		return S_OK;
	}

	// Process change

	RenderStateMapIterator posFind = m_mapRenderStates.find(dwState);

	if (m_mapRenderStates.end() == posFind || posFind->second != dwValue)
	{
		m_mapRenderStates[dwState] = dwValue;
	}
	else
	{
		m_nFilteredStateChanges++;

		return S_OK;
	}

	HRESULT hr = m_rGraphics.m_pD3DDevice->SetRenderState(dwState, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(Error::D3D_DEVICE_SETRENDERSTATE,
			__FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManagerPure::SetTextureStageState(DWORD dwStage,
	D3DTEXTURESTAGESTATETYPE dwType,
	DWORD dwValue)
{
	_ASSERT(dwStage >= 0 && dwStage < MANAGE_STAGES);
	_ASSERT(dwType >= 0 && dwType <= D3DTSS_CONSTANT);

	// Record change in the state block

	if (m_pRecStates != NULL)
	{
		m_pRecStates->CacheTextureStageState(dwStage, dwType, dwValue);
		return S_OK;
	}

	// Process change

	TextureStageStateMapIterator posFind =
		m_armapTextureStageStates[dwStage].find(dwType);

	if (m_armapTextureStageStates[dwStage].end() == posFind ||
		posFind->second != dwValue)
	{
		m_armapTextureStageStates[dwStage][dwType] = dwValue;
	}
	else
	{
		m_nFilteredStateChanges++;

		return S_OK;
	}

	HRESULT hr = m_rGraphics.m_pD3DDevice->SetTextureStageState(dwStage,
		dwType, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(
			Error::D3D_DEVICE_SETTEXTURESTAGESTATE, __FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

HRESULT CALLBACK StateManagerPure::SetSamplerState(DWORD dwSampler,
												   D3DSAMPLERSTATETYPE dwType,
												   DWORD dwValue)
{
	_ASSERT(dwSampler >= 0 && dwSampler < MANAGE_STAGES);
	_ASSERT(dwType >= 0 && dwType <= D3DSAMP_DMAPOFFSET);

	// Record change in the state block

	if (m_pRecStates != NULL)
	{
		m_pRecStates->CacheSamplerState(dwSampler, dwType, dwValue);
		return S_OK;
	}

	// Process change

	SamplerStateMapIterator posFind =
		m_armapSamplerStates[dwSampler].find(dwType);

	if (m_armapSamplerStates[dwSampler].end() == posFind ||
		posFind->second != dwValue)
	{
		m_armapSamplerStates[dwSampler][dwType] = dwValue;
	}
	else
	{
		m_nFilteredStateChanges++;

		return S_OK;
	}

	HRESULT hr = m_rGraphics.m_pD3DDevice->SetSamplerState(dwSampler,
		dwType, dwValue);

	if (FAILED(hr))
		throw m_rGraphics.m_rEngine.GetErrors().Push(
			Error::D3D_DEVICE_SETSAMPLERSTATE, __FUNCTIONW__, hr);

	m_nStateChanges++;

	return hr;
}

/*----------------------------------------------------------*\
| StateBlock implementation
\*----------------------------------------------------------*/

StateBlock::StateBlock(Graphics& rGraphics): m_rGraphics(rGraphics),
											 m_pD3DState(NULL)
{
}

StateBlock::~StateBlock(void)
{
	Empty();
}

void StateBlock::Capture(D3DSTATEBLOCKTYPE nCaptureType)
{
	Empty();	

	if (m_rGraphics.GetDeviceFlags() & D3DCREATE_PUREDEVICE)
	{
		CaptureCachedStates();
	}
	else
	{
		HRESULT hr = m_rGraphics.GetDevice()->CreateStateBlock(
			nCaptureType, &m_pD3DState);

		if (FAILED(hr))
			throw m_rGraphics.GetEngine().GetErrors().Push(
				Error::D3D_DEVICE_CREATESTATEBLOCK,
				__FUNCTIONW__, hr);
	}
}

void StateBlock::Capture(void)
{
	if (m_rGraphics.GetDeviceFlags() & D3DCREATE_PUREDEVICE)
	{
		StateManagerPure* pManager =
			dynamic_cast<StateManagerPure*>(m_rGraphics.GetStates());

		if (NULL == pManager)
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::INVALID_CALL,
				__FUNCTIONW__);

		for(StateChangeArrayIterator pos = m_arState.begin();
			pos != m_arState.end();
			pos++)
		{
			switch(pos->nType)
			{
			case StateChange::STATE_RENDER:
				{
					RenderStateMapIterator posFind =
						pManager->m_mapRenderStates.find(
							D3DRENDERSTATETYPE(pos->dwState));

					if (posFind != pManager->m_mapRenderStates.end())
						pos->dwValue = posFind->second;
				}
				break;
			case StateChange::STATE_SAMPLER:
				{
					SamplerStateMapIterator posFind =
						pManager->m_armapSamplerStates[pos->dwIndex].find(
							D3DSAMPLERSTATETYPE(pos->dwState));

					if (posFind != pManager->m_armapSamplerStates[pos->dwIndex].end())
						pos->dwValue = posFind->second;
				}
				break;
			case StateChange::STATE_TEXTURESTAGE:
				{
					TextureStageStateMapIterator posFind =
						pManager->m_armapTextureStageStates[pos->dwIndex].find(
							D3DTEXTURESTAGESTATETYPE(pos->dwState));

					if (posFind != pManager->m_armapTextureStageStates[pos->dwIndex].end())
						pos->dwValue = posFind->second;
				}
				break;
			}
		}
	}
	else
	{
		if (NULL == m_pD3DState)
			return;

		HRESULT hr = m_pD3DState->Capture();

		if (FAILED(hr))
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::D3D_STATEBLOCK_CAPTURE,
				__FUNCTIONW__, hr);
	}
}

void StateBlock::Begin(void)
{
	Empty();

	if (m_rGraphics.GetStates() != NULL)
		m_rGraphics.GetStates()->m_pRecStates = this;

	if (~m_rGraphics.GetDeviceFlags() & D3DCREATE_PUREDEVICE)
	{
		HRESULT hr = m_rGraphics.GetDevice()->BeginStateBlock();

		if (FAILED(hr))
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::D3D_DEVICE_BEGINSTATEBLOCK,
				__FUNCTIONW__, hr);
	}
}

void StateBlock::End(void)
{
	if (m_rGraphics.GetStates()->m_pRecStates != this)
		return;

	m_rGraphics.GetStates()->m_pRecStates = NULL;

	if (m_rGraphics.GetDeviceFlags() & D3DCREATE_PUREDEVICE)
	{
		// Sort and remove duplicates

		std::sort(m_arState.begin(), m_arState.end());

		m_arState.erase(
			std::unique(m_arState.begin(), m_arState.end()),
			m_arState.end());
	}
	else
	{
		HRESULT hr = m_rGraphics.GetDevice()->EndStateBlock(&m_pD3DState);

		if (FAILED(hr))
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::D3D_DEVICE_ENDSTATEBLOCK,
				__FUNCTIONW__, hr);
	}
}

void StateBlock::Apply(void)
{
	if (m_rGraphics.GetDeviceFlags() & D3DCREATE_PUREDEVICE)
	{
		ApplyCachedStates();
	}
	else
	{
		if (NULL == m_pD3DState)
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::INVALID_CALL,
				__FUNCTIONW__);

		HRESULT hr = m_pD3DState->Apply();

		if (FAILED(hr))
			throw m_rGraphics.GetEngine().GetErrors().Push(Error::D3D_STATEBLOCK_APPLY,
				__FUNCTIONW__, hr);
	}
}

void StateBlock::Empty(void)
{
	SAFERELEASE(m_pD3DState);

	if (m_arState.empty() == false)
		m_arState.clear();
}

void StateBlock::CaptureCachedStates(void)
{
	// If operating on a pure device, capture cached states

	StateManagerPure* pManager =
		dynamic_cast<StateManagerPure*>(m_rGraphics.GetStates());

	if (NULL == pManager)
		throw m_rGraphics.GetEngine().GetErrors().Push(
			Error::INVALID_CALL, __FUNCTIONW__);

	// Capture render states

	for(RenderStateMapIterator pos =
		pManager->m_mapRenderStates.begin();
		pos != pManager->m_mapRenderStates.end();
		pos++)
	{
		m_arState.push_back(
			StateChange(StateChange::STATE_RENDER, 0,
				pos->first, pos->second));
	}

		for(UINT n = 0; n < StateManager::MANAGE_STAGES; n++)
	{
		// Capture texture stage states

		for(TextureStageStateMapIterator pos =
			pManager->m_armapTextureStageStates[n].begin();
			pos != pManager->m_armapTextureStageStates[n].end();
			pos++)
		{
			m_arState.push_back(
				StateChange(StateChange::STATE_TEXTURESTAGE, n,
					pos->first, pos->second));
		}

		// Capture sampler states

		for(SamplerStateMapIterator pos =
			pManager->m_armapSamplerStates[n].begin();
			pos != pManager->m_armapSamplerStates[n].end();
			pos++)
		{
			m_arState.push_back(
				StateChange(StateChange::STATE_SAMPLER, n,
					pos->first, pos->second));
		}
	}
}

void StateBlock::ApplyCachedStates(void)
{
	// If operating under pure device, apply cached states

	StateManagerPure* pManager =
		dynamic_cast<StateManagerPure*>(m_rGraphics.GetStates());

	if (NULL == pManager)
		throw m_rGraphics.GetEngine().GetErrors().Push(Error::INVALID_CALL,
			__FUNCTIONW__);

	// Apply all cached states

	for(std::vector<StateChange>::iterator pos = m_arState.begin();
		pos != m_arState.end();
		pos++)
	{
		switch(pos->nType)
		{
		case StateChange::STATE_RENDER:
			pManager->SetRenderState(
				D3DRENDERSTATETYPE(pos->dwState), pos->dwValue);
			break;
		case StateChange::STATE_SAMPLER:
			pManager->SetSamplerState(
				pos->dwIndex, D3DSAMPLERSTATETYPE(pos->dwState), pos->dwValue);
			break;
		case StateChange::STATE_TEXTURESTAGE:
			pManager->SetTextureStageState(
				pos->dwIndex, D3DTEXTURESTAGESTATETYPE(pos->dwState), pos->dwValue);
			break;
		}
	}
}