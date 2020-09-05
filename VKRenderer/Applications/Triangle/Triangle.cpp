#include "Triangle.h"

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


	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsofVertex()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDesc = {};

		attributeDesc.resize(3);

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

	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "Basic.vert";
	ShaderFileNames[1] = "Basic.frag";

	TrianglePipeline.ShaderFileNames = ShaderFileNames;

	// Vertex Input
	TrianglePipeline.vertexBindingDesc = TriangleVertex::getBindingDescription();
	TrianglePipeline.AttributeDescriptionsofVertex = TriangleVertex::getAttributeDescriptionsofVertex();
	
	// Input Assembly
	TrianglePipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		

	// Rasterizer
	TrianglePipeline.polygonMode = VK_POLYGON_MODE_FILL;
	TrianglePipeline.cullMode = VK_CULL_MODE_BACK_BIT;
	TrianglePipeline.frontFaceCullingMode = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	TrianglePipeline.depthBiasEnableMode = VK_FALSE;
	

	//Create Pipeline Layout b4 creating Graphics Pipeline
	TrianglePipeline.a_descriptorSetLayout = m_descriptorSetLayout;

	TrianglePipeline.renderPass = m_renderPass;
	TrianglePipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&TrianglePipeline);

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
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

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

	m_renderer->CreateRenderPass(renderPassdesc, &m_renderPass);

	
}

//===================================================================
//Creating Frame Buffers
//===================================================================
void Triangle::CreateFrameBuffers()
{
	m_renderer->m_swapChainFrameBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImageViews.size());

	for (uint32_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImageViews.size(); ++i)
	{
		std::vector<VkImageView> attachmentsVector = { m_renderer->m_swapChainDescription.m_SwapChainImageViews[i] };

		m_FBO.attachmentCount = 1;
		m_FBO.FBOHeight = m_renderer->m_swapChainDescription.m_swapChainExtent.height;
		m_FBO.FBOWidth	= m_renderer->m_swapChainDescription.m_swapChainExtent.width;
		//m_FBO.FrameBuffer = m_renderer->m_swapChainFrameBuffer[i].FrameBuffer;
		m_FBO.Attachments = attachmentsVector;

		m_renderer->CreateFrameBuffer(m_FBO, m_renderPass, &m_renderer->m_swapChainFrameBuffer[i].FrameBuffer);
	}

}

//===================================================================
//Command Pool
//===================================================================

void Triangle::CreateCommandPool()
{
	m_renderer->CreateCommandPool(&m_commandPool);
}

//===================================================================
//Command Buffers
//===================================================================

void Triangle::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_renderer->m_swapChainFrameBuffer.size());
	
	//Allocate Command Buffer
	m_renderer->AllocateCommandBuffers(m_commandBuffers, m_commandPool);

	/*VkCommandBufferAllocateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	createInfo.commandPool = m_commandPool;
	createInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();


	if (vkAllocateCommandBuffers(m_renderer->m_device, &createInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Command Buffers");
	}*/

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
			renderpassBeginInfo.framebuffer = m_renderer->m_swapChainFrameBuffer[i].FrameBuffer;// m_swapChainFrameBuffer[i];
			renderpassBeginInfo.renderArea.offset = { 0,0 };
			renderpassBeginInfo.renderArea.extent = m_renderer->m_swapChainDescription.m_swapChainExtent;


			//Clear Color//
			VkClearValue clearColor = { 0.0,0.0,0.0,1.0 };
			renderpassBeginInfo.clearValueCount = 1;
			renderpassBeginInfo.pClearValues = &clearColor;


			vkCmdBeginRenderPass(m_commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, TrianglePipeline.a_Pipeline);

				VkBuffer vertexBuffers[] = { m_TriangleVertexBuffer.Buffer };
				VkDeviceSize offset = { 0 };
				vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &m_TriangleVertexBuffer.Buffer, &offset);

				vkCmdBindIndexBuffer(m_commandBuffers[i], m_RectangleIndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);

				//vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(Triangle_vertices.size()), 1, 0, 0);

				vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, TrianglePipeline.a_pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

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
	m_renderer->CreateSemaphoresandFences();
}


//=====================================================================================================================
//Create & Update Uniform Buffer[Generalize this]
//=====================================================================================================================


void Triangle::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_TriangleUniformBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImages.size());
	
	for (int i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)

	{		
		m_renderer->CreateBufferWithoutStaging(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_TriangleUniformBuffer[i].Buffer, m_TriangleUniformBuffer[i].BufferMemory);
	}

	return;
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
	mvp_UBO.ProjectionMatrix = glm::perspective(glm::radians(45.0f), m_renderer->m_swapChainDescription.m_swapChainExtent.width / (float)m_renderer->m_swapChainDescription.m_swapChainExtent.height, 0.1f, 10.0f);
	mvp_UBO.ProjectionMatrix[1][1] *= -1;


	//Copy the data

	void* data;

	vkMapMemory(m_renderer->m_device, m_TriangleUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_renderer->m_device, m_TriangleUniformBuffer[a_imageIndex].BufferMemory);


}

//===================================================================
//Create Index Buffer
//===================================================================

void Triangle::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Rectangle_Indices[0]) * Rectangle_Indices.size();

	//Create Staging Buffer before transfering
	/*VkBuffer stagingBuffer;
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
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);7*/

	m_renderer->CreateBuffer(Rectangle_Indices.data(), bufferSize, &m_RectangleIndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_commandPool);

}

//===================================================================
//Creaate Vertex Buffer
//===================================================================

void Triangle::CreateVertexBuffer()//Make this Generic
{

	VkDeviceSize bufferSize = sizeof(Rectangle_vertices[0]) * Rectangle_vertices.size();

	//Create Staging Buffer before transfering
	//VkBuffer stagingBuffer;
	//VkDeviceMemory stagingBufferMemory;
	//CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	stagingBuffer, stagingBufferMemory);


	//void* data;
	//vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	//	memcpy(data, Rectangle_vertices.data(), (size_t)bufferSize);
	//vkUnmapMemory(m_device, stagingBufferMemory);


	//CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//			 m_TriangleVertexBuffer.Buffer, m_TriangleVertexBuffer.BufferMemory);

	//CopyBuffer(stagingBuffer, m_TriangleVertexBuffer.Buffer, bufferSize);

	////Get rid of the staging buffers
	//vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	//vkFreeMemory(m_device, stagingBufferMemory, nullptr);


	m_renderer->CreateBuffer(Rectangle_vertices.data(), bufferSize, &m_TriangleVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_commandPool);
	
	
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

	std::vector<VkDescriptorSetLayoutBinding> layoutBindingVector;

	layoutBindingVector.reserve(1);

	layoutBindingVector.push_back(layoutBinding);

	m_renderer->CreateDescriptorSetLayout(layoutBindingVector, &m_descriptorSetLayout);
}

//=====================================================================================================================
// Descriptor Pool & Set Creation
//=====================================================================================================================

void Triangle::CreateDescriptorPool()
{

	std::vector< VkDescriptorPoolSize > poolSize = {};

	poolSize.resize(1);

	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.m_SwapChainImages.size());

	m_renderer->CreateDescriptorPool(poolSize, static_cast<uint32_t>(m_renderer->m_swapChainDescription.m_SwapChainImages.size()),
		1, &m_DescriptorPool);


	/*VkDescriptorPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &poolSize;
	createInfo.maxSets = static_cast<uint32_t>(m_SwapChainImages.size());


	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create Desciptor Pool");
	}*/

}

void Triangle::CreateDesciptorSets()
{
	std::vector<VkWriteDescriptorSet> descriptorWriteInfo;

	descriptorWriteInfo.resize(1);

	descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteInfo[0].dstBinding = 0;
	descriptorWriteInfo[0].dstArrayElement = 0;
	descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWriteInfo[0].descriptorCount = 1;
	descriptorWriteInfo[0].pImageInfo = nullptr;
	descriptorWriteInfo[0].pTexelBufferView = nullptr;

	m_renderer->CreateDesciptorSets(m_renderer->m_swapChainDescription.m_SwapChainImages.size(), m_descriptorSetLayout,
		m_TriangleUniformBuffer, sizeof(UniformBufferObject), descriptorWriteInfo, m_DescriptorPool,
		m_DescriptorSets);



}

//===================================================================
//Recreating Swap Chains
//===================================================================

void Triangle::ReCreateSwapChain()
{

	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_renderer->m_window, &width, &height);
		glfwWaitEvents();
	}


	//vkDeviceWaitIdle(m_device);
	vkDeviceWaitIdle(m_renderer->getDevice());

	//TODO: call VkRenderer
	//CleanUpSwapChain();

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
	vkWaitForFences(m_renderer->m_device, 1, &m_renderer->m_inflightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_renderer->m_device, 1, &m_renderer->m_inflightFences[m_currentFrame]);

	//===================================================================
	//1.Acquire Image form SwapChain
	//2.Execute command buffer on that image
	//3.Return Image to swap chain for presentation
	//===================================================================

	uint32_t imageIndex;

	VkResult result = m_renderer->AcquireNextImage(&imageIndex, m_currentFrame);


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

	FrameSubmissionDesc submissionDesc = {};

	submissionDesc.imageIndex = &imageIndex;
	submissionDesc.commandBufferCount = 1;
	submissionDesc.commandBuffer = &m_commandBuffers[imageIndex];
	submissionDesc.currentFrameNumber = m_currentFrame;
	submissionDesc.result = result;

	m_renderer->SubmissionAndPresentation(submissionDesc);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Triangle::Destroy()
{
	vkFreeCommandBuffers(m_renderer->m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, TrianglePipeline.a_Pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, TrianglePipeline.a_pipelineLayout, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)
	{
		//Triangle's UBO
		vkDestroyBuffer(m_renderer->m_device, m_TriangleUniformBuffer[i].Buffer, nullptr);
		vkFreeMemory(m_renderer->m_device, m_TriangleUniformBuffer[i].BufferMemory, nullptr);
	}

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	//Destroy Rectangle Index Buffer
	vkDestroyBuffer(m_renderer->m_device, m_RectangleIndexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_RectangleIndexBuffer.BufferMemory, nullptr);

	//Destroy Triangle Index Buffer
	vkDestroyBuffer(m_renderer->m_device, m_TriangleVertexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_TriangleVertexBuffer.BufferMemory, nullptr);

	vkDestroyCommandPool(m_renderer->m_device, m_commandPool, nullptr);

	// Remove all the Vulkan related intialized values
	m_renderer->Destroy();

	//Destroy the renderer
	if (m_renderer != NULL)
		delete m_renderer;
}