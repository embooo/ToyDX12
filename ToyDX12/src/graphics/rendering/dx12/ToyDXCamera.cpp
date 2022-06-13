#include "pch.h"
#include "ToyDXCamera.h"

using namespace DirectX;

void ToyDX::Camera::Init()
{
	SetControlParameters();
	SetFrustum();
	BuildViewMatrix();
}

void ToyDX::Camera::Rotate(float fMousePosX, float fMousePosY)
{
	// Compute angle deltas in radians
	float verticalAngleDeltaRad = m_fVerticalSensitivity * XMConvertToRadians(fMousePosX - m_fLastMousePosX);
	float horizontalAngleDeltaRad = m_fHorizontalSensitivity * XMConvertToRadians(fMousePosY - m_fLastMousePosY);
	
	// Update angles
	m_fTheta    = std::clamp(m_fTheta + verticalAngleDeltaRad, 0.1f, std::numbers::pi_v<float> -0.1f);
	m_fPhi += horizontalAngleDeltaRad;

	// Update previous mouse position
	m_fLastMousePosX = fMousePosX;
	m_fLastMousePosY = fMousePosY;
}

ToyDX::Camera& ToyDX::Camera::SetControlParameters(const ControlParams& st_ControlParams, float fRadius)
{
	m_stControlParams = st_ControlParams;
	m_fRadius = fRadius;
	return *this;
}

ToyDX::Camera& ToyDX::Camera::BuildViewMatrix()
{
	// Convert spherical (R, Theta, Phi) coordinates to cartesian (x, y, z)
	float x = m_fRadius * cosf(m_fTheta) * sinf(m_fPhi);
	float y = m_fRadius * sinf(m_fTheta) * sinf(m_fPhi);
	float z = m_fRadius * cosf(m_fPhi);

	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);

	return *this;
}

ToyDX::Camera& ToyDX::Camera::SetFrustum(const Frustum& st_Frustum)
{
	m_stFrustum = st_Frustum;
	return *this;
}
