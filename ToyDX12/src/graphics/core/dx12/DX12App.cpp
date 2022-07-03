#include "pch.h"

#include "Timer.h"

#include "DX12App.h"
#include "DX12RenderingPipeline.h"

DX12App::DX12App(HINSTANCE hInstance)
    : m_hInstance(hInstance)
{
}


bool DX12App::Initialize()
{
    InitWindow(m_hInstance, SW_SHOW); // Init a Win32 window
    InitRenderingPipeline(); // Init Direct3D
    m_Timer = std::make_unique<Timer>();

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

int DX12App::Run()
{
    MSG msg = {};

    m_Timer->Reset();

    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else // Engine related updates/draws
        {
            m_Timer->Tick();

            if (bIsPaused)
            {
                Sleep(100);
            }
            else
            {
                const double& dt = m_Timer->GetDeltaTime();
                Update(m_Timer.get());
                Draw(dt);
            }

        }
    }

    return static_cast<int>(msg.wParam);
}

//*********************************************************

void DX12App::Terminate()
{
    mp_DX12RenderingPipeline->Terminate();
}

//*********************************************************

void DX12App::OnResize(LPARAM lParam)
{
   
}

void DX12App::OnResize(WPARAM wParam)
{

}

void DX12App::OnMouseClick(WPARAM buttonState, int xPos, int yPos)
{
}


void DX12App::OnMouseMove(WPARAM buttonState, int xPos, int yPos)
{
    
}

void DX12App::OnMouseUp(int xPos, int yPos)
{
    
}

void DX12App::OnMouseDown(int xPos, int yPos)
{
    bFirstClick = true;
    m_LastClickPosX = xPos;
    m_LastClickPosY = yPos;
}

void DX12App::OnKeyPressed(WPARAM buttonState, LPARAM lParam)
{
}


