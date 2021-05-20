
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
// Helper Functions
//===================================================================

QueueFamilyIndices vkRenderer::FindQueueFamalies()
{
	return VulkanHelper::FindQueueFamilies(m_physicalDevice, m_surface);
}

//===================================================================
//GLFW Input recording
//===================================================================
void vkRenderer::ProcessInput(GLFWwindow* window, float deltaTime)
{
	//If Esc button is pressed we close
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	
	//Update Keys pressed status for the camera update
	m_MainCamera->keys.forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.up = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? true : false;
	m_MainCamera->keys.down = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? true : false;

	if (m_MainCamera->keys.backward || m_MainCamera->keys.up ||
		m_MainCamera->keys.down || m_MainCamera->keys.right ||
		m_MainCamera->keys.left || m_MainCamera->keys.forward)
	{
		m_MainCamera->ProcessKeyBoardMovement(deltaTime);
	}
}

void vkRenderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<vkRenderer*>(glfwGetWindowUserPointer(window));
	app->m_frameBufferResized = true;
}

bool firstMouse = true;
float lastX = (float)WIDTH / 2.0f;
float lastY = (float)HEIGHT / 2.0f;

static void MousePosCallBack(GLFWwindow* window, double xpos, double ypos)
{
	auto app = reinterpret_cast<vkRenderer*>(glfwGetWindowUserPointer(window));

	int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)(xpos - lastX);
	float yoffset = (float)(lastY - ypos); // reversed since y-coordinates go from bottom to top

	lastX = (float)xpos;
	lastY = (float)ypos;

	if (mouseState)
		app->m_MainCamera->ProcessMouseMovement(xoffset, yoffset);

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
// Creating Image & Image Views[Used to view Images]
//===================================================================
void vkRenderer::CreateImage(TextureBufferDesc* a_texBuffDesc)
{
	VkImageCreateInfo ImageCreateInfo = {};

	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.extent.width = a_texBuffDesc->ImageWidth;
	ImageCreateInfo.extent.height = a_texBuffDesc->ImageHeight;
	ImageCreateInfo.extent.depth = 1;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.format = a_texBuffDesc->imageFormat;
	ImageCreateInfo.tiling = a_texBuffDesc->tiling;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ImageCreateInfo.usage = a_texBuffDesc->usageFlags;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.flags = 0;


	if (vkCreateImage(m_device, &ImageCreateInfo, nullptr, &(a_texBuffDesc->BufferImage)) != VK_SUCCESS)
	{
		std::cout << " Failed to create Image \n";
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, a_texBuffDesc->BufferImage, &memRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = VulkanHelper::FindMemoryType(m_physicalDevice,memRequirements.memoryTypeBits, a_texBuffDesc->propertyFlags);

	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &a_texBuffDesc->BufferMemory) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate memory to the image \n";
	}

	vkBindImageMemory(m_device, a_texBuffDesc->BufferImage, a_texBuffDesc->BufferMemory, 0);
}

void vkRenderer::CreateImageView(VkImage a_image, VkFormat a_format, VkImageAspectFlags a_aspectFlags, VkImageView *a_imageView)
{
	VulkanHelper::CreateImageView(m_device, a_image, a_format, a_aspectFlags, a_imageView);
	return;
}

void vkRenderer::CreateTextureSampler(SamplerCreationDesc a_createInfo, VkSampler *a_sampler)
{
	VkSamplerCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.magFilter = a_createInfo.magFilter;
	createInfo.minFilter = a_createInfo.minFilter;

	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	createInfo.anisotropyEnable = a_createInfo.anisotropyEnable;
	createInfo.maxAnisotropy = 16; // lower value bad quality more performance

	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	//******* VERY IMP************/
	createInfo.unnormalizedCoordinates = VK_FALSE;

	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.mipmapMode = a_createInfo.MipMode;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	VulkanHelper::CreateSampler(m_device, createInfo, a_sampler);
	
	return;
}

////Don't include this in a header file////
#define STB_IMAGE_IMPLEMENTATION
#include "../../Dependencies/STB/stb_image.h"

void vkRenderer::LoadImageTexture(std::string textureName, TextureBufferDesc *a_imageData, VkCommandPool a_commandPool,
	VkCommandBuffer* a_commandBuffer)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(textureName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		std::cout << "Failed to load Texture : " << textureName << "\n";


	BufferDesc stagingBuffer;
		
	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.Buffer, stagingBuffer.BufferMemory);

	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);


	//free the loaded image
	stbi_image_free(pixels);

	//Set the image Property

	a_imageData->ImageWidth = texWidth;
	a_imageData->ImageHeight = texHeight;
	a_imageData->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	a_imageData->tiling = VK_IMAGE_TILING_OPTIMAL;
	a_imageData->usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	a_imageData->propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateImage(a_imageData);

	VulkanHelper::TransitionImageLayouts(m_device, a_commandPool, m_graphicsQueue, a_imageData->BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VulkanHelper::CopyBufferToImage(m_device, a_commandPool, m_graphicsQueue, stagingBuffer.Buffer, *a_imageData);

	VulkanHelper::TransitionImageLayouts(m_device, a_commandPool, m_graphicsQueue, a_imageData->BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);
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
	renderpassInfo.dependencyCount = static_cast<uint32_t>(a_renderPassDesc.subpassDependecy.size());
	renderpassInfo.pDependencies = a_renderPassDesc.subpassDependecy.data();

	VulkanHelper::CreateRenderPass(m_device, renderpassInfo, a_renderPass);

	
}

//===================================================================
//Shader Related Functions
//===================================================================
std::vector<VkPipelineShaderStageCreateInfo> vkRenderer::ShaderStageInfoGeneration(std::vector<std::string> ShaderNames)
{
	std::vector<VkPipelineShaderStageCreateInfo>	shaderStages;
	
	shaderStages.reserve(ShaderNames.size());
	shadermodules.resize(ShaderNames.size());

	// Generate SPIRV shader files
	rsrcLdr.GenerateSPIRVShaders(ShaderNames);

	for (int i = 0; i < ShaderNames.size(); ++i)
	{
		
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
		if (rsrcLdr.getFileOperationobj().CheckIfStringExists(SPIRVFileNames, "vert"))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().CheckIfStringExists(SPIRVFileNames, "frag"))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().CheckIfStringExists(SPIRVFileNames, "geom"))
		{
			shaderCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			shaderCreateInfo.pName = "main";
		}
		else if (rsrcLdr.getFileOperationobj().CheckIfStringExists(SPIRVFileNames, "comp"))
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
	viewPort.width  = static_cast<float>(m_swapChainDescription.m_swapChainExtent.width);	//a_pipelineInfo->viewportWidth;
	viewPort.height = static_cast<float>(m_swapChainDescription.m_swapChainExtent.height);	//a_pipelineInfo->viewportHeigth;
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
	createGraphicsPipelineInfo.stageCount = static_cast<uint32_t>(shaderInfo.size());
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
	VulkanHelper::TransitionImageLayouts(m_device, a_commandPool, m_graphicsQueue, a_image,
		a_format, a_oldLayout, a_newLayout);
}


//===================================================================
//Command Pool
//===================================================================

void vkRenderer::CreateCommandPool(VkCommandPool* a_commandPool)
{
	QueueFamilyIndices queuefamilyindeces = VulkanHelper::FindQueueFamilies(m_physicalDevice, m_surface);

	VkCommandPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queuefamilyindeces.graphicsFamily.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VulkanHelper::CreateCommandPool(m_device, createInfo, a_commandPool);

	return;
}


void vkRenderer::AllocateCommandBuffers(std::vector<VkCommandBuffer> &a_cmdBuffer, VkCommandPool a_cmdPool)
{
	VkCommandBufferAllocateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	createInfo.commandPool = a_cmdPool;
	createInfo.commandBufferCount = (uint32_t)a_cmdBuffer.size();

	VulkanHelper::AllocateCommandBuffers(m_device, createInfo, a_cmdBuffer);
	
}


//===================================================================
//Buffer Creation
//===================================================================
//, const ModelInfo a_modelDesc
void vkRenderer::CreateBuffer(void const* databuffer, VkDeviceSize a_bufferSize, BufferDesc* a_BufferToCreate, VkBufferUsageFlags a_usage,
	VkCommandPool a_commandPool)
{
		
	VkDeviceSize bufferSize = a_bufferSize;
	
	//Create Staging Buffer before transfering
	
	BufferDesc stagingBuffer;

	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.Buffer, stagingBuffer.BufferMemory);
	
	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, bufferSize, 0, &data);
	
	memcpy(data, databuffer, (size_t)bufferSize);//a_modelDesc.indexbufferData.data()
	
	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);
	
	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | a_usage, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, a_BufferToCreate->Buffer, a_BufferToCreate->BufferMemory);

	VulkanHelper::CopyBuffer(m_device, a_commandPool, m_graphicsQueue, stagingBuffer.Buffer, a_BufferToCreate->Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);
	
	return;
}


void vkRenderer::CreateBufferWithoutStaging(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties,
	VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory)
{
	VulkanHelper::CreateBuffer(m_device, m_physicalDevice, a_size, a_usage, a_properties, a_buffer, a_bufferMemory);
	
	return;
}


//===================================================================
// Create and Set Descriptor Layouts

//This functions is used to set the layout which will be 
//sent to Shaders. Ex: UBO to Vertex Shader and Sampler to 
//Fragment/Pixel Shader.
//===================================================================

void vkRenderer::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> layoutBindings,
	VkDescriptorSetLayout *a_descriptorSetLayout)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};

	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	VulkanHelper::CreateDescriptorSetLayout(m_device, layoutInfo, a_descriptorSetLayout);
	
	return;
}

//=====================================================================================================================
// Descriptor Pool & Set Creation
//=====================================================================================================================


void vkRenderer::CreateDescriptorPool(std::vector< VkDescriptorPoolSize > a_poolSize, uint32_t a_maxSets, uint32_t a_poolSizeCount,
	VkDescriptorPool* a_descriptorPool)
{
	VkDescriptorPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = a_poolSizeCount;
	createInfo.pPoolSizes = a_poolSize.data();
	createInfo.maxSets = a_maxSets;


	VulkanHelper::CreateDescriptorPool(m_device, createInfo, a_descriptorPool);

	return;
}


void vkRenderer::CreateDesciptorSets(uint32_t descriptorSetCount, VkDescriptorSetLayout a_descriptorSetLayout,
	std::vector <BufferDesc> a_descBuffer, VkDeviceSize    a_rangeSize, std::vector<VkWriteDescriptorSet> descriptorWriteInfo,
	VkDescriptorPool a_descriptorPool, std::vector<VkDescriptorSet> &a_descriptorSet)
{
	std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, a_descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = a_descriptorPool;
	allocateInfo.descriptorSetCount = descriptorSetCount;
	allocateInfo.pSetLayouts = layouts.data();

	a_descriptorSet.resize(descriptorSetCount);

	if (vkAllocateDescriptorSets(m_device, &allocateInfo, a_descriptorSet.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Desciptor Sets");
	}

	for (size_t i = 0; i < descriptorWriteInfo.size(); ++i)
	{
		for (size_t j = 0; j < descriptorSetCount; ++j)
		{
			VkDescriptorBufferInfo bufferInfo = {};

			bufferInfo.buffer = a_descBuffer[i].Buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = a_rangeSize;


			descriptorWriteInfo[i].dstSet = a_descriptorSet[j];
			descriptorWriteInfo[i].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWriteInfo.size()), descriptorWriteInfo.data(), 0, nullptr);
		}		

	}
}


void vkRenderer::AllocateDescriptorSets(VkDescriptorPool a_descriptorPool, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSet> &a_DescriptorSets)
{
	VkDescriptorSetAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = a_descriptorPool;
	allocateInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainDescription.m_SwapChainImages.size());
	allocateInfo.pSetLayouts = layouts.data();

	a_DescriptorSets.resize(m_swapChainDescription.m_SwapChainImages.size());

	VulkanHelper::AllocateDescriptorSets(m_device, allocateInfo, a_DescriptorSets);

	return;
}

void vkRenderer::UpdateDescriptorSets(std::vector< VkWriteDescriptorSet>& a_descriptorWriteInfo)
{
	VulkanHelper::UpdateDescriptorSets(m_device, a_descriptorWriteInfo);

	return;
}

//===================================================================
//Creating Frame Buffers
//===================================================================
void vkRenderer::CreateFrameBuffer(FrameBufferDesc a_fboDesc, VkRenderPass a_renderPass, VkFramebuffer* a_frameBuffer)
{
	VkFramebufferCreateInfo fbcreateInfo = {};

	fbcreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbcreateInfo.renderPass = a_renderPass;
	fbcreateInfo.attachmentCount = a_fboDesc.attachmentCount;
	fbcreateInfo.pAttachments = a_fboDesc.Attachments.data();
	fbcreateInfo.width  = static_cast<uint32_t>(a_fboDesc.FBOWidth);
	fbcreateInfo.height = static_cast<uint32_t>(a_fboDesc.FBOHeight);
	fbcreateInfo.layers = 1;

	VulkanHelper::CreateFrameBuffer(m_device, &fbcreateInfo, a_frameBuffer);
	
}

//===================================================================
//Creating Semaphores and Fences
//===================================================================
void vkRenderer::CreateSemaphoresandFences()
{
	m_imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	m_inflightFences.resize(MAX_FRAMES_IN_FLIGHT);


	VkSemaphoreCreateInfo createSemaphoreInfo = {};

	createSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo createFenceInfo = {};

	createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VulkanHelper::CreateSemaphoresandFences(m_device, createSemaphoreInfo, createFenceInfo,
			&m_imageAvailableSemaphore[i], &m_renderFinishedSemaphore[i], &m_inflightFences[i]);
	}

	return;
}

//===================================================================
//Submitting and Presenting a Frame
//===================================================================

VkResult vkRenderer::AcquireNextImage(uint32_t *a_imageIndex, size_t a_currentFrameNumber)
{
	return vkAcquireNextImageKHR(m_device, m_swapChainDescription.m_swapChain,
		std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore[a_currentFrameNumber], VK_NULL_HANDLE, a_imageIndex);

}


void vkRenderer::SubmissionAndPresentation(FrameSubmissionDesc a_frameSubmissionDesc)
{
	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore[a_frameSubmissionDesc.currentFrameNumber] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = a_frameSubmissionDesc.commandBufferCount;
	submitInfo.pCommandBuffers = a_frameSubmissionDesc.commandBuffer;

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore[a_frameSubmissionDesc.currentFrameNumber] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VulkanHelper::ResetFence(m_device, 1, &m_inflightFences[a_frameSubmissionDesc.currentFrameNumber]);

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inflightFences[a_frameSubmissionDesc.currentFrameNumber]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Draw Command Buffers");
	}


	VkPresentInfoKHR presentInfo = {};

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChainDescription.m_swapChain };

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = a_frameSubmissionDesc.imageIndex;
	presentInfo.pResults = nullptr;

	a_frameSubmissionDesc.result = VulkanHelper::QueuePresent(m_PresentQueue, &presentInfo);

	if (a_frameSubmissionDesc.result == VK_ERROR_OUT_OF_DATE_KHR || a_frameSubmissionDesc.result == VK_SUBOPTIMAL_KHR
		|| m_frameBufferResized)
	{
		m_frameBufferResized = false;
		//TODO: need to fix it
		//ReCreateSwapChain();
	}
	else if (a_frameSubmissionDesc.result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present Swap Chain image!");
	}

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
	m_swapChainDescription.m_SwapChainImageViews = VulkanHelper::CreateSwapChainImageView(m_swapChainDescription, m_device);
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
		vkDestroyFramebuffer(m_device, m_swapChainFrameBuffer[i].FrameBuffer, nullptr);
	}

	for (size_t i = 0; i < m_swapChainDescription.m_SwapChainImageViews.size(); ++i)
	{
		vkDestroyImageView(m_device, m_swapChainDescription.m_SwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChainDescription.m_swapChain, nullptr);

}

void vkRenderer::Destroy()
{
	//=====================================================================
	//Delete Vulkan related things[Generalize when deleting the resources]
	//=====================================================================

	CleanUpSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
		vkDestroyFence(m_device, m_inflightFences[i], nullptr);
	}

	vkDestroyDevice(m_device, nullptr);

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

