#include "pch.h"
#include "TDXRenderer.h"
#include "TDXShader.h"

#include "DX12Geometry.h"
#include "ToyDXUploadBuffer.h"
#include "DX12RenderingPipeline.h"
#include "TDXMesh.h"

void ToyDX::Renderer::Initialize()
{
	// Renderer
	ID3D12CommandAllocator& rst_CommandAllocator = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue = DX12RenderingPipeline::GetCommandQueue();

	ThrowIfFailed(rst_CommandList.Reset(&rst_CommandAllocator, nullptr));

	// Create a mesh
	m_Mesh = std::make_shared<ToyDX::Mesh>();
	m_Mesh->CreateCube();
	m_Mesh->Scale({ 1, 1, 1 });

	LoadShaders();
	CreateConstantBuffer();
	CreateRootSignature();
	SetRasterizerState();
	CreatePipelineStateObject();

	ThrowIfFailed(rst_CommandList.Close());
	ID3D12CommandList* a_CmdLists[1] = { &rst_CommandList };
	rst_CommandQueue.ExecuteCommandLists(_countof(a_CmdLists), a_CmdLists);
	DX12RenderingPipeline::FlushCommandQueue();
}

void ToyDX::Renderer::Render()
{
	ID3D12CommandAllocator& rst_CommandAllocator = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue = DX12RenderingPipeline::GetCommandQueue();

	int w = 1;
	int h = 1;

	for (int x = 0; x < w; x++)
	{
		for (int z = 0; z < h; z++)
		{
			
			rst_CommandList.DrawIndexedInstanced(m_Mesh->IndexCount(), 1, 0, 0, 0);
		}
	}
}

void ToyDX::Renderer::CreateConstantBuffer()
{
	// Create heap to store constant buffers descriptors
	if (m_ConstantBuffer == nullptr)
	{
		m_ConstantBuffer = std::make_unique<ToyDX::UploadBuffer>();
		m_ConstantBuffer->Create(DX12RenderingPipeline::GetDevice(), 1, sizeof(PerObjectData), true);

		m_CbvHeap = DX12RenderingPipeline::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		// Address to the start of the buffer
		D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = m_ConstantBuffer->GetResource()->GetGPUVirtualAddress();

		// Get offset to ith constant buffer
		int i = 0;
		cbvAddress += i * m_ConstantBuffer->GetElementSizeInBytes();

		// Create a descriptor to the ith constant buffer
		DX12RenderingPipeline::CreateConstantBufferView(m_CbvHeap.Get(), cbvAddress, m_ConstantBuffer->GetElementSizeInBytes());
	}
}

void ToyDX::Renderer::CreateRootSignature()
{
	// A Root Signature defines what resources the application will bind to the rendering pipeline (it doesn't bind the resources)
	// Root parameter : can be a descriptor table, root descriptor or root constant 
	CD3DX12_ROOT_PARAMETER rootParameterSlot[1] = { };

	// Create a single descriptor table of Constant Buffer Views
	CD3DX12_DESCRIPTOR_RANGE cbvDescriptorTable(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// How many descriptors in the table
		0	// Shader register that this root parameter (here a constant buffer) will be bound to. (register (b0))
	);

	rootParameterSlot[0].InitAsDescriptorTable(1, &cbvDescriptorTable);

	// Root signature : array of root parameters
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pParameters = rootParameterSlot;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// *** Create *** 
	// 1. A Root Signature 
	// 2. with a single slot
	// 3. pointing to descriptor range 
	// 4. consisting of a single constant buffer 
	ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSignatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	ThrowIfFailed(DX12RenderingPipeline::GetDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void ToyDX::Renderer::BindResources()
{
	ID3D12GraphicsCommandList& rst_CommandList = DX12RenderingPipeline::GetCommandList();

	// Bind pipeline state
	DX12RenderingPipeline::GetCommandList().SetPipelineState(m_Pso.Get());
	DX12RenderingPipeline::GetCommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set Vertex and Index buffer views
	DX12RenderingPipeline::GetCommandList().IASetVertexBuffers(0, 1, &m_Mesh->GetVertexBufferView());
	DX12RenderingPipeline::GetCommandList().IASetIndexBuffer(&m_Mesh->GetIndexBufferView());

	// Set the root signature we are going to use 
	rst_CommandList.SetGraphicsRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	rst_CommandList.SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvDescriptor(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
	cbvDescriptor.Offset(0, DX12RenderingPipeline::GetCachedValues().descriptorSizes.CBV_SRV_UAV);

	rst_CommandList.SetGraphicsRootDescriptorTable(0, cbvDescriptor);
}

void ToyDX::Renderer::SetRasterizerState(bool bWireframe, bool bBackFaceCulling)
{
	m_RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//m_RasterizerState.FillMode = bWireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	//m_RasterizerState.CullMode = bBackFaceCulling ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;

	// By default : Front face is clockwise
}

void ToyDX::Renderer::CreatePipelineStateObject()
{
	DXGI_FORMAT rtvFormats[] = {DXGI_FORMAT_R8G8B8A8_UNORM};
	m_Pso = DX12RenderingPipeline::CreatePipelineStateObject(
		m_RootSignature.Get(),
		m_DefaultVertexShader->GetByteCode(),
		m_DefaultPixelShader->GetByteCode(),
		*m_Mesh->GetInputLayout(),
		rtvFormats,
		m_RasterizerState,
		_countof(rtvFormats)
	);
}

void ToyDX::Renderer::LoadShaders()
{
	m_DefaultVertexShader = std::make_shared<Shader>();
	m_Mesh->GetInputLayout();
	m_DefaultVertexShader->Compile(L"C:\\dev\\GithubRepos\\ToyDX12\\ToyDX12\\data\\shaders\\DefaultVertex.hlsl", "main", ShaderKind::VERTEX);

	m_DefaultPixelShader = std::make_shared<Shader>();
	m_DefaultPixelShader->Compile(L"C:\\dev\\GithubRepos\\ToyDX12\\ToyDX12\\data\\shaders\\DefaultPixel.hlsl", "main", ShaderKind::PIXEL);
}

void ToyDX::Renderer::Terminate()
{
}