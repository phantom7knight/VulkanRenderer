#pragma once

#include "stdafx.h"

class Camera;
class vkRenderer
{
public:

	//Variables
	bool m_frameBufferResized = false;

	ResourceLoader rsrcLdr;

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

	BufferDesc							m_TriangleVertexBuffer;

	BufferDesc							m_RectangleIndexBuffer;

	std::vector<BufferDesc>				m_TriangleUniformBuffer;
	
	std::vector<BufferDesc>				m_ModelUniformBuffer;

	VkDescriptorSetLayout				m_descriptorSetLayout;

	VkDescriptorPool					m_DescriptorPool;

	std::vector<VkDescriptorSet>		m_DescriptorSets;


	Camera								*m_MainCamera;

	//Functions
	vkRenderer();
	virtual ~vkRenderer();
	
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



	//Vulkan Related Functions
	bool CreateInstance();
	void setupDebugMessenger();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void pickPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void CreateLogicalDevice();
	void CreateSurface();

	//Swap Chain Related
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice a_device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector< VkPresentModeKHR >& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& a_capabilities);
	void CreateSwapChain();
	void CleanUpSwapChain();


	void CreateImageView();
	
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	//Buffer Creation Related
	uint32_t findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties);
	void CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory);
	void CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size);
	void CopyBufferToImage(VkBuffer buffer, TextureBufferDesc desc);


	//Command Buffer Related
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer a_commandBuffer);

	void TransitionImageLayouts(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
;



	
	void Init();

	void SetUpSwapChain();

	bool InitGLFW();

	void InitializeVulkan();
	
	void ProcessInput(GLFWwindow* window);


	//Pure Virtual so that the inherited class can override.
	virtual void PrepareApp();
	
	//Pure Virtual
	virtual void Draw(float deltaTime) = 0;
	virtual void Update(float deltaTime) = 0;
	
	virtual void Destroy();

};

