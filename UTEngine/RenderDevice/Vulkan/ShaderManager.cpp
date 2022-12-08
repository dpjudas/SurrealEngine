
#include "Precomp.h"
#include "ShaderManager.h"
#include "FileResource.h"
#include <zvulkan/vulkanbuilders.h>
#include "VulkanRenderDevice.h"

ShaderManager::ShaderManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	ShaderBuilder::Init();

	Scene.VertexShader = ShaderBuilder()
		.VertexShader(LoadShaderCode("shaders/Scene.vert"))
		.DebugName("vertexShader")
		.Create("vertexShader", renderer->Device.get());

	Scene.FragmentShader = ShaderBuilder()
		.FragmentShader(LoadShaderCode("shaders/Scene.frag"))
		.DebugName("fragmentShader")
		.Create("fragmentShader", renderer->Device.get());

	Scene.FragmentShaderAlphaTest = ShaderBuilder()
		.FragmentShader(LoadShaderCode("shaders/Scene.frag", "#define ALPHATEST"))
		.DebugName("fragmentShader")
		.Create("fragmentShader", renderer->Device.get());

	if (renderer->SupportsBindless)
	{
		SceneBindless.VertexShader = ShaderBuilder()
			.VertexShader(LoadShaderCode("shaders/Scene.vert", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES"))
			.DebugName("vertexShader")
			.Create("vertexShader", renderer->Device.get());

		SceneBindless.FragmentShader = ShaderBuilder()
			.FragmentShader(LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES"))
			.DebugName("fragmentShader")
			.Create("fragmentShader", renderer->Device.get());

		SceneBindless.FragmentShaderAlphaTest = ShaderBuilder()
			.FragmentShader(LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES\r\n#define ALPHATEST"))
			.DebugName("fragmentShader")
			.Create("fragmentShader", renderer->Device.get());
	}

	ppVertexShader = ShaderBuilder()
		.VertexShader(LoadShaderCode("shaders/PPStep.vert"))
		.DebugName("ppVertexShader")
		.Create("ppVertexShader", renderer->Device.get());

	ppFragmentPresentShader = ShaderBuilder()
		.FragmentShader(LoadShaderCode("shaders/Present.frag"))
		.DebugName("ppFragmentPresentShader")
		.Create("ppFragmentPresentShader", renderer->Device.get());
}

ShaderManager::~ShaderManager()
{
	ShaderBuilder::Deinit();
}

std::string ShaderManager::LoadShaderCode(const std::string& filename, const std::string& defines)
{
	const char* shaderversion = R"(
		#version 450
		#extension GL_ARB_separate_shader_objects : enable
	)";
	return shaderversion + defines + "\r\n#line 1\r\n" + FileResource::readAllText(filename);
}
