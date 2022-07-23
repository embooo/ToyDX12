#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Logger.h"
#include "MeshLoader.h"
#include "TDXMesh.h"

using namespace DirectX;

std::string MeshLoader::m_MeshRootPath;

void MeshLoader::LoadGltf(const char* sz_Filename, MeshData* mesh)
{
	cgltf_options options = { };
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, sz_Filename, &data);
	
	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers(&options, data, sz_Filename);

		m_MeshRootPath = sz_Filename;
		m_MeshRootPath = m_MeshRootPath.substr(0, m_MeshRootPath.find_last_of('/') + 1).c_str();


		//LOG_WARN("MESH ROOT PATH : {0}", m_MeshRootPath);

		if (result == cgltf_result_success)
		{
			for (size_t i = 0; i < data->nodes_count; ++i)
			{
				cgltf_node& currNode = data->nodes[i];

				ProcessGltfNode(false, &currNode, mesh);
			}
		}

		LOG_INFO("Loaded : {0}", sz_Filename);
		LOG_INFO("# Materials : {0}", mesh->materials.size());
		LOG_INFO("# Textures : {0}", mesh->textures.size());

		cgltf_free(data);
	}
	else
	{
		LOG_ERROR("Could not read GLTF/GLB file. [error code : {0}]", result);
		assert(false);
	}

	m_MeshRootPath.clear();
}

static void LoadVertices(cgltf_primitive* primitive, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	size_t firstVertex = st_Mesh->Vertices.size();

	std::vector<DirectX::XMFLOAT3> positionsBuffer;
	std::vector<DirectX::XMFLOAT3> normalsBuffer;
	std::vector<DirectX::XMFLOAT2> texCoordBuffer;
	std::vector<DirectX::XMFLOAT4> tangentBuffer;
	
	// Build raw buffers containing each attributes components
	for (int attribIdx = 0; attribIdx < primitive->attributes_count; ++attribIdx)
	{
		cgltf_attribute* attribute = &primitive->attributes[attribIdx];

		switch (attribute->type)
		{
		case cgltf_attribute_type_position:
		{
			////LOG_DEBUG("        Positions : {0}", attribute->data->count);
			positionsBuffer.resize(attribute->data->count);
			for (int i = 0; i < attribute->data->count; ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &positionsBuffer[i].x, 3);
			} 
		}
		break;

		case cgltf_attribute_type_normal:
		{
			////LOG_DEBUG("        Normals : {0}", attribute->data->count);
			normalsBuffer.resize(attribute->data->count);
			for (int i = 0; i < normalsBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &normalsBuffer[i].x, 3);
			}
		}
		break;

		case cgltf_attribute_type_texcoord:
		{
			////LOG_DEBUG("        Normals : {0}", attribute->data->count);
			texCoordBuffer.resize(attribute->data->count);
			for (int i = 0; i < texCoordBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &texCoordBuffer[i].x, 2);
			}
		}
		break;
		
		case cgltf_attribute_type_tangent:
		{
			////LOG_DEBUG("        Tangents : {0}", attribute->data->count);
			tangentBuffer.resize(attribute->data->count);
			for (int i = 0; i < tangentBuffer.size(); ++i)
			{
				cgltf_accessor_read_float(attribute->data, i, &tangentBuffer[i].x, 4);
			}
		}
		break;

		}
	}

	// Build vertices
	for (int i = 0; i < positionsBuffer.size(); ++i)
	{
		Vertex vertex;
		{
			vertex.Pos	     = positionsBuffer[i];
			vertex.Normal	 = normalsBuffer.empty() ? DirectX::XMFLOAT3{ 0.0, 0.0, 0.0 } : normalsBuffer[i];
			vertex.Tangent	 = tangentBuffer.empty() ? DirectX::XMFLOAT4{ 0.0, 0.0, 0.0, 0.0 } : tangentBuffer[i];
			vertex.TexCoord0 = texCoordBuffer.empty() ? DirectX::XMFLOAT2{ 0.0, 0.0 } : texCoordBuffer[i];
		}

		st_Mesh->Vertices.push_back(vertex);
	}
	
	////LOG_DEBUG("        Vertex buffer size : {0}", st_Mesh.vertices.size());
}

static void LoadIndices(cgltf_primitive* rawPrimitive, Primitive* primitive, MeshData* st_Mesh)
{
	primitive->NumIndices = rawPrimitive->indices->count;
	for (int idx = 0; idx < rawPrimitive->indices->count; ++idx)
	{
		st_Mesh->Indices.push_back(uint16_t(cgltf_accessor_read_index(rawPrimitive->indices, idx)));
	}

	////LOG_DEBUG("        Index buffer size : {0}", st_Mesh.indices.size());
}

void SetMaterial(MeshData* data, Primitive* primitive, cgltf_material* rawMaterial, MaterialProperties& material)
{
	auto materialIte = data->materialTable.find(rawMaterial->name);

	if (rawMaterial->name != nullptr && materialIte != data->materialTable.end()) 
	{
		primitive->MaterialName = materialIte->first;
		primitive->MaterialId   = materialIte->second;
	}
	else
	{
		// Insert new material
		int materialId = data->materials.size();

		std::string name = std::string("Material ").append(std::to_string(materialId));

		material.Id = materialId;
		material.name = name;

		primitive->MaterialName = material.name;
		primitive->MaterialId = materialId;

		data->materialTable.insert({ name.c_str(), materialId});
		data->materials.push_back(material);
	}
}

int LoadImageFile(MeshData* data, const std::string& rootPath, cgltf_image* image)
{
	char* uri = image->uri;

	// Loads an image into memory if it isn't already and returns a handle to it
	int width, height, channels;
	
	std::string path = rootPath + uri;
	
	int handle = -1;

	auto textureIte = data->textureTable.find(uri);

	if (textureIte != data->textureTable.end())
	{
		handle = textureIte->second;
	}
	else
	{
		unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (imgData == NULL)
		{
			//LOG_ERROR("MeshLoader::LoadImageFile : Could not load image at {0}.", path);
			return -1;
		}

		// Load and store new texture
		Texture texture { 
			.Name = image->name ? image->name : path.c_str(),
			.Id = (int)data->textures.size(), 
			.Width = width, 
			.Height = height, 
			.Channels = STBI_rgb_alpha,
			.data = imgData
		};
		handle = texture.Id;
		// Update texture list
		data->textures.push_back(texture);
		
		data->textureTable[uri] = texture.Id;

		//LOG_INFO("MeshLoader::LoadImageFile : Loaded {0} - {1}x{2}x{3}", path.substr(path.find_last_of('/') + 1, path.size()), width, height, channels);
	}

	return handle;
}


void LoadMaterial(cgltf_primitive* rawPrimitive, Primitive* primitive, MeshData* data)
{
	// https://kcoley.github.io/glTF/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness/
	// Default model used in this engine is Specular-Glossiness
	cgltf_material* material = rawPrimitive->material;
	
	MetallicRoughness  metallicRoughness;
	SpecularGlossiness specularGlossiness;

	MaterialProperties materialProperties;

	// Load additional textures : emissive, normal maps ...
	cgltf_texture* emissiveTexture = material->emissive_texture.texture;
	cgltf_texture* normalTexture = material->normal_texture.texture;

	if (emissiveTexture)
	{
		materialProperties.hEmissiveTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, emissiveTexture->image);
		materialProperties.hasEmissive = true;
	}

	if (normalTexture)
	{
		materialProperties.hNormalTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, normalTexture->image);
		materialProperties.hasNormalMap = true;
	}

	if (material->has_pbr_specular_glossiness)
	{
		//LOG_DEBUG("MATERIAL WORKFLOW: Specular-Glossiness, Name : {0}", material->name);
		
		materialProperties.type = MaterialWorkflowType::SpecularGlossiness;
		materialProperties.specularGlossiness =
		{
			.DiffuseFactor = DirectX::XMFLOAT4(material->pbr_specular_glossiness.diffuse_factor),
			.SpecularFactor = DirectX::XMFLOAT3(material->pbr_specular_glossiness.specular_factor),
			.GlossinessFactor = material->pbr_specular_glossiness.glossiness_factor
		};

		// Texture loading
		cgltf_texture* diffuseTexture = material->pbr_specular_glossiness.diffuse_texture.texture;
		cgltf_texture* specularGlossinessTexture = material->pbr_specular_glossiness.specular_glossiness_texture.texture;

		if (diffuseTexture != nullptr)
		{
			materialProperties.specularGlossiness.hDiffuseTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, diffuseTexture->image);
			materialProperties.specularGlossiness.hasDiffuse = true;

		}
		
		if (specularGlossinessTexture != nullptr)
		{
			materialProperties.specularGlossiness.hSpecularGlossinessTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, specularGlossinessTexture->image);
			materialProperties.specularGlossiness.hasSpecularGlossiness = true;

		}

		SetMaterial(data, primitive, material, materialProperties);
	}

	else if (material->has_pbr_metallic_roughness)
	{
		LOG_DEBUG("Material Workflow: Metallic/Roughness, Name : {0}", material->name ? material->name : "Unnamed");

		materialProperties.type = MaterialWorkflowType::MetallicRoughness;
		materialProperties.metallicRoughness = 
		{
			.BaseColor = DirectX::XMFLOAT4(material->pbr_metallic_roughness.base_color_factor),
			.Metallic = material->pbr_metallic_roughness.metallic_factor,
			.Roughness = material->pbr_metallic_roughness.roughness_factor
		};

		// Texture loading
		cgltf_texture* baseColorTexture = material->pbr_metallic_roughness.base_color_texture.texture;
		cgltf_texture* metallicRoughnessTexture = material->pbr_metallic_roughness.metallic_roughness_texture.texture;
		
		if (baseColorTexture != nullptr)
		{
			materialProperties.metallicRoughness.hBaseColorTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, baseColorTexture->image);
			materialProperties.metallicRoughness.hasBaseColorTex = true;
		}
	

		if (metallicRoughnessTexture != nullptr)
		{
			materialProperties.metallicRoughness.hMetallicRoughnessTexture = LoadImageFile(data, MeshLoader::m_MeshRootPath, metallicRoughnessTexture->image);
			materialProperties.metallicRoughness.hasMetallicRoughnessTex = true;
		}

		SetMaterial(data, primitive, material, materialProperties);
	}


}

static void LoadPrimitive(cgltf_primitive* primitive, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	////LOG_DEBUG("      Primitive : {0} attributes, {1} indices", primitive->attributes_count, primitive->indices->count);

	Primitive p = {.StartIndexLocation = st_Mesh->Indices.size(), .BaseVertexLocation = st_Mesh->Vertices.size(), .WorldMatrix = currWorldMat };

	LoadIndices(primitive, &p, st_Mesh);
	LoadVertices(primitive, st_Mesh, currWorldMat);
	LoadMaterial(primitive, &p, st_Mesh);

	st_Mesh->Primitives.push_back(p);
}


static void LoadMesh(cgltf_mesh* mesh, MeshData* st_Mesh, const DirectX::XMMATRIX& currWorldMat)
{
	//LOG_DEBUG("    Mesh '{0}': {1} primitives", mesh->name ? mesh->name : "Unnamed", mesh->primitives_count);
	
	for (int i = 0; i < mesh->primitives_count; ++i)
	{
		LoadPrimitive(&mesh->primitives[i], st_Mesh, currWorldMat);
	}
}

void LoadTransform(cgltf_node* p_Node, DirectX::XMMATRIX& currWorldMat)
{
	DirectX::XMMATRIX RHtoLH =
	{
		1,  0,  0,  0,
		0,  1,  0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1
	};

	//LOG_DEBUG("TRANSFORM : World");
	float worldMat[16];
	cgltf_node_transform_world(p_Node, worldMat);
	
	currWorldMat = DirectX::XMMatrixMultiply(DirectX::XMMATRIX(worldMat), RHtoLH);

	for (int i = 0; i < 16; i += 4)
	{
		//LOG_DEBUG("{0}, {1}, {2}, {3}", worldMat[i], worldMat[i + 1], worldMat[i + 2], worldMat[i + 3]);
	}
}

void MeshLoader::ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, MeshData* mesh)
{
	DirectX::XMMATRIX currWorldMat = DirectX::XMMatrixIdentity();

	if (bIsChild)
	{
		//LOG_DEBUG("    Child Node : {0}, Children : {1}", p_Node->name ? p_Node->name : "Unnamed", p_Node->children_count);
	}
	else
	{
		//LOG_DEBUG("=====================================");
		//LOG_DEBUG("Node : {0}, Children : {1}", p_Node->name ? p_Node->name : "Unnamed", p_Node->children_count);
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
			//LOG_DEBUG("  Parent Node : {0}", p_Node->name ? p_Node->name : "Unnamed");
		}
		ProcessGltfNode(true, p_Node->children[j], mesh);
	}
}
