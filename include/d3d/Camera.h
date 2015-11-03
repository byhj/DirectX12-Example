#ifndef __Camera_H_
#define __Camera_H_

#include <DirectXMath.h>
#include <windows.h>

using namespace DirectX;

namespace byhj
{
	namespace d3d
	{
		class Camera
		{
		public:
			Camera()
			{
				ZeroMemory(&m_KeysPressed, sizeof(m_KeysPressed));
			}
			~Camera() = default;

			void Init(XMFLOAT3 pos);
			void Update(float elapsedTime);
			
			XMMATRIX GetViewMatrix() const;
			XMMATRIX GetProjMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);
			
			void SetMoveSpeed(float unitsPerSecond);
			void SetTurnSpeed(float radiansPerSecond);
			void OnKeyDown(WPARAM key);
			void OnKeyUp(WPARAM key);

		private:
			void Reset();

			struct  KeysPressed
			{
				bool w;
				bool a;
				bool s;
				bool d;

				bool left;
				bool right;
				bool up;
				bool down;
			};

			XMFLOAT3 m_InitalPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 m_Pos       = XMFLOAT3(0.0f, 0.0f, 0.0f);
			XMFLOAT3 m_Look      = XMFLOAT3(0.0f, 0.0f, -1.0f);
			XMFLOAT3 m_Up        = XMFLOAT3(0.0f, 1.0f, 0.0f);

			float m_Yaw = XM_PI;   //+z
			float m_Pitch = 0.0f;  // xz plane

			float m_MoveSpeed = 20.0f;
			float m_TurnSpeed = XM_PIDIV2;

			KeysPressed m_KeysPressed;

		};
	}
}
#endif