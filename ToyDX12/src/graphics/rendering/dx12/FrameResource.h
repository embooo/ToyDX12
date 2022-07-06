#pragma once
#include <wrl/client.h>
#include <memory>

#include "ToyDXUploadBuffer.h"

class ID3D12Device;
class ID3D12CommandAllocator;

namespace ToyDX
{
	class UploadBuffer;

	// Store resources needed by the CPU to build the command list for a frame
	class FrameResource
	{
	public:
		FrameResource(ID3D12Device* p_Device, UINT ui_NumPasses, UINT ui_NumObjects, UINT ui_NumMaterials);
		FrameResource(const FrameResource&) = delete;
		FrameResource& operator=(const FrameResource&) = delete;

		// Command list allocator
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAllocator;

		// Each frame has its constant buffer
		std::unique_ptr<UploadBuffer> cbPerPass   = nullptr;
		std::unique_ptr<UploadBuffer> cbPerObject = nullptr;
		std::unique_ptr<UploadBuffer> cbMaterial  = nullptr;

		// Constant buffer setters
		template<typename T>
		void SetPerPassData(T& data, unsigned int numElements)
		{
			cbPerPass->Create(deviceHandle, 1, sizeof(T), true);
		}

		template<typename T>
		void SetPerObjectData(T& data, unsigned int numElements)
		{
			cbPerObject->Create(deviceHandle, 1, sizeof(T), true);
		}
		

		// To check if the frame resources are still in use by the GPU
		UINT64 FenceValue = 0;
		~FrameResource() = default;
	protected:
		ID3D12Device* deviceHandle;
	};
}