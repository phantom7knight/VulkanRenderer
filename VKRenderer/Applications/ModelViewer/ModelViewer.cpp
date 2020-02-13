#include "ModelViewer.h"



ModelViewer::ModelViewer()
{
}


ModelViewer::~ModelViewer()
{
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


	//Each renderpass can have multiple sub-passes
	//which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;



	//Render Pass Info

	VkRenderPassCreateInfo renderpassInfo = {};

	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &colorAttachment;
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
	VkDescriptorSetLayoutBinding layoutBinding = {};

	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};

	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//TODO: look into this later, since these are optional
	rasterizer.depthBiasClamp = 0.0f;			//TODO: look into this later, since these are optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		//TODO: look into this later, since these are optional


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
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
	createGraphicsPipelineInfo.pRasterizationState = &rasterizer;
	createGraphicsPipelineInfo.pMultisampleState = &multiSampling;
	createGraphicsPipelineInfo.pDepthStencilState = nullptr;
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
		VkImageView attachments[] = { m_SwapChainImageViews[i] };

		VkFramebufferCreateInfo fbcreateInfo = {};

		fbcreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbcreateInfo.renderPass = m_renderPass;
		fbcreateInfo.attachmentCount = 1;
		fbcreateInfo.pAttachments = attachments;
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
	createInfo.flags = 0;

	if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Command Pool");
	}
}

void ModelViewer::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(ModelUBO);

	m_ModelUniformBuffer.resize(m_SwapChainImages.size());
	//m_uniformBuffersMemory.resize(m_SwapChainImages.size());

	for (int i = 0; i < m_SwapChainImages.size(); ++i)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, m_ModelUniformBuffer[i].Buffer, m_ModelUniformBuffer[i].BufferMemory);
	}
}

void ModelViewer::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};

	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(m_SwapChainImages.size());

	VkDescriptorPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &poolSize;
	createInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());


	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Desciptor Pool");
	}
}

void ModelViewer::CreateDesciptorSets()
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

		VkWriteDescriptorSet descriptorWriteInfo = {};

		descriptorWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo.dstSet = m_DescriptorSets[i];
		descriptorWriteInfo.dstBinding = 0;
		descriptorWriteInfo.dstArrayElement = 0;
		descriptorWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo.descriptorCount = 1;
		descriptorWriteInfo.pBufferInfo = &bufferInfo;
		descriptorWriteInfo.pImageInfo = nullptr;
		descriptorWriteInfo.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_device, 1, &descriptorWriteInfo, 0, nullptr);

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

	//Record Command Buffer data
	for (size_t i = 0; i < m_commandBuffers.size(); ++i)
	{
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
		VkClearValue clearColor = { 0.0,0.0,0.0,1.0 };
		renderpassBeginInfo.clearValueCount = 1;
		renderpassBeginInfo.pClearValues = &clearColor;


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

		vkCmdEndRenderPass(m_commandBuffers[i]);

		//End Recording
		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record Command Buffer");
		}

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

void ModelViewer::UpdateUniformBuffer(uint32_t a_imageIndex, float a_deltaTime)
{
	//TODO: Fix this
	a_deltaTime = 0.5f;


	ModelUBO mvp_UBO = {};

	//Model Matrix
	mvp_UBO.ModelMatrix = glm::rotate(glm::mat4(1.0f), a_deltaTime * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvp_UBO.ModelMatrix = glm::scale(mvp_UBO.ModelMatrix, glm::vec3(0.01));


	//View Matrix
	mvp_UBO.ViewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));


	//Projection Matrix
	mvp_UBO.ProjectionMatrix = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
	mvp_UBO.ProjectionMatrix[1][1] *= -1;


	//Copy the data

	void* data;

	vkMapMemory(m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory);
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

	CleanUpSwapChain();

	CreateSwapChain();

	CreateImageView();

	CreateRenderPass();

	CreateGraphicsPipeline();

	CreateFrameBuffers();

	CreateUniformBuffer();

	CreateCommandBuffers();

	CreateDescriptorPool();

	CreateDesciptorSets();

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

	m_indexBufferCount = modelinfor.indexbufferData.size();

}

////Don't include in a header file
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
	image1.ImageHeigth		= texHeight;
	image1.imageFormat		= VK_FORMAT_R8G8B8A8_SRGB;
	image1.tiling			= VK_IMAGE_TILING_OPTIMAL;
	image1.usageFlags		= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image1.propertyFlags	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateImage(image1);

	TransitionImageLayouts(image1.BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	CopyBufferToImage(stagingBuffer.Buffer, image1);

	//we do this to have access to the shader to a sampler
	TransitionImageLayouts(image1.BufferImage, VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_device, stagingBuffer.Buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffer.BufferMemory, nullptr);

}

void ModelViewer::CreateImage(TextureBufferDesc a_texBuffDesc)
{
	VkImageCreateInfo ImageCreateInfo = {};

	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.extent.width = a_texBuffDesc.ImageWidth;
	ImageCreateInfo.extent.height = a_texBuffDesc.ImageHeigth;
	ImageCreateInfo.extent.depth = 1;
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.format = a_texBuffDesc.imageFormat;
	ImageCreateInfo.tiling = a_texBuffDesc.tiling;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ImageCreateInfo.usage = a_texBuffDesc.usageFlags;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.flags = 0;


	if (vkCreateImage(m_device, &ImageCreateInfo, nullptr, &(image1.BufferImage)) != VK_SUCCESS)
	{
		std::cout << " Failed to create Image \n";
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, image1.BufferImage, &memRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, a_texBuffDesc.propertyFlags);

	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &image1.BufferMemory) != VK_SUCCESS)
	{
		std::cout << "Failed to allocate memory to the image \n";
	}

	vkBindImageMemory(m_device, image1.BufferImage, image1.BufferMemory, 0);
}


void ModelViewer::CreateImageTextureView()
{
	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image1.BufferImage;
	createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	createInfo.pNext = nullptr;
	createInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(m_device, &createInfo, nullptr, &textureImageView) != VK_SUCCESS)
	{
		std::cout << "Unable Image view for the texutre provided \n";
		return;
	}

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



void ModelViewer::PrepareApp()
{
	vkRenderer::PrepareApp();

	CreateRenderPass();

	CreateDescriptorSetLayout();

	CreateGraphicsPipeline();

	CreateFrameBuffers();

	CreateCommandPool();

	LoadAModel("../../Assets/Models/christmas-ball/source/Christmas_Ball_Sketchfab.fbx");

	LoadTexture("../../Assets/Textures/Statue.jpg");

	CreateImageTextureView();

	CreateTextureSampler();

	CreateUniformBuffer();

	CreateDescriptorPool();

	CreateDesciptorSets();

	CreateCommandBuffers();

	CreateSemaphoresandFences();


	

}

void ModelViewer::Update(float deltaTime)
{
	//Update Camera
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

	//Update Uniform Buffers which needs to be sent to Shader every frames
	UpdateUniformBuffer(imageIndex, deltaTime);

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
}

void ModelViewer::Destroy()
{
	vkDestroySampler(m_device, textureSampler, nullptr);
	vkDestroyImageView(m_device, textureImageView, nullptr);

	//destroy Image
	vkDestroyImage(m_device, image1.BufferImage, nullptr);
	vkFreeMemory(m_device, image1.BufferMemory, nullptr);
}