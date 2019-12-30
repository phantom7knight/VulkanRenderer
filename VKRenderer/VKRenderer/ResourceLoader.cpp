#include "stdafx.h"
#include "ResourceLoader.h"

//if error then add "static" keyword for this function
std::vector<char, std::allocator<char>> FileOperations::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to read file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

bool ResourceLoader::checkIfCharacterExists(const std::string a_string, char a_toSearch)
{
	if (a_string.find(a_toSearch))
	{
		return true;
	}

	return false;
}

std::string FileOperations::ReplaceCharacter(const std::string a_str, char a_toReplaceWith, char a_toSearchFor)
{
	std::string res = a_str;


	for (unsigned i = 0; i < (unsigned)res.length(); ++i)
	{
		if (res[i] == a_toSearchFor)
			res[i] = a_toReplaceWith;
	}


	return res;
}


bool FileOperations::IfFileExists(const char* filename)
{
	std::ifstream ifile(filename);
	return (bool)ifile;
}


FILE* FileOperations::OpenFile(std::string a_FileName,const char* flags)
{
	FILE* fp;
	fopen_s(&fp, a_FileName.c_str(), flags);
	return fp;
}

long FileOperations::TellFile(FILE* file)
{
	long result = ftell(file);
	return result;
}

bool FileOperations::SeekFIle(FILE* fp, long offset, int origin)
{
	return fseek(fp, offset, origin) == 0;
}

unsigned FileOperations::GetFileSize(std::string a_FileName)
{
	//Open File and get Handle
	FILE* fp = OpenFile(a_FileName, "r");
	if (!fp)
		return 0;

	long curPos = TellFile(fp);
	SeekFIle(fp, 0, SEEK_END);
	size_t length = TellFile(fp);
	SeekFIle(fp, curPos, SEEK_SET);
	return (unsigned)length;
}

std::string ResourceLoader::get_current_dir()
{
	char buff[FILENAME_MAX]; //create string buffer to hold path
	_getcwd(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}

#pragma region Shader-Loading

VkShaderModule ResourceLoader::createShaderModule(ShaderDesc desc)
{
	VkShaderModuleCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = desc.shaderCode->size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(desc.shaderCode->data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule(desc.a_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shader Module");
	}

	return shaderModule;

}

bool ResourceLoader::CheckifSPIRVGenerated(std::vector<std::string> a_fileNames)
{
	//check for the shader bytecode file existence
	for (int i = 0; i < a_fileNames.size(); ++i)
	{
		std::string file_name = "Shaders/BinaryCode/" + a_fileNames[i] + ".spv";
		//First check if file exists
		if (!m_fileOpsObj.IfFileExists(file_name.c_str()))
		{
			std::cout << "ByteCode for the file " + a_fileNames[i] + " doesn't exist \n";;
			return false;
		}

		//Secondly check the size of the file
		if (m_fileOpsObj.GetFileSize(file_name) == 0)
		{
			std::cout << "Filesize for the file " + a_fileNames[i] + " was 0 \n";
			return false;
		}
	}

	return true;
}

void ResourceLoader::GenerateBatchFile(std::vector<std::string> fileNames)
{
	std::ofstream file;

	file.open("CompileShaders.Bat");
	
	const std::string vulkansdk_name =  getenv("VULKAN_SDK");
	
	std::string vulkansdk_name_replaced;

	if (checkIfCharacterExists(vulkansdk_name, '\\'))
	{
		vulkansdk_name_replaced = m_fileOpsObj.ReplaceCharacter(vulkansdk_name, '/', '\\' );
	}
	else
	{
		vulkansdk_name_replaced = vulkansdk_name;
	}

	//Generate SPIR-V files for all the mentioned Shader file names
	for (int i = 0; i < fileNames.size(); ++i)
	{
		file << vulkansdk_name_replaced + "/Bin/glslangValidator.exe -V Shaders/" + fileNames[i] + " -o Shaders/BinaryCode/" + fileNames[i] + ".spv\n";
	}
		
	file.close();

	return;
}

bool ResourceLoader::CreateDirectoryFolder(std::string a_pathName)
{
	bool success = (CreateDirectoryA(a_pathName.c_str(), NULL) == TRUE) || (GetLastError() == ERROR_ALREADY_EXISTS);

	if (success)
	{
		std::cout << "Directory has been created successfully \n";
	}
	else
	{
		std::cout << "Directory has been not been created successfully \n";
		return false;
	}

	return true;
}

void ResourceLoader::CreateFolderForSPIRV(std::string a_pathName)
{
	CreateDirectoryFolder(a_pathName);
}

void ResourceLoader::RunShaderBatchFile()
{
	std::cout << "================================================ \n";
	std::cout << " Compiling SPIR-V Shaders \n";
	std::cout << "================================================ \n";
	
	std::string shaderBatchFile = "CompileShaders.Bat";
	
	system((shaderBatchFile).c_str());

	std::cout << "================================================ \n";
	
}

void ResourceLoader::GenerateSPIRVShaders(std::vector<std::string> ShaderFileNames)
{
	if (CheckifSPIRVGenerated(ShaderFileNames))
	{
		m_bGeneratedSPIRV = true;
	}

	if (!m_bGeneratedSPIRV)
	{
		//Generate a batch file with the glslvalidator
		GenerateBatchFile(ShaderFileNames);

		//Create A directory for the Binary Code to be stored
		CreateFolderForSPIRV("Shaders/BinaryCode");

		//Run the Batch File to generate shader code
		RunShaderBatchFile();
	}
	
}

#pragma endregion




#pragma region Model-Loading



#pragma endregion