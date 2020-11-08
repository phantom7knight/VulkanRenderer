#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"
#include "../../../VKRenderer/Core/RendererVulkan/Renderer/vkRenderer.h"

class OffScreen
{
private:
	vkRenderer* m_renderer;
	FrameBufferDesc	m_OffScreenFBO;
	TextureBufferDesc m_OffScreenImage;

public:
	OffScreen(vkRenderer* a_renderer) : m_renderer(a_renderer)
	{
	}

	void GenerateOffScreenFrameBuffer();
	void Testing();

};