#include "pch.h"

#include "DX12App.h"

void DX12App::Run(HINSTANCE hInstance, int nCmdShow)
{
    Init(hInstance, nCmdShow);

    // Process user events
    Update();

    // Close
    Terminate();
}

//*********************************************************

void DX12App::Init(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize Window
	mp_Window = std::make_unique<Win32Window>(1280, 720, L"ToyDX12");
	mp_Window->Init(hInstance, nCmdShow);
    mp_Window->SetAppHandle(this);

    // Initialize Logger
    Logger::Init("ToyEngine");

    // Initialize Direct3D 
    mp_DX12RenderingPipeline = std::make_unique<DX12RenderingPipeline>();
    mp_DX12RenderingPipeline->Init(*mp_Window);
}

//*********************************************************

int DX12App::Update()
{
    mp_Window->Update();

    return 0;
}

//*********************************************************

void DX12App::Terminate()
{
    mp_Window->Terminate();
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


