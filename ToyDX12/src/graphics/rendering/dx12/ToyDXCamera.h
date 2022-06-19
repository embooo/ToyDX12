#pragma once

enum class CameraState : int
{
	None = 0,
	MoveForward = 1 << 1,
	MoveBackward = 1 << 2,
	MoveLeft = 1 << 3,
	MoveRight = 1 << 4,
	MoveUp = 1 << 5,
	MoveDown = 1 << 6,
};

template<typename T>
static bool HasFlag(T a, T b)
{
	using U = std::underlying_type_t<T>;
	return (U(a) & U(b)) == U(b);
}

template<typename T> T operator | (T a, T b)
{
	using U = std::underlying_type_t<T>;
	return (T)((U)a | (U)b);
}

namespace ToyDX
{
	using namespace DirectX;

	struct 
	{
		DirectX::XMVECTOR Up = { 0.0f, 1.0f, 0.0f, 0.0f };
		DirectX::XMVECTOR Forward = { 0.0f, 0.0f, 1.0f, 0.0f };
		DirectX::XMVECTOR Right = { 1.0f, 0.0f, 0.0f, 0.0f };
	} LeftHandedAxis;





	struct Frustum
	{
		float fAspectRatio = 1.0f;
		float fFovY = DirectX::XMConvertToRadians(45.0);
		float fNearZ = 1.0;
		float fFarZ  = 1000.0f;
	};

	struct ControlParams
	{
		float fVerticalSensitivity   = 0.15f;
		float fHorizontalSensitivity = 0.15f;
	};

	class Camera
	{
	public:
		Camera() = default;

		void Init(const Frustum& st_Frustum);
		void Rotate(float fMouseDeltaX, float fMouseDeltaY, double deltaTime);
		void Translate(XMVECTOR v_Axis, double deltaTime);

		// Updates camera position based on the current move states 
		bool NeedUpdate();
		void UpdatePosition(double deltaTime);

		// Getters
		const Frustum& GetFrustum() { return m_stFrustum; }
		DirectX::XMMATRIX& GetViewMatrix() { return m_ViewMatrix; }
		DirectX::XMMATRIX& GetProjMatrix() { return m_ProjMatrix; }

		// Setters
		Camera& SetControlParameters(const ControlParams& st_ControlParams = {}, float fRadius = 5.0);
		Camera& SetFrustum(const Frustum& st_Frustum);

		Camera& UpdateProjMatrix();
		Camera& UpdateViewMatrix();

		// 
		void AddMoveState(CameraState e_State);
		CameraState m_MoveState = CameraState::None;

		~Camera() = default;
	protected:
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ProjMatrix;

		DirectX::XMVECTOR m_Pos     = { 0.0, 0.0,  0.0 };
		DirectX::XMVECTOR m_Forward = { 0.0, 0.0,  1.0 };
		DirectX::XMVECTOR m_Up      = { 0.0, 1.0,  0.0 };
		DirectX::XMVECTOR m_Right   = { 1.0, 0.0,  0.0 };

		float m_fLastMousePosX = 0.0f;
		float m_fLastMousePosY = 0.0f;
		
		float f_TranslateIncrement     = 2.5f;
		float m_fVerticalSensitivity   = 15.0f;
		float m_fHorizontalSensitivity = 15.0f;

		float m_fPitch = 0.0;
		float m_fYaw   = 0.0;	// Vertical angle

		Frustum m_stFrustum;
		ControlParams m_stControlParams;
	};
}