#include "../../PCH/stdafx.h"
#include "../../Helper/Validation Layer/ValidationLayer.hpp"


namespace VulkanHelper
{
	//===================================================================
	//Create Vulkan Instance
	//===================================================================
	bool CreateInstance(VkInstance *a_vulkanInstance)
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

		VkResult res = vkCreateInstance(&createInfo, nullptr, a_vulkanInstance);

		if (res != VK_SUCCESS)
		{
			std::cerr << "Failed to Initialize Instance" << std::endl;
			return false;
		}

		return true;
	}

	void CreateSurface(VkInstance a_vulkanInstance, GLFWwindow* a_window, VkSurfaceKHR	*a_surface)
	{
		// GLFW provides functionality of vkCreateWin32SurfaceKHR in itself so we use this instead
		if (glfwCreateWindowSurface(a_vulkanInstance, a_window, nullptr, a_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set Windows Surface");
		}

	}

	void SetupDebugMessenger(VkInstance a_vulkanInstance, VkDebugUtilsMessengerEXT	*a_debugMessenger)
	{
		if (!enableValidationLayer)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		if (CreateDebugUtilsMessengerEXT(a_vulkanInstance, &createInfo, nullptr, a_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to setup debug messenger");
		}


	}

	//Here we check for the Swap Chain capabilities
	//If the device supports swap chain functionality
	bool CheckDeviceExtenstionSupport(VkPhysicalDevice device)
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

	bool IsDeviceSuitable(VkPhysicalDevice a_device, VkSurfaceKHR a_surface)
	{

		QueueFamilyIndices indices = FindQueueFamilies(a_device, a_surface);

		bool extensionSupported = CheckDeviceExtenstionSupport(a_device);

		//We check if the Physical Device supports Swap Chain
		bool isSwapChainSupported = false;

		if (extensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(a_device,a_surface);
			isSwapChainSupported = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(a_device, &supportedFeatures);

		return indices.isComplete() && extensionSupported && isSwapChainSupported;

	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR	a_surface)
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
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, a_surface, &presentSupport);

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

	void PickPhysicalDevice(VkInstance a_vulkanInstance, VkPhysicalDevice &a_physicalDevice, VkSurfaceKHR a_surface)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(a_vulkanInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			//check if they support for Vulkan
			throw std::runtime_error("There aren't any GPU's which support Vulkan");
		}

		std::vector<VkPhysicalDevice> devicesList(deviceCount);
		vkEnumeratePhysicalDevices(a_vulkanInstance, &deviceCount, devicesList.data());

		for (const auto& device : devicesList)
		{
			if (IsDeviceSuitable(device, a_surface)) //Checks if they meet requirements of Vulkan
			{
				a_physicalDevice = device;
				break;
			}
		}

		if (a_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Couldn't find suitable GPU");
		}

	}

	//===================================================================
	//Logical Device
	//===================================================================
	VkDevice CreateLogicalDevice(VkPhysicalDevice a_physicalDevice, VkSurfaceKHR a_surface, VkQueue	*a_GraphicsQueue, VkQueue *a_PresentQueue)
	{
		VkDevice a_device;

		QueueFamilyIndices indices = FindQueueFamilies(a_physicalDevice, a_surface);

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

		if (vkCreateDevice(a_physicalDevice, &createInfo, nullptr, &a_device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Logical Device");
		}

		//Get the graphics queue
		int index_here = 0;//Since we have only 1 queue we give index 0 to it for graphics queue that is
		vkGetDeviceQueue(a_device, indices.graphicsFamily.value(), index_here, a_GraphicsQueue);
		vkGetDeviceQueue(a_device, indices.presentFamily.value(), index_here, a_PresentQueue);


		return a_device;

	}


	//===================================================================
	//Swap Chain
	//===================================================================
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice a_device, VkSurfaceKHR a_surface)
	{
		SwapChainSupportDetails details;

		//Get Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(a_device, a_surface, &details.capabilities);

		//Get Format
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(a_device, a_surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(a_device, a_surface, &formatCount, details.formats.data());

		}

		//Get Surface Present Mode
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(a_device, a_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(a_device, a_surface, &presentModeCount, details.presentModes.data());

		}


		return details;
	}

	//Here we set the swap surface format generally for color space
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * a_window)
	{

		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(a_window, &width, &height);

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
	void CreateSwapChain(VkPhysicalDevice a_physicalDevice, VkSurfaceKHR a_surface, GLFWwindow* a_window, VkDevice a_device,
						SwapChainDesc *a_swapChainDescription)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(a_physicalDevice, a_surface);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D swapExtent = ChooseSwapExtent(swapChainSupport.capabilities, a_window);

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
		createInfo.surface = a_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = swapExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		//createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

		QueueFamilyIndices indices = FindQueueFamilies(a_physicalDevice, a_surface);
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

		if (vkCreateSwapchainKHR(a_device, &createInfo, nullptr, &a_swapChainDescription->m_swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable/Failed to create Swap Chain");
		}

		vkGetSwapchainImagesKHR(a_device, a_swapChainDescription->m_swapChain, &imageCount, nullptr);
		a_swapChainDescription->m_SwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(a_device, a_swapChainDescription->m_swapChain, &imageCount, a_swapChainDescription->m_SwapChainImages.data());

		//store in mem variables
		a_swapChainDescription->m_swapChainFormat = surfaceFormat.format;
		a_swapChainDescription->m_swapChainExtent = swapExtent;


	}


	std::vector<VkImageView> CreateSwapChainImageView(SwapChainDesc	a_swapChainDescription, VkDevice a_device)
	{
		std::vector<VkImageView> SwapChainImageViews;
		
		SwapChainImageViews.resize(a_swapChainDescription.m_SwapChainImages.size());

		for (size_t i = 0; i < a_swapChainDescription.m_SwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo = { };

			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = a_swapChainDescription.m_SwapChainImages[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = a_swapChainDescription.m_swapChainFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(a_device, &createInfo, nullptr, &SwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Unable to create Image Views");
			}

		}

		return SwapChainImageViews;
	}



	//===================================================================
	//Command Buffer Recording Related
	//===================================================================

	VkCommandBuffer BeginSingleTimeCommands(VkDevice a_device, VkCommandPool a_commandPool)
	{
		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = a_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(a_device, &allocInfo, &commandBuffer);

		//Start recording Command Buffer

		VkCommandBufferBeginInfo bufferBeginInfo = {};

		bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);

		return commandBuffer;
	}

	//TODO: Check This
	void EndSingleTimeCommands(VkCommandBuffer *a_commandBuffer, VkCommandPool a_commandPool, VkDevice a_device, VkQueue a_graphicsQueue)
	{
		vkEndCommandBuffer(*a_commandBuffer);

		VkSubmitInfo submitInfo = {};

		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = a_commandBuffer;

		vkQueueSubmit(a_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(a_graphicsQueue);
		vkFreeCommandBuffers(a_device, a_commandPool, 1, a_commandBuffer);
	}


	void TransitionImageLayouts(VkDevice a_device, VkCommandPool a_commandPool, VkCommandBuffer* a_commandBuffer, VkQueue a_graphicsQueue,
		VkImage image, VkFormat format, VkImageLayout a_oldLayout, VkImageLayout a_newLayout)
	{
		VkCommandBuffer cmdBuffer = BeginSingleTimeCommands(a_device, a_commandPool);

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






		EndSingleTimeCommands(&cmdBuffer,a_commandPool,a_device, a_graphicsQueue);
	}


	//===================================================================
	//Buffer Description
	//===================================================================


	uint32_t FindMemoryType(VkPhysicalDevice a_physicalDevice, uint32_t typeFiler, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(a_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");


	}


	void CreateBuffer(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory)
	{
		/////For EX: Buffer for the coordinates of Triangle which are being sent to the VS
		VkBufferCreateInfo bufferCreateInfo = {};

		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = a_size;
		bufferCreateInfo.usage = a_usage;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(a_device, &bufferCreateInfo, nullptr, &a_buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to Create Buffer");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(a_device, a_buffer, &memRequirements);

		VkMemoryAllocateInfo allocateInfo = {};

		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memRequirements.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(a_physicalDevice,memRequirements.memoryTypeBits, a_properties);// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Allocated memory for the Vertex Buffer
		if (vkAllocateMemory(a_device, &allocateInfo, nullptr, &a_bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory for the Buffer");
		}
		vkBindBufferMemory(a_device, a_buffer, a_bufferMemory, 0);


	}


	void CopyBuffer(VkDevice a_device, VkCommandPool a_commandPool, VkQueue a_graphicsQueue, VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size)
	{
		
		VkCommandBuffer a_cmdBuffer = BeginSingleTimeCommands(a_device, a_commandPool);

		VkBufferCopy copyRegion = {};

		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = a_size;

		vkCmdCopyBuffer(a_cmdBuffer, a_srcBuffer, a_dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(&a_cmdBuffer, a_commandPool, a_device, a_graphicsQueue);

		

	}


	void CopyBufferToImage(VkDevice a_device, VkCommandPool a_commandPool, VkQueue a_graphicsQueue, VkBuffer buffer, TextureBufferDesc desc)
	{
		VkCommandBuffer cmdBuffer = BeginSingleTimeCommands(a_device, a_commandPool);

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



		EndSingleTimeCommands(&cmdBuffer, a_commandPool, a_device, a_graphicsQueue);
	}

}