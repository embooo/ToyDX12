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

	LOG_INFO("{0}::Init() Hello !", m_DebugName);

	// Camera initialization
	m_Camera.Init({ .fAspectRatio = DX12App::GetWindowAspectRatio(), .fFovY = DirectX::XMConvertToRadians(45.0f), .fNearZ = 1.0f,.fFarZ = 1000.0f,});
	
	// Renderer
	m_Renderer = std::make_unique<ToyDX::Renderer>();
	m_Renderer->Initialize();
	m_Renderer->SetRenderingPipelineHandle(mp_DX12RenderingPipeline.get());
	m_Renderer->SetCameraHandle(&m_Camera);
	m_Renderer->SetTimerHandle(m_Timer.get());
}

void HelloApp::Update(double deltaTime)
{
}

//*********************************************************

void HelloApp::Update(const Timer* const timer)
{
	m_DeltaTime = timer->GetDeltaTime();

	UpdateCamera(m_DeltaTime);

	m_Renderer->UpdateFrameResource();
}

//*********************************************************

void HelloApp::Draw(double deltaTime)
{
	{
		m_Renderer->Render();
	}
}

//*********************************************************

void HelloApp::OnResize(LPARAM lParam)
{
	m_Camera.SetFrustum({ .fAspectRatio = GetWindowAspectRatio() });
}

void HelloApp::OnResize(WPARAM wParam)
{
	m_Camera.SetFrustum({ .fAspectRatio = GetWindowAspectRatio() });
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