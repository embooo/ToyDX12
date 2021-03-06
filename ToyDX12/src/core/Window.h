#pragma once

#include "Logger.h"

class Window
{
public:

	Window() = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	Window(int32_t Width, int32_t Height, const char* Title);

	bool IsClosed() const { return m_IsClosed; }

	int32_t GetWidth()		const { return m_Width; }
	int32_t GetHeight()		const { return m_Height; }

	virtual ~Window() = default;
protected:
	bool m_IsClosed;
	int32_t m_Width;
	int32_t m_Height;
	const char* m_Title;
};

//*********************************************************
// Windows implementation
//*********************************************************

class DX12App;

class Win32Window : public Window
{
public:
	Win32Window(int32_t Width, int32_t Height, const WCHAR* Title, bool bUseConsole = true);

	const WCHAR* GetTitle()	const { return m_Title; }
	static HWND GetHWND() { return s_HWND; }
	static LRESULT StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void SetAppHandle(DX12App* handle) { m_AppHandle = handle; };
	void Init(HINSTANCE hInstance, int nCmdShow);
	void CreateConsole();
	int Update();
	int Terminate();

	float GetAspectRatio() { return m_fAspectRatio; }
	void UpdateWindowSize();

	// Event handlers
	void OnResize(LPARAM lParam); // User is resizing the window
	void OnResize(WPARAM wParam); // User clicked on a resizing button (maximize/minimize)
	void OnMouseUp(LPARAM lParam);
	void OnMouseDown(LPARAM lParam);
	void OnMouseMove(WPARAM buttonState, LPARAM lParam);
	void OnKeyPressed(WPARAM buttonState, LPARAM lParam);

	bool bIsResizing = false;
	bool bHasFocus = false;

private:
	float m_fAspectRatio;
	bool m_bUseConsole;
	const WCHAR* m_Title;
	static HWND s_HWND;
	DX12App* m_AppHandle;
};