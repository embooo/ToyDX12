#include "pch.h"

#include "DX12App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    DX12App app;

    app.Run(hInstance, nCmdShow);

    return 0;
}