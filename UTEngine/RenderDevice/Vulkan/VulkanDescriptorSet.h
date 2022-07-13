#pragma once

#include "VulkanObjects.h"
#include "Math/vec.h"
#include "Math/mat.h"

class VulkanRenderDevice;
class VulkanTexture;

class VulkanDescriptorSetManager
{
public:
	VulkanDescriptorSetManager(VulkanRenderDevice* renderer);
	~VulkanDescriptorSetManager();

	void UpdateLightSet();
	VulkanDescriptorSet* GetLightSet() { return LightSet.get(); }

	VulkanDescriptorSet* GetTextureSet(uint32_t PolyFlags, VulkanTexture* tex, VulkanTexture* lightmap = nullptr, VulkanTexture* macrotex = nullptr, VulkanTexture* detailtex = nullptr, bool clamp = false);
	void ClearTextureDescriptors();

	std::unique_ptr<VulkanDescriptorSetLayout> TextureSetLayout;
	std::unique_ptr<VulkanDescriptorSetLayout> LightSetLayout;

private:
	void CreateTextureSetLayout();
	void CreateLightSet();
	void CreateNullTexture();

	VulkanRenderDevice* renderer = nullptr;

	std::unique_ptr<VulkanImage> NullTexture;
	std::unique_ptr<VulkanImageView> NullTextureView;

	std::vector<std::unique_ptr<VulkanDescriptorPool>> TextureSetPool;
	int TextureSetPoolSetsLeft = 0;

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

	std::map<TexDescriptorKey, std::unique_ptr<VulkanDescriptorSet>> TextureDescriptorSets;

	std::unique_ptr<VulkanDescriptorPool> LightSetPool;
	std::unique_ptr<VulkanDescriptorSet> LightSet;
};
