#pragma once

#include "DX12Geometry.h"

class cgltf_node;
struct MeshData;

class MeshLoader
{
public:
	MeshLoader() = delete;
	~MeshLoader() = delete;

	static void LoadGltf(const char* sz_Filename, MeshData* mesh);

protected:
	static void ProcessGltfNode(bool bIsChild, cgltf_node* p_Node, MeshData* mesh);
};