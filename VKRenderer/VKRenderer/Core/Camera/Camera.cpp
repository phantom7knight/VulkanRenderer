#include"../PCH/stdafx.h"
#include "Camera.h"

Camera::Camera()
{

	Camera_Pos_ = glm::vec3(0.0f, 0.0f, 50.0f);
	Camera_Up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	Camera_Front_ = glm::vec3(0.0f, 0.0f, -1.0f);

	Camera_WorldUp_ = Camera_Up_;

	camera_move_speed_ = 0.05f;

	Yaw = -90.0f;
	Pitch = 0.0f;

	m_MouseSensitivity = 0.1f;

	CameraUpdate();

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
	float fVelocity = camera_move_speed_ * 0.05f;

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

	else if (keys.up)
	{
		Camera_Pos_ -= Camera_Up_ * fVelocity;
	}

	else if (keys.down)
	{
		Camera_Pos_ += Camera_Up_ * fVelocity;
	}

	CameraUpdate();
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
	CameraUpdate();
}

void Camera::CameraUpdate()
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