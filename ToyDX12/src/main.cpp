#include "pch.h"

#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    Win32Window window(1280, 720, L"ToyDX12");
    return Win32App::Run(&window, hInstance, nCmdShow);
}