#pragma once

#include "DataBuffer.h"
#include "IODevice.h"
#include <string>
#include <optional>

class File : public IODevice
{
public:
	static std::shared_ptr<File> createAlways(const std::string& filename);
	static std::shared_ptr<File> openExisting(const std::string& filename);

	static bool tryDelete(const std::string& filename);
	static void deleteAlways(const std::string& filename);

	static int64_t getLastWriteTime(const std::string& filename);
	static std::optional<int64_t> tryGetLastWriteTime(const std::string& filename);
	static bool exists(const std::string filename);

	static std::string readAllText(const std::string& filename);
	static void writeAllText(const std::string& filename, const std::string& text);

	static std::shared_ptr<DataBuffer> readAllBytes(const std::string& filename);
	static void writeAllBytes(const std::string& filename, const void* data, const uint64_t size);
	static void writeAllBytes(const std::string& filename, const std::shared_ptr<DataBuffer>& data) { writeAllBytes(filename, data->data(), data->size()); }
};
