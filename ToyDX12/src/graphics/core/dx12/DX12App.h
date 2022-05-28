#pragma once

#include "IApp.h"
#include "DX12RenderingPipeline.h"

class DX12App : IApp
{
public:
	DX12App() = default;
	DX12App(const DX12App&) = delete;
	DX12App& operator=(const DX12App&) = delete;

	void Run(HINSTANCE hInstance, int nCmdShow);
	void Init(HINSTANCE hInstance, int nCmdShow);
	void Init() override {};
	int  Update() override;
	void Terminate() override;
protected:
	std::unique_ptr<Win32Window> mp_Window;
	std::unique_ptr<DX12RenderingPipeline> mp_DX12RenderingPipeline;
	static LRESULT CALLBACK ProcessEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};