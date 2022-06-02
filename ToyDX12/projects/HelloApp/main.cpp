#include "pch.h"

#include "HelloApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    HelloApp app(hInstance);

    app.Init();
    app.Run();

    return 0;
}