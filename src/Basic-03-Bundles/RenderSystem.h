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

	static const UINT FrameCount = 2;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	ComPtr<IDXGISwapChain3>           m_pSwapChain;
	ComPtr<ID3D12Device>              m_pD3D12Device;
	ComPtr<ID3D12Fence>               m_pD3D12Fence;
	ComPtr<ID3D12Resource>            m_pRenderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;
	ComPtr<ID3D12CommandAllocator>    m_pBundleAllocator;
	ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
	ComPtr<ID3D12DescriptorHeap>      m_pRTVHeap;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12GraphicsCommandList> m_pBundleList;

	UINT   m_RTVDescriptorSize;
	UINT   m_FrameIndex;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue;

	Triangle m_Triangle;
};


}

#endif