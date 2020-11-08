#include "Offscreen.h"


void OffScreen::Testing()
{

}

const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;

void OffScreen::GenerateOffScreenFrameBuffer()
{

	//m_OffScreenImage.textureType = TEXTURE_TYPE::eTEXTURETYPE_OFFSCREEN;

	//const int32_t dimensions = 512;
	//const uint32_t numMips = static_cast<uint32_t>(floor(log2(dimensions))) + 1;

	//m_OffScreenImage.ImageHeight = dimensions;
	//m_OffScreenImage.ImageWidth = dimensions;
	//m_OffScreenImage.mipLevels = 1;
	//m_OffScreenImage.arrayLayers = 1;
	//m_OffScreenImage.imageFormat = format;
	//m_OffScreenImage.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//m_OffScreenImage.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	//m_OffScreenImage.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	//m_renderer->CreateImage(&m_OffScreenImage);

	//m_renderer->CreateImageView(m_OffScreenImage.BufferImage, format, VK_IMAGE_ASPECT_COLOR_BIT, &m_OffScreenImage.ImageView);





	return;
}