
#include"../PCH/stdafx.h"
#include "Camera.h"

//
//Camera::Camera()
//{
//	camProperties.updated = false;
//	camProperties.rotation_speed = 1.0f;
//	camProperties.translation_speed = 1.0f;
//
//	camProperties.position = glm::vec3(0.0, -1.0, 0.0);
//}
//
//Camera::~Camera()
//{
//}
//
//
//void Camera::update_view_matrix()
//{
//	glm::mat4 rotation_matrix = glm::mat4(1.0f);
//	glm::mat4 transformation_matrix;
//
//	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
//	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
//	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
//
//	transformation_matrix = glm::translate(glm::mat4(1.0f), camProperties.position);
//
//	matrices.view = transformation_matrix * rotation_matrix;
//	//matrices.view = rotation_matrix * transformation_matrix;
//	
//	camProperties.updated = true;
//}
//
//bool Camera::moving()
//{
//	return keys.left || keys.right || keys.up || keys.down;
//}
//
//float Camera::get_near_clip()
//{
//	return camProperties.znear;
//}
//
//float Camera::get_far_clip()
//{
//	return camProperties.zfar;
//}
//
//void Camera::set_perspective(float fov, float aspect, float znear, float zfar)
//{
//	this->camProperties.fov = fov;
//	this->camProperties.znear = znear;
//	this->camProperties.zfar = zfar;
//	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
//}
//
//void Camera::update_aspect_ratio(float aspect)
//{
//	matrices.perspective = glm::perspective(glm::radians(camProperties.fov), aspect, camProperties.znear, camProperties.zfar);
//	//matrices.perspective = glm::ortho(0.f, 400.f, 0.f, 400.f, -1.f, 1.f);
//}
//
//void Camera::set_position(const glm::vec3& position)
//{
//	this->camProperties.position = position;
//	update_view_matrix();
//}
//
//void Camera::set_rotation(const glm::vec3& rotation)
//{
//	this->camProperties.rotation = rotation;
//	update_view_matrix();
//}
//
//void Camera::rotate(const glm::vec3& delta)
//{
//	this->camProperties.rotation += delta;
//	update_view_matrix();
//}
//
//void Camera::set_translation(const glm::vec3& translation)
//{
//	this->camProperties.position = translation;
//	update_view_matrix();
//}
//
//void Camera::translate(const glm::vec3& delta)
//{
//	this->camProperties.position += delta;
//	update_view_matrix();
//}
//
//void Camera::update(float deltaTime)
//{
//	camProperties.updated = false;
//	if (moving())
//	{
//		
//		front.x = -cos(glm::radians(camProperties.rotation.x)) * sin(glm::radians(camProperties.rotation.y));
//		front.y = sin(glm::radians(camProperties.rotation.x));
//		front.z = cos(glm::radians(camProperties.rotation.x)) * cos(glm::radians(camProperties.rotation.y));
//		front = glm::normalize(front);
//
//		move_speed = deltaTime * camProperties.translation_speed;
//
//		if (keys.up)
//		{
//			camProperties.position += front * move_speed;
//		}
//		if (keys.down)
//		{
//			camProperties.position -= front * move_speed;
//		}
//		if (keys.left)
//		{
//			camProperties.position -= glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
//		}
//		if (keys.right)
//		{
//			camProperties.position += glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
//		}
//
//		update_view_matrix();
//	}
//}
//
//bool Camera::update_gamepad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time)
//{
//	bool changed = false;
//
//	// Use the common console thumbstick layout
//	// Left = view, right = move
//
//	const float dead_zone = 0.0015f;
//	const float range = 1.0f - dead_zone;
//
//	glm::vec3 front;
//	front.x = -cos(glm::radians(camProperties.rotation.x)) * sin(glm::radians(camProperties.rotation.y));
//	front.y =  sin(glm::radians(camProperties.rotation.x));
//	front.z =  cos(glm::radians(camProperties.rotation.x)) * cos(glm::radians(camProperties.rotation.y));
//	front = glm::normalize(front);
//
//	float move_speed = delta_time * camProperties.translation_speed * 2.0f;
//	float new_rotation_speed = delta_time * camProperties.rotation_speed * 50.0f;
//
//	// Move
//	if (fabsf(axis_left.y) > dead_zone)
//	{
//		float pos = (fabsf(axis_left.y) - dead_zone) / range;
//		camProperties.position -= front * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * move_speed;
//		changed = true;
//	}
//	if (fabsf(axis_left.x) > dead_zone)
//	{
//		float pos = (fabsf(axis_left.x) - dead_zone) / range;
//		camProperties.position += glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * move_speed;
//		changed = true;
//	}
//
//	// Rotate
//	if (fabsf(axis_right.x) > dead_zone)
//	{
//		float pos = (fabsf(axis_right.x) - dead_zone) / range;
//		camProperties.rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * new_rotation_speed;
//		changed = true;
//	}
//	if (fabsf(axis_right.y) > dead_zone)
//	{
//		float pos = (fabsf(axis_right.y) - dead_zone) / range;
//		camProperties.rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * new_rotation_speed;
//		changed = true;
//	}
//	
//
//	if (changed)
//	{
//		update_view_matrix();
//	}
//
//	return changed;
//}

Camera* Camera::m_Instance = nullptr;


Camera::Camera()
{

	Camera_Pos_ = glm::vec3(0.0f, 0.0f, 50.0f);
	Camera_Up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	Camera_Front_ = glm::vec3(0.0f, 0.0f, -1.0f);

	Camera_WorldUp_ = Camera_Up_;

	camera_move_speed_ = 0.75f;

	Yaw = -90.0f;
	Pitch = 0.0f;

	m_MouseSensitivity = 0.1f;

	CameraUpdate(0.0f);

#pragma region Presepective-Matrix

	float angle = 20.0f;
	float fov_ = glm::radians(angle);
	float AspectRatio = (4.0f / 3.0f);
	float NearPlane = 0.1f;
	float FarPlane = 10000.0f;

	matrices.perspective = glm::perspective(fov_, AspectRatio, NearPlane, FarPlane);

#pragma endregion


#pragma region Orthographic-Matrix

	float left = 0.0f;
	float right = WIDTH;
	float bottom = 0.0f;
	float top = HEIGHT;

	matrices.orthographic = glm::ortho(left, right, bottom, top, -1.0f, FarPlane);

#pragma endregion

}

Camera::~Camera()
{

}

Camera* Camera::getInstance()
{
	if (!m_Instance)
		m_Instance = new Camera();


	return m_Instance;
}

void Camera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
	matrices.perspective = glm::perspective(fov, aspect, znear, zfar);
}

void Camera::SetPosition(const glm::vec3& position)
{
	Camera_Pos_ = position;
}

void Camera::ProcessKeyBoardMovement(float a_fDeltaTime)
{
	float fVelocity = camera_move_speed_ * a_fDeltaTime;

	if (keys.forward)
	{
		Camera_Pos_ += Camera_Front_ * fVelocity;
	}

	else if (keys.backward)
	{
		Camera_Pos_ -= Camera_Front_ * fVelocity;
	}

	else if (keys.left)
	{
		Camera_Pos_ -= Camera_Right_ * fVelocity;
	}

	else if (keys.right)
	{
		Camera_Pos_ += Camera_Right_ * fVelocity;
	}

	else if (keys.down)
	{
		Camera_Pos_ -= Camera_Up_ * fVelocity;
	}

	else if (keys.up)
	{
		Camera_Pos_ += Camera_Up_ * fVelocity;
	}

	CameraUpdate(0.0f);
}

//===================================================================================
//CREDITS : learnopengl.com[JOEY DE VRIES]
//===================================================================================

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= m_MouseSensitivity;
	yoffset *= m_MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// TODO: add smooth movement for mouse and add delta time
	// Update Front, Right and Up Vectors using the updated Eular angles
	CameraUpdate(0.0f);
}

void Camera::CameraUpdate(float deltaTime)
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Camera_Front_ = glm::normalize(front);

	Camera_Right_ = glm::normalize(glm::cross(Camera_Front_, Camera_WorldUp_));
	Camera_Up_ = glm::normalize(glm::cross(Camera_Right_, Camera_Front_));


	matrices.view = glm::lookAt(Camera_Pos_, Camera_Pos_ + Camera_Front_, Camera_Up_);
}