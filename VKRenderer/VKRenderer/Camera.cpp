/* Copyright (c) 2019, Sascha Willems*/


#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
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
}