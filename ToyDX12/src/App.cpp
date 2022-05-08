#include "pch.h"

#include "App.h"

// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloWindow/Win32Application.cpp

int Win32App::Run(Win32Window* pWindow, HINSTANCE hInstance, int nCmdShow)
{
    pWindow->Create(hInstance, WindowProc, nCmdShow);
    // Renderer OnInit function 

    // Window main loop
    MSG msg = {};
    pWindow->Update(msg);

    pWindow->Close();

    // Renderer OnDestroy function 

    return static_cast<char>(msg.wParam);
}

LRESULT Win32App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Win32Window* pWindow = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the Win32Window* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
