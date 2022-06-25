#include "pch.h"
#include "ToyDXCamera.h"
#include "MathUtil.h"

namespace ToyDX
{
	using namespace DirectX;

	void Camera::Init(const Frustum& st_Frustum)
	{
		SetControlParameters();
		SetFrustum(st_Frustum);
		UpdateProjMatrix();
		UpdateViewMatrix();

	}

	bool Camera::NeedUpdate()
	{
		return m_MoveState != CameraState::None;
	}

	void Camera::UpdatePosition(double deltaTime)
	{
		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveForward))
		{
			Translate(m_Forward, deltaTime);
		}

		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveBackward))
		{
			Translate(-1*m_Forward, deltaTime);
		}

		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveLeft))
		{
			Translate(-1*m_Right, deltaTime);
		}

		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveRight))
		{
			Translate(m_Right, deltaTime);
		}

		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveUp))
		{
			Translate(m_Up, deltaTime);
		}

		if (HasFlag<CameraState>(m_MoveState, CameraState::MoveDown))
		{
			Translate(-m_Up, deltaTime);
		}

		// Clear 
		m_MoveState = CameraState::None;
	}

	void Camera::Translate(XMVECTOR v_Axis, double deltaTime)
	{
		m_Pos = XMVectorMultiplyAdd(XMVectorReplicate(f_TranslateIncrement * deltaTime), v_Axis, m_Pos);

		//LOG_WARN("({0}, {1}, {2}), dt = {3}", XMVectorGetX(m_Pos), XMVectorGetY(m_Pos), XMVectorGetZ(m_Pos), deltaTime);

	}

	void Camera::Rotate(float fMouseDeltaX, float fMouseDeltaY, double deltaTime)
	{
		// Rotation is not commutative, order matters
		// Combining Pitch and Yaw may add an unwanted roll to the camera
		// A solution is to yaw around the global Y (up) axis and pitch around the local X (right) axis
		// https://gamedev.stackexchange.com/questions/136174/im-rotating-an-object-on-two-axes-so-why-does-it-keep-twisting-around-the-thir
		
		// Pitch 
		{ 
			
			XMMATRIX R = XMMatrixRotationAxis(m_Right, XMConvertToRadians(fMouseDeltaY * deltaTime * m_fVerticalSensitivity));

			m_Forward = XMVector3TransformNormal(m_Forward, R);
			m_Up = XMVector3TransformNormal(m_Up, R);
		}
		
		// Yaw
		{ 
			XMMATRIX R = XMMatrixRotationY(XMConvertToRadians(fMouseDeltaX * deltaTime) * m_fVerticalSensitivity) ;

			m_Forward = XMVector3TransformNormal(m_Forward, R);
			m_Right   = XMVector3TransformNormal(m_Right, R);
		}

	}

	Camera& Camera::SetControlParameters(const ControlParams& st_ControlParams, float fRadius)
	{
		m_stControlParams = st_ControlParams;
		
		return *this;
	}

	Camera& Camera::UpdateViewMatrix()
	{
		m_Forward = XMVector3Normalize(m_Forward);
		m_Up      = XMVector3Normalize(XMVector3Cross(m_Forward, m_Right));
		m_Right   = XMVector3Cross(m_Up, m_Forward);

		m_ViewMatrix = XMMatrixLookToLH(m_Pos, m_Forward, m_Up);
		
		return *this;
	}

	void Camera::AddMoveState(CameraState e_State)
	{
		m_MoveState = m_MoveState | e_State;
	}

	Camera& Camera::UpdateProjMatrix()
	{

		m_ProjMatrix = XMMatrixPerspectiveFovLH(m_stFrustum.fFovY, m_stFrustum.fAspectRatio, m_stFrustum.fNearZ, m_stFrustum.fFarZ);

		return *this;
	}

	Camera& Camera::SetFrustum(const Frustum& st_Frustum)
	{
		m_stFrustum = st_Frustum;

		UpdateProjMatrix();

		return *this;
	}
}


