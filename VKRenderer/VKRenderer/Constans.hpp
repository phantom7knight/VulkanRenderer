#include "stdafx.h"


const int WIDTH		= 1600;
const int HEIGHT	= 900;


#define IMAGE_COUNT 3
const int MAX_FRAMES_IN_FLIGHT = 2;


typedef struct BufferDesc
{
	VkBuffer		Buffer = VK_NULL_HANDLE;

	VkDeviceMemory	BufferMemory = VK_NULL_HANDLE;

}Buffer;

typedef struct 
{
	VkImage					BufferImage = VK_NULL_HANDLE;
	VkDeviceMemory			BufferMemory = VK_NULL_HANDLE;
	VkFormat				imageFormat;
	VkImageTiling			tiling;
	VkImageUsageFlags		usageFlags;
	VkMemoryPropertyFlags	propertyFlags;
	int						ImageWidth = 0;
	int						ImageHeight = 0;
}TextureBufferDesc;

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;

	std::vector<VkSurfaceFormatKHR> formats;

	std::vector<VkPresentModeKHR> presentModes;

};


struct CameraMatrices
{
	glm::mat4 perspective;
	glm::mat4 view;

	CameraMatrices() :perspective(glm::mat4(1)), view(glm::mat4(1))
	{

	}
};

struct CameraKeys
{
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
} ;


struct CameraProperties
{
	////Camera's Position and Rotation
	//glm::vec3 rotation = glm::vec3();
	//glm::vec3 position = glm::vec3();
	//const glm::vec3 defPosition = glm::vec3(0.0, 0.0, -10.5);

	//float rotation_speed = 0.5f;
	//float translation_speed = 0.5f;

	
	////to check if the camera is updated
	//bool updated;

	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Euler Angles
	float Yaw;
	float Pitch;

	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	//Field of View
	float fov;
	//Near Plane and Far Plane
	float znear, zfar;

};


struct MousePositions
{
	float currentPosX = 0;
	float currentPosY = 0;
	float PrevPosX = 0;
	float PrevPosY = 0;
};
