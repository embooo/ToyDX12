#pragma once

#include "DX12App.h"
#include "MathUtil.h"

#include "ToyDXCamera.h"

namespace ToyDX
{
	class UploadBuffer;
	class Renderer;
}

class HelloApp : public DX12App
{
public:
	HelloApp(HINSTANCE hInstance, const char* sz_DebugName = "HelloApp");
	~HelloApp() override;

	virtual void Init() override;
	virtual void Update(double deltaTime) override;
	virtual void Draw(double deltaTime) override;

	virtual void Terminate() override;

	// Event handlers
	virtual void OnResize(LPARAM lParam) override;
	virtual void OnResize(WPARAM wParam) override;
	virtual void OnMouseMove(WPARAM buttonState, int xPos, int yPos) override;
	virtual void OnKeyPressed(WPARAM buttonState, LPARAM lParam) override;

	// 
	void UpdateCamera(double deltaTime);
	double m_DeltaTime;

protected:
	const char* m_DebugName;
	
	// Upload buffers
	ComPtr<ID3D12Resource> p_VertexBufferCPU = nullptr; 
	ComPtr<ID3D12Resource> p_IndexBufferCPU  = nullptr; 

	// Heaps
	ComPtr<ID3D12DescriptorHeap> m_CbvHeap;

protected:
	ToyDX::Camera m_Camera;
	std::unique_ptr<ToyDX::Renderer> m_Renderer;
};
