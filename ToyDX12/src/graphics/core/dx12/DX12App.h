#pragma once

#include "IApp.h"
#include "DX12RenderingPipeline.h"

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
	bool Run();
	bool InitRenderingPipeline();
	static DX12App* GetApp() { return s_App; }

	// App specific functions
	// For example : code to init/update meshes, cameras, animations ...
	virtual void Init() = 0; 
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Terminate() = 0;

	// Event handlers
	virtual void OnResize();
	virtual void OnMouseClick();
	virtual void OnMouseMove(int xPos, int yPos);
	virtual void OnMouseUp(int xPos, int yPos);
	virtual void OnMouseDown(int xPos, int yPos);

	// App state variables
	bool bIsPaused = false;

protected:
	static DX12App* s_App;

	HINSTANCE m_hInstance;
	std::unique_ptr<Win32Window> mp_Window;
	std::unique_ptr<DX12RenderingPipeline> mp_DX12RenderingPipeline;
};