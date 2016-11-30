/*------------------------------------------------------------------*\
|
| ThunderGraphics.h
|
|-------------------------------------------------------------------
|
| Content: ThunderStorm rendering class(es)
| Created: 08/10/2008
|
|-------------------------------------------------------------------
| This software is licensed under GNU GPLv3 (see ..\license.htm)
\*------------------------------------------------------------------*/

#ifndef THUNDER_GRAPHICS_H
#define THUNDER_GRAPHICS_H

/*----------------------------------------------------------*\
| Includes
\*----------------------------------------------------------*/

#include "ThunderVertex.h"		// using Vertex and buffer classes
#include "ThunderStates.h"		// using state manager classes
#include "ThunderMaterial.h"	// using material classes

/*----------------------------------------------------------*\
| Namespace
\*----------------------------------------------------------*/

namespace ThunderStorm {

/*----------------------------------------------------------*\
| Declarations
\*----------------------------------------------------------*/

class Engine;					// referencing Engine
class Client;					// referencing Client
class Texture;					// referencing Texture
class Renderable;				// referencing Renderable, declared below
class StateManager;				// referencing StateManager, declared below

/*----------------------------------------------------------*\
| Definitions
\*----------------------------------------------------------*/

typedef std::vector<Renderable> RenderableArray;
typedef std::vector<Renderable>::iterator RenderableArrayIterator;
typedef std::vector<Renderable>::const_iterator RenderableArrayConstIterator;
typedef std::vector<Renderable>::reverse_iterator RenderableArrayReverseIterator;

typedef std::vector<D3DXMATRIX> MatrixArray;
typedef std::vector<D3DXMATRIX>::iterator MatrixArrayIterator;
typedef std::vector<D3DXMATRIX>::const_iterator MatrixArrayConstIterator;


/*----------------------------------------------------------*\
| Graphics class
\*----------------------------------------------------------*/

class Graphics
{
public:
	//
	// Constants
	//

	// Supported color modes

	enum DeviceFormats
	{
		// (Default) Same as current desktop format
		FORMAT_DESKTOP,

		// 32-bit color with unused alpha component
		FORMAT_X8R8G8B8,

		// 16-bit color with unused alpha component
		FORMAT_X1R5G5B5,

		// 16-bit color with 6 bits for green
		FORMAT_R5G6B5,

		// Number of device formats defined
		FORMAT_COUNT
	};

	// Reset types

	enum ResetTypes
	{
		// Reset the device if it reports lost status
		RESET_DEFAULT,

		// Reset the device (always)
		RESET_DEVICE,

		// Re-create the device
		RECREATE_DEVICE
	};

	// Device format mapping to Direct3D constants
	static const DWORD DW_DEVICE_FORMATS[];

	// Triangle vertex declaration (2D)
	static const D3DVERTEXELEMENT9 VD_TRI[];

	// Line vertex declaration (2D)
	static const D3DVERTEXELEMENT9 VD_LINE[];

	// Point vertex declaration (2D)
	static const D3DVERTEXELEMENT9 VD_POINT[];

	// Point Sprite vertex declaration (3D)
	static const D3DVERTEXELEMENT9 VD_PARTICLE[];

private:
	//
	// Members
	//

	// Keep reference to the engine
	Engine& m_rEngine;

	// Direct3D9 instance
	LPDIRECT3D9 m_pD3D;

	// Direct3DDevice9 instance
	LPDIRECT3DDEVICE9 m_pD3DDevice;

	// Device state manager
	StateManager* m_pStateManager;

	// Device type initialized with
	D3DDEVTYPE m_nD3DDeviceType;

	// Flags used to create device
	DWORD m_dwD3DDeviceFlags;

	// Default adapter caps
	D3DCAPS9 m_D3DDeviceCaps;

	// Current display mode
	D3DDISPLAYMODE m_D3DDeviceMode;

	// Parameters used to create device
	D3DPRESENT_PARAMETERS m_D3DDeviceParams;

	// Last set parameters (before resolution change, etc)
	D3DPRESENT_PARAMETERS m_LastD3DDeviceParams;

	// Scene state
	int m_nRendering;

	// Batching state
	int m_nBatching;

	// Clipping state
	bool m_bClipping;

	// Current frame index, referred to when needing to do things only once per frame
	DWORD m_dwFrame;

	// FPS time accumulator
	float m_fFpsTime;

	// FPS frame accumulator
	int m_nFpsFrames;

	// Current frame rate, frames per second (recalculated every second)
	int m_nFps;

	// Batches submitted while rendering current scene
	UINT m_uBatches;

	// Average number of primitives per batch
	UINT m_uMaxPrimsBatch;

	// Renderables submitted
	UINT m_uRenderables;

	// Triangles submitted
	UINT m_uTriangles;

	// Lines submitted
	UINT m_uLines;

	// Points submitted
	UINT m_uPoints;

	// Batches last frame
	UINT m_uLastBatches;

	// Prims per batch last frame
	UINT m_uLastMaxPrimsBatch;

	// Renderables last frame
	UINT m_uLastRenderables;

	// Triangles last frame
	UINT m_uLastTriangles;

	// Lines last frame
	UINT m_uLastLines;

	// Points last frame
	UINT m_uLastPoints;

	// Sort batches by material before submitting?
	bool m_bSortMaterial;

	// Sort batches by depth before submitting?
	bool m_bSortDepth;

	// Print render queue contents to debug channel on next frame
	bool m_bDebugCaptureQueue;

	// Vertex declaration for triangles
	LPDIRECT3DVERTEXDECLARATION9 m_pTriVD;

	// Vertex declaration for lines
	LPDIRECT3DVERTEXDECLARATION9 m_pLineVD;

	// Vertex declaration for points
	LPDIRECT3DVERTEXDECLARATION9 m_pPointVD;

	// Vertex declaration for point sprites
	LPDIRECT3DVERTEXDECLARATION9 m_pParticleVD;

	// Vertex cache for render queue
	VertexCache m_VC;

	// Vertex buffer for triangles
	VertexBuffer m_TriVB;

	// Vertex buffer for lines & points
	VertexBuffer m_LinePtVB;

	// Vertex buffer for particles
	VertexBuffer m_ParVB;

	// Index buffer for triangles (allocated and filled on init)
	IndexBuffer m_TriIB;

	// Index buffer for lines (allocated and filled on init)
	IndexBuffer m_LineIB;

	// Render queue for batches
	RenderableArray m_arRenderQueue;

	// Transforms used in render queue (so you can compare matrices by comparing their index)
	MatrixArray m_arTransforms;

	// Shared material instances
	MaterialInstancePool m_materialInstances;

	// Shared effect parameters
	EffectPool m_effectPool;

	// Material override when in wireframe mode (Engine::OPTION_WIREFRAME)
	MaterialInstance* m_pWireframeMaterial;

public:
	Graphics(Engine& rEngine);
	~Graphics(void);

public:
	//
	// Engine
	//

	inline Engine& GetEngine(void)
	{
		return m_rEngine;
	}

	//
	// Initialization
	//

	void Initialize(HWND hDeviceWindow,
					bool bFullScreen,
					int nResolutionWidth,
					int nResolutionHeight,
					DeviceFormats nFormat,
					D3DMULTISAMPLE_TYPE nMultiSampleType,
					DWORD dwMultiSampleQuality,
					DWORD dwRefreshRate,
					bool bVSync,
					bool bUseHardwareAcceleration,
					bool bUseSoftwareVertexProcessing,
					bool bPureDevice);

	inline bool IsInitialized(void) const
	{
		return (m_pD3DDevice != NULL);
	}

	//
	// Direct3D
	//

	inline LPDIRECT3D9 GetDirect3D(void)
	{
		return m_pD3D;
	}

	inline LPDIRECT3DDEVICE9 GetDevice(void)
	{
		return m_pD3DDevice;
	}

	inline StateManager* GetStates(void)
	{
		return m_pStateManager;
	}

	inline D3DDEVTYPE GetDeviceType(void) const
	{
		return m_nD3DDeviceType;
	}

	inline void SetDeviceType(D3DDEVTYPE nUpdateDevType)
	{
		m_nD3DDeviceType = nUpdateDevType;
	}

	inline DWORD GetDeviceFlags(void) const
	{
		return m_dwD3DDeviceFlags;
	}

	inline void SetDeviceFlags(DWORD dwFlags)
	{
		m_dwD3DDeviceFlags = dwFlags;
	}

	inline const D3DCAPS9& GetDeviceCaps(void) const
	{
		return m_D3DDeviceCaps;
	}

	inline void SetDeviceCaps(const D3DCAPS9& rUpdateDeviceCaps)
	{
		CopyMemory(&m_D3DDeviceCaps, &rUpdateDeviceCaps, sizeof(D3DCAPS9));
	}

	inline const D3DDISPLAYMODE& GetDeviceMode(void) const
	{
		return m_D3DDeviceMode;
	}

	inline void SetDeviceMode(const D3DDISPLAYMODE& rUpdateMode)
	{
		CopyMemory(&m_D3DDeviceMode, &rUpdateMode, sizeof(D3DDISPLAYMODE));
	}

	inline const D3DPRESENT_PARAMETERS& GetDeviceParams(void) const
	{
		return m_D3DDeviceParams;
	}

	inline const D3DPRESENT_PARAMETERS& GetLastDeviceParams(void) const
	{
		return m_LastD3DDeviceParams;
	}

	inline void SetDeviceParams(const D3DPRESENT_PARAMETERS& dp)
	{
		CopyMemory(&m_LastD3DDeviceParams, &m_D3DDeviceParams, sizeof(D3DPRESENT_PARAMETERS));
		CopyMemory(&m_D3DDeviceParams, &dp, sizeof(D3DPRESENT_PARAMETERS));
	}

	static DWORD GetAdapterMemory(void);

	//
	// Effect Pool
	//

	inline EffectPool& GetEffectPool(void)
	{
		return m_effectPool;
	}

	//
	// Material Pool
	//

	inline MaterialInstancePool& GetMaterialInstancePool(void)
	{
		return m_materialInstances;
	}

	//
	// Rendering (High Level)
	//

	void BeginScene(void);
	void EndScene(void);

	inline bool IsRendering(void) const
	{
		return (m_nRendering > 0);
	}

	void BeginClipping(const RECT& rcClip);
	void EndClipping(void);

	inline bool IsClipping(void) const
	{
		return m_bClipping;
	}

	void BeginBatch(bool bSortDepth = true, bool bSortMaterial = true);

	void EndBatch(void);
	void FlushBatch(void);

	inline bool IsBatching(void) const
	{
		return (m_nBatching > 0);
	}
	
	void Clear(D3DCOLOR clrClear);
	void Clear(D3DCOLOR clrClear, LPCRECT prcClear);

	void RenderQuad(const MaterialInstance& rMaterialInst,
		const Vector2& rvecPosition,
		D3DCOLOR clrBlend = Color::BLEND_ONE,
		float fZOrder = 0.0f,
		const Vector2* pvecPivot = NULL,
		const D3DXMATRIX* pmtxTransform = NULL);

	void RenderQuad(const MaterialInstance& rMaterialInst,
		const Vector2& rvecPosition,
		const Vector2& rvecSize,
		D3DCOLOR clrBlend = Color::BLEND_ONE);

	void RenderRectangle(const MaterialInstance& rMaterialInst,
		const Rect& rrc, D3DCOLOR clrBlend, float fZOrder = 0.0f,
		const Vector2* pvecPivot = NULL,
		const D3DXMATRIX* pmtxTransform = NULL);

	void RenderLine(const MaterialInstance& rMaterialInst,
		const Vector2& rvecStart, const Vector2& rvecEnd,
		D3DCOLOR clrBlend, float fZOrder = 0.0f);

	void RenderLines(const MaterialInstance& rMaterialInst,
		const VertexLine* pVertices, UINT uVertexCount, UINT uPrimCount,
		float fZOrder = 0.0f, const Vector2* pvecPivot = NULL,
		const D3DXMATRIX* pmtxTransform = NULL);

	void RenderPoints(const MaterialInstance& rMaterialInst,
		const VertexPoint* pPoints, UINT uCount, float fZOrder = 0.0f);

	void RenderParticles(const MaterialInstance& rMaterialInst,
		const VertexParticle* pParticles, UINT uCount, float fZOrder = 0.0f);

	void Present(void);

	void Reset(ResetTypes nReset = RESET_DEFAULT);

	MaterialInstanceShared* GetWireframeMaterial(void);

	inline void SetWireframeMaterial(MaterialInstance* pInstance)
	{
		m_pWireframeMaterial = pInstance;
	}

	void Screenshot(LPDIRECT3DSURFACE9* pOutSurface);
	void Screenshot(LPCWSTR pszPath, D3DXIMAGE_FILEFORMAT nFormat = D3DXIFF_BMP);

	//
	// Statistics
	//

	inline int GetFPS(void) const
	{
		return m_nFps;
	}

	inline DWORD GetFrameID(void) const
	{
		return m_dwFrame;
	}

	inline UINT GetBatchCount(void) const
	{
		return m_uLastBatches;
	}

	inline void UpdateBatchCount(UINT uAdd)
	{
		m_uBatches += uAdd;
	}

	inline UINT GetMaxPrimitivesPerBatch(void) const
	{
		return m_uLastMaxPrimsBatch;
	}

	inline void UpdateMaxPrimitivesPerBatch(UINT uCompare)
	{
		if (m_uMaxPrimsBatch < uCompare)
			m_uMaxPrimsBatch = uCompare;
	}

	inline UINT GetRenderableCount(void) const
	{
		return m_uLastRenderables;
	}

	inline UINT GetTriangleCount(void) const
	{
		return m_uLastTriangles;
	}

	inline void UpdateTriangleCount(UINT uAdd)
	{
		m_uTriangles += uAdd;
	}

	inline UINT GetLineCount(void) const
	{
		return m_uLastLines;
	}

	inline void UpdateLineCount(UINT uAdd)
	{
		m_uLines += uAdd;
	}

	inline UINT GetPointCount(void) const
	{
		return m_uLastPoints;
	}

	inline void UpdatePointCount(UINT uAdd)
	{
		m_uPoints += uAdd;
	}

	inline void DebugCaptureQueue(void)
	{
		m_bDebugCaptureQueue = true;
	}

	//
	// Rendering (Low Level)
	//

	inline LPDIRECT3DVERTEXDECLARATION9 GetTriangleVD(void)
	{
		return m_pTriVD;
	}

	inline LPDIRECT3DVERTEXDECLARATION9 GetLineVD(void)
	{
		return m_pLineVD;
	}

	inline LPDIRECT3DVERTEXDECLARATION9 GetPointVD(void)
	{
		return m_pPointVD;
	}

	inline LPDIRECT3DVERTEXDECLARATION9 GetParticleVD(void)
	{
		return m_pParticleVD;
	}

	inline VertexBuffer& GetTriangleVB(void)
	{
		return m_TriVB;
	}

	inline VertexBuffer& GetLinePointVB(void)
	{
		return m_LinePtVB;
	}

	inline VertexBuffer& GetParticleVB(void)
	{
		return m_ParVB;
	}

	inline IndexBuffer& GetTriangleIB(void)
	{
		return m_TriIB;
	}

	inline IndexBuffer& GetLineIB(void)
	{
		return m_LineIB;
	}

	inline RenderableArray& GetRenderQueue(void)
	{
		return m_arRenderQueue;
	}

	inline VertexCache& GetVertexCache(void)
	{
		return m_VC;
	}

	//
	// Deinitialization
	//

	void Empty(void);

public:
	//
	// Friends
	//

	friend class StateManager;
	friend class StateManagerPure;

private:
	//
	// Vertex and Index Cache
	//

	void InitializeCache(bool bVideoMemoryOnly);
	void EmptyCache(bool bVideoMemoryOnly);

	void EmptyRenderQueue(void);

	UINT AddTransform(const D3DXMATRIX& rTransform);
	void EmptyTransforms(void);
};

/*----------------------------------------------------------*\
| Renderable class
\*----------------------------------------------------------*/

class Renderable
{
public:
	enum Type
	{
		TYPE_POINTLIST = 1,
		TYPE_LINELIST = 2,
		TYPE_LINESTRIP = 3,
		TYPE_TRIANGLELIST = 4,
		TYPE_PARTICLELIST = 7
	};

public:
	// Type of this renderable (supported: trilist, linelist, points)
	Type nType;

	// Z stacking order
	float fZOrder;

	// Material to use
	MaterialInstanceShared* pMaterial;

	// Vertices
	LPBYTE pbVertices;

	// Vertex count
	UINT uVertexCount;

	// Primitive count (index count / indices per primitive)
	UINT uPrimitiveCount;

	// Transform used (from transform array)
	UINT nTransform;

public:
	static bool CompareDepth(const Renderable& r1,
		const Renderable& r2);
	
	static bool CompareMaterial(const Renderable& r1,
		const Renderable& r2);

	static bool CompareDepthMaterial(const Renderable& r1,
		const Renderable& r2);
};

} // namespace ThunderStorm

#endif // THUNDER_GRAPHICS_H