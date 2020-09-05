#include "../../PCH/stdafx.h"


//Validation Layer
//using standard LunarG layer here
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//If not Debug do not run the validation layer check's
#ifdef _DEBUG
const bool enableValidationLayer = true;
#else
const bool enableValidationLayer = false;
#endif // NDEBUG



bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			std::cerr << "Layer not found" << std::endl;
			return false;
		}

	}

	return true;

}



std::vector<const char*> getRequiredExtensions()
{
	uint32_t glfwextenstionCount = 0;
	const char** glfwExtenstions;

	glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwextenstionCount);

	std::vector<const char*> extensions(glfwExtenstions, glfwExtenstions + glfwextenstionCount);

	if (enableValidationLayer)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) 
{

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}


VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
	
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else 
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

}