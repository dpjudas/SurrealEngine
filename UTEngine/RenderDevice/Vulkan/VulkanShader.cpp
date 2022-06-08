
#include "Precomp.h"
#include "VulkanShader.h"
#include "VulkanBuilders.h"
#include "VulkanRenderDevice.h"
#include "FileResource.h"

VulkanShaderManager::VulkanShaderManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	ShaderBuilder::init();

	vertexShader = CreateVertexShader("shaders/Scene.vert");
	fragmentShader = CreateFragmentShader("shaders/Scene.frag");
	fragmentShaderAlphaTest = CreateFragmentShader("shaders/Scene.frag", "#define ALPHATEST");
}

VulkanShaderManager::~VulkanShaderManager()
{
	ShaderBuilder::deinit();
}

std::unique_ptr<VulkanShader> VulkanShaderManager::CreateVertexShader(const std::string& name, const std::string& defines)
{
	ShaderBuilder builder;
	builder.setVertexShader(LoadShaderCode(name, defines));
	return builder.create(renderer->Device);
}

std::unique_ptr<VulkanShader> VulkanShaderManager::CreateFragmentShader(const std::string& name, const std::string& defines)
{
	ShaderBuilder builder;
	builder.setFragmentShader(LoadShaderCode(name, defines));
	return builder.create(renderer->Device);
}

std::string VulkanShaderManager::LoadShaderCode(const std::string& filename, const std::string& defines)
{
	const char* shaderversion = R"(
		#version 460
		#extension GL_ARB_separate_shader_objects : enable
		#extension GL_EXT_ray_query : enable
	)";
	return shaderversion + defines + "\r\n#line 1\r\n" + FileResource::readAllText(filename);
}
