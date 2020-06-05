#pragma once

#include "../../VKRenderer/Core/Application/Application.h"
#include "../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

class Triangle : public Application
{

private:
	
	void CreateRenderPass();

	void CreateGraphicsPipeline();

	void CreateFrameBuffers();

	void CreateCommandPool();

	void CreateCommandBuffers();

	void CreateSemaphoresandFences();

	void CreateUniformBuffer();

	void CreateIndexBuffer();

	void CreateVertexBuffer();
	
	void CreateDescriptorSetLayout();
	
	void CreateDescriptorPool();
	
	void CreateDesciptorSets();
	
	void UpdateUniformBuffer(uint32_t a_imageIndex, float a_deltaTime);

	void ReCreateSwapChain();



	//variables
	vkRenderer* m_renderer;

	VkRenderPass m_TriangleRenderPass;

	GraphicsPipelineInfo TrianglePipeline;

	VkRenderPass m_renderPass;

	VkDescriptorSetLayout m_descriptorSetLayout;

	FrameBufferDesc	m_FBO;

	std::vector<BufferDesc>				m_TriangleUniformBuffer;

	VkCommandPool			m_commandPool;


public:

	Triangle();
	
	~Triangle() {}

	//Init Initialization of the properties here
	virtual void Init();

	// Inherited via vkRenderer
	virtual void PrepareApp();
	
	// Inherited via vkRenderer
	virtual void Update(float deltaTime) override;

	// Inherited via vkRenderer
	virtual void Draw(float deltaTime)  override;
	
	virtual void Destroy() override;

};

