#pragma once

#include "IPipeline.h"
#include "DX12Device.h"
#include "DX12CachedValues.h"
#include "Window.h"

class DX12RenderingPipeline : public IPipeline
{
public:
	DX12RenderingPipeline() = default;

	void Init() override;
	void Init(const Win32Window& p_Window);
	void Terminate() override;

	void CreateCommandObjects();

	void CreateSwapchain(HWND hWnd, UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format = DXGI_FORMAT_R8G8B8A8_UNORM, UINT ui_BufferCount = s_NumSwapChainBuffers);

	void CheckMSAASupport();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE e_Type, UINT ui_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS e_Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, UINT ui_NodeMask = 0);
	~DX12RenderingPipeline() override = default;

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;
protected:
	std::unique_ptr<DX12Device> mp_DX12Device;
	ComPtr<ID3D12Fence> mp_Fence;
	ComPtr<IDXGISwapChain> mp_SwapChain;

	ComPtr<ID3D12CommandQueue> mp_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> mp_CommandList;
	ComPtr<ID3D12CommandAllocator> mp_CommandAllocator;

	ComPtr<ID3D12DescriptorHeap> mp_DescriptorHeapRTV;
	ComPtr<ID3D12DescriptorHeap> mp_DescriptorHeapDSV;

	DXGI_FORMAT m_BackBufferFormat;
	int m_Msaa4XQuality;
	static const int s_NumSwapChainBuffers = 2;
	int m_CurrentBackBuffer = 0;

	// Cached informations 
	DX12CachedValues m_CachedValues;
};

