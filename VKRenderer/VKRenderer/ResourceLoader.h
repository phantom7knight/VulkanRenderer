#pragma once

//==================================
// File Operations
//==================================

static std::vector<char> readFile(const std::string& filename);
bool IfFileExists(const char* filename);
FILE* OpenFile(std::string a_FileName, const char* flags);
long TellFile(FILE* file);
bool SeekFIle(FILE* fp, long offset, int origin);
unsigned GetFileSize(std::string a_FileName);

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
	ResourceLoader() : m_bGeneratedSPIRV(false)
	{

	}
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
	bool CheckifSPIRVGenerated(std::vector<std::string> ShaderFileNames);

	//Bool to check if the SPIR-V files have been generated
	bool m_bGeneratedSPIRV;

};