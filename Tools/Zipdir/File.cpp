
#include "File.h"
#include "UTF16.h"
#include <cstdio>
#include <stdexcept>

#ifndef WIN32
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#endif

enum class SeekPoint
{
	begin,
	current,
	end
};

#ifdef WIN32

#define NOMINMAX
#include <Windows.h>

class FileImpl : public File
{
public:
	FileImpl(HANDLE handle) : handle(handle)
	{
	}

	~FileImpl()
	{
		CloseHandle(handle);
	}

	void write(const void* data, size_t size) override
	{
		size_t pos = 0;
		while (pos < size)
		{
			size_t writesize = std::min(size, (size_t)0xffffffff);
			BOOL result = WriteFile(handle, (const uint8_t*)data + pos, (DWORD)writesize, nullptr, nullptr);
			if (result == FALSE)
				throw std::runtime_error("WriteFile failed");
			pos += writesize;
		}
	}

	size_t try_read(void* data, size_t size) override
	{
		size_t pos = 0;
		while (pos < size)
		{
			size_t readsize = std::min(size, (size_t)0xffffffff);
			DWORD bytesRead = 0;
			BOOL result = ReadFile(handle, (uint8_t*)data + pos, (DWORD)readsize, &bytesRead, nullptr);
			if (result == FALSE)
				throw std::runtime_error("ReadFile failed");
			pos += bytesRead;
			if (bytesRead != readsize)
				break;
		}
		return pos;
	}

	int64_t size() const override
	{
		LARGE_INTEGER fileSize;
		BOOL result = GetFileSizeEx(handle, &fileSize);
		if (result == FALSE)
			throw std::runtime_error("GetFileSizeEx failed");
		return fileSize.QuadPart;
	}

	int64_t seek(int64_t offset) override
	{
		return seek(offset, SeekPoint::begin);
	}

	int64_t seek_from_current(int64_t offset) override
	{
		return seek(offset, SeekPoint::current);
	}

	int64_t seek_from_end(int64_t offset) override
	{
		return seek(offset, SeekPoint::end);
	}

	int64_t seek(int64_t offset, SeekPoint origin)
	{
		LARGE_INTEGER off, newoff;
		off.QuadPart = offset;
		DWORD moveMethod = FILE_BEGIN;
		if (origin == SeekPoint::current) moveMethod = FILE_CURRENT;
		else if (origin == SeekPoint::end) moveMethod = FILE_END;
		BOOL result = SetFilePointerEx(handle, off, &newoff, moveMethod);
		if (result == FALSE)
			throw std::runtime_error("SetFilePointerEx failed");
		return newoff.QuadPart;
	}

	HANDLE handle = INVALID_HANDLE_VALUE;
};

std::shared_ptr<File> File::createAlways(const std::string& filename)
{
	HANDLE handle = CreateFileW(to_utf16(filename).c_str(), FILE_WRITE_ACCESS, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::openExisting(const std::string& filename)
{
	HANDLE handle = CreateFileW(to_utf16(filename).c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

bool File::tryDelete(const std::string& filename)
{
	return DeleteFile(to_utf16(filename).c_str()) == TRUE;
}

void File::deleteAlways(const std::string& filename)
{
	if (!tryDelete(filename))
		throw std::runtime_error("Could not delete " + filename);
}

int64_t File::getLastWriteTime(const std::string& filename)
{
	HANDLE handle = CreateFileW(to_utf16(filename).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open " + filename);

	FILETIME filetime = {};
	BOOL result = GetFileTime(handle, nullptr, nullptr, &filetime);
	CloseHandle(handle);

	if (result == FALSE)
		throw std::runtime_error("GetFileTime failed for " + filename);

	LARGE_INTEGER li;
	li.LowPart = filetime.dwLowDateTime;
	li.HighPart = filetime.dwHighDateTime;
	return li.QuadPart;
}

#else

#include <stdio.h>

class FileImpl : public File
{
public:
	FileImpl(FILE* handle) : handle(handle)
	{
	}

	~FileImpl()
	{
		fclose(handle);
	}

	void write(const void* data, size_t size) override
	{
		size_t result = fwrite(data, 1, size, handle);
		if (result != size)
			throw std::runtime_error("File write failed");
	}

	size_t try_read(void* data, size_t size) override
	{
		return fread(data, 1, size, handle);
	}

	int64_t size() const override
	{
		auto pos = ftell(handle);
		auto result = fseek(handle, 0, SEEK_END);
		if (result == -1)
			throw std::runtime_error("File seek failed");
		auto length = ftell(handle);
		fseek(handle, pos, SEEK_SET);
		if (length == -1)
			throw std::runtime_error("File tell failed");
		return length;
	}

	int64_t seek(int64_t offset) override
	{
		return seek(offset, SeekPoint::begin);
	}

	int64_t seek_from_current(int64_t offset) override
	{
		return seek(offset, SeekPoint::current);
	}

	int64_t seek_from_end(int64_t offset) override
	{
		return seek(offset, SeekPoint::end);
	}

	int64_t seek(int64_t offset, SeekPoint origin)
	{
		if (origin == SeekPoint::current)
		{
			auto result = fseek(handle, offset, SEEK_CUR);
			if (result == -1)
				throw std::runtime_error("File seek failed");
		}
		else if (origin == SeekPoint::end)
		{
			auto result = fseek(handle, offset, SEEK_END);
			if (result == -1)
				throw std::runtime_error("File seek failed");
		}
		else
		{
			auto result = fseek(handle, offset, SEEK_SET);
			if (result == -1)
				throw std::runtime_error("File seek failed");
		}
		return tell();
	}

	uint64_t tell()
	{
		auto result = ftell(handle);
		if (result == -1)
			throw std::runtime_error("File tell failed");
		return result;
	}

	FILE* handle = nullptr;
};

std::shared_ptr<File> File::createAlways(const std::string& filename)
{
	FILE* handle = fopen(filename.c_str(), "wb");
	if (!handle)
		throw std::runtime_error("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::openExisting(const std::string& filename)
{
	FILE* handle = fopen(filename.c_str(), "rb");
	if (!handle)
		throw std::runtime_error("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

int64_t File::getLastWriteTime(const std::string& filename)
{
	struct stat s = {};
	int result = stat(filename.c_str(), &s);
	if (result < 0)
		throw std::runtime_error("Could not open " + filename);
	return s.st_mtime;
}

bool File::tryDelete(const std::string& filename)
{
	return unlink(filename.c_str()) == 0;
}

void File::deleteAlways(const std::string& filename)
{
	if (!tryDelete(filename))
		throw std::runtime_error("Could not delete " + filename);
}

#endif

bool File::exists(const std::string filename)
{
	try
	{
		auto file = openExisting(filename);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

std::string File::readAllText(const std::string& filename)
{
	auto file = File::openExisting(filename);
	std::string buffer(file->size(), ' ');
	file->read(buffer.data(), buffer.size());
	return buffer;
}

void File::writeAllText(const std::string& filename, const std::string& text)
{
	auto file = File::createAlways(filename);
	file->write(text.data(), text.size());
}

std::shared_ptr<DataBuffer> File::readAllBytes(const std::string& filename)
{
	auto file = File::openExisting(filename);
	auto buffer = DataBuffer::create(file->size());
	file->read(buffer->data(), buffer->size());
	return buffer;
}

void File::writeAllBytes(const std::string& filename, const void* data, const uint64_t size)
{
	auto file = File::createAlways(filename);
	file->write(data, size);
}

std::optional<int64_t> File::tryGetLastWriteTime(const std::string& filename)
{
	try
	{
		return File::getLastWriteTime(filename);
	}
	catch (...)
	{
		return {};
	}
}
