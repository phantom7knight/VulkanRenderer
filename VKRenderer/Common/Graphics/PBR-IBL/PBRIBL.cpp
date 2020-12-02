#include "PBRIBL.h"

PBRIBL::PBRIBL(vkRenderer* a_renderer): m_renderer(a_renderer)
{
}

void PBRIBL::LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer *a_cmdBuffer)
{
	HDRtexture.arrayLayers = 6;
	HDRtexture.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	HDRtexture.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	// Setup Texture Data
	m_renderer->LoadImageTexture(a_textureName, &HDRtexture, a_cmdPool, a_cmdBuffer);
	
	// Setup Texture's Image View
	m_renderer->CreateImageView(HDRtexture.BufferImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &HDRtexture.ImageView, VK_IMAGE_VIEW_TYPE_CUBE);

	// Setup Texture Sampler
	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.magFilter = VK_FILTER_LINEAR;
	samplerDesc.minFilter = VK_FILTER_LINEAR;
	samplerDesc.MipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	m_renderer->CreateTextureSampler(samplerDesc, &HDRtexture.Sampler);
}

#pragma region IrradianceMap

const VkFormat irradianceCubeMapFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

// Add Push Constant data
struct PushBlock
{
	glm::mat4 mvp;
	float delta_Phi = (2.0f * float(PI)) / 180.0f;
	float delta_Theta = (0.5f * float(PI)) / 64.0f;
}pushBlock;

void PBRIBL::ImageDataIrradianceCubeMap()
{
	irradianceMap.textureType = TEXTURE_TYPE::eTEXTURETYPE_CUBEMAP;

	irradianceMap.ImageHeight = dimensions;
	irradianceMap.ImageWidth = dimensions;
	irradianceMap.mipLevels = numMips;
	irradianceMap.arrayLayers = 6;
	irradianceMap.imageFormat = irradianceCubeMapFormat;
	irradianceMap.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	irradianceMap.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	m_renderer->CreateImage(&irradianceMap);

	m_renderer->CreateImageView(irradianceMap.BufferImage, irradianceCubeMapFormat, VK_IMAGE_ASPECT_COLOR_BIT, &irradianceMap.ImageView
		, VK_IMAGE_VIEW_TYPE_CUBE, numMips, 6);

	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.MipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerDesc.minLod = 0.0f;
	samplerDesc.maxLod = static_cast<float>(numMips);
	samplerDesc.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	m_renderer->CreateTextureSampler(samplerDesc, &irradianceMap.Sampler);
}

void PBRIBL::RenderPassIrradianceCubeMap()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = irradianceCubeMapFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass should start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		//How render pass final image should translate at end of render pass


	// Each render pass can have multiple sub-passes
	// which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;

	std::vector< VkSubpassDependency> subPassDependency = {};

	subPassDependency.resize(2);

	subPassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[0].dstSubpass = 0;
	subPassDependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subPassDependency[1].srcSubpass = 0;
	subPassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	//array of attachments for this render pass
	std::vector< VkAttachmentDescription> attachments = { colorAttachment };

	std::vector<VkAttachmentReference> attachmentReferences;

	attachmentReferences.resize(attachments.size());

	attachmentReferences[0] = colorAttachmentRef;

	RenderPassInfo renderPassdesc = {};

	renderPassdesc.attachmentDescriptions = attachments;
	renderPassdesc.attachmentReferences = attachmentReferences;
	renderPassdesc.subpassDependecy = subPassDependency;
	renderPassdesc.subpassInfo = subpassInfo;

	m_renderer->CreateRenderPass(renderPassdesc, &m_renderPass);

	return;
}

void PBRIBL::OffScreenIrradianceCubeMapSetup(VkCommandPool a_cmdPool)
{
	// off screen image creation
	OffScreenImage.textureType = TEXTURE_TYPE::eTEXTURETYPE_OFFSCREEN;

	const int32_t dimensions = 64;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

	OffScreenImage.ImageHeight = dimensions;
	OffScreenImage.ImageWidth = dimensions;
	OffScreenImage.mipLevels = 1;
	OffScreenImage.arrayLayers = 1;
	OffScreenImage.imageFormat = irradianceCubeMapFormat;
	OffScreenImage.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	OffScreenImage.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	m_renderer->CreateImage(&OffScreenImage);

	// off screen image view creation
	m_renderer->CreateImageView(OffScreenImage.BufferImage, irradianceCubeMapFormat, VK_IMAGE_ASPECT_COLOR_BIT, &OffScreenImage.ImageView);

	// frame buffer creation

	std::vector< VkImageView> attachments = { OffScreenImage.ImageView };

	m_OffscreenFBO.attachmentCount = 1;
	m_OffscreenFBO.Attachments = attachments;
	m_OffscreenFBO.FBOHeight = dimensions;
	m_OffscreenFBO.FBOWidth = dimensions;

	m_renderer->CreateFrameBuffer(m_OffscreenFBO, m_renderPass, &m_OffscreenFBO.FrameBuffer);

	// cmd buffer
	std::vector<VkCommandBuffer>	cmdBuffer;
	cmdBuffer.resize(1);

	m_renderer->TransitionImageLayouts(a_cmdPool, &cmdBuffer[0], OffScreenImage.BufferImage,
		irradianceCubeMapFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return;
}

void PBRIBL::DescriptorSetupIrradianceCubeMap()
{
	// Descriptor
	VkDescriptorSetLayoutBinding IrradianceCubeMapBinding = {};

	IrradianceCubeMapBinding.binding = 0;
	IrradianceCubeMapBinding.descriptorCount = 1;
	IrradianceCubeMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	IrradianceCubeMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	IrradianceCubeMapBinding.pImmutableSamplers = nullptr;

	std::vector< VkDescriptorSetLayoutBinding > descriptorsVector = { IrradianceCubeMapBinding };

	m_renderer->CreateDescriptorSetLayout(descriptorsVector, &m_descriptorSetLayout);

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	poolSizes.resize(1);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	m_renderer->CreateDescriptorPool(poolSizes, 2, static_cast<uint32_t>(poolSizes.size()), &m_DescriptorPool);

	// Descriptor Sets
	std::vector<VkDescriptorSetLayout> layouts(1, m_descriptorSetLayout);

	m_renderer->AllocateDescriptorSets(m_DescriptorPool, layouts, m_DescriptorSets);

	VkDescriptorImageInfo environmentCubeMapImageInfo = {};

	environmentCubeMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	environmentCubeMapImageInfo.sampler = HDRtexture.Sampler;
	environmentCubeMapImageInfo.imageView = HDRtexture.ImageView;

	std::vector<VkWriteDescriptorSet> descriptorWriteInfo = {};

	descriptorWriteInfo.resize(1);

	descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteInfo[0].dstSet = m_DescriptorSets[0];
	descriptorWriteInfo[0].dstBinding = 0;
	descriptorWriteInfo[0].dstArrayElement = 0;
	descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWriteInfo[0].descriptorCount = 1;
	descriptorWriteInfo[0].pImageInfo = &environmentCubeMapImageInfo;
	descriptorWriteInfo[0].pBufferInfo = nullptr;
	descriptorWriteInfo[0].pTexelBufferView = nullptr;

	m_renderer->UpdateDescriptorSets(descriptorWriteInfo);

	return;
}

void PBRIBL::PipelineSetupIrradianceCubeMap()
{
	VkPushConstantRange pushConstantRange = {};

	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushBlock);

	std::vector<VkPushConstantRange> pushConstantRanges = { pushConstantRange };

	IBLPipelines.IrradianceEnvMapGraphicsPipeline.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.pPushConstantRanges = pushConstantRanges.data();

	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "IrradianceCube.vert";
	ShaderFileNames[1] = "IrradianceCube.frag";

	IBLPipelines.IrradianceEnvMapGraphicsPipeline.ShaderFileNames = ShaderFileNames;

	// Vertex Input
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.vertexBindingDesc = m_renderer->rsrcLdr.getModelLoaderobj().getBindingDescription();;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.AttributeDescriptionsofVertex = m_renderer->rsrcLdr.getModelLoaderobj().getAttributeDescriptionsofVertex();

	//Input Assembly
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterizer
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.polygonMode = VK_POLYGON_MODE_FILL;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.cullMode = VK_CULL_MODE_BACK_BIT;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.frontFaceCullingMode = VK_FRONT_FACE_CLOCKWISE;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.depthBiasEnableMode = VK_FALSE;

	// Depth Testing
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.depthTestEnable = VK_TRUE;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.depthWriteEnable = VK_TRUE;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.depthCompareOperation = VK_COMPARE_OP_LESS;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.stencilTestEnable = VK_FALSE;

	//Create Pipeline Layout b4 creating Graphics Pipeline
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_descriptorSetLayout = m_descriptorSetLayout;

	IBLPipelines.IrradianceEnvMapGraphicsPipeline.renderPass = m_renderPass;
	IBLPipelines.IrradianceEnvMapGraphicsPipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&IBLPipelines.IrradianceEnvMapGraphicsPipeline);

	return;
}

void PBRIBL::RenderIrradianceCubeMap(VkCommandPool a_cmdPool)
{
	VkRenderPassBeginInfo renderpassBeginInfo = {};

	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = m_renderPass;
	renderpassBeginInfo.framebuffer = m_OffscreenFBO.FrameBuffer;
	renderpassBeginInfo.renderArea.offset = { 0,0 };
	renderpassBeginInfo.renderArea.extent.width = dimensions;
	renderpassBeginInfo.renderArea.extent.height = dimensions;

	//Clear Color//
	VkClearValue clearColor[1];
	clearColor[0] = { 0.,0.,0.2,1.0 };
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearColor;

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	// Create Command Buffer
	m_commandBuffers.resize(1);
	m_renderer->AllocateCommandBuffers(m_commandBuffers, a_cmdPool);

	VkCommandBufferBeginInfo beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	//Start Recording
	if (vkBeginCommandBuffer(m_commandBuffers[0], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to begin recording Command Buffer");
	}

	// Setup ViewPort and Scissor

	VkViewport viewPort = {};

	viewPort.width = dimensions;
	viewPort.height = dimensions;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	VkRect2D scissor = {};

	scissor.offset = { 0,0 };
	scissor.extent.width = dimensions;
	scissor.extent.height = dimensions;

	vkCmdSetScissor(m_commandBuffers[0], 0, 1, &scissor);
	vkCmdSetViewport(m_commandBuffers[0], 0, 1, &viewPort);

	VkImageSubresourceRange subResourceRange = {};

	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = numMips;
	subResourceRange.layerCount = 6;

	// Change Image layout for cube map faces for transfer to FBO destination
	m_renderer->SetImageLayout(
		m_commandBuffers[0],
		irradianceMap.BufferImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subResourceRange);

	for (uint32_t m = 0; m < numMips; ++m)
	{
		for (uint32_t f = 0; f < 6; ++f)
		{
			viewPort.width = static_cast<float>(dimensions * std::pow(0.5f, m));
			viewPort.height = static_cast<float>(dimensions * std::pow(0.5f, m));
			vkCmdSetViewport(m_commandBuffers[0], 0, 1, &viewPort);

			vkCmdBeginRenderPass(m_commandBuffers[0], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				pushBlock.mvp = glm::perspective((float)(PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

				vkCmdPushConstants(m_commandBuffers[0], IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);

				vkCmdBindPipeline(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_Pipeline);
				vkCmdBindDescriptorSets(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_pipelineLayout, 
					0, 1, &m_DescriptorSets[0], 0, NULL);

				VkDeviceSize offset = { 0 };

				//Bind Vertex Buffer
				vkCmdBindVertexBuffers(m_commandBuffers[0], 0, 1, &VertexBuffer.Buffer, &offset);
				
				//Bind Index Buffer
				vkCmdBindIndexBuffer(m_commandBuffers[0], IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
				
				//Call Draw Indexed for the skybox
				vkCmdDrawIndexed(m_commandBuffers[0], static_cast<uint32_t>(m_indexBufferCount), 1, 0, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[0]);

			m_renderer->SetImageLayout(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Copy region for transfer from framebuffer to cube face
			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewPort.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewPort.height);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				irradianceMap.BufferImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);

			// Transform framebuffer color attachment back
			m_renderer->SetImageLayout(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	m_renderer->SetImageLayout(
		m_commandBuffers[0],
		irradianceMap.BufferImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subResourceRange);

	//End Recording
	m_renderer->FlushCommandBuffer(m_commandBuffers[0], a_cmdPool);

	return;
}

void PBRIBL::DestroyIrradianceCubeMap(VkCommandPool a_cmdPool)
{
	vkDestroyImageView(m_renderer->m_device, OffScreenImage.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, OffScreenImage.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, OffScreenImage.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, OffScreenImage.Sampler, nullptr);

	vkFreeCommandBuffers(m_renderer->m_device, a_cmdPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_Pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, IBLPipelines.IrradianceEnvMapGraphicsPipeline.a_pipelineLayout, nullptr);

	vkDestroyFramebuffer(m_renderer->m_device, m_OffscreenFBO.FrameBuffer, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	m_DescriptorSets.clear();
	m_commandBuffers.clear();

	return;
}

void PBRIBL::GenerateIrradianceCubeMap(VkCommandPool a_cmdPool)
{
	// Image related data
	ImageDataIrradianceCubeMap();

	// Create Render Pass
	RenderPassIrradianceCubeMap();

	// Create Off-Screen Frame Buffer & cmd buffer
	OffScreenIrradianceCubeMapSetup(a_cmdPool);

	// Create Descriptor related Info and Pipeline Layout
	DescriptorSetupIrradianceCubeMap();

	// Create Pipeline for Cube Map
	PipelineSetupIrradianceCubeMap();

	// Render the CubeMap
	RenderIrradianceCubeMap(a_cmdPool);

	// Destroy resources created for cube map
	DestroyIrradianceCubeMap(a_cmdPool);

	return;
}
#pragma endregion

#pragma region PreFilteredCubeMap

const VkFormat cubeMapFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

struct PreFilterPushBlock {
	glm::mat4 mvp;
	float roughness;
	uint32_t numSamples = 32u;
} preFilterPushBlock;

void PBRIBL::ImageDataPreFilteredCubeMap()
{
	dimensions = 512;
	numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

	preFilteredCubeMap.textureType = TEXTURE_TYPE::eTEXTURETYPE_CUBEMAP;
	
	preFilteredCubeMap.ImageHeight = dimensions;
	preFilteredCubeMap.ImageWidth = dimensions;
	preFilteredCubeMap.mipLevels = numMips;
	preFilteredCubeMap.arrayLayers = 6;
	preFilteredCubeMap.imageFormat = cubeMapFormat;
	preFilteredCubeMap.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	preFilteredCubeMap.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	m_renderer->CreateImage(&preFilteredCubeMap);

	m_renderer->CreateImageView(preFilteredCubeMap.BufferImage, cubeMapFormat, VK_IMAGE_ASPECT_COLOR_BIT, &preFilteredCubeMap.ImageView
		, VK_IMAGE_VIEW_TYPE_CUBE, numMips, 6);

	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.MipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerDesc.minLod = 0.0f;
	samplerDesc.maxLod = static_cast<float>(numMips);
	samplerDesc.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	m_renderer->CreateTextureSampler(samplerDesc, &preFilteredCubeMap.Sampler);
}

void PBRIBL::RenderPassPreFilteredCubeMap()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = cubeMapFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass should start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		//How render pass final image should translate at end of render pass


	// Each render pass can have multiple sub-passes
	// which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	
	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;

	std::vector< VkSubpassDependency> subPassDependency = {};

	subPassDependency.resize(2);

	subPassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[0].dstSubpass = 0;
	subPassDependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subPassDependency[1].srcSubpass = 0;
	subPassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	//array of attachments for this render pass
	std::vector< VkAttachmentDescription> attachments = { colorAttachment};

	std::vector<VkAttachmentReference> attachmentReferences;

	attachmentReferences.resize(attachments.size());

	attachmentReferences[0] = colorAttachmentRef;

	RenderPassInfo renderPassdesc = {};

	renderPassdesc.attachmentDescriptions = attachments;
	renderPassdesc.attachmentReferences = attachmentReferences;
	renderPassdesc.subpassDependecy = subPassDependency;
	renderPassdesc.subpassInfo = subpassInfo;

	m_renderer->CreateRenderPass(renderPassdesc, &m_renderPass);

	return;
}

void PBRIBL::OffScreenPreFilteredCubeMapSetup(VkCommandPool a_cmdPool)
{
	// off screen image creation
	OffScreenImage.textureType = TEXTURE_TYPE::eTEXTURETYPE_OFFSCREEN;

	const int32_t dimensions = 512;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

	OffScreenImage.ImageHeight = dimensions;
	OffScreenImage.ImageWidth = dimensions;
	OffScreenImage.mipLevels = 1;
	OffScreenImage.arrayLayers = 1;
	OffScreenImage.imageFormat = cubeMapFormat;
	OffScreenImage.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	OffScreenImage.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	m_renderer->CreateImage(&OffScreenImage);

	// off screen image view creation
	m_renderer->CreateImageView(OffScreenImage.BufferImage, cubeMapFormat, VK_IMAGE_ASPECT_COLOR_BIT, &OffScreenImage.ImageView);

	// frame buffer creation

	std::vector< VkImageView> attachments = { OffScreenImage.ImageView };

	m_OffscreenFBO.attachmentCount = 1;
	m_OffscreenFBO.Attachments = attachments;
	m_OffscreenFBO.FBOHeight = dimensions;
	m_OffscreenFBO.FBOWidth = dimensions;

	m_renderer->CreateFrameBuffer(m_OffscreenFBO, m_renderPass, &m_OffscreenFBO.FrameBuffer);

	// cmd buffer
	std::vector<VkCommandBuffer>	cmdBuffer;
	cmdBuffer.resize(1);

	m_renderer->TransitionImageLayouts(a_cmdPool, &cmdBuffer[0], OffScreenImage.BufferImage,
		cubeMapFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return;
}

void PBRIBL::DescriptorSetupPreFilteredCubeMap()
{
	// Descriptor
	VkDescriptorSetLayoutBinding preFilterCubeMapBinding = {};

	preFilterCubeMapBinding.binding = 0;
	preFilterCubeMapBinding.descriptorCount = 1;
	preFilterCubeMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	preFilterCubeMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	preFilterCubeMapBinding.pImmutableSamplers = nullptr;

	std::vector< VkDescriptorSetLayoutBinding > descriptorsVector = { preFilterCubeMapBinding };

	m_renderer->CreateDescriptorSetLayout(descriptorsVector, &m_descriptorSetLayout);

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	poolSizes.resize(1);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	m_renderer->CreateDescriptorPool(poolSizes, 2, static_cast<uint32_t>(poolSizes.size()), &m_DescriptorPool);

	// Descriptor Sets
	std::vector<VkDescriptorSetLayout> layouts(1, m_descriptorSetLayout);

	m_renderer->AllocateDescriptorSets(m_DescriptorPool, layouts, m_DescriptorSets);

	VkDescriptorImageInfo environmentCubeMapImageInfo = {};

	environmentCubeMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	environmentCubeMapImageInfo.sampler = HDRtexture.Sampler;
	environmentCubeMapImageInfo.imageView = HDRtexture.ImageView;

	std::vector<VkWriteDescriptorSet> descriptorWriteInfo = {};

	descriptorWriteInfo.resize(1);

	descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteInfo[0].dstSet = m_DescriptorSets[0];
	descriptorWriteInfo[0].dstBinding = 0;
	descriptorWriteInfo[0].dstArrayElement = 0;
	descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWriteInfo[0].descriptorCount = 1;
	descriptorWriteInfo[0].pImageInfo = &environmentCubeMapImageInfo;
	descriptorWriteInfo[0].pBufferInfo = nullptr;
	descriptorWriteInfo[0].pTexelBufferView = nullptr;

	m_renderer->UpdateDescriptorSets(descriptorWriteInfo);

	return;
}

void PBRIBL::PipelineSetupPreFiltererdCubeMap()
{
	VkPushConstantRange pushConstantRange = {};

	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PreFilterPushBlock);

	std::vector<VkPushConstantRange> pushConstantRanges = { pushConstantRange };

	IBLPipelines.PreFilterEnvMapGraphicsPipeline.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.pPushConstantRanges = pushConstantRanges.data();



	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "PreFilterEnvMap.vert";
	ShaderFileNames[1] = "PreFilterEnvMap.frag";

	IBLPipelines.PreFilterEnvMapGraphicsPipeline.ShaderFileNames = ShaderFileNames;

	// Vertex Input
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.vertexBindingDesc = m_renderer->rsrcLdr.getModelLoaderobj().getBindingDescription();;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.AttributeDescriptionsofVertex = m_renderer->rsrcLdr.getModelLoaderobj().getAttributeDescriptionsofVertex();

	//Input Assembly
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterizer
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.polygonMode = VK_POLYGON_MODE_FILL;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.cullMode = VK_CULL_MODE_BACK_BIT;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.frontFaceCullingMode = VK_FRONT_FACE_CLOCKWISE;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.depthBiasEnableMode = VK_FALSE;

	// Depth Testing
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.depthTestEnable = VK_TRUE;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.depthWriteEnable = VK_TRUE;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.depthCompareOperation = VK_COMPARE_OP_LESS;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.stencilTestEnable = VK_FALSE;

	//Create Pipeline Layout b4 creating Graphics Pipeline
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_descriptorSetLayout = m_descriptorSetLayout;

	IBLPipelines.PreFilterEnvMapGraphicsPipeline.renderPass = m_renderPass;
	IBLPipelines.PreFilterEnvMapGraphicsPipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&IBLPipelines.PreFilterEnvMapGraphicsPipeline);

	return;
}

void PBRIBL::RenderPreFilteredCubeMap(VkCommandPool a_cmdPool)
{
	VkRenderPassBeginInfo renderpassBeginInfo = {};

	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = m_renderPass;
	renderpassBeginInfo.framebuffer = m_OffscreenFBO.FrameBuffer;
	renderpassBeginInfo.renderArea.offset = { 0,0 };
	renderpassBeginInfo.renderArea.extent.width = dimensions;
	renderpassBeginInfo.renderArea.extent.height = dimensions;

	//Clear Color
	VkClearValue clearColor[1];
	clearColor[0] = { 0.,0.,0.2,1.0 };
	renderpassBeginInfo.clearValueCount = 2;
	renderpassBeginInfo.pClearValues = clearColor;

	std::vector<glm::mat4> matrices = {
		// POSITIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_X
		glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Y
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// POSITIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// NEGATIVE_Z
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
	};

	// Create Command Buffer
	m_commandBuffers.resize(1);
	m_renderer->AllocateCommandBuffers(m_commandBuffers, a_cmdPool);

	VkCommandBufferBeginInfo beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	//Start Recording
	if (vkBeginCommandBuffer(m_commandBuffers[0], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to begin recording Command Buffer");
	}

	// Setup ViewPort and Scissor

	VkViewport viewPort = {};

	viewPort.width = dimensions;
	viewPort.height = dimensions;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	VkRect2D scissor = {};

	scissor.offset = { 0,0 };
	scissor.extent.width = dimensions;
	scissor.extent.height = dimensions;

	vkCmdSetScissor(m_commandBuffers[0], 0, 1, &scissor);
	vkCmdSetViewport(m_commandBuffers[0], 0, 1, &viewPort);

	VkImageSubresourceRange subResourceRange = {};

	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = numMips;
	subResourceRange.layerCount = 6;

	// Change Image layout for cube map faces for transfer to FBO destination
	m_renderer->SetImageLayout(
		m_commandBuffers[0],
		preFilteredCubeMap.BufferImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subResourceRange);

	for (uint32_t m = 0; m < numMips; ++m)
	{
		preFilterPushBlock.roughness = (float)m / (float)(numMips - 1);

		for (uint32_t f = 0; f < 6; ++f)
		{
			viewPort.width = static_cast<float>(dimensions * std::pow(0.5f, m));
			viewPort.height = static_cast<float>(dimensions * std::pow(0.5f, m));
			vkCmdSetViewport(m_commandBuffers[0], 0, 1, &viewPort);

			vkCmdBeginRenderPass(m_commandBuffers[0], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			preFilterPushBlock.mvp = glm::perspective((float)(PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

			vkCmdPushConstants(m_commandBuffers[0], IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PreFilterPushBlock), &preFilterPushBlock);

			vkCmdBindPipeline(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_Pipeline);
			vkCmdBindDescriptorSets(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_pipelineLayout,
				0, 1, &m_DescriptorSets[0], 0, NULL);

			VkDeviceSize offset = { 0 };

			//Bind Vertex Buffer
			vkCmdBindVertexBuffers(m_commandBuffers[0], 0, 1, &VertexBuffer.Buffer, &offset);

			//Bind Index Buffer
			vkCmdBindIndexBuffer(m_commandBuffers[0], IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

			//Call Draw Indexed for the skybox
			vkCmdDrawIndexed(m_commandBuffers[0], static_cast<uint32_t>(m_indexBufferCount), 1, 0, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[0]);

			m_renderer->SetImageLayout(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			// Copy region for transfer from framebuffer to cube face
			VkImageCopy copyRegion = {};

			copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = { 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.dstSubresource.baseArrayLayer = f;
			copyRegion.dstSubresource.mipLevel = m;
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = { 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewPort.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewPort.height);
			copyRegion.extent.depth = 1;

			vkCmdCopyImage(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				preFilteredCubeMap.BufferImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&copyRegion);

			// Transform framebuffer color attachment back
			m_renderer->SetImageLayout(
				m_commandBuffers[0],
				OffScreenImage.BufferImage,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	m_renderer->SetImageLayout(
		m_commandBuffers[0],
		preFilteredCubeMap.BufferImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		subResourceRange);

	//End Recording
	m_renderer->FlushCommandBuffer(m_commandBuffers[0], a_cmdPool);

	return;
}

void PBRIBL::DestroyPreFilteredCubeMap(VkCommandPool a_cmdPool)
{
	vkDestroyImageView(m_renderer->m_device, OffScreenImage.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, OffScreenImage.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, OffScreenImage.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, OffScreenImage.Sampler, nullptr);

	vkFreeCommandBuffers(m_renderer->m_device, a_cmdPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_Pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, IBLPipelines.PreFilterEnvMapGraphicsPipeline.a_pipelineLayout, nullptr);

	vkDestroyFramebuffer(m_renderer->m_device, m_OffscreenFBO.FrameBuffer, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	m_DescriptorSets.clear();
	m_commandBuffers.clear();

	return;
}

void PBRIBL::GeneratePreFilteredCubeMap(VkCommandPool a_cmdPool)
{
	// Image related data
	ImageDataPreFilteredCubeMap();

	// Create Render Pass
	RenderPassPreFilteredCubeMap();

	// Create Off-Screen Frame Buffer & cmd buffer
	OffScreenPreFilteredCubeMapSetup(a_cmdPool);

	// Create Descriptor related Info and Pipeline Layout
	DescriptorSetupPreFilteredCubeMap();

	// Create Pipeline for Cube Map
	PipelineSetupPreFiltererdCubeMap();

	// Render the CubeMap
	RenderPreFilteredCubeMap(a_cmdPool);

	// Destroy resources created for cube map
	DestroyPreFilteredCubeMap(a_cmdPool);
	
	return;
}

#pragma endregion

#pragma region BRDF_LUT

void PBRIBL::ImageDataBRDFLUTMap()
{
	brdfLUTMap.textureType = TEXTURE_TYPE::eTEXTURETYPE_OFFSCREEN;

	brdfLUTMap.ImageHeight = 512;
	brdfLUTMap.ImageWidth = 512;
	brdfLUTMap.mipLevels = 1;
	brdfLUTMap.arrayLayers = 1;
	brdfLUTMap.imageFormat = VK_FORMAT_R16G16_SFLOAT;
	brdfLUTMap.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	m_renderer->CreateImage(&brdfLUTMap);

	m_renderer->CreateImageView(brdfLUTMap.BufferImage, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &brdfLUTMap.ImageView);

	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.MipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerDesc.minLod = 0.0f;
	samplerDesc.maxLod = 1.0f;
	samplerDesc.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	m_renderer->CreateTextureSampler(samplerDesc, &brdfLUTMap.Sampler);
}

void PBRIBL::RenderPassBRDFLUTMap()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = VK_FORMAT_R16G16_SFLOAT;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			//How render pass should start with
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		//How render pass final image should translate at end of render pass


	// Each render pass can have multiple sub-passes
	// which will help or can be used for the Post-Processing,...etc

	VkAttachmentReference colorAttachmentRef = {};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpassInfo = {};

	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = 1;	//layout(location = 0) out vec4 outColor this is where it will be referenced
	subpassInfo.pColorAttachments = &colorAttachmentRef;

	std::vector<VkSubpassDependency> subPassDependency = {};

	subPassDependency.resize(2);

	subPassDependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[0].dstSubpass = 0;
	subPassDependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subPassDependency[1].srcSubpass = 0;
	subPassDependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subPassDependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subPassDependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subPassDependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subPassDependency[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	//array of attachments for this render pass
	std::vector<VkAttachmentDescription> attachments = { colorAttachment };

	std::vector<VkAttachmentReference> attachmentReferences;

	attachmentReferences.resize(attachments.size());

	attachmentReferences[0] = colorAttachmentRef;

	RenderPassInfo renderPassdesc = {};

	renderPassdesc.attachmentDescriptions = attachments;
	renderPassdesc.attachmentReferences = attachmentReferences;
	renderPassdesc.subpassDependecy = subPassDependency;
	renderPassdesc.subpassInfo = subpassInfo;

	m_renderer->CreateRenderPass(renderPassdesc, &m_renderPass);

	return;
}

void PBRIBL::FBOBRDFLUTMapSetup()
{
	// frame buffer creation
	std::vector<VkImageView> attachments = { brdfLUTMap.ImageView };

	m_FBO.attachmentCount = 1;
	m_FBO.Attachments = attachments;
	m_FBO.FBOHeight = 512;
	m_FBO.FBOWidth = 512;

	m_renderer->CreateFrameBuffer(m_FBO, m_renderPass, &m_FBO.FrameBuffer);

	return;
}

void PBRIBL::DescriptorSetupBRDFLUTMap()
{
	// Descriptor
	VkDescriptorSetLayoutBinding brdfLUTMapBinding = {};

	brdfLUTMapBinding.binding = 0;
	brdfLUTMapBinding.descriptorCount = 1;
	brdfLUTMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	brdfLUTMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	brdfLUTMapBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> descriptorsVector = { brdfLUTMapBinding };

	m_renderer->CreateDescriptorSetLayout(descriptorsVector, &m_descriptorSetLayout);

	// Descriptor Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	poolSizes.resize(1);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1;

	m_renderer->CreateDescriptorPool(poolSizes, 2, static_cast<uint32_t>(poolSizes.size()), &m_DescriptorPool);

	// Descriptor Sets
	std::vector<VkDescriptorSetLayout> layouts(1, m_descriptorSetLayout);

	m_renderer->AllocateDescriptorSets(m_DescriptorPool, layouts, m_DescriptorSets);

	/*VkDescriptorImageInfo environmentCubeMapImageInfo = {};

	environmentCubeMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	environmentCubeMapImageInfo.sampler = HDRtexture.Sampler;
	environmentCubeMapImageInfo.imageView = HDRtexture.ImageView;

	std::vector<VkWriteDescriptorSet> descriptorWriteInfo = {};

	descriptorWriteInfo.resize(1);

	descriptorWriteInfo[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriteInfo[0].dstSet = m_DescriptorSets[0];
	descriptorWriteInfo[0].dstBinding = 0;
	descriptorWriteInfo[0].dstArrayElement = 0;
	descriptorWriteInfo[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWriteInfo[0].descriptorCount = 1;
	descriptorWriteInfo[0].pImageInfo = &environmentCubeMapImageInfo;
	descriptorWriteInfo[0].pBufferInfo = nullptr;
	descriptorWriteInfo[0].pTexelBufferView = nullptr;

	m_renderer->UpdateDescriptorSets(descriptorWriteInfo);*/

	return;
}

void PBRIBL::PipelineSetupBRDFLUTMap()
{
	std::vector<std::string> ShaderFileNames;

	ShaderFileNames.resize(2);

	ShaderFileNames[0] = "BRDFLUT.vert";
	ShaderFileNames[1] = "BRDFLUT.frag";

	IBLPipelines.BRDFLUTMapGraphicsPipeline.ShaderFileNames = ShaderFileNames;

	// Vertex Input
	IBLPipelines.BRDFLUTMapGraphicsPipeline.vertexBindingDescInit = false;

	//Input Assembly
	IBLPipelines.BRDFLUTMapGraphicsPipeline.pipelineTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterizer
	IBLPipelines.BRDFLUTMapGraphicsPipeline.polygonMode = VK_POLYGON_MODE_FILL;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.cullMode = VK_CULL_MODE_BACK_BIT;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.frontFaceCullingMode = VK_FRONT_FACE_CLOCKWISE;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.depthBiasEnableMode = VK_FALSE;

	// Depth Testing
	IBLPipelines.BRDFLUTMapGraphicsPipeline.depthTestEnable = VK_TRUE;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.depthWriteEnable = VK_TRUE;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.depthCompareOperation = VK_COMPARE_OP_LESS;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.stencilTestEnable = VK_FALSE;

	// Dynamic States
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	IBLPipelines.BRDFLUTMapGraphicsPipeline.pDynamicStates = dynamicStateEnables.data();
	IBLPipelines.BRDFLUTMapGraphicsPipeline.dynamicStatesCount = dynamicStateEnables.size();

	//Create Pipeline Layout b4 creating Graphics Pipeline
	IBLPipelines.BRDFLUTMapGraphicsPipeline.a_descriptorSetLayout = m_descriptorSetLayout;

	IBLPipelines.BRDFLUTMapGraphicsPipeline.renderPass = m_renderPass;
	IBLPipelines.BRDFLUTMapGraphicsPipeline.subpass = 0;

	m_renderer->CreateGraphicsPipeline(&IBLPipelines.BRDFLUTMapGraphicsPipeline);

	return;
}

void PBRIBL::RenderBRDFLUTMap(VkCommandPool a_cmdPool)
{
	VkRenderPassBeginInfo renderpassBeginInfo = {};

	renderpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpassBeginInfo.renderPass = m_renderPass;
	renderpassBeginInfo.framebuffer = m_FBO.FrameBuffer;
	renderpassBeginInfo.renderArea.offset = { 0,0 };
	renderpassBeginInfo.renderArea.extent.width = 512;
	renderpassBeginInfo.renderArea.extent.height = 512;

	//Clear Color
	VkClearValue clearColor[1];
	clearColor[0] = { 0.,0.,0.,1.0 };
	renderpassBeginInfo.clearValueCount = 1;
	renderpassBeginInfo.pClearValues = clearColor;

	// Create Command Buffer
	m_commandBuffers.resize(1);
	m_renderer->AllocateCommandBuffers(m_commandBuffers, a_cmdPool);

	VkCommandBufferBeginInfo beginInfo = {};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	//Start Recording
	if (vkBeginCommandBuffer(m_commandBuffers[0], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to begin recording Command Buffer");
	}

	vkCmdBeginRenderPass(m_commandBuffers[0], &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewPort = {};

	viewPort.width = 512;
	viewPort.height = 512;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth = 1.0f;

	VkRect2D scissor = {};

	scissor.offset = { 0,0 };
	scissor.extent.width = 512;
	scissor.extent.height = 512;

	vkCmdSetScissor(m_commandBuffers[0], 0, 1, &scissor);
	vkCmdSetViewport(m_commandBuffers[0], 0, 1, &viewPort);

	vkCmdBindPipeline(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, IBLPipelines.BRDFLUTMapGraphicsPipeline.a_Pipeline);

	vkCmdDraw(m_commandBuffers[0], 3, 1, 0, 0);

	vkCmdEndRenderPass(m_commandBuffers[0]);

	m_renderer->FlushCommandBuffer(m_commandBuffers[0], a_cmdPool, true);
}

void PBRIBL::DestroyBRDFLUTMap(VkCommandPool a_cmdPool)
{
	vkFreeCommandBuffers(m_renderer->m_device, a_cmdPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

	vkDestroyPipeline(m_renderer->m_device, IBLPipelines.BRDFLUTMapGraphicsPipeline.a_Pipeline, nullptr);

	vkDestroyPipelineLayout(m_renderer->m_device, IBLPipelines.BRDFLUTMapGraphicsPipeline.a_pipelineLayout, nullptr);

	vkDestroyFramebuffer(m_renderer->m_device, m_FBO.FrameBuffer, nullptr);

	vkDestroyRenderPass(m_renderer->m_device, m_renderPass, nullptr);

	vkDestroyDescriptorPool(m_renderer->m_device, m_DescriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(m_renderer->m_device, m_descriptorSetLayout, nullptr);

	m_DescriptorSets.clear();
	m_commandBuffers.clear();

	return;
}

void PBRIBL::GenerateBRDFLUT(VkCommandPool a_cmdPool)
{
	ImageDataBRDFLUTMap();
	RenderPassBRDFLUTMap();
	FBOBRDFLUTMapSetup();
	DescriptorSetupBRDFLUTMap();
	PipelineSetupBRDFLUTMap();
	RenderBRDFLUTMap(a_cmdPool);
	DestroyBRDFLUTMap(a_cmdPool);
}
#pragma endregion

void PBRIBL::LoadAssets(VkCommandPool a_cmdPool)
{
	ModelInfo modelinfor = m_renderer->rsrcLdr.LoadModelResource("../../Assets/Models/cube/cube.obj");

	m_renderer->SetUpVertexBuffer(modelinfor, &VertexBuffer, a_cmdPool);
	m_renderer->SetUpIndexBuffer(modelinfor, &IndexBuffer, a_cmdPool);

	m_indexBufferCount = static_cast<uint32_t>(modelinfor.indexbufferData.size());

}

void PBRIBL::Initialization(VkCommandPool a_cmdPool)
{
	// Load Skybox Cube
	LoadAssets(a_cmdPool);

	GenerateIrradianceCubeMap(a_cmdPool);
	GeneratePreFilteredCubeMap(a_cmdPool);
	GenerateBRDFLUT(a_cmdPool);
}

void PBRIBL::Update(float deltaTime)
{
	return;
}

void PBRIBL::Draw(float deltaTime)
{
	return;
}

void PBRIBL::Destroy()
{
	// Irradiance Map cleanup
	vkDestroyImageView(m_renderer->m_device, irradianceMap.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, irradianceMap.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, irradianceMap.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, irradianceMap.Sampler, nullptr);

	// PreFiltered Cube Map cleanup
	vkDestroyImageView(m_renderer->m_device, preFilteredCubeMap.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, preFilteredCubeMap.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, preFilteredCubeMap.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, preFilteredCubeMap.Sampler, nullptr);
	
	// BRDFLUT Map cleanup
	vkDestroyImageView(m_renderer->m_device, brdfLUTMap.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, brdfLUTMap.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, brdfLUTMap.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, brdfLUTMap.Sampler, nullptr);

	// HDRtexture cleanup
	vkDestroyImageView(m_renderer->m_device, HDRtexture.ImageView, nullptr);
	vkDestroyImage(m_renderer->m_device, HDRtexture.BufferImage, nullptr);
	vkFreeMemory(m_renderer->m_device, HDRtexture.BufferMemory, nullptr);
	vkDestroySampler(m_renderer->m_device, HDRtexture.Sampler, nullptr);

	//Destroy skybox's Index Buffer
	vkDestroyBuffer(m_renderer->m_device, IndexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, IndexBuffer.BufferMemory, nullptr);

	//Destroy skybox's Vertex Buffer
	vkDestroyBuffer(m_renderer->m_device, VertexBuffer.Buffer, nullptr);
	vkFreeMemory(m_renderer->m_device, VertexBuffer.BufferMemory, nullptr);
}