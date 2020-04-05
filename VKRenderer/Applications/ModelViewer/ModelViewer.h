#pragma once
#include "../../VKRenderer/vkRenderer.h"


struct ModelUBO
{
	glm::mat4 ModelMatrix;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
};

class Camera;
class ModelViewer : public vkRenderer
{
public:
	ModelViewer();
	~ModelViewer();

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

	void CreateTextureSampler();

	void CreateDepthResources();


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

	
	//Depth Related variables
	TextureBufferDesc depthImageInfo;
	VkImageView depthImageView;

};

