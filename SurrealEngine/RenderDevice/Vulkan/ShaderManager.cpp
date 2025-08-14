
#include "Precomp.h"
#include "ShaderManager.h"
#include "FileResource.h"
#include <zvulkan/vulkanbuilders.h>
#include "VulkanRenderDevice.h"

ShaderManager::ShaderManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	GLSLCompiler::Init();

	Scene.VertexShader = GLSLCompiler()
		.Type(ShaderType::Vertex)
		.AddSource("shaders/Scene.vert", LoadShaderCode("shaders/Scene.vert"))
		.Compile(renderer->Device.get());

	Scene.FragmentShader = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag"))
		.Compile(renderer->Device.get());

	Scene.FragmentShaderAlphaTest = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag", "#define ALPHATEST"))
		.Compile(renderer->Device.get());

	if (renderer->SupportsBindless)
	{
		SceneBindless.VertexShader = GLSLCompiler()
			.Type(ShaderType::Vertex)
			.AddSource("shaders/Scene.vert", LoadShaderCode("shaders/Scene.vert", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES"))
			.Compile(renderer->Device.get());

		SceneBindless.FragmentShader = GLSLCompiler()
			.Type(ShaderType::Fragment)
			.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES"))
			.Compile(renderer->Device.get());

		SceneBindless.FragmentShaderAlphaTest = GLSLCompiler()
			.Type(ShaderType::Fragment)
			.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define BINDLESS_TEXTURES\r\n#define ALPHATEST"))
			.Compile(renderer->Device.get());
	}

	ppVertexShader = GLSLCompiler()
		.Type(ShaderType::Vertex)
		.AddSource("shaders/PPStep.vert", LoadShaderCode("shaders/PPStep.vert"))
		.Compile(renderer->Device.get());

	ppFragmentPresentShader = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/Present.frag", LoadShaderCode("shaders/Present.frag"))
		.Compile(renderer->Device.get());
}

ShaderManager::~ShaderManager()
{
	GLSLCompiler::Deinit();
}

std::string ShaderManager::LoadShaderCode(const std::string& filename, const std::string& defines)
{
	const char* shaderversion = R"(
		#version 450
		#extension GL_ARB_separate_shader_objects : enable
	)";
	return shaderversion + defines + "\r\n#line 1\r\n" + FileResource::readAllText(filename);
}
