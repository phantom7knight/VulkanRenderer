#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

class PBRIBL
{
private:

	TextureBufferDesc HDRtexture;
	TextureBufferDesc irradianceMap;
	TextureBufferDesc preFilteredCubeMap;
	TextureBufferDesc OffScreenImage;

	VkRenderPass					m_renderPass;
	FrameBufferDesc					m_OffscreenFBO;
	std::vector<VkCommandBuffer>	m_cmdBuffer;
	VkDescriptorSetLayout			m_skyboxdescriptorSetLayout;
	VkDescriptorPool				m_DescriptorPool;
	VkDescriptorPool				m_skyboxDescriptorPool;
	std::vector<VkDescriptorSet>	m_DescriptorSets;

	/*VkCommandPool					m_commandPool;
	VkDescriptorSetLayout			m_descriptorSetLayout;
	BufferDesc						m_ModelVertexBuffer;
	BufferDesc						m_ModelIndexBuffer;
	std::vector<BufferDesc>			m_ModelUniformBuffer;
	std::vector<BufferDesc>			m_LightInfoUniformBuffer;
	std::vector<VkDescriptorSet>	skyboxDescriptorSets;
	size_t							m_currentFrame = 0;
	Pipelines						appPipelines;*/

	// Instance of the Renderer created
	vkRenderer *m_renderer;

	// Generate Irradiance Map
	void GenerateIrradianceMap();

	// Generate Pre-filtered Map
	void ImageDataPreFilteredCubeMap();
	void RenderPassPreFilteredCubeMap();
	void OffScreenPreFilteredCubeMapSetup(VkCommandPool a_cmdPool);
	void DescriptorSetupPreFilteredCubeMap();
	void GeneratePreFilteredCubeMap(VkCommandPool a_cmdPool);

public:
	// Setup the loading of the HDR Texture
	void LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer* a_cmdBuffer);

	void Initialization(VkCommandPool a_cmdPool);
	void Update(float deltaTime);
	void Draw(float deltaTime);
	void Destroy();

	PBRIBL(vkRenderer *a_renderer);
};