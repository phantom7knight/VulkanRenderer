
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

