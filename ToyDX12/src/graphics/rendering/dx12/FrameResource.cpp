#include "pch.h"

#include "FrameResource.h"

ToyDX::FrameResource::FrameResource(ID3D12Device* p_Device, UINT ui_NumPasses, UINT ui_NumObjects)
	: deviceHandle(p_Device)
{
	ThrowIfFailed(p_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdListAllocator.GetAddressOf())));

	cbPerPass   = std::make_unique<UploadBuffer>();
	cbPerObject = std::make_unique<UploadBuffer>();
}
