#pragma once

#include "../../VKRenderer/Core/Application/Application.h"
#include "../../Common/Graphics/Material.h"
#include "../../VKRenderer/Core/Threading/thread_pool.hpp"

struct ModelUBO
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};

struct LightInfoUBO
{
	glm::vec3	lightColor;
	int			specularIntensity;
	glm::vec3	lightPosition;
	int			lightModel;
	glm::vec3	camPosition;
	int			lightIntensity;

	LightInfoUBO() : lightColor(glm::vec3(1.0, 1.0, 1.0)), specularIntensity(4)
		, lightPosition(glm::vec3(0.0, 0.0, 0.0)), camPosition(glm::vec3(0.0, 0.0, 0.0)), lightModel(0), lightIntensity(1)
	{
	}
};

// Model's buffer related
struct ModelBuffersInfo
{
	BufferDesc VertexBUffer;
	BufferDesc IndexBUffer;
	int indexBufferCount = 0;
};

struct OffScreenFrameBuffer
{
	FrameBufferDesc albedoSpecMap;
	FrameBufferDesc normalMap;
	FrameBufferDesc positionMap;
	FrameBufferDesc depthMap;
	VkRenderPass renderPass;
};

class Camera;
class Deferred : public Application
{
private:

	//Helper functions for this application

	void SetUpCameraProperties(Camera* a_cam);
	
	void SetUpIndexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_IndexBUffer);
	
	void SetUpVertexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_VertexBUffer);

	void CreateAttachments();

	void CreateOffScreenFBO();

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

	void UpdateUniformBuffer(uint32_t a_imageIndex , CameraMatrices properties_Cam, float a_deltaTime);

	void UpdateCommandBuffers(uint32_t a_imageIndex);
	
	void ReCreateSwapChain();

	void CreateImageTextureView();

	void CreateTextureSampler();

	void CreateDepthResources();

	void setGuiVariables();

	void InitGui();

	void DrawGui(VkCommandBuffer a_cmdBuffer);

	void LoadAllTextures();

	void LoadAModel(std::string fileName);

	void LoadModelsBufferResources();

	void LoadTexture(std::string fileName, TextureBufferDesc* a_imageTex);

	void ResourcesLoading();

private:

	

	CameraMatrices cam_matrices;

	Material PBRMaterial;

	//Depth related variables
	TextureBufferDesc depthImageInfo;
	

	// light related
	bool		m_showGUILight;
	glm::vec3	m_lightPosGUILight;
	glm::vec3	m_lightColorGUILight;
	int			m_SpecularIntensityGUILight;
	int			m_lightModelGUILight; // current lighting model
	int			m_lightIntensityGUILight;

	// Application related variables
	vkRenderer* m_renderer;
	VkRenderPass										m_renderPass;
	VkCommandPool										m_commandPool;
	std::vector<VkCommandPool>							m_commandPoolList;
	VkDescriptorSetLayout								m_descriptorSetLayout;
	VkDescriptorPool									m_DescriptorPool;
	FrameBufferDesc										m_FBO;
	GraphicsPipelineInfo								ModelGraphicsPipeline;
	BufferDesc											m_ModelVertexBuffer;
	BufferDesc											m_ModelIndexBuffer;
	std::vector<VkCommandBuffer>						m_commandBuffers;
	std::vector<BufferDesc>								m_ModelUniformBuffer;
	std::vector<BufferDesc>								m_LightInfoUniformBuffer;
	std::vector<VkDescriptorSet>						m_DescriptorSets;
	size_t												m_currentFrame = 0;
	std::unordered_map<std::string, ModelInfo>			m_modelInfos;
	std::unordered_map<std::string, ModelBuffersInfo>	m_modelBufferInfos;

	Threading::thread_pool pool;

public:
	Deferred();
	~Deferred();

	virtual vkRenderer* getRenderer()
	{
		return m_renderer;
	}

	//Init Initialization of the properties here
	virtual void Init();

	// Inherited via vkRenderer
	virtual void PrepareApp();

	// Inherited via vkRenderer
	virtual void Update(float deltaTime);

	// Inherited via vkRenderer
	virtual void Draw(float deltaTime);

	virtual void Destroy() override;

};

