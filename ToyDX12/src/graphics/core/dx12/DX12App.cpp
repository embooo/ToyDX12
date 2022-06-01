#include "pch.h"

#include "DX12App.h"

DX12App::DX12App(HINSTANCE hInstance)
    : m_hInstance(hInstance)
{
}


bool DX12App::Initialize()
{
    InitWindow(m_hInstance, SW_SHOW); // Init a Win32 window
    InitRenderingPipeline(); // Init Direct3D

    return true;
}

bool DX12App::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize Window
	mp_Window = std::make_unique<Win32Window>(1280, 720, L"ToyDX12");
	mp_Window->Init(hInstance, nCmdShow);
    mp_Window->SetAppHandle(this);

    // Initialize Logger
    Logger::Init("ToyEngine");

    return true;
}

bool DX12App::InitRenderingPipeline()
{
    if (mp_Window)
    {
        mp_DX12RenderingPipeline = std::make_unique<DX12RenderingPipeline>();
        mp_DX12RenderingPipeline->Init(*mp_Window);

        return true;
    }

    return false;
}

DX12App::~DX12App()
{
    if (mp_DX12RenderingPipeline->GetDevice() != nullptr)
    {
        mp_DX12RenderingPipeline->FlushCommandQueue();
    }
}

//*********************************************************

bool DX12App::UpdateWindow()
{
    mp_Window->Update();

    return true;
}

//*********************************************************

void DX12App::Terminate()
{
    mp_DX12RenderingPipeline->Terminate();
}

//*********************************************************

void DX12App::OnResize()
{
   
}

void DX12App::OnMouseClick()
{
}

void DX12App::OnMouseMove(int xPos, int yPos)
{
    LOG_INFO("Event : Mouse move : {0} {1}", xPos, yPos);
}

void DX12App::OnMouseUp(int xPos, int yPos)
{
    LOG_INFO("Event : Left Mouse Button Up : {0} {1}", xPos, yPos);
}

void DX12App::OnMouseDown(int xPos, int yPos)
{
    LOG_INFO("Event : Left Mouse Button Down : {0} {1}", xPos, yPos);
}


