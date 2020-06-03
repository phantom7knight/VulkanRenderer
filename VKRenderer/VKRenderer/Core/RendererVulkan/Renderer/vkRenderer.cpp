
#include "vkRenderer.h"
#include "../../ResourceLoading/ResourceLoader.h"
#include "../../Camera/Camera.h"

#include "../VulkanHelper/VulkanHelper.hpp"


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
	m_MainCamera->keys.up = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.down = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? true : false;

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
// Creating Image Views[Used to view Images]
//===================================================================
void vkRenderer::createImageView(VkImage a_image, VkFormat a_format, VkImageAspectFlags a_aspectFlags, VkImageView *a_imageView)
{
	VulkanHelper::CreateImageView(m_device, a_image, a_format, a_aspectFlags, a_imageView);
	return;
}


//===================================================================
//RenderPass Creation
//===================================================================
void vkRenderer::CreateRenderPass(RenderPassInfo a_renderPassDesc, VkRenderPass* a_renderPass)
{
	
	//Render Pass Info

	VkRenderPassCreateInfo renderpassInfo = {};

	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.attachmentCount = static_cast<uint32_t>(a_renderPassDesc.attachmentDescriptions.size());
	renderpassInfo.pAttachments = a_renderPassDesc.attachmentDescriptions.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &(a_renderPassDesc.subpassInfo);
	renderpassInfo.dependencyCount = a_renderPassDesc.subpassDependecy.size();
	renderpassInfo.pDependencies = a_renderPassDesc.subpassDependecy.data();

	VulkanHelper::CreateRenderPass(m_device, renderpassInfo, a_renderPass);

	
}

//===================================================================
//Shader Related Functions
//===================================================================

std::vector<VkPipelineShaderStageCreateInfo> vkRenderer::ShaderStageInfoGeneration(std::vector<std::string> ShaderNames)
{
	std::vector<VkPipelineShaderStageCreateInfo>	shaderStages;
	
	shaderStages.resize(ShaderNames.size());
	shadermodules.resize(ShaderNames.size());

	for (int i = 0; i < ShaderNames.size(); ++i)
	{
		// Generate SPIRV shader files
		rsrcLdr.GenerateSPIRVShaders(ShaderNames);

		std::string SPIRVFileNames = "Shaders/BinaryCode/" + ShaderNames[i] + ".spv";
		auto ShaderCode = rsrcLdr.getFileOperationobj().readFile(SPIRVFileNames);

		ShaderDesc shader_info = {};

		shader_info.a_device = m_device;
		shader_info.shaderCode = &(ShaderCode);

		shadermodules[i] = rsrcLdr.createShaderModule(shader_info);

		VkPipelineShaderStageCreateInfo  shaderCreateInfo = {};

		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderCreateInfo.module = shadermodules[i];

		//find the correct shader staga
		// TODO: Check checkIfCharacterExists() call
		if (rsrcLdr.getFileOperationobj().checkIfCharacterExists(SPIRVFileNames, 'vert'))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().checkIfCharacterExists(SPIRVFileNames, 'frag'))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().checkIfCharacterExists(SPIRVFileNames, 'geom'))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().checkIfCharacterExists(SPIRVFileNames, 'comp'))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			shaderCreateInfo.pName = "CSmain";
		}

		shaderStages.push_back(shaderCreateInfo);
	}

	return shaderStages;
}




//===================================================================
//Pipeline Creation
//===================================================================

void vkRenderer::CreateGraphicsPipeline(GraphicsPipelineInfo* a_pipelineInfo)
{
	//Get Shader Info
	std::vector<VkPipelineShaderStageCreateInfo> shaderInfo = ShaderStageInfoGeneration(a_pipelineInfo->ShaderFileNames);


	// Vertex Input
	VkPipelineVertexInputStateCreateInfo VertexInputInfo = {};

	VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDesc	= a_pipelineInfo->vertexBindingDesc;
	auto attributeDesc	= a_pipelineInfo->AttributeDescriptionsofVertex;

	VertexInputInfo.vertexBindingDescriptionCount = 1;
	VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size());
	VertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	VertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();

	//Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};

	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = a_pipelineInfo->pipelineTopology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//View Ports
	VkViewport viewPort = {};//TODO :This can be configurable by user

	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = m_swapChainDescription.m_swapChainExtent.width;	//a_pipelineInfo->viewportWidth;
	viewPort.height = m_swapChainDescription.m_swapChainExtent.height;	//a_pipelineInfo->viewportHeigth;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;
	
	//Scissors
	VkRect2D scissor = {};

	scissor.offset = { 0,0 };
	scissor.extent = m_swapChainDescription.m_swapChainExtent;

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
	rasterizer.polygonMode = a_pipelineInfo->polygonMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = a_pipelineInfo->cullMode;
	rasterizer.frontFace = a_pipelineInfo->frontFaceCullingMode;
	rasterizer.depthBiasEnable = a_pipelineInfo->depthBiasEnableMode;
	rasterizer.depthBiasConstantFactor = 0.0f;	
	rasterizer.depthBiasClamp = 0.0f;			
	rasterizer.depthBiasSlopeFactor = 0.0f;		


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

	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = a_pipelineInfo->depthTestEnable;
	depthStencilInfo.depthWriteEnable = a_pipelineInfo->depthWriteEnable;
	depthStencilInfo.depthCompareOp = a_pipelineInfo->depthCompareOperation;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = a_pipelineInfo->stencilTestEnable;
	depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

	//Color Blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
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
	pipelineLayoutInfo.pSetLayouts = &a_pipelineInfo->a_descriptorSetLayout;
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VulkanHelper::CreatePipelineLayout(m_device, pipelineLayoutInfo, &a_pipelineInfo->a_pipelineLayout);

	//Finally create the Graphics Pipeline
	VkGraphicsPipelineCreateInfo createGraphicsPipelineInfo = {};

	createGraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createGraphicsPipelineInfo.stageCount = shaderInfo.size();
	createGraphicsPipelineInfo.pStages = shaderInfo.data();
	createGraphicsPipelineInfo.pVertexInputState = &VertexInputInfo;
	createGraphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
	createGraphicsPipelineInfo.pViewportState = &viewPortState;
	createGraphicsPipelineInfo.pRasterizationState = &rasterizer;
	createGraphicsPipelineInfo.pMultisampleState = &multiSampling;
	createGraphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;
	createGraphicsPipelineInfo.pColorBlendState = &colorBlending;
	createGraphicsPipelineInfo.pDynamicState = nullptr;
	createGraphicsPipelineInfo.layout = a_pipelineInfo->a_pipelineLayout;
	createGraphicsPipelineInfo.renderPass = a_pipelineInfo->renderPass;
	createGraphicsPipelineInfo.subpass = a_pipelineInfo->subpass;

	createGraphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	createGraphicsPipelineInfo.basePipelineIndex = -1;

	VulkanHelper::CreateGraphicsPipeline(m_device, createGraphicsPipelineInfo,
		&a_pipelineInfo->a_Pipeline);

	// Destroy Shader Modules which were set
	for (int i = 0; i < shadermodules.size(); ++i)
	{
		VulkanHelper::DestroyShaderModule(m_device, shadermodules[i]);
	}

	return;
}

//TODO: Finish This
void vkRenderer::CreateComputePipeline(ComputePipelineInfo a_computePipelineInfo)
{

	return;
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
//Command Buffer Recording Related
//===================================================================

VkCommandBuffer vkRenderer::BeginSingleTimeCommands(VkCommandPool a_commandPool)
{
	VkCommandBuffer resultCmdBuffer = VulkanHelper::BeginSingleTimeCommands(m_device, a_commandPool);
	return resultCmdBuffer;
}

void vkRenderer::EndSingleTimeCommands(VkCommandBuffer* a_commandBuffer, VkCommandPool a_commandPool)
{
	VulkanHelper::EndSingleTimeCommands(a_commandBuffer, a_commandPool, m_device, m_graphicsQueue);
}

void vkRenderer::TransitionImageLayouts(VkCommandPool a_commandPool, VkCommandBuffer* a_commandBuffer,
	VkImage a_image, VkFormat a_format, VkImageLayout a_oldLayout, VkImageLayout a_newLayout)
{
	VulkanHelper::TransitionImageLayouts(m_device, a_commandPool, a_commandBuffer, m_graphicsQueue, a_image,
		a_format, a_oldLayout, a_newLayout);
}


//===================================================================
//Buffer Creation
//===================================================================

void vkRenderer::CreateBuffer(const ModelInfo a_modelDesc, BufferDesc* a_BufferToCreate, VkBufferUsageFlags a_usage,
	VkMemoryPropertyFlags a_properties, VkCommandPool a_commandPool)
{
	VkDeviceSize bufferSize = a_modelDesc.indexBufferSize;
	
	//Create Staging Buffer before transfering
	
	BufferDesc stagingBuffer;

	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.Buffer, stagingBuffer.BufferMemory);
	
	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, bufferSize, 0, &data);
	
	memcpy(data, a_modelDesc.indexbufferData.data(), (size_t)bufferSize);
	
	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);
	
	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | a_usage, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, a_BufferToCreate->Buffer, a_BufferToCreate->BufferMemory);

	VulkanHelper::CopyBuffer(m_device, a_commandPool, m_graphicsQueue, stagingBuffer.Buffer, a_BufferToCreate->Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);
	
	return;
}



//===================================================================
//Vulkan Initialization Function
//===================================================================

void vkRenderer::InitializeVulkan()
{
	VulkanHelper::CreateInstance(&m_VulkanInstance);

	VulkanHelper::CreateSurface(m_VulkanInstance, m_window, &m_surface);

	VulkanHelper::SetupDebugMessenger(m_VulkanInstance, &m_debugMessenger);

	VulkanHelper::PickPhysicalDevice(m_VulkanInstance, m_physicalDevice, m_surface);

	m_device = VulkanHelper::CreateLogicalDevice(m_physicalDevice, m_surface, &m_graphicsQueue, &m_PresentQueue);

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
	VulkanHelper::CreateSwapChain(m_physicalDevice, m_surface, m_window, m_device, &m_swapChainDescription);
}

void vkRenderer::PrepareApp()
{
	SetUpSwapChain();
	m_SwapChainImageViews = VulkanHelper::CreateSwapChainImageView(m_swapChainDescription, m_device);
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

	vkDestroySwapchainKHR(m_device, m_swapChainDescription.m_swapChain, nullptr);

	for (size_t i = 0; i < m_swapChainDescription.m_SwapChainImages.size(); ++i)
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

