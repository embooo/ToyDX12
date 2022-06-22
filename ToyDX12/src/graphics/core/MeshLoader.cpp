#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Logger.h"
#include "MeshLoader.h"
#include "DirectXMath.h"

void MeshLoader::LoadGltf(const char* sz_Filename, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, DirectX::XMFLOAT4X4& worldMat)
{
	cgltf_options options = { };
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, sz_Filename, &data);
	LOG_WARN("{}\n", result);
	
	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers(&options, data, sz_Filename);

		if (result == cgltf_result_success)
		{
			for (size_t i = 0; i < data->nodes_count; ++i)
			{
				cgltf_node& currNode = data->nodes[i];
				LOG_WARN("Node : {0}, Children : {1}", currNode.name, currNode.children_count);

				ProcessGltfNode(false, &currNode, vertices, indices, worldMat);
				
				for (size_t j = 0; j < currNode.children_count; ++j)
				{
					ProcessGltfNode(true, currNode.children[j], vertices, indices, worldMat);
				}

			}
		}

		cgltf_free(data);
	}
}


void MeshLoader::ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, DirectX::XMFLOAT4X4& worldMat)
{
	if (p_Node->mesh)
	{
		cgltf_node_transform_world(p_Node, *worldMat.m);
		
		if (bIsChild)
		{
			LOG_WARN("	Child Mesh : {0}, Primitives : {1}", p_Node->mesh->name, p_Node->mesh->primitives_count);
		}
		else
		{
			LOG_WARN("	Mesh : {0}, Primitives : {1}", p_Node->mesh->name, p_Node->mesh->primitives_count);
		}

		const cgltf_mesh* mesh = p_Node->mesh;
		for(size_t i = 0; i < mesh->primitives_count; ++i)
		{
			const cgltf_primitive& primitive = mesh->primitives[i];
			
			// VERTICES
			for (int attr = 0; attr < primitive.attributes_count; ++attr)
			{
				const cgltf_attribute& currAttribute = primitive.attributes[attr];

				if (currAttribute.data)
				{
					vertices.resize(currAttribute.data->count);

					// Fill vertex buffer depending on attribute type
					if (strcmp(currAttribute.name, "POSITION") == 0)
					{
						// data->count is the number of elements of this attribute type
						for (int pos = 0; pos < currAttribute.data->count; ++pos) {
							if (cgltf_accessor_read_float(currAttribute.data, pos, &vertices[pos].Pos.x, 3))
							{
								//LOG_WARN("Found position : {0}, {1}, {2}\n", vertices[pos].Pos.x, vertices[pos].Pos.y, vertices[pos].Pos.z);
							}
						}
					}

					else if (strcmp(currAttribute.name, "NORMAL") == 0)
					{
						for (int nrm = 0; nrm < currAttribute.data->count; ++nrm) {
							if (cgltf_accessor_read_float(currAttribute.data, nrm, &vertices[nrm].Normal.x, 3))
							{
								//LOG_WARN("Found normal : {0}, {1}, {2}\n", vertices[nrm].Normal.x, vertices[nrm].Normal.y, vertices[nrm].Normal.z);
							}
						}
					}

					else if (strcmp(currAttribute.name, "TANGENT") == 0)
					{

					}

					else if (strcmp(currAttribute.name, "TEXCOORD_0") == 0)
					{

					}

					else if (strcmp(currAttribute.name, "TEXCOORD_1") == 0)
					{

					}
				}



			}
			
			// INDICES
			if (primitive.indices)
			{
				const cgltf_accessor* accessor = primitive.indices;
				indices.resize(accessor->count);

				for (int ind = 0; ind < accessor->count; ++ind)
				{
					indices[ind] = (uint16_t)cgltf_accessor_read_index(accessor, ind);
				}
			}

		}


	}
}