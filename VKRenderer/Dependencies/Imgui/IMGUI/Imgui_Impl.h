#pragma once

#include "Source Files/imgui.h"
#include "Source Files/imgui_impl_glfw.h"
#include "Source Files/imgui_impl_vulkan.h"

class Imgui_Impl
{
public:
	Imgui_Impl() :m_showGuiWindow(true)
	{
	}
	~Imgui_Impl() 
	{
		//delete the instance
		if (m_instance)
			delete m_instance;
	}


	void Init(GLFWwindow* a_window, ImGui_ImplVulkan_InitInfo a_GUIInitInfo, VkRenderPass a_renderPass,
		VkCommandPool a_cmdPool);

	void Gui_BeginFrame();
	void Gui_Render(VkCommandBuffer command_buffer);

	void DrawGui(VkCommandBuffer a_cmdBuffer);

	void DestroyGui(VkDevice a_device);

	void CreateGUIDescriptorPool(VkDevice a_device);
	//void CreateCommandPool(VkCommandPoolCreateFlags a_poolFlags, uint32_t a_queueFamilyIndex, VkDevice a_device);

	static Imgui_Impl* getInstance()
	{
		if (m_instance == NULL)
			return new Imgui_Impl();

		return m_instance;
	}

	Imgui_Impl* operator=(const Imgui_Impl& op) = delete;

	bool				m_showGuiWindow;
private:
	VkDescriptorPool	m_guiDescriptorPool;
	VkCommandPool		m_guiCommandPool;


	static Imgui_Impl* m_instance;

};

