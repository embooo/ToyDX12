#pragma once

#include "DX12Geometry.h"

class cgltf_node;

struct GltfMesh;
struct GltfPrimitive;
struct GltfModel;

struct GltfMesh
{
	DirectX::XMMATRIX worldMatrix;
	std::vector<Vertex>   vertices;
	std::vector<uint16_t> indices;

	std::vector<GltfPrimitive> primitives;
};


struct GltfPrimitive
{
	size_t NumIndices;	
	size_t StartIndexLocation;	// The location of the first index read by the GPU from the index buffer. == Number of indices before the first index of this primitive
	size_t BaseVertexLocation;  // A value added to each index before reading a vertex from the vertex buffer == Number of vertices before the first vertex of this primitive
};

struct GltfModel
{
	std::vector<GltfMesh> meshes;
};

class MeshLoader
{
public:
	MeshLoader() = delete;
	~MeshLoader() = delete;

	static void LoadGltf(const char* sz_Filename, GltfMesh& mesh);

protected:
	static void ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, GltfMesh& mesh);
};