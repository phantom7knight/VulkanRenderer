#pragma once

#include "stdafx.h"

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


class vkRenderer
{
public:

	//Variables
	bool m_frameBufferResized = false;


	//Functions
	vkRenderer();
	virtual ~vkRenderer();


	//void Init();
	bool InitGLFW();
	//bool InitVulkan();

	//void RenderLoop(float deltaTime);
	//void Update(float deltaTime);
	//void Draw(float deltaTime);

	//void Destroy();

	//static vkRenderer* getInstance();
 
	GLFWwindow* getWindow()
	{
		return m_window;
	}
	const VkDevice& getDevice()
	{
		return m_device;
	}

	//Vulkan Related Functions
	VkInstance getVulkanInstance();
	VkDebugUtilsMessengerEXT getDebugMessenger();
	VkQueue getGraphicsQueue()
	{
		return m_graphicsQueue;
	}

	
public:

	//static vkRenderer*					m_instance;
	GLFWwindow*							m_window;


	//Vulkan Related Parameters
	VkInstance							m_VulkanInstance;
	VkDebugUtilsMessengerEXT			m_debugMessenger;
	VkPhysicalDevice					m_physicalDevice = VK_NULL_HANDLE;

	VkDevice							m_device;
	VkQueue								m_graphicsQueue;
	VkSurfaceKHR						m_surface; // This is for relating Windows and Vulkan
	VkQueue								m_PresentQueue;

	VkSwapchainKHR						m_swapChain;
	std::vector<VkImage>				m_SwapChainImages;
	VkFormat							m_swapChainFormat;
	VkExtent2D							m_swapChainExtent;

	std::vector<VkImageView>			m_SwapChainImageViews;
	
	VkRenderPass						m_renderPass;//TODO : This can be modified later
	VkPipelineLayout					m_pipelineLayout;//TODO : This has to be per Shader/Obj [Look into it]
	VkPipeline							m_graphicsPipeline;

	std::vector<VkFramebuffer>			m_swapChainFrameBuffer;

	VkCommandPool						m_CommandPool;

	std::vector<VkCommandBuffer>		m_commandBuffers;

	std::vector<VkSemaphore>			m_imageAvailableSemaphore;
	std::vector<VkSemaphore>			m_renderFinishedSemaphore;

	std::vector<VkFence>				m_inflightFences;

	size_t								m_currentFrame = 0;

	VkBuffer							m_TriangleVertexBuffer;

	VkDeviceMemory						m_vertexBufferMemory;

	VkBuffer							m_RectangleIndexBuffer;

	VkDeviceMemory						m_IndexBufferMemory;

	VkDescriptorSetLayout				m_descriptorSetLayout;

	std::vector<VkBuffer>				m_uniformBuffers;

	std::vector<VkDeviceMemory>			m_uniformBuffersMemory;

	VkDescriptorPool					m_DescriptorPool;

	std::vector<VkDescriptorSet>		m_DescriptorSets;


	//Vulkan Related Functions
	bool CreateInstance();
	void setupDebugMessenger();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void pickPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void CreateLogicalDevice();
	void CreateSurface();

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice a_device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector< VkPresentModeKHR >& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& a_capabilities);
	void CreateSwapChain();

	void CreateImageView();

	void CreateGraphicsPipeline();

	VkShaderModule createShaderModule(const std::vector<char>& shaderCode);

	void CleanUpSwapChain();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	//void CreateVertexBuffer();

	uint32_t findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties);

	void CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory);

	void CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size);


	void SetUpSwapChain();
	
	void Init();

	void InitializeVulkan();
	
	//Pure Virtual so that the inherited class can override.
	virtual void PrepareApp();
	
	//Pure Virtual
	virtual void Draw(float deltaTime) = 0;
	virtual void Update(float deltaTime) = 0;
	
	virtual void Destroy();
};

