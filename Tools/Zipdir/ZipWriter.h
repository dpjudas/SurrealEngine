
#pragma once

#include "IODevice.h"
#include <string>
#include <memory>

class ZipWriter
{
public:
	static std::unique_ptr<ZipWriter> create(std::shared_ptr<IODevice> output);

	virtual ~ZipWriter() = default;
	virtual void addFile(const std::string& filename, bool compress, const void* data, uint64_t size) = 0;
	virtual void addFile(const std::string& filename, bool compress_file, const void* data, uint64_t size, int64_t lastModified) = 0;
	virtual void writeToc() = 0;
};
