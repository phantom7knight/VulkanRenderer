#pragma once
class Camera
{

private:
	CameraMatrices matrices;

public:
	Camera();
	~Camera();
	
	//Variables
	float camera_move_speed_;
	glm::vec3 Camera_Pos_;
	glm::vec3 Camera_Front_;
	glm::vec3 Camera_Up_;
	glm::vec3 Camera_Right_;
	glm::vec3 Camera_WorldUp_;

	float Yaw;
	float Pitch;

	float m_MovementSpeed;
	float m_MouseSensitivity;
	float m_Zoom;
	
	InputKeys keys;

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	void ProcessKeyBoardMovement(double a_fDeltaTime);

	void CameraUpdate();

	void SetPerspective(float fov, float aspect, float znear, float zfar);

	void SetPosition(const glm::vec3& position);

	const glm::vec3& GetCameraPos()
	{
		return Camera_Pos_;
	}

	glm::mat4 GetViewMatrix()
	{
		return matrices.view;
	}

	glm::mat4 GetpersepectiveMatrix()
	{
		return matrices.perspective;
	}

	glm::mat4 GetOrthographicMatrix()
	{
		return matrices.orthographic;
	}

	bool m_updateDirty;
};

