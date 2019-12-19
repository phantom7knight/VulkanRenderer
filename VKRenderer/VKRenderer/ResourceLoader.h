#pragma once

//==================================
// File Operations
//==================================

static std::vector<char> readFile(const std::string& filename);


//==================================



//==================================
// Shader Related
//==================================

struct ShaderDesc
{
	VkDevice a_device;
	const std::vector<char>& shaderCode;
};

//==================================




class ResourceLoader
{

public:
	ResourceLoader() {}
	~ResourceLoader() {}
	
	//Shader Related
	VkShaderModule createShaderModule(ShaderDesc desc);
	void GenerateBatchFile(std::vector<std::string> fileNames);
	void RunShaderBatchFile();
	void GenerateSPIRVShaders(std::vector<std::string> a_filenames);
	
	
	
	void CreateFolderForSPIRV(std::string a_pathName);


private:
	//File Related
	std::vector<char> readFile(const std::string& filename);
	bool CreateDirectoryFolder(std::string pathName);
	bool checkIfCharacterExists(const std::string a_string, char a_toSearch);
	std::string get_current_dir();
};