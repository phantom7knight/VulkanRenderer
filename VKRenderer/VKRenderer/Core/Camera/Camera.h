/* Copyright (c) 2019, Sascha Willems*/


#pragma once
/*class Camera
{
public:
	void update(float deltaTime);

	// Update camera passing separate axis data (gamepad)
	// Returns true if view or position has been changed
	bool update_gamepad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time);

	bool getIsUpdate()
	{
		return camProperties.updated;
	}

	Camera();
	~Camera();

	
	CameraMatrices matrices;

	CameraKeys keys;

	glm::vec3 front;

	float move_speed;

	bool moving();

	float get_near_clip();

	float get_far_clip();

	void set_perspective(float fov, float aspect, float znear, float zfar);

	void update_aspect_ratio(float aspect);

	void set_position(const glm::vec3& position);

	void set_rotation(const glm::vec3& rotation);

	void rotate(const glm::vec3& delta);

	void set_translation(const glm::vec3& translation);

	void translate(const glm::vec3& delta);

	CameraProperties camProperties;

	void update_view_matrix();
};*/

class Camera
{

private:
	static Camera* m_Instance;
	CameraMatrices matrices;

public:
	Camera();
	~Camera();
	static Camera* getInstance();

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
	
	CameraKeys keys;

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	void ProcessKeyBoardMovement(float a_fDeltaTime);

	void CameraUpdate();

	void SetPerspective(float fov, float aspect, float znear, float zfar);

	void SetPosition(const glm::vec3& position);

	const glm::vec3& GetCameraPos()
	{
		return Camera_Pos_;
	}

	glm::mat4 GetViewMatrix()
	{
		//matrices.view = glm::lookAt(Camera_Pos_, Camera_Pos_ + Camera_Front_, Camera_Up_);
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

};

