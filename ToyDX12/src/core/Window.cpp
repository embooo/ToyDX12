#include "pch.h"

#include "Window.h"
#include "DX12App.h"

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

void Win32Window::Init(HINSTANCE hInstance, int nCmdShow)
{
    // Initialize window
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = StaticWndProc;
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

    assert(windowRect.bottom > 0);
    m_fAspectRatio = (float)windowRect.right / windowRect.bottom;

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

int Win32Window::Update()
{
    return 0;
}

//*********************************************************
// https://devblogs.microsoft.com/oldnewthing/20140127-00/?p=1963
// https://devblogs.microsoft.com/oldnewthing/20191014-00/?p=102992
LRESULT Win32Window::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Win32Window* self;
    if (uMsg == WM_NCCREATE) 
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        self = static_cast<Win32Window*>(lpcs->lpCreateParams);
        self->s_HWND = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else 
    {
        self = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self) 
    {
        return self->WndProc(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//*********************************************************

LRESULT Win32Window::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        Terminate();
        return 0;

    //******************* Mouse events 
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/mouse-input-notifications
    case WM_MOUSEMOVE:
        OnMouseMove(wParam, lParam);
        return 0;

    case WM_LBUTTONDOWN:
        OnMouseDown(lParam);
        return 0;

    case WM_LBUTTONUP:
        OnMouseUp(lParam);
        return 0;

    //******************* Keyboard events 
    case WM_KEYDOWN:
        OnKeyPressed(wParam, lParam);
        return 0;

    //******************* Window events 
    // https://docs.microsoft.com/fr-fr/windows/win32/winmsg/windowing
    case WM_SIZING: // User is Resizing window
        OnResize(lParam);
        return 0;

    case WM_ENTERSIZEMOVE: // User starts Resizing or moving window
        m_AppHandle->bIsPaused = true;
        return 0;

    case WM_EXITSIZEMOVE: // User ends Resizing or moving window
        m_AppHandle->bIsPaused = false;
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 200;
        mmi->ptMinTrackSize.y = 200;

        return 0;
    }

    default:
        // Handle any messages the switch statement didn't.
        return DefWindowProc(s_HWND, uMsg, wParam, lParam);
    }
}

//*********************************************************

void Win32Window::OnResize(LPARAM lParam)
{
    RECT rect = {};
    
    if (GetClientRect(s_HWND, &rect))
    {
        m_Width  = rect.right;
        m_Height = rect.bottom;

        m_fAspectRatio = (float)m_Width / m_Height;
    }

    bIsResizing = false;

    // Propagate
    m_AppHandle->OnResize(lParam);

    LOG_INFO("Event : Window resize (AR={0}): {1} {2}", m_fAspectRatio, m_Width, m_Height);
}

void Win32Window::OnMouseDown(LPARAM lParam)
{
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    m_AppHandle->OnMouseDown(xPos, yPos);
}

void Win32Window::OnMouseUp(LPARAM lParam)
{
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    m_AppHandle->OnMouseUp(xPos, yPos);
}

void Win32Window::OnMouseMove(WPARAM buttonState, LPARAM lParam)
{
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    m_AppHandle->OnMouseMove(buttonState, xPos, yPos);
}

void Win32Window::OnKeyPressed(WPARAM buttonState, LPARAM lParam)
{
    m_AppHandle->OnKeyPressed(buttonState, lParam);
}

//*********************************************************

int Win32Window::Terminate()
{
    if (MessageBox(s_HWND, L"Really quit?", m_Title, MB_OKCANCEL) == IDOK)
    {
        m_AppHandle->Terminate();
        m_IsClosed = true;

        if (m_bUseConsole)
        {
            FreeConsole();
        }

        DestroyWindow(s_HWND);
        PostQuitMessage(0);
    }
    else
    {
        return 0;
    }
}
