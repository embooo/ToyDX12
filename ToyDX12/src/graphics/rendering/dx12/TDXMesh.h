#pragma once

#include "wrl/client.h"
#include "MathUtil.h"

struct D3D12_INDEX_BUFFER_VIEW;
struct D3D12_VERTEX_BUFFER_VIEW;
class BasicVertex;

namespace ToyDX
{
	struct Transform
	{
		DirectX::XMVECTOR Translation = {0.0f, 0.0f, 0.0f};
		DirectX::XMVECTOR Rotation	  = {0.0f, 0.0f, 0.0f}; // Pitch, Roll, Yaw 
		DirectX::XMVECTOR Scale		  = {1.0f, 1.0f, 1.0f};

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();

		void ComputeWorldMatrix()
		{
			WorldMatrix = ComputeWorldMatrix(Translation, Rotation, Scale);
		}

		static DirectX::XMMATRIX ComputeWorldMatrix(DirectX::XMVECTOR vTranslation, DirectX::XMVECTOR vRotation, DirectX::XMVECTOR vScale)
		{
			auto T = DirectX::XMMatrixTranslationFromVector(vTranslation);
			auto R = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMVectorGetY(vRotation), DirectX::XMVectorGetX(vRotation), DirectX::XMVectorGetZ(vRotation));
			auto S = DirectX::XMMatrixScalingFromVector(vScale);

			return T * R * S;
		}
	};

	class Mesh
	{
	public:
		Mesh() = default;

		void Create();
		void CreateCube();
		void CreateTriangle();
		void Create(BasicVertex* a_Vertices, size_t ul_NumVertices, uint16_t* a_Indices, size_t ul_NumIndices);

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

		~Mesh() { LOG_WARN("Mesh::~Mesh()"); };
	protected:
		// Geometric properties
		Transform m_Transform;
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