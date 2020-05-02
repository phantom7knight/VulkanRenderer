#pragma once
#include "../../VKRenderer/vkRenderer.h"


struct ModelUBO
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};


struct LightInfoUBO
{
	glm::vec3 lightColor;
	int specularIntensity;
	glm::vec3 lightPosition;
	glm::vec3 camPosition;

	LightInfoUBO() : lightColor(glm::vec3(1.0, 1.0, 1.0)), specularIntensity(4)
		, lightPosition(glm::vec3(0.0, 0.0, 0.0)), camPosition(glm::vec3(0.0, 0.0, 0.0))
	{
		
	}

};

class Camera;
class ShadowMapping : public vkRenderer
{
public:
	ShadowMapping();
	~ShadowMapping();

	// Inherited via vkRenderer
	virtual void PrepareApp();

	// Inherited via vkRenderer
	virtual void Update(float deltaTime) override;

	// Inherited via vkRenderer
	virtual void Draw(float deltaTime)  override;

	virtual void Destroy() override;


	//Helper functions for this application

	void SetUpCameraProperties(Camera* a_cam);
	
	void SetUpIndexBuffer(const ModelInfo a_modelDesc	, BufferDesc *a_IndexBUffer);
	
	void SetUpVertexBuffer(const ModelInfo a_modelDesc	, BufferDesc *a_VertexBUffer);

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

	void CreateTextureSampler(VkSampler* a_textureSampler, VkSamplerAddressMode a_addressMode);

	void CreateDepthResources();

	void setGuiVariables();

	void InitGui();

	void DrawGui(VkCommandBuffer a_cmdBuffer);

	//Shadows

	void InitShadowsSetup();

	void InitShadowsFrameBuffer();

	void CreateShadowsRenderPass();

	void CreateShadowsImageViews();


private:

	void LoadAModel(std::string fileName);
	void LoadTexture(std::string fileName);
	void CreateImage(TextureBufferDesc *a_texBufferDesc);

	BufferDesc IndexBUffer;
	BufferDesc VertexBUffer;

	int m_indexBufferCount = 0;

	TextureBufferDesc image1 = {};
	VkImageView textureImageView;
	VkSampler textureSampler;
	CameraMatrices cam_matrices;
	//glm::vec3 getCamProperties

	
	//Depth Related variables
	TextureBufferDesc depthImageInfo;
	VkImageView depthImageView;

	bool m_showGUILight;
	glm::vec3	m_lightPosGUILight;
	glm::vec3	m_lightColorGUILight;
	int		m_SpecularIntensityGUILight;

	//Shadows
	VkRenderPass m_ShadowsRenderPass;
	
	TextureBufferDesc ShadowPassImageInfo;
	VkImageView ShadowPassImageView;

	VkSampler ShadowPassSampler;

	VkFramebuffer m_ShadowPassFrameBuffer;

};

