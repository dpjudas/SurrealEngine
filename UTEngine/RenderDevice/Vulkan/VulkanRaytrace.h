
#pragma once

#include "VulkanObjects.h"
#include "Math/vec.h"

class ULevel;
class VulkanRenderDevice;

class LevelMesh
{
public:
	std::vector<vec3> MeshVertices;
	std::vector<uint32_t> MeshElements;
};

class VulkanRaytraceManager
{
public:
	VulkanRaytraceManager(VulkanRenderDevice* renderer);
	~VulkanRaytraceManager();

	void SetLevelMesh(LevelMesh* mesh);

	VulkanAccelerationStructure* GetAccelStruct();

private:
	void Reset();
	void CreateVulkanObjects();
	void CreateVertexAndIndexBuffers();
	void CreateBottomLevelAccelerationStructure();
	void CreateTopLevelAccelerationStructure();

	VulkanRenderDevice* renderer = nullptr;

	LevelMesh* Mesh = nullptr;

	std::unique_ptr<VulkanBuffer> vertexBuffer;
	std::unique_ptr<VulkanBuffer> indexBuffer;
	std::unique_ptr<VulkanBuffer> transferBuffer;

	std::unique_ptr<VulkanBuffer> blScratchBuffer;
	std::unique_ptr<VulkanBuffer> blAccelStructBuffer;
	std::unique_ptr<VulkanAccelerationStructure> blAccelStruct;

	std::unique_ptr<VulkanBuffer> tlTransferBuffer;
	std::unique_ptr<VulkanBuffer> tlScratchBuffer;
	std::unique_ptr<VulkanBuffer> tlInstanceBuffer;
	std::unique_ptr<VulkanBuffer> tlAccelStructBuffer;
	std::unique_ptr<VulkanAccelerationStructure> tlAccelStruct;

	ULevel* Level = nullptr;
};
