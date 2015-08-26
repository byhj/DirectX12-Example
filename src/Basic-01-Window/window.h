#ifndef WINDOW_H
#define WINDOW_H

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <dxgi1_4.h>
#include <windows.h>

using namespace DirectX;
using namespace Microsoft::WRL;

namespace byhj
{

class Window
{
public:

	void Init();
	void Update();
	void Render();
	void Shutdown();

private:

	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	static const UINT FrameCount = 2;

	ComPtr<IDXGISwapChain3>           m_pSwapChain                ;
	ComPtr<ID3D12Device>              m_pD3D12Device              ;
	ComPtr<ID3D12Fence>               m_pD3D12Fence               ;
	ComPtr<ID3D12Resource>            m_pRenderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator         ;
	ComPtr<ID3D12CommandQueue>        m_pCommandQueue             ;
	ComPtr<ID3D12RootSignature>       m_pRootSignature            ;
	ComPtr<ID3D12DescriptorHeap>      m_pRTVHeap                  ;
	ComPtr<ID3D12PipelineState>       m_pPipelineState            ;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList              ;
	ComPtr<ID3D12Resource>            m_pVertexBuffer             ;

	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	UINT m_RTVDescriptorSize;
	UINT m_FrameIndex;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue;

	void LoadAssets();
	void LoadPipeline();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};

}
#endif