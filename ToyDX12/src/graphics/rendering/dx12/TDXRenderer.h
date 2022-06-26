#include "IRenderer.h"

#include "MathUtil.h"
#include "ToyDXUploadBuffer.h"

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

		void BindResources();

		void SetCameraHandle(Camera* handle) { m_CameraHandle = handle; }
		void SetRasterizerState(bool bWireframe = false, bool bBackFaceCulling = true);
		void SetPipelineState(ID3D12PipelineState* p_Pso) { m_Pso = p_Pso; };
		ID3D12PipelineState* GetPipelineState() { return m_Pso.Get(); };
		UploadBuffer* GetConstantBuffer() { return m_ConstantBuffer.get(); };
		void CreatePipelineStateObject();
	public:
		std::shared_ptr<Mesh> m_Mesh;
		Camera* m_CameraHandle;
	public:
		
		// TODO : Move to a more appropriate class
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		void CreateRootSignature();
		void CreateConstantBuffer();

		// Constant buffer data
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap;

		// Constant buffer 
		std::unique_ptr<UploadBuffer> m_ConstantBuffer;

		~Renderer() = default;

	protected:
		void LoadShaders();
		
		CD3DX12_RASTERIZER_DESC m_RasterizerState;
		std::shared_ptr<Shader> m_DefaultVertexShader = nullptr;
		std::shared_ptr<Shader> m_DefaultPixelShader  = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_Pso;
	};
}


struct PerObjectData
{
	DirectX::XMFLOAT4X4 gWorld = MathUtil::Float4x4Identity();
	DirectX::XMFLOAT4X4 gView  = MathUtil::Float4x4Identity();
	DirectX::XMFLOAT4X4 gProj  = MathUtil::Float4x4Identity();
};