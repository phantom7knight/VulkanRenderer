#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

class PBRIBL
{
private:

	TextureBufferDesc HDRtexture;
	TextureBufferDesc irradianceMap;
	TextureBufferDesc preFilteredCubeMap;

	VkRenderPass					m_renderPass;
	/*VkCommandPool					m_commandPool;
	VkDescriptorSetLayout			m_descriptorSetLayout;
	VkDescriptorSetLayout			skyboxdescriptorSetLayout;
	VkDescriptorPool				m_DescriptorPool;
	VkDescriptorPool				skyboxDescriptorPool;
	FrameBufferDesc					m_FBO;
	BufferDesc						m_ModelVertexBuffer;
	BufferDesc						m_ModelIndexBuffer;
	std::vector<VkCommandBuffer>	m_commandBuffers;
	std::vector<BufferDesc>			m_ModelUniformBuffer;
	std::vector<BufferDesc>			m_LightInfoUniformBuffer;
	std::vector<VkDescriptorSet>	m_DescriptorSets;
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
	void FrameBufferPreFilteredCubeMap();
	void GeneratePreFilteredCubeMap();

public:
	// Setup the loading of the HDR Texture
	void LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer* a_cmdBuffer);

	void Initialization();
	void Update(float deltaTime);
	void Draw(float deltaTime);
	void Destroy();

	PBRIBL(vkRenderer *a_renderer);
};