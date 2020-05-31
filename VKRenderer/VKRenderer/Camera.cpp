/* Copyright (c) 2019, Sascha Willems*/


#include "stdafx.h"
#include "Camera.h"

/*Camera::Camera()
{
	camProperties.updated = false;
	camProperties.rotation_speed = 1.0f;
	camProperties.translation_speed = 1.0f;

	camProperties.position = glm::vec3(0.0, -1.0, 0.0);
}

Camera::~Camera()
{
}


void Camera::update_view_matrix()
{
	glm::mat4 rotation_matrix = glm::mat4(1.0f);
	glm::mat4 transformation_matrix;

	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation_matrix = glm::rotate(rotation_matrix, glm::radians(camProperties.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	transformation_matrix = glm::translate(glm::mat4(1.0f), camProperties.position);

	matrices.view = transformation_matrix * rotation_matrix;
	//matrices.view = rotation_matrix * transformation_matrix;
	
	camProperties.updated = true;
}

bool Camera::moving()
{
	return keys.left || keys.right || keys.up || keys.down;
}

float Camera::get_near_clip()
{
	return camProperties.znear;
}

float Camera::get_far_clip()
{
	return camProperties.zfar;
}

void Camera::set_perspective(float fov, float aspect, float znear, float zfar)
{
	this->camProperties.fov = fov;
	this->camProperties.znear = znear;
	this->camProperties.zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
}

void Camera::update_aspect_ratio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(camProperties.fov), aspect, camProperties.znear, camProperties.zfar);
	//matrices.perspective = glm::ortho(0.f, 400.f, 0.f, 400.f, -1.f, 1.f);
}

void Camera::set_position(const glm::vec3& position)
{
	this->camProperties.position = position;
	update_view_matrix();
}

void Camera::set_rotation(const glm::vec3& rotation)
{
	this->camProperties.rotation = rotation;
	update_view_matrix();
}

void Camera::rotate(const glm::vec3& delta)
{
	this->camProperties.rotation += delta;
	update_view_matrix();
}

void Camera::set_translation(const glm::vec3& translation)
{
	this->camProperties.position = translation;
	update_view_matrix();
}

void Camera::translate(const glm::vec3& delta)
{
	this->camProperties.position += delta;
	update_view_matrix();
}

void Camera::update(float deltaTime)
{
	camProperties.updated = false;
	if (moving())
	{
		
		front.x = -cos(glm::radians(camProperties.rotation.x)) * sin(glm::radians(camProperties.rotation.y));
		front.y = sin(glm::radians(camProperties.rotation.x));
		front.z = cos(glm::radians(camProperties.rotation.x)) * cos(glm::radians(camProperties.rotation.y));
		front = glm::normalize(front);

		move_speed = deltaTime * camProperties.translation_speed;

		if (keys.up)
		{
			camProperties.position += front * move_speed;
		}
		if (keys.down)
		{
			camProperties.position -= front * move_speed;
		}
		if (keys.left)
		{
			camProperties.position -= glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
		}
		if (keys.right)
		{
			camProperties.position += glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
		}

		update_view_matrix();
	}
}

bool Camera::update_gamepad(glm::vec2 axis_left, glm::vec2 axis_right, float delta_time)
{
	bool changed = false;

	// Use the common console thumbstick layout
	// Left = view, right = move

	const float dead_zone = 0.0015f;
	const float range = 1.0f - dead_zone;

	glm::vec3 front;
	front.x = -cos(glm::radians(camProperties.rotation.x)) * sin(glm::radians(camProperties.rotation.y));
	front.y =  sin(glm::radians(camProperties.rotation.x));
	front.z =  cos(glm::radians(camProperties.rotation.x)) * cos(glm::radians(camProperties.rotation.y));
	front = glm::normalize(front);

	float move_speed = delta_time * camProperties.translation_speed * 2.0f;
	float new_rotation_speed = delta_time * camProperties.rotation_speed * 50.0f;

	// Move
	if (fabsf(axis_left.y) > dead_zone)
	{
		float pos = (fabsf(axis_left.y) - dead_zone) / range;
		camProperties.position -= front * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * move_speed;
		changed = true;
	}
	if (fabsf(axis_left.x) > dead_zone)
	{
		float pos = (fabsf(axis_left.x) - dead_zone) / range;
		camProperties.position += glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * move_speed;
		changed = true;
	}

	// Rotate
	if (fabsf(axis_right.x) > dead_zone)
	{
		float pos = (fabsf(axis_right.x) - dead_zone) / range;
		camProperties.rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * new_rotation_speed;
		changed = true;
	}
	if (fabsf(axis_right.y) > dead_zone)
	{
		float pos = (fabsf(axis_right.y) - dead_zone) / range;
		camProperties.rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * new_rotation_speed;
		changed = true;
	}
	

	if (changed)
	{
		update_view_matrix();
	}

	return changed;
}*/


//===========================================================================================

	//<summary>
	// Constructor with vectors
	//</summary>
	Camera::Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
	{
		camProperties.Front = glm::vec3(0.0f, 0.0f, -1.0f); 
		camProperties.MovementSpeed = SPEED;
		camProperties.MouseSensitivity = SENSITIVITY; 
		camProperties.Zoom = ZOOM;
		camProperties.Position = position;
		camProperties.WorldUp = up;
		camProperties.Yaw = yaw;
		camProperties.Pitch = pitch;
		updateCameraVectors();
	}

	/// <summary>
	///  Constructor with scalar values
	/// </summary>
	Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
	{
		camProperties.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		camProperties.MovementSpeed = SPEED;
		camProperties.MouseSensitivity = SENSITIVITY;
		camProperties.Zoom = ZOOM;
		camProperties.Position = glm::vec3(posX, posY, posZ);
		camProperties.WorldUp = glm::vec3(upX, upY, upZ);
		camProperties.Yaw = yaw;
		camProperties.Pitch = pitch;
		updateCameraVectors();
	}

	//<summary>
	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	//</summary>
	glm::mat4 Camera::GetViewMatrix()
	{
		return glm::lookAt(camProperties.Position, camProperties.Position + camProperties.Front, camProperties.Up);
	}

	//<summary>
	// Sets and Gets the perspective matrix
	//</summary>
	glm::mat4 Camera::SetGetPerspectiveMatrix(float fov, float aspect, float znear, float zfar)
	{
		this->camProperties.fov = fov;
		this->camProperties.znear = znear;
		this->camProperties.zfar = zfar;
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);

		return matrices.perspective;
	}

	glm::vec3 Camera::GetCameraPosition()
	{
		return camProperties.Position;
	}


	//<summary>
	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	//</summary>
	void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = camProperties.MovementSpeed * deltaTime;
		if (direction == FORWARD)
			camProperties.Position += camProperties.Front * velocity;
		if (direction == BACKWARD)
			camProperties.Position -= camProperties.Front * velocity;
		if (direction == LEFT)
			camProperties.Position -= camProperties.Right * velocity;
		if (direction == RIGHT)
			camProperties.Position += camProperties.Right * velocity;
	}

	//<summary>
	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	//</summary>
	void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= camProperties.MouseSensitivity;
		yoffset *= camProperties.MouseSensitivity;

		camProperties.Yaw	+= xoffset;
		camProperties.Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (!constrainPitch)
		{
			if (camProperties.Pitch > 89.0f)
				camProperties.Pitch = 89.0f;
			if (camProperties.Pitch < -89.0f)
				camProperties.Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	//<summary>
	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	//</summary>
	void Camera::ProcessMouseScroll(float yoffset)
	{
		if (camProperties.Zoom >= 1.0f && camProperties.Zoom <= 45.0f)
			camProperties.Zoom -= yoffset;
		if (camProperties.Zoom <= 1.0f)
			camProperties.Zoom = 1.0f;
		if (camProperties.Zoom >= 45.0f)
			camProperties.Zoom = 45.0f;
		
		// TODO: Fix it
		//camProperties.Position += camProperties.Zoom;;
	}

	//<summary>
	// Calculates the front vector from the Camera's (updated) Euler Angles
	//</summary>
	void Camera::updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		
		front.x = cos(glm::radians(camProperties.Yaw)) * cos(glm::radians(camProperties.Pitch));
		front.y = sin(glm::radians(camProperties.Pitch));
		front.z = sin(glm::radians(camProperties.Yaw)) * cos(glm::radians(camProperties.Pitch));
		
		camProperties.Front = glm::normalize(front);

		// Also re-calculate the Right and Up vector
		camProperties.Right = glm::normalize(glm::cross(camProperties.Front, camProperties.WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		camProperties.Up = glm::normalize(glm::cross(camProperties.Right, camProperties.Front));

	}