#pragma once

#include "../../VKRenderer/Core/Application/Application.h"

class Camera;
class ComputeCull : public Application
{
private:

	//Helper functions for this application

	void SetUpCameraProperties(Camera* a_cam);

	void CreateRenderPass();

	void CreateDescriptorSetLayout();

	void CreateGraphicsPipeline();

	void CreateFrameBuffers();

	void CreateCommandPool();

	void CreateUniformBuffer();

	void CreateDescriptorPool();

	void CreateDescriptorSets();

	void CreateCommandBuffers();

	void CreateSemaphoresandFences();

	void UpdateUniformBuffer(uint32_t a_imageIndex, CameraMatrices properties_Cam);

	void UpdateCommandBuffers(uint32_t a_imageIndex);

	void ReCreateSwapChain();

	void CreateDepthResources();

	void setGuiVariables();

	void InitGui();

	void DrawGui(VkCommandBuffer a_cmdBuffer);


private:

	void LoadModel(ModelInfo& a_modelInfo, std::string a_fileName);

	//BufferDesc IndexBUffer;
	//BufferDesc VertexBUffer;
	//
	//int m_indexBufferCount = 0;

	CameraMatrices cam_matrices;

	//Depth Related variables
	TextureBufferDesc depthImageInfo;
	VkImageView depthImageView;

	bool		m_showGUILight;
	glm::vec3	m_lightPosGUILight;
	glm::vec3	m_lightColorGUILight;
	int			m_SpecularIntensityGUILight;
	int			m_lightModelGUILight;
	bool		m_showPhongGUILight;
	bool		m_showBRDFGUILight;
	float		m_roughnessGUILight;

	// Variables
	vkRenderer* m_renderer;
	VkRenderPass					m_renderPass;
	VkCommandPool					m_commandPool;
	VkDescriptorSetLayout			m_descriptorSetLayout;
	VkDescriptorPool				m_DescriptorPool;
	FrameBufferDesc					m_FBO;
	//GraphicsPipelineInfo			ModelGraphicsPipeline;
	//BufferDesc						m_ModelVertexBuffer;
	//BufferDesc						m_ModelIndexBuffer;
	std::vector<VkCommandBuffer>	m_commandBuffers;
	//std::vector<BufferDesc>			m_ModelUniformBuffer;
	//std::vector<BufferDesc>			m_LightInfoUniformBuffer;
	std::vector<VkDescriptorSet>	m_DescriptorSets;
	size_t							m_currentFrame = 0;

	// Models
	ModelInfo						m_modelInfo;

public:
	ComputeCull();
	~ComputeCull();

	virtual inline vkRenderer* getRenderer() { return m_renderer; }

	//Init Initialization of the properties here
	virtual void Init();

	// Inherited via vkRenderer
	virtual void PrepareApp();

	// Inherited via vkRenderer
	virtual void Update(double deltaTime);

	// Inherited via vkRenderer
	virtual void Draw(double deltaTime);

	virtual void Destroy() override;
};