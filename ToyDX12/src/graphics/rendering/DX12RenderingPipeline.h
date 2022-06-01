#pragma once

#include "IPipeline.h"
#include "DX12Device.h"
#include "DX12CachedValues.h"
#include "ToyDXResource.h"
#include "Window.h"

class DX12RenderingPipeline : public IPipeline
{
public:
	DX12RenderingPipeline() = default;
	~DX12RenderingPipeline() override = default;

	void Init() override;
	void Init(const Win32Window& p_Window);
	void Terminate() override;

	void CreateCommandObjects();
	void CreateFence();
	void CreateSwapchain(HWND hWnd, UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format = DXGI_FORMAT_R8G8B8A8_UNORM, UINT ui_BufferCount = s_NumSwapChainBuffers);

	void CheckMSAASupport();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE e_Type, UINT ui_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS e_Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	void CreateDepthStencil(UINT ui_Width, UINT ui_Height, UINT u_mipLevels = 1, DXGI_FORMAT e_Format = DXGI_FORMAT_D32_FLOAT);

	void SetViewport(UINT ui_Width, UINT ui_Height);
	void SetScissorRectangle(UINT ui_Width, UINT ui_Height);

	void TransitionResource(ToyDX::Resource& st_Resource, D3D12_RESOURCE_STATES e_stateBefore, D3D12_RESOURCE_STATES e_stateAfter);

	ID3D12Device* GetDevice() const { return &mp_TDXDevice->GetDevice(); }

	void ResetCommandList();
	void FlushCommandQueue();

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;

protected:
	// Device
	std::unique_ptr<DX12Device> mp_TDXDevice;

	// Fence
	ComPtr<ID3D12Fence> mp_Fence;
	UINT64 m_CurrentFenceValue;

	// Command objects
	ComPtr<ID3D12CommandQueue> mp_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> mp_CommandList;
	ComPtr<ID3D12CommandAllocator> mp_CommandAllocator;

	// Descriptor heaps
	ComPtr<ID3D12DescriptorHeap> mp_RTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mp_DSVDescriptorHeap;

	// SwapChain
	static const int s_NumSwapChainBuffers = 2;
	ComPtr<IDXGISwapChain> mp_SwapChain;
	ComPtr<ID3D12Resource> mp_SwapChainBuffers[s_NumSwapChainBuffers];
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_SwapChainRTViews[s_NumSwapChainBuffers];

	int  m_CurrentBackBuffer = 0;
	bool m_bUse4xMsaa = false;
	UINT m_Msaa4xQuality	= 0;
	UINT m_BackBufferWidth  = 0;
	UINT m_BackBufferHeight = 0;
	DXGI_FORMAT m_BackBufferFormat;
	DXGI_SAMPLE_DESC m_SampleDesc;

	// Depth-Stencil
	ToyDX::Resource mst_DepthStencil;

	// Cached informations 
	DX12CachedValues m_CachedValues;
};


