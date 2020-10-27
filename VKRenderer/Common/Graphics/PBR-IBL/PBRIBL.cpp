#include "PBRIBL.h"

PBRIBL::PBRIBL(vkRenderer* a_renderer): m_renderer(a_renderer)
{
}

void PBRIBL::LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer *a_cmdBuffer)
{
	m_HDRtexture.arrayLayers = 6;
	m_HDRtexture.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	m_HDRtexture.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	// Setup Texture Data
	m_renderer->LoadImageTexture(a_textureName, &m_HDRtexture, a_cmdPool, a_cmdBuffer);
	
	// Setup Texture's Image View
	m_renderer->CreateImageView(m_HDRtexture.BufferImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, &m_HDRtexture.ImageView, VK_IMAGE_VIEW_TYPE_CUBE);

	// Setup Texture Sampler
	SamplerCreationDesc samplerDesc = {};

	samplerDesc.anisotropyEnable = VK_TRUE;
	samplerDesc.magFilter = VK_FILTER_LINEAR;
	samplerDesc.minFilter = VK_FILTER_LINEAR;
	samplerDesc.MipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	m_renderer->CreateTextureSampler(samplerDesc, &m_HDRtexture.Sampler);


}


void PBRIBL::GenerateIrradianceMap()
{


	return;
}