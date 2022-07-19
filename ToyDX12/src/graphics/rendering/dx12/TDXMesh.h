#pragma once

#include "wrl/client.h"
#include "MathUtil.h"
#include "DX12RenderingPipeline.h"
#include "DX12Geometry.h"
#include "Material.h"
#include "MeshLoader.h"

#include <set>

struct D3D12_INDEX_BUFFER_VIEW;
struct D3D12_VERTEX_BUFFER_VIEW;
struct BasicVertex;
struct Primitive;

struct Primitive
{
	size_t NumIndices;
	size_t StartIndexLocation;	// The location of the first index read by the GPU from the index buffer. == Number of indices before the first index of this primitive
	size_t BaseVertexLocation;  // A value added to each index before reading a vertex from the vertex buffer == Number of vertices before the first vertex of this primitive
	int MaterialId;	// To retrieve material properties is the unordered map
	std::string MaterialName;	// To retrieve material properties is the unordered map

	DirectX::XMMATRIX WorldMatrix;
};

struct MeshData
{
	std::vector<Vertex>    Vertices;
	std::vector<uint16_t>  Indices;
	std::vector<Primitive> Primitives;

	std::vector<MaterialProperties> materials;
	std::unordered_map<const char*, int> materialTable;

	std::vector<Texture> textures;
	std::unordered_map<const char*, int> textureTable;
};

namespace ToyDX
{
	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const char* sz_Filename) ;

		void CreateFromFile(const char* sz_Filename);

	public:
		template <typename T>
		void Create(T* a_Vertices, size_t ul_NumVertices, uint16_t* a_Indices, size_t ul_NumIndices, D3D12_INPUT_LAYOUT_DESC* inputLayout)
		{
			m_InputLayout = inputLayout;

			m_IndexCount  = ul_NumIndices;
			m_VertexCount = ul_NumVertices;

			const UINT64 ui64_VertexBufferSizeInBytes = ul_NumVertices * sizeof(T);
			const UINT64 ui64_IndexBufferSizeInBytes  = ul_NumIndices * sizeof(uint16_t);

			p_VertexBufferGPU.Reset();
			p_IndexBufferGPU.Reset();

			// Create the vertex buffer and a view to it
			p_VertexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Vertices, ui64_VertexBufferSizeInBytes, p_VertexBufferCPU, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			p_IndexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Indices, ui64_IndexBufferSizeInBytes, p_IndexBufferCPU, D3D12_RESOURCE_STATE_INDEX_BUFFER);

			// Create buffer views
			m_VertexBufferView = DX12RenderingPipeline::CreateVertexBufferView(p_VertexBufferGPU, ui64_VertexBufferSizeInBytes, sizeof(T));
			m_IndexBufferView = DX12RenderingPipeline::CreateIndexBufferView(p_IndexBufferGPU, ui64_IndexBufferSizeInBytes, DXGI_FORMAT_R16_UINT);
		}

		size_t IndexCount() { return m_IndexCount; }
		size_t VertexCount() { return m_VertexCount; }

		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_IndexBufferView; }
		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView()  const { return m_VertexBufferView; }
		D3D12_INPUT_LAYOUT_DESC* GetInputLayout() { return m_InputLayout; }

		MeshData Data;

		~Mesh();
	protected:


		size_t m_IndexCount = 0;
		size_t m_VertexCount = 0;

		// Default buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> p_VertexBufferGPU = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> p_IndexBufferGPU  = nullptr;

		// Upload buffers
		Microsoft::WRL::ComPtr<ID3D12Resource> p_VertexBufferCPU  = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> p_IndexBufferCPU   = nullptr;

		// Descriptors
		D3D12_INDEX_BUFFER_VIEW  m_IndexBufferView;
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INPUT_LAYOUT_DESC* m_InputLayout = nullptr;
	};
}