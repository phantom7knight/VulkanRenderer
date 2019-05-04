#include "stdafx.h"
#include "vkRenderer.h"
#include "ValidationLayer.hpp"



//Static variable declaration
vkRenderer* vkRenderer::m_instance = nullptr;

vkRenderer * vkRenderer::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new vkRenderer();
	}

	return m_instance;
}

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

bool vkRenderer::InitGLFW()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "VkRenderer", nullptr, nullptr);

	if (!m_window)
		return false;

	return true;
}


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
	appInfo.apiVersion = VK_API_VERSION_1_0;


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

bool vkRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	//
	//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete();
	
}

void vkRenderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("There are any GPU's which support Vulkan");//check if they support for Vulkan
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

QueueFamilyIndices vkRenderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}


bool vkRenderer::InitVulkan()
{
	if (!CreateInstance())
		return false;

	setupDebugMessenger();

	pickPhysicalDevice();
	 

	return true;
}


void vkRenderer::Init()
{

	if (!InitGLFW())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}

	if (!InitVulkan())
	{
		std::cerr << "Failed to initialize Vulkan" << std::endl;
	}

}

void vkRenderer::Run()
{


}

void vkRenderer::Update()
{


}

void vkRenderer::mainloop()
{
	

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();


		/*Run();
		Update();*/

	}

}

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



void vkRenderer::Destroy()
{
	//==========================================
	//Delete Vulkan related things
	//==========================================

	if (enableValidationLayer)
	{
		DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_debugMessenger, nullptr);
	}
	
	vkDestroyInstance(m_VulkanInstance,nullptr);

	
	glfwDestroyWindow(m_window);

	glfwTerminate();


}

