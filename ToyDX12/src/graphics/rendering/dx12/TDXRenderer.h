#include "IRenderer.h"

#include "MathUtil.h"
#include "ToyDXUploadBuffer.h"
#include "FrameResource.h"
#include "Drawable.h"
#include "Timer.h"

class DX12RenderingPipeline;

struct PerObjectData
{
	DirectX::XMFLOAT4X4 gWorld = MathUtil::Float4x4Identity();
};

struct PerPassData
{
	DirectX::XMFLOAT4X4 gView;
	DirectX::XMFLOAT4X4 gInvView;
	DirectX::XMFLOAT4X4 gProj;
	DirectX::XMFLOAT4X4 gInvProj;
	DirectX::XMFLOAT3   gEyePosWS;
	float pad0;
	float gNear;
	float gFar;
	float gDeltaTime;
	float gTotalTime;
};

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseFactor = { 1.0f, 1.0f, 1.0f, 1.0f };	// DiffuseAlbedo
	DirectX::XMFLOAT3 SpecularFactor = { 0.01f, 0.01f, 0.01f };		// F0
	float GlossinessFactor = 1 - 0.25f;								// 1 - Roughness
};

namespace ToyDX
{
	class Shader;
	class UploadBuffer;
	class Mesh;
	class Camera;

	class Renderer : public IRenderer
	{
	public:
		Renderer() = default;

		void Initialize() override;
		void Render() override;
		void Terminate() override;

		void UpdateFrameResource();
		void UpdatePerObjectCBs();
		void UpdatePerPassCB();
		void UpdateMaterialCBs();

		void RenderOpaques(ID3D12GraphicsCommandList* cmdList, std::vector<Drawable*>& drawables);

		void CreatePipelineStateObjects();
		void AdvanceToNextFrameResource();
		FrameResource* GetCurrentFrameResource() { return m_CurrentFrameResource; }
		
		~Renderer() = default;
	public:
		void SetRasterizerState(bool bWireframe = false, bool bBackFaceCulling = true);
		void SetPipelineState(ID3D12PipelineState* p_Pso) { m_CurrentPSO = p_Pso; };
		void SetCameraHandle(Camera* handle) { m_CameraHandle = handle; }
		void SetTimerHandle(Timer* handle) { m_TimerHandle = handle; }
		void SetRenderingPipelineHandle(DX12RenderingPipeline* handle) { m_hRenderingPipeline = handle; }
	public:
		ID3D12PipelineState* GetPipelineState(const char* name) { return m_PipelineStateObjects.at(name).Get(); };
		UploadBuffer* GetConstantBuffer() { return m_ConstantBuffer.get(); };

	protected:
		std::vector< std::unique_ptr<Drawable>> m_AllDrawables;
		std::vector<Drawable*> m_OpaqueDrawables;
		std::vector<std::unique_ptr<Mesh>> m_Meshes;
		std::unordered_map <const char*, std::unique_ptr<Material>> m_Materials;
		Camera* m_CameraHandle;
		Timer* m_TimerHandle;

	protected:
		// TODO : Move to a more appropriate class
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		void BuildRootSignature();
		void CreateCbvHeap(UINT ui_NumDescriptors);

	protected:
		int m_CurrentFrameResourceIdx = 0;
		FrameResource* m_CurrentFrameResource;
		std::vector<std::unique_ptr<FrameResource>> m_FrameResources;

		// Offset in the descriptor heap to the first per pass CBV
		int m_IndexOf_FirstPerPassCbv_DescriptorHeap;
		int m_IndexOf_FirstMaterialCbv_DescriptorHeap;


		void BuildFrameResources();
		void BuildDescriptorHeaps();
		void BuildConstantBufferViews();
		void BuildMaterials();
		void BuildDrawables();
		void LoadMeshes();

		PerPassData perPassData;

		UINT64 m_CurrentFence = 0;
	protected:
		// Constant buffer data
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;
		// Constant buffer 
		std::unique_ptr<UploadBuffer> m_ConstantBuffer;

	protected:
		void LoadShaders();
		
		std::shared_ptr<Shader> m_DefaultVertexShader = nullptr;
		std::shared_ptr<Shader> m_DefaultPixelShader = nullptr;
	protected:
		CD3DX12_RASTERIZER_DESC m_RasterizerState;


		std::unordered_map<const char*, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PipelineStateObjects;

		ID3D12PipelineState* m_CurrentPSO;

		DX12RenderingPipeline* m_hRenderingPipeline;
	};
}


