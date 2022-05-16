#pragma once

#include "Window.h"

class App
{
public:
	App() = default;
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	
	virtual ~App() = default;

public:
	virtual int Run() = 0;
};

//*********************************************************
// Windows implementation
//*********************************************************

class Win32Window;

class Win32App : App
{
public:
	static int Run(HINSTANCE hInstance, int nCmdShow);
protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};