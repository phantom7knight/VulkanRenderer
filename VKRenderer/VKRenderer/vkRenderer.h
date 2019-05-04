#pragma once



struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value();
	}
};





class vkRenderer
{
public:
	vkRenderer();
	~vkRenderer();


	void Init();
	bool InitGLFW();
	bool InitVulkan();

	void mainloop();
	void Run();
	void Update();

	void Destroy();

	static vkRenderer* getInstance();
 
	GLFWwindow* getWindow()
	{
		return m_window;
	}


	//Vulkan Related Functions
	bool CreateInstance();
	void setupDebugMessenger();
	VkInstance getVulkanInstance();
	VkDebugUtilsMessengerEXT getDebugMessenger();
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void pickPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

private:

	static vkRenderer* m_instance;
	GLFWwindow* m_window;


	//Vulkan Related Parameters
	VkInstance m_VulkanInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	bool isDeviceSuitable(VkPhysicalDevice device);

};

