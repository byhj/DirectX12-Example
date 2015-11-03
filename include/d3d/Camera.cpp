#include "Camera.h"

namespace byhj
{
	namespace d3d
	{
		void Camera::Init(XMFLOAT3 pos)
		{
			m_InitalPos = pos;
			Reset();
		}

		void Camera::SetMoveSpeed(float unitsPerSecond) 
		{
			m_MoveSpeed = unitsPerSecond;
		}

		void Camera::SetTurnSpeed(float radiansPerSecond)
		{
			m_TurnSpeed = radiansPerSecond;
		}

		void Camera::Reset()
		{
			m_Pos = m_InitalPos;
			m_Yaw = XM_PI;
			m_Pitch = 0.0f;
			m_Look ={ 0, 0, -1 };
		}

		void Camera::Update(float elapsedTime)
		{

			//Calculate the move vector in camera space 
			XMFLOAT3 move(0, 0, 0);
			if (m_KeysPressed.a)
				move.x -= 1.0f;
			if (m_KeysPressed.d)
				move.x += 1.0f;
			if (m_KeysPressed.w)
				move.z -= 1.0f;
			if (m_KeysPressed.s)
				move.z += 1.0f;

			if (fabs(move.x) > 0.1f && fabs(move.z) > 0.1f)
			{
				XMVECTOR vector = XMVector3Normalize(XMLoadFloat3(&move));
				move.x = XMVectorGetX(vector);
				move.z = XMVectorGetZ(vector);
			}
			float moveInterval = m_MoveSpeed * elapsedTime;
			float rotateInterval = m_TurnSpeed * elapsedTime;

			if (m_KeysPressed.left)
				m_Yaw += rotateInterval;
			if (m_KeysPressed.right)
				m_Yaw -= rotateInterval;
			if (m_KeysPressed.up)
				m_Pitch += rotateInterval;
			if (m_KeysPressed.down)
				m_Pitch -= rotateInterval;

			// Prevent looking too far up or down.
			m_Pitch = min(m_Pitch, XM_PIDIV4);
			m_Pitch = max(-XM_PIDIV4, m_Pitch);

			// Move the camera in model space.
			float x = move.x * -cosf(m_Yaw) - move.z * sinf(m_Yaw);
			float z = move.x * sinf(m_Yaw) - move.z * cosf(m_Yaw);
			m_Pos.x += x * moveInterval;
			m_Pos.z += z * moveInterval;

			// Determine the look direction.
			float r = cosf(m_Pitch);
			m_Look.x = r * sinf(m_Yaw);
			m_Look.y = sinf(m_Pitch);
			m_Look.z = r * cosf(m_Yaw);

		}

		XMMATRIX Camera::GetProjMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
		{
			return XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
		}

		void Camera::OnKeyDown(WPARAM key)
		{
			switch (key)
			{
			case 'W':
				m_KeysPressed.w = true;
				break;
			case 'A':
				m_KeysPressed.a = true;
				break;
			case 'S':
				m_KeysPressed.s = true;
				break;
			case 'D':
				m_KeysPressed.d = true;
				break;
			case VK_LEFT:
				m_KeysPressed.left = true;
				break;
			case VK_RIGHT:
				m_KeysPressed.right = true;
				break;
			case VK_UP:
				m_KeysPressed.up = true;
				break;
			case VK_DOWN:
				m_KeysPressed.down = true;
				break;
			case VK_ESCAPE:
				Reset();
				break;
			}
		}

		void Camera::OnKeyUp(WPARAM key)
		{
			switch (key)
			{
			case 'W':
				m_KeysPressed.w = false;
				break;
			case 'A':
				m_KeysPressed.a = false;
				break;
			case 'S':
				m_KeysPressed.s = false;
				break;
			case 'D':
				m_KeysPressed.d = false;
				break;
			case VK_LEFT:
				m_KeysPressed.left = false;
				break;
			case VK_RIGHT:
				m_KeysPressed.right = false;
				break;
			case VK_UP:
				m_KeysPressed.up = false;
				break;
			case VK_DOWN:
				m_KeysPressed.down = false;
				break;
			}
		}

	}
}