#pragma once

#include "MathUtil.h"

const int NumFrameResources = 3;

enum class MaterialWorkflowType
{
	None = 0,
	SpecularGlossiness = 1,
	MetallicRoughness  = 2
};

struct SpecularGlossiness
{
	DirectX::XMFLOAT4 DiffuseFactor = { 1.0f, 1.0f, 1.0f, 1.0f };	// DiffuseAlbedo
	DirectX::XMFLOAT3 SpecularFactor = { 0.01f, 0.01f, 0.01f };		// F0
	float GlossinessFactor = 1 - 0.25f;								// 1 - Roughness
};

struct MetallicRoughness
{
	DirectX::XMFLOAT4 BaseColorFactor;
	float MetallicFactor = 0.0f;
	float RoughnessFactor = 0.25f;
};

union MaterialWorkflow
{
	struct SpecularGlossiness specularGlossiness;
	struct MetallicRoughness  metallicRoughness;

	MaterialWorkflow() { memset(this, 0, sizeof(MaterialWorkflow)); }
};


struct MaterialProperties
{
	int Id = -1;

	MaterialWorkflowType type;
	MaterialWorkflow workflow;
};

struct Material
{
	const char* Name;

	// Index corresponding to this material into the constant buffer 
	int CBIndex = -1;

	// Index corresponding to the diffuse texture into the SRV heap
	int DiffuseSrvHeapIndex = -1;

	// Indicate that the material has changed and we need to update the constant buffers
	// of each frame resources
	int NumFramesDirty = NumFrameResources;

	// Constant buffer data
	// Physical properties
	MaterialProperties properties;
	DirectX::XMFLOAT4X4 MatTransform = MathUtil::Float4x4Identity();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
