
#include "../../PCH/stdafx.h"

const int WIDTH		= 1600;
const int HEIGHT	= 900;

#define IMAGE_COUNT 3
const int MAX_FRAMES_IN_FLIGHT = 2;

const int TARGET_FPS = 120;
enum TEXTURE_TYPE
{
	eTEXTURETYPE_ALBEDO,
	eTEXTURETYPE_NORMAL,
	eTEXTURETYPE_SPECULAR,
	eTEXTURETYPE_METALLIC,
	eTEXTURETYPE_ROUGHNESS,
	eTEXTURETYPE_AO,
	eTEXTURETYPE_EMISSIVE,
	eTEXTURETYPE_NOISE,
	eTEXTURETYPE_DEPTH
};

typedef struct BufferDesc
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory	bufferMemory = VK_NULL_HANDLE;

}Buffer;

struct TextureBufferDesc
{
	VkImage					bufferImage = VK_NULL_HANDLE;
	VkDeviceMemory			bufferMemory = VK_NULL_HANDLE;
	VkSampler				Sampler;
	VkImageView				imageView;
	VkFormat				imageFormat;
	VkImageTiling			tiling;
	VkImageUsageFlags		usageFlags;
	VkMemoryPropertyFlags	propertyFlags;
	int						imageWidth = 0;
	int						imageHeight = 0;
	TEXTURE_TYPE			textureType;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct SwapChainDesc
{
	VkSwapchainKHR						swapChain;
	VkFormat							swapChainFormat;
	VkExtent2D							swapChainExtent;
	std::vector<VkImage>				swapChainImages;
	std::vector<VkImageView>			swapChainImageViews;
};

struct CameraMatrices
{
	glm::mat4 perspective;
	glm::mat4 orthographic;
	glm::mat4 view;

	CameraMatrices() :
		perspective(glm::mat4()),
		view(glm::mat4()),
		orthographic(glm::mat4())
	{}
};

struct CameraKeys
{
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool forward = false;
	bool backward = false;
} ;

struct CameraProperties
{
	//Camera's Position and Rotation
	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
	const glm::vec3 defPosition = glm::vec3(0.0, 0.0, -10.5);

	float rotation_speed = 0.5f;
	float translation_speed = 0.5f;

	//Field of View
	float fov;
	//Near Plane and Far Plane
	float znear, zfar;

	//to check if the camera is updated
	bool updated;
};

struct MousePositions
{
	float currentPosX = 0;
	float currentPosY = 0;
	float PrevPosX = 0;
	float PrevPosY = 0;
};

struct RenderPassInfo
{
	std::vector< VkAttachmentDescription> attachmentDescriptions;
	std::vector<VkAttachmentReference> attachmentReferences;
	std::vector< VkSubpassDependency> subpassDependecy;
	VkSubpassDescription subpassInfo = {};
};

enum ShaderStage
{
	eVERTEX_SHADER,
	ePIXEL_SHADER,
	eGEOMETRY_SHADER,
	eCOMPUTE_SHADER
};


struct ShaderPipelineModule
{
	std::vector<char, std::allocator<char>> shaderCode;
	VkShaderModule							shaderModule;
	VkPipelineShaderStageCreateInfo			shaderCreateInfo;
	ShaderStage								currentShader;
};

struct GraphicsPipelineInfo
{
	std::vector<std::string>						shaderFileNames;
	VkVertexInputBindingDescription					vertexBindingDesc;
	std::vector<VkVertexInputAttributeDescription>	attributeDescriptionsofVertex;

	VkPrimitiveTopology								pipelineTopology;

	float											viewportWidth = 1920.00;
	float											viewportHeigth = 1080.00;

	VkPolygonMode									polygonMode;
	VkCullModeFlags									cullMode;
	VkFrontFace										frontFaceCullingMode;
	bool											depthBiasEnableMode;

	bool											depthTestEnable;
	bool											depthWriteEnable;
	bool											stencilTestEnable;
	VkCompareOp										depthCompareOperation;

	VkDescriptorSetLayout							descriptorSetLayout;
	VkRenderPass									renderPass;
	uint32_t										subpass;
	
	//Final Pipeline Info
	VkPipelineLayout								pipelineLayout;
	VkPipeline										pipeline;
	
};

// TODO: Finish this later
struct ComputePipelineInfo
{
};

struct FrameBufferDesc
{
	VkImage								image;
	VkImageView							imageView;
	uint32_t							fboWidth;
	uint32_t							fboHeight;
};

struct FrameSubmissionDesc
{
	uint32_t						*imageIndex;
	size_t							currentFrameNumber;
	uint32_t						commandBufferCount;
	VkCommandBuffer					*commandBuffer;
	VkResult						result;
};

struct SamplerCreationDesc
{
	VkFilter			minFilter;
	VkFilter			magFilter;
	VkBool32			anisotropyEnable;
	VkSamplerMipmapMode mipMode;
};