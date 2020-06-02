#pragma once

#include "../../PCH/stdafx.h"

class Camera;
class vkRenderer
{
public:

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


	void CreateSwapChainImageView();
	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *a_imageView);
	
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	//Buffer Creation Related
	uint32_t findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties);
	void CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory);
	void CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size);
	void CopyBufferToImage(VkBuffer buffer, TextureBufferDesc desc);

	void CreateBuffer(const ModelInfo a_modelDesc, BufferDesc* a_BufferToCreate, VkBufferUsageFlags a_usage,
		VkMemoryPropertyFlags a_properties, VkCommandPool a_commandPool);

	//Command Buffer Related
	VkCommandBuffer BeginSingleTimeCommands(VkCommandPool a_commandPool);
	void EndSingleTimeCommands(VkCommandBuffer* a_commandBuffer, VkCommandPool a_commandPool);
	void TransitionImageLayouts(VkCommandPool a_commandPool, VkCommandBuffer* a_commandBuffer,
		VkImage a_image, VkFormat a_format, VkImageLayout a_oldLayout, VkImageLayout a_newLayout);


	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	bool hasStencilComponent(VkFormat format);

#pragma region New_Fns
	void CreateRenderPass(RenderPassInfo a_renderPassDesc, VkRenderPass* a_renderPass);
	void CreatePipeline();
#pragma endregion

	
	void Init();

	void SetUpSwapChain();

	bool InitGLFW();

	void InitializeVulkan();
	
	void ProcessInput(GLFWwindow* window);

	//Setup Swap-Chain for the App
	void PrepareApp();
	
	//Pure Virtual
	void Draw(float deltaTime) ;
	void Update(float deltaTime) ;
	
	virtual void Destroy();

	//Variables

	MousePositions mousePos;

	Camera* m_MainCamera;

public:
	
#pragma region Variables
	bool m_frameBufferResized = false;

	ResourceLoader rsrcLdr;

	GLFWwindow* m_window;


	//Vulkan Related Parameters
	VkInstance							m_VulkanInstance;
	VkDebugUtilsMessengerEXT			m_debugMessenger;
	VkPhysicalDevice					m_physicalDevice = VK_NULL_HANDLE;

	VkDevice							m_device;
	VkQueue								m_graphicsQueue;
	VkSurfaceKHR						m_surface; // This is for relating Windows and Vulkan
	VkQueue								m_PresentQueue;

	SwapChainDesc						m_swapChainDescription;

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

	std::vector<BufferDesc>				m_LightInfoUniformBuffer;

	VkDescriptorSetLayout				m_descriptorSetLayout;

	VkDescriptorPool					m_DescriptorPool;

	std::vector<VkDescriptorSet>		m_DescriptorSets;



#pragma endregion



};

