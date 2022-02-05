#include "ComputeCull.h"
#include "../../Dependencies/Imgui/IMGUI/Imgui_Impl.h"
#include "../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"
#include "../../VKRenderer/Core/Camera/Camera.h"

// TODO: add Camera class include if need be


ComputeCull::ComputeCull() : m_showGUILight(true), m_showPhongGUILight(false), m_showBRDFGUILight(true)
{
	//Initialize Renderer
	m_renderer = new vkRenderer();

}


ComputeCull::~ComputeCull()
{
}

void ComputeCull::SetUpCameraProperties(Camera* a_cam)
{
	//SetUp Camera Properties
	a_cam->SetPosition(glm::vec3(0.0, 0.0, 5.0f));
	//a_cam->camProperties.rotation_speed	   = 0.2f;
	//a_cam->camProperties.translation_speed = 0.002f;

	//set proj matrix
	a_cam->SetPerspective(glm::radians(45.0f), (float)m_renderer->m_swapChainDescription.m_swapChainExtent.width / (float)m_renderer->m_swapChainDescription.m_swapChainExtent.height, 0.1f, 1000.0f);

}

void ComputeCull::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = m_renderer->m_swapChainDescription.m_swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//TODO : Programmable

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass shud start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//How render pass final image shud translate at end of render pass

	VkAttachmentDescription depthAttachment = {};

	depthAttachment.format = m_renderer->FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass shud start with
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		//How render pass final image shud translate at end of render pass


	//Each renderpass can have multiple sub-passes
	//which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};

	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;
	subpassInfo.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//array of attachments for this render pass
	std::vector< VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };

	std::vector<VkAttachmentReference> attachmentReferences;

	attachmentReferences.resize(attachments.size());

	attachmentReferences[0] = colorAttachmentRef;
	attachmentReferences[1] = depthAttachmentRef;

	std::vector<VkSubpassDependency> subPassDependency = { dependency };

	RenderPassInfo renderPassdesc = {};

	renderPassdesc.attachmentDescriptions = attachments;
	renderPassdesc.attachmentReferences = attachmentReferences;
	renderPassdesc.subpassDependecy = subPassDependency;
	renderPassdesc.subpassInfo = subpassInfo;

	m_renderer->CreateRenderPass(renderPassdesc, &m_renderPass);
}

void ComputeCull::CreateDescriptorSetLayout()
{
	//create binding for UBO
	//used in vertex shader
	VkDescriptorSetLayoutBinding layoutBinding = {};

	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding LightlayoutBinding = {};

	LightlayoutBinding.binding = 1;
	LightlayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LightlayoutBinding.descriptorCount = 1;
	LightlayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	LightlayoutBinding.pImmutableSamplers = nullptr;



	//create an vector of descriptors
	std::vector< VkDescriptorSetLayoutBinding> descriptorsVector = { layoutBinding , LightlayoutBinding };

	m_renderer->CreateDescriptorSetLayout(descriptorsVector, &m_descriptorSetLayout);
}

void ComputeCull::CreateGraphicsPipeline()
{
	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "Model.vert";
	ShaderFileNames[1] = "Model.frag";

	ModelGraphicsPipeline.ShaderFileNames = ShaderFileNames;

	// Vertex Input
	ModelGraphicsPipeline.vertexBindingDesc = m_renderer->rsrcLdr.getModelLoaderobj().getBindingDescription();;
	ModelGraphicsPipeline.AttributeDescriptionsofVertex = m_renderer->rsrcLdr.getModelLoaderobj().getAttributeDescriptionsofVertex();

	//Input Assembly
	ModelGraphicsPipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterizer
	ModelGraphicsPipeline.polygonMode = VK_POLYGON_MODE_FILL;
	ModelGraphicsPipeline.cullMode = VK_CULL_MODE_BACK_BIT;
	ModelGraphicsPipeline.frontFaceCullingMode = VK_FRONT_FACE_CLOCKWISE;
	ModelGraphicsPipeline.depthBiasEnableMode = VK_FALSE;

	// Depth Testing
	ModelGraphicsPipeline.depthTestEnable = VK_TRUE;
	ModelGraphicsPipeline.depthWriteEnable = VK_TRUE;
	ModelGraphicsPipeline.depthCompareOperation = VK_COMPARE_OP_LESS;
	ModelGraphicsPipeline.stencilTestEnable = VK_FALSE;

	//Create Pipeline Layout b4 creating Graphics Pipeline
	ModelGraphicsPipeline.a_descriptorSetLayout = m_descriptorSetLayout;

	ModelGraphicsPipeline.renderPass = m_renderPass;
	ModelGraphicsPipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&ModelGraphicsPipeline);
}

void ComputeCull::CreateFrameBuffers()
{
	m_renderer->m_swapChainFrameBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImageViews.size());

	for (uint32_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImageViews.size(); ++i)
	{
		std::vector< VkImageView> attachments = { m_renderer->m_swapChainDescription.m_SwapChainImageViews[i],
													depthImageView };

		m_FBO.attachmentCount = static_cast<uint32_t>(attachments.size());
		m_FBO.Attachments = attachments;
		m_FBO.FBOWidth  = static_cast<float>(m_renderer->m_swapChainDescription.m_swapChainExtent.width);
		m_FBO.FBOHeight = static_cast<float>(m_renderer->m_swapChainDescription.m_swapChainExtent.height);

		m_renderer->CreateFrameBuffer(m_FBO, m_renderPass, &m_renderer->m_swapChainFrameBuffer[i].FrameBuffer);
	}
}

void ComputeCull::CreateCommandPool()
{
	m_renderer->CreateCommandPool(&m_commandPool);
}

void ComputeCull::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(MVPUBO);
	VkDeviceSize lightBufferSize = sizeof(LightInfoUBO);

	m_ModelUniformBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImages.size());
	m_LightInfoUniformBuffer.resize(m_renderer->m_swapChainDescription.m_SwapChainImages.size());

	for (int i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)

	{
		m_renderer->CreateBufferWithoutStaging(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_ModelUniformBuffer[i].Buffer, m_ModelUniformBuffer[i].BufferMemory);

		m_renderer->CreateBufferWithoutStaging(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_LightInfoUniformBuffer[i].Buffer, m_LightInfoUniformBuffer[i].BufferMemory);
	}

	return;
}

void ComputeCull::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	poolSizes.resize(2);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.m_SwapChainImages.size());

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.m_SwapChainImages.size());

	m_renderer->CreateDescriptorPool(poolSizes, static_cast<uint32_t>(m_renderer->m_swapChainDescription.m_SwapChainImages.size()),
		static_cast<uint32_t>(poolSizes.size()), &m_DescriptorPool);

}

void ComputeCull::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_renderer->m_swapChainDescription.m_SwapChainImages.size(), m_descriptorSetLayout);

	m_renderer->AllocateDescriptorSets(m_DescriptorPool, layouts, m_DescriptorSets);

	for (size_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};

		bufferInfo.buffer = m_ModelUniformBuffer[i].Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(MVPUBO);

		VkDescriptorBufferInfo lightBufferInfo = {};

		lightBufferInfo.buffer = m_LightInfoUniformBuffer[i].Buffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(LightInfoUBO);

		std::vector< VkWriteDescriptorSet> descriptorWriteInfo = {};

		descriptorWriteInfo.resize(2);

		descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[0].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[0].dstBinding = 0;
		descriptorWriteInfo[0].dstArrayElement = 0;
		descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo[0].descriptorCount = 1;
		descriptorWriteInfo[0].pBufferInfo = &bufferInfo;
		descriptorWriteInfo[0].pImageInfo = nullptr;
		descriptorWriteInfo[0].pTexelBufferView = nullptr;

		descriptorWriteInfo[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[1].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[1].dstBinding = 1;
		descriptorWriteInfo[1].dstArrayElement = 0;
		descriptorWriteInfo[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo[1].descriptorCount = 1;
		descriptorWriteInfo[1].pBufferInfo = &lightBufferInfo;
		descriptorWriteInfo[1].pImageInfo = nullptr;
		descriptorWriteInfo[1].pTexelBufferView = nullptr;

		m_renderer->UpdateDescriptorSets(descriptorWriteInfo);

	}
}

void ComputeCull::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_renderer->m_swapChainFrameBuffer.size());

	m_renderer->AllocateCommandBuffers(m_commandBuffers, m_commandPool);

}

void ComputeCull::CreateSemaphoresandFences()
{
	m_renderer->CreateSemaphoresandFences();
}

void ComputeCull::UpdateUniformBuffer(uint32_t a_imageIndex, CameraMatrices properties_Cam)
{

#pragma region MVP_Update
	MVPUBO mvp_UBO = {};

	//Model Matrix
	mvp_UBO.ModelMatrix = glm::mat4(1);
	mvp_UBO.ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
	//mvp_UBO.ModelMatrix = glm::rotate(mvp_UBO.ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvp_UBO.ModelMatrix = glm::scale(mvp_UBO.ModelMatrix, glm::vec3(1.0f));


	//View Matrix
	mvp_UBO.ViewMatrix = cam_matrices.view;

	//Projection Matrix
	mvp_UBO.ProjectionMatrix = cam_matrices.perspective;
	mvp_UBO.ProjectionMatrix[1][1] *= -1;

	//Copy the data

	void* data;

	vkMapMemory(m_renderer->m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_renderer->m_device, m_ModelUniformBuffer[a_imageIndex].BufferMemory);
#pragma endregion

#pragma region LightInfo_Update
	LightInfoUBO lightInfo_UBO = {};

	//lightPosition
	lightInfo_UBO.lightPosition = m_lightPosGUILight;

	lightInfo_UBO.lightColor = m_lightColorGUILight;

	lightInfo_UBO.specularIntensity = m_SpecularIntensityGUILight;

	// TODO: Fix This
	lightInfo_UBO.camPosition = m_renderer->m_MainCamera->GetCameraPos();

	lightInfo_UBO.lightModel = m_lightModelGUILight;

	//Copy the data

	data = NULL;

	vkMapMemory(m_renderer->m_device, m_LightInfoUniformBuffer[a_imageIndex].BufferMemory, 0, sizeof(lightInfo_UBO), 0, &data);
	memcpy(data, &lightInfo_UBO, sizeof(lightInfo_UBO));
	vkUnmapMemory(m_renderer->m_device, m_LightInfoUniformBuffer[a_imageIndex].BufferMemory);

#pragma endregion

}

void ComputeCull::DrawGui(VkCommandBuffer a_cmdBuffer)
{
	Imgui_Impl::getInstance()->Gui_BeginFrame();

	if (m_showGUILight)
	{
		ImGui::Begin("Light Properties", &m_showGUILight);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

		ImGui::SliderFloat3("Light Position", &m_lightPosGUILight.x, -200.0f, 200.0f);

		ImGui::SliderFloat3("Light Color", &m_lightColorGUILight.x, 0.0f, 1.0f);

		ImGui::SliderInt("Spec Intensity", &m_SpecularIntensityGUILight, 32, 256);

		ImGui::End();
	}


	Imgui_Impl::getInstance()->Gui_Render(a_cmdBuffer);
}

void ComputeCull::UpdateCommandBuffers(uint32_t a_imageIndex)
{
	uint32_t i = a_imageIndex;
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
		renderpassBeginInfo.framebuffer = m_renderer->m_swapChainFrameBuffer[i].FrameBuffer;
		renderpassBeginInfo.renderArea.offset = { 0,0 };
		renderpassBeginInfo.renderArea.extent = m_renderer->m_swapChainDescription.m_swapChainExtent;


		//Clear Color//
		VkClearValue clearColor[2];
		clearColor[0] = { 0.0,0.0,0.0,1.0 };
		clearColor[1] = { 1.0f, 0.0f };
		renderpassBeginInfo.clearValueCount = 2;
		renderpassBeginInfo.pClearValues = clearColor;


		vkCmdBeginRenderPass(m_commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ModelGraphicsPipeline.a_pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ModelGraphicsPipeline.a_Pipeline);

		VkDeviceSize offset = { 0 };

		//Bind Vertex Buffer
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &VertexBUffer.Buffer, &offset);

		//Bind Index Buffer
		vkCmdBindIndexBuffer(m_commandBuffers[i], IndexBUffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		//Call Draw Indexed for the model
		vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_indexBufferCount), 1, 0, 0, 0);

		//==================================================
		//Draw UI
		DrawGui(m_commandBuffers[i]);

		//==================================================

		vkCmdEndRenderPass(m_commandBuffers[i]);

	//End Recording
	if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record Command Buffer");
	}

}

void ComputeCull::ReCreateSwapChain()
{
	/*
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}


		vkDeviceWaitIdle(m_device);

		Destroy();

		CleanUpSwapChain();

		CreateSwapChain();

		CreateSwapChainImageView();

		CreateRenderPass();

		CreateGraphicsPipeline();

		CreateDepthResources();

		CreateFrameBuffers();

		CreateUniformBuffer();

		CreateCommandBuffers();

		CreateDescriptorPool();

		CreateDescriptorSets();

		CreateCommandPool();

		*/
}

void ComputeCull::SetUpVertexBuffer(const ModelInfoData a_modelDesc, BufferDesc* a_VertexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.vertexBufferSize;

	m_renderer->CreateBuffer(a_modelDesc.vertexbufferData.data(), bufferSize, a_VertexBUffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		m_commandPool);
}

void ComputeCull::SetUpIndexBuffer(const ModelInfoData a_modelDesc, BufferDesc* a_IndexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.indexBufferSize;

	m_renderer->CreateBuffer(a_modelDesc.indexbufferData.data(), bufferSize, a_IndexBUffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		m_commandPool);

	return;
}

void ComputeCull::LoadAModel(std::string fileName)
{
	ModelInfoData modelinfor = m_renderer->rsrcLdr.LoadModelResource(fileName);

	//Load Index and Vertex Buffer
	SetUpVertexBuffer(modelinfor, &VertexBUffer);
	SetUpIndexBuffer(modelinfor, &IndexBUffer);

	m_indexBufferCount = static_cast<uint32_t>(modelinfor.indexbufferData.size());

}

void ComputeCull::CreateDepthResources()
{
	VkFormat depthFormat = m_renderer->FindDepthFormat();

	depthImageInfo.ImageHeight = m_renderer->m_swapChainDescription.m_swapChainExtent.height;
	depthImageInfo.ImageWidth = m_renderer->m_swapChainDescription.m_swapChainExtent.width;
	depthImageInfo.imageFormat = depthFormat;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	m_renderer->CreateImage(&depthImageInfo);

	m_renderer->CreateImageView(depthImageInfo.BufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageView);

	//TransitionImageLayouts(depthImageInfo.BufferImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

void ComputeCull::setGuiVariables()
{
	m_lightPosGUILight = glm::vec3(91.30, -73.913, 160.870);
	m_lightColorGUILight = glm::vec3(1.0, 1.0, 1.0);
	m_SpecularIntensityGUILight = 32;
	m_lightModelGUILight = 0;
	m_roughnessGUILight = 1.058f;
}

void ComputeCull::InitGui()
{

	setGuiVariables();

	ImGui_ImplVulkan_InitInfo imguiInfo = {};

	imguiInfo.Instance = m_renderer->m_VulkanInstance;
	imguiInfo.Allocator = nullptr;
	imguiInfo.Device = m_renderer->m_device;
	imguiInfo.PhysicalDevice = m_renderer->m_physicalDevice;
	imguiInfo.DescriptorPool = nullptr;//this will be gui created descriptor pool
	imguiInfo.ImageCount = IMAGE_COUNT;
	imguiInfo.MinImageCount = 2;
	imguiInfo.Queue = m_renderer->m_graphicsQueue;
	imguiInfo.QueueFamily = m_renderer->FindQueueFamalies().graphicsFamily.value();
	imguiInfo.PipelineCache = nullptr;


	//Init the GUI for IMGUI
	Imgui_Impl::getInstance()->Init(m_renderer->m_window, imguiInfo, m_renderPass, m_commandPool);


}

//==========================================================================================================

void ComputeCull::Init()
{
	m_renderer->Init();

	return;
}

void ComputeCull::PrepareApp()
{
	m_renderer->PrepareApp();

	CreateRenderPass();

	CreateDescriptorSetLayout();

	CreateCommandPool();

	CreateDepthResources();

	CreateFrameBuffers();

	CreateGraphicsPipeline();

#pragma region Model_Load
	LoadAModel("../../Assets/Models/monkey/monkey.obj");
	//LoadAModel("../../Assets/Models/cornell_box/cornell_box.obj");
	//LoadAModel("../../Assets/Models/VulkanScene/vulkanscene_shadow.dae");
	//LoadAModel("../../Assets/Models/venus/venus.fbx");
#pragma endregion

	CreateUniformBuffer();

	CreateDescriptorPool();

	CreateDescriptorSets();

	CreateCommandBuffers();

	CreateSemaphoresandFences();

	//// set up the camera position
	SetUpCameraProperties(m_renderer->m_MainCamera);

	//Initialize Dear ImGui
	InitGui();

}

void ComputeCull::Update(double deltaTime)
{
	m_renderer->ProcessInput(m_renderer->m_window, deltaTime);

	cam_matrices.perspective = m_renderer->m_MainCamera->GetpersepectiveMatrix();

	//set view matrix
	cam_matrices.view = m_renderer->m_MainCamera->GetViewMatrix();

	return;
}

void ComputeCull::Draw(double deltaTime)
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

	//update our command buffers
	UpdateCommandBuffers(imageIndex);

	//Update Uniform Buffers which needs to be sent to Shader every frames
	UpdateUniformBuffer(imageIndex, cam_matrices);

	FrameSubmissionDesc submissionDesc = {};

	submissionDesc.imageIndex = &imageIndex;
	submissionDesc.commandBufferCount = 1;
	submissionDesc.commandBuffer = &m_commandBuffers[imageIndex];
	submissionDesc.currentFrameNumber = m_currentFrame;
	submissionDesc.result = result;

	m_renderer->SubmissionAndPresentation(submissionDesc);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void ComputeCull::Destroy()
{
	//depth Image
	vkDestroyImageView(m_renderer->m_device, depthImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, depthImageInfo.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, depthImageInfo.BufferMemory, nullptr);

	Imgui_Impl::getInstance()->DestroyGui(m_renderer->m_device);

	vkFreeCommandBuffers(m_renderer->m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, ModelGraphicsPipeline.a_Pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, ModelGraphicsPipeline.a_pipelineLayout, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_renderer->m_swapChainDescription.m_SwapChainImages.size(); ++i)
	{
		//Model's UBO
		vkDestroyBuffer(m_renderer->m_device, m_ModelUniformBuffer[i].Buffer, nullptr);
		vkFreeMemory(m_renderer->m_device, m_ModelUniformBuffer[i].BufferMemory, nullptr);

		//Light's UBO
		vkDestroyBuffer(m_renderer->m_device, m_LightInfoUniformBuffer[i].Buffer, nullptr);
		vkFreeMemory(m_renderer->m_device, m_LightInfoUniformBuffer[i].BufferMemory, nullptr);
	}

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	//Destroy Model's Index Buffer
	vkDestroyBuffer(m_renderer->m_device, m_ModelIndexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_ModelIndexBuffer.BufferMemory, nullptr);

	//Destroy Model's Vertex Buffer
	vkDestroyBuffer(m_renderer->m_device, m_ModelVertexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_ModelVertexBuffer.BufferMemory, nullptr);

	vkDestroyCommandPool(m_renderer->m_device, m_commandPool, nullptr);

	// Remove all the Vulkan related intialized values
	m_renderer->Destroy();

	//Destroy the renderer
	if (m_renderer != NULL)
		delete m_renderer;

}