#pragma once

#include "Source Files/imgui.h"
#include "Source Files/imgui_impl_glfw.h"
#include "Source Files/imgui_impl_vulkan.h""


class Imgui_Impl
{
public:
	void Init(GLFWwindow* a_window, ImGui_ImplVulkan_InitInfo a_GUIInitInfo, VkRenderPass a_renderPass, VkQueue a_graphicsQueue
		, VkCommandPoolCreateFlags a_poolFlags, uint32_t a_queueFamilyIndex);

	void Gui_BeginFrame();
	void Gui_Render(VkCommandBuffer command_buffer);

	void CreateGUIDescriptorPool(VkDevice a_device);
	void CreateCommandPool(VkCommandPoolCreateFlags a_poolFlags, uint32_t a_queueFamilyIndex, VkDevice a_device);


private:
	VkDescriptorPool	m_guiDescriptorPool;
	VkCommandPool		m_guiCommandPool;

};

