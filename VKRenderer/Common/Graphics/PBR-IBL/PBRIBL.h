#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

class PBRIBL
{
private:

	TextureBufferDesc m_HDRtexture;
	TextureBufferDesc irradianceMap;

	// Instance of the Renderer created
	vkRenderer *m_renderer;

public:
	// Setup the loading of the HDR Texture
	void LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer* a_cmdBuffer);

	// Generate Irradiance Map
	void GenerateIrradianceMap();

	PBRIBL(vkRenderer *a_renderer);
};