#pragma once

#include "DX12Geometry.h"

class cgltf_node;

class MeshLoader
{
public:
	MeshLoader() = delete;
	~MeshLoader() = delete;

	static void LoadGltf(const char* sz_Filename, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, DirectX::XMFLOAT4X4& worldMat);

protected:
	static void ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, DirectX::XMFLOAT4X4& worldMat);
};