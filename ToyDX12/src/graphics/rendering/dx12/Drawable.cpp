#include "pch.h"

#include "Drawable.h"

namespace ToyDX
{
	Drawable::Drawable(ToyDX::Mesh* mesh)
		: Mesh(mesh)
	{
		// Check if the mesh is an aggregate of primitives, with different offsets in the vertex/index buffer
		// or a single primitive, spanning the whole buffers

		HasSubMeshes = mesh->Data.Primitives.size() > 1;

		if (HasSubMeshes == false)
		{
			NumIndices = mesh->Data.Primitives[0].NumIndices;
			StartIndexLocation = mesh->Data.Primitives[0].StartIndexLocation;
			BaseVertexLocation = mesh->Data.Primitives[0].BaseVertexLocation;
		}
	}
	Drawable::Drawable(ToyDX::Mesh* mesh, Primitive* primitive, Material* material)
		: Mesh(mesh), material(material)
	{
		HasSubMeshes = false;

		NumIndices = primitive->NumIndices;
		WorldMatrix = &primitive->WorldMatrix;
		StartIndexLocation = primitive->StartIndexLocation;
		BaseVertexLocation = primitive->BaseVertexLocation;
	}
}



