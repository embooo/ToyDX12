#include "pch.h"

#include "DX12Device.h"

void DX12Device::Init()
{
	// Optionally enable debug layer 
#if defined(DEBUG_BUILD)
	EnableDebugLayer();
#endif

	// Try to create physical device
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mp_DXGIFactory6)));
	HRESULT hardwareDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mp_Device));

	// Fallback to software device WARP
	if (FAILED(hardwareDeviceResult))
	{
		ComPtr<IDXGIAdapter> p_WarpAdapter;
		ThrowIfFailed(mp_DXGIFactory6->EnumWarpAdapter(IID_PPV_ARGS(&p_WarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(p_WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&p_WarpAdapter)));
		LOG_INFO("DX12Device: Created software device.");
	}
	else
	{
		LOG_INFO("DX12Device: Created hardware device.");
	}
}

//*********************************************************

void DX12Device::Terminate()
{
}

//*********************************************************

void DX12Device::EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
	
	LOG_INFO("DX12Device: Enabled debug layer.");
}

UINT DX12Device::GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE e_Type) const
{
	return mp_Device->GetDescriptorHandleIncrementSize(e_Type);
}
