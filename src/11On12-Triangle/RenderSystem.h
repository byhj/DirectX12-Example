#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "d3d/App.h"
#include "triangle.h"

#include <d2d1_3.h>
#include <dwrite.h>
#include <d3d11on12.h>
#include <string>
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
	void RenderUI();
	void PopulateCommandList();
	void WaitForGpu();
	void MoveToNextFrame();
	void LoadAssets();
	void LoadPipeline();

	static const UINT FrameCount = 3;
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	ComPtr<IDXGISwapChain3>           m_pSwapChain;
	ComPtr<ID3D11DeviceContext>       m_pD3D11DeviceContext;
	ComPtr<ID3D11On12Device>          m_pD3D11On12Device;
	ComPtr<ID3D12Device>              m_pD3D12Device;

	ComPtr<IDWriteFactory>            m_pDWriteFactory;
	ComPtr<ID2D1Factory3>             m_pD2D1Factory;
	ComPtr<ID2D1Device2>              m_pD2DD1evice;
	ComPtr<ID2D1DeviceContext2>       m_pD2D1DeviceContext;

	ComPtr<ID3D12Fence>               m_pD3D12Fence;
	ComPtr<ID3D12Resource>            m_pRenderTargets[FrameCount];
	ComPtr<ID3D11Resource>            m_pWrappedBackBuffers[FrameCount];
	ComPtr<ID2D1Bitmap1>              m_pD2D1RenderTargets[FrameCount];
	
	ComPtr<ID3D12CommandAllocator>    m_pCommandAllocator;
	ComPtr<ID3D12CommandQueue>        m_pCommandQueue;
	ComPtr<ID3D12DescriptorHeap>      m_pRTVHeap;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

	ComPtr<ID2D1SolidColorBrush> m_pTextBrush;
	ComPtr<IDWriteTextFormat>    m_pTextFormat;

	UINT   m_RTVDescriptorSize;
	UINT   m_FrameIndex;
	HANDLE m_FenceEvent;
	UINT64 m_FenceValues[FrameCount];

	Triangle m_Triangle;
};


}

#endif