#include "pch.h"

#include "DX12RenderingPipeline.h"
#include "Window.h"

std::unique_ptr<DX12Device>			DX12RenderingPipeline::s_TDXDevice;
UINT64								DX12RenderingPipeline::s_CurrentFenceValue;

ComPtr<ID3D12Fence>					DX12RenderingPipeline::s_Fence;
ComPtr<ID3D12GraphicsCommandList>	DX12RenderingPipeline::s_CommandList;
ComPtr<ID3D12CommandQueue>			DX12RenderingPipeline::s_CommandQueue;
ComPtr<ID3D12CommandAllocator>		DX12RenderingPipeline::s_CmdAllocator;

void DX12RenderingPipeline::Init()
{
}

void DX12RenderingPipeline::Init(const Win32Window& p_Window)
{
	// https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/direct3d12/creating-a-basic-direct3d-12-component.md#initialize

	//Create the device / Enable the debug layer
	s_TDXDevice = std::make_unique<DX12Device>();
	s_TDXDevice->Init();

	// Create the command objects (command queue/list/allocator)
	// Command list is created and by open by default, use CreateCommandList1 to create a closed command list
	CreateCommandObjects();
	CreateFence();

	// Create descriptors for the render targets in the swapchain and for the depth-stencil
	mp_RTVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_NumSwapChainBuffers);
	mp_DSVDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	// Cache descriptor sizes for later usage
	m_CachedValues.descriptorSizes.RTV = s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_CachedValues.descriptorSizes.DSV = s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CachedValues.descriptorSizes.CBV_SRV_UAV = s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create the swap chain and depth stencil
	CreateSwapchain(Win32Window::GetHWND(), p_Window.GetWidth(), p_Window.GetHeight());
	CreateDepthStencil(m_BackBufferWidth, m_BackBufferHeight);

	// Set viewport and scissor rectangle
	SetViewport(m_BackBufferWidth, m_BackBufferWidth);
	SetScissorRectangle(m_BackBufferWidth, m_BackBufferWidth);

	CloseCommandList();
}

//*********************************************************

void DX12RenderingPipeline::CreateCommandObjects()
{
	s_CommandQueue.Reset();
	s_CommandList.Reset();
	s_CmdAllocator.Reset();

	//-----------------------------------------------------------------------
	// Description
	//-----------------------------------------------------------------------
	D3D12_COMMAND_QUEUE_DESC st_CmdQueueDesc = {};
	st_CmdQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
	st_CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateCommandQueue(&st_CmdQueueDesc, IID_PPV_ARGS(&s_CommandQueue)));
	s_CommandQueue->SetName(L"Command Queue");
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateCommandAllocator(st_CmdQueueDesc.Type, IID_PPV_ARGS(&s_CmdAllocator)));
	s_CmdAllocator->SetName(L"Command Allocator");


	// CreateCommandList1 to create a command list in a closed state
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device4-createcommandlist1
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateCommandList(0, st_CmdQueueDesc.Type, s_CmdAllocator.Get(), nullptr, IID_PPV_ARGS(s_CommandList.GetAddressOf())));
	s_CommandList->SetName(L"Command List");

	LOG_INFO("DX12Pipeline: Created command objects.");
}

void DX12RenderingPipeline::CreateFence()
{
	s_CurrentFenceValue = 0;
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(s_Fence.GetAddressOf())));
	s_Fence->SetName(L"Fence");
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

	ThrowIfFailed(s_TDXDevice->GetDevice().CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevelsDesc,
		sizeof(qualityLevelsDesc)));

	assert(qualityLevelsDesc.NumQualityLevels > 0 && "Unexpected MSAA quality level.");

	m_Msaa4xQuality = qualityLevelsDesc.NumQualityLevels;
}

//*********************************************************

void DX12RenderingPipeline::CreateSwapchain(HWND hWnd,  UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format, UINT ui_BufferCount)
{
	ID3D12Device& device = s_TDXDevice->GetDevice();

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
	ThrowIfFailed(s_TDXDevice->GetFactory().CreateSwapChain(s_CommandQueue.Get(), &st_SwapChainDesc, &mp_SwapChain));
	m_BackBufferFormat = e_Format;
	for (unsigned int i = 0; i < ui_BufferCount; i++)
	{
		ThrowIfFailed(mp_SwapChain->GetBuffer(i, IID_PPV_ARGS(&mp_SwapChainBuffers[i])));

		mp_SwapChainBuffers[i]->SetName(L"SwapChain Buffer " + i);
	}

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
		device.CreateRenderTargetView(mp_SwapChainBuffers[i].Get(), nullptr, rtvHeapHandle);

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
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(pDescriptorHeap.GetAddressOf())));
	pDescriptorHeap->SetName(L"Descriptor Heap");

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
	
	// Optimized clear values for clear calls
	m_ClearValues = {};
	m_ClearValues.Format			   = e_Format;
	m_ClearValues.DepthStencil.Depth   = 1.0f;
	m_ClearValues.DepthStencil.Stencil = 0;
	m_ClearValues.Color[0] = 1.0f;
	m_ClearValues.Color[1] = 0.0f;
	m_ClearValues.Color[2] = 1.0f;
	m_ClearValues.Color[3] = 1.0f;

	// Heap that the resource will be commited to
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // Depth-Stencil is only read/written to by the GPU 
	

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	
	// Create Depth-Stencil resource
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &m_ClearValues, IID_PPV_ARGS(mp_DepthStencil.GetAddressOf())));
	mp_DepthStencil->SetName(L"Depth-Stencil Resource");

	// Create Depth-Stencil View
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHeapHandle(mp_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	s_TDXDevice->GetDevice().CreateDepthStencilView(mp_DepthStencil.Get(), nullptr, dsvHeapHandle);
}

//*********************************************************

void DX12RenderingPipeline::SetViewport(UINT ui_Width, UINT ui_Height)
{
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = static_cast<float>(ui_Width);
	m_Viewport.Height = static_cast<float>(ui_Height);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	s_CommandList->RSSetViewports(1, &m_Viewport);

	LOG_INFO("DX12Pipeline: Set Viewport.");
}

//*********************************************************

void DX12RenderingPipeline::SetScissorRectangle(UINT ui_Width, UINT ui_Height)
{
	m_ScissorRect = { 0, 0, static_cast<LONG>(ui_Width), static_cast<LONG>(ui_Height) };
	s_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	LOG_INFO("DX12Pipeline: Set Scissor Rectangle.");
}

//*********************************************************

void DX12RenderingPipeline::TransitionResource(ToyDX::Resource& st_Resource, D3D12_RESOURCE_STATES e_stateBefore, D3D12_RESOURCE_STATES e_stateAfter)
{
	if (e_stateBefore != e_stateAfter)
	{
		CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(st_Resource.GetResourcePtr(), e_stateBefore, e_stateAfter);
		s_CommandList->ResourceBarrier(1, &transitionBarrier);
		st_Resource.CurrentState = e_stateAfter;
	}
}

void DX12RenderingPipeline::ResetCommandList()
{
	ThrowIfFailed(s_CommandList->Reset(s_CmdAllocator.Get(), nullptr));
}

void DX12RenderingPipeline::CloseCommandList()
{
	ThrowIfFailed(s_CommandList->Close());
}

// Forces the CPU to wait until the GPU has finished processing all the commands in the queue
void DX12RenderingPipeline::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	s_CurrentFenceValue++;

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won’t be
	// set until the GPU finishes processing all the commands prior to
	// this Signal().
	ThrowIfFailed(s_CommandQueue->Signal(s_Fence.Get(), s_CurrentFenceValue));

	// Wait until the GPU has completed commands up to this fence point.
	if (s_Fence->GetCompletedValue() < s_CurrentFenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.
		ThrowIfFailed(s_Fence->SetEventOnCompletion(s_CurrentFenceValue, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

//*********************************************************

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderingPipeline::GetCurrentBackBufferView()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mp_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),// Handle to start of descriptor heap
		m_iCurrentBackBuffer, // Index of the RTV in heap
		m_CachedValues.descriptorSizes.RTV); // Size (in bytes) of a RTV descriptor
}

//*********************************************************

ID3D12Resource* DX12RenderingPipeline::GetCurrentBackBuffer() const
{
	return mp_SwapChainBuffers[m_iCurrentBackBuffer].Get();
}

ID3D12Resource* DX12RenderingPipeline::GetDepthStencil() const
{
	return mst_DepthStencil.pResource.Get();
}

//*********************************************************

D3D12_CPU_DESCRIPTOR_HANDLE DX12RenderingPipeline::GetDepthStencilView() 
{
	return mp_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

//*********************************************************

ComPtr<ID3D12Resource> DX12RenderingPipeline::CreateDefaultBuffer(const void* pData, UINT64 ui64_SizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& p_UploadBuffer)
{
	// To create a buffer in the Default Heap that can only be accessed by the GPU
	// We first need to store the data into an Upload Buffer in the Upload Heap accessible by the CPU
	// and then upload the data to the Default Buffer

	//*********************************************************
	//	Default Buffer Resource creation
	//*********************************************************

	ComPtr<ID3D12Resource> p_DefaultBuffer;
	CD3DX12_HEAP_PROPERTIES defaultHeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(ui64_SizeInBytes);
	ThrowIfFailed(DX12RenderingPipeline::GetDevice()->CreateCommittedResource(
		&defaultHeapProperty, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(p_DefaultBuffer.GetAddressOf())));

	//*********************************************************
	//	Upload Buffer Resource creation 
	//*********************************************************
	CD3DX12_HEAP_PROPERTIES uploadHeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	
	ThrowIfFailed(DX12RenderingPipeline::GetDevice()->CreateCommittedResource(
		&uploadHeapProperty, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(p_UploadBuffer.GetAddressOf())));

	//**************************************************************
	//	Description of the data to be copied into the default buffer 
	//**************************************************************

	D3D12_SUBRESOURCE_DATA subResourceDataDesc = {};
	subResourceDataDesc.pData = pData;
	subResourceDataDesc.RowPitch = ui64_SizeInBytes;
	subResourceDataDesc.RowPitch = ui64_SizeInBytes;

	//	Schedule the copy to the default buffer, by using the upload buffer as an intermediate buffer
	UpdateSubresources<1>(&DX12RenderingPipeline::GetCommandList(),
		p_DefaultBuffer.Get(), p_UploadBuffer.Get(),
		0, 0, 1, &subResourceDataDesc);

	// The upload buffer has to be kept alive at least until the UpdateSubresources command is executed in the command queue
	// After that, the Upload buffer can be safely released
	return p_DefaultBuffer;
}

//*********************************************************

D3D12_VERTEX_BUFFER_VIEW DX12RenderingPipeline::CreateVertexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource>& p_VertexBufferGPU, UINT ui_SizeInBytes, UINT ui_StrideInBytes)
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewDesc = {};
	vertexBufferViewDesc.BufferLocation = p_VertexBufferGPU.Get()->GetGPUVirtualAddress();
	vertexBufferViewDesc.SizeInBytes    = ui_SizeInBytes;	// Size in bytes of the vertex buffer
	vertexBufferViewDesc.StrideInBytes  = ui_StrideInBytes;	// Size in bytes of a single vertex

	return vertexBufferViewDesc;
}

//*********************************************************

D3D12_INDEX_BUFFER_VIEW DX12RenderingPipeline::CreateIndexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource>& p_IndexBufferGPU, UINT ui_SizeInBytes, DXGI_FORMAT e_Format)
{
	D3D12_INDEX_BUFFER_VIEW indexBufferViewDesc = {};
	indexBufferViewDesc.BufferLocation = p_IndexBufferGPU.Get()->GetGPUVirtualAddress();
	indexBufferViewDesc.SizeInBytes = ui_SizeInBytes;	// Size in bytes of the index buffer
	indexBufferViewDesc.Format = e_Format;	// DXGI_FORMAT_R16_UINT or DXGI_FORMAT_R32_UINT

	return indexBufferViewDesc;
}

//*********************************************************

void DX12RenderingPipeline::Terminate()
{
}

