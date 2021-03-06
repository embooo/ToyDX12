#pragma once

#include "IDevice.h"
#include "dxgi1_6.h"

using namespace Microsoft::WRL;

class DX12Device : public IDevice
{
public:
	DX12Device() = default;

	void Init() override;
	void Terminate() override;
	void EnableDebugLayer();
	void EnableShaderBasedValidation(); // GPU-based validation
	void CheckRaytracingSupport();

	UINT GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE e_Type) const;

	ID3D12Device5& GetDevice() const { return *mp_Device.Get(); }
	IDXGIFactory6& GetFactory() const { return *mp_DXGIFactory6.Get(); }

	~DX12Device() override = default;
protected:
	ComPtr<ID3D12Device5> mp_Device;
	ComPtr<IDXGIFactory6> mp_DXGIFactory6;
};

struct DX12Descriptor
{
	UINT ui_Size;
	D3D12_DESCRIPTOR_HEAP_TYPE e_Type;
};

