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


