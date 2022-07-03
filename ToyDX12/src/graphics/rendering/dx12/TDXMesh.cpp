#include "pch.h"
#include "TDXMesh.h"

#include "MeshLoader.h"

namespace ToyDX
{
	void Mesh::CreateFromFile(const char* sz_Filename)
	{
		const char* ext = strrchr(sz_Filename, '.');

		if (!ext)
		{
			return;
		}

		ext = ext + 1;

		// GLTF Format
		if (strcmp(ext, "gltf") == 0 || strcmp(ext, "glb") == 0)
		{
			DirectX::XMFLOAT4X4 worldMatrix = MathUtil::Float4x4Identity();
			MeshLoader::LoadGltf(sz_Filename, &Data);

			Create<Vertex>(Data.Vertices.data(), Data.Vertices.size(), Data.Indices.data(), Data.Indices.size(), &VertexInputLayoutDesc);
		}

	}
}

