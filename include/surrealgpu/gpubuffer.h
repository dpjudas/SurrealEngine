#pragma once

#include <cstdint>
#include <string>
#include "vulkanobjects.h"

class GPUDevice;
class GPUPromise;

struct GPUUsage
{
	enum
	{
		mapRead = 0x1,
		mapWrite = 0x2,
		copySrc = 0x4,
		copyDst = 0x8,
		index = 0x10,
		vertex = 0x20,
		uniform = 0x40,
		storage = 0x80,
		indirect = 0x100,
		queryResolve = 0x200,
	};
};
typedef uint32_t GPUUsageFlags;

class GPUBufferDesc
{
public:
	std::string label;
	bool mappedAtCreation = false;
	size_t size = 0;
	GPUUsageFlags usage = 0;
};

class GPUMappedRange
{
public:
	void* data = nullptr;
	size_t offset = 0;
	size_t size = 0;
};

class GPUBuffer
{
public:
	GPUBuffer(GPUDevice* device, const GPUBufferDesc& desc);

	const GPUMappedRange& getMappedRange() const { return mappedRange; }

	GPUPromise mapAsync();
	void unmap();

	void destroy();

	GPUDevice* device = nullptr;
	GPUBufferDesc desc;

	std::unique_ptr<VulkanBuffer> buffer;

private:
	GPUMappedRange mappedRange;
};
