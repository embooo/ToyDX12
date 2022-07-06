#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Logger.h"
#include "MeshLoader.h"
#include "TDXMesh.h"
#include "DirectXMath.h"


void MeshLoader::LoadGltf(const char* sz_Filename, MeshData* mesh)
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

				ProcessGltfNode(false, &currNode, mesh);
			}
		}


		LOG_INFO("# Materials : {0}", mesh->materials.size());

		cgltf_free(data);
	}
	else
	{
		LOG_ERROR("Could not read GLTF/GLB file. [error code : {0}]", result);
		assert(false);
	}
}

static void LoadVertices(cgltf_primitive* primitive, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	size_t firstVertex = st_Mesh->Vertices.size();

	std::vector<DirectX::XMFLOAT3> positionsBuffer;
	std::vector<DirectX::XMFLOAT3> normalsBuffer;
	std::vector<DirectX::XMFLOAT2> texCoordBuffer;
	std::vector<DirectX::XMFLOAT3> tangentBuffer;
	
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
		case cgltf_attribute_type_tangent:
		{
			//LOG_DEBUG("        Tangents : {0}", attribute->data->count);
			tangentBuffer.resize(attribute->data->count);
			for (int i = 0; i < tangentBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &tangentBuffer[i].x, 3);
			}
		}
		break;

		}
	}

	// Build vertices
	for (int i = 0; i < positionsBuffer.size(); ++i)
	{
		auto Pos = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&positionsBuffer[i]), currWorldMat);
		
		Vertex vertex;
		{
			DirectX::XMStoreFloat3(&vertex.Pos, Pos);
			vertex.Normal	 = normalsBuffer.empty() ? DirectX::XMFLOAT3{ 0.0, 0.0, 0.0 } : normalsBuffer[i];
			vertex.Tangent	 = tangentBuffer.empty() ? DirectX::XMFLOAT3{ 0.0, 0.0, 0.0 } : tangentBuffer[i];
			vertex.TexCoord0 = texCoordBuffer.empty() ? DirectX::XMFLOAT2{ 0.0, 0.0 } : texCoordBuffer[i];
		}

		st_Mesh->Vertices.push_back(vertex);
	}
	
	//LOG_ERROR("        Vertex buffer size : {0}", st_Mesh.vertices.size());
}

static void LoadIndices(cgltf_primitive* rawPrimitive, Primitive* primitive, MeshData* st_Mesh)
{
	primitive->NumIndices = rawPrimitive->indices->count;
	for (int idx = 0; idx < rawPrimitive->indices->count; ++idx)
	{
		st_Mesh->Indices.push_back(uint16_t(cgltf_accessor_read_index(rawPrimitive->indices, idx)));
	}

	//LOG_ERROR("        Index buffer size : {0}", st_Mesh.indices.size());
}

void SetMaterial(MeshData* data, Primitive* primitive, cgltf_material* rawMaterial, MaterialProperties& material)
{
	auto materialIte = data->materialTable.find(rawMaterial->name);

	if (materialIte != data->materialTable.end())
	{
		primitive->MaterialId = materialIte->second;
	}
	else
	{
		// Insert new material
		int materialId = data->materials.size();

		material.Id = materialId;
		data->materialTable.insert({ rawMaterial->name, materialId });
		data->materials.push_back(material);
		primitive->MaterialId = materialId;
	}
}

void LoadMaterial(cgltf_primitive* rawPrimitive, Primitive* primitive, MeshData* data)
{
	// https://kcoley.github.io/glTF/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness/
	// Default model used in this engine is Specular-Glossiness
	cgltf_material* material = rawPrimitive->material;
	
	MetallicRoughness  metallicRoughness;
	SpecularGlossiness specularGlossiness;

	if (material->has_pbr_specular_glossiness)
	{
		LOG_ERROR("MATERIAL WORKFLOW: Specular-Glossiness, Name : {0}", material->name);

		specularGlossiness.DiffuseFactor    = DirectX::XMFLOAT4(material->pbr_specular_glossiness.diffuse_factor);
		specularGlossiness.GlossinessFactor = material->pbr_specular_glossiness.glossiness_factor;
		specularGlossiness.SpecularFactor   = DirectX::XMFLOAT3(material->pbr_specular_glossiness.specular_factor);

		material->pbr_specular_glossiness.diffuse_texture;
		material->pbr_specular_glossiness.specular_glossiness_texture;

		
		
		MaterialProperties materialProperties;
		materialProperties.type = MaterialWorkflowType::SpecularGlossiness;
		materialProperties.workflow.specularGlossiness = specularGlossiness;

		SetMaterial(data, primitive, material, materialProperties);
	}

	else if (material->has_pbr_metallic_roughness)
	{
		LOG_ERROR("MATERIAL WORKFLOW: Metallic-Roughness, Name : {0}", material->name);
		
		metallicRoughness.BaseColorFactor = DirectX::XMFLOAT4(material->pbr_metallic_roughness.base_color_factor);
		metallicRoughness.MetallicFactor  = material->pbr_metallic_roughness.metallic_factor;
		metallicRoughness.RoughnessFactor = material->pbr_metallic_roughness.roughness_factor;

		material->pbr_metallic_roughness.base_color_texture;
		material->pbr_metallic_roughness.metallic_roughness_texture;

		MaterialProperties materialProperties;
		materialProperties.type = MaterialWorkflowType::MetallicRoughness;
		materialProperties.workflow.metallicRoughness = metallicRoughness;

		SetMaterial(data, primitive, material, materialProperties);
	}
}

static void LoadPrimitive(cgltf_primitive* primitive, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	//LOG_DEBUG("      Primitive : {0} attributes, {1} indices", primitive->attributes_count, primitive->indices->count);

	Primitive p = {.StartIndexLocation = st_Mesh->Indices.size(), .BaseVertexLocation = st_Mesh->Vertices.size()};

	LoadIndices(primitive, &p, st_Mesh);
	LoadVertices(primitive, st_Mesh, currWorldMat);
	LoadMaterial(primitive, &p, st_Mesh);

	st_Mesh->Primitives.push_back(p);
}


static void LoadMesh(cgltf_mesh* mesh, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	LOG_DEBUG("    Mesh '{0}': {1} primitives", mesh->name ? mesh->name : "Unnamed", mesh->primitives_count);

	for (int i = 0; i < mesh->primitives_count; ++i)
	{
		LoadPrimitive(&mesh->primitives[i], st_Mesh, currWorldMat);
	}
}

void LoadTransform(cgltf_node* p_Node, DirectX::XMMATRIX& currWorldMat)
{
	LOG_DEBUG("TRANSFORM : World");
	float worldMat[16];
	cgltf_node_transform_world(p_Node, worldMat);

	currWorldMat = DirectX::XMMATRIX(worldMat);
	for (int i = 0; i < 16; i += 4)
	{
		LOG_DEBUG("{0}, {1}, {2}, {3}", worldMat[i], worldMat[i + 1], worldMat[i + 2], worldMat[i + 3]);
	}
}

void MeshLoader::ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, MeshData* mesh)
{
	DirectX::XMMATRIX currWorldMat = DirectX::XMMatrixIdentity();

	if (bIsChild)
	{
		LOG_DEBUG("    Child Node : {0}, Children : {1}", p_Node->name ? p_Node->name : "Unnamed", p_Node->children_count);
	}
	else
	{
		LOG_DEBUG("=====================================");
		LOG_DEBUG("Node : {0}, Children : {1}", p_Node->name ? p_Node->name : "Unnamed", p_Node->children_count);
	}

	if (p_Node->mesh)
	{
		LoadTransform(p_Node, currWorldMat);
		LoadMesh(p_Node->mesh, mesh, currWorldMat);
	}

	for (size_t j = 0; j < p_Node->children_count; ++j)
	{
		if (j == 0)
		{
			LOG_DEBUG("  Parent Node : {0}", p_Node->name ? p_Node->name : "Unnamed");
		}
		ProcessGltfNode(true, p_Node->children[j], mesh);
	}
}

