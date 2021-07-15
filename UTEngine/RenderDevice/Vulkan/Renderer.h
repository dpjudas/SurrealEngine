#pragma once

#include "VulkanObjects.h"
#include "Math/vec.h"
#include "Math/mat.h"

class SceneBuffers;
class SceneLights;
class SceneRenderPass;
class SceneSamplers;
class ShadowmapRenderPass;
class Postprocess;
class VulkanPostprocess;
class VulkanTexture;
struct SceneVertex;
struct FTextureInfo;

class Renderer
{
public:
#ifdef WIN32
	Renderer(HWND windowHandle, bool vsync, int vk_device, bool vk_debug, std::function<void(const char* typestr, const std::string& msg)> printLogCallback);
	~Renderer();
#endif

	void SubmitCommands(bool present, int presentWidth, int presentHeight);
	VulkanCommandBuffer* GetTransferCommands();
	VulkanCommandBuffer* GetDrawCommands();
	void DeleteFrameObjects();

	void CopyScreenToBuffer(int w, int h, void* data, float gamma);

	static std::unique_ptr<VulkanShader> CreateVertexShader(VulkanDevice* device, const std::string& name, const std::string& defines = {});
	static std::unique_ptr<VulkanShader> CreateFragmentShader(VulkanDevice* device, const std::string& name, const std::string& defines = {});
	static std::unique_ptr<VulkanShader> CreateComputeShader(VulkanDevice* device, const std::string& name, const std::string& defines = {});
	static std::string LoadShaderCode(const std::string& filename, const std::string& defines);

	void CreateScenePipelineLayout();
	void CreateSceneDescriptorSetLayout();
	void CreateSceneVertexBuffer();
	void CreateNullTexture();

	VulkanTexture* GetTexture(FTextureInfo* texture, uint32_t polyFlags);
	VulkanDescriptorSet* GetTextureDescriptorSet(uint32_t PolyFlags, VulkanTexture* tex, VulkanTexture* lightmap = nullptr, VulkanTexture* macrotex = nullptr, VulkanTexture* detailtex = nullptr, bool clamp = false);
	void ClearTextureCache();

#ifdef WIN32
	HWND WindowHandle = 0;
#endif

	std::unique_ptr<VulkanDevice> Device;
	std::unique_ptr<VulkanSwapChain> SwapChain;
	std::unique_ptr<VulkanSemaphore> ImageAvailableSemaphore;
	std::unique_ptr<VulkanSemaphore> RenderFinishedSemaphore;
	std::unique_ptr<VulkanSemaphore> TransferSemaphore;
	std::unique_ptr<VulkanFence> RenderFinishedFence;
	std::unique_ptr<VulkanCommandPool> CommandPool;
	VulkanCommandBuffer* DrawCommands = nullptr;
	VulkanCommandBuffer* TransferCommands = nullptr;
	uint32_t PresentImageIndex = 0xffffffff;

	struct DeleteList
	{
		std::vector<std::unique_ptr<VulkanImage>> images;
		std::vector<std::unique_ptr<VulkanImageView>> imageViews;
		std::vector<std::unique_ptr<VulkanBuffer>> buffers;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> descriptors;
	};
	std::unique_ptr<DeleteList> FrameDeleteList;

	std::unique_ptr<VulkanDescriptorSetLayout> SceneDescriptorSetLayout;
	std::unique_ptr<VulkanPipelineLayout> ScenePipelineLayout;
	std::vector<VulkanDescriptorPool*> SceneDescriptorPool;
	int SceneDescriptorPoolSetsLeft = 0;

	std::unique_ptr<SceneSamplers> SceneSamplers_;
	std::unique_ptr<VulkanImage> NullTexture;
	std::unique_ptr<VulkanImageView> NullTextureView;

	std::unique_ptr<Postprocess> PostprocessModel;
	std::unique_ptr<VulkanPostprocess> Postprocess_;
	std::unique_ptr<SceneBuffers> SceneBuffers_;
	std::unique_ptr<SceneLights> SceneLights_;
	std::unique_ptr<SceneRenderPass> SceneRenderPass_;
	std::unique_ptr<ShadowmapRenderPass> ShadowmapRenderPass_;

	std::unique_ptr<VulkanBuffer> SceneVertexBuffer;
	SceneVertex* SceneVertices = nullptr;
	size_t SceneVertexPos = 0;
	static const int MaxSceneVertices = 1'000'000;

	struct TexDescriptorKey
	{
		TexDescriptorKey(VulkanTexture* tex, VulkanTexture* lightmap, VulkanTexture* detailtex, VulkanTexture* macrotex, uint32_t sampler) : tex(tex), lightmap(lightmap), detailtex(detailtex), macrotex(macrotex), sampler(sampler) { }

		bool operator<(const TexDescriptorKey& other) const
		{
			if (tex != other.tex)
				return tex < other.tex;
			else if (lightmap != other.lightmap)
				return lightmap < other.lightmap;
			else if (detailtex != other.detailtex)
				return detailtex < other.detailtex;
			else if (macrotex != other.macrotex)
				return macrotex < other.macrotex;
			else
				return sampler < other.sampler;
		}

		VulkanTexture* tex;
		VulkanTexture* lightmap;
		VulkanTexture* detailtex;
		VulkanTexture* macrotex;
		uint32_t sampler;
	};

	std::map<uint64_t, std::unique_ptr<VulkanTexture>> TextureCache;
	std::map<TexDescriptorKey, VulkanDescriptorSet*> TextureDescriptorSets;
};
