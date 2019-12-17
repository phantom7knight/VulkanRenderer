
#include "vkRenderer.h"
#include "ValidationLayer.hpp"
#include "ResourceLoader.h"



//===================================================================
//Vertex Buffer Use
//===================================================================

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normals;
	glm::vec2 TexCoords;


	//Set Vertex Binding Desc i.e like setting VAO in OpenGL

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc = {};

		bindingDesc.binding = 0;	//Since we are using only one array for the data that is Triangle_vertices we have 1 binding and order starts from 0
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}


	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptionsofVertex()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDesc = {};

		//Position
		attributeDesc[0].binding = 0;
		attributeDesc[0].location = 0;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[0].offset = offsetof(Vertex, Position);

		//Normals
		attributeDesc[1].binding = 0;
		attributeDesc[1].location = 1;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[1].offset = offsetof(Vertex, Normals);

		//TexCoords
		attributeDesc[2].binding = 0;
		attributeDesc[2].location = 2;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[2].offset = offsetof(Vertex, TexCoords);


		return attributeDesc;
	}



};


const std::vector<Vertex> Triangle_vertices = {
	{	{0.0,-0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5, 0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{-0.5,0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	}
};

const std::vector<Vertex> Rectangle_vertices = {
	{	{-0.5,-0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5, -0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5,0.5,0.0},			{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{-0.5,0.5,0.0},			{0.0,0.0,1.0},		{0.0,0.0}	}
};

const std::vector<uint16_t> Rectangle_Indices = {
	0,1,2,2,3,0
};
 

struct UniformBufferObject
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};



//===================================================================




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


vkRenderer::vkRenderer()
{
}

vkRenderer::~vkRenderer()
{
}


//TODO: Fix this
void vkRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	//vkRenderer* temp = *this;// vkRenderer::getInstance();
	//(glfwGetWindowUserPointer(window));
	//temp->m_frameBufferResized = true;
}

bool vkRenderer::InitGLFW()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "VkRenderer", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

	if (!m_window)
		return false;

	return true;
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

void vkRenderer::CreateImageView()
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
// Create and Set Descriptor Layouts[maybe generalize?]
//===================================================================

//void vkRenderer::CreateDescriptorSetLayout()
//{
//	VkDescriptorSetLayoutBinding layoutBinding = {};
//
//	layoutBinding.binding = 0;
//	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	layoutBinding.descriptorCount = 1;
//	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//	layoutBinding.pImmutableSamplers = nullptr;
//
//	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
//
//	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//	layoutInfo.bindingCount = 1;
//	layoutInfo.pBindings = &layoutBinding;
//
//	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create Descriptor Set Layout");
//	}
//	   
//}


//===================================================================
//Create Graphics Pipeline and Shader Related Functions
//===================================================================

/*static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to read file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}*/

/*VkShaderModule vkRenderer::createShaderModule(const std::vector<char>& shaderCode)
{
	VkShaderModuleCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shader Module");
	}

	return shaderModule;

}*/

/*void vkRenderer::CreateGraphicsPipeline()
{

	//=============================================
	//generate binary code

	//Compile Shader


	//=============================================

	//Read Shader Binary Code
	auto VertexShaderCode	= readFile("Shaders/BinaryCode/Basic.vert.spv");
	auto PixelShaderCode	= readFile("Shaders/BinaryCode/Basic.frag.spv");

	//Generate respective Shader Modules
	VkShaderModule vertexShaderModule = createShaderModule(VertexShaderCode);
	VkShaderModule pixelShaderModule = createShaderModule(PixelShaderCode);

	//Vertex Shader Pipeline
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};

	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderCreateInfo.module = vertexShaderModule;
	vertexShaderCreateInfo.pName = "main";

	//Pixel Shader Pipeline
	VkPipelineShaderStageCreateInfo pixelShaderCreateInfo = {};

	pixelShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pixelShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pixelShaderCreateInfo.module = pixelShaderModule;
	pixelShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo ,pixelShaderCreateInfo };


	// Vertex Input
	VkPipelineVertexInputStateCreateInfo VertexInputInfo = {};

	VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDesc = Vertex::getBindingDescription();
	auto attributeDesc = Vertex::getAttributeDescriptionsofVertex();

	VertexInputInfo.vertexBindingDescriptionCount = 1;
	VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size());
	VertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	VertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();


	//Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};

	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//TODO :This can be configurable by user
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//View Ports
	VkViewport viewPort = {};//TODO :This can be configurable by user
	
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = (float)m_swapChainExtent.width;
	viewPort.height = (float)m_swapChainExtent.height;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	//Scissors
	VkRect2D scissor = {};

	scissor.offset = { 0,0 };
	scissor.extent = m_swapChainExtent;

	VkPipelineViewportStateCreateInfo viewPortState = {};

	viewPortState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewPortState.viewportCount = 1;
	viewPortState.pViewports = &viewPort;
	viewPortState.scissorCount = 1;
	viewPortState.pScissors = &scissor;

	//Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//TODO :This can be configurable by user
	rasterizer.lineWidth = 1.0f;//TODO :This can be configurable by user
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//TODO: look into this later, since these are optional
	rasterizer.depthBiasClamp = 0.0f;			//TODO: look into this later, since these are optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		//TODO: look into this later, since these are optional


	//Multisampling
	VkPipelineMultisampleStateCreateInfo multiSampling = {};

	multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampling.sampleShadingEnable = VK_FALSE;
	multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampling.minSampleShading = 1.0f;
	multiSampling.pSampleMask = nullptr;
	multiSampling.alphaToCoverageEnable = VK_FALSE;
	multiSampling.alphaToOneEnable = VK_FALSE;

	//Depth Testing and Stencil Testing
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};

	//Color Blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};//For individual attached frame buffer settings

	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;//TODO : This can be changed later look at the website
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};

	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	//Create Pipeline Layout b4 creating Graphics Pipeline
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout");
	}

	//Finally create the Graphics Pipeline
	VkGraphicsPipelineCreateInfo createGraphicsPipelineInfo = {};

	createGraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createGraphicsPipelineInfo.stageCount = 2;
	createGraphicsPipelineInfo.pStages = shaderStages;
	createGraphicsPipelineInfo.pVertexInputState = &VertexInputInfo;
	createGraphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
	createGraphicsPipelineInfo.pViewportState = &viewPortState;
	createGraphicsPipelineInfo.pRasterizationState = &rasterizer;
	createGraphicsPipelineInfo.pMultisampleState = &multiSampling;
	createGraphicsPipelineInfo.pDepthStencilState = nullptr;
	createGraphicsPipelineInfo.pColorBlendState = &colorBlending;
	createGraphicsPipelineInfo.pDynamicState = nullptr;
	createGraphicsPipelineInfo.layout = m_pipelineLayout;
	createGraphicsPipelineInfo.renderPass = m_renderPass;
	createGraphicsPipelineInfo.subpass = 0;

	createGraphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	createGraphicsPipelineInfo.basePipelineIndex = -1;

	//TODO: Make this Generic for creating Pipeline's
	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createGraphicsPipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to Create Graphics Pipeline");
	}


	//Destroy all shader modules

	vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_device, pixelShaderModule, nullptr);

}*/

//===================================================================
//Creating Render Pass
//TODO :Can be made generic
//===================================================================

//void vkRenderer::CreateRenderPass()
//{
//	VkAttachmentDescription colorAttachment = {};
//	
//	colorAttachment.format = m_swapChainFormat;
//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//TODO : Programmable
//
//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//
//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass shud start with
//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//How render pass final image shud translate at end of render pass
//
//
//	//Each renderpass can have multiple sub-passes
//	//which will help or can be used for the Post-Processing,...etc
//
//	VkAttachmentReference colorAttachmentRef = {};
//
//	colorAttachmentRef.attachment = 0;
//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//	VkSubpassDescription subpassInfo = {};
//	
//	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpassInfo.colorAttachmentCount = 1;					//layout(location = 0) out vec4 outColor this is where it will be referenced
//	subpassInfo.pColorAttachments = &colorAttachmentRef;
//
//	VkSubpassDependency dependency = {};
//
//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependency.dstSubpass = 0;
//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.srcAccessMask = 0;
//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//
//		
//	//Render Pass Info
//
//	VkRenderPassCreateInfo renderpassInfo = {};
//
//	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderpassInfo.attachmentCount = 1;
//	renderpassInfo.pAttachments = &colorAttachment;
//	renderpassInfo.subpassCount = 1;
//	renderpassInfo.pSubpasses = &subpassInfo;
//	renderpassInfo.dependencyCount = 1;
//	renderpassInfo.pDependencies = &dependency;
//
//	if (vkCreateRenderPass(m_device, &renderpassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to create Render Pass");
//	}
//
//
//}

//===================================================================
//Creating Frame Buffers
//===================================================================

//void vkRenderer::CreateFrameBuffers()
//{
//	m_swapChainFrameBuffer.resize(m_SwapChainImageViews.size());
//
//	for (uint32_t i = 0; i < m_SwapChainImageViews.size(); ++i)
//	{
//		VkImageView attachments[] = { m_SwapChainImageViews[i] };
//
//		VkFramebufferCreateInfo fbcreateInfo = {};
//
//		fbcreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		fbcreateInfo.renderPass = m_renderPass;
//		fbcreateInfo.attachmentCount = 1;
//		fbcreateInfo.pAttachments = attachments;
//		fbcreateInfo.width = m_swapChainExtent.width;
//		fbcreateInfo.height = m_swapChainExtent.height;
//		fbcreateInfo.layers = 1;
//
//		if (vkCreateFramebuffer(m_device, &fbcreateInfo, nullptr, &m_swapChainFrameBuffer[i]) != VK_SUCCESS)
//		{
//			throw std::runtime_error("Unable to create Frame Buffer");
//		}
//	}
//
//}
//

//===================================================================
//Command Pool
//===================================================================

//void vkRenderer::CreateCommandPool()
//{
//	QueueFamilyIndices queuefamilyindeces = findQueueFamilies(m_physicalDevice);
//
//	VkCommandPoolCreateInfo createInfo = {};
//
//	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//	createInfo.queueFamilyIndex = queuefamilyindeces.graphicsFamily.value();
//	createInfo.flags = 0;
//
//	if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to create Command Pool");
//	}
//}

//===================================================================
//Command Buffers
//===================================================================

//void vkRenderer::CreateCommandBuffers()
//{
//	m_commandBuffers.resize(m_swapChainFrameBuffer.size());
//
//	VkCommandBufferAllocateInfo createInfo = {};
//
//	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	createInfo.commandPool = m_CommandPool;
//	createInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();
//
//
//	if (vkAllocateCommandBuffers(m_device, &createInfo,m_commandBuffers.data()) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to create Command Buffers");
//	}
//
//	//Record Command Buffer data
//	for (size_t i = 0; i < m_commandBuffers.size(); ++i)
//	{
//		VkCommandBufferBeginInfo beginInfo = {};
//
//		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
//		beginInfo.pInheritanceInfo = nullptr;
//
//		if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
//		{
//			throw std::runtime_error("Unable to begin recording Command Buffer");
//		}
//
//		VkRenderPassBeginInfo renderpassBeginInfo = {};
//		
//		renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		renderpassBeginInfo.renderPass = m_renderPass;
//		renderpassBeginInfo.framebuffer = m_swapChainFrameBuffer[i];
//		renderpassBeginInfo.renderArea.offset = { 0,0 };
//		renderpassBeginInfo.renderArea.extent = m_swapChainExtent;
//
//
//		//Clear Color//
//		VkClearValue clearColor = { 0.0,0.0,0.0,1.0 };
//		renderpassBeginInfo.clearValueCount = 1;
//		renderpassBeginInfo.pClearValues = &clearColor;
//		
//
//		vkCmdBeginRenderPass(m_commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
//
//			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
//
//			VkBuffer vertexBuffers[] = { m_TriangleVertexBuffer };
//			VkDeviceSize offset = { 0 };
//			vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &m_TriangleVertexBuffer, &offset);
//
//			vkCmdBindIndexBuffer(m_commandBuffers[i], m_RectangleIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
//
//			//vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(Triangle_vertices.size()), 1, 0, 0);
//
//			vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);
//
//			vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(Rectangle_Indices.size()), 1, 0, 0, 0);
//
//		vkCmdEndRenderPass(m_commandBuffers[i]);
//
//		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
//		{
//			throw std::runtime_error("Failed to record Command Buffer");
//		}
//	
//	}
//}
//

//===================================================================
// Semaphore
//===================================================================

//void vkRenderer::CreateSemaphoresandFences()
//{
//
//	m_imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
//	m_renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
//	m_inflightFences.resize(MAX_FRAMES_IN_FLIGHT);
//
//
//	VkSemaphoreCreateInfo createSemaphoreInfo = {};
//
//	createSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//	VkFenceCreateInfo createFenceInfo = {};
//
//	createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//	createFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
//	{
//		if (vkCreateSemaphore(m_device, &createSemaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS ||
//			vkCreateSemaphore(m_device, &createSemaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS ||
//			vkCreateFence(m_device, &createFenceInfo, nullptr, &m_inflightFences[i]) != VK_SUCCESS )
//		{
//			throw std::runtime_error("Failed to create Semaphore");
//		}
//	}
//
//	
//
//}


//===================================================================
//Recreating Swap Chains
//===================================================================

//TODO: Fix this

//void vkRenderer::ReCreateSwapChain()
//{
//
//	int width = 0, height = 0;
//	while (width == 0 || height == 0)
//	{
//		glfwGetFramebufferSize(m_window, &width, &height);
//		glfwWaitEvents();
//	}
//
//
//
//	vkDeviceWaitIdle(m_device);
//
//	CleanUpSwapChain();
//
//	CreateSwapChain();
//
//	CreateImageView();
//
//	CreateRenderPass();
//
//	CreateGraphicsPipeline(); 
//
//	CreateFrameBuffers();
//	
//	CreateUniformBuffer();
//
//	CreateCommandBuffers();
//
//	//CreateDescriptorPool();
//	//
//	//CreateDesciptorSets();
//
//	CreateCommandPool();
//	
//
//}

//===================================================================
//Creaate Vertex Buffer
//===================================================================

//uint32_t vkRenderer::findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties)
//{
//	VkPhysicalDeviceMemoryProperties memProperties;
//	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
//
//	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
//	{
//		if (typeFiler&(1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
//		{
//			return i;
//		}
//	}
//
//	throw std::runtime_error("Failed to find suitable memory type!");
//
//
//}


//void vkRenderer::CreateVertexBuffer()//Make this Generic
//{
//
//	VkDeviceSize bufferSize = sizeof(Rectangle_vertices[0]) * Rectangle_vertices.size();
//
//	//Create Staging Buffer before transfering
//	VkBuffer stagingBuffer;
//	VkDeviceMemory stagingBufferMemory;
//	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//		stagingBuffer, stagingBufferMemory);
//
//
//	void* data;
//	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
//		memcpy(data, Rectangle_vertices.data(), (size_t)bufferSize);
//	vkUnmapMemory(m_device, stagingBufferMemory);
//
//
//	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//				 m_TriangleVertexBuffer, m_vertexBufferMemory);
//
//	CopyBuffer(stagingBuffer, m_TriangleVertexBuffer, bufferSize);
//
//	//Get rid of the staging buffers
//	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
//	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
//	
//	
//}

//===================================================================
//Create Index Buffer
//===================================================================

//void vkRenderer::CreateIndexBuffer()
//{
//	VkDeviceSize bufferSize = sizeof(Rectangle_Indices[0]) * Rectangle_Indices.size();
//
//	//Create Staging Buffer before transfering
//	VkBuffer stagingBuffer;
//	VkDeviceMemory stagingBufferMemory;
//	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//		stagingBuffer, stagingBufferMemory);
//
//
//	void* data;
//	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
//	
//		memcpy(data, Rectangle_Indices.data(), (size_t)bufferSize);
//	
//	vkUnmapMemory(m_device, stagingBufferMemory);
//
//
//	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//				 m_RectangleIndexBuffer, m_IndexBufferMemory);
//
//	CopyBuffer(stagingBuffer, m_RectangleIndexBuffer, bufferSize);
//
//	//Get rid of the staging buffers
//	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
//	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
//}

//=====================================================================================================================
//Create & Update Uniform Buffer[Generalize this]
//=====================================================================================================================

//void vkRenderer::CreateUniformBuffer()
//{
//	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//
//	m_uniformBuffers.resize(m_SwapChainImages.size());
//	m_uniformBuffersMemory.resize(m_SwapChainImages.size());
//
//	for (int i = 0; i < m_SwapChainImages.size(); ++i)
//	{
//		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i],
//					 m_uniformBuffersMemory[i]);
//	}
//
//
//}
// 

//void vkRenderer::UpdateUniformBuffer(uint32_t a_imageIndex, float a_deltaTime)
//{
//	UniformBufferObject mvp_UBO = {};
//
//	//Model Matrix
//	mvp_UBO.ModelMatrix = glm::rotate(glm::mat4(1.0f), a_deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//
//
//	//View Matrix
//	mvp_UBO.ViewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
//
//
//	//Projection Matrix
//	mvp_UBO.ProjectionMatrix = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
//	mvp_UBO.ProjectionMatrix[1][1] *= -1;
//
//
//	//Copy the data
//
//	void* data;
//
//	vkMapMemory(m_device, m_uniformBuffersMemory[a_imageIndex], 0, sizeof(mvp_UBO), 0, &data);
//		memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
//	vkUnmapMemory(m_device, m_uniformBuffersMemory[a_imageIndex]);
//
//
//}




//=====================================================================================================================
//Any Buffer Creation [Generic for any Buffer Creation] eg: Index Buffer or Vertex Buffer or even maybe Uniform Buffer
//=====================================================================================================================

//void vkRenderer::CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory)
//{
//	/////For EX: Buffer for the coordinates of Triangle which are being sent to the VS
//	VkBufferCreateInfo bufferCreateInfo = {};
//
//	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferCreateInfo.size = a_size;
//	bufferCreateInfo.usage = a_usage;
//	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//	if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &a_buffer) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to Create Buffer");
//	}
//
//	VkMemoryRequirements memRequirements;
//	vkGetBufferMemoryRequirements(m_device, a_buffer, &memRequirements);
//
//	VkMemoryAllocateInfo allocateInfo = {};
//
//	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	allocateInfo.allocationSize = memRequirements.size;
//	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, a_properties);// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//	//Allocated memory for the Vertex Buffer
//	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &a_bufferMemory) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to allocate memory for the Buffer");
//	}
//	vkBindBufferMemory(m_device, a_buffer, a_bufferMemory, 0);
//
//	
//}


//void vkRenderer::CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size)
//{
//	VkCommandBufferAllocateInfo allocInfo = {};
//
//	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandPool = m_CommandPool;
//	allocInfo.commandBufferCount = 1;
//
//	VkCommandBuffer commandBuffer;
//	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
//
//	//Start recording Command Buffer
//
//	VkCommandBufferBeginInfo bufferBeginInfo = {};
//
//	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);
//
//		VkBufferCopy copyRegion = {};
//
//		copyRegion.srcOffset = 0;
//		copyRegion.dstOffset = 0;
//		copyRegion.size = a_size;
//
//		vkCmdCopyBuffer(commandBuffer, a_srcBuffer, a_dstBuffer, 1, &copyRegion);
//
//	vkEndCommandBuffer(commandBuffer);
//
//	VkSubmitInfo submitInfo = {};
//
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//	vkQueueWaitIdle(m_graphicsQueue);
//	vkFreeCommandBuffers(m_device, m_CommandPool, 1, &commandBuffer);
//
//
//}


//=====================================================================================================================
// Descriptor Pool & Set Creation
//=====================================================================================================================

//void vkRenderer::CreateDescriptorPool()
//{
//
//	VkDescriptorPoolSize poolSize = {};
//
//	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//	poolSize.descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());
//
//	VkDescriptorPoolCreateInfo createInfo = {};
//
//	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	createInfo.poolSizeCount = 1;
//	createInfo.pPoolSizes = &poolSize;
//	createInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());
//
//
//	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to create Desciptor Pool");
//	}
//
//
//
//	
//}


//void vkRenderer::CreateDesciptorSets()
//{
//	std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_descriptorSetLayout);
//
//	VkDescriptorSetAllocateInfo allocateInfo = {};
//
//	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	allocateInfo.descriptorPool = m_DescriptorPool;
//	allocateInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
//	allocateInfo.pSetLayouts = layouts.data();
//
//	m_DescriptorSets.resize(m_SwapChainImages.size());
//
//	if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_DescriptorSets.data()) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Unable to create Desciptor Sets");
//	}
//
//	for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
//	{
//		VkDescriptorBufferInfo bufferInfo = {};
//
//		bufferInfo.buffer = m_uniformBuffers[i];
//		bufferInfo.offset = 0;
//		bufferInfo.range = sizeof(UniformBufferObject);
//
//		VkWriteDescriptorSet descriptorWriteInfo = {};
//
//		descriptorWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//		descriptorWriteInfo.dstSet = m_DescriptorSets[i];
//		descriptorWriteInfo.dstBinding = 0;
//		descriptorWriteInfo.dstArrayElement = 0;
//		descriptorWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//		descriptorWriteInfo.descriptorCount = 1;
//		descriptorWriteInfo.pBufferInfo = &bufferInfo;
//		descriptorWriteInfo.pImageInfo = nullptr;
//		descriptorWriteInfo.pTexelBufferView = nullptr;
//
//		vkUpdateDescriptorSets(m_device, 1, &descriptorWriteInfo, 0, nullptr);
//
//	}
//
//}


//===================================================================
//Vulkan Initialization Function
//===================================================================

//bool vkRenderer::InitVulkan()
//{
//	if (!CreateInstance())d
//		return false;
//
//	CreateSurface();D
//
//	setupDebugMessenger();D
//
//	pickPhysicalDevice();D
//	 
//	CreateLogicalDevice();D
//
//	CreateSwapChain();D
//
//	CreateImageView();D
//
//	CreateRenderPass();
//
//	CreateDescriptorSetLayout();
//
//	CreateGraphicsPipeline();	//Make this programmable from outside later[this is similar to what TheForge does when they make Pipeline]
//
//	CreateFrameBuffers();
//
//	CreateCommandPool();
//
//	CreateVertexBuffer();
//
//	CreateIndexBuffer();
//
//	CreateUniformBuffer();
//
//	CreateDescriptorPool();
//
//	CreateDesciptorSets();
//	
//	CreateCommandBuffers();
//	
//	CreateSemaphoresandFences();
//
//	return true;
//}
//

void vkRenderer::InitializeVulkan()
{
	//if (!CreateInstance())
		//return false;

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

	/*if (!InitVulkan())
	{
		std::cerr << "Failed to initialize Vulkan" << std::endl;
	}*/

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
	CreateImageView();
}


//===================================================================
//Renderer each loop updation
//===================================================================

//void vkRenderer::Update(float deltaTime)
//{
//	//Update any resources on CPU before sending it to GPU
//
//	//Call Uniform Buffer's Update Call
//
//
//
//
//}

//void vkRenderer::Draw(float deltaTime)
//{
//
//	vkWaitForFences(m_device, 1, &m_inflightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
//	vkResetFences(m_device, 1, &m_inflightFences[m_currentFrame]);
//
//
//
//	//===================================================================
//	//1.Acquire Image form SwapChain
//	//2.Execute command buffer on that image
//	//3.Return Image to swap chain for presentation
//	//===================================================================
//
//	uint32_t imageIndex;
//
//	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR)
//	{
//		ReCreateSwapChain();
//		return;
//	}
//	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
//	{
//		throw std::runtime_error("Failed to acquire swap chain image");
//	}
//
//	//Update Uniform Buffers which needs to be sent to Shader every frame
//	UpdateUniformBuffer(imageIndex, deltaTime);
//
//	//Subit info of which semaphores are being used for Queue
//	VkSubmitInfo submitInfo = {};
//
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore[m_currentFrame] };
//	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = waitSemaphores;
//	submitInfo.pWaitDstStageMask = waitStages;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
//
//	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore[m_currentFrame] };
//
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = signalSemaphores;
//
//	vkResetFences(m_device, 1, &m_inflightFences[m_currentFrame]);
//
//	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inflightFences[m_currentFrame]) != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to submit Draw Command Buffers");
//	}
//
//
//	VkPresentInfoKHR presentInfo = {};
//
//	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//	presentInfo.waitSemaphoreCount = 1;
//	presentInfo.pWaitSemaphores = signalSemaphores;
//
//	VkSwapchainKHR swapChains[] = { m_swapChain };
//
//	presentInfo.swapchainCount = 1;
//	presentInfo.pSwapchains = swapChains;
//	presentInfo.pImageIndices = &imageIndex;
//	presentInfo.pResults = nullptr;
//
//	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
//
//	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized)
//	{
//		m_frameBufferResized = false;
//		ReCreateSwapChain();
//	}
//	else if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to present Swap Chain image!");
//	}
//
//	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//
//}

//void vkRenderer::RenderLoop(float deltaTime)
//{
//	
//	Update(deltaTime);
//	Draw(deltaTime);
//	
//
//}



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
		vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
		vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
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

	vkDestroyBuffer(m_device, m_RectangleIndexBuffer, nullptr);
	vkFreeMemory(m_device, m_IndexBufferMemory, nullptr);


	vkDestroyBuffer(m_device, m_TriangleVertexBuffer, nullptr);
	vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

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
	
}

