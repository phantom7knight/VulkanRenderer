#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//#include "../Dependencies/ASSIMP/Include/assimp/Importer.hpp"
//#include "../Dependencies/ASSIMP/Include/assimp/scene.h"
//#include "../Dependencies/ASSIMP/Include/assimp/postprocess.h"

//==================================
// File Operations
//==================================

class FileOperations
{
public:
	bool				IfFileExists(const char* filename);
	FILE*				OpenFile(std::string a_FileName, const char* flags);
	long				TellFile(FILE* file);
	bool				SeekFIle(FILE* fp, long offset, int origin);
	unsigned			GetFileSize(std::string a_FileName);
	std::vector<char,std::allocator<char>>	readFile(const std::string& filename);
	std::string ReplaceCharacter(const std::string a_str, char a_toReplaceWith, char a_toSearchFor);
};

//==================================

typedef struct ShaderDesc
{
	VkDevice a_device;
	const std::vector<char>* shaderCode;
}ShaderDesc;


struct VertexInfo
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 UV;
	glm::vec3 Tangent;
	glm::vec3 BiTangent;
	glm::vec3 Color;
};


class MeshLoader
{
public:



};


class ResourceLoader
{

public:
	ResourceLoader() : m_bGeneratedSPIRV(false)
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
	inline FileOperations getFileOperationobj()
	{
		return m_fileOpsObj;
	}
	


private:
	
	bool				CreateDirectoryFolder(std::string pathName);
	bool				checkIfCharacterExists(const std::string a_string, char a_toSearch);
	std::string			get_current_dir();
	bool				CheckifSPIRVGenerated(std::vector<std::string> ShaderFileNames);

	//Bool to check if the SPIR-V files have been generated
	bool m_bGeneratedSPIRV;
	FileOperations m_fileOpsObj;

};