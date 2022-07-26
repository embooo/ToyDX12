#pragma once

#include "d3d12.h"
#include "DirectXMath.h"

struct BasicVertex
{
	DirectX::XMFLOAT3 Pos;			// Offset : 0 bytes
	DirectX::XMFLOAT2 TexCoord0;	// Offset : 12 bytes
};

const D3D12_INPUT_ELEMENT_DESC BasicVertexElementsDesc[]
{
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
	{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos;			// Offset : 0 bytes
	DirectX::XMFLOAT3 Normal;		// Offset : 12 bytes
	DirectX::XMFLOAT3 Tangent;		// Offset : 24 bytes
	DirectX::XMFLOAT2 TexCoord0;	// Offset : 36 bytes
};

const D3D12_INPUT_ELEMENT_DESC VertexElementsDesc[]
{
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
	{ "POSITION",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "NORMAL",		0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "TANGENT",	0,	DXGI_FORMAT_R32G32B32_FLOAT,	0,	24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{ "TEXCOORD",	0,	DXGI_FORMAT_R32G32_FLOAT,		0,	36,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

static D3D12_INPUT_LAYOUT_DESC BasicVertexInputLayoutDesc   = { BasicVertexElementsDesc, 2 };
static D3D12_INPUT_LAYOUT_DESC VertexInputLayoutDesc		= { VertexElementsDesc, 4 };