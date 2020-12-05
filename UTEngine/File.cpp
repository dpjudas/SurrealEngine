
#include "Precomp.h"
#include "File.h"
#include <Windows.h>
#include <stdexcept>

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

	void write(const void *data, int size) override
	{
		BOOL result = WriteFile(handle, data, size, nullptr, nullptr);
		if (result == FALSE)
			throw std::runtime_error("WriteFile failed");
	}

	void read(void *data, size_t size) override
	{
		DWORD bytesRead = 0;
		BOOL result = ReadFile(handle, data, (DWORD)size, &bytesRead, nullptr);
		if (result == FALSE || bytesRead != size)
			throw std::runtime_error("ReadFile failed");
	}

	int64_t size() override
	{
		LARGE_INTEGER fileSize;
		BOOL result = GetFileSizeEx(handle, &fileSize);
		if (result == FALSE)
			throw std::runtime_error("GetFileSizeEx failed");
		return fileSize.QuadPart;
	}

	void seek(int64_t offset, SeekPoint origin) override
	{
		LARGE_INTEGER off, newoff;
		off.QuadPart = offset;
		DWORD moveMethod = FILE_BEGIN;
		if (origin == SeekPoint::current) moveMethod = FILE_CURRENT;
		else if (origin == SeekPoint::end) moveMethod = FILE_END;
		BOOL result = SetFilePointerEx(handle, off, &newoff, moveMethod);
		if (result == FALSE)
			throw std::runtime_error("SetFilePointerEx failed");
	}

	uint64_t tell() override
	{
		LARGE_INTEGER offset, delta;
		delta.QuadPart = 0;
		BOOL result = SetFilePointerEx(handle, delta, &offset, FILE_CURRENT);
		if (result == FALSE)
			throw std::runtime_error("SetFilePointerEx failed");
		return offset.QuadPart;
	}

	HANDLE handle = INVALID_HANDLE_VALUE;
};

std::shared_ptr<File> File::create_always(const std::string &filename)
{
	HANDLE handle = CreateFileA(filename.c_str(), FILE_WRITE_ACCESS, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::open_existing(const std::string &filename)
{
	HANDLE handle = CreateFileA(filename.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

/////////////////////////////////////////////////////////////////////////////

std::vector<std::string> Directory::files(const std::string &filename)
{
	std::vector<std::string> files;

	WIN32_FIND_DATAA fileinfo;
	HANDLE handle = FindFirstFileA(filename.c_str(), &fileinfo);
	if (handle == INVALID_HANDLE_VALUE)
		return {};

	try
	{
		do
		{
			bool is_directory = !!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			if (!is_directory)
				files.push_back(fileinfo.cFileName);
		} while (FindNextFileA(handle, &fileinfo) == TRUE);
		FindClose(handle);
	}
	catch (...)
	{
		FindClose(handle);
		throw;
	}

	return files;
}

/////////////////////////////////////////////////////////////////////////////

bool FilePath::has_extension(const std::string &filename, const char *checkext)
{
	auto fileext = extension(filename);
	return _stricmp(fileext.c_str(), checkext) == 0;
}

std::string FilePath::extension(const std::string &filename)
{
	std::string file = last_component(filename);
	std::string::size_type pos = file.find_last_of('.');
	if (pos == std::string::npos)
		return std::string();

#ifndef WIN32
	// Files beginning with a dot is not a filename extension in Unix.
	// This is different from Windows where it is considered the extension.
	if (path_type == FilePathType::file_system && pos == 0)
		return std::string();
#endif

	return file.substr(pos + 1);

}

std::string FilePath::remove_extension(const std::string &filename)
{
	std::string file = last_component(filename);
	std::string::size_type pos = file.find_last_of('.');
	if (pos == std::string::npos)
		return filename;
	else
		return filename.substr(0, filename.length() - file.length() + pos);
}

std::string FilePath::last_component(const std::string &path)
{
#ifdef WIN32
	auto last_slash = path.find_last_of("/\\");
	if (last_slash != std::string::npos)
		return path.substr(last_slash + 1);
	else
		return path;
#else
	auto last_slash = path.find_last_of('/');
	if (last_slash != std::string::npos)
		return path.substr(last_slash + 1);
	else
		return path;
#endif
}

std::string FilePath::remove_last_component(const std::string &path)
{
#ifdef WIN32
	auto last_slash = path.find_last_of("/\\");
	if (last_slash != std::string::npos)
		return path.substr(0, last_slash + 1);
	else
		return std::string();
#else
	auto last_slash = path.find_last_of('/');
	if (last_slash != std::string::npos)
		return path.substr(0, last_slash + 1);
	else
		return std::string();
#endif
}

std::string FilePath::combine(const std::string &path1, const std::string &path2)
{
#ifdef WIN32
	if (path1.empty())
		return path2;
	else if (path2.empty())
		return path1;
	else if (path2.front() == '/' || path2.front() == '\\')
		return path2;
	else if (path1.back() != '/' && path1.back() != '\\')
		return path1 + "\\" + path2;
	else
		return path1 + path2;
#else
	if (path1.empty())
		return path2;
	else if (path2.empty())
		return path1;
	else if (path2.front() == '/')
		return path2;
	else if (path1.back() != '/')
		return path1 + "/" + path2;
	else
		return path1 + path2;
#endif
}
