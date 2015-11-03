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

	m_Triangle.init_buffer(m_pD3D12Device);
	m_Triangle.init_shader(m_pD3D12Device);

	LoadAssets();

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

	RenderUI();

	// Present the frame.
	ThrowIfFailed(m_pSwapChain->Present(0, 0));

	MoveToNextFrame();

}

void RenderSysem::v_Shutdown()
{


	WaitForGpu();

   CloseHandle(m_FenceEvent);
}

void RenderSysem::LoadPipeline()
{

	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions ={};
#ifdef _DEBUG
	//Enagle the D2D debug layer
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	// Enable the D3D11 debug layer.
	d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

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

// Create an 11 deivce wrapped around the 12 device and share
//12's command queue
	ComPtr<ID3D11Device> d3d11Device;
	ThrowIfFailed(D3D11On12CreateDevice(
		m_pD3D12Device.Get(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_pCommandQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&m_pD3D11DeviceContext,
		nullptr
		));

	//Query the 11on12 devie from the d3vice
	ThrowIfFailed(d3d11Device.As(&m_pD3D11On12Device));


	//Create D2D/Dwrite compoents;
	{
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_pD2D1Factory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_pD3D11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_pD2D1Factory->CreateDevice(dxgiDevice.Get(), &m_pD2DD1evice));
		ThrowIfFailed(m_pD2DD1evice->CreateDeviceContext(deviceOptions, &m_pD2D1DeviceContext) );
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pDWriteFactory));

	}
	//Query the desktop's dpi settings, which will be used to create
	//D2D's render targets;
	float dipX;
	float dipY;
	m_pD2D1Factory->GetDesktopDpi(&dipX, &dipY);
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dipX,
		dipY
		);
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

		// Create a RTV, D2D render target, and a command allocator for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n])));
			m_pD3D12Device->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
			
			// Create a wrapped 11On12 resource of this back buffer. Since we are 
			// rendering all D3D12 content first and then all D2D content, we specify 
			// the In resource state as RENDER_TARGET - because D3D12 will have last 
			// used it in this state - and the Out resource state as PRESENT. When 
			// ReleaseWrappedResources() is called on the 11On12 device, the resource 
			// will be transitioned to the PRESENT state.

			D3D11_RESOURCE_FLAGS d3d11Flags ={ D3D11_BIND_RENDER_TARGET };
			ThrowIfFailed(m_pD3D11On12Device->CreateWrappedResource(
				m_pRenderTargets[n].Get(),
				&d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT,
				IID_PPV_ARGS(&m_pWrappedBackBuffers[n])
				));

			//Create a render target for D2D to draw directly to this back buffer
			ComPtr<IDXGISurface> surface;
			ThrowIfFailed(m_pWrappedBackBuffers[n].As(&surface));
			ThrowIfFailed(m_pD2D1DeviceContext->CreateBitmapFromDxgiSurface(
				surface.Get(),
				&bitmapProperties,
				&m_pD2D1RenderTargets[n]
				));

			rtvHandle.Offset(1, m_RTVDescriptorSize);

			ThrowIfFailed(m_pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocators[n])));

		}
	}

}


void RenderSysem::LoadAssets()
{

/////////////////////Create D2D/DWrite objects for rendering text/////////////////////////
	{
		ThrowIfFailed(m_pD2D1DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pTextBrush));
		ThrowIfFailed(m_pDWriteFactory->CreateTextFormat(
			L"Verdana",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			50,
			L"en-us",
			&m_pTextFormat
			));
		ThrowIfFailed(m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	ThrowIfFailed(m_pD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocators[m_FrameIndex].Get(), m_Triangle.GetPipelineState().Get(), IID_PPV_ARGS(&m_pCommandList)));
	
//	UpdateSubresources<1>(m_pCommandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);
//	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));


	// Close the command list and execute it to begin the vertex buffer copy into the default heap/
	ThrowIfFailed(m_pCommandList->Close());
	ID3D12CommandList* ppCommandLists[] ={ m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait unitl assets have been uploaded to the GPU
	{
		ThrowIfFailed(m_pD3D12Device->CreateFence(m_FenceValues[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pD3D12Fence.GetAddressOf())));
		m_FenceValues[m_FrameIndex]++;

		// Create an event handle to use for frame synchronization.
		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (m_FenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForGpu();
	}
}


void RenderSysem::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_pCommandAllocators[m_FrameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	auto pPipelineState = m_Triangle.GetPipelineState();
	ThrowIfFailed(m_pCommandList->Reset(m_pCommandAllocators[m_FrameIndex].Get(), pPipelineState.Get()));
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
	//m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	
	// Note: do not transition the render target to present here.
	// the transition will occur when the wrapped 11On12 render
	// target resource is released.

	ThrowIfFailed(m_pCommandList->Close());


}


void  RenderSysem::WaitForGpu()
{

	//Schedule a Signal command in the queue.

	ThrowIfFailed(m_pCommandQueue->Signal(m_pD3D12Fence.Get(), m_FenceValues[m_FrameIndex]));

	// Wait until the fence was been processed
	ThrowIfFailed(m_pD3D12Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	//Increment the fence value for the current frame.
	++m_FenceValues[m_FrameIndex];
}

//Render text over D3D12 using D2D via the 11 on12 devie

void RenderSysem::RenderUI()
{
	D2D1_SIZE_F rtSize = m_pD2D1RenderTargets[m_FrameIndex]->GetSize();
	D2D1_RECT_F textRext = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"11On12";

	//Acquire our wrapped render target resource for the current back buffer.
	m_pD3D11On12Device->AcquireWrappedResources(m_pWrappedBackBuffers[m_FrameIndex].GetAddressOf(), 1);

	//Render Text directly to the back buffer
	m_pD2D1DeviceContext->SetTarget(m_pD2D1RenderTargets[m_FrameIndex].Get());
	m_pD2D1DeviceContext->BeginDraw();
	m_pD2D1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_pD2D1DeviceContext->DrawTextW(
		text,
		_countof(text) - 1,
		m_pTextFormat.Get(),
		&textRext,
		m_pTextBrush.Get()
		);
	ThrowIfFailed(m_pD2D1DeviceContext->EndDraw());

	//Release our wrapped render target resource.Releasing transitions the back buffer
	//resource to the state specified as the OutState when the wrapped resource was created
	m_pD3D11On12Device->ReleaseWrappedResources(m_pWrappedBackBuffers[m_FrameIndex].GetAddressOf(), 1);

	//Flush to submit the 11 command list to the shared command queue.
	m_pD3D11DeviceContext->Flush();

}

//Prepare to render the next frame
void RenderSysem::MoveToNextFrame()
{
	//Schedule a Signal command in the queue
	const UINT64 currentFenceValue = m_FenceValues[m_FrameIndex];
	ThrowIfFailed(m_pCommandQueue->Signal(m_pD3D12Fence.Get(), currentFenceValue));

	//Update the frame index
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	//If the next frame is not ready to be rendered yet, wait unit it is ready
	if (m_pD3D12Fence->GetCompletedValue() < m_FenceValues[m_FrameIndex])
	{
		ThrowIfFailed(m_pD3D12Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	}

	//Set the fence value for the next frame
	m_FenceValues[m_FrameIndex] = currentFenceValue + 1;
}


}