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
void PBRIBL::GenerateIrradianceMap()
{


	return;
}
#pragma endregion


#pragma region PreFilteredCubeMap

const VkFormat cubeMapFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

void PBRIBL::ImageDataPreFilteredCubeMap()
{
	preFilteredCubeMap.textureType = TEXTURE_TYPE::eTEXTURETYPE_CUBEMAP;
	
	const int32_t dimensions = 512;
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

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

void PBRIBL::OffScreemPreFilteredCubeMap(VkCommandPool a_cmdPool)
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
	m_cmdBuffer.resize(1);

	m_renderer->TransitionImageLayouts(a_cmdPool, &m_cmdBuffer[0], OffScreenImage.BufferImage,
		cubeMapFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return;
}

void PBRIBL::GeneratePreFilteredCubeMap(VkCommandPool a_cmdPool)
{
	// Image related data
	ImageDataPreFilteredCubeMap();

	// Create Render Pass
	RenderPassPreFilteredCubeMap();

	// Create Off-Screen Frame Buffer & cmd buffer
	OffScreemPreFilteredCubeMap(a_cmdPool);

	// Create Command buffer for off screen
	//CmdBufferPreFilteredCubeMap();

	// Create Pipeline for Cube Map

	// Create descriptor data
	
	return;
}
#pragma endregion

void PBRIBL::Initialization(VkCommandPool a_cmdPool)
{
	GeneratePreFilteredCubeMap(a_cmdPool);
	GenerateIrradianceMap();
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

}