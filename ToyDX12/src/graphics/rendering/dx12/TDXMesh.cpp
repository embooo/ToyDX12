#include "pch.h"
#include "TDXMesh.h"

#include "MeshLoader.h"

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
		Create<BasicVertex>(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices), &BasicVertexInputLayoutDesc);
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

		Create<BasicVertex>(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices), &BasicVertexInputLayoutDesc);
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

		Create<BasicVertex>(a_Vertices, _countof(a_Vertices), a_Indices, _countof(a_Indices), &BasicVertexInputLayoutDesc);
	}

	void Mesh::CreateFromFile(const char* sz_Filename)
	{
		const char* ext = strrchr(sz_Filename, '.');

		if (!ext)
		{
			return;
		}

		ext = ext + 1;

		if (strcmp(ext, "gltf") == 0 || strcmp(ext, "glb") == 0)
		{
			DirectX::XMFLOAT4X4 worldMatrix = MathUtil::Float4x4Identity();
			MeshLoader::LoadGltf(sz_Filename, gltfMesh);

			//m_Transform.WorldMatrix = gltfMesh.worldMatrix;

			Create<Vertex>(gltfMesh.vertices.data(), gltfMesh.vertices.size(), gltfMesh.indices.data(), gltfMesh.indices.size(), &VertexInputLayoutDesc);

			Scale({ 0.1, 0.1, 0.1 });
		}


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

