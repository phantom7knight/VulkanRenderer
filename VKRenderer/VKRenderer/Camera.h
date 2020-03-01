/* Copyright (c) 2019, Sascha Willems*/


#pragma once
class Camera
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
};

