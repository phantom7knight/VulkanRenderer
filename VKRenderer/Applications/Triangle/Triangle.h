#pragma once

#include "../../VKRenderer/vkRenderer.h"





class Triangle : public vkRenderer
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

public:

	

	// Inherited via vkRenderer
	virtual void PrepareApp();
	
	// Inherited via vkRenderer
	virtual void Update(float deltaTime) override;

	// Inherited via vkRenderer
	virtual void Draw(float deltaTime)  override;
	
	virtual void Destroy() override;

};

