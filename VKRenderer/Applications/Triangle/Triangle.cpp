#include "Triangle.h"
#include "../../VKRenderer/stdafx.h"


//===================================================================
//Vertex Buffer Use
//===================================================================

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normals;
	glm::vec2 TexCoords;


	//Set Vertex Binding Desc i.e like setting VAO in OpenGL

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc = {};

		bindingDesc.binding = 0;	//Since we are using only one array for the data that is Triangle_vertices we have 1 binding and order starts from 0
		bindingDesc.stride = sizeof(Vertex);
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
		attributeDesc[0].offset = offsetof(Vertex, Position);

		//Normals
		attributeDesc[1].binding = 0;
		attributeDesc[1].location = 1;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[1].offset = offsetof(Vertex, Normals);

		//TexCoords
		attributeDesc[2].binding = 0;
		attributeDesc[2].location = 2;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[2].offset = offsetof(Vertex, TexCoords);


		return attributeDesc;
	}



};


const std::vector<Vertex> Triangle_vertices = {
	{	{0.0,-0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{0.5, 0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	},
	{	{-0.5,0.5,0.0},		{0.0,0.0,1.0},		{0.0,0.0}	}
};

const std::vector<Vertex> Rectangle_vertices = {
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



//===================================================================

#pragma region Vulkan-Helper-Functions

uint32_t vkRenderer::findMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");


}



void vkRenderer::CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_properties, VkBuffer& a_buffer, VkDeviceMemory& a_bufferMemory)
{
	/////For EX: Buffer for the coordinates of Triangle which are being sent to the VS
	VkBufferCreateInfo bufferCreateInfo = {};

	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = a_size;
	bufferCreateInfo.usage = a_usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &a_buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to Create Buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, a_buffer, &memRequirements);

	VkMemoryAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, a_properties);// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//Allocated memory for the Vertex Buffer
	if (vkAllocateMemory(m_device, &allocateInfo, nullptr, &a_bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate memory for the Buffer");
	}
	vkBindBufferMemory(m_device, a_buffer, a_bufferMemory, 0);


}


void vkRenderer::CopyBuffer(VkBuffer a_srcBuffer, VkBuffer a_dstBuffer, VkDeviceSize a_size)
{
	VkCommandBufferAllocateInfo allocInfo = {};

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	//Start recording Command Buffer

	VkCommandBufferBeginInfo bufferBeginInfo = {};

	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);

	VkBufferCopy copyRegion = {};

	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = a_size;

	vkCmdCopyBuffer(commandBuffer, a_srcBuffer, a_dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);
	vkFreeCommandBuffers(m_device, m_CommandPool, 1, &commandBuffer);


}

#pragma endregion

//=====================================================================================================================
//Create & Update Uniform Buffer[Generalize this]
//=====================================================================================================================


void Triangle::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(m_SwapChainImages.size());
	m_uniformBuffersMemory.resize(m_SwapChainImages.size());

	for (int i = 0; i < m_SwapChainImages.size(); ++i)
	{
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);
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

	vkMapMemory(m_device, m_uniformBuffersMemory[a_imageIndex], 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_device, m_uniformBuffersMemory[a_imageIndex]);


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
		m_RectangleIndexBuffer, m_IndexBufferMemory);

	CopyBuffer(stagingBuffer, m_RectangleIndexBuffer, bufferSize);

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
				 m_TriangleVertexBuffer, m_vertexBufferMemory);

	CopyBuffer(stagingBuffer, m_TriangleVertexBuffer, bufferSize);

	//Get rid of the staging buffers
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
	
	
}


//===================================================================
// Create and Set Descriptor Layouts[maybe generalize?]
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

		bufferInfo.buffer = m_uniformBuffers[i];
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





void Triangle::PrepareApp()
{
	//First call base app's Preperations
	vkRenderer::PrepareApp();

	CreateUniformBuffer();
	CreateIndexBuffer();
	CreateVertexBuffer();
	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	CreateDesciptorSets();

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

}

