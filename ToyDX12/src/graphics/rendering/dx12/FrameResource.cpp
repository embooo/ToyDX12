#include "pch.h"

#include "FrameResource.h"
#include "TDXRenderer.h"
#include "DX12RenderingPipeline.h"

ToyDX::FrameResource::FrameResource(ID3D12Device* p_Device, UINT ui_NumPasses, UINT ui_NumObjects, UINT ui_NumMaterials)
	: deviceHandle(p_Device)
{
	ThrowIfFailed(p_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAllocator.GetAddressOf())));

	cbPerPass   = std::make_unique<UploadBuffer>();
	cbPerObject = std::make_unique<UploadBuffer>();
	cbMaterial  = std::make_unique<UploadBuffer>();

	cbPerObject->Create(DX12RenderingPipeline::GetDevice(), ui_NumObjects, sizeof(PerObjectData), true, L"PerObject");
	cbPerPass->Create(DX12RenderingPipeline::GetDevice(), 1, sizeof(PerPassData), true, L"PerPass");
	cbMaterial->Create(DX12RenderingPipeline::GetDevice(), ui_NumMaterials, sizeof(MaterialConstants), true, L"PerMaterial");
}
