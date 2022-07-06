#pragma once

#include "DirectXMath.h"
#include "MathUtil.h"
#include "d3d12.h"
#include "TDXMesh.h"

static const int DefaultNumFrameResources = 3;

namespace ToyDX
{
	// Describes a drawable item and the data that is needed to render it
	class Drawable
	{
	public:
		Drawable() = default;
		Drawable(Mesh* mesh);
		Drawable(Mesh* mesh, Primitive* primitive, Material* material);

		DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixTranslationFromVector({0,0,0});

		Mesh* Mesh = nullptr;

		Material* material = nullptr;
		
		bool HasSubMeshes = false;

		// Number of frame resources that need an update, initially : all frame resources
		unsigned int NumFramesDirty = DefaultNumFrameResources;

		// Index in GPU constant buffer of the corresponding per object CB for this drawable
		size_t PerObjectCbIndex = -1;

		size_t NumIndices = 0;	
		size_t StartIndexLocation = 0;	// The location of the first index read by the GPU from the index buffer. == Number of indices before the first index of this primitive
		size_t BaseVertexLocation = 0;  // A value added to each index before reading a vertex from the vertex buffer == Number of vertices before the first vertex of this primitive

		// Type of geometric primitive
		D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};
}