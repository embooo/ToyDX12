#pragma once

#include <wrl/client.h>

class ID3D12Resource;
class ID3D12Device;

namespace ToyDX
{
	class UploadBuffer
	{
	public:
		UploadBuffer();
		UploadBuffer(const UploadBuffer&) = delete;
		UploadBuffer& operator=(const UploadBuffer&) = delete;

		ID3D12Resource* GetResource() const { return m_UploadBuffer.Get(); }

		void Create(ID3D12Device* p_Device, UINT ui_NumElements, size_t sz_ElementSizeInBytes, bool bIsConstantBuffer);
		void CopyData(int i_Index, void* p_Data, size_t sz_DataSizeInBytes);
		void Destroy();

		bool IsConstantBuffer() { return m_bIsConstantBuffer; };

		~UploadBuffer() = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
		BYTE* m_MappedData;
		size_t m_szElementSizeInBytes;
		bool m_bIsConstantBuffer;
	};
}