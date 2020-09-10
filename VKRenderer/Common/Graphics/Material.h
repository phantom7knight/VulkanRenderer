#pragma once
#include "../../VKRenderer/Core/PCH/stdafx.h"

class Material
{
public:
	float m_metallic;
	float m_roughness;
	float m_specular;

	TextureBufferDesc albedoMap;
	TextureBufferDesc normalMap;
	TextureBufferDesc specularMap;
	TextureBufferDesc metallicMap;
	TextureBufferDesc roughnessMap;
	TextureBufferDesc emissiveMap;
	TextureBufferDesc aoMap;


	Material()
	{
		m_metallic = 1.0f;
		m_roughness = 1.0f;
		m_specular = 1.0f;
	}

	Material(float a_metallic, float a_roughness, float a_specular, std::string a_MaterialName)
		:m_metallic(a_metallic), m_roughness(a_roughness), m_specular(a_specular)
	{
	}
};	