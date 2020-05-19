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


// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    
    CameraProperties camProperties;

    CameraMatrices matrices;

    // Constructor with vectors
    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();

    void SetPerspectiveMatrix(float fov, float aspect, float znear, float zfar);
	
	// returns the perspective matrix
    glm::mat4 GetPerspectiveMatrix();

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch);

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

	//Get the current camera position
	glm::vec3 GetCameraPosition();

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};