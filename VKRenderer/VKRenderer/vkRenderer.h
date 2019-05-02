#pragma once
class vkRenderer
{
public:
	vkRenderer();
	~vkRenderer();


	void Init();
	bool InitGLFW();
	bool InitVulkan();

	void mainloop();
	void Run();
	void Update();

	void Destroy();

	static vkRenderer* getInstance();
 
	GLFWwindow* getWindow()
	{
		return m_window;
	}


	//Vulkan Related Functions
	bool CreateInstance();


private:

	static vkRenderer* m_instance;
	GLFWwindow* m_window;


	//Vulkan Related Parameters
	VkInstance m_VulkanInstance;

};

