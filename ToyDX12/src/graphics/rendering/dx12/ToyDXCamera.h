#pragma once

namespace ToyDX
{
	struct Frustum
	{
		float fFovY = std::numbers::pi_v<float> / 3;
		float fNear = 1.0f;
		float fFar  = 1000.0f;
	};

	struct ControlParams
	{
		float fVerticalSensitivity   = 0.25f;
		float fHorizontalSensitivity = 0.25f;
	};

	class Camera
	{
	public:
		Camera() = default;

		void Init();
		void Rotate(float fMousePosX, float fMousePosY);
		const Frustum& GetFrustum() { return m_stFrustum; }
		Camera& SetControlParameters(const ControlParams& st_ControlParams = {}, float fRadius = 5.0);
		Camera& SetFrustum(const Frustum& st_Frustum = {});
		Camera& BuildViewMatrix();

		DirectX::XMMATRIX& GetViewMatrix() { return m_ViewMatrix; }

		~Camera() = default;
	protected:
		DirectX::XMMATRIX m_ViewMatrix;

		DirectX::XMVECTOR m_Pos;
		DirectX::XMVECTOR m_Target;
		DirectX::XMVECTOR m_Up;

		float m_fLastMousePosX = 0.0f;
		float m_fLastMousePosY = 0.0f;
		
		float m_fVerticalSensitivity = 0.25f;
		float m_fHorizontalSensitivity = 0.25f;

		float m_fRadius = 5.0f;	
		float m_fTheta = std::numbers::pi_v<float> / 4;	// Vertical angle
		float m_fPhi   = 1.5f * std::numbers::pi_v<float>;	// Horizontal angle

		Frustum m_stFrustum;
		ControlParams m_stControlParams;
	};
}