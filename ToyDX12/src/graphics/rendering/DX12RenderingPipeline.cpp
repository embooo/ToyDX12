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
	mp_TDXDevice = std::make_unique<DX12Device>();
	mp_TDXDevice->Init();

	// Create the command objects (command queue/list/allocator)
	CreateCommandObjects();

	// Create descriptors for the render targets in the swapchain and for the depth-stencil
	mp_RTVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_NumSwapChainBuffers);
	mp_DSVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	// Cache descriptor sizes for later usage
	m_CachedValues.descriptorSizes.RTV = mp_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_CachedValues.descriptorSizes.DSV = mp_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CachedValues.descriptorSizes.CBV_SRV_UAV = mp_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create the swap chain and depth stencil
	CreateSwapchain(Win32Window::GetHWND(), p_Window.GetWidth(), p_Window.GetHeight());
	CreateDepthStencil(m_BackBufferWidth, m_BackBufferHeight);
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
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateCommandQueue(&st_CmdQueueDesc, IID_PPV_ARGS(&mp_CommandQueue)));
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateCommandAllocator(st_CmdQueueDesc.Type, IID_PPV_ARGS(&mp_CommandAllocator)));
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateCommandList(0, st_CmdQueueDesc.Type, mp_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(mp_CommandList.GetAddressOf())));

	ThrowIfFailed(mp_CommandList->Close());

	LOG_INFO("DX12Pipeline: Created command objects.");
}

//*********************************************************

void DX12RenderingPipeline::CheckMSAASupport()
{
	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevelsDesc;
	qualityLevelsDesc.Format = m_BackBufferFormat;
	qualityLevelsDesc.SampleCount = 4;
	qualityLevelsDesc.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevelsDesc.NumQualityLevels = 0;

	//-----------------------------------------------------------------------

	ThrowIfFailed(mp_TDXDevice->GetDevice().CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevelsDesc,
		sizeof(qualityLevelsDesc)));

	assert(qualityLevelsDesc.NumQualityLevels > 0 && "Unexpected MSAA quality level.");

	m_Msaa4xQuality = qualityLevelsDesc.NumQualityLevels;
}

//*********************************************************

void DX12RenderingPipeline::CreateSwapchain(HWND hWnd,  UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format, UINT ui_BufferCount)
{
	ID3D12Device& device = mp_TDXDevice->GetDevice();

	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC st_SwapChainDesc = {};

	// Backbuffer properties
	st_SwapChainDesc.BufferDesc = {};
	st_SwapChainDesc.BufferDesc.Width  = ui_Width;
	st_SwapChainDesc.BufferDesc.Height = ui_Height;
	st_SwapChainDesc.BufferDesc.Format = e_Format;
	st_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	st_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	st_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	st_SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	m_BackBufferWidth  = ui_Width;
	m_BackBufferHeight = ui_Height;

	// Sampling properties
	st_SwapChainDesc.SampleDesc.Count = m_bUse4xMsaa ? 4 : 1; //sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	st_SwapChainDesc.SampleDesc.Quality = m_bUse4xMsaa ? (m_Msaa4xQuality - 1) : 0;	//sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;

	m_SampleDesc = { st_SwapChainDesc.SampleDesc.Count, st_SwapChainDesc.SampleDesc.Quality };

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
	mp_TDXDevice->GetFactory().CreateSwapChain(mp_CommandQueue.Get(), &st_SwapChainDesc, &mp_SwapChain);
	m_BackBufferFormat = e_Format;

	LOG_INFO("DX12Pipeline: Created swapchain.");

	//-----------------------------------------------------------------------
	// Also create render target views to each buffer in the swapchain
	//-----------------------------------------------------------------------
	ComPtr<ID3D12Resource> SwapChainBuffers[s_NumSwapChainBuffers];
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mp_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (unsigned int i = 0; i < ui_BufferCount; i++)
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

ComPtr<ID3D12DescriptorHeap> DX12RenderingPipeline::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE e_Type, UINT ui_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS e_Flags)
{
	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = ui_NumDescriptors;
	descriptorHeapDesc.Type = e_Type;
	descriptorHeapDesc.Flags = e_Flags;
	descriptorHeapDesc.NodeMask = 0;
	 
	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(pDescriptorHeap.GetAddressOf())));

	return pDescriptorHeap;
}

//*********************************************************

void DX12RenderingPipeline::CreateDepthStencil(UINT ui_Width, UINT ui_Height, UINT u_mipLevels, DXGI_FORMAT e_Format)
{
	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension			 = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment			 = 0;
	depthStencilDesc.Width				 = ui_Width;
	depthStencilDesc.Height				 = ui_Height;
	depthStencilDesc.DepthOrArraySize    = 1;
	depthStencilDesc.MipLevels			 = u_mipLevels;
	depthStencilDesc.Format				 = e_Format;
	depthStencilDesc.SampleDesc			 = m_SampleDesc;
	depthStencilDesc.Layout				 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags			     = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	
	// Optimized clear value for clear calls
	D3D12_CLEAR_VALUE optClearValueDesc	   = {};
	optClearValueDesc.Format			   = e_Format;
	optClearValueDesc.DepthStencil.Depth   = 1.0f;
	optClearValueDesc.DepthStencil.Stencil = 0;

	// Heap that the resource will be commited to
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // Depth-Stencil is only read/written to by the GPU 
	

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	
	// Create Depth-Stencil resource
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClearValueDesc, IID_PPV_ARGS(mst_DepthStencil.pResource.GetAddressOf())));

	// Save current state
	mst_DepthStencil.CurrentState = D3D12_RESOURCE_STATE_COMMON;

	// Create Depth-Stencil View

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle(mp_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	mp_TDXDevice->GetDevice().CreateDepthStencilView(mst_DepthStencil.GetResourcePtr(), nullptr, dsvHeapHandle);

	mst_DepthStencil.CPUDescriptor = dsvHeapHandle;

	// Transition the resource to be used as a Depth buffer
	TransitionResource(mst_DepthStencil, mst_DepthStencil.CurrentState, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

//*********************************************************

void DX12RenderingPipeline::TransitionResource(ToyDX::Resource& st_Resource, D3D12_RESOURCE_STATES e_stateBefore, D3D12_RESOURCE_STATES e_stateAfter)
{
	assert(e_stateBefore != e_stateAfter);
	CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(st_Resource.GetResourcePtr(), e_stateBefore, e_stateAfter);

	mp_CommandList->ResourceBarrier(1, &transitionBarrier);
	st_Resource.CurrentState = e_stateAfter;
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

