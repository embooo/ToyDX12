#include "pch.h"

#include "DX12RenderingPipeline.h"
#include "Window.h"

void DX12RenderingPipeline::Init()
{
}

void DX12RenderingPipeline::Init(const Win32Window& p_Window)
{
	// https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d12/creating-a-basic-direct3d-12-component.md#initialize

	//Create the device / Enable the debug layer
	mp_DX12Device = std::make_unique<DX12Device>();
	mp_DX12Device->Init();

	// Create the command objects (command queue/list/allocator)
	CreateCommandObjects();

		// Create descriptors for the render targets in the swapchain and for the depth-stencil
	mp_RTVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_NumSwapChainBuffers);
	mp_DSVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	// Create the swap chain
	CreateSwapchain(Win32Window::GetHWND(), p_Window.GetHeight(), p_Window.GetWidth());
	
	// Cache descriptor sizes for later usage
	m_CachedValues.descriptorSizes.RTV = mp_DX12Device->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_CachedValues.descriptorSizes.DSV = mp_DX12Device->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CachedValues.descriptorSizes.CBV_SRV_UAV = mp_DX12Device->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	

}

//*********************************************************

void DX12RenderingPipeline::CreateCommandObjects()
{
	mp_CommandQueue.Reset();
	mp_CommandList.Reset();
	mp_CommandAllocator.Reset();

	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_COMMAND_QUEUE_DESC st_CmdQueueDesc = {};
	st_CmdQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
	st_CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	ThrowIfFailed(mp_DX12Device->GetDevice().CreateCommandQueue(&st_CmdQueueDesc, IID_PPV_ARGS(&mp_CommandQueue)));
	ThrowIfFailed(mp_DX12Device->GetDevice().CreateCommandAllocator(st_CmdQueueDesc.Type, IID_PPV_ARGS(&mp_CommandAllocator)));
	ThrowIfFailed(mp_DX12Device->GetDevice().CreateCommandList(0, st_CmdQueueDesc.Type, mp_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(mp_CommandList.GetAddressOf())));

	mp_CommandList->Close();

	LOG_INFO("DX12Pipeline: Created command objects.");
}

//*********************************************************

void DX12RenderingPipeline::CheckMSAASupport()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevelsDesc;
	qualityLevelsDesc.Format = m_BackBufferFormat;
	qualityLevelsDesc.SampleCount = 4;
	qualityLevelsDesc.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevelsDesc.NumQualityLevels = 0;
	ThrowIfFailed(mp_DX12Device->GetDevice().CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevelsDesc,
		sizeof(qualityLevelsDesc)));
	m_Msaa4XQuality = qualityLevelsDesc.NumQualityLevels;
	assert(m_Msaa4XQuality > 0 && "Unexpected MSAA quality level.");
}

//*********************************************************

void DX12RenderingPipeline::CreateSwapchain(HWND hWnd,  UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format, UINT ui_BufferCount)
{
	ID3D12Device& device = mp_DX12Device->GetDevice();

	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC st_SwapChainDesc = {};

	// Backbuffer properties
	st_SwapChainDesc.BufferDesc = {};
	st_SwapChainDesc.BufferDesc.Width = ui_Width;
	st_SwapChainDesc.BufferDesc.Height = ui_Height;
	st_SwapChainDesc.BufferDesc.Format = e_Format;
	st_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	st_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	st_SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	// Sampling properties
	st_SwapChainDesc.SampleDesc.Count = 1; //sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	st_SwapChainDesc.SampleDesc.Quality = 0;	//sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;

	// Swapchain usage : render to backbuffer
	st_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Number of buffers in the swapchain : double buffering by default
	st_SwapChainDesc.BufferCount = ui_BufferCount;

	// Handle to the window we are rendering into
	st_SwapChainDesc.OutputWindow = hWnd;

	// Fullscreen or windowed : windowed by default
	st_SwapChainDesc.Windowed = true;

	// State of the backbuffer after a call to Present
	st_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Additional flags
	// Choose the appropriate display mode when switching to Fullscreen mode 
	st_SwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	mp_DX12Device->GetFactory().CreateSwapChain(mp_CommandQueue.Get(), &st_SwapChainDesc, &mp_SwapChain);
	m_BackBufferFormat = e_Format;

	LOG_INFO("DX12Pipeline: Created swapchain.");

	//-----------------------------------------------------------------------
	// Also create render target views to each buffer in the swapchain
	//-----------------------------------------------------------------------
	ComPtr<ID3D12Resource> SwapChainBuffers[s_NumSwapChainBuffers];

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mp_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < ui_BufferCount; i++)
	{
		// Get ith buffer in swapchain
		ThrowIfFailed(mp_SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffers[i])));

		// Create render target view to it
		device.CreateRenderTargetView(SwapChainBuffers[i].Get(),
			nullptr, rtvHeapHandle);

		m_SwapChainRTViews[i] = rtvHeapHandle;

		// Offset to next descriptor
		rtvHeapHandle.Offset(1, m_CachedValues.descriptorSizes.RTV);
	}
}

//*********************************************************

ComPtr<ID3D12DescriptorHeap> DX12RenderingPipeline::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE e_Type, UINT ui_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS e_Flags, UINT ui_NodeMask)
{
	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = ui_NumDescriptors;
	descriptorHeapDesc.Type = e_Type;
	descriptorHeapDesc.Flags = e_Flags;
	descriptorHeapDesc.NodeMask = ui_NodeMask;
	 
	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	ThrowIfFailed(mp_DX12Device->GetDevice().CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(pDescriptorHeap.GetAddressOf())));

	return pDescriptorHeap;
}

//*********************************************************

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderingPipeline::GetCurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mp_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),// Handle to start of descriptor heap
		m_CurrentBackBuffer, // Index of the RTV in heap
		m_CachedValues.descriptorSizes.RTV); // Size (in bytes) of a RTV descriptor
}

//*********************************************************

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderingPipeline::GetDepthStencilView() const
{
	return mp_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

//*********************************************************

void DX12RenderingPipeline::Terminate()
{
}

