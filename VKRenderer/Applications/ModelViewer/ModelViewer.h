#pragma once

#include "../../VKRenderer/Core/Application/Application.h"
#include "../../Common/Graphics/Material.h"

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
	float		ObjRoughness;

	LightInfoUBO() : lightColor(glm::vec3(1.0, 1.0, 1.0)), specularIntensity(4)
		, lightPosition(glm::vec3(0.0, 0.0, 0.0)), camPosition(glm::vec3(0.0, 0.0, 0.0)), lightModel(0), ObjRoughness(0.3f)
	{
		
	}

};

class Camera;
class ModelViewer : public Application
{
private:

	//Helper functions for this application

	void SetUpCameraProperties(Camera* a_cam);
	
	void SetUpIndexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_IndexBUffer);
	
	void SetUpVertexBuffer(const ModelInfo a_modelDesc, BufferDesc *a_VertexBUffer);

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

	void UpdateUniformBuffer(uint32_t a_imageIndex , CameraMatrices properties_Cam);

	void UpdateCommandBuffers(uint32_t a_imageIndex);
	
	void ReCreateSwapChain();

	void CreateImageTextureView();

	void CreateTextureSampler();

	void CreateDepthResources();

	void setGuiVariables();

	void InitGui();

	void DrawGui(VkCommandBuffer a_cmdBuffer);

	void LoadAllTextures();

private:

	void LoadAModel(std::string fileName);
	void LoadTexture(std::string fileName, TextureBufferDesc* a_imageTex);

	// Model's buffer related
	BufferDesc VertexBUffer;
	BufferDesc IndexBUffer;
	int m_indexBufferCount = 0;

	CameraMatrices cam_matrices;

	// Loaded Texture related
	//TextureBufferDesc image1 = {};
	//TextureBufferDesc image2 = {};
	//TextureBufferDesc image3 = {};
	//TextureBufferDesc image4 = {};

	Material PBRMaterial;


	//Depth related variables
	TextureBufferDesc depthImageInfo;
	

	bool m_showGUILight;
	glm::vec3	m_lightPosGUILight;
	glm::vec3	m_lightColorGUILight;
	int		m_SpecularIntensityGUILight;
	int		m_lightModelGUILight;
	bool m_showPhongGUILight;
	bool m_showBRDFGUILight;
	float m_roughnessGUILight;

	// Application related variables
	vkRenderer*						m_renderer;
	VkRenderPass					m_renderPass;
	VkCommandPool					m_commandPool;
	VkDescriptorSetLayout			m_descriptorSetLayout;
	VkDescriptorPool				m_DescriptorPool;
	FrameBufferDesc					m_FBO;
	GraphicsPipelineInfo			ModelGraphicsPipeline;
	BufferDesc						m_ModelVertexBuffer;
	BufferDesc						m_ModelIndexBuffer;
	std::vector<VkCommandBuffer>	m_commandBuffers;
	std::vector<BufferDesc>			m_ModelUniformBuffer;
	std::vector<BufferDesc>			m_LightInfoUniformBuffer;
	std::vector<VkDescriptorSet>	m_DescriptorSets;
	size_t							m_currentFrame = 0;

public:
	ModelViewer();
	~ModelViewer();

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

