#include "pch.h"
#include "ToyDXUploadBuffer.h"

ToyDX::UploadBuffer::UploadBuffer()
	: m_bIsConstantBuffer(false), m_MappedData(nullptr), m_szElementSizeInBytes(0) 
{}

ToyDX::UploadBuffer& ToyDX::UploadBuffer::operator=(UploadBuffer&& other) noexcept
{
	if (this != &other)
	{
		m_UploadBuffer = std::move(other.m_UploadBuffer);
		m_MappedData = other.m_MappedData;
		m_szElementSizeInBytes = other.m_szElementSizeInBytes;
		m_bIsConstantBuffer = other.m_bIsConstantBuffer;

		other.m_UploadBuffer->Release();
	}

	return *this;
}

//*********************************************************

void ToyDX::UploadBuffer::Create(ID3D12Device* p_Device, UINT ui_NumElements, size_t sz_ElementSizeInBytes, bool bIsConstantBuffer, const wchar_t* sz_DebugName)
{
	if (bIsConstantBuffer)
	{
		sz_ElementSizeInBytes = CalcConstantBufferSize(sz_ElementSizeInBytes);
	}

	m_bIsConstantBuffer = bIsConstantBuffer;
	m_szElementSizeInBytes = sz_ElementSizeInBytes;

	// Create and map resource
	CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sz_ElementSizeInBytes * ui_NumElements);

	ThrowIfFailed(p_Device->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_UploadBuffer.GetAddressOf())));

	m_UploadBuffer->SetName(sz_DebugName);

	// Obtain a pointer to the resource data
	// Map the entire resource by specifying nullptr as the range
	ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, (void**)(&m_MappedData) ));
}

//*********************************************************

void ToyDX::UploadBuffer::CopyData(int i_Index, void* p_Data, size_t sz_DataSizeInBytes)
{
	memcpy(&m_MappedData[i_Index * m_szElementSizeInBytes], p_Data, sz_DataSizeInBytes);
}

//*********************************************************

void ToyDX::UploadBuffer::Destroy()
{
	if (m_UploadBuffer != nullptr)
	{
		m_UploadBuffer->Unmap(0, nullptr);
		m_MappedData = nullptr;
	}
}

size_t ToyDX::UploadBuffer::CalcConstantBufferSize(size_t ConstantBufferSizeCPU)
{
	size_t ConstantBufferSizeGPU = ConstantBufferSizeCPU;

	// Constant buffers are updated once per frame
	// Size must be a multiple of the minimum hardware allocation size (256 bytes)
	if (ConstantBufferSizeGPU % 256 != 0)
	{
		// Round constant buffer size to the nearest multiple of 256
		// Add 255 and mask the lower two bytes because they are not multiples of 256 (15 * 16^0 < 256 ; 15 * 16^1 < 256)
		// Example with a constant buffer size of 300 : 300 + 255 = 555 (0x022B) 
		// 0x022B & ~0x00FF = 0x022B & 0xFF00 = 0x0200 = 512 bytes
		ConstantBufferSizeGPU = (ConstantBufferSizeGPU + 255) & ~255;
	}

	return ConstantBufferSizeGPU;
}
