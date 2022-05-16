#include "pch.h"

#include "App.h"

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloWindow/Win32Application.cpp

int Win32App::Run(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize Window
    Win32Window window(1280, 720, L"ToyDX12");
    window.Create(hInstance, WindowProc, nCmdShow);

    // Initialize Logger
    Logger::Init("ToyEngine");

    // Process user events
    MSG msg = {};
    window.Update(msg);

    // Close
    window.Close();

    return static_cast<char>(msg.wParam);
}

LRESULT Win32App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
