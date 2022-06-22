#include "pch.h"

#include "HelloApp.h"

#include "MathUtil.h"

#include "TDXRenderer.h"

#include "TDXMesh.h"

using namespace DirectX;

HelloApp::HelloApp(HINSTANCE hInstance, const char* sz_DebugName)
	: DX12App(hInstance), m_DebugName(sz_DebugName), m_DeltaTime(0.0)
{

}

//*********************************************************



//*********************************************************

void HelloApp::Init()
{
	// Initialize basic functionalities : Window, Direct3D...
	if (!DX12App::Initialize())
	{
		return;
	}

	// Camera initialization
	m_Camera.Init({ .fAspectRatio = DX12App::GetWindowAspectRatio(), .fFovY = DirectX::XMConvertToRadians(45.0f), .fNearZ = 1.0f,.fFarZ = 1000.0f,});
	
	// Renderer
	m_Renderer = std::make_unique<ToyDX::Renderer>();
	m_Renderer->Initialize();
	
	LOG_INFO("{0}::Init() Hello !", m_DebugName);
}

//*********************************************************

void HelloApp::Update(double deltaTime)
{
	m_DeltaTime = deltaTime;
	UpdateCamera(m_DeltaTime);

	// Update constant buffer storing WVP matrix
	PerObjectData constantsBufferData;
	DirectX::XMStoreFloat4x4(&constantsBufferData.gWorld, XMMatrixTranspose(m_Renderer->m_Mesh->GetWorldMatrix()));
	DirectX::XMStoreFloat4x4(&constantsBufferData.gView,  XMMatrixTranspose(m_Camera.GetViewMatrix()));
	DirectX::XMStoreFloat4x4(&constantsBufferData.gProj,  XMMatrixTranspose(m_Camera.GetProjMatrix()));
	m_Renderer->GetConstantBuffer()->CopyData(0, &constantsBufferData, sizeof(PerObjectData));
}

//*********************************************************

void HelloApp::Draw(double deltaTime)
{
	ID3D12CommandAllocator& rst_CommandAllocator = DX12RenderingPipeline::GetCommandAllocator();
	ID3D12GraphicsCommandList& rst_CommandList = DX12RenderingPipeline::GetCommandList();
	ID3D12CommandQueue& rst_CommandQueue = DX12RenderingPipeline::GetCommandQueue();

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished
	// execution on the GPU.
	ThrowIfFailed(rst_CommandAllocator.Reset());
	
	// A command list can be reset after it has been added to the
	// command queue via ExecuteCommandList. Reusing the command list reuses memory.
	ThrowIfFailed(rst_CommandList.Reset(&rst_CommandAllocator, m_Renderer->GetPipelineState()));

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
		1.0,
		0,
		0, nullptr); // Clear entire render target

	// Specify the buffers we are going to render to.
	rst_CommandList.OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

	{
		m_Renderer->BindResources();
		m_Renderer->Render();
	}

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

//*********************************************************

void HelloApp::OnResize(LPARAM lParam)
{
	m_Camera.SetFrustum({ .fAspectRatio = GetWindowAspectRatio() });
	m_Camera.UpdateProjMatrix();
}


void HelloApp::OnMouseMove(WPARAM buttonState, int xPos, int yPos)
{
	if (buttonState & MK_LBUTTON)
	{
		if (bFirstClick)
		{
			m_LastClickPosX = xPos;
			m_LastClickPosY = yPos;
			bFirstClick = false;
		}
		else
		{
			m_Camera.Rotate(xPos - m_LastClickPosX, yPos - m_LastClickPosY, m_DeltaTime);
			m_Camera.UpdateViewMatrix();

			m_LastClickPosX = xPos;
			m_LastClickPosY = yPos;
		}

	}

}

void HelloApp::OnKeyPressed(WPARAM key, LPARAM lParam)
{
	//switch (key)
	//{
	//case VK_UP:
	//	m_Camera.AddMoveState(CameraState::MoveForward);
	//	break;
	//case VK_DOWN:
	//	m_Camera.AddMoveState(CameraState::MoveBackward);
	//	break;
	//case VK_LEFT:
	//	m_Camera.AddMoveState(CameraState::MoveLeft);
	//	break;
	//case VK_RIGHT:
	//	m_Camera.AddMoveState(CameraState::MoveRight);
	//	break;
	//}
}

void HelloApp::UpdateCamera(double deltaTime)
{
	if (GetAsyncKeyState('Z'))
	{
		m_Camera.AddMoveState(CameraState::MoveForward);
	}

	if (GetAsyncKeyState('S'))
	{
		m_Camera.AddMoveState(CameraState::MoveBackward);
	}

	if (GetAsyncKeyState('Q'))
	{
		m_Camera.AddMoveState(CameraState::MoveLeft);
	}

	if (GetAsyncKeyState('D'))
	{
		m_Camera.AddMoveState(CameraState::MoveRight);
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		m_Camera.AddMoveState(CameraState::MoveUp);
	}

	if ( GetAsyncKeyState(VK_LSHIFT))
	{
		m_Camera.AddMoveState(CameraState::MoveDown);
	}

	if (m_Camera.NeedUpdate())
	{
		m_Camera.UpdatePosition(deltaTime);
		m_Camera.UpdateViewMatrix();
	}
}

//*********************************************************
//*********************************************************

void HelloApp::Terminate()
{
}

//*********************************************************

HelloApp::~HelloApp()
{
}