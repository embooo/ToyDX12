#include "pch.h"

#include "DX12App.h"

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloWindow/Win32Application.cpp

void DX12App::Run(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize Window
    Init(hInstance, nCmdShow);

    // Initialize rendering pipeline

    // Process user events
    Update();

    // Close
    Terminate();
}
void DX12App::Init(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize window
    {
        mp_Window = std::make_unique<Win32Window>(1280, 720, L"ToyDX12");
        assert(mp_Window);
        mp_Window->Create(hInstance, ProcessEvents, nCmdShow);
    }

    // Initialize Logger
    Logger::Init("ToyEngine");

    // Initialize Rendering Pipeline
    mp_DX12RenderingPipeline = std::make_unique<DX12RenderingPipeline>();
    mp_DX12RenderingPipeline->Init();
}

int DX12App::Update()
{
    MSG msg = {};
    mp_Window->Update(msg);

    return static_cast<char>(msg.wParam);
}

void DX12App::Terminate()
{
    mp_Window->Close();
    mp_DX12RenderingPipeline->Terminate();
}

LRESULT DX12App::ProcessEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
