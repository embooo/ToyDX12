#pragma once

#include "IApp.h"

class Timer;
class DX12RenderingPipeline;

class DX12App
{
public:
	DX12App(HINSTANCE hInstance);
	DX12App(const DX12App&) = delete;
	DX12App& operator=(const DX12App&) = delete;
	virtual ~DX12App();

	// Global app functions
	bool Initialize();
	bool InitWindow(HINSTANCE hInstance, int nCmdShow);
	int  Run();
	bool InitRenderingPipeline();
	static DX12App* GetApp() { return s_App; }

	// App specific functions
	// For example : code to init/update meshes, cameras, animations ...
	virtual void Init() = 0; 
	virtual void Update(double deltaTime) = 0;
	virtual void Update(const Timer* const timer) = 0;
	virtual void Draw(double deltaTime) = 0;
	virtual void Terminate() = 0;

	// Event handlers
	virtual void OnResize(LPARAM lParam);
	virtual void OnResize(WPARAM wParam);
	virtual void OnMouseClick(WPARAM buttonState, int xPos, int yPos);
	virtual void OnMouseMove(WPARAM buttonState, int xPos, int yPos);
	virtual void OnMouseUp(int xPos, int yPos);
	virtual void OnMouseDown(int xPos, int yPos);
	virtual void OnKeyPressed(WPARAM buttonState, LPARAM lParam);

	// App state variables
	bool bIsPaused = false;
	float GetWindowAspectRatio() { return mp_Window->GetAspectRatio(); }

	bool bFirstClick = false;
	int m_LastClickPosX = 0;
	int m_LastClickPosY = 0;

protected:
	static DX12App* s_App;
	std::unique_ptr<Timer> m_Timer;
	HINSTANCE m_hInstance;
	std::unique_ptr<Win32Window> mp_Window;
	std::unique_ptr<DX12RenderingPipeline> mp_DX12RenderingPipeline;
};