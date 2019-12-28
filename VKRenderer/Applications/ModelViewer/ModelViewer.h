#pragma once

#include "../../VKRenderer/vkRenderer.h"

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

};

