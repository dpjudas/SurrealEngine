#pragma once

#include "VulkanObjects.h"

class VulkanRenderDevice;

class VulkanShaderManager
{
public:
	VulkanShaderManager(VulkanRenderDevice* renderer);
	~VulkanShaderManager();

	std::unique_ptr<VulkanShader> CreateVertexShader(const std::string& name, const std::string& defines = {});
	std::unique_ptr<VulkanShader> CreateFragmentShader(const std::string& name, const std::string& defines = {});

	std::unique_ptr<VulkanShader> vertexShader;
	std::unique_ptr<VulkanShader> fragmentShader;
	std::unique_ptr<VulkanShader> fragmentShaderAlphaTest;

private:
	static std::string LoadShaderCode(const std::string& filename, const std::string& defines);

	VulkanRenderDevice* renderer = nullptr;
};
