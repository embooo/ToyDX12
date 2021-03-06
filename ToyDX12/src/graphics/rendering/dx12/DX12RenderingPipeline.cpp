#include "pch.h"

#include "DX12RenderingPipeline.h"
#include "ToyDXUploadBuffer.h"
#include "Window.h"

#include <d3d12.h>
#include "DirectXTex.h"

std::unique_ptr<DX12Device>			DX12RenderingPipeline::s_TDXDevice;
UINT64								DX12RenderingPipeline::s_CurrentFenceValue;

ComPtr<ID3D12Fence>					DX12RenderingPipeline::s_Fence;
ComPtr<ID3D12GraphicsCommandList>	DX12RenderingPipeline::s_CommandList;
ComPtr<ID3D12CommandQueue>			DX12RenderingPipeline::s_CommandQueue;
ComPtr<ID3D12CommandAllocator>		DX12RenderingPipeline::s_CmdAllocator;
DXGI_SAMPLE_DESC					DX12RenderingPipeline::s_SampleDesc;

UINT								DX12RenderingPipeline::CBV_SRV_UAV_Size;
UINT								DX12RenderingPipeline::RTV_Size;
UINT								DX12RenderingPipeline::DSV_Size;
UINT								DX12RenderingPipeline::SMP_Size;

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
	RTV_Size			= s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSV_Size			= s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CBV_SRV_UAV_Size	= s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SMP_Size	    = s_TDXDevice->GetDescriptorSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	// Create the swap chain and depth stencil
	CreateSwapchain(Win32Window::GetHWND(), p_Window.GetWidth(), p_Window.GetHeight());
	CreateDepthStencil(m_BackBufferWidth, m_BackBufferHeight);

	// Set viewport and scissor rectangle
	SetViewport(m_BackBufferWidth, m_BackBufferHeight);
	SetScissorRectangle(m_BackBufferWidth, m_BackBufferHeight);

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

	s_SampleDesc = { st_SwapChainDesc.SampleDesc.Count, st_SwapChainDesc.SampleDesc.Quality };

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


	// Clear values
	RTClearValues.Format = e_Format;
	RTClearValues.Color[0] = 0.87;
	RTClearValues.Color[1] = 0.87;
	RTClearValues.Color[2] = 0.87;
	RTClearValues.Color[3] = 1.0f;

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
		rtvHeapHandle.Offset(1, RTV_Size);
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
	depthStencilDesc.SampleDesc			 = s_SampleDesc;
	depthStencilDesc.Layout				 = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags			     = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	
	// Optimized clear values for clear calls
	DSClearValues.Format			   = e_Format;
	DSClearValues.DepthStencil.Depth   = 1.0f;
	DSClearValues.DepthStencil.Stencil = 0;
	//ClearValues.Color[0] = 0.1f;
	//ClearValues.Color[1] = 0.1f;
	//ClearValues.Color[2] = 0.1f;
	//ClearValues.Color[3] = 1.0f;

	// Heap that the resource will be commited to
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // Depth-Stencil is only read/written to by the GPU 
	

	//-----------------------------------------------------------------------
	// Creation
	//-----------------------------------------------------------------------
	
	// Create Depth-Stencil resource
	ThrowIfFailed(s_TDXDevice->GetDevice().CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &DSClearValues, IID_PPV_ARGS(mp_DepthStencil.GetAddressOf())));
	mp_DepthStencil->SetName(L"Depth-Stencil Resource");

	// Create Depth-Stencil View
	m_DepthStencilView = CD3DX12_CPU_DESCRIPTOR_HANDLE(mp_DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	
	s_TDXDevice->GetDevice().CreateDepthStencilView(mp_DepthStencil.Get(), nullptr, m_DepthStencilView);
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
	// Because we are on the GPU timeline, the new fence point won?t be
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

ID3D12Resource* DX12RenderingPipeline::GetCurrentBackBuffer() const
{
	return mp_SwapChainBuffers[m_iCurrentBackBuffer].Get();
}

ID3D12Resource* DX12RenderingPipeline::GetDepthStencil() const
{
	return mp_DepthStencil.Get();
}



//*********************************************************

ComPtr<ID3D12Resource> DX12RenderingPipeline::CreateDefaultBuffer(const void* pData, UINT64 ui64_SizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& p_UploadBuffer, D3D12_RESOURCE_STATES e_ResourceState)
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
	UpdateSubresources<1>(&DX12RenderingPipeline::GetCommandList(), p_DefaultBuffer.Get(), p_UploadBuffer.Get(), 0, 0, 1, &subResourceDataDesc);

	CD3DX12_RESOURCE_BARRIER barriers[1] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(p_DefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, e_ResourceState)
	};

	DX12RenderingPipeline::GetCommandList().ResourceBarrier(_countof(barriers), barriers);

	// The upload buffer has to be kept alive at least until the UpdateSubresources command is executed in the command queue
	// After that, the Upload buffer can be safely released
	return p_DefaultBuffer;
}

//*********************************************************

D3D12_VERTEX_BUFFER_VIEW DX12RenderingPipeline::CreateVertexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource>& p_VertexBufferGPU, UINT64 ui_SizeInBytes, UINT64 ui_StrideInBytes)
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

void DX12RenderingPipeline::CreateConstantBufferView(ID3D12DescriptorHeap* st_CbvHeap, D3D12_GPU_VIRTUAL_ADDRESS ui64_CbvAddress, UINT ui_SizeInBytes)
{
	// Creates a constant buffer descriptor in the given descriptor heap
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = ui64_CbvAddress;
	cbvDesc.SizeInBytes = ui_SizeInBytes;

	D3D12_CPU_DESCRIPTOR_HANDLE cbvDescriptor;

	DX12RenderingPipeline::GetDevice()->CreateConstantBufferView(&cbvDesc, st_CbvHeap->GetCPUDescriptorHandleForHeapStart());
}

ComPtr<ID3D12PipelineState> DX12RenderingPipeline::CreatePipelineStateObject(
	ID3D12RootSignature* p_RootSignature,
	ID3DBlob*		  st_VsByteCode,
	ID3DBlob*		  st_PsByteCode,
	D3D12_INPUT_LAYOUT_DESC		  st_InputLayout,
	DXGI_FORMAT					  a_RtvFormats[8],
	CD3DX12_RASTERIZER_DESC		  st_RasterizerState,
	UINT						  ui_NumRenderTargets,
	DXGI_SAMPLE_DESC			  st_SampleDesc,
	DXGI_FORMAT					  e_DsvFormat,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE st_PrimitiveType)
{
	D3D12_DEPTH_STENCIL_DESC depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};

	pipelineStateDesc.InputLayout = st_InputLayout;
	pipelineStateDesc.pRootSignature = p_RootSignature;
	pipelineStateDesc.VS = { (BYTE*)st_VsByteCode->GetBufferPointer(), st_VsByteCode->GetBufferSize() };
	pipelineStateDesc.PS = { (BYTE*)st_PsByteCode->GetBufferPointer(), st_PsByteCode->GetBufferSize() };
	pipelineStateDesc.RasterizerState = st_RasterizerState;
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineStateDesc.DepthStencilState = depthStencilState;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = a_RtvFormats[0];
	pipelineStateDesc.DSVFormat  = e_DsvFormat;
	pipelineStateDesc.SampleDesc = st_SampleDesc;
	
	//.StreamOutput = 0,
	//.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT), // Default blend state for now
	//.SampleMask = 0xFFFFFFFF,
	//.RasterizerState = st_RasterizerState,
	//.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT), // Default depth-stencil state for now
	//.InputLayout = st_InputLayout,
	//.PrimitiveTopologyType = st_PrimitiveType,
	//.NumRenderTargets = ui_NumRenderTargets,
	//.RTVFormats = *a_RtvFormats,
	//.DSVFormat = e_DsvFormat,
	//.SampleDesc = st_SampleDesc

	ComPtr<ID3D12PipelineState> pso;
	ThrowIfFailed(DX12RenderingPipeline::GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pso.GetAddressOf())));

	return pso;
}

void DX12RenderingPipeline::CreateTexture2D(UINT64 ui_Width, UINT ui_Height, UINT ui_Channels, DXGI_FORMAT e_Format, unsigned char* data, ComPtr<ID3D12Resource>& m_texture, ComPtr<ID3D12Resource>& textureUploadHeap, const std::wstring& debugName)
{
	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.


	ID3D12Device* p_Device = DX12RenderingPipeline::GetDevice();
	ID3D12GraphicsCommandList* p_CmdList = &DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue* p_CmdQueue = &DX12RenderingPipeline::GetCommandQueue();
	ID3D12CommandAllocator* p_CmdAlloc = &DX12RenderingPipeline::GetCommandAllocator();
	ThrowIfFailed(p_CmdList->Reset(p_CmdAlloc, nullptr));

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = e_Format;
		textureDesc.Width = ui_Width;
		textureDesc.Height = ui_Height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ThrowIfFailed(p_Device->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_texture)));

		m_texture->SetName(debugName.c_str());

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

		CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		// Create the GPU upload buffer.
		ThrowIfFailed(p_Device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = &data[0];
		textureData.RowPitch = ui_Width * ui_Channels;
		textureData.SlicePitch = textureData.RowPitch * ui_Height;

		CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		UpdateSubresources(p_CmdList, m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
		p_CmdList->ResourceBarrier(1, &transitionBarrier);
	}

	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(p_CmdList->Close());
	ID3D12CommandList* ppCommandLists[] = { p_CmdList };
	p_CmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	FlushCommandQueue();

}
//*********************************************************

void DX12RenderingPipeline::Terminate()
{
}

