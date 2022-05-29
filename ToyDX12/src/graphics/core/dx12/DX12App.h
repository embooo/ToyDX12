#pragma once

#include "IApp.h"
#include "DX12RenderingPipeline.h"

class DX12App : IApp
{
public:
	DX12App() = default;
	DX12App(const DX12App&) = delete;
	DX12App& operator=(const DX12App&) = delete;

	virtual void Run(HINSTANCE hInstance, int nCmdShow);
	virtual void Init(HINSTANCE hInstance, int nCmdShow);
	virtual void Init() override {};
	virtual int  Update() override;
	virtual void Terminate() override;

	bool bIsPaused = false;

	// Event handlers
	virtual void OnResize();
	virtual void OnMouseClick();
	virtual void OnMouseMove(int xPos, int yPos);
	virtual void OnMouseUp(int xPos, int yPos);
	virtual void OnMouseDown(int xPos, int yPos);
protected:
	std::unique_ptr<Win32Window> mp_Window;
	std::unique_ptr<DX12RenderingPipeline> mp_DX12RenderingPipeline;
};