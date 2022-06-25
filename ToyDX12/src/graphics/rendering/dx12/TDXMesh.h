#pragma once

#include "wrl/client.h"
#include "MathUtil.h"
#include "DX12RenderingPipeline.h"
#include "DX12Geometry.h"
#include "MeshLoader.h"

struct D3D12_INDEX_BUFFER_VIEW;
struct D3D12_VERTEX_BUFFER_VIEW;
class BasicVertex;

namespace ToyDX
{
	struct Transform
	{
		Transform()
			: WorldMatrix(DirectX::XMMatrixIdentity())
		{}
		DirectX::XMVECTOR Translation = {0.0f, 0.0f, 0.0f};
		DirectX::XMVECTOR Rotation	  = {0.0f, 0.0f, 0.0f}; // Pitch, Roll, Yaw 
		DirectX::XMVECTOR Scale		  = {1.0f, 1.0f, 1.0f};

		DirectX::XMMATRIX WorldMatrix;

		void ComputeWorldMatrix()
		{
			WorldMatrix = ComputeWorldMatrix(Translation, Rotation, Scale);
		}

		static DirectX::XMMATRIX ComputeWorldMatrix(DirectX::XMVECTOR vTranslation, DirectX::XMVECTOR vRotation, DirectX::XMVECTOR vScale)
		{
			auto T = DirectX::XMMatrixTranslationFromVector(vTranslation);
			auto R = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMVectorGetY(vRotation), DirectX::XMVectorGetX(vRotation), DirectX::XMVectorGetZ(vRotation));
			auto S = DirectX::XMMatrixScalingFromVector(vScale);

			// Assuming column-major matrices
			return T * R * S;
		}
	};

	struct SubMeshes
	{
		std::vector<int> FirstVertexPosition;
		std::vector<int> FirstIndexPosition;
	};

	enum class MeshFormat
	{
		NONE, 
		GLTF
	};

	class Mesh
	{
	public:
		Mesh() = default;

		void Create();
		void CreateCube();
		void CreateTriangle();
		void CreateFromFile(const char* sz_Filename);

	public:
		template <typename T>
		void Create(T* a_Vertices, size_t ul_NumVertices, uint16_t* a_Indices, size_t ul_NumIndices, D3D12_INPUT_LAYOUT_DESC* inputLayout)
		{
			m_InputLayout = inputLayout;

			m_ulNumIndices = ul_NumIndices;
			m_ulNumVertices = ul_NumVertices;

			const UINT64 ui64_VertexBufferSizeInBytes = ul_NumVertices * sizeof(T);
			const UINT64 ui64_IndexBufferSizeInBytes = ul_NumIndices * sizeof(uint16_t);

			p_VertexBufferGPU.Reset();
			p_IndexBufferGPU.Reset();

			// Create the vertex buffer and a view to it
			p_VertexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Vertices, ui64_VertexBufferSizeInBytes, p_VertexBufferCPU, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			p_IndexBufferGPU = DX12RenderingPipeline::CreateDefaultBuffer(a_Indices, ui64_IndexBufferSizeInBytes, p_IndexBufferCPU, D3D12_RESOURCE_STATE_INDEX_BUFFER);

			// Create buffer views
			m_VertexBufferView = DX12RenderingPipeline::CreateVertexBufferView(p_VertexBufferGPU, ui64_VertexBufferSizeInBytes, sizeof(T));
			m_IndexBufferView = DX12RenderingPipeline::CreateIndexBufferView(p_IndexBufferGPU, ui64_IndexBufferSizeInBytes, DXGI_FORMAT_R16_UINT);

			// Init transform
			m_Transform.ComputeWorldMatrix();
		}

		size_t IndexCount() { return m_ulNumIndices; }
		size_t NumVertices() { return m_ulNumVertices; }

		const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_IndexBufferView; }
		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView()  const { return m_VertexBufferView; }
		D3D12_INPUT_LAYOUT_DESC* GetInputLayout() { return m_InputLayout; }

		void Scale(DirectX::XMVECTOR vScale);
		void Translate(DirectX::XMVECTOR vTranslate);
		void SetPosition(DirectX::XMVECTOR vPos);

		const Transform& GetTransform() const { return m_Transform; };
		const DirectX::XMMATRIX& GetWorldMatrix() const { return m_Transform.WorldMatrix; };

		GltfMesh gltfMesh;

		~Mesh() { LOG_WARN("Mesh::~Mesh()"); };
	protected:
		// Geometric properties
		Transform m_Transform;

		// File format
		MeshFormat fileFormat = MeshFormat::NONE;



	protected:
		size_t m_ulNumIndices  = 0;
		size_t m_ulNumVertices = 0;

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