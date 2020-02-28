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

typedef struct TextureBufferDesc
{
	VkImage					BufferImage = VK_NULL_HANDLE;
	VkDeviceMemory			BufferMemory = VK_NULL_HANDLE;
	VkFormat				imageFormat;
	VkImageTiling			tiling;
	VkImageUsageFlags		usageFlags;
	VkMemoryPropertyFlags	propertyFlags;
	int						ImageWidth;
	int						ImageHeigth;
};

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

	CameraMatrices() :perspective(glm::mat4()), view(glm::mat4())
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
	//Camera's Position and Rotation
	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();

	float rotation_speed = 1.0f;
	float translation_speed = 1.0f;

	//Field of View
	float fov;
	//Near Plane and Far Plane
	float znear, zfar;

	//to check if the camera is updated
	bool updated;
};