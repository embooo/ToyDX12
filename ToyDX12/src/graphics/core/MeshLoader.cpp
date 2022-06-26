#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Logger.h"
#include "MeshLoader.h"
#include "DirectXMath.h"

void MeshLoader::LoadGltf(const char* sz_Filename, GltfMesh& mesh)
{
	cgltf_options options = { };
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, sz_Filename, &data);
	
	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers(&options, data, sz_Filename);

		if (result == cgltf_result_success)
		{
			for (size_t i = 0; i < data->nodes_count; ++i)
			{
				cgltf_node& currNode = data->nodes[i];
				LOG_DEBUG("=====================================");
				LOG_DEBUG("Node : {0}, Children : {1}", currNode.name ? currNode.name : "UNNAMED", currNode.children_count);

				ProcessGltfNode(false, &currNode, mesh);
				
				for (size_t j = 0; j < currNode.children_count; ++j)
				{
					LOG_DEBUG("  Child Node : {0}, Children : {1}", currNode.name ? currNode.name : "Unnamed", currNode.children_count);
					ProcessGltfNode(true, currNode.children[j], mesh);
				}

			}
		}

		cgltf_free(data);
	}
	else
	{
		LOG_ERROR("Could not read GLTF/GLB file. [error code : {0}]", result);
		return;
	}
}

static void LoadVertices(cgltf_primitive* primitive, GltfMesh& st_Mesh)
{
	size_t firstVertex = st_Mesh.vertices.size();

	std::vector<DirectX::XMFLOAT3> positionsBuffer;
	std::vector<DirectX::XMFLOAT3> normalsBuffer;
	std::vector<DirectX::XMFLOAT2> texCoordBuffer;

	// Build raw buffers containing each attributes components
	for (int attribIdx = 0; attribIdx < primitive->attributes_count; ++attribIdx)
	{
		cgltf_attribute* attribute = &primitive->attributes[attribIdx];

		switch (attribute->type)
		{
		case cgltf_attribute_type_position:
		{
			//LOG_DEBUG("        Positions : {0}", attribute->data->count);
			positionsBuffer.resize(attribute->data->count);
			for (int i = 0; i < attribute->data->count; ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &positionsBuffer[i].x, 3);
			}
		}
		break;

		case cgltf_attribute_type_normal:
		{
			//LOG_DEBUG("        Normals : {0}", attribute->data->count);
			normalsBuffer.resize(attribute->data->count);
			for (int i = 0; i < normalsBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &normalsBuffer[i].x, 3);
			}
		}
		case cgltf_attribute_type_texcoord:
		{
			//LOG_DEBUG("        Normals : {0}", attribute->data->count);
			texCoordBuffer.resize(attribute->data->count);
			for (int i = 0; i < texCoordBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &texCoordBuffer[i].x, 2);
			}
		}
		break;

		}
	}

	// Build vertices
	for (int i = 0; i < positionsBuffer.size(); ++i)
	{
		Vertex vertex =
		{
			.Pos    = positionsBuffer[i], 
			.Normal = normalsBuffer[i],
			.TexCoord0 = texCoordBuffer[i],
			.TexCoord1 = {0,0}
		};

		st_Mesh.vertices.push_back(vertex);
	}

	//LOG_ERROR("        Vertex buffer size : {0}", st_Mesh.vertices.size());


}

static void LoadIndices(cgltf_primitive* primitive, GltfMesh& st_Mesh)
{
	st_Mesh.primitives.back().NumIndices = primitive->indices->count;
	for (int idx = 0; idx < primitive->indices->count; ++idx)
	{
		st_Mesh.indices.push_back(uint16_t(cgltf_accessor_read_index(primitive->indices, idx)));
	}

	//LOG_ERROR("        Index buffer size : {0}", st_Mesh.indices.size());
}

static void LoadPrimitive(cgltf_primitive* primitive, GltfMesh& st_Mesh)
{
	//LOG_DEBUG("      Primitive : {0} attributes, {1} indices", primitive->attributes_count, primitive->indices->count);

	GltfPrimitive p = {.StartIndexLocation = st_Mesh.indices.size(), .BaseVertexLocation = st_Mesh.vertices.size()};
	st_Mesh.primitives.push_back(p);

	LoadIndices(primitive, st_Mesh);
	LoadVertices(primitive, st_Mesh);
}


static void LoadMesh(cgltf_mesh* mesh, GltfMesh& st_Mesh)
{
	LOG_DEBUG("    Mesh '{0}': {1} primitives", mesh->name ? mesh->name : "Unnamed", mesh->primitives_count);

	for (int i = 0; i < mesh->primitives_count; ++i)
	{
		LoadPrimitive(&mesh->primitives[i], st_Mesh);
	}
}

void MeshLoader::ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, GltfMesh& mesh)
{
	if (p_Node->mesh)
	{
		float WorldTransformBuffer[16];
		cgltf_node_transform_world(p_Node, WorldTransformBuffer);
		DirectX::XMMATRIX WorldTransformMat(WorldTransformBuffer);
		mesh.worldMatrix = WorldTransformMat;
		LoadMesh(p_Node->mesh, mesh);
	}
}

