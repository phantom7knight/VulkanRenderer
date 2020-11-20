#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


//==================================
// File Operations
//==================================

class FileOperations
{
public:

	FileOperations() {}
	~FileOperations() {}

	bool									IfFileExists(const char* filename);
	FILE*									OpenFile(std::string a_FileName, const char* flags);
	long									TellFile(FILE* file);
	bool									SeekFIle(FILE* fp, long offset, int origin);
	unsigned								GetFileSize(std::string a_FileName);
	bool									checkIfCharacterExists(const std::string a_string, char a_toSearch);
	bool									CheckIfStringExists(const std::string a_string, std::string a_toSearch);
	std::vector<char,std::allocator<char>>	readFile(const std::string& filename);
	std::string								ReplaceCharacter(const std::string a_str, char a_toReplaceWith, char a_toSearchFor);
	std::string								get_current_dir();
};

//==================================

typedef struct ShaderDesc
{
	VkDevice a_device;
	const std::vector<char>* shaderCode;
	ShaderStage a_shaderStage;
}ShaderDesc;


struct VertexInfo
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 UV;
	glm::vec3 Tangent;
	glm::vec3 BiTangent;

	// TODO: look into removing this static
	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDesc = {};

		bindingDesc.binding = 0;	//Since we are using only one array for the data that is Triangle_vertices we have 1 binding and order starts from 0
		bindingDesc.stride = sizeof(VertexInfo); //TODO: Check Size
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDesc;
	}


	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsofVertex()
	{
		std::vector< VkVertexInputAttributeDescription > attributeDesc;

		attributeDesc.resize(5);

		//Position
		attributeDesc[0].binding = 0;
		attributeDesc[0].location = 0;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[0].offset = offsetof(VertexInfo, Position);

		//Normals
		attributeDesc[1].binding = 0;
		attributeDesc[1].location = 1;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[1].offset = offsetof(VertexInfo, Normal);

		//TexCoords
		attributeDesc[2].binding = 0;
		attributeDesc[2].location = 2;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDesc[2].offset = offsetof(VertexInfo, UV);

		//Tangent
		attributeDesc[3].binding = 0;
		attributeDesc[3].location = 3;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[3].offset = offsetof(VertexInfo, Tangent);

		//BiTangent
		attributeDesc[4].binding = 0;
		attributeDesc[4].location = 4;		//Binding number which corresponds to layout(location = NO_) this "NO_" number
		attributeDesc[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDesc[4].offset = offsetof(VertexInfo, BiTangent);


		return attributeDesc;
	}
};

struct ModelInfo
{
	uint32_t vertexBufferSize = 0;
	std::vector<VertexInfo> vertexbufferData;
	
	uint32_t indexBufferSize = 0;
	std::vector<uint32_t> indexbufferData;
};

//Maybe Load Materials if available for the model
class MeshLoader
{
public:
	MeshLoader() {}
	~MeshLoader() {}

	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptionsofVertex();

	ModelInfo LoadModel(std::string fileName);


};

class ResourceLoader
{

public:
	ResourceLoader()
	{

	}
	~ResourceLoader() {}
	
	//--------------------------------------------------------------------------
	//Shader Related
	VkShaderModule	createShaderModule(ShaderDesc desc);
	void			GenerateBatchFile(std::vector<std::string> fileNames);
	void			RunShaderBatchFile();
	void			GenerateSPIRVShaders(std::vector<std::string> a_filenames);
	void			CreateFolderForSPIRV(std::string a_pathName);
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	//Model Loading related

	ModelInfo			LoadModelResource(std::string fileName);

	//--------------------------------------------------------------------------
	inline FileOperations getFileOperationobj()
	{
		return m_fileOpsObj;
	}

	inline MeshLoader getModelLoaderobj()
	{
		return m_MeshLoaderObj;
	}
	/*
	inline ImageLoader getImageLoaderobj()
	{
		return m_ImageLoaderObj;
	}*/
	

private:
	
	bool				CreateDirectoryFolder(std::string pathName);
	bool				CheckifSPIRVGenerated(std::vector<std::string> ShaderFileNames);

	//Bool to check if the SPIR-V files have been generated
	FileOperations m_fileOpsObj;
	MeshLoader m_MeshLoaderObj;
	//ImageLoader m_ImageLoaderObj;

};