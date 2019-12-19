#include "stdafx.h"
#include "ResourceLoader.h"

//if error then add "static" keyword for this function
std::vector<char> ResourceLoader::readFile(const std::string& filename)
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

VkShaderModule ResourceLoader::createShaderModule(ShaderDesc desc)
{
	VkShaderModuleCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = desc.shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(desc.shaderCode.data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule(desc.a_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Shader Module");
	}

	return shaderModule;

}

bool ResourceLoader::checkIfCharacterExists(const std::string a_string, char a_toSearch)
{
	if (a_string.find(a_toSearch))
	{
		return true;
	}

	return false;
}

std::string ReplaceCharacter(const std::string a_str, char a_toReplace, char a_toSearch)
{
	std::string res = a_str;


	for (unsigned i = 0; i < (unsigned)res.length(); ++i)
	{
		if (res[i] == a_toSearch)
			res[i] = a_toReplace;
	}


	return res;
}


bool IfFileExists(const char* filename)
{
	std::ifstream ifile(filename);
	return (bool)ifile;
}

void ResourceLoader::GenerateBatchFile(std::vector<std::string> fileNames)
{
	std::ofstream file{ "CompileShaders.Bat" };
	
	const std::string vulkansdk_name =  getenv("VULKAN_SDK");
	
	std::string vulkansdk_name_replaced;

	if (checkIfCharacterExists(vulkansdk_name, '\\'))
	{
		vulkansdk_name_replaced = ReplaceCharacter(vulkansdk_name, '/', '\\' );
	}
	else
	{
		vulkansdk_name_replaced = vulkansdk_name;
	}

	//Generate SPIR-V files for all the mentioned Shader file names
	for (int i = 0; i < fileNames.size(); ++i)
	{
		file << vulkansdk_name_replaced + "/Bin/glslangValidator.exe -V " + "Shaders/" + fileNames[i] + " -o Shaders/BinaryCode/" + fileNames[i] + ".spv\n";
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

std::string ResourceLoader::get_current_dir() 
{
	char buff[FILENAME_MAX]; //create string buffer to hold path
	_getcwd(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
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
	//Generate a batch file with the glslvalidator
	GenerateBatchFile(ShaderFileNames);

	//Create A directory for the Binary Code to be stored
	CreateFolderForSPIRV("Shaders/BinaryCode");

	//Run the Batch File to generate shader code
	RunShaderBatchFile();

}
