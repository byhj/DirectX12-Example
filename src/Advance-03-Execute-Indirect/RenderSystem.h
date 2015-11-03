#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "d3d/App.h"
#include "triangle.h"

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <windows.h>

using namespace DirectX;
using namespace Microsoft::WRL;

namespace byhj
{

class RenderSysem : public byhj::d3d::App
{
public:

	void v_Init();
	void v_Update();
	void v_Render();
	void v_Shutdown();

private:

	void PopulateCommandList();
	void WaitForPreviousFrame();
	void LoadAssets();
	void LoadPipeline();

	static const UINT FrameCount = 3;
	static const UINT TriangleCount = 1024;
	static const UINT TriangleResourceCount = TriangleCount * FrameCount;	// Should match the value in compute.hlsl.
	static const UINT computeThreadBlockSize = 128;							// The x and y offsets used by the triangle vertices.
	static const float TrinagleHalfWidth;									// The z offset used by the triangle vertices.
	static const float TriangleDepth;										// The +/- x offset of the clipping planes in homogenous space [-1,1].
	static const float CullingCutoff;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	ComPtr<IDXGISwapChain3>           m_pSwapChain;
	ComPtr<ID3D12Device>              m_pD3D12Device;
	ComPtr<ID3D12Fence>               m_pD3D12Fence;
	ComPtr<ID3D12Resource>            m_pRenderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
	ComPtr<ID3D12DescriptorHeap>      m_pRTVHeap;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

	UINT   m_RTVDescriptorSize;
	UINT   m_FrameIndex;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue;

	Triangle m_Triangle;
};


}

#endif