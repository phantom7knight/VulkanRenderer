#include "stdafx.h"
#include "vkRenderer.h"


//Static variable declaration
vkRenderer* vkRenderer::m_instance = nullptr;

vkRenderer * vkRenderer::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new vkRenderer();
	}

	return m_instance;
}


vkRenderer::vkRenderer()
{
}


vkRenderer::~vkRenderer()
{
}

bool vkRenderer::InitGLFW()
{

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(WIDTH, HEIGHT, "VkRenderer", nullptr, nullptr);

	if (!m_window)
		return false;

	return true;
}


bool vkRenderer::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "Hello World!!";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "VkRenderer";
	appInfo.apiVersion = VK_API_VERSION_1_0;


	//Used to mention the validation layers which we want to use
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwextentioncount = 0;
	const char** glfwExtenstions;

	glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwextentioncount);
	createInfo.enabledExtensionCount = glfwextentioncount;
	createInfo.ppEnabledExtensionNames = glfwExtenstions;
	createInfo.enabledLayerCount = 0;

	VkResult res = vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance);

	if (res != VK_SUCCESS)
	{
		std::cerr << "Failed to Initialize Instance" << std::endl;
		return false;
	}

	return true;
}



bool vkRenderer::InitVulkan()
{
	if (!CreateInstance())
		return false;



	return true;
}


void vkRenderer::Init()
{

	if (!InitGLFW())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
	}

	if (!InitVulkan())
	{
		std::cerr << "Failed to initialize Vulkan" << std::endl;
	}

}

void vkRenderer::Run()
{


}

void vkRenderer::Update()
{


}

void vkRenderer::mainloop()
{
	/*Run();
	Update();*/

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
	}

}



void vkRenderer::Destroy()
{

	//Delete Vulkan related things
	vkDestroyInstance(m_VulkanInstance,nullptr);




	glfwDestroyWindow(m_window);

	glfwTerminate();


}

