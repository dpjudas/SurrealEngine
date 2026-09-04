#pragma once

#include "IODevice.h"
#include <memory>

class DataBuffer;
class MemoryDeviceImpl;

class MemoryDevice : public IODevice
{
public:
	static std::shared_ptr<MemoryDevice> create();
	static std::shared_ptr<MemoryDevice> open(const std::shared_ptr<DataBuffer> &buffer);

	virtual const std::shared_ptr<DataBuffer> &buffer() const = 0;
	virtual void setBuffer(const std::shared_ptr<DataBuffer> &buffer) = 0;
};
