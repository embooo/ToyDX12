#include "pch.h"
#include "TDXMesh.h"

#include "DX12Geometry.h"
#include "DX12RenderingPipeline.h"

namespace ToyDX
{
	void Mesh::Create()
	{
		BasicVertex a_Vertices[] =
		{
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3( 1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f),  DirectX::XMFLOAT2(0.0f, 0.0f) },
			{ DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f),  DirectX::XMFLOAT2(1.0f, 0.0f) }
		};

		uint16_t a_Indices[] =
		{
			1, 0, 2,
			2, 3, 1
		};

		// Create GPU resources
		Create(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices));
	}

	void Mesh::CreateCube()
	{
		BasicVertex a_Vertices[] =
		{
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
			{ DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }
		};

		uint16_t a_Indices[] =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		Create(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices));
	}

	void Mesh::CreateTriangle()
	{
		BasicVertex a_Vertices[] =
		{
			{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(-0.5f, 0.0f) },
			{ DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.5f) },
			{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.5f, -0.5f) }
		};

		uint16_t a_Indices[] =
		{
			0, 1, 2
		};

		Create(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices));
	}

	void Mesh::Create(BasicVertex* a_Vertices, size_t ul_NumVertices, uint16_t* a_Indices, size_t ul_NumIndices)
	{
		m_InputLayout = &BasicVertexInputLayoutDesc;

		m_ulNumIndices   = ul_NumIndices;
		m_ulNumVertices = ul_NumVertices;

		const UINT64 ui64_VertexBufferSizeInBytes = ul_NumVertices * sizeof(BasicVertex);
		const UINT64 ui64_IndexBufferSizeInBytes  = ul_NumIndices  * sizeof(uint16_t);

		p_VertexBufferGPU.Reset();
		p_IndexBufferGPU.Reset();

		// Create the vertex buffer and a view to it
		p_VertexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Vertices, ui64_VertexBufferSizeInBytes, p_VertexBufferCPU, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		p_IndexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Indices, ui64_IndexBufferSizeInBytes, p_IndexBufferCPU, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// Create buffer views
		m_VertexBufferView = DX12RenderingPipeline::CreateVertexBufferView(p_VertexBufferGPU, ui64_VertexBufferSizeInBytes, sizeof(BasicVertex));
		m_IndexBufferView = DX12RenderingPipeline::CreateIndexBufferView(p_IndexBufferGPU, ui64_IndexBufferSizeInBytes, DXGI_FORMAT_R16_UINT);

		// Init transform
		m_Transform.ComputeWorldMatrix();
	}

	void Mesh::Scale(DirectX::XMVECTOR vScale)
	{
		m_Transform.Scale = vScale;
		m_Transform.ComputeWorldMatrix();
	}

	void Mesh::Translate(DirectX::XMVECTOR vTranslate)
	{
		m_Transform.Translation = DirectX::XMVectorAdd(m_Transform.Translation, vTranslate);
		
		m_Transform.ComputeWorldMatrix();
	}

	void Mesh::SetPosition(DirectX::XMVECTOR vPos)
	{
		m_Transform.Translation = vPos;
		m_Transform.ComputeWorldMatrix();
	}

}

