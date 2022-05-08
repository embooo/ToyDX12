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
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}
