#include "pch.h"
#include "Timer.h"

#include "TDXRenderer.h"
#include "TDXShader.h"

#include "DX12Geometry.h"
#include "ToyDXUploadBuffer.h"
#include "DX12RenderingPipeline.h"
#include "TDXMesh.h"
#include "ToyDXCamera.h"
#include "FrameResource.h"

void ToyDX::Renderer::Initialize()
{
	// Renderer
	ID3D12CommandAllocator& rst_CommandAllocator = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue = DX12RenderingPipeline::GetCommandQueue();

	ThrowIfFailed(rst_CommandList.Reset(&rst_CommandAllocator, nullptr));

	// Create a mesh
	LoadMeshes();
	BuildMaterials();
	BuildDrawables();
	LoadShaders();

	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildConstantBufferViews();
	BuildRootSignature();
	CreatePipelineStateObjects();


	ThrowIfFailed(rst_CommandList.Close());
	ID3D12CommandList* a_CmdLists[1] = { &rst_CommandList };
	rst_CommandQueue.ExecuteCommandLists(_countof(a_CmdLists), a_CmdLists);
	DX12RenderingPipeline::FlushCommandQueue();
}

void ToyDX::Renderer::UpdateFrameResource()
{
	ID3D12CommandQueue& r_CommandQueue = DX12RenderingPipeline::GetCommandQueue();

	AdvanceToNextFrameResource();

	UINT64 LastCompletedFenceValue = DX12RenderingPipeline::s_Fence->GetCompletedValue(); // Last completed fence
	UINT64 FrameResourceFenceValue = GetCurrentFrameResource()->FenceValue; // Last completed fence

	// Wait until GPU has completed the commands up to this fence point 
	// Before updating this frame resource
	if (FrameResourceFenceValue != 0 && LastCompletedFenceValue < FrameResourceFenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		DX12RenderingPipeline::s_Fence->SetEventOnCompletion(GetCurrentFrameResource()->FenceValue, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdatePerObjectCBs();
	UpdatePerPassCB();
	UpdateMaterialCBs();
}

void ToyDX::Renderer::UpdatePerObjectCBs()
{
	// Called once per frame
	UploadBuffer* currObjectCB = m_CurrentFrameResource->cbPerObject.get();

	for (auto& d : m_AllDrawables)
	{
		if (d->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX world = d->WorldMatrix;

			PerObjectData perObjectData;
			DirectX::XMStoreFloat4x4(&perObjectData.gWorld, DirectX::XMMatrixTranspose(world));
			currObjectCB->CopyData(d->PerObjectCbIndex, &perObjectData, sizeof(PerObjectData));
			d->NumFramesDirty--;
		}
	}
}

void ToyDX::Renderer::UpdatePerPassCB()
{
	auto ViewMat = DirectX::XMMatrixTranspose(m_CameraHandle->GetViewMatrix());
	auto ProjMat = DirectX::XMMatrixTranspose(m_CameraHandle->GetProjMatrix());

	DirectX::XMStoreFloat4x4(&perPassData.gView, ViewMat);
	DirectX::XMStoreFloat4x4(&perPassData.gInvView, XMMatrixInverse(nullptr, ViewMat));
	DirectX::XMStoreFloat4x4(&perPassData.gProj, ProjMat);
	DirectX::XMStoreFloat4x4(&perPassData.gInvProj, XMMatrixInverse(nullptr, ProjMat));
	DirectX::XMStoreFloat3(&perPassData.gEyePosWS, m_CameraHandle->GetPosWS());

	perPassData.gNear = m_CameraHandle->GetFrustum().fNearZ;
	perPassData.gFar  = m_CameraHandle->GetFrustum().fFarZ;
	perPassData.gDeltaTime = m_TimerHandle->GetDeltaTime();
	perPassData.gTotalTime = m_TimerHandle->GetTotalTime();
	
	UploadBuffer* currPassCB = m_CurrentFrameResource->cbPerPass.get();
	currPassCB->CopyData(0, &perPassData, sizeof(PerPassData));
}

void ToyDX::Renderer::UpdateMaterialCBs()
{
	UploadBuffer* currMaterialCb = m_CurrentFrameResource->cbMaterial.get();
	
	for (auto& material : m_Materials)
	{
		Material* mat = material.second.get();

		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;

			matConstants.DiffuseFactor    = mat->properties.workflow.specularGlossiness.DiffuseFactor;
			matConstants.SpecularFactor   = mat->properties.workflow.specularGlossiness.SpecularFactor;
			matConstants.GlossinessFactor = mat->properties.workflow.specularGlossiness.GlossinessFactor;

			currMaterialCb->CopyData(mat->CBIndex, &matConstants, sizeof(MaterialConstants));

			mat->NumFramesDirty--;
		}
	}
}

void ToyDX::Renderer::RenderOpaques(ID3D12GraphicsCommandList* cmdList, std::vector<Drawable*>& opaques)
{
	size_t NumFrameResources = m_FrameResources.size();
	size_t NumMaterials = m_Materials.size();
	size_t NumOpaques = m_AllDrawables.size();
	size_t NumPerPassCbv = m_FrameResources.size(); // 1 pass cbv per frame resource

	for (size_t i=0; i < NumOpaques; ++i)
	{
		Drawable* obj = m_AllDrawables[i].get();
		cmdList->IASetPrimitiveTopology(obj->PrimitiveTopology);
		cmdList->IASetVertexBuffers(0, 1, &obj->Mesh->GetVertexBufferView());
		cmdList->IASetIndexBuffer(&obj->Mesh->GetIndexBufferView());
		
		// Offset in the descriptor heap for this drawable's per object constant buffer
		size_t objectDescriptorIndexInDescHeap = m_CurrentFrameResourceIdx * NumOpaques + obj->PerObjectCbIndex;
		D3D12_GPU_DESCRIPTOR_HANDLE objectDescriptorTable = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart()).Offset(objectDescriptorIndexInDescHeap, DX12RenderingPipeline::CBV_SRV_UAV_Size);
		cmdList->SetGraphicsRootDescriptorTable(0, objectDescriptorTable);

		// Set material
		size_t materialDescriptorIndexInDescHeap = (m_IndexOf_FirstMaterialCbv_DescriptorHeap + m_CurrentFrameResourceIdx * NumMaterials) + obj->material->CBIndex;
		D3D12_GPU_DESCRIPTOR_HANDLE materialDescriptorTable = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart()).Offset(materialDescriptorIndexInDescHeap, DX12RenderingPipeline::CBV_SRV_UAV_Size);
		cmdList->SetGraphicsRootDescriptorTable(1, materialDescriptorTable);

		if (obj->HasSubMeshes)
		{
			for (size_t p = 0; p < obj->Mesh->Data.Primitives.size(); ++p)
			{
				Primitive* prim = &obj->Mesh->Data.Primitives[p];
				cmdList->DrawIndexedInstanced(prim->NumIndices, 1, prim->StartIndexLocation, prim->BaseVertexLocation, 0);
			}
		}
		else
		{
			cmdList->DrawIndexedInstanced(obj->NumIndices, 1, obj->StartIndexLocation, obj->BaseVertexLocation, 0);
		}
	}
}

void ToyDX::Renderer::Render()
{
	ID3D12CommandAllocator& rst_CommandAllocator    = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList      = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue            = DX12RenderingPipeline::GetCommandQueue();

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished
	// execution on the GPU.
	ThrowIfFailed(m_CurrentFrameResource->cmdListAllocator->Reset());

	// A command list can be reset after it has been added to the
	// command queue via ExecuteCommandList. Reusing the command list reuses memory.
	ThrowIfFailed(rst_CommandList.Reset(m_CurrentFrameResource->cmdListAllocator.Get(), GetPipelineState("Solid")));

	// Indicate a state transition on the resource usage.
	ID3D12Resource* backBufferResource = m_hRenderingPipeline->GetCurrentBackBuffer();
	ID3D12Resource* depthStencilResource = m_hRenderingPipeline->GetDepthStencil();
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = m_hRenderingPipeline->GetCurrentBackBufferView();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	rst_CommandList.ResourceBarrier(1, &barrier);

	// Set the viewport and scissor rect. This needs to be reset
	// whenever the command list is reset.
	rst_CommandList.RSSetViewports(1, &m_hRenderingPipeline->GetViewport());
	rst_CommandList.RSSetScissorRects(1, &m_hRenderingPipeline->GetScissorRect());

	// Clear the back buffer and depth buffer.
	rst_CommandList.ClearRenderTargetView(backBufferView, m_hRenderingPipeline->RTClearValues.Color, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_hRenderingPipeline->GetDepthStencilView();
	rst_CommandList.ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		m_hRenderingPipeline->DSClearValues.DepthStencil.Depth,
		m_hRenderingPipeline->DSClearValues.DepthStencil.Stencil,
		0, nullptr); // Clear entire render target

	// Specify the buffers we are going to render to.
	rst_CommandList.OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	rst_CommandList.SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	rst_CommandList.SetGraphicsRootSignature(m_RootSignature.Get());

	// Build and submit command lists for this frame.
	{
		// Set per pass constant buffer
		int passCbvIndex = m_IndexOf_FirstPerPassCbv_DescriptorHeap + m_CurrentFrameResourceIdx;
		D3D12_GPU_DESCRIPTOR_HANDLE passCbvDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart()).Offset(passCbvIndex, DX12RenderingPipeline::CBV_SRV_UAV_Size);
		rst_CommandList.SetGraphicsRootDescriptorTable(2, passCbvDescriptor);

		// Set Per Object Constant Buffer and render
		RenderOpaques(&rst_CommandList, m_OpaqueDrawables);
	}

	// Indicate a state transition on the resource usage.
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	rst_CommandList.ResourceBarrier(1, &barrier);

	// Stop recording commands
	ThrowIfFailed(rst_CommandList.Close());

	// Add command list to GPU command queue for execution
	ID3D12CommandList* a_CommandLists[] = { &rst_CommandList };
	rst_CommandQueue.ExecuteCommandLists(_countof(a_CommandLists), a_CommandLists);

	// Present then swap front and back buffer
	ThrowIfFailed(m_hRenderingPipeline->GetSwapChain()->Present(0, 0)); // SyncInterval = 0  : V-Sync disabled
	m_hRenderingPipeline->SetCurrentBackBufferIndex((m_hRenderingPipeline->CurrentBackBufferIndex() + 1) % m_hRenderingPipeline->SwapChainBufferCount());

	// Advance the fence value to mark commands up to this fence point.
	m_CurrentFrameResource->FenceValue = ++m_CurrentFence;
	rst_CommandQueue.Signal(DX12RenderingPipeline::s_Fence.Get(), m_CurrentFence);
}

void ToyDX::Renderer::AdvanceToNextFrameResource()
{
	m_CurrentFrameResourceIdx = (m_CurrentFrameResourceIdx + 1) % DefaultNumFrameResources;
	m_CurrentFrameResource = m_FrameResources[m_CurrentFrameResourceIdx].get();
}

void ToyDX::Renderer::BuildFrameResources()
{
	for (int i = 0; i < DefaultNumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(
			DX12RenderingPipeline::GetDevice(),
			1, // Number of passes
			m_AllDrawables.size(),// Number of objects
			m_Materials.size()
		));
	}

	m_CurrentFrameResourceIdx = 0;
	m_CurrentFrameResource = m_FrameResources[m_CurrentFrameResourceIdx].get();
}

void ToyDX::Renderer::BuildDescriptorHeaps()
{
	size_t NumDrawables = m_AllDrawables.size();
	size_t NumFrameResources = m_FrameResources.size();
	size_t NumMaterials = m_Materials.size();

	// We have 1 CBV per frame resource (per pass constants)
	// We have 1 CBV per drawable (per object constant)
	// We have 1 CBV per material (per material constants)
	// We need to create (NumFrameResources * NumDrawables) + NumFrameResources CBVs
	size_t NumDescriptors = (NumDrawables + NumMaterials) * NumFrameResources + NumFrameResources;

	// Offset to the materials CBVs
	m_IndexOf_FirstMaterialCbv_DescriptorHeap = NumDrawables * NumFrameResources;

	// Offset to the per pass CBVs
	m_IndexOf_FirstPerPassCbv_DescriptorHeap = (NumDrawables + NumMaterials) * NumFrameResources;

	m_CbvHeap = DX12RenderingPipeline::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void ToyDX::Renderer::BuildConstantBufferViews()
{
	// Build per object constant buffer views
	size_t PerObjectCbSizeCPU = sizeof(PerObjectData);
	size_t PerObjectCbSizeGPU = UploadBuffer::CalcConstantBufferSize(PerObjectCbSizeCPU);

	size_t NumObjects = m_AllDrawables.size();

	for (int i = 0; i < m_FrameResources.size(); ++i)
	{
		const auto& cbObject = m_FrameResources[i]->cbPerObject->GetResource();

		for (int j = 0; j < NumObjects; ++j)
		{
			D3D12_GPU_VIRTUAL_ADDRESS CurrentCbGPUAddr = cbObject->GetGPUVirtualAddress();

			// Offset to the jth constant buffer 
			CurrentCbGPUAddr += j * PerObjectCbSizeGPU;

			// Offset to the its descriptor in the descriptor heap
			int indexInDescriptorHeap = i * NumObjects + j;
			D3D12_CPU_DESCRIPTOR_HANDLE descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart()).Offset(indexInDescriptorHeap, DX12RenderingPipeline::CBV_SRV_UAV_Size);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = CurrentCbGPUAddr;
			cbvDesc.SizeInBytes    = PerObjectCbSizeGPU;

			DX12RenderingPipeline::GetDevice()->CreateConstantBufferView(&cbvDesc, descriptor);
		}
	}

	// Build materials constant buffer views
	size_t MaterialCbSizeCPU = sizeof(MaterialConstants);
	size_t MaterialCbSizeGPU = UploadBuffer::CalcConstantBufferSize(MaterialCbSizeCPU);

	size_t NumMaterials = m_Materials.size();

	for (int i = 0; i < m_FrameResources.size(); ++i)
	{
		const auto& materialCB = m_FrameResources[i]->cbMaterial->GetResource();

		for (int j = 0; j < NumMaterials; ++j)
		{
			D3D12_GPU_VIRTUAL_ADDRESS CurrentCbGPUAddr = materialCB->GetGPUVirtualAddress();

			// Offset to the jth constant buffer 
			CurrentCbGPUAddr += j * MaterialCbSizeGPU;

			// Offset to the its descriptor in the descriptor heap
			int indexInDescriptorHeap = (m_IndexOf_FirstMaterialCbv_DescriptorHeap + i * NumMaterials) + j;

			D3D12_CPU_DESCRIPTOR_HANDLE descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart()).Offset(indexInDescriptorHeap, DX12RenderingPipeline::CBV_SRV_UAV_Size);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = CurrentCbGPUAddr;
			cbvDesc.SizeInBytes = MaterialCbSizeGPU;

			DX12RenderingPipeline::GetDevice()->CreateConstantBufferView(&cbvDesc, descriptor);
		}
	}

	// Build per pass constant buffer views
	size_t PerPassCbSizeCPU = sizeof(PerPassData);
	size_t PerPassCbSizeGPU = UploadBuffer::CalcConstantBufferSize(PerPassCbSizeCPU);

	for (int i = 0; i < m_FrameResources.size(); ++i)
	{
		D3D12_GPU_VIRTUAL_ADDRESS CurrentCbGPUAddr = m_FrameResources[i]->cbPerPass->GetResource()->GetGPUVirtualAddress();

		// Offset to the its descriptor in the descriptor heap
		int indexInDescriptorHeap = m_IndexOf_FirstPerPassCbv_DescriptorHeap + i;

		D3D12_CPU_DESCRIPTOR_HANDLE descriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart()).Offset(indexInDescriptorHeap, DX12RenderingPipeline::CBV_SRV_UAV_Size);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = CurrentCbGPUAddr;
		cbvDesc.SizeInBytes    = PerPassCbSizeGPU;

		DX12RenderingPipeline::GetDevice()->CreateConstantBufferView(&cbvDesc, descriptor);
	}

}

void ToyDX::Renderer::BuildMaterials()
{
	int CBIndex = 0;
	for (auto& mesh : m_Meshes)
	{
		for (auto& material : mesh->Data.materials)
		{
			std::unique_ptr<Material> rendererMaterial = std::make_unique<Material>();
			rendererMaterial->Name = material.name;
			rendererMaterial->properties.type = material.type;
			rendererMaterial->properties.workflow = material.workflow;
			rendererMaterial->CBIndex = CBIndex++;

			m_Materials[material.name] = std::move(rendererMaterial);
		}
	}
}

void ToyDX::Renderer::BuildRootSignature()
{
	// A Root Signature defines what resources the application will bind to the rendering pipeline (it doesn't bind the resources)
	// Root parameter : can be a descriptor table, root descriptor or root constant 
	CD3DX12_ROOT_PARAMETER rootParameterSlot[3] = { };

	// Create descriptor table of Constant Buffer Views
	CD3DX12_DESCRIPTOR_RANGE objectsCbvDescriptorTable0(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	// How many CBV descriptors in the table
		0	// Shader register that this root parameter (here a constant buffer) will be bound to. (e.g register (b0))
	);

	CD3DX12_DESCRIPTOR_RANGE materialsCbvDescriptorTable1(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 
		1,
		1
	);

	// Create descriptor table of Constant Buffer Views
	CD3DX12_DESCRIPTOR_RANGE passCbvDescriptorTable2(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1,	
		2	
	);

	rootParameterSlot[0].InitAsDescriptorTable(1, &objectsCbvDescriptorTable0);
	rootParameterSlot[1].InitAsDescriptorTable(1, &materialsCbvDescriptorTable1);
	rootParameterSlot[2].InitAsDescriptorTable(1, &passCbvDescriptorTable2);

	// A Root signature is an array of root parameters
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 3;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pParameters = rootParameterSlot;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// *** Create *** 
	// 1. A Root Signature 
	// 2. with N slots
	// 3. pointing to a descriptor range 
	// 4. consisting of a M constant buffer 
	ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, rootSignatureBlob.GetAddressOf(), errorBlob.GetAddressOf());
	ThrowIfFailed(DX12RenderingPipeline::GetDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void ToyDX::Renderer::SetRasterizerState(bool bWireframe, bool bBackFaceCulling)
{
	m_RasterizerState =
	CD3DX12_RASTERIZER_DESC (D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE,
    FALSE /* FrontCounterClockwise */,
    D3D12_DEFAULT_DEPTH_BIAS,
    D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
    D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
    TRUE /* DepthClipEnable */,
    TRUE /* MultisampleEnable */,
    FALSE /* AntialiasedLineEnable */,
    0 /* ForceSampleCount */,
    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);


	// By default : Front face is clockwise
}

void ToyDX::Renderer::CreatePipelineStateObjects()
{
	DXGI_FORMAT rtvFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM };

	m_PipelineStateObjects.insert
	(
		{ "Wireframe", DX12RenderingPipeline::CreatePipelineStateObject
			(
				m_RootSignature.Get(),
				m_DefaultVertexShader->GetByteCode(),
				m_DefaultPixelShader->GetByteCode(),
				*m_AllDrawables[0]->Mesh->GetInputLayout(),
				rtvFormats,
				CD3DX12_RASTERIZER_DESC (D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE,
				FALSE /* FrontCounterClockwise */,
				D3D12_DEFAULT_DEPTH_BIAS,
				D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
				D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
				TRUE /* DepthClipEnable */,
				TRUE /* MultisampleEnable */,
				FALSE /* AntialiasedLineEnable */,
				0 /* ForceSampleCount */,
				D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF),
				_countof(rtvFormats)
			) 
		}
	);

	m_PipelineStateObjects.insert
	(
		{ "Solid", DX12RenderingPipeline::CreatePipelineStateObject
			(
				m_RootSignature.Get(),
				m_DefaultVertexShader->GetByteCode(),
				m_DefaultPixelShader->GetByteCode(),
				*m_AllDrawables[0]->Mesh->GetInputLayout(),
				rtvFormats,
				CD3DX12_RASTERIZER_DESC(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK,
				FALSE /* FrontCounterClockwise */,
				D3D12_DEFAULT_DEPTH_BIAS,
				D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
				D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
				TRUE /* DepthClipEnable */,
				TRUE /* MultisampleEnable */,
				FALSE /* AntialiasedLineEnable */,
				0 /* ForceSampleCount */,
				D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF),
				_countof(rtvFormats)
			)
		}
	);
}

void ToyDX::Renderer::LoadShaders()
{
	m_DefaultVertexShader = std::make_shared<Shader>();
	m_DefaultVertexShader->Compile(L"./data/shaders/DefaultVertex.hlsl", "main", ShaderKind::VERTEX);

	m_DefaultPixelShader = std::make_shared<Shader>();
	m_DefaultPixelShader->Compile(L"./data/shaders/DefaultPixel.hlsl", "main", ShaderKind::PIXEL);
}

void ToyDX::Renderer::LoadMeshes()
{
	m_Meshes.push_back(std::make_unique<Mesh>("./data/models/unity_adam_head/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/suzanne/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/intel_sponza/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/unity_lieutenant_head/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/chest/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/930turbo/scene.gltf"));
	//m_Meshes.push_back(std::make_unique<Mesh>("./data/models/duck/scene.gltf"));

	//m_Meshes.back()->CreateFromFile("./data/models/trex/scene.glb");
	//m_Meshes.back()->CreateFromFile("./data/models/teapot/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/unity_adam_head/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/unity_lieutenant_head/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/trex/scene.glb");
	//m_Meshes.back()->CreateFromFile("./data/models/intel_sponza/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/intel_sponza_curtains/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/chest/scene.gltf");
	//m_Meshes.back()->CreateFromFile("./data/models/chandelier/scene.gltf");

}

void ToyDX::Renderer::BuildDrawables()
{
	int PerObjectCbIndex = 0;
	for (auto& mesh : m_Meshes)
	{
		for (auto& primitive : mesh->Data.Primitives)
		{
			Material* rendererMaterial = m_Materials.at(primitive.MaterialName).get();
			
			m_AllDrawables.push_back(std::make_unique<Drawable>(mesh.get(), &primitive, rendererMaterial));
			m_AllDrawables.back()->PerObjectCbIndex = PerObjectCbIndex++;
		}
	}
}

void ToyDX::Renderer::Terminate()
{
}