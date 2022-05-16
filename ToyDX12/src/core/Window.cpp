#include "pch.h"

#include "Window.h"

HWND Win32Window::s_HWND = nullptr;

Window::Window(int32_t Width, int32_t Height, const char* Title)
	: m_Width(Width), m_Height(Height), m_Title(Title), m_IsClosed(false)
{
}

//*********************************************************

Win32Window::Win32Window(int32_t Width, int32_t Height, const WCHAR* Title, bool bUseConsole)
    : m_bUseConsole(bUseConsole)
{
    m_IsClosed = false;
    m_Width = Width;
    m_Height = Height;
    m_Title = Title;
}

//*********************************************************

void Win32Window::Create(HINSTANCE hInstance, WNDPROC eventCallbackFunc, int nCmdShow)
{
    // Initialize window
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = eventCallbackFunc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"Win32AppClass";
    windowClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(GetWidth()), static_cast<LONG>(GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Create the window and store a handle to it.
    s_HWND = CreateWindow(
        windowClass.lpszClassName,
        GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        this);

    if (m_bUseConsole)
    {
        CreateConsole();
    }

    ShowWindow(s_HWND, nCmdShow);
}

//*********************************************************

void Win32Window::CreateConsole()
{
    if (!AllocConsole()) {
        return;
    }

    // Initialize in/out streams

    // std::cout, std::clog, std::cerr, std::cin
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    HANDLE hConOut = CreateFile(TEXT("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn  = CreateFile(TEXT("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();
}

//*********************************************************

void Win32Window::Update(MSG& msg)
{
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

//*********************************************************

void Win32Window::Close()
{
    if (m_bUseConsole)
    {
        FreeConsole();
    }
    m_IsClosed = true;
}
