
#include "vkRenderer.h"
#include "ValidationLayer.hpp"
#include "ResourceLoader.h"
#include "Camera.h"



//We add Swap Chain Extenstion to the Current Device
const std::vector<const char*> deviceExtenstion = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


VkInstance vkRenderer::getVulkanInstance()
{
	return m_VulkanInstance;
}

VkDebugUtilsMessengerEXT vkRenderer::getDebugMessenger()
{
	return m_debugMessenger;
}


vkRenderer::vkRenderer() :m_MainCamera(new Camera())
{
}

vkRenderer::~vkRenderer()
{
}

void vkRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<vkRenderer*>(glfwGetWindowUserPointer(window));
	app->m_frameBufferResized = true;
}

static void MousePosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	auto app = reinterpret_cast<vkRenderer*>(glfwGetWindowUserPointer(window));

	app->mousePos.currentPosX = (float)xpos;
	app->mousePos.currentPosY = (float)ypos;

	int dx = (int)(app->mousePos.PrevPosX - app->mousePos.currentPosX);
	int dy = (int)(-app->mousePos.PrevPosY + app->mousePos.currentPosY);

	//if left mouse button pressed
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		app->m_MainCamera->camProperties.rotation.x += dy * app->m_MainCamera->camProperties.rotation_speed;
		app->m_MainCamera->camProperties.rotation.y -= dx * app->m_MainCamera->camProperties.rotation_speed;
		app->m_MainCamera->camProperties.rotation += glm::vec3(dy * app->m_MainCamera->camProperties.rotation_speed, -dx * app->m_MainCamera->camProperties.rotation_speed, 0.0);
		app->m_MainCamera->update_view_matrix();
	}

	app->mousePos.PrevPosX	=	app->mousePos.currentPosX;
	app->mousePos.PrevPosY	=	app->mousePos.currentPosY;

}

bool vkRenderer::InitGLFW()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "VkRenderer", nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	glfwSetCursorPosCallback(m_window, MousePosCallBack);

	if (!m_window)
		return false;

	return true;
}

//===================================================================
//Buffer Description
//===================================================================

uint32_t vkRenderer::findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");


}


void vkRenderer::CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory)
{
	/////For EX: Buffer for the coordinates of Triangle which are being sent to the VS
	VkBufferCreateInfo bufferCreateInfo = {};

	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = a_size;
	bufferCreateInfo.usage = a_usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &a_buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to Create Buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, a_buffer, &memRequirements);

	VkMemoryAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, a_properties);// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//Allocated memory for the Vertex Buffer
	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &a_bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for the Buffer");
	}
	vkBindBufferMemory(m_device, a_buffer, a_bufferMemory, 0);


}


void vkRenderer::CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size)
{
	//VkCommandBufferAllocateInfo allocInfo = {};
	//
	//allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//allocInfo.commandPool = m_CommandPool;
	//allocInfo.commandBufferCount = 1;
	//
	//VkCommandBuffer commandBuffer;
	//vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
	//
	////Start recording Command Buffer
	//
	//VkCommandBufferBeginInfo bufferBeginInfo = {};
	//
	//bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//
	//vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);

	VkCommandBuffer a_cmdBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion = {};

	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = a_size;

	vkCmdCopyBuffer(a_cmdBuffer, a_srcBuffer, a_dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(a_cmdBuffer);

	// vkEndCommandBuffer(commandBuffer);
	// 
	// VkSubmitInfo submitInfo = {};
	// 
	// submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// submitInfo.commandBufferCount = 1;
	// submitInfo.pCommandBuffers = &commandBuffer;
	// 
	// vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	// vkQueueWaitIdle(m_graphicsQueue);
	// vkFreeCommandBuffers(m_device, m_CommandPool, 1, &commandBuffer);


}


void vkRenderer::CopyBufferToImage(VkBuffer buffer, TextureBufferDesc desc)
{
	VkCommandBuffer cmdBuffer = BeginSingleTimeCommands();

	VkBufferImageCopy region = {};

	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0,0,0 };
	region.imageExtent = { static_cast<uint32_t>(desc.ImageWidth), static_cast<uint32_t>(desc.ImageHeight), 1 };

	vkCmdCopyBufferToImage(cmdBuffer, buffer, desc.BufferImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);



	EndSingleTimeCommands(cmdBuffer);
}

//===================================================================
//Create Vulkan Instance
//===================================================================
bool vkRenderer::CreateInstance()
{

	if (enableValidationLayer && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Validation layer requested but not available!");
	}


	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Hello World!!";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VkRenderer";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);// VK_API_VERSION_1_1;


	//Used to mention the validation layers which we want to use
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extenstions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extenstions.size());
	createInfo.ppEnabledExtensionNames = extenstions.data();

	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	VkResult res = vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance);

	if (res != VK_SUCCESS)
	{
		std::cerr << "Failed to Initialize Instance" << std::endl;
		return false;
	}

	return true;
}

//===================================================================
//Debug Related
//===================================================================

void vkRenderer::setupDebugMessenger()
{
	if (!enableValidationLayer)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	
	createInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback	= debugCallback;
	createInfo.pUserData		= nullptr;

	if (CreateDebugUtilsMessengerEXT(m_VulkanInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger");
	}


}

//Here we check for the Swap Chain capabilities
//If the device supports swap chain functionality
bool checkDeviceExtenstionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	
	std::set<std::string> requiredExtensions(deviceExtenstion.begin(), deviceExtenstion.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool vkRenderer::isDeviceSuitable(VkPhysicalDevice a_device)
{

	QueueFamilyIndices indices = findQueueFamilies(a_device);

	bool extensionSupported = checkDeviceExtenstionSupport(a_device);

	//We check if the Physical Device supports Swap Chain
	bool isSwapChainSupported = false;

	if (extensionSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(a_device);
		isSwapChainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(a_device, &supportedFeatures);

	return indices.isComplete() && extensionSupported && isSwapChainSupported;
	
}

QueueFamilyIndices vkRenderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

//===================================================================
//Physical Device
//===================================================================

void vkRenderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		//check if they support for Vulkan
		throw std::runtime_error("There aren't any GPU's which support Vulkan");
	}

	std::vector<VkPhysicalDevice> devicesList(deviceCount);
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devicesList.data());

	for (const auto& device : devicesList)
	{
		if (isDeviceSuitable(device))//Checks if they meet requirements of Vulkan
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Couldn't find suitable GPU");
	}

}


//===================================================================
//Logical Device
//===================================================================
void vkRenderer::CreateLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queuecreateInfos;
	std::set<uint32_t>uniqueQueueFamilies = { indices.graphicsFamily.value(),indices.presentFamily.value() };

	float queueProirity = 1.0f;
	for (uint32_t queuefamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo  queuecreateInfo = {};
		queuecreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queuecreateInfo.queueFamilyIndex = queuefamily;
		queuecreateInfo.queueCount = 1; //For now which can be later adjusted as per queue's requirements
		queuecreateInfo.pQueuePriorities = &queueProirity;
		queuecreateInfos.push_back(queuecreateInfo);
	}

	//Features which we will be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	//Device Info
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queuecreateInfos.size());
	createInfo.pQueueCreateInfos = queuecreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	
	//We mention the Swap Chain info to the device
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtenstion.size());
	createInfo.ppEnabledExtensionNames = deviceExtenstion.data();


	if (enableValidationLayer)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	//We now create Device

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Logical Device");
	}

	//Get the graphics queue
	int index_here = 0;//Since we have only 1 queue we give index 0 to it for graphics queue that is
	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), index_here, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), index_here, &m_PresentQueue);

}

//===================================================================
//Create Surface
//===================================================================
void vkRenderer::CreateSurface()
{
	// GLFW provides functionality of vkCreateWin32SurfaceKHR in itself so we use this instead
	if (glfwCreateWindowSurface(m_VulkanInstance, m_window, nullptr, &m_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set Windows Surface");
	}

}

//===================================================================
//Swap Chain
//===================================================================
SwapChainSupportDetails vkRenderer::querySwapChainSupport(VkPhysicalDevice a_device)
{
	SwapChainSupportDetails details;

	//Get Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(a_device, m_surface, &details.capabilities);

	//Get Format
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(a_device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(a_device, m_surface, &formatCount, details.formats.data());

	}

	//Get Surface Present Mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(a_device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(a_device, m_surface, &presentModeCount, details.presentModes.data());

	}


	return details;
}

//Here we set the swap surface format generally for color space
VkSurfaceFormatKHR vkRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//Set the default Color Space
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_SNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	//Set the available combination as mentioned below
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}
	 
	//else set the default case
	return availableFormats[0];
}

//Here we set the swap present mode
VkPresentModeKHR vkRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
	
}

//Here we set the swap extent
VkExtent2D vkRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else 
	{
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

}

//Create Swap Chain itself
void vkRenderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat	= chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode		= chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D swapExtent				= chooseSwapExtent(swapChainSupport.capabilities);

	//We set how many back buffer images we need
	uint32_t imageCount = IMAGE_COUNT;// swapChainSupport.capabilities.minImageCount;

	//TODO: check
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		//Set the most possible count
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = swapExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	//Create Swap Chain

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable/Failed to create Swap Chain");
	}

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_SwapChainImages.data());

	//store in mem variables
	m_swapChainFormat = surfaceFormat.format;
	m_swapChainExtent = swapExtent;


}



//===================================================================
// Creating Image Views[Used to view Images]
//===================================================================
void vkRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *a_imageView)
{
	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.format = format;
	createInfo.pNext = nullptr;
	createInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_device, &createInfo, nullptr, a_imageView) != VK_SUCCESS)
	{
		std::cout << "Unable Image view for the texture provided \n";
		return;
	}

}


//TODO: REMOVE IT AND ADD THE ABOVE FUNCTION AS DEFAULT
void vkRenderer::CreateSwapChainImageView()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());

	for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo = { };

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create Image Views");
		}

	}

}


//===================================================================
//Command Buffer Recording Related
//===================================================================

VkCommandBuffer vkRenderer::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	//Start recording Command Buffer

	VkCommandBufferBeginInfo bufferBeginInfo = {};

	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);

return commandBuffer;
}

void vkRenderer::EndSingleTimeCommands(VkCommandBuffer a_commandBuffer)
{
	vkEndCommandBuffer(a_commandBuffer);

	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &a_commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);
	vkFreeCommandBuffers(m_device, m_CommandPool, 1, &a_commandBuffer);
}


void vkRenderer::TransitionImageLayouts(VkImage image, VkFormat format, VkImageLayout a_oldLayout, VkImageLayout a_newLayout)
{
	VkCommandBuffer cmdBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	barrier.oldLayout = a_oldLayout;
	barrier.newLayout = a_newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	//these commands ahead say which part of image are effected
	//and which properties are effected[image & subresourceRange].
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (a_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && a_newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;


		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	}
	else if (a_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && a_newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;


		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	//else if (a_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && a_newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
	//{
	//	barrier.srcAccessMask = 0;
	//	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	//
	//	sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	//	destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//}
	else
	{
		std::cout << "Unsupported Layout Transition! \n";
	}


	vkCmdPipelineBarrier(
		cmdBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);






	EndSingleTimeCommands(cmdBuffer);
}

//===================================================================
//GLFW Input recording
//===================================================================
void vkRenderer::ProcessInput(GLFWwindow* window)
{
	//If Esc button is pressed we close
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		m_MainCamera->camProperties.position = m_MainCamera->camProperties.defPosition;
	}

	//Update Keys pressed status for the camera update
	m_MainCamera->keys.up		= glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.down		= glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.right	= glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.left		= glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? true : false;

}



//===================================================================
//Depth related helpers
//===================================================================
VkFormat vkRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (auto format : candidates)
	{
		VkFormatProperties properties_here;

		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties_here);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties_here.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties_here.optimalTilingFeatures & features) == features)
			return format;
		else
			throw std::runtime_error ("Failed to find appropriate format");
	}

	return VkFormat();
}

VkFormat vkRenderer::FindDepthFormat()
{
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
}

bool vkRenderer::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}



//===================================================================
//Vulkan Initialization Function
//===================================================================

void vkRenderer::InitializeVulkan()
{

	CreateInstance();

	CreateSurface();

	setupDebugMessenger();

	pickPhysicalDevice();

	CreateLogicalDevice();

}

//===================================================================
//Renderer Init
//===================================================================

void vkRenderer::Init()
{

	if (!InitGLFW())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}

	InitializeVulkan();



}

//===================================================================
//Prepare Base renderer class
//===================================================================

void vkRenderer::SetUpSwapChain()
{
	CreateSwapChain();
}

void vkRenderer::PrepareApp()
{
	SetUpSwapChain();
	CreateSwapChainImageView();
}


//===================================================================
//Deinitializers
//===================================================================

void vkRenderer::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator )
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VulkanInstance, "vkDestroyDebugUtilsMessengerEXT");
		
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}


void vkRenderer::CleanUpSwapChain()
{

	
	for (size_t i = 0; i < m_swapChainFrameBuffer.size(); ++i)
	{
		vkDestroyFramebuffer(m_device, m_swapChainFrameBuffer[i], nullptr);
	}

	vkFreeCommandBuffers(m_device, m_CommandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
	{
		vkDestroyImageView(m_device, m_SwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

	for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
	{
		//Triangle's UBO
		vkDestroyBuffer(m_device, m_TriangleUniformBuffer[i].Buffer, nullptr);
		vkFreeMemory(m_device, m_TriangleUniformBuffer[i].BufferMemory, nullptr);
		
		//Model's UBO
		vkDestroyBuffer(m_device, m_ModelUniformBuffer[i].Buffer, nullptr);
		vkFreeMemory(m_device, m_ModelUniformBuffer[i].BufferMemory, nullptr);
	}

	vkDestroyDescriptorPool(m_device, m_DescriptorPool, nullptr);

}


void vkRenderer::Destroy()
{
	//=====================================================================
	//Delete Vulkan related things[Generalize when deleting the resources]
	//=====================================================================

	CleanUpSwapChain();


	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

	//Destroy Rectangle Index Buffer
	vkDestroyBuffer(m_device, m_RectangleIndexBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, m_RectangleIndexBuffer.BufferMemory, nullptr);

	//Destroy Triangle Index Buffer
	vkDestroyBuffer(m_device, m_TriangleVertexBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, m_TriangleVertexBuffer.BufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
		vkDestroyFence(m_device, m_inflightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_device, m_CommandPool, nullptr);

	vkDestroyDevice(m_device,nullptr);

	if (enableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_VulkanInstance, m_surface, nullptr);
	
	vkDestroyInstance(m_VulkanInstance,nullptr);
	
	glfwDestroyWindow(m_window);

	glfwTerminate();

	//Destroy Camera
	if (m_MainCamera != NULL)
	{
		delete m_MainCamera;
	}
	
}

