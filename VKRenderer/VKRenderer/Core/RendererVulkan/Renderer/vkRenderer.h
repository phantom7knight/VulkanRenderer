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
	void CreateBufferWithoutStaging(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties,
		VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory);
	void CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size);
	void CopyBufferToImage(VkBuffer buffer, TextureBufferDesc desc);

	void CreateBuffer(void const* databuffer, VkDeviceSize a_bufferSize, BufferDesc* a_BufferToCreate, VkBufferUsageFlags a_usage,
		VkCommandPool a_commandPool);

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
	void CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> layoutBindings,
		VkDescriptorSetLayout *a_descriptorSetLayout);
	void CreateGraphicsPipeline(GraphicsPipelineInfo* a_pipelineInfo);
	void CreateComputePipeline(ComputePipelineInfo a_computePipelineInfo);
	std::vector<VkPipelineShaderStageCreateInfo>  ShaderStageInfoGeneration(std::vector<std::string>ShaderNames);
	void CreateFrameBuffer(FrameBufferDesc a_fboDesc, VkRenderPass a_renderPass);
	void CreateCommandBuffers(std::vector<VkCommandBuffer> a_cmdBuffer, VkCommandPool a_cmdPool);
	void CreateCommandPool(VkCommandPool* a_commandPool);
	void CreateDescriptorPool(VkDescriptorPoolSize a_poolSize, uint32_t a_maxSets, uint32_t a_poolSizeCount,
		VkDescriptorPool* a_descriptorPool);
	void CreateDesciptorSets(uint32_t descriptorSetCount, VkDescriptorSetLayout a_descriptorSetLayout,
		std::vector<BufferDesc> a_descBuffer, VkDeviceSize    a_rangeSize, std::vector<VkWriteDescriptorSet> descriptorWriteInfo,
		VkDescriptorPool a_descriptorPool, std::vector<VkDescriptorSet> a_descriptorSet);
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

	GraphicsPipelineInfo GraphicsPipeline;

	//Shader Modules to destroy later
	std::vector<VkShaderModule>	shadermodules;

	SwapChainDesc						m_swapChainDescription;

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


	//std::vector<VkImageView>			m_SwapChainImageViews;

	VkRenderPass						m_renderPass;//TODO : This can be modified later
	VkPipelineLayout					m_pipelineLayout;//TODO : This has to be per Shader/Obj [Look into it]
	VkPipeline							m_graphicsPipeline;

	// Swap Chain Frame Buffer
	std::vector<FrameBufferDesc>		m_swapChainFrameBuffer;

	VkCommandPool						m_CommandPool;

	//std::vector<VkCommandBuffer>		m_commandBuffers;

	std::vector<VkSemaphore>			m_imageAvailableSemaphore;
	std::vector<VkSemaphore>			m_renderFinishedSemaphore;

	std::vector<VkFence>				m_inflightFences;

	size_t								m_currentFrame = 0;
		
	std::vector<BufferDesc>				m_ModelUniformBuffer;

	std::vector<BufferDesc>				m_LightInfoUniformBuffer;

	/*VkDescriptorSetLayout				m_descriptorSetLayout;

	VkDescriptorPool					m_DescriptorPool;

	std::vector<VkDescriptorSet>		m_DescriptorSets;*/



#pragma endregion



};

