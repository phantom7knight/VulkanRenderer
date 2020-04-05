#include "ModelViewer.h"
#include "../../VKRenderer/Camera.h"
#include "../../Dependencies/Imgui/IMGUI/Imgui_Impl.h"



ModelViewer::ModelViewer() : m_showGUILight(true)
{
}


ModelViewer::~ModelViewer()
{
}

void ModelViewer::SetUpCameraProperties(Camera* a_cam)
{
	//SetUp Camera Properties
	a_cam->set_position(glm::vec3(0.0, 0.0, -10.5));
	a_cam->camProperties.rotation_speed	   = 0.2f;
	a_cam->camProperties.translation_speed = 0.002f;

	//set proj matrix
	a_cam->set_perspective(glm::radians(45.0f), (float)m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 1000.0f);

}

void ModelViewer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = m_swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//TODO : Programmable

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass shud start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//How render pass final image shud translate at end of render pass

	VkAttachmentDescription depthAttachment = {};

	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass shud start with
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		//How render pass final image shud translate at end of render pass


	//Each renderpass can have multiple sub-passes
	//which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};

	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;
	subpassInfo.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	//array of attachments for this render pass
	std::array< VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	//Render Pass Info
	VkRenderPassCreateInfo renderpassInfo = {};

	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderpassInfo.pAttachments = attachments.data();
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpassInfo;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_device, &renderpassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Render Pass");
	}
}

void ModelViewer::CreateDescriptorSetLayout()
{
	//create binding for UBO
	//used in vertex shader
	VkDescriptorSetLayoutBinding layoutBinding = {};

	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	//create binding for sampler
	//used in pixel shader
	VkDescriptorSetLayoutBinding samplerBinding = {};

	samplerBinding.binding = 1;
	samplerBinding.descriptorCount = 1;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.pImmutableSamplers = nullptr;

	//TODO: check the binding for the layout
	VkDescriptorSetLayoutBinding LightlayoutBinding = {};

	LightlayoutBinding.binding = 2;
	LightlayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LightlayoutBinding.descriptorCount = 1;
	LightlayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	LightlayoutBinding.pImmutableSamplers = nullptr;



	//create an array of descriptors
	std::array< VkDescriptorSetLayoutBinding, 3> descriptorsArray = { layoutBinding ,samplerBinding, LightlayoutBinding };


	VkDescriptorSetLayoutCreateInfo layoutInfo = {};

	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(descriptorsArray.size());
	layoutInfo.pBindings = descriptorsArray.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Descriptor Set Layout");
	}
}

void ModelViewer::CreateGraphicsPipeline()
{
	//generate SPIRV binary code
	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "Model.vert";
	ShaderFileNames[1] = "Model.frag";

	rsrcLdr.GenerateSPIRVShaders(ShaderFileNames);

	//=============================================

	//Read Shader Binary Code
	auto VertexShaderCode = rsrcLdr.getFileOperationobj().readFile("Shaders/BinaryCode/Model.vert.spv");
	auto PixelShaderCode = rsrcLdr.getFileOperationobj().readFile("Shaders/BinaryCode/Model.frag.spv");

	//Generate respective Shader Modules
	ShaderDesc shader_info = {};

	shader_info.a_device = m_device;

	//Vertex shader module
	shader_info.shaderCode = &VertexShaderCode;
	VkShaderModule vertexShaderModule = rsrcLdr.createShaderModule(shader_info);

	//pixel shader module
	shader_info.shaderCode = &PixelShaderCode;
	VkShaderModule pixelShaderModule = rsrcLdr.createShaderModule(shader_info);

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

	auto bindingDesc = rsrcLdr.getModelLoaderobj().getBindingDescription();
	auto attributeDesc = rsrcLdr.getModelLoaderobj().getAttributeDescriptionsofVertex();

	VertexInputInfo.vertexBindingDescriptionCount = 1;
	VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size());
	VertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	VertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();


	//Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};

	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//TODO :This can be configurable by user
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.flags = 0;

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
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;	//TODO :This can be configurable by user
	rasterizer.lineWidth = 1.0f;					//TODO :This can be configurable by user
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	
	rasterizer.depthBiasClamp = 0.0f;			
	rasterizer.depthBiasSlopeFactor = 0.0f;		
	rasterizer.flags = 0;


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
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

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
	//pipelineLayoutInfo.pushConstantRangeCount = 0; //TODO: USE Push Constants
	//pipelineLayoutInfo.pPushConstantRanges = nullptr; //TODO: USE Push Constants

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
	createGraphicsPipelineInfo.pDepthStencilState = &depthStencilInfo;
	createGraphicsPipelineInfo.pRasterizationState = &rasterizer;
	createGraphicsPipelineInfo.pMultisampleState = &multiSampling;
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
}

void ModelViewer::CreateFrameBuffers()
{
	m_swapChainFrameBuffer.resize(m_SwapChainImageViews.size());

	for (uint32_t i = 0; i < m_SwapChainImageViews.size(); ++i)
	{
		std::array< VkImageView,2> attachments[] = { m_SwapChainImageViews[i], depthImageView };

		VkFramebufferCreateInfo fbcreateInfo = {};

		fbcreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbcreateInfo.renderPass = m_renderPass;
		fbcreateInfo.attachmentCount = static_cast<uint32_t>(attachments->size());
		fbcreateInfo.pAttachments = attachments->data();
		fbcreateInfo.width = m_swapChainExtent.width;
		fbcreateInfo.height = m_swapChainExtent.height;
		fbcreateInfo.layers = 1;

		if (vkCreateFramebuffer(m_device, &fbcreateInfo, nullptr, &m_swapChainFrameBuffer[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to create Frame Buffer");
		}
	}
}

void ModelViewer::CreateCommandPool()
{
	QueueFamilyIndices queuefamilyindeces = findQueueFamilies(m_physicalDevice);

	VkCommandPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queuefamilyindeces.graphicsFamily.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Command Pool");
	}
}

void ModelViewer::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(ModelUBO);
	VkDeviceSize lightBufferSize = sizeof(LightInfoUBO);

	m_ModelUniformBuffer.resize(m_SwapChainImages.size());
	m_LightInfoUniformBuffer.resize(m_SwapChainImages.size());

	for (int i = 0; i < m_SwapChainImages.size(); ++i)
	{
		//model uniform buffer creation
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, m_ModelUniformBuffer[i].Buffer, m_ModelUniformBuffer[i].BufferMemory);
		
		//light info uniform buffer creation
		CreateBuffer(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, m_LightInfoUniformBuffer[i].Buffer, m_LightInfoUniformBuffer[i].BufferMemory);
	}
}

void ModelViewer::CreateDescriptorPool()
{
	
	std::array<VkDescriptorPoolSize, 3> poolSizes = {};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

	VkDescriptorPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();
	createInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());


	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Desciptor Pool");
	}
}

void ModelViewer::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_SwapChainImages.size(), m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = m_DescriptorPool;
	allocateInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapChainImages.size());
	allocateInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(m_SwapChainImages.size());

	if (vkAllocateDescriptorSets(m_device, &allocateInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Desciptor Sets");
	}

	for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};

		bufferInfo.buffer = m_ModelUniformBuffer[i].Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ModelUBO);

		VkDescriptorBufferInfo lightBufferInfo = {};

		lightBufferInfo.buffer = m_LightInfoUniformBuffer[i].Buffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(LightInfoUBO);

		VkDescriptorImageInfo imageInfo = {};

		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = textureSampler;
		imageInfo.imageView = textureImageView;

		std::array< VkWriteDescriptorSet,3> descriptorWriteInfo = {};

		descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[0].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[0].dstBinding = 0;
		descriptorWriteInfo[0].dstArrayElement = 0;
		descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo[0].descriptorCount = 1;
		descriptorWriteInfo[0].pBufferInfo = &bufferInfo;
		descriptorWriteInfo[0].pImageInfo = nullptr;
		descriptorWriteInfo[0].pTexelBufferView = nullptr;

		descriptorWriteInfo[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[1].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[1].dstBinding = 1;
		descriptorWriteInfo[1].dstArrayElement = 0;
		descriptorWriteInfo[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWriteInfo[1].descriptorCount = 1;
		descriptorWriteInfo[1].pImageInfo = &imageInfo;
		descriptorWriteInfo[1].pBufferInfo = nullptr; //TODO: Check this
		descriptorWriteInfo[1].pTexelBufferView = nullptr;

		descriptorWriteInfo[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[2].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[2].dstBinding = 2;
		descriptorWriteInfo[2].dstArrayElement = 0;
		descriptorWriteInfo[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo[2].descriptorCount = 1;
		descriptorWriteInfo[2].pBufferInfo = &lightBufferInfo;
		descriptorWriteInfo[2].pImageInfo = nullptr;
		descriptorWriteInfo[2].pTexelBufferView = nullptr;
		
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWriteInfo.size()), descriptorWriteInfo.data(), 0, nullptr);

	}

}

void ModelViewer::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_swapChainFrameBuffer.size());

	VkCommandBufferAllocateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	createInfo.commandPool = m_CommandPool;
	createInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();


	if (vkAllocateCommandBuffers(m_device, &createInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Command Buffers");
	}

}

void ModelViewer::CreateSemaphoresandFences()
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
		if (vkCreateSemaphore(m_device, &createSemaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &createSemaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &createFenceInfo, nullptr, &m_inflightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Semaphore");
		}
	}



}

void ModelViewer::UpdateUniformBuffer(uint32_t a_imageIndex , CameraMatrices properties_Cam)
{

#pragma region MVP_Update
	ModelUBO mvp_UBO = {};

	//Model Matrix
	mvp_UBO.ModelMatrix = glm::mat4(1);
	mvp_UBO.ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
	//mvp_UBO.ModelMatrix = glm::rotate(mvp_UBO.ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvp_UBO.ModelMatrix = glm::scale(mvp_UBO.ModelMatrix, glm::vec3(0.005f));


	//View Matrix
	mvp_UBO.ViewMatrix = cam_matrices.view;


	//Projection Matrix
	mvp_UBO.ProjectionMatrix = cam_matrices.perspective;
	mvp_UBO.ProjectionMatrix[1][1] *= -1;


	//Copy the data

	void* data;

	vkMapMemory(m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory);
#pragma endregion

#pragma region LightInfo_Update
	LightInfoUBO lightInfo_UBO = {};

	//lightPosition
	lightInfo_UBO.lightPosition = m_lightPosGUILight;

	lightInfo_UBO.lightColor = m_lightColorGUILight;

	lightInfo_UBO.specularIntensity = m_SpecularIntensityGUILight;

	lightInfo_UBO.camPosition = glm::vec3(0.0, 0.0, -10.5);

	
	//Copy the data

	data = NULL;

	vkMapMemory(m_device, m_LightInfoUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(lightInfo_UBO), 0, &data);
	memcpy(data, &lightInfo_UBO, sizeof(lightInfo_UBO));
	vkUnmapMemory(m_device, m_LightInfoUniformBuffer[a_imageIndex].BufferMemory);

#pragma endregion

}

void ModelViewer::DrawGui(VkCommandBuffer a_cmdBuffer)
{
	Imgui_Impl::getInstance()->Gui_BeginFrame();

	if (m_showGUILight)
	{
		
		ImGui::Begin("Another Window", &m_showGUILight);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				
		ImGui::Text("Hello from another window!");

		ImGui::SliderFloat3("Light Position", &m_lightPosGUILight.x, -200.0f, 200.0f);

		ImGui::SliderFloat3("Light Color", &m_lightColorGUILight.x, 0.0f, 1.0f);

		ImGui::SliderInt("Spec Intensity", &m_SpecularIntensityGUILight, 2, 256);


		if (ImGui::Button("Close Me"))
			m_showGUILight = false;
		
		ImGui::End();
	}


	Imgui_Impl::getInstance()->Gui_Render(a_cmdBuffer);
}


void ModelViewer::UpdateCommandBuffers(uint32_t a_imageIndex)
{
	uint32_t i = a_imageIndex;
	VkCommandBufferBeginInfo beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	//Start Recording
	if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to begin recording Command Buffer");
	}

	VkRenderPassBeginInfo renderpassBeginInfo = {};

	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = m_renderPass;
	renderpassBeginInfo.framebuffer = m_swapChainFrameBuffer[i];
	renderpassBeginInfo.renderArea.offset = { 0,0 };
	renderpassBeginInfo.renderArea.extent = m_swapChainExtent;


	//Clear Color//
	VkClearValue clearColor[2];
	clearColor[0] = { 0.0,0.0,0.0,1.0 };
	clearColor[1] = { 1.0f, 0.0f };
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearColor;


	vkCmdBeginRenderPass(m_commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

	vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkDeviceSize offset = { 0 };

	//Bind Vertex Buffer
	vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &VertexBUffer.Buffer, &offset);

	//Bind Index Buffer
	vkCmdBindIndexBuffer(m_commandBuffers[i], IndexBUffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

	//Call Draw Indexed for the model
	vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_indexBufferCount), 1, 0, 0, 0);
	
	//==================================================
	//Draw UI
	DrawGui(m_commandBuffers[i]);
	
	//==================================================
	
	vkCmdEndRenderPass(m_commandBuffers[i]);

	//End Recording
	if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record Command Buffer");
	}

}

void ModelViewer::ReCreateSwapChain()
{

	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}


	vkDeviceWaitIdle(m_device);

	Destroy();

	CleanUpSwapChain();

	CreateSwapChain();

	CreateSwapChainImageView();

	CreateRenderPass();

	CreateGraphicsPipeline();

	CreateDepthResources();

	CreateFrameBuffers();

	CreateUniformBuffer();

	CreateCommandBuffers();

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandPool();


}

void ModelViewer::SetUpVertexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_VertexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.vertexBufferSize;

	//Create Staging Buffer before transfering

	BufferDesc stagingBuffer;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.Buffer, stagingBuffer.BufferMemory);


	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, bufferSize, 0, &data);

	memcpy(data, a_modelDesc.vertexbufferData.data(), (size_t)bufferSize);

	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);


	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		a_VertexBUffer->Buffer, a_VertexBUffer->BufferMemory);

	CopyBuffer(stagingBuffer.Buffer, a_VertexBUffer->Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);

	return;
}

void ModelViewer::SetUpIndexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_IndexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.indexBufferSize;

	//Create Staging Buffer before transfering

	BufferDesc stagingBuffer;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.Buffer, stagingBuffer.BufferMemory);


	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, bufferSize, 0, &data);

	memcpy(data, a_modelDesc.indexbufferData.data(), (size_t)bufferSize);

	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);


	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		a_IndexBUffer->Buffer, a_IndexBUffer->BufferMemory);

	CopyBuffer(stagingBuffer.Buffer, a_IndexBUffer->Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);

	return;
}

void ModelViewer::LoadAModel(std::string fileName)
{
	ModelInfo modelinfor = 	rsrcLdr.LoadModelResource(fileName);

	//Load Index and Vertex Buffer
	SetUpVertexBuffer(modelinfor, &VertexBUffer);
	SetUpIndexBuffer(modelinfor	, &IndexBUffer);

	m_indexBufferCount = static_cast<uint32_t>(modelinfor.indexbufferData.size());

}

////Don't include this in a header file////
#define STB_IMAGE_IMPLEMENTATION
#include "../../Dependencies/STB/stb_image.h"

void ModelViewer::LoadTexture(std::string textureName)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(textureName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		std::cout << "Failed to load Texture : " << textureName << "\n";


	BufferDesc stagingBuffer;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.Buffer, stagingBuffer.BufferMemory);

	void* data;
	vkMapMemory(m_device, stagingBuffer.BufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBuffer.BufferMemory);


	//free the loaded image
	stbi_image_free(pixels);

	//Set the image Property

	image1.ImageWidth		= texWidth;
	image1.ImageHeight		= texHeight;
	image1.imageFormat		= VK_FORMAT_R8G8B8A8_SRGB;
	image1.tiling			= VK_IMAGE_TILING_OPTIMAL;
	image1.usageFlags		= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image1.propertyFlags	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateImage(&image1);

	TransitionImageLayouts(image1.BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	CopyBufferToImage(stagingBuffer.Buffer, image1);

	//we do this to have access to the shader to a sampler
	TransitionImageLayouts(image1.BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);

}

//TODO: Generalize this
void ModelViewer::CreateImage(TextureBufferDesc *a_texBuffDesc)
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
	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, a_texBuffDesc->propertyFlags);

	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &a_texBuffDesc->BufferMemory) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate memory to the image \n";
	}

	vkBindImageMemory(m_device, a_texBuffDesc->BufferImage, a_texBuffDesc->BufferMemory, 0);
}

void ModelViewer::CreateImageTextureView()
{
	createImageView(image1.BufferImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &textureImageView);
}

void ModelViewer::CreateTextureSampler()
{
	VkSamplerCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;

	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = 16; // lower value bad quality more performance

	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	//******* VERY IMP************//
	createInfo.unnormalizedCoordinates = VK_FALSE;

	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_device, &createInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create sampler for the texture provided");
		return;
	}



}


void ModelViewer::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();

	depthImageInfo.ImageHeight = m_swapChainExtent.height;
	depthImageInfo.ImageWidth = m_swapChainExtent.width;
	depthImageInfo.imageFormat = depthFormat;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	CreateImage(&depthImageInfo);

	createImageView(depthImageInfo.BufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);

	//TransitionImageLayouts(depthImageInfo.BufferImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void ModelViewer::setGuiVariables()
{
	m_lightPosGUILight = glm::vec3(0.0, -100.0, 0.0);
	m_lightColorGUILight = glm::vec3(1.0, 1.0, 1.0);
	m_SpecularIntensityGUILight = 4;
}

void ModelViewer::InitGui()
{

	setGuiVariables();

	ImGui_ImplVulkan_InitInfo imguiInfo = {};

	imguiInfo.Instance = m_VulkanInstance;
	imguiInfo.Allocator = nullptr;
	imguiInfo.Device = m_device;
	imguiInfo.PhysicalDevice = m_physicalDevice;
	imguiInfo.DescriptorPool = nullptr;//this will be gui created descriptor pool
	imguiInfo.ImageCount = IMAGE_COUNT;
	imguiInfo.MinImageCount = 2;
	imguiInfo.Queue = m_graphicsQueue;
	imguiInfo.QueueFamily = findQueueFamilies(m_physicalDevice).graphicsFamily.value();
	imguiInfo.PipelineCache = nullptr;


	//Init the GUI for IMGUI
	Imgui_Impl::getInstance()->Init(m_window, imguiInfo, m_renderPass, m_CommandPool);


}

//==========================================================================================================

void ModelViewer::PrepareApp()
{
	vkRenderer::PrepareApp();

	CreateRenderPass();

	CreateDescriptorSetLayout();

	CreateCommandPool();

	CreateDepthResources();

	CreateFrameBuffers();

	CreateGraphicsPipeline();

	//LoadAModel("../../Assets/Models/cornell_box/cornell_box.obj");
	LoadAModel("../../Assets/Models/monkey/monkey.obj");
	//LoadAModel("../../Assets/Models/VulkanScene/vulkanscene_shadow.dae");
	//LoadAModel("../../Assets/Models/venus/venus.fbx");

	//LoadTexture("../../Assets/Textures/Statue.jpg");
	LoadTexture("../../Assets/Textures/green.jpg");

	CreateImageTextureView();

	CreateTextureSampler();

	CreateUniformBuffer();

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	CreateSemaphoresandFences();

	// set up the camera position
	SetUpCameraProperties(m_MainCamera);

	//Initialize Dear ImGui
	InitGui();

}

void ModelViewer::Update(float deltaTime)
{
	ProcessInput(m_window);

	cam_matrices.perspective = m_MainCamera->matrices.perspective;

	//set view matrix
	cam_matrices.view = m_MainCamera->matrices.view;


}

void ModelViewer::Draw(float deltaTime)
{
	vkWaitForFences(m_device, 1, &m_inflightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_device, 1, &m_inflightFences[m_currentFrame]);


	//===================================================================
	//1.Acquire Image form SwapChain
	//2.Execute command buffer on that image
	//3.Return Image to swap chain for presentation
	//===================================================================

	uint32_t imageIndex;

	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		//TODO: need to fix it
		ReCreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image");
	}

	//update our command buffers
	UpdateCommandBuffers(imageIndex);

	//Update Uniform Buffers which needs to be sent to Shader every frames
	UpdateUniformBuffer(imageIndex , cam_matrices);

	//Subit info of which semaphores are being used for Queue
	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore[m_currentFrame] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_device, 1, &m_inflightFences[m_currentFrame]);

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inflightFences[m_currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Draw Command Buffers");
	}


	VkPresentInfoKHR presentInfo = {};

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized)
	{
		m_frameBufferResized = false;
		//TODO: need to fix it
		ReCreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present Swap Chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	m_MainCamera->update(deltaTime);

}

void ModelViewer::Destroy()
{
	//depth Image
	vkDestroyImageView(m_device, depthImageView, nullptr);
	vkDestroyImage(m_device, depthImageInfo.BufferImage, nullptr);
	vkFreeMemory(m_device, depthImageInfo.BufferMemory, nullptr);


	vkDestroySampler(m_device, textureSampler, nullptr);
	vkDestroyImageView(m_device, textureImageView, nullptr);

	//destroy Image
	vkDestroyImage(m_device, image1.BufferImage, nullptr);
	vkFreeMemory(m_device, image1.BufferMemory, nullptr);


	Imgui_Impl::getInstance()->DestroyGui(m_device);

}