#include "Triangle.h"
#include "../../VKRenderer/Core/PCH/stdafx.h"


//===================================================================
//Vertex Buffer Use
//===================================================================

struct TriangleVertex
{
	glm::vec3 Position;
	glm::vec3 Normals;
	glm::vec2 TexCoords;


	//Set Vertex Binding Desc i.e like setting VAO in OpenGL

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc = {};

		bindingDesc.binding = 0;	//Since we are using only one array for the data that is Triangle_vertices we have 1 binding and order starts from 0
		bindingDesc.stride = sizeof(TriangleVertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}


	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptionsofVertex()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDesc = {};

		//Position
		attributeDesc[0].binding = 0;
		attributeDesc[0].location = 0;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[0].offset = offsetof(TriangleVertex, Position);

		//Normals
		attributeDesc[1].binding = 0;
		attributeDesc[1].location = 1;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[1].offset = offsetof(TriangleVertex, Normals);

		//TexCoords
		attributeDesc[2].binding = 0;
		attributeDesc[2].location = 2;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[2].offset = offsetof(TriangleVertex, TexCoords);


		return attributeDesc;
	}



};


const std::vector<TriangleVertex> Triangle_vertices = {
	{	{0.0,-0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5, 0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{-0.5,0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	}
};

const std::vector<TriangleVertex> Rectangle_vertices = {
	{	{-0.5,-0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5, -0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5,0.5,0.0},			{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{-0.5,0.5,0.0},			{0.0,0.0,1.0},		{0.0,0.0}	}
};

const std::vector<uint16_t> Rectangle_Indices = {
	0,1,2,2,3,0
};


struct UniformBufferObject
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};


Triangle::Triangle()
{
	//Initialize Renderer
	m_renderer = new vkRenderer();


}




//===================================================================
//Create Graphics Pipeline and Shader Related Functions
//===================================================================

void Triangle::CreateGraphicsPipeline()
{
	//Resize f
	TrianglePipeline.ShaderPipelineDesc.resize(2);
	//generate SPIRV binary code
	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "Basic.vert";
	ShaderFileNames[1] = "Basic.frag";

	rsrcLdr.GenerateSPIRVShaders(ShaderFileNames);

	//=============================================

	//Read Shader Binary Code
	auto VertexShaderCode	= rsrcLdr.getFileOperationobj().readFile("Shaders/BinaryCode/Basic.vert.spv");
	auto PixelShaderCode	= rsrcLdr.getFileOperationobj().readFile("Shaders/BinaryCode/Basic.frag.spv");

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

	auto bindingDesc = TriangleVertex::getBindingDescription();
	auto attributeDesc = TriangleVertex::getAttributeDescriptionsofVertex();

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
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//TODO :This can be configurable by user
	rasterizer.lineWidth = 1.0f;//TODO :This can be configurable by user
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


//===================================================================
//Creating Render Pass
//TODO :Can be made generic
//===================================================================

void Triangle::CreateRenderPass()
{
	std::vector< VkAttachmentDescription> attachmentDescriptions;

	attachmentDescriptions.resize(1);

	attachmentDescriptions[0].format = m_renderer->m_swapChainDescription.m_swapChainFormat;
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::vector<VkAttachmentReference> attachmentReferences;

	attachmentReferences.resize(attachmentDescriptions.size());

	attachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;					//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &attachmentReferences[0];

	std::vector< VkSubpassDependency> subpassDependecy;

	subpassDependecy.resize(1);

	subpassDependecy[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependecy[0].dstSubpass = 0;
	subpassDependecy[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependecy[0].srcAccessMask = 0;
	subpassDependecy[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependecy[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	RenderPassInfo renderPassdesc = {};

	renderPassdesc.attachmentDescriptions	= attachmentDescriptions;
	renderPassdesc.attachmentReferences		= attachmentReferences;
	renderPassdesc.subpassInfo				= subpassInfo;
	renderPassdesc.subpassDependecy			= subpassDependecy;

	m_renderer->CreateRenderPass(renderPassdesc, &m_TriangleRenderPass);

	
}



//===================================================================
//Creating Frame Buffers
//===================================================================
void Triangle::CreateFrameBuffers()
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



//===================================================================
//Command Pool
//===================================================================

void Triangle::CreateCommandPool()
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


//===================================================================
//Command Buffers
//===================================================================

void Triangle::CreateCommandBuffers()
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

				vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

				VkBuffer vertexBuffers[] = { m_TriangleVertexBuffer.Buffer };
				VkDeviceSize offset = { 0 };
				vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &m_TriangleVertexBuffer.Buffer, &offset);

				vkCmdBindIndexBuffer(m_commandBuffers[i], m_RectangleIndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);

				//vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(Triangle_vertices.size()), 1, 0, 0);

				vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

				vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(Rectangle_Indices.size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[i]);

		//End Recording
		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record Command Buffer");
		}

	}
}


//===================================================================
// Semaphore & Fences
//===================================================================

void Triangle::CreateSemaphoresandFences()
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


//=====================================================================================================================
//Create & Update Uniform Buffer[Generalize this]
//=====================================================================================================================


void Triangle::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_TriangleUniformBuffer.resize(m_SwapChainImages.size());
	//m_uniformBuffersMemory.resize(m_SwapChainImages.size());

	for (int i = 0; i < m_SwapChainImages.size(); ++i)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_TriangleUniformBuffer[i].Buffer,
			m_TriangleUniformBuffer[i].BufferMemory);
	}


}

void Triangle::UpdateUniformBuffer(uint32_t a_imageIndex, float a_deltaTime)
{
	//TODO: Fix this
	a_deltaTime = 0.5f;


	UniformBufferObject mvp_UBO = {};

	//Model Matrix
	mvp_UBO.ModelMatrix = glm::rotate(glm::mat4(1.0f), a_deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));


	//View Matrix
	mvp_UBO.ViewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));


	//Projection Matrix
	mvp_UBO.ProjectionMatrix = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float)m_swapChainExtent.height, 0.1f, 10.0f);
	mvp_UBO.ProjectionMatrix[1][1] *= -1;


	//Copy the data

	void* data;

	vkMapMemory(m_device, m_TriangleUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_device, m_TriangleUniformBuffer[a_imageIndex].BufferMemory);


}



//===================================================================
//Create Index Buffer
//===================================================================

void Triangle::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Rectangle_Indices[0]) * Rectangle_Indices.size();

	//Create Staging Buffer before transfering
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);


	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);

	memcpy(data, Rectangle_Indices.data(), (size_t)bufferSize);

	vkUnmapMemory(m_device, stagingBufferMemory);


	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_RectangleIndexBuffer.Buffer, m_RectangleIndexBuffer.BufferMemory);

	CopyBuffer(stagingBuffer, m_RectangleIndexBuffer.Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}



//===================================================================
//Creaate Vertex Buffer
//===================================================================


void Triangle::CreateVertexBuffer()//Make this Generic
{

	VkDeviceSize bufferSize = sizeof(Rectangle_vertices[0]) * Rectangle_vertices.size();

	//Create Staging Buffer before transfering
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);


	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Rectangle_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);


	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 m_TriangleVertexBuffer.Buffer, m_TriangleVertexBuffer.BufferMemory);

	CopyBuffer(stagingBuffer, m_TriangleVertexBuffer.Buffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
	
	
}


//===================================================================
// Create and Set Descriptor Layouts[maybe generalize?]

//This functions is used to set the layout which will be 
//sent to Shaders. Ex: UBO to Vertex Shader and Sampler to 
//Fragment/Pixel Shader.
//===================================================================

void Triangle::CreateDescriptorSetLayout()
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




//=====================================================================================================================
// Descriptor Pool & Set Creation
//=====================================================================================================================

void Triangle::CreateDescriptorPool()
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


void Triangle::CreateDesciptorSets()
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

		bufferInfo.buffer = m_TriangleUniformBuffer[i].Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

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


//===================================================================
//Recreating Swap Chains
//===================================================================


void Triangle::ReCreateSwapChain()
{

	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}


	//vkDeviceWaitIdle(m_device);
	vkDeviceWaitIdle(m_renderer->getDevice());

	//TODO: call VkRenderer
	CleanUpSwapChain();

	//CreateSwapChain();
	//
	//CreateSwapChainImageView();

	m_renderer->PrepareApp();

	CreateRenderPass();

	CreateGraphicsPipeline();

	CreateFrameBuffers();

	CreateUniformBuffer();

	CreateCommandBuffers();

	CreateDescriptorPool();
	
	CreateDesciptorSets();

	CreateCommandPool();


}

void Triangle::Init()
{
	m_renderer->Init();

	return;
}

void Triangle::PrepareApp()
{
	//Renderer's Prepare App Sequence
	m_renderer->PrepareApp();
	
	CreateRenderPass();

	CreateDescriptorSetLayout();

	CreateGraphicsPipeline();

	CreateFrameBuffers();

	CreateCommandPool();

	CreateVertexBuffer();

	CreateIndexBuffer();

	CreateUniformBuffer();
	
	CreateDescriptorPool();
	
	CreateDesciptorSets();

	CreateCommandBuffers();

	CreateSemaphoresandFences();

}


void Triangle::Update(float deltaTime)
{


	return;
}


void Triangle::Draw(float deltaTime)
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




void Triangle::Destroy()
{


	//Destroy the renderer
	if (m_renderer != NULL)
		delete m_renderer;
}