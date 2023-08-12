#pragma once

#include <zvulkan/vulkanobjects.h>
#include <zvulkan/vulkanbuilders.h>
#include <unordered_map>

class VulkanRenderDevice;
class CachedTexture;

struct TexDescriptorKey
{
	TexDescriptorKey(CachedTexture* tex, CachedTexture* lightmap, CachedTexture* detailtex, CachedTexture* macrotex, uint32_t sampler) : tex(tex), lightmap(lightmap), detailtex(detailtex), macrotex(macrotex), sampler(sampler) { }

	bool operator==(const TexDescriptorKey& other) const
	{
		return tex == other.tex && lightmap == other.lightmap && detailtex == other.detailtex && macrotex == other.macrotex && sampler == other.sampler;
	}

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

	CachedTexture* tex;
	CachedTexture* lightmap;
	CachedTexture* detailtex;
	CachedTexture* macrotex;
	uint32_t sampler;
};

template<> struct std::hash<TexDescriptorKey>
{
	std::size_t operator()(const TexDescriptorKey& k) const
	{
		return (((std::size_t)k.tex ^ (std::size_t)k.lightmap));
	}
};

class DescriptorSetManager
{
public:
	DescriptorSetManager(VulkanRenderDevice* renderer);
	~DescriptorSetManager();

	VulkanDescriptorSet* GetTextureDescriptorSet(uint32_t PolyFlags, CachedTexture* tex, CachedTexture* lightmap = nullptr, CachedTexture* macrotex = nullptr, CachedTexture* detailtex = nullptr, bool clamp = false);
	void ClearCache();

	int GetTextureArrayIndex(uint32_t PolyFlags, CachedTexture* tex, bool clamp = false);
	VulkanDescriptorSet* GetBindlessDescriptorSet() { return SceneBindlessDescriptorSet.get(); }
	void UpdateBindlessDescriptorSet();

	VulkanDescriptorSet* GetPresentDescriptorSet() { return PresentDescriptorSet.get(); }

	static const int MaxBindlessTextures = 16536;

	std::unique_ptr<VulkanDescriptorSetLayout> SceneBindlessDescriptorSetLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> SceneDescriptorSetLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> PresentDescriptorSetLayout;

private:
	void CreateBindlessSceneDescriptorSet();
	void CreateSceneDescriptorSetLayout();
	void CreatePresentDescriptorSetLayout();
	void CreatePresentDescriptorSet();

	VulkanRenderDevice* renderer = nullptr;

	std::unique_ptr<VulkanDescriptorPool> SceneBindlessDescriptorPool;
	std::unique_ptr<VulkanDescriptorSet> SceneBindlessDescriptorSet;
	WriteDescriptors WriteBindless;
	int NextBindlessIndex = 0;

	std::vector<std::unique_ptr<VulkanDescriptorPool>> SceneDescriptorPool;
	int SceneDescriptorPoolSetsLeft = 0;

	std::unordered_map<TexDescriptorKey, std::unique_ptr<VulkanDescriptorSet>> TextureDescriptorSets;

	std::unique_ptr<VulkanDescriptorPool> PresentDescriptorPool;
	std::unique_ptr<VulkanDescriptorSet> PresentDescriptorSet;
};
