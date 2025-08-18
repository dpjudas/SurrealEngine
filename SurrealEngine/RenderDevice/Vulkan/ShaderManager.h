#pragma once

#include <zvulkan/vulkanobjects.h>
#include "Math/mat.h"

class VulkanRenderDevice;

struct SceneVertex
{
	uint32_t Flags;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexCoord2;
	vec2 TexCoord3;
	vec2 TexCoord4;
	vec4 Color;
	ivec4 TextureBinds;
};

struct ScenePushConstants
{
	mat4 objectToProjection;
	vec4 nearClip;
	uint32_t hitIndex;
	uint32_t padding1, padding2, padding3;
};

struct PresentPushConstants
{
	float Contrast;
	float Saturation;
	float Brightness;
	float HdrScale;
	vec4 GammaCorrection;
};

struct BloomPushConstants
{
	float SampleWeights[8];
};

class ShaderManager
{
public:
	ShaderManager(VulkanRenderDevice* renderer);
	~ShaderManager();

	struct SceneShaders
	{
		std::vector<uint32_t> VertexShader;
		std::vector<uint32_t> FragmentShader;
		std::vector<uint32_t> FragmentShaderAlphaTest;
	} Scene;

	struct
	{
		std::vector<uint32_t> VertexShader;
		std::vector<uint32_t> FragmentPresentShader[16];
	} Postprocess;

	struct
	{
		std::vector<uint32_t> Extract;
		std::vector<uint32_t> Combine;
		std::vector<uint32_t> BlurVertical;
		std::vector<uint32_t> BlurHorizontal;
	} Bloom;

	static std::string LoadShaderCode(const std::string& filename, const std::string& defines = {});

private:
	VulkanRenderDevice* renderer = nullptr;
};
