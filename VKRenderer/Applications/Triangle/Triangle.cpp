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
	
	//Input Assembly
	TrianglePipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
	//Rasterizer

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
	m_renderer->m_swapChainFrameBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImageViews.size());

	for (uint32_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImageViews.size(); ++i)
	{
		VkImageView attachments[] = { m_renderer->m_swapChainDescription.m_SwapChainImageViews[i] };


		m_FBO.attachmentCount = 1;
		m_FBO.FBOHeight = m_renderer->m_swapChainDescription.m_swapChainExtent.height;
		m_FBO.FBOWidth	= m_renderer->m_swapChainDescription.m_swapChainExtent.width;
		m_FBO.FrameBuffer = m_renderer->m_swapChainFrameBuffer[i].FrameBuffer;
		m_FBO.Attachments = attachments;

		m_renderer->CreateFrameBuffer(m_FBO, m_renderPass);
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
	
	for (int i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)

	{
		m_renderer->CreateBuffer(, bufferSize, &m_TriangleUniformBuffer[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_commandPool);

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

	std::vector<VkDescriptorSetLayoutBinding> layoutBindingVector;

	layoutBindingVector.resize(1);

	layoutBindingVector.push_back(layoutBinding);

	m_renderer->CreateDescriptorSetLayout(layoutBindingVector, &m_descriptorSetLayout);
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