#pragma once
#pragma once

#include "Window.h"

class App
{
protected:
	App() = default;
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	
	virtual ~App() = default;
public:
	virtual int Run();
};

//*********************************************************
// Windows implementation
//*********************************************************

class Win32Window;

class Win32App : App
{
public:
	static int Run(Win32Window* pWindow, HINSTANCE hInstance, int nCmdShow);
protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};