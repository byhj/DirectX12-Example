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
	void Update();
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
	void init_buffer(ComPtr<ID3D12Device> pD3D12Device);
	void init_shader(ComPtr<ID3D12Device> pD3D12Device);
private:


	struct Vertex
	{
		XMFLOAT3 position;
	};
	struct ConstantBufferData
	{
		XMFLOAT4 velocity;
		XMFLOAT4 offset;
		XMFLOAT4 color;
		XMMATRIX projection;

		// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
		// to be array-indexed.
		float padding[36];
	};

	//Root constants for the compute shader
	struct CSRootConstancts
	{
		float xOffset;
		float zOffset;
		float cullOffset;
		float commandCount;
	};


	ComPtr<ID3D12PipelineState>  m_pPipelineState;
	ComPtr<ID3D12RootSignature>  m_pRootSignature;
	ComPtr<ID3D12Resource>       m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW     m_VertexBufferView;
};

}
#endif