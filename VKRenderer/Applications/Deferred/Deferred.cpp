#include "Deferred.h"
#include "../../Dependencies/Imgui/IMGUI/Imgui_Impl.h"
#include "../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"
#include "../../VKRenderer/Core/Camera/Camera.h"
#include "../../../Dependencies/Optick/src/optick.h"

#define Is_MT_Enabled 0


Deferred::Deferred() : m_showGUILight(true)
{
	//Initialize Renderer
	m_renderer = new vkRenderer();

}

Deferred::~Deferred()
{
}

void Deferred::SetUpCameraProperties(Camera* a_cam)
{
	//SetUp Camera Properties
	a_cam->SetPosition(glm::vec3(0.0, 0.0, 5.0f));
	//a_cam->camProperties.rotation_speed	   = 0.2f;
	//a_cam->camProperties.translation_speed = 0.002f;
	
	//set projection matrix
	a_cam->SetPerspective(glm::radians(45.0f), 
		(float)m_renderer->m_swapChainDescription.swapChainExtent.width / (float)m_renderer->m_swapChainDescription.swapChainExtent.height, 
		0.1f,
		1000.0f);

}

void Deferred::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = m_renderer->m_swapChainDescription.swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//TODO : Programmable

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass should start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//How render pass final image should translate at end of render pass

	VkAttachmentDescription depthAttachment = {};

	depthAttachment.format = m_renderer->FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass should start with
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		//How render pass final image should translate at end of render pass


	//Each render pass can have multiple sub-passes
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

void Deferred::CreateDescriptorSetLayout()
{
	//create binding for UBO
	//used in vertex shader
	VkDescriptorSetLayoutBinding layoutBinding = {};

	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	//create binding for sampler
	//used in pixel shader
	VkDescriptorSetLayoutBinding samplerAlbedoBinding = {};

	samplerAlbedoBinding.binding = 1;
	samplerAlbedoBinding.descriptorCount = 1;
	samplerAlbedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerAlbedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerAlbedoBinding.pImmutableSamplers = nullptr;

	//TODO: check the binding for the layout
	VkDescriptorSetLayoutBinding LightlayoutBinding = {};

	LightlayoutBinding.binding = 2;
	LightlayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LightlayoutBinding.descriptorCount = 1;
	LightlayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	LightlayoutBinding.pImmutableSamplers = nullptr;

	//create binding for sampler
	//used in pixel shader
	VkDescriptorSetLayoutBinding samplerMetallicBinding = {};

	samplerMetallicBinding.binding = 3;
	samplerMetallicBinding.descriptorCount = 1;
	samplerMetallicBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerMetallicBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerMetallicBinding.pImmutableSamplers = nullptr;

	//create binding for sampler
	//used in pixel shader
	VkDescriptorSetLayoutBinding samplerRoughnessBinding = {};

	samplerRoughnessBinding.binding = 4;
	samplerRoughnessBinding.descriptorCount = 1;
	samplerRoughnessBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerRoughnessBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerRoughnessBinding.pImmutableSamplers = nullptr;



	//create an vector of descriptors
	std::vector< VkDescriptorSetLayoutBinding> descriptorsVector = { layoutBinding ,samplerAlbedoBinding, LightlayoutBinding, samplerMetallicBinding, samplerRoughnessBinding };

	m_renderer->CreateDescriptorSetLayout(descriptorsVector, &m_descriptorSetLayout);
}

void Deferred::CreateGraphicsPipeline()
{
	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "Model.vert";
	ShaderFileNames[1] = "Model.frag";

	ModelGraphicsPipeline.shaderFileNames = ShaderFileNames;

	// Vertex Input
	ModelGraphicsPipeline.vertexBindingDesc = m_renderer->rsrcLdr.getModelLoaderobj().getBindingDescription();;
	ModelGraphicsPipeline.attributeDescriptionsofVertex = m_renderer->rsrcLdr.getModelLoaderobj().getAttributeDescriptionsofVertex();

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
	ModelGraphicsPipeline.descriptorSetLayout = m_descriptorSetLayout;

	ModelGraphicsPipeline.renderPass = m_renderPass;
	ModelGraphicsPipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&ModelGraphicsPipeline);
}

void Deferred::CreateFrameBuffers()
{
	m_renderer->m_swapChainFrameBuffer.resize(m_renderer->m_swapChainDescription.swapChainImageViews.size());

	for (uint32_t i = 0; i < m_renderer->m_swapChainDescription.swapChainImageViews.size(); ++i)
	{
		std::vector< VkImageView> attachments = { m_renderer->m_swapChainDescription.swapChainImageViews[i],
													depthImageInfo.imageView };

		m_FBO.attachmentCount = static_cast<uint32_t>(attachments.size());
		m_FBO.attachments = attachments;
		m_FBO.fboWidth = (float)m_renderer->m_swapChainDescription.swapChainExtent.width;
		m_FBO.fboHeight = (float)m_renderer->m_swapChainDescription.swapChainExtent.height;

		m_renderer->CreateFrameBuffer(m_FBO, m_renderPass, &m_renderer->m_swapChainFrameBuffer[i].frameBuffer);
	}
}

void Deferred::CreateCommandPool()
{
	m_renderer->CreateCommandPool(&m_commandPool);
	m_commandPoolList.push_back(m_commandPool);
}

void Deferred::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(ModelUBO);
	VkDeviceSize lightBufferSize = sizeof(LightInfoUBO);

	m_ModelUniformBuffer.resize(m_renderer->m_swapChainDescription.swapChainImages.size());
	m_LightInfoUniformBuffer.resize(m_renderer->m_swapChainDescription.swapChainImages.size());

	for (int i = 0; i < m_renderer->m_swapChainDescription.swapChainImages.size(); ++i)

	{
		m_renderer->CreateBufferWithoutStaging(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_ModelUniformBuffer[i].buffer, m_ModelUniformBuffer[i].bufferMemory);

		m_renderer->CreateBufferWithoutStaging(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_LightInfoUniformBuffer[i].buffer, m_LightInfoUniformBuffer[i].bufferMemory);
	}

	return;
}

void Deferred::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	poolSizes.resize(5);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size());

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size());

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size());

	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size());

	poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[4].descriptorCount = static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size());

	m_renderer->CreateDescriptorPool(poolSizes, static_cast<uint32_t>(m_renderer->m_swapChainDescription.swapChainImages.size()),
		static_cast<uint32_t>(poolSizes.size()), &m_DescriptorPool);

}

void Deferred::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_renderer->m_swapChainDescription.swapChainImages.size(), m_descriptorSetLayout);

	m_renderer->AllocateDescriptorSets(m_DescriptorPool, layouts, m_DescriptorSets);

	for (size_t i = 0; i < m_renderer->m_swapChainDescription.swapChainImages.size(); ++i)
	{
		VkDescriptorBufferInfo bufferInfo = {};

		bufferInfo.buffer = m_ModelUniformBuffer[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ModelUBO);

		VkDescriptorBufferInfo lightBufferInfo = {};

		lightBufferInfo.buffer = m_LightInfoUniformBuffer[i].buffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(LightInfoUBO);

		VkDescriptorImageInfo albedoImageInfo = {};

		albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoImageInfo.sampler = PBRMaterial.albedoMap.Sampler;
		albedoImageInfo.imageView = PBRMaterial.albedoMap.imageView;

		VkDescriptorImageInfo metallicImageInfo = {};

		metallicImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		metallicImageInfo.sampler = PBRMaterial.metallicMap.Sampler;
		metallicImageInfo.imageView = PBRMaterial.metallicMap.imageView;

		VkDescriptorImageInfo roughnessImageInfo = {};

		roughnessImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		roughnessImageInfo.sampler = PBRMaterial.roughnessMap.Sampler;
		roughnessImageInfo.imageView = PBRMaterial.roughnessMap.imageView;

		std::vector< VkWriteDescriptorSet> descriptorWriteInfo = {};

		descriptorWriteInfo.resize(5);

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
		descriptorWriteInfo[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWriteInfo[1].descriptorCount = 1;
		descriptorWriteInfo[1].pImageInfo = &albedoImageInfo;
		descriptorWriteInfo[1].pBufferInfo = nullptr; //TODO: Check this
		descriptorWriteInfo[1].pTexelBufferView = nullptr;

		descriptorWriteInfo[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[2].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[2].dstBinding = 2;
		descriptorWriteInfo[2].dstArrayElement = 0;
		descriptorWriteInfo[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteInfo[2].descriptorCount = 1;
		descriptorWriteInfo[2].pBufferInfo = &lightBufferInfo;
		descriptorWriteInfo[2].pImageInfo = nullptr;
		descriptorWriteInfo[2].pTexelBufferView = nullptr;

		descriptorWriteInfo[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[3].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[3].dstBinding = 3;
		descriptorWriteInfo[3].dstArrayElement = 0;
		descriptorWriteInfo[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWriteInfo[3].descriptorCount = 1;
		descriptorWriteInfo[3].pImageInfo = &metallicImageInfo;
		descriptorWriteInfo[3].pBufferInfo = nullptr; //TODO: Check this
		descriptorWriteInfo[3].pTexelBufferView = nullptr;

		descriptorWriteInfo[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteInfo[4].dstSet = m_DescriptorSets[i];
		descriptorWriteInfo[4].dstBinding = 4;
		descriptorWriteInfo[4].dstArrayElement = 0;
		descriptorWriteInfo[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWriteInfo[4].descriptorCount = 1;
		descriptorWriteInfo[4].pImageInfo = &roughnessImageInfo;
		descriptorWriteInfo[4].pBufferInfo = nullptr; //TODO: Check this
		descriptorWriteInfo[4].pTexelBufferView = nullptr;

		m_renderer->UpdateDescriptorSets(descriptorWriteInfo);
		
	}
}

void Deferred::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_renderer->m_swapChainFrameBuffer.size());

	m_renderer->AllocateCommandBuffers(m_commandBuffers, m_commandPool);

}

void Deferred::CreateSemaphoresandFences()
{
	m_renderer->CreateSemaphoresandFences();
}

void Deferred::UpdateUniformBuffer(uint32_t a_imageIndex , CameraMatrices a_propertiesCam, float a_deltaTime)
{

#pragma region MVP_Update
	ModelUBO mvp_UBO = {};

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

	vkMapMemory(m_renderer->m_device, m_ModelUniformBuffer[a_imageIndex].bufferMemory, 0, sizeof(mvp_UBO), 0, &data);
	memcpy(data, &mvp_UBO, sizeof(mvp_UBO));
	vkUnmapMemory(m_renderer->m_device, m_ModelUniformBuffer[a_imageIndex].bufferMemory);
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

	lightInfo_UBO.lightIntensity = m_lightIntensityGUILight;
		
	//Copy the data

	data = NULL;

	vkMapMemory(m_renderer->m_device, m_LightInfoUniformBuffer[a_imageIndex].bufferMemory, 0, sizeof(lightInfo_UBO), 0, &data);
	memcpy(data, &lightInfo_UBO, sizeof(lightInfo_UBO));
	vkUnmapMemory(m_renderer->m_device, m_LightInfoUniformBuffer[a_imageIndex].bufferMemory);

#pragma endregion
	
}

void Deferred::DrawGui(VkCommandBuffer a_cmdBuffer)
{
	Imgui_Impl::getInstance()->Gui_BeginFrame();

	if (m_showGUILight)
	{		
		ImGui::Begin("Light Properties", &m_showGUILight);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		
		//Lighting Mode
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFont* font_current = ImGui::GetFont();

			ImGui::RadioButton("Phong", &m_lightModelGUILight, 0); ImGui::SameLine();
			ImGui::RadioButton("BRDF", &m_lightModelGUILight, 1);
		}
		
		ImGui::SliderFloat3("Light Position", &m_lightPosGUILight.x, -400.0f, 400.0f);

		ImGui::SliderFloat3("Light Color", &m_lightColorGUILight.x, 0.0f, 1.0f);

		if(!m_lightModelGUILight)
			ImGui::SliderInt("Spec Intensity", &m_SpecularIntensityGUILight, 2, 256);
		else
			ImGui::SliderInt("Light Intensity", &m_lightIntensityGUILight, 10, 25);
		
		ImGui::End();
	}

	Imgui_Impl::getInstance()->Gui_Render(a_cmdBuffer);
}

void Deferred::UpdateCommandBuffers(uint32_t a_imageIndex)
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
	renderpassBeginInfo.framebuffer = m_renderer->m_swapChainFrameBuffer[i].frameBuffer;
	renderpassBeginInfo.renderArea.offset = { 0,0 };
	renderpassBeginInfo.renderArea.extent = m_renderer->m_swapChainDescription.swapChainExtent;


	//Clear Color//
	VkClearValue clearColor[2];
	clearColor[0] = { 0.,0.,0.,1.0 };
	clearColor[1] = { 1.0f, 0.0f };
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearColor;


	vkCmdBeginRenderPass(m_commandBuffers[i], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ModelGraphicsPipeline.pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, ModelGraphicsPipeline.pipeline);

		VkDeviceSize offset = { 0 };

		//Bind Vertex Buffer
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &m_modelBufferInfos["../../Assets/Models/Sphere/Sphere.fbx"].VertexBUffer.buffer, &offset);

		//Bind Index Buffer
		vkCmdBindIndexBuffer(m_commandBuffers[i], m_modelBufferInfos["../../Assets/Models/Sphere/Sphere.fbx"].IndexBUffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		//Call Draw Indexed for the model
		vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(m_modelBufferInfos["../../Assets/Models/Sphere/Sphere.fbx"].indexBufferCount), 1, 0, 0, 0);
		
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

void Deferred::ReCreateSwapChain()
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

void Deferred::SetUpVertexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_VertexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.vertexBufferSize;

	VkCommandPool vertBuffcommandPool;

	//create cmd pool
	m_renderer->CreateCommandPool(&vertBuffcommandPool);
	m_commandPoolList.push_back(vertBuffcommandPool);

	m_renderer->CreateBuffer(a_modelDesc.vertexbufferData.data(), bufferSize, a_VertexBUffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		vertBuffcommandPool);
}

void Deferred::SetUpIndexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_IndexBUffer)
{
	VkDeviceSize bufferSize = a_modelDesc.indexBufferSize;

	VkCommandPool indxBuffcommandPool;

	//create cmd pool
	m_renderer->CreateCommandPool(&indxBuffcommandPool);
	m_commandPoolList.push_back(indxBuffcommandPool);

	m_renderer->CreateBuffer(a_modelDesc.indexbufferData.data(), bufferSize, a_IndexBUffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		indxBuffcommandPool);

	return;
}

void Deferred::LoadAModel(std::string fileName)
{
#if Is_MT_Enabled 1
	pool.push_task([this, fileName] () {
		m_modelInfos[fileName] = m_renderer->rsrcLdr.LoadModelResource(fileName);
		}
	);
#else
	m_modelInfos[fileName] = m_renderer->rsrcLdr.LoadModelResource(fileName);
#endif
}

void Deferred::LoadModelsBufferResources()
{
	for (const auto& [key, value] : m_modelInfos)
	{
		ModelBuffersInfo bufferInfo;

		//Load Index and Vertex Buffer
		SetUpVertexBuffer(m_modelInfos[key], &bufferInfo.VertexBUffer);
		SetUpIndexBuffer(m_modelInfos[key], &bufferInfo.IndexBUffer);

		bufferInfo.indexBufferCount = static_cast<uint32_t>(m_modelInfos[key].indexbufferData.size());
		std::string name = key;
		m_modelBufferInfos[key] = bufferInfo;
	}
	return;
}

void Deferred::LoadTexture(std::string a_textureName, TextureBufferDesc * a_imageTex)
{
	// todo_rt: create a new cmd pool
	VkCommandPool imageBuffcommandPool;

	//create cmdpool
	m_renderer->CreateCommandPool(&imageBuffcommandPool);
	m_commandPoolList.push_back(imageBuffcommandPool);

	m_renderer->LoadImageTexture(a_textureName, a_imageTex, imageBuffcommandPool, m_commandBuffers.data());
}

void Deferred::CreateImageTextureView()
{
	m_renderer->CreateImageView(PBRMaterial.albedoMap.bufferImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &PBRMaterial.albedoMap.imageView);
	m_renderer->CreateImageView(PBRMaterial.metallicMap.bufferImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &PBRMaterial.metallicMap.imageView);
	m_renderer->CreateImageView(PBRMaterial.roughnessMap.bufferImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &PBRMaterial.roughnessMap.imageView);
}

void Deferred::LoadAllTextures()
{
	OPTICK_FRAME("Loading Textures");
	

#if Is_MT_Enabled 1

	pool.push_task([this]() {
		LoadTexture("../../Assets/Textures/Sphere/Albedo.png", &PBRMaterial.albedoMap);
		});

	//LoadTexture("../../Assets/Textures/Statue.jpg");
	pool.push_task([this]() {
		LoadTexture("../../Assets/Textures/Sphere/Metallic.png", &PBRMaterial.metallicMap);
		});

	pool.push_task([this]() {
		LoadTexture("../../Assets/Textures/Sphere/Roughness.png", &PBRMaterial.roughnessMap);
		});
#else
	LoadTexture("../../Assets/Textures/Sphere/Albedo.png", &PBRMaterial.albedoMap);
	LoadTexture("../../Assets/Textures/Sphere/Metallic.png", &PBRMaterial.metallicMap);
	LoadTexture("../../Assets/Textures/Sphere/Roughness.png", &PBRMaterial.roughnessMap);
	//LoadTexture("../../Assets/Textures/Kabuto/Albedo.png");
	//LoadTexture("../../Assets/Textures/green.jpg");
#endif
	
	
	
}

void Deferred::CreateTextureSampler()
{
	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.magFilter = VK_FILTER_LINEAR;
	samplerDesc.minFilter = VK_FILTER_LINEAR;
	samplerDesc.mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	m_renderer->CreateTextureSampler(samplerDesc, &PBRMaterial.albedoMap.Sampler);
	m_renderer->CreateTextureSampler(samplerDesc, &PBRMaterial.metallicMap.Sampler);
	m_renderer->CreateTextureSampler(samplerDesc, &PBRMaterial.roughnessMap.Sampler);

}

void Deferred::CreateDepthResources()
{
	depthImageInfo.textureType = TEXTURE_TYPE::eTEXTURETYPE_DEPTH;

	VkFormat depthFormat = m_renderer->FindDepthFormat();

	depthImageInfo.imageHeight = m_renderer->m_swapChainDescription.swapChainExtent.height;
	depthImageInfo.imageWidth = m_renderer->m_swapChainDescription.swapChainExtent.width;
	depthImageInfo.imageFormat = depthFormat;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	m_renderer->CreateImage(&depthImageInfo);

	m_renderer->CreateImageView(depthImageInfo.bufferImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depthImageInfo.imageView);

	//TransitionImageLayouts(depthImageInfo.BufferImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
}

void Deferred::ResourcesLoading()
{

#pragma region Model_Load
	LoadAModel("../../Assets/Models/monkey/monkey.obj");
	LoadAModel("../../Assets/Models/monkey/suzanne.obj");
	//LoadAModel("../../Assets/Models/ShaderBall/shaderBall.obj");
	LoadAModel("../../Assets/Models/Sphere/Sphere.fbx");
	//LoadAModel("../../Assets/Models/VulkanScene/vulkanscene_shadow.dae");
	LoadAModel("../../Assets/Models/venus/venus.fbx");

	pool.wait_for_tasks();

	// load each model's buffer info
	LoadModelsBufferResources();
	
#pragma endregion

#pragma region Models_Tex
	LoadAllTextures();
#pragma endregion

#if _DEBUG
	auto completitionTask = []()
	{
		std::cout << "loading model and textures Task executed" << std::endl;
	};

	pool.submit(completitionTask);
#endif

	// wait for the tasks to finish
	pool.wait_for_tasks();
}

void Deferred::setGuiVariables()
{
	m_lightPosGUILight = glm::vec3(91.30, -73.913, 160.870);
	m_lightColorGUILight = glm::vec3(1.0, 1.0, 1.0);
	m_SpecularIntensityGUILight = 4;
	m_lightModelGUILight = 1;
	m_lightIntensityGUILight = 12;
}

void Deferred::InitGui()
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

void Deferred::Init()
{
	m_renderer->Init();

	return;
}

void Deferred::PrepareApp()
{
	m_renderer->PrepareApp();

	CreateRenderPass();

	CreateDescriptorSetLayout();
	
	CreateCommandPool();
	
	CreateDepthResources();
	
	CreateFrameBuffers();
	
	CreateGraphicsPipeline();

	ResourcesLoading();

	CreateImageTextureView();
	
	CreateTextureSampler();
	
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

void Deferred::Update(float deltaTime)
{
	m_renderer->ProcessInput(m_renderer->m_window, deltaTime);
	
	cam_matrices.perspective = m_renderer->m_MainCamera->GetpersepectiveMatrix();
	
	//set view matrix
	cam_matrices.view = m_renderer->m_MainCamera->GetViewMatrix();

	return;
}

void Deferred::Draw(float deltaTime)
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
	UpdateUniformBuffer(imageIndex, cam_matrices, deltaTime);

	FrameSubmissionDesc submissionDesc = {};

	submissionDesc.imageIndex = &imageIndex;
	submissionDesc.commandBufferCount = 1;
	submissionDesc.commandBuffer = &m_commandBuffers[imageIndex];
	submissionDesc.currentFrameNumber = m_currentFrame;
	submissionDesc.result = result;

	m_renderer->SubmissionAndPresentation(submissionDesc);

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Deferred::Destroy()
{
	//depth Image
	vkDestroyImageView(m_renderer->m_device, depthImageInfo.imageView, nullptr);
	vkDestroyImage(m_renderer->m_device, depthImageInfo.bufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, depthImageInfo.bufferMemory, nullptr);

	vkDestroySampler(m_renderer->m_device, PBRMaterial.albedoMap.Sampler, nullptr);
	vkDestroyImageView(m_renderer->m_device, PBRMaterial.albedoMap.imageView, nullptr);
	vkDestroySampler(m_renderer->m_device, PBRMaterial.metallicMap.Sampler, nullptr);
	vkDestroyImageView(m_renderer->m_device, PBRMaterial.metallicMap.imageView, nullptr);
	vkDestroySampler(m_renderer->m_device, PBRMaterial.roughnessMap.Sampler, nullptr);
	vkDestroyImageView(m_renderer->m_device, PBRMaterial.roughnessMap.imageView, nullptr);

	//destroy Image
	vkDestroyImage(m_renderer->m_device, PBRMaterial.albedoMap.bufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, PBRMaterial.albedoMap.bufferMemory, nullptr);
	vkDestroyImage(m_renderer->m_device, PBRMaterial.metallicMap.bufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, PBRMaterial.metallicMap.bufferMemory, nullptr);
	vkDestroyImage(m_renderer->m_device, PBRMaterial.roughnessMap.bufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, PBRMaterial.roughnessMap.bufferMemory, nullptr);

	Imgui_Impl::getInstance()->DestroyGui(m_renderer->m_device);

	vkFreeCommandBuffers(m_renderer->m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, ModelGraphicsPipeline.pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, ModelGraphicsPipeline.pipelineLayout, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_renderer->m_swapChainDescription.swapChainImages.size(); ++i)
	{
		//Model's UBO
		vkDestroyBuffer(m_renderer->m_device, m_ModelUniformBuffer[i].buffer, nullptr);
		vkFreeMemory(m_renderer->m_device, m_ModelUniformBuffer[i].bufferMemory, nullptr);
		
		//Light's UBO
		vkDestroyBuffer(m_renderer->m_device, m_LightInfoUniformBuffer[i].buffer, nullptr);
		vkFreeMemory(m_renderer->m_device, m_LightInfoUniformBuffer[i].bufferMemory, nullptr);
	}

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	//Destroy Model's Index Buffer
	vkDestroyBuffer(m_renderer->m_device, m_ModelIndexBuffer.buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_ModelIndexBuffer.bufferMemory, nullptr);

	//Destroy Model's Vertex Buffer
	vkDestroyBuffer(m_renderer->m_device, m_ModelVertexBuffer.buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, m_ModelVertexBuffer.bufferMemory, nullptr);

	// clear all the allocated CommandPools
	for(auto cmdPool: m_commandPoolList)
		vkDestroyCommandPool(m_renderer->m_device, cmdPool, nullptr);

	// Remove all the Vulkan related intialized values
	m_renderer->Destroy();

	//Destroy the renderer
	if (m_renderer != NULL)
		delete m_renderer;
}