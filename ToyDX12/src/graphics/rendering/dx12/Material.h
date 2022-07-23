#pragma once

#include "MathUtil.h"

const int NumFrameResources = 3;

enum class MaterialWorkflowType
{
	None = 0,
	SpecularGlossiness = 1,
	MetallicRoughness  = 2
};

struct Texture
{
	const char* Name;
	int Id = -1;
	int Width = -1;
	int Height = -1;
	int Channels = -1;
	unsigned char* data;

	int SrvHeapIndex = -1;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

struct SpecularGlossiness
{
	DirectX::XMFLOAT4 DiffuseFactor = { 1.0f, 1.0f, 1.0f, 1.0f };	// DiffuseAlbedo
	DirectX::XMFLOAT3 SpecularFactor = { 0.01f, 0.01f, 0.01f };		// F0
	float GlossinessFactor = 1 - 0.25f;								// 1 - Roughness

	bool hasDiffuse = false;
	bool hasSpecularGlossiness = false;
	int hDiffuseTexture= -1;
	int SpecGlossSrvHeapIndex = -1;
	int hSpecularGlossinessTexture = -1;
};

struct MetallicRoughness
{
	DirectX::XMFLOAT4 BaseColor;
	float Metallic = 0.0f;
	float Roughness = 0.25f;

	bool hasBaseColorTex = false;
	bool hasMetallicRoughnessTex = false;

	int hBaseColorTexture = -1;
	int hMetallicRoughnessTexture = -1;

	int MetallicRoughnessSrvHeapIndex = -1;
};

struct MaterialProperties
{
	std::string name;
	int Id = -1;

	bool hasEmissive = false;
	bool hasNormalMap = false;
	int hEmissiveTexture = -1;
	int hNormalTexture   = -1;

	MaterialWorkflowType type;

	struct SpecularGlossiness specularGlossiness;
	struct MetallicRoughness  metallicRoughness;
};



struct Material
{
	std::string Name;

	// Index corresponding to this material into the constant buffer 
	int CBIndex = -1;

	// Index corresponding to the diffuse texture into the SRV heap
	int DiffuseSrvHeapIndex = -1; 
	int BaseColorSrvHeapIndex = -1; 

	int NormalSrvHeapIndex = -1; 

	// Indicate that the material has changed and we need to update the constant buffers
	// of each frame resources
	int NumFramesDirty = NumFrameResources;

	// Constant buffer data
	// Physical properties
	MaterialProperties properties;
	DirectX::XMFLOAT4X4 MatTransform = MathUtil::Float4x4Identity();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
