#include "IRenderer.h"

#include "MathUtil.h"
#include "ToyDXUploadBuffer.h"
#include "FrameResource.h"
#include "Drawable.h"
#include "Timer.h"
#include "TDXShader.h"

class DX12RenderingPipeline;

struct PerObjectData
{
	DirectX::XMFLOAT4X4 gWorld = MathUtil::Float4x4Identity();
	DirectX::XMFLOAT4X4 gWorldInvTranspose = MathUtil::Float4x4Identity();
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
	// Specular/Glossiness
	DirectX::XMFLOAT4 DiffuseFactor = { 1.0f, 1.0f, 1.0f, 1.0f };	// DiffuseAlbedo
	DirectX::XMFLOAT3 SpecularFactor = { 0.01f, 0.01f, 0.01f };		// F0
	float GlossinessFactor = 1 - 0.25f;								// 1 - Roughness
};

struct MetallicRoughnessMaterial
{
	// Metallic/Roughness
	DirectX::XMFLOAT4 BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float Metallic  = 0.0f;
	float Roughness = 1.0f;
};

enum PsoList
{
	Wireframe = 0,
	PbrMetallicRoughness = 1,
	PsoCount
};

enum ShaderList
{
	Default_Vertex = 0,
	Default_Pixel  = 1,
	PbrMetallicRoughness_Pixel = 2,
	ShaderCount
};

namespace ToyDX
{
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


		void RenderDrawables(ID3D12GraphicsCommandList& r_cmdList, std::vector<Drawable*>& drawables);
		void RecompileShaders();
		void CreatePipelineStateObjects();
		void AdvanceToNextFrameResource();
		FrameResource* GetCurrentFrameResource() { return m_CurrentFrameResource; }
		
		~Renderer() = default;

	private:
		std::unordered_map <int, Texture*> m_Textures;
		Texture m_FallbackTexture;
		std::vector<UINT8>  CreateFallbackTexture();
	public:
		void SetRasterizerState(bool bWireframe = false, bool bBackFaceCulling = true);
		void SetCameraHandle(Camera* handle) { m_CameraHandle = handle; }
		void SetTimerHandle(Timer* handle) { m_TimerHandle = handle; }
		void SetRenderingPipelineHandle(DX12RenderingPipeline* handle) { m_hRenderingPipeline = handle; }
	public:
		ID3D12PipelineState* GetPipelineState(PsoList e_PsoName) { return m_PSOTable[e_PsoName].Get(); };

	protected:
		std::vector< std::unique_ptr<Drawable>> m_AllDrawables;
		std::vector<Drawable*> m_OpaqueDrawables;
		std::vector<std::unique_ptr<Mesh>> m_Meshes;
		std::unordered_map <std::string, std::unique_ptr<Material>> m_Materials;

		void CreateShaderResourceView(const Texture& texture, ID3D12DescriptorHeap* CbvSrvUavHeap, int SrvIndexInDescriptorHeap);

		
		Camera* m_CameraHandle;
		Timer* m_TimerHandle;

	protected:
		// TODO : Move to a more appropriate class
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		void BuildRootSignature();

	protected:
		int m_CurrentFrameResourceIdx = 0;
		FrameResource* m_CurrentFrameResource;
		std::vector<std::unique_ptr<FrameResource>> m_FrameResources;

		// Offset in the descriptor heap to the first per pass CBV
		int m_IndexOf_FirstPerPassCbv_DescriptorHeap;
		int m_IndexOf_FirstMaterialCbv_DescriptorHeap;
		int m_IndexOf_FirstSrv_DescriptorHeap;

		int m_TotalMaterialCount = 0;
		int m_TotalTextureCount = 1; 
		int m_TotalDrawableCount = 0;

		void BuildFrameResources();
		void CreateDescriptorHeap_Cbv_Srv();
		void CreateStaticSamplers();
		void CreateConstantBufferViews();
		void LoadMaterials();
		void BuildDrawables();
		void LoadMeshes();
		void LoadTextures();

		std::array<D3D12_STATIC_SAMPLER_DESC, 6> m_StaticSamplers;

		PerPassData perPassData;

		UINT64 m_CurrentFence = 0;
	protected:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvSrvHeap;

	protected:
		void LoadShaders();
		Shader m_ShaderTable[ShaderList::ShaderCount];

	protected:
		CD3DX12_RASTERIZER_DESC m_RasterizerState;

		DX12RenderingPipeline* m_hRenderingPipeline;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSOTable[PsoList::PsoCount];

	protected:
		bool m_Raster = true;
		void RenderRaytraced(ID3D12GraphicsCommandList& r_CmdList,  D3D12_CPU_DESCRIPTOR_HANDLE backbufferView, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView);
		void RenderRasterized(ID3D12GraphicsCommandList& r_CmdList, D3D12_CPU_DESCRIPTOR_HANDLE backbufferView, D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView);
	public:
		void ToggleRenderMode();	// Toggle between raytraced or rasterized graphics
	};
}



