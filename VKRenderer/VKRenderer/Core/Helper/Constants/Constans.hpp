
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
	VkBuffer		Buffer = VK_NULL_HANDLE;

	VkDeviceMemory	BufferMemory = VK_NULL_HANDLE;

}Buffer;

struct TextureBufferDesc
{
	VkImage					BufferImage = VK_NULL_HANDLE;
	VkDeviceMemory			BufferMemory = VK_NULL_HANDLE;
	VkSampler				Sampler;
	VkImageView				ImageView;
	VkFormat				imageFormat;
	VkImageTiling			tiling;
	VkImageUsageFlags		usageFlags;
	VkMemoryPropertyFlags	propertyFlags;
	int						ImageWidth = 0;
	int						ImageHeight = 0;
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
	VkSwapchainKHR						m_swapChain;
	VkFormat							m_swapChainFormat;
	VkExtent2D							m_swapChainExtent;
	std::vector<VkImage>				m_SwapChainImages;
	std::vector<VkImageView>			m_SwapChainImageViews;
};

struct CameraMatrices
{
	glm::mat4 perspective;
	glm::mat4 orthographic;
	glm::mat4 view;

	CameraMatrices() :perspective(glm::mat4()), view(glm::mat4()), orthographic(glm::mat4())
	{

	}
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
	std::vector<char, std::allocator<char>> ShaderCode;
	VkShaderModule							shaderModule;
	VkPipelineShaderStageCreateInfo			shaderCreateInfo;
	ShaderStage								currentShader;
};

struct GraphicsPipelineInfo
{
	std::vector<std::string>						ShaderFileNames;
	VkVertexInputBindingDescription					vertexBindingDesc;
	std::vector<VkVertexInputAttributeDescription>	AttributeDescriptionsofVertex;

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

	VkDescriptorSetLayout							a_descriptorSetLayout;
	VkRenderPass                                    renderPass;
	uint32_t                                        subpass;
	
	//Final Pipeline Info
	VkPipelineLayout								a_pipelineLayout;
	VkPipeline										a_Pipeline;
	
};

// TODO: Finish this later
struct ComputePipelineInfo
{

};

struct FrameBufferDesc
{
	uint32_t							attachmentCount;
	std::vector< VkImageView >			Attachments;
	uint32_t							FBOWidth;
	uint32_t							FBOHeight;
	VkFramebuffer						FrameBuffer;
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
	VkSamplerMipmapMode MipMode;

};