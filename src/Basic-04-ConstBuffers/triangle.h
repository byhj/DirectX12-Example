#ifndef Triangle_H
#define Triangle_H

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

namespace byhj
{

class Triangle
{
public:

	void Init();
	void Update(const XMFLOAT4X4 &World, const XMFLOAT4X4 &View, const XMFLOAT4X4 &Proj);
	void Render(ComPtr<ID3D12GraphicsCommandList> pCommandList);
	void Shutdown();

	ComPtr<ID3D12PipelineState> GetPipelineState()
	{
		return m_pPipelineState;
	}
	ComPtr<ID3D12RootSignature> GetRootSignature()
	{
		return m_pRootSignature;
	}
	ComPtr<ID3D12DescriptorHeap> GetCBVHeap()
	{
		return m_pCBVHeap;
	}
	void init_buffer(ComPtr<ID3D12Device> pD3D12Device);
	void init_shader(ComPtr<ID3D12Device> pD3D12Device);
private:


	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct OffsetBuffer
	{
		XMFLOAT4 offset;
	};
	OffsetBuffer m_Offset;
	UINT8* m_pCbvDataBegin;

	ComPtr<ID3D12PipelineState>  m_pPipelineState;
	ComPtr<ID3D12RootSignature>  m_pRootSignature;
	ComPtr<ID3D12Resource>       m_pVertexBuffer;
	ComPtr<ID3D12DescriptorHeap> m_pCBVHeap;
	ComPtr<ID3D12Resource>       m_pMatrixBuffer;
	D3D12_VERTEX_BUFFER_VIEW     m_VertexBufferView;
};

}
#endif