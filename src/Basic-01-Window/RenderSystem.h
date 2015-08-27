#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "d3d/App.h"
#include "window.h"

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

	ComPtr<IDXGISwapChain3>           m_pSwapChain;
	ComPtr<ID3D12Device>              m_pD3D12Device;
	ComPtr<ID3D12Fence>               m_pD3D12Fence;
	ComPtr<ID3D12Resource>            m_pRenderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
	ComPtr<ID3D12DescriptorHeap>      m_pRTVHeap;
	ComPtr<ID3D12PipelineState>       m_pPipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

	UINT   m_RTVDescriptorSize;
	UINT   m_FrameIndex;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue;

	Window m_Window;
};


}

#endif