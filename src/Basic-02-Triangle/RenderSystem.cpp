#include "RenderSystem.h"

#include "d3d/Utility.h"
#include "d3dx12.h"
#include <d3dcompiler.h>

namespace byhj
{

void RenderSysem::v_Init()
{
	m_Viewport.Width = static_cast<float>(m_ScreenWidth);
	m_Viewport.Height = static_cast<float>(m_ScreenHeight);
	m_Viewport.MaxDepth = 1.0f;

	m_ScissorRect.right = static_cast<LONG>(m_ScreenWidth);
	m_ScissorRect.bottom = static_cast<LONG>(m_ScreenHeight);

	LoadPipeline();
	LoadAssets();
	m_Triangle.init_buffer(m_pD3D12Device);
	m_Triangle.init_shader(m_pD3D12Device);
}

void RenderSysem::v_Update()
{

}

void RenderSysem::v_Render()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList *ppCommandLists[] ={ m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_pSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void RenderSysem::v_Shutdown()
{


	WaitForPreviousFrame();

	//ClouseHandle(m_FenceEvent);
}

void RenderSysem::LoadPipeline()
{

#ifdef _DEBUG
	//Enable the D3D12 debug layer
	ComPtr<ID3D12Debug> pDebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
	{
		pDebugController->EnableDebugLayer();
	}
#endif

	ComPtr<IDXGIFactory4> pFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)));

	bool UseWarpDevice = false;

	if (UseWarpDevice)
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pD3D12Device)));
	}
	else
	{
		ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pD3D12Device)));
	}

///////////////////////	//Describe and Create the command queue//////////////////////////////////////////////

	D3D12_COMMAND_QUEUE_DESC queueDesc ={};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(m_pD3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));

	//Describe and Create the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc ={};
	swapChainDesc.BufferCount       = FrameCount;
	swapChainDesc.BufferDesc.Width  = m_ScreenWidth;
	swapChainDesc.BufferDesc.Height = m_ScreenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow      = GetHwnd();
	swapChainDesc.SampleDesc.Count  = 1;
	swapChainDesc.Windowed          = TRUE;

	ComPtr<IDXGISwapChain> pSwapChain;
	ThrowIfFailed(pFactory->CreateSwapChain(
		m_pCommandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		&swapChainDesc,
		&pSwapChain
		)
		);

	ThrowIfFailed(pSwapChain.As(&m_pSwapChain));
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

///////////////////////// Create Decriptor heaps /////////////////////////////////////////////


	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc ={};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ThrowIfFailed(m_pD3D12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVHeap)));

		m_RTVDescriptorSize = m_pD3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}

	//Create frame resources
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n])));
			m_pD3D12Device->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_RTVDescriptorSize);
		}
	}
	ThrowIfFailed(m_pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));

}


void RenderSysem::LoadAssets()
{
	// Create the command list.
	ThrowIfFailed(m_pD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_pCommandList->Close());

	// Create synchronization objects.
	{
		ThrowIfFailed(m_pD3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pD3D12Fence)));
		m_FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (m_FenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}


void RenderSysem::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_pCommandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	auto pPipelineState = m_Triangle.GetPipelineState();
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocator.Get(), pPipelineState.Get()));
	auto pRootSignature = m_Triangle.GetRootSignature();
	m_pCommandList->SetGraphicsRootSignature(pRootSignature.Get());

	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
	m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] ={ 0.0f, 0.2f, 0.4f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_Triangle.Render(m_pCommandList);

	//pCIndicate that the back buffer will now be used to present.
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_pCommandList->Close());


}


void  RenderSysem::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_FenceValue;
	ThrowIfFailed(m_pCommandQueue->Signal(m_pD3D12Fence.Get(), fence));
	m_FenceValue++;

	// Wait until the previous frame is finished.
	if (m_pD3D12Fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_pD3D12Fence->SetEventOnCompletion(fence, m_FenceEvent));
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

}