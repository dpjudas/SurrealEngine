
#include "VulkanRaytrace.h"
#include "VulkanBuilders.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderDevice.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"

VulkanRaytraceManager::VulkanRaytraceManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
}

VulkanRaytraceManager::~VulkanRaytraceManager()
{
}

void VulkanRaytraceManager::SetLevelMesh(LevelMesh* mesh)
{
	if (mesh != Mesh)
	{
		Reset();
		Mesh = mesh;
		if (Mesh)
		{
			if (renderer->Device->supportsDeviceExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME))
			{
				CreateVulkanObjects();
			}
		}
	}
}

static void AddLevelNode(LevelMesh& mesh, const BspNode& node)
{
	auto model = engine->Level->Model;

	const BspNode* polynode = &node;
	while (true)
	{
		if (polynode->NumVertices > 0 && polynode->Surf >= 0)
		{
			BspSurface& surface = model->Surfaces[polynode->Surf];
			uint32_t PolyFlags = surface.PolyFlags;
			if (surface.Material)
			{
				// To do: implement packed booleans in the VM so that this can be done as a single uint32_t
				UTexture* t = surface.Material;
				PolyFlags |= ((uint32_t)t->bInvisible()) << 0;
				PolyFlags |= ((uint32_t)t->bMasked()) << 1;
				PolyFlags |= ((uint32_t)t->bTransparent()) << 2;
				PolyFlags |= ((uint32_t)t->bNotSolid()) << 3;
				PolyFlags |= ((uint32_t)t->bEnvironment()) << 4;
				PolyFlags |= ((uint32_t)t->bSemisolid()) << 5;
				PolyFlags |= ((uint32_t)t->bModulate()) << 6;
				PolyFlags |= ((uint32_t)t->bFakeBackdrop()) << 7;
				PolyFlags |= ((uint32_t)t->bTwoSided()) << 8;
				PolyFlags |= ((uint32_t)t->bAutoUPan()) << 9;
				PolyFlags |= ((uint32_t)t->bAutoVPan()) << 10;
				PolyFlags |= ((uint32_t)t->bNoSmooth()) << 11;
				PolyFlags |= ((uint32_t)t->bBigWavy()) << 12;
				PolyFlags |= ((uint32_t)t->bSmallWavy()) << 13;
				PolyFlags |= ((uint32_t)t->bWaterWavy()) << 14;
				PolyFlags |= ((uint32_t)t->bLowShadowDetail()) << 15;
				PolyFlags |= ((uint32_t)t->bNoMerge()) << 16;
				PolyFlags |= ((uint32_t)t->bCloudWavy()) << 17;
				PolyFlags |= ((uint32_t)t->bDirtyShadows()) << 18;
				PolyFlags |= ((uint32_t)t->bHighLedge()) << 19;
				PolyFlags |= ((uint32_t)t->bSpecialLit()) << 20;
				PolyFlags |= ((uint32_t)t->bGouraud()) << 21;
				PolyFlags |= ((uint32_t)t->bUnlit()) << 22;
				PolyFlags |= ((uint32_t)t->bHighShadowDetail()) << 23;
				PolyFlags |= ((uint32_t)t->bPortal()) << 24;
				PolyFlags |= ((uint32_t)t->bMirrored()) << 25;
				PolyFlags |= ((uint32_t)t->bX2()) << 26;
				PolyFlags |= ((uint32_t)t->bX3()) << 27;
				PolyFlags |= ((uint32_t)t->bX4()) << 28;
				PolyFlags |= ((uint32_t)t->bX5()) << 29;
				PolyFlags |= ((uint32_t)t->bX6()) << 30;
				PolyFlags |= ((uint32_t)t->bX7()) << 31;
			}

			if ((PolyFlags & (PF_Invisible | PF_FakeBackdrop)) == 0)
			{
				size_t startVert = mesh.MeshVertices.size();

				BspVert* v = &model->Vertices[polynode->VertPool];
				for (int j = 0; j < polynode->NumVertices; j++)
				{
					mesh.MeshVertices.push_back(model->Points[v[j].Vertex]);
				}

				for (int j = 2; j < polynode->NumVertices; j++)
				{
					mesh.MeshElements.push_back((int)(startVert));
					mesh.MeshElements.push_back((int)(startVert + j - 1));
					mesh.MeshElements.push_back((int)(startVert + j));
				}
			}
		}

		if (polynode->Plane < 0) break;
		polynode = &model->Nodes[polynode->Plane];
	}

	if (node.Front >= 0)
		AddLevelNode(mesh, model->Nodes[node.Front]);

	if (node.Back >= 0)
		AddLevelNode(mesh, model->Nodes[node.Back]);
}

VulkanAccelerationStructure* VulkanRaytraceManager::GetAccelStruct()
{
	if (engine->Level != Level)
	{
		Reset();
		Level = engine->Level;

		LevelMesh dummy;
		AddLevelNode(dummy, Level->Model->Nodes[0]);
		Mesh = &dummy;
		CreateVulkanObjects();
		Mesh = nullptr;
	}

	if (tlAccelStruct)
		return tlAccelStruct.get();

	// We need a dummy accel struct to keep vulkan happy:
	LevelMesh dummy;

	dummy.MeshVertices.push_back({ -1.0f, -1.0f, -1.0f });
	dummy.MeshVertices.push_back({  1.0f, -1.0f, -1.0f });
	dummy.MeshVertices.push_back({  1.0f,  1.0f, -1.0f });

	dummy.MeshVertices.push_back({ -1.0f, -1.0f, -1.0f });
	dummy.MeshVertices.push_back({ -1.0f,  1.0f, -1.0f });
	dummy.MeshVertices.push_back({  1.0f,  1.0f, -1.0f });

	dummy.MeshVertices.push_back({ -1.0f, -1.0f,  1.0f });
	dummy.MeshVertices.push_back({  1.0f, -1.0f,  1.0f });
	dummy.MeshVertices.push_back({  1.0f,  1.0f,  1.0f });

	dummy.MeshVertices.push_back({ -1.0f, -1.0f,  1.0f });
	dummy.MeshVertices.push_back({ -1.0f,  1.0f,  1.0f });
	dummy.MeshVertices.push_back({  1.0f,  1.0f,  1.0f });

	for (int i = 0; i < 3 * 4; i++)
		dummy.MeshElements.push_back(i);

	Mesh = &dummy;
	CreateVulkanObjects();
	Mesh = nullptr;
	return tlAccelStruct.get();
}

void VulkanRaytraceManager::Reset()
{
	auto fb = renderer;
	if (fb)
	{
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(vertexBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(indexBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(transferBuffer));

		fb->Commands->FrameDeleteList->buffers.push_back(std::move(blScratchBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(blAccelStructBuffer));
		fb->Commands->FrameDeleteList->accelStructs.push_back(std::move(blAccelStruct));

		fb->Commands->FrameDeleteList->buffers.push_back(std::move(tlTransferBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(tlScratchBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(tlInstanceBuffer));
		fb->Commands->FrameDeleteList->buffers.push_back(std::move(tlAccelStructBuffer));
		fb->Commands->FrameDeleteList->accelStructs.push_back(std::move(tlAccelStruct));
	}
}

void VulkanRaytraceManager::CreateVulkanObjects()
{
	CreateVertexAndIndexBuffers();
	CreateBottomLevelAccelerationStructure();
	CreateTopLevelAccelerationStructure();
}

void VulkanRaytraceManager::CreateVertexAndIndexBuffers()
{
	static_assert(sizeof(vec3) == 3 * 4, "sizeof(vec3) is not 12 bytes!");

	size_t vertexbuffersize = (size_t)Mesh->MeshVertices.size() * sizeof(vec3);
	size_t indexbuffersize = (size_t)Mesh->MeshElements.size() * sizeof(uint32_t);
	size_t transferbuffersize = vertexbuffersize + indexbuffersize;
	size_t vertexoffset = 0;
	size_t indexoffset = vertexoffset + vertexbuffersize;

	BufferBuilder tbuilder;
	tbuilder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	tbuilder.setSize(transferbuffersize);
	transferBuffer = tbuilder.create(renderer->Device);
	transferBuffer->SetDebugName("transferBuffer");
	uint8_t* data = (uint8_t*)transferBuffer->Map(0, transferbuffersize);
	memcpy(data + vertexoffset, Mesh->MeshVertices.data(), vertexbuffersize);
	memcpy(data + indexoffset, Mesh->MeshElements.data(), indexbuffersize);
	transferBuffer->Unmap();

	BufferBuilder vbuilder;
	vbuilder.setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	vbuilder.setSize(vertexbuffersize);
	vertexBuffer = vbuilder.create(renderer->Device);
	vertexBuffer->SetDebugName("vertexBuffer");

	BufferBuilder ibuilder;
	ibuilder.setUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	ibuilder.setSize(indexbuffersize);
	indexBuffer = ibuilder.create(renderer->Device);
	indexBuffer->SetDebugName("indexBuffer");

	renderer->Commands->GetTransferCommands()->copyBuffer(transferBuffer.get(), vertexBuffer.get(), vertexoffset);
	renderer->Commands->GetTransferCommands()->copyBuffer(transferBuffer.get(), indexBuffer.get(), indexoffset);

	// Finish transfer before using it for building
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	renderer->Commands->GetTransferCommands()->pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void VulkanRaytraceManager::CreateBottomLevelAccelerationStructure()
{
	VkBufferDeviceAddressInfo info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	info.buffer = vertexBuffer->buffer;
	VkDeviceAddress vertexAddress = vkGetBufferDeviceAddress(renderer->Device->device, &info);

	info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	info.buffer = indexBuffer->buffer;
	VkDeviceAddress indexAddress = vkGetBufferDeviceAddress(renderer->Device->device, &info);

	VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
	triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	triangles.vertexData.deviceAddress = vertexAddress;
	triangles.vertexStride = sizeof(vec3);
	triangles.indexType = VK_INDEX_TYPE_UINT32;
	triangles.indexData.deviceAddress = indexAddress;
	triangles.maxVertex = (uint32_t)Mesh->MeshVertices.size() - 1;

	VkAccelerationStructureGeometryKHR accelStructBLDesc = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	accelStructBLDesc.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelStructBLDesc.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelStructBLDesc.geometry.triangles = triangles;

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo = {};
	rangeInfo.primitiveCount = (uint32_t)Mesh->MeshElements.size() / 3;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &accelStructBLDesc;

	uint32_t maxPrimitiveCount = rangeInfo.primitiveCount;

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	vkGetAccelerationStructureBuildSizesKHR(renderer->Device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &maxPrimitiveCount, &sizeInfo);

	BufferBuilder blbufbuilder;
	blbufbuilder.setUsage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	blbufbuilder.setSize(sizeInfo.accelerationStructureSize);
	blAccelStructBuffer = blbufbuilder.create(renderer->Device);
	blAccelStructBuffer->SetDebugName("blAccelStructBuffer");

	VkAccelerationStructureKHR blAccelStructHandle = {};
	VkAccelerationStructureCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	createInfo.buffer = blAccelStructBuffer->buffer;
	createInfo.size = sizeInfo.accelerationStructureSize;
	VkResult result = vkCreateAccelerationStructureKHR(renderer->Device->device, &createInfo, nullptr, &blAccelStructHandle);
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkCreateAccelerationStructureKHR failed");
	blAccelStruct = std::make_unique<VulkanAccelerationStructure>(renderer->Device, blAccelStructHandle);
	blAccelStruct->SetDebugName("blAccelStruct");

	BufferBuilder sbuilder;
	sbuilder.setUsage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	sbuilder.setSize(sizeInfo.buildScratchSize);
	blScratchBuffer = sbuilder.create(renderer->Device);
	blScratchBuffer->SetDebugName("blScratchBuffer");

	info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	info.buffer = blScratchBuffer->buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(renderer->Device->device, &info);

	buildInfo.dstAccelerationStructure = blAccelStruct->accelstruct;
	buildInfo.scratchData.deviceAddress = scratchAddress;
	VkAccelerationStructureBuildRangeInfoKHR* rangeInfos[] = { &rangeInfo };
	renderer->Commands->GetTransferCommands()->buildAccelerationStructures(1, &buildInfo, rangeInfos);

	// Finish building before using it as input to a toplevel accel structure
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	renderer->Commands->GetTransferCommands()->pipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void VulkanRaytraceManager::CreateTopLevelAccelerationStructure()
{
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	addressInfo.accelerationStructure = blAccelStruct->accelstruct;
	VkDeviceAddress blAccelStructAddress = vkGetAccelerationStructureDeviceAddressKHR(renderer->Device->device, &addressInfo);

	VkAccelerationStructureInstanceKHR instance = {};
	instance.transform.matrix[0][0] = 1.0f;
	instance.transform.matrix[1][1] = 1.0f;
	instance.transform.matrix[2][2] = 1.0f;
	instance.instanceCustomIndex = 0;
	instance.accelerationStructureReference = blAccelStructAddress;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.mask = 0xff;
	instance.instanceShaderBindingTableRecordOffset = 0;

	BufferBuilder tbuilder;
	tbuilder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	tbuilder.setSize(sizeof(VkAccelerationStructureInstanceKHR));
	tlTransferBuffer = tbuilder.create(renderer->Device);
	tlTransferBuffer->SetDebugName("tlTransferBuffer");
	auto data = (uint8_t*)tlTransferBuffer->Map(0, sizeof(VkAccelerationStructureInstanceKHR));
	memcpy(data, &instance, sizeof(VkAccelerationStructureInstanceKHR));
	tlTransferBuffer->Unmap();

	BufferBuilder instbufbuilder;
	instbufbuilder.setUsage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	instbufbuilder.setSize(sizeof(VkAccelerationStructureInstanceKHR));
	tlInstanceBuffer = instbufbuilder.create(renderer->Device);
	tlInstanceBuffer->SetDebugName("tlInstanceBuffer");

	renderer->Commands->GetTransferCommands()->copyBuffer(tlTransferBuffer.get(), tlInstanceBuffer.get());

	// Finish transfering before using it as input
	VkMemoryBarrier barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	renderer->Commands->GetTransferCommands()->pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

	VkBufferDeviceAddressInfo info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	info.buffer = tlInstanceBuffer->buffer;
	VkDeviceAddress instanceBufferAddress = vkGetBufferDeviceAddress(renderer->Device->device, &info);

	VkAccelerationStructureGeometryInstancesDataKHR instances = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
	instances.data.deviceAddress = instanceBufferAddress;

	VkAccelerationStructureGeometryKHR accelStructTLDesc = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	accelStructTLDesc.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelStructTLDesc.geometry.instances = instances;

	VkAccelerationStructureBuildRangeInfoKHR rangeInfo = {};
	rangeInfo.primitiveCount = 1;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &accelStructTLDesc;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

	uint32_t maxInstanceCount = 1;

	VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	vkGetAccelerationStructureBuildSizesKHR(renderer->Device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &maxInstanceCount, &sizeInfo);

	BufferBuilder tlbufbuilder;
	tlbufbuilder.setUsage(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	tlbufbuilder.setSize(sizeInfo.accelerationStructureSize);
	tlAccelStructBuffer = tlbufbuilder.create(renderer->Device);
	tlAccelStructBuffer->SetDebugName("tlAccelStructBuffer");

	VkAccelerationStructureKHR tlAccelStructHandle = {};
	VkAccelerationStructureCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	createInfo.buffer = tlAccelStructBuffer->buffer;
	createInfo.size = sizeInfo.accelerationStructureSize;
	VkResult result = vkCreateAccelerationStructureKHR(renderer->Device->device, &createInfo, nullptr, &tlAccelStructHandle);
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkCreateAccelerationStructureKHR failed");
	tlAccelStruct = std::make_unique<VulkanAccelerationStructure>(renderer->Device, tlAccelStructHandle);

	BufferBuilder sbuilder;
	sbuilder.setUsage(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	sbuilder.setSize(sizeInfo.buildScratchSize);
	tlScratchBuffer = sbuilder.create(renderer->Device);
	tlScratchBuffer->SetDebugName("tlScratchBuffer");

	info = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	info.buffer = tlScratchBuffer->buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(renderer->Device->device, &info);

	buildInfo.dstAccelerationStructure = tlAccelStruct->accelstruct;
	buildInfo.scratchData.deviceAddress = scratchAddress;

	VkAccelerationStructureBuildRangeInfoKHR* rangeInfos[] = { &rangeInfo };
	renderer->Commands->GetTransferCommands()->buildAccelerationStructures(1, &buildInfo, rangeInfos);

	// Finish building the accel struct before using as input in a fragment shader
	PipelineBarrier finishbuildbarrier;
	finishbuildbarrier.addMemory(VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_ACCESS_SHADER_READ_BIT);
	finishbuildbarrier.execute(renderer->Commands->GetTransferCommands(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}
