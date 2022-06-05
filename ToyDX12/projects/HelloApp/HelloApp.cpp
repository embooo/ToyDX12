#include "pch.h"

#include "HelloApp.h"

HelloApp::HelloApp(HINSTANCE hInstance, const char* sz_DebugName)
	: DX12App(hInstance), m_DebugName(sz_DebugName)
{

}

void HelloApp::Init()
{
	// Initialize basic functionalities : Window, Direct3D...
	if (!DX12App::Initialize())
	{
		return;
	}

	LOG_INFO("{0}::Init() Hello !", m_DebugName);
}

void HelloApp::Update()
{
}

void HelloApp::Draw()
{
	ID3D12CommandAllocator& rst_CommandAllocator = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList   = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue		 = DX12RenderingPipeline::GetCommandQueue();

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished
	// execution on the GPU.
	ThrowIfFailed(rst_CommandAllocator.Reset());

	// A command list can be reset after it has been added to the
	// command queue via ExecuteCommandList. Reusing the command list reuses memory.
	ThrowIfFailed(rst_CommandList.Reset(&rst_CommandAllocator, nullptr));

	// Indicate a state transition on the resource usage.
	ID3D12Resource* backBufferResource = mp_DX12RenderingPipeline->GetCurrentBackBuffer();
	ID3D12Resource* depthStencilResource = mp_DX12RenderingPipeline->GetDepthStencil();
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = mp_DX12RenderingPipeline->GetCurrentBackBufferView();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	rst_CommandList.ResourceBarrier(1, &barrier);


	// Set the viewport and scissor rect. This needs to be reset
	// whenever the command list is reset.
	rst_CommandList.RSSetViewports(1, &mp_DX12RenderingPipeline->GetViewport());
	rst_CommandList.RSSetScissorRects(1, &mp_DX12RenderingPipeline->GetScissorRect());

	// Clear the back buffer and depth buffer.
	rst_CommandList.ClearRenderTargetView(backBufferView, mp_DX12RenderingPipeline->GetClearValues().Color, 0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = mp_DX12RenderingPipeline->GetDepthStencilView();

	rst_CommandList.ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		mp_DX12RenderingPipeline->GetClearValues().DepthStencil.Depth,
		mp_DX12RenderingPipeline->GetClearValues().DepthStencil.Stencil,
		0, nullptr); // Clear entire render target

	// Specify the buffers we are going to render to.
	rst_CommandList.OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	// Indicate a state transition on the resource usage.
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	rst_CommandList.ResourceBarrier(1, &barrier);


	// Stop recording commands
	ThrowIfFailed(rst_CommandList.Close());

	// Add command list to GPU command queue for execution
	ID3D12CommandList* a_CommandLists[] = { &rst_CommandList };
	rst_CommandQueue.ExecuteCommandLists(_countof(a_CommandLists), a_CommandLists);

	// Present then swap front and back buffer
	ThrowIfFailed(mp_DX12RenderingPipeline->GetSwapChain()->Present(0, 0)); // SyncInterval = 0  : V-Sync disabled
	mp_DX12RenderingPipeline->SetCurrentBackBufferIndex((mp_DX12RenderingPipeline->CurrentBackBufferIndex() + 1) % mp_DX12RenderingPipeline->SwapChainBufferCount());

	// Wait until frame commands are complete.
	DX12RenderingPipeline::FlushCommandQueue();
}

void HelloApp::Terminate()
{
}

HelloApp::~HelloApp()
{
}