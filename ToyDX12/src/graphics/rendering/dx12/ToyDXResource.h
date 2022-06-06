#pragma once

namespace ToyDX
{
	struct Resource
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
		ID3D12Resource* GetResourcePtr() { return pResource.Get(); }

		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUDescriptor;
		D3D12_RESOURCE_STATES  CurrentState;
	};
}
