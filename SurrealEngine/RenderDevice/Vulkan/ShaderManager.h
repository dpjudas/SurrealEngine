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
};

struct PresentPushConstants
{
	float InvGamma;
	float padding1;
	float padding2;
	float padding3;
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
	} Scene, SceneBindless;

	std::vector<uint32_t> ppVertexShader;
	std::vector<uint32_t> ppFragmentPresentShader;

	static std::string LoadShaderCode(const std::string& filename, const std::string& defines = {});

private:
	VulkanRenderDevice* renderer = nullptr;
};
