#include "pch.h"

#include "TDXShader.h"
#include "DX12RenderingPipeline.h"

using namespace Microsoft::WRL;

Microsoft::WRL::ComPtr<ID3DBlob> ToyDX::Shader::Compile(const WCHAR* sz_Filename, const char* sz_EntryPoint, ShaderKind e_ShaderProgramKind)
{
	// Optional compile flags
	UINT shaderCompileOptions = 0;
	
#if defined(DEBUG_BUILD)
	shaderCompileOptions |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
#endif

	// Generate a string containing the shader program type and version 
	const char* sz_Target = "";
	switch (e_ShaderProgramKind)
	{
	case ShaderKind::VERTEX:
		sz_Target = "vs_5_0";
		break;
	case ShaderKind::PIXEL:
		sz_Target = "ps_5_0";
		break;
	case ShaderKind::COMPUTE:
		sz_Target = "cs_5_0";
		break;

	default:
		{
			LOG_ERROR("Unreconized shader program type.");
			return nullptr;
		}
	}

	// Compilation
	ComPtr<ID3DBlob> shaderCompileErrors   = nullptr;

	HRESULT result = D3DCompileFromFile(sz_Filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, sz_EntryPoint, sz_Target, shaderCompileOptions, 0, m_Bytecode.GetAddressOf(), shaderCompileErrors.GetAddressOf());

	if (shaderCompileErrors != nullptr)
	{
		LOG_ERROR("SHADER COMPILE ERROR : \n");
		LOG_ERROR((char*)shaderCompileErrors->GetBufferPointer());
		OutputDebugStringA((char*)shaderCompileErrors->GetBufferPointer());
	}

	ThrowIfFailed(result);

	return m_Bytecode;
}

void ToyDX::Shader::Release()
{
	if (m_Bytecode != nullptr)
	{
		m_Bytecode->Release();
	}
}
