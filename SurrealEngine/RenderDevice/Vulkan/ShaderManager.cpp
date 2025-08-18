
#include "Precomp.h"
#include "ShaderManager.h"
#include "FileResource.h"
#include "VulkanRenderDevice.h"
#include <zvulkan/vulkanbuilders.h>

ShaderManager::ShaderManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	GLSLCompiler::Init();

	Scene.VertexShader = GLSLCompiler()
		.Type(ShaderType::Vertex)
		.AddSource("shaders/Scene.vert", LoadShaderCode("shaders/Scene.vert", "#extension GL_EXT_nonuniform_qualifier : enable\r\n"))
		.Compile(renderer->Device.get());

	Scene.FragmentShader = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#"))
		.Compile(renderer->Device.get());

	Scene.FragmentShaderAlphaTest = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/Scene.frag", LoadShaderCode("shaders/Scene.frag", "#extension GL_EXT_nonuniform_qualifier : enable\r\n#define ALPHATEST"))
		.Compile(renderer->Device.get());

	Postprocess.VertexShader = GLSLCompiler()
		.Type(ShaderType::Vertex)
		.AddSource("shaders/PPStep.vert", LoadShaderCode("shaders/PPStep.vert"))
		.Compile(renderer->Device.get());

	static const char* transferFunctions[2] = { nullptr, "HDR_MODE" };
	static const char* gammaModes[2] = { "GAMMA_MODE_D3D9", "GAMMA_MODE_XOPENGL" };
	static const char* colorModes[4] = { nullptr, "COLOR_CORRECT_MODE0", "COLOR_CORRECT_MODE1", "COLOR_CORRECT_MODE2" };
	for (int i = 0; i < 16; i++)
	{
		std::string defines;
		if (transferFunctions[i & 1]) defines += std::string("#define ") + transferFunctions[i & 1] + "\r\n";
		if (gammaModes[(i >> 1) & 1]) defines += std::string("#define ") + gammaModes[(i >> 1) & 1] + "\r\n";
		if (colorModes[(i >> 2) & 3]) defines += std::string("#define ") + colorModes[(i >> 2) & 3] + "\r\n";

		Postprocess.FragmentPresentShader[i] = GLSLCompiler()
			.Type(ShaderType::Fragment)
			.AddSource("shaders/Present.frag", LoadShaderCode("shaders/Present.frag", defines))
			.Compile(renderer->Device.get());
	}

	Bloom.Extract = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/BloomExtract.frag", LoadShaderCode("shaders/BloomExtract.frag"))
		.Compile(renderer->Device.get());

	Bloom.Combine = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/BloomCombine.frag", LoadShaderCode("shaders/BloomCombine.frag"))
		.Compile(renderer->Device.get());

	Bloom.BlurVertical = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/BlurVertical.frag", LoadShaderCode("shaders/Blur.frag", "#define BLUR_VERTICAL"))
		.Compile(renderer->Device.get());

	Bloom.BlurHorizontal = GLSLCompiler()
		.Type(ShaderType::Fragment)
		.AddSource("shaders/BlurHorizontal.frag", LoadShaderCode("shaders/Blur.frag", "#define BLUR_HORIZONTAL"))
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
