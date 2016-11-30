/*------------------------------------------------------------------*\
|
| ThunderStates.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm pipeline state management class(es)
| Created: 10/04/2010
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_STATES_H
#define THUNDER_STATES_H

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Graphics;					// referencing Graphics
class EffectParameterInfo;		// referencing EffectParameterInfo
class StateChange;				// referencing StateChange, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::map<D3DRENDERSTATETYPE, DWORD> RenderStateMap;
typedef std::map<D3DRENDERSTATETYPE, DWORD>::iterator RenderStateMapIterator;
typedef std::map<D3DRENDERSTATETYPE, DWORD>::const_iterator RenderStateMapConstIterator;

typedef std::map<D3DSAMPLERSTATETYPE, DWORD> SamplerStateMap;
typedef std::map<D3DSAMPLERSTATETYPE, DWORD>::iterator SamplerStateMapIterator;
typedef std::map<D3DSAMPLERSTATETYPE, DWORD>::const_iterator SamplerStateMapConstIterator;

typedef std::map<D3DTEXTURESTAGESTATETYPE, DWORD> TextureStageStateMap;
typedef std::map<D3DTEXTURESTAGESTATETYPE, DWORD>::iterator TextureStageStateMapIterator;
typedef std::map<D3DTEXTURESTAGESTATETYPE, DWORD>::const_iterator TextureStageStateMapConstIterator;

typedef std::vector<StateChange> StateChangeArray;
typedef std::vector<StateChange>::iterator StateChangeArrayIterator;
typedef std::vector<StateChange>::const_iterator StateChangeArrayConstIterator;


/*----------------------------------------------------------*\
| StateChange class
\*----------------------------------------------------------*/

class StateChange
{
public:
	//
	// Constants
	//

	enum StateType
	{
		STATE_RENDER,
		STATE_SAMPLER,
		STATE_TEXTURESTAGE
	};

public:
	//
	// Members
	//

	StateType nType;
	DWORD dwIndex;
	DWORD dwState;
	DWORD dwValue;

public:
	StateChange(void): nType(STATE_RENDER),
					   dwIndex(0),
					   dwState(0),
					   dwValue(0)
	{
	}

	StateChange(StateType nInitType,
				DWORD dwInitIndex,
				DWORD dwInitState,
				DWORD dwInitVal):
				nType(nInitType),
				dwIndex(dwInitIndex),
				dwState(dwInitState),
				dwValue(dwInitVal)
	{
	}

public:
	bool operator==(const StateChange& rCompare)
	{
		return (nType == rCompare.nType &&
				dwIndex == rCompare.dwIndex &&
				dwState == rCompare.dwState &&
				dwValue == rCompare.dwValue);
	}

	bool operator<(const StateChange& rCompare)
	{
		return (nType < rCompare.nType &&
				dwIndex < rCompare.dwIndex &&
				dwState < rCompare.dwState &&
				dwValue < rCompare.dwValue);
	}
};

/*----------------------------------------------------------*\
| StateBlock class
\*----------------------------------------------------------*/

class StateBlock
{
private:
	Graphics& m_rGraphics;

	LPDIRECT3DSTATEBLOCK9 m_pD3DState;

	StateChangeArray m_arState;

public:
	StateBlock(Graphics& rGraphics);
	~StateBlock(void);

public:
	void Capture(D3DSTATEBLOCKTYPE nCaptureType);
	void Capture(void);	

	void Begin(void);
	void End(void);

	void Apply(void);

	void Empty(void);

	//
	// Friends
	//

	friend class StateManagerPure;

private:
	//
	// Private Functions
	//

	inline void CacheRenderState(DWORD dwState, DWORD dwValue)
	{
		m_arState.push_back(
			StateChange(StateChange::STATE_RENDER, 0,
				dwState, dwValue));
	}

	inline void CacheSamplerState(DWORD dwSampler,
								  DWORD dwState,
								  DWORD dwValue)
	{
		m_arState.push_back(
			StateChange(StateChange::STATE_SAMPLER, dwSampler,
				dwState, dwValue));
	}

	inline void CacheTextureStageState(DWORD dwStage,
									   DWORD dwState,
									   DWORD dwValue)
	{
		m_arState.push_back(
			StateChange(StateChange::STATE_TEXTURESTAGE, dwStage,
				dwState, dwValue));
	}

	void CaptureCachedStates(void);
	void ApplyCachedStates(void);
};

/*----------------------------------------------------------*\
| StateManager class
\*----------------------------------------------------------*/

class StateManager: public ID3DXEffectStateManager
{
public:
	//
	// Constants
	//

	enum TransformCombines
	{
		TRANSFORM_VIEWPROJ,
		TRANSFORM_WORLDVIEWPROJ
	};

	static const UINT MANAGE_STAGES = 8;

protected:
	//
	// Members
	//

	// Keep reference to Graphics
	Graphics& m_rGraphics;

	// Currently recorded state block if any
	StateBlock* m_pRecStates;

	// State changes made since BeginScene
	int m_nStateChanges;

	// State changes made last frame
	int m_nLastStateChanges;

	// Reference tracking
	ULONG m_nRefs;

	// Frame ID when VP matrix last changed
	DWORD m_dwFrameVPChanged;

	// Frame ID when WVP matrix last changed
	DWORD m_dwFrameWVPChanged;

	// Current world transform
	D3DXMATRIX m_mtxWorld;

	// Current view transform
	D3DXMATRIX m_mtxView;

	// Current projection transform
	D3DXMATRIX m_mtxProj;

	// View-Projection transform combination
	D3DXMATRIX m_mtxViewProj;

	// World-View-Projection transform combination
	D3DXMATRIX m_mtxWorldViewProj;

	// Default states for rendering to revert to
	StateBlock m_DefaultStates;

public:
	StateManager(Graphics& rGraphics);
	virtual ~StateManager(void);

public:
	//
	// States
	//	

	inline int GetStateChangeCount(void) const
	{
		return m_nLastStateChanges;
	}

	virtual int GetFilteredStateChangeCount(void) const;

	virtual void BeginScene(void);
	virtual void ResetStates(void);
	
	inline void ApplyDefaultStates(void)
	{
		m_DefaultStates.Apply();
	}

	inline void OnLostDevice(bool bRecreate)
	{
		m_DefaultStates.Empty();
	}

	inline void OnResetDevice(bool bRecreate)
	{
		ResetStates();
	}

	inline DWORD GetLastWVPChangeFrame(void) const
	{
		return m_dwFrameWVPChanged;
	}

	inline DWORD GetLastVPChangeFrame(void) const
	{
		return m_dwFrameVPChanged;
	}

	const D3DMATRIX& GetTransform(D3DTRANSFORMSTATETYPE dwType = D3DTS_WORLD);

	const D3DMATRIX& GetTransformCombination(TransformCombines nType = TRANSFORM_WORLDVIEWPROJ);

	void SetTransformOrthoProjection(float fWidth, float fHeight);

	virtual HRESULT CALLBACK SetFVF(DWORD dwFVF);

	virtual HRESULT CALLBACK SetTransform(D3DTRANSFORMSTATETYPE dwType,
										  const D3DMATRIX* pMatrix);

	virtual HRESULT CALLBACK SetTexture(DWORD dwStage,
										LPDIRECT3DBASETEXTURE9 pTexture);

	virtual HRESULT CALLBACK SetRenderState(D3DRENDERSTATETYPE dwState,
											DWORD dwValue);

	virtual HRESULT CALLBACK SetTextureStageState(DWORD dwStage,
												  D3DTEXTURESTAGESTATETYPE dwType,
												  DWORD dwValue);

	virtual HRESULT CALLBACK SetSamplerState(DWORD dwSampler,
											 D3DSAMPLERSTATETYPE dwType,
											 DWORD dwValue);

	virtual HRESULT CALLBACK LightEnable(DWORD dwIndex, BOOL bEnable);

	virtual HRESULT CALLBACK SetLight(DWORD dwIndex, const D3DLIGHT9* pLight);

	virtual HRESULT CALLBACK SetMaterial(const D3DMATERIAL9* pMaterial);

	virtual HRESULT CALLBACK SetNPatchMode(float fSegments);

	virtual HRESULT CALLBACK SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader);

	virtual HRESULT CALLBACK SetPixelShaderConstantB(UINT nStartRegister,
													 const BOOL* pConstantData,
													 UINT nRegisterCount);

	virtual HRESULT CALLBACK SetPixelShaderConstantF(UINT nStartRegister,
													 const float* pConstantData,
													 UINT nRegisterCount);

	virtual HRESULT CALLBACK SetPixelShaderConstantI(UINT nStartRegister,
													 const int* pConstantData,
													 UINT nRegisterCount);

	virtual HRESULT CALLBACK SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader);

	virtual HRESULT CALLBACK SetVertexShaderConstantB(UINT nStartRegister,
													  const BOOL* pConstantData,
													  UINT nRegisterCount);

	virtual HRESULT CALLBACK SetVertexShaderConstantF(UINT nStartRegister,
													  const float* pConstantData,
													  UINT nRegisterCount);

	virtual HRESULT CALLBACK SetVertexShaderConstantI(UINT nStartRegister,
													  const int* pConstantData,
													  UINT nRegisterCount);

	//
	// Interface Management
	//

	virtual HRESULT CALLBACK QueryInterface(REFIID iid, LPVOID* ppv);
	virtual ULONG CALLBACK AddRef(void);
	virtual ULONG CALLBACK Release(void);

	//
	// Friends
	//

	friend class StateBlock;
};

/*----------------------------------------------------------*\
| StateManagerPure class - state management for pure device
\*----------------------------------------------------------*/

class StateManagerPure: public StateManager
{
private:
	// State changes that didn't have to be made
	int m_nFilteredStateChanges;

	// Same as above, for last frame
	int m_nLastFiltered;

	// Render state cache
	RenderStateMap m_mapRenderStates;

	// Texture stage state cache
	TextureStageStateMap m_armapTextureStageStates[MANAGE_STAGES];

	// Sampler state cache
	SamplerStateMap m_armapSamplerStates[MANAGE_STAGES];			

public:
	StateManagerPure(Graphics& rGraphics);
	virtual ~StateManagerPure(void);

public:
	//
	// States
	//

	virtual int GetFilteredStateChangeCount(void) const;
	virtual void BeginScene(void);
	virtual void ResetStates(void);

	virtual HRESULT CALLBACK SetRenderState(D3DRENDERSTATETYPE dwState, DWORD dwValue);	
	virtual HRESULT CALLBACK SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwType, DWORD dwValue);
	virtual HRESULT CALLBACK SetSamplerState(DWORD dwSampler, D3DSAMPLERSTATETYPE dwType, DWORD dwValue);

	//
	// Friends
	//

	friend class StateBlock;
};

} // namespace ThunderStorm

#endif // THUNDER_STATES_H