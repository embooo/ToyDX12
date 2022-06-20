#pragma once
#include <DirectXMath.h>

const double M_PI = 3.1415926;

class MathUtil
{
public:
	MathUtil() = delete;
	~MathUtil() = delete;
	MathUtil(MathUtil&) = delete;
	MathUtil operator=(MathUtil&) = delete;

	static constexpr DirectX::XMFLOAT4X4 Float4x4Identity()
	{
		return DirectX::XMFLOAT4X4
		(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
	}
};

