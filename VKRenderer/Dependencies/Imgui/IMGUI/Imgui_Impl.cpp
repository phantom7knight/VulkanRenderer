#include <iostream>

#include "Imgui_Impl.h"

Imgui_Impl* Imgui_Impl::m_instance = nullptr;

VkCommandBuffer BeginCmdBuffer(VkDevice a_device, VkCommandPool a_CommandPool)
{
    VkCommandBufferAllocateInfo allocInfo = {};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = a_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(a_device, &allocInfo, &commandBuffer);

    //Start recording Command Buffer

    VkCommandBufferBeginInfo bufferBeginInfo = {};

    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo);

    return commandBuffer;
}


void EndCmdBufferOperation(VkCommandBuffer a_commandBuffer, VkQueue a_graphicsQueue, VkDevice a_device, VkCommandPool a_cmdPool)
{
    vkEndCommandBuffer(a_commandBuffer);

    VkSubmitInfo submitInfo = {};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &a_commandBuffer;

    vkQueueSubmit(a_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(a_graphicsQueue);
    vkFreeCommandBuffers(a_device, a_cmdPool, 1, &a_commandBuffer);
}


void Imgui_Impl::Init(GLFWwindow* a_window, ImGui_ImplVulkan_InitInfo a_GUIInitInfo, VkRenderPass a_renderPass,
    VkCommandPool a_cmdPool)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    //setup descriptor pool
    CreateGUIDescriptorPool(a_GUIInitInfo.Device);


    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(a_window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};

    init_info.Instance = a_GUIInitInfo.Instance;
    init_info.PhysicalDevice = a_GUIInitInfo.PhysicalDevice;
    init_info.Device = a_GUIInitInfo.Device;
    init_info.QueueFamily = a_GUIInitInfo.QueueFamily;
    init_info.Queue = a_GUIInitInfo.Queue;
    init_info.PipelineCache = a_GUIInitInfo.PipelineCache;
    init_info.DescriptorPool = m_guiDescriptorPool;
    init_info.Allocator = a_GUIInitInfo.Allocator;
    init_info.MinImageCount = a_GUIInitInfo.MinImageCount;
    init_info.ImageCount = a_GUIInitInfo.ImageCount;
    
    ImGui_ImplVulkan_Init(&init_info, a_renderPass);


    // Upload Fonts
    {
        // Use any command queue
        //CreateCommandPool(a_poolFlags, a_queueFamilyIndex, a_GUIInitInfo.Device);
        VkCommandBuffer command_buffer = BeginCmdBuffer(a_GUIInitInfo.Device, a_cmdPool);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        EndCmdBufferOperation(command_buffer, a_GUIInitInfo.Queue, a_GUIInitInfo.Device, a_cmdPool);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

void Imgui_Impl::CreateGUIDescriptorPool(VkDevice a_device)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(a_device, &pool_info, nullptr, &m_guiDescriptorPool) != VK_SUCCESS)
    {
        std::cerr << "Descriptor pool cannot be allocated";
    }

}



// Start the Dear ImGui frame
void Imgui_Impl::Gui_BeginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Imgui_Impl::Gui_Render(VkCommandBuffer command_buffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
}

void Imgui_Impl::DestroyGui(VkDevice a_device)
{
    vkDeviceWaitIdle(a_device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(a_device, m_guiDescriptorPool, nullptr);
}