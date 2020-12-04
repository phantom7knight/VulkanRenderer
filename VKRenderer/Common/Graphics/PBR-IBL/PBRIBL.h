#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

struct PBRIBLPipelines
{
	GraphicsPipelineInfo			IrradianceEnvMapGraphicsPipeline;
	GraphicsPipelineInfo			PreFilterEnvMapGraphicsPipeline;
	GraphicsPipelineInfo			BRDFLUTMapGraphicsPipeline;
};

class PBRIBL
{
private:

	int32_t dimensions = 64;
	uint32_t numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

	TextureBufferDesc HDRtexture;
	TextureBufferDesc irradianceMap;
	TextureBufferDesc preFilteredCubeMap;
	TextureBufferDesc brdfLUTMap;
	TextureBufferDesc OffScreenImage;

	VkRenderPass					m_renderPass;
	FrameBufferDesc					m_OffscreenFBO;
	FrameBufferDesc					m_FBO;
	VkDescriptorSetLayout			m_descriptorSetLayout;
	VkDescriptorPool				m_DescriptorPool;
	VkDescriptorPool				m_skyboxDescriptorPool;
	std::vector<VkDescriptorSet>	m_DescriptorSets;
	PBRIBLPipelines					IBLPipelines;
	std::vector< VkCommandBuffer>	m_commandBuffers;

	// Model's buffer related
	BufferDesc VertexBuffer;
	BufferDesc IndexBuffer;
	int m_indexBufferCount = 0;

	// Instance of the Renderer created
	vkRenderer *m_renderer;

	// Generate Irradiance Map
	void ImageDataIrradianceCubeMap();
	void RenderPassIrradianceCubeMap();
	void OffScreenIrradianceCubeMapSetup(VkCommandPool a_cmdPool);
	void DescriptorSetupIrradianceCubeMap();
	void PipelineSetupIrradianceCubeMap();
	void RenderIrradianceCubeMap(VkCommandPool a_cmdPool);
	void DestroyIrradianceCubeMap(VkCommandPool a_cmdPool);
	void GenerateIrradianceCubeMap(VkCommandPool a_cmdPool);

	// Generate Pre-filtered Map
	void ImageDataPreFilteredCubeMap();
	void RenderPassPreFilteredCubeMap();
	void OffScreenPreFilteredCubeMapSetup(VkCommandPool a_cmdPool);
	void DescriptorSetupPreFilteredCubeMap();
	void PipelineSetupPreFiltererdCubeMap();
	void RenderPreFilteredCubeMap(VkCommandPool a_cmdPool);
	void DestroyPreFilteredCubeMap(VkCommandPool a_cmdPool);
	void GeneratePreFilteredCubeMap(VkCommandPool a_cmdPool);

	// Generate BRDF LUT
	void ImageDataBRDFLUTMap();
	void RenderPassBRDFLUTMap();
	void FBOBRDFLUTMapSetup();
	void DescriptorSetupBRDFLUTMap();
	void PipelineSetupBRDFLUTMap();
	void RenderBRDFLUTMap(VkCommandPool a_cmdPool);
	void DestroyBRDFLUTMap(VkCommandPool a_cmdPool);
	void GenerateBRDFLUT(VkCommandPool a_cmdPool);

	void LoadAssets(VkCommandPool a_cmdPool);

public:
	// Setup the loading of the HDR Texture
	void LoadHDRImageData(std::string a_textureName, VkCommandPool a_cmdPool, VkCommandBuffer* a_cmdBuffer);

	void Initialization(VkCommandPool a_cmdPool);
	void Update(float deltaTime);
	void Draw(float deltaTime);
	void Destroy();

	PBRIBL(vkRenderer *a_renderer);

	// Obtain the Env Maps
	inline std::vector<TextureBufferDesc> GetPBRIBLMaps()
	{
		std::vector< TextureBufferDesc> result = { irradianceMap, preFilteredCubeMap, brdfLUTMap };
		return result;
	}


};