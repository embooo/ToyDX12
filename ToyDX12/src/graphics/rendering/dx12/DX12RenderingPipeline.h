#pragma once

#include "IPipeline.h"
#include "DX12Device.h"
#include "DX12CachedValues.h"
#include "ToyDXResource.h"
#include "Window.h"

class DX12RenderingPipeline : public IPipeline
{
public:
	DX12RenderingPipeline() = default;
	~DX12RenderingPipeline() override = default;

	void Init() override;
	void Init(const Win32Window& p_Window);
	void Terminate() override;

	void CreateCommandObjects();
	void CreateFence();
	void CreateSwapchain(HWND hWnd, UINT ui_Width, UINT ui_Height, DXGI_FORMAT e_Format = DXGI_FORMAT_R8G8B8A8_UNORM, UINT ui_BufferCount = s_NumSwapChainBuffers);

	void CheckMSAASupport();

	static ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE e_Type, UINT ui_NumDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS e_Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	void CreateDepthStencil(UINT ui_Width, UINT ui_Height, UINT u_mipLevels = 1, DXGI_FORMAT e_Format = DXGI_FORMAT_D32_FLOAT);

	void SetViewport(UINT ui_Width, UINT ui_Height);
	void SetScissorRectangle(UINT ui_Width, UINT ui_Height);

	void TransitionResource(ToyDX::Resource& st_Resource, D3D12_RESOURCE_STATES e_stateBefore, D3D12_RESOURCE_STATES e_stateAfter);

	static ID3D12Device* GetDevice() { return &s_TDXDevice->GetDevice(); }
	static ID3D12GraphicsCommandList& GetCommandList() { return *s_CommandList.Get(); };
	static ID3D12CommandAllocator& GetCommandAllocator() { return *s_CmdAllocator.Get(); };
	static ID3D12CommandQueue& GetCommandQueue() { return *s_CommandQueue.Get(); };
	D3D12_VIEWPORT& GetViewport() { return m_Viewport; };
	D3D12_RECT& GetScissorRect() { return m_ScissorRect; };

	static void ResetCommandList();
	static void CloseCommandList();
	static void FlushCommandQueue();

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_DepthStencilView; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const { return m_SwapChainRTViews[m_iCurrentBackBuffer]; }

	ID3D12Resource* GetCurrentBackBuffer() const;
	ID3D12Resource* GetDepthStencil() const;
	int SwapChainBufferCount()   const { return s_NumSwapChainBuffers; }
	int CurrentBackBufferIndex() const { return m_iCurrentBackBuffer; };
	void SetCurrentBackBufferIndex(int index) { assert(index >= 0 && index < s_NumSwapChainBuffers); m_iCurrentBackBuffer = index; };
	IDXGISwapChain* GetSwapChain() const { return mp_SwapChain.Get(); }
	D3D12_CLEAR_VALUE DSClearValues;
	D3D12_CLEAR_VALUE RTClearValues;


	// Creates a default buffer in the GPU default heap by using an upload buffer as an intermediate CPU accessible buffer
	static ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* pData, UINT64 ui64_SizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& p_UploadBuffer, D3D12_RESOURCE_STATES e_ResourceState);
	static D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource>& p_VertexBufferGPU, UINT ui_SizeInBytes, UINT ui_StrideInBytes);
	static D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource>& p_IndexBufferGPU, UINT ui_SizeInBytes, DXGI_FORMAT e_Format = DXGI_FORMAT_R16_UINT);
	static void CreateConstantBufferView(ID3D12DescriptorHeap* st_CbvHeap, D3D12_GPU_VIRTUAL_ADDRESS ui64_CbvAddress, UINT ui_SizeInBytes);

	// Pipeline state object
	static ComPtr<ID3D12PipelineState> CreatePipelineStateObject(
		ID3D12RootSignature* p_RootSignature,
		ID3DBlob* st_VsByteCode,
		ID3DBlob* st_PsByteCode,
		D3D12_INPUT_LAYOUT_DESC		  st_InputLayout,
		DXGI_FORMAT					  a_RtvFormats[8],
		CD3DX12_RASTERIZER_DESC		  st_RasterizerState,
		UINT						  ui_NumRenderTargets,
		DXGI_SAMPLE_DESC			  st_SampleDesc = DX12RenderingPipeline::s_SampleDesc,
		DXGI_FORMAT					  e_DsvFormat = DXGI_FORMAT_D32_FLOAT,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE st_PrimitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	// Device
	static std::unique_ptr<DX12Device> s_TDXDevice;

	// Fence
	static ComPtr<ID3D12Fence> s_Fence;
	static UINT64 s_CurrentFenceValue;

	// Command objects
	static ComPtr<ID3D12CommandQueue> s_CommandQueue;
	static ComPtr<ID3D12GraphicsCommandList> s_CommandList;
	static ComPtr<ID3D12CommandAllocator> s_CmdAllocator;

	// Descriptor heaps
	ComPtr<ID3D12DescriptorHeap> mp_RTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> mp_DSVDescriptorHeap;

	// Viewport
	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	// SwapChain
	static const int s_NumSwapChainBuffers = 2;
	ComPtr<IDXGISwapChain> mp_SwapChain;
	ComPtr<ID3D12Resource> mp_SwapChainBuffers[s_NumSwapChainBuffers];
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_SwapChainRTViews[s_NumSwapChainBuffers];


	int  m_iCurrentBackBuffer = 0;
	bool m_bUse4xMsaa = false;
	UINT m_Msaa4xQuality	= 0;
	UINT m_BackBufferWidth  = 0;
	UINT m_BackBufferHeight = 0;
	DXGI_FORMAT m_BackBufferFormat;
	static DXGI_SAMPLE_DESC s_SampleDesc;

	// Depth-Stencil
	ComPtr<ID3D12Resource> mp_DepthStencil;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_DepthStencilView;
	ToyDX::Resource mst_DepthStencil;

	// Cached informations 
	static DX12CachedValues& GetCachedValues() { return s_CachedValues; }
	static DX12CachedValues s_CachedValues;
};


