#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <numbers>

#include <windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <shellapi.h>
#include <io.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <memory>

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "Logger.h"

// https://github.com/Microsoft/DirectXTK/wiki/ThrowIfFailed

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
}
