#pragma once

#include <wrl/client.h>

enum class ShaderKind
{
	NONE,
	VERTEX,
	PIXEL,
	COMPUTE
};

namespace ToyDX
{
	class Shader
	{
	public:
		Shader() = default;
		~Shader() = default;

		Microsoft::WRL::ComPtr<ID3DBlob> Compile(const WCHAR* sz_Filename, const char* sz_EntryPoint, ShaderKind e_ShaderProgramKind);
		ID3DBlob* GetByteCode() const { return m_Bytecode.Get(); }
		D3D12_INPUT_LAYOUT_DESC* GetInputLayout() { return m_InputLayout; }
		void Release();
	private:
		ShaderKind m_ShaderKind = ShaderKind::NONE;
		D3D12_INPUT_LAYOUT_DESC* m_InputLayout = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> m_Bytecode = nullptr;
	};
}