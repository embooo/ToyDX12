#pragma once

class Window
{
public:

	Window() = default;
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	Window(int32_t Width, int32_t Height, std::string Title);

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

class Win32Window : public Window
{
public:
	Win32Window(int32_t Width, int32_t Height, std::wstring Title);

	const WCHAR* GetTitle()	const { return m_Title.c_str(); }
	static HWND GetHWND() { return s_HWND; }

	void Create(HINSTANCE hInstance, WNDPROC eventCallbackFunc, int nCmdShow);
	void Update(MSG& msg);
	void Close();
private:
	std::wstring m_Title;
	static HWND s_HWND;
};