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

	// Set viewport and scissor rectangle
	SetViewport(m_BackBufferWidth, m_BackBufferWidth);
	SetScissorRectangle(m_BackBufferWidth, m_BackBufferWidth);
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

	// CreateCommandList1 to create a command list in a closed state
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device4-createcommandlist1
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateCommandList1(0, st_CmdQueueDesc.Type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(mp_CommandList.GetAddressOf())));

	LOG_INFO("DX12Pipeline: Created command objects.");
}

void DX12RenderingPipeline::CreateFence()
{
	m_CurrentFenceValue = 0;
	ThrowIfFailed(mp_TDXDevice->GetDevice().CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(mp_Fence.GetAddressOf())));
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
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mp_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (unsigned int i = 0; i < ui_BufferCount; i++)
	{
		// Get ith buffer in swapchain
		ThrowIfFailed(mp_SwapChain->GetBuffer(i, IID_PPV_ARGS(&mp_SwapChainBuffers[i])));

		// Create render target view to it
		device.CreateRenderTargetView(mp_SwapChainBuffers[i].Get(),
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

void DX12RenderingPipeline::SetViewport(UINT ui_Width, UINT ui_Height)
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(ui_Width);
	viewport.Height = static_cast<float>(ui_Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	mp_CommandList->RSSetViewports(1, &viewport);

	LOG_INFO("DX12Pipeline: Set Viewport.");
}

//*********************************************************

void DX12RenderingPipeline::SetScissorRectangle(UINT ui_Width, UINT ui_Height)
{
	D3D12_RECT scissorRect;
	scissorRect = { 0, 0, static_cast<LONG>(ui_Width), static_cast<LONG>(ui_Height) };
	mp_CommandList->RSSetScissorRects(1, &scissorRect);

	LOG_INFO("DX12Pipeline: Set Scissor Rectangle.");
}

//*********************************************************

void DX12RenderingPipeline::TransitionResource(ToyDX::Resource& st_Resource, D3D12_RESOURCE_STATES e_stateBefore, D3D12_RESOURCE_STATES e_stateAfter)
{
	if (e_stateBefore != e_stateAfter)
	{
		CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(st_Resource.GetResourcePtr(), e_stateBefore, e_stateAfter);
		mp_CommandList->ResourceBarrier(1, &transitionBarrier);
		st_Resource.CurrentState = e_stateAfter;
	}
}

void DX12RenderingPipeline::ResetCommandList()
{
}

// Forces the CPU to wait until the GPU has finished processing all the commands in the queue
void DX12RenderingPipeline::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_CurrentFenceValue++;

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won’t be
	// set until the GPU finishes processing all the commands prior to
	// this Signal().
	ThrowIfFailed(mp_CommandQueue->Signal(mp_Fence.Get(), m_CurrentFenceValue));

	// Wait until the GPU has completed commands up to this fence point.
	if (mp_Fence->GetCompletedValue() < m_CurrentFenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.
		ThrowIfFailed(mp_Fence->SetEventOnCompletion(m_CurrentFenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
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

