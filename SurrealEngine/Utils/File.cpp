
#include "Precomp.h"
#include "Utils/File.h"
#include "Utils/UTF16.h"
#include "Utils/StrCompare.h"
#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#include <Shlobj.h>
#else
#include <libgen.h>
#include <fnmatch.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef EXTERN___PROGNAME
extern const char* __progname;
#endif
#endif
#include "Utils/Exception.h"
#include <cstring>
#include <sstream>
#include <cstdio>

#ifdef WIN32

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

	void write(const void *data, size_t size) override
	{
		size_t pos = 0;
		while (pos < size)
		{
			size_t writesize = std::min(size, (size_t)0xffffffff);
			BOOL result = WriteFile(handle, (const uint8_t*)data + pos, (DWORD)writesize, nullptr, nullptr);
			if (result == FALSE)
				Exception::Throw("WriteFile failed");
			pos += writesize;
		}
	}

	void read(void *data, size_t size) override
	{
		size_t pos = 0;
		while (pos < size)
		{
			size_t readsize = std::min(size, (size_t)0xffffffff);
			DWORD bytesRead = 0;
			BOOL result = ReadFile(handle, (uint8_t*)data + pos, (DWORD)readsize, &bytesRead, nullptr);
			if (result == FALSE || bytesRead != readsize)
				Exception::Throw("ReadFile failed");
			pos += readsize;
		}
	}

	int64_t size() override
	{
		LARGE_INTEGER fileSize;
		BOOL result = GetFileSizeEx(handle, &fileSize);
		if (result == FALSE)
			Exception::Throw("GetFileSizeEx failed");
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
			Exception::Throw("SetFilePointerEx failed");
	}

	uint64_t tell() override
	{
		LARGE_INTEGER offset, delta;
		delta.QuadPart = 0;
		BOOL result = SetFilePointerEx(handle, delta, &offset, FILE_CURRENT);
		if (result == FALSE)
			Exception::Throw("SetFilePointerEx failed");
		return offset.QuadPart;
	}

	HANDLE handle = INVALID_HANDLE_VALUE;
};

std::shared_ptr<File> File::create_always(const std::string &filename)
{
	HANDLE handle = CreateFile(to_utf16(filename).c_str(), FILE_WRITE_ACCESS, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		Exception::Throw("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::open_existing(const std::string &filename)
{
	HANDLE handle = CreateFile(to_utf16(filename).c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		Exception::Throw("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

bool File::try_delete(const std::string& filename)
{
	return DeleteFile(to_utf16(filename).c_str()) == TRUE;
}

void File::delete_always(const std::string& filename)
{
	if (!try_delete(filename))
		throw std::runtime_error("Could not delete " + filename);
}

void File::rename(const std::string& sourceName, const std::string& destinationName)
{
	std::filesystem::rename(sourceName, destinationName);
}

#else

class FileImpl : public File
{
public:
	FileImpl(FILE* handle) : handle(handle)
	{
	}

	~FileImpl()
	{
		if (handle)
			fclose(handle);
	}

	int64_t size() override
	{
		auto pos = ftell(handle);
		fseek(handle, 0, SEEK_END);
		auto s = ftell(handle);
		fseek(handle, pos, SEEK_SET);
		return s;
	}

	void read(void *data, size_t size) override
	{
		if (size == 0)
			return;

		size_t result = fread(data, size, 1, handle);
		if (result != 1)
			Exception::Throw("fread failed");
	}

	void write(const void *data, size_t size) override
	{
		if (size == 0)
			return;

		size_t result = fwrite(data, size, 1, handle);
		if (result != 1)
			Exception::Throw("fwrite failed");
	}

	void seek(int64_t offset, SeekPoint origin = SeekPoint::begin) override
	{
		int moveMethod = SEEK_SET;
		if (origin == SeekPoint::current) moveMethod = SEEK_CUR;
		else if (origin == SeekPoint::end) moveMethod = SEEK_END;
		int result = fseek(handle, offset, moveMethod);
		if (result != 0)
			Exception::Throw("fseek failed");
	}

	uint64_t tell() override
	{
		auto result = ftell(handle);
		if (result == -1)
			Exception::Throw("ftell failed");
		return result;
	}

	FILE* handle = nullptr;
};

std::shared_ptr<File> File::create_always(const std::string &filename)
{
	FILE* handle = fopen(filename.c_str(), "wb");
	if (handle == nullptr)
		Exception::Throw("Could not create " + filename);

	return std::make_shared<FileImpl>(handle);
}

std::shared_ptr<File> File::open_existing(const std::string &filename)
{
	FILE* handle = fopen(filename.c_str(), "rb");
	if (handle == nullptr)
		Exception::Throw("Could not open " + filename);

	return std::make_shared<FileImpl>(handle);
}

bool File::try_delete(const std::string& filename)
{
	return std::remove(filename.c_str()) == 0;
}

void File::delete_always(const std::string& filename)
{
	if (!try_delete(filename))
		throw std::runtime_error("Could not delete " + filename);
}

void File::rename(const std::string& sourceName, const std::string& destinationName)
{
	std::filesystem::rename(sourceName, destinationName);
}

#endif

void File::write_all_bytes(const std::string& filename, const void* data, size_t size)
{
	auto file = create_always(filename);
	file->write(data, size);
}

void File::write_all_text(const std::string& filename, const std::string& text)
{
	auto file = create_always(filename);
	file->write(text.data(), text.size());
}

Array<uint8_t> File::read_all_bytes(const std::string& filename)
{
	auto file = open_existing(filename);
	Array<uint8_t> buffer(file->size());
	file->read(buffer.data(), buffer.size());
	return buffer;
}

std::string File::read_all_text(const std::string& filename)
{
	auto file = open_existing(filename);
	auto size = file->size();
	if (size == 0) return {};
	std::string buffer;
	buffer.resize(size);
	file->read(&buffer[0], buffer.size());
	return buffer;
}

Array<std::string> File::read_all_lines(const std::string& filename)
{
	std::string text = read_all_text(filename);

	if (text.empty())
		return {};

	Array<std::string> result;

	std::string buffer;

	std::stringstream ss(text);

	while (std::getline(ss, buffer)) // Default delimiter is '\n'
		result.push_back(buffer);

	return result;
}

/////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

void Directory::create(const std::string& path)
{
	BOOL result = CreateDirectory(to_utf16(path).c_str(), nullptr);
	if (result == FALSE)
	{
		DWORD error = GetLastError();
		if (error == ERROR_ALREADY_EXISTS)
		{
			return;
		}
		else if (error == ERROR_PATH_NOT_FOUND)
		{
			try
			{
				std::string parent = fs::path(path).parent_path().string();
				if (!parent.empty())
				{
					Directory::create(parent);
					if (CreateDirectory(to_utf16(path).c_str(), nullptr) == TRUE)
						return;
				}
			}
			catch (...)
			{
			}
		}
		throw std::runtime_error("Could not create directory for path " + path);
	}
}

std::string Directory::localAppData()
{
	PWSTR path = nullptr;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &path)))
		throw std::runtime_error("SHGetKnownFolderPath(FOLDERID_LocalAppData) failed");
	try
	{
		std::string folder = from_utf16(path);
		CoTaskMemFree(path);
		return folder;
	}
	catch (...)
	{
		CoTaskMemFree(path);
		throw;
	}
}

#else

void Directory::create(const std::string& dirname)
{
	if (mkdir(dirname.c_str(), 0777) == 0)
		return;
	if (errno == EEXIST)
		return;
	if (errno == ENOENT)
	{
		try
		{
			fs::path parent = fs::path(dirname).parent_path();
			if (!parent.empty())
			{
				Directory::create(parent.string());
				if (mkdir(dirname.c_str(), 0777) == 0)
					return;
			}
		}
		catch (...)
		{
		}
	}
	Exception::Throw("Could not create directory " + dirname);
}

std::string Directory::localAppData()
{
	// We're trying to find "~/.config" here
	const char* homeDir = getenv("HOME");

	if (!homeDir)
		throw std::runtime_error("HOME environment variable is not set... somehow.");

	return (fs::path(homeDir) / ".config").string();
}

#endif

std::string OS::executable_path()
{
#if defined(WIN32)
	WCHAR exe_filename[_MAX_PATH];
	DWORD len = GetModuleFileName(nullptr, exe_filename, _MAX_PATH);
	if (len == 0 || len == _MAX_PATH)
		Exception::Throw("GetModuleFileName failed!");

	WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR];
	_wsplitpath_s(exe_filename, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

	return from_utf16(std::wstring(drive) + dir);
#elif defined(__APPLE__)
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		CFURLRef mainURL = CFBundleCopyBundleURL(mainBundle);

		if (mainURL)
		{
			char exe_file[PATH_MAX];
			int ok = CFURLGetFileSystemRepresentation(mainURL, (Boolean)true, (UInt8*)exe_file, PATH_MAX);
			if (ok)
			{
				return std::string(exe_file) + "/";
			}
		}
	}

	Exception::Throw("get_exe_path failed");
#else
	#ifndef PROC_EXE_PATH
	#define PROC_EXE_PATH "/proc/self/exe"
	#endif

	char exe_file[PATH_MAX];
	int size;
	struct stat sb;
	if (lstat(PROC_EXE_PATH, &sb) < 0)
	{
		#ifdef EXTERN___PROGNAME
			char* pathenv, * name, * end;
			char fname[PATH_MAX];
			char cwd[PATH_MAX];
			struct stat sba;

			exe_file[0] = '\0';
			if ((pathenv = getenv("PATH")) != nullptr)
			{
				for (name = pathenv; name; name = end)
				{
					if ((end = strchr(name, ':')))
						*end++ = '\0';
					snprintf(fname, sizeof(fname),
						"%s/%s", name, (char*)__progname);
					if (stat(fname, &sba) == 0) {
						snprintf(exe_file, sizeof(exe_file),
							"%s/", name);
						break;
					}
				}
			}
			// if getenv failed or path still not found
			// try current directory as last resort
			if (!exe_file[0])
			{
				if (getcwd(cwd, sizeof(cwd)) != nullptr)
				{
					snprintf(fname, sizeof(fname),
						"%s/%s", cwd, (char*)__progname);
					if (stat(fname, &sba) == 0)
						snprintf(exe_file, sizeof(exe_file),
							"%s/", cwd);
				}
			}
			if (!exe_file[0])
				Exception::Throw("get_exe_path: could not find path");
			else
				return std::string(exe_file);
		#else
			Exception::Throw("get_exe_path: proc file system not accesible");
		#endif
	}
	else
	{
		size = readlink(PROC_EXE_PATH, exe_file, PATH_MAX);
		if (size < 0)
		{
			Exception::Throw(strerror(errno));
		}
		else
		{
			exe_file[size] = '\0';
			return std::string(dirname(exe_file)) + "/";
		}
	}

	#endif
}

std::string OS::get_default_font_name()
{
#ifdef WIN32
	return "segoeui.ttf";
#elif defined(APPLE)
	return "SFUIDisplay-Regular.otf"; // Guess this is the default on Apple?
#else
	return "DejaVuSans.ttf";
#endif
}

std::string OS::find_truetype_font(const std::string& font_name_and_extension)
{
#ifdef WIN32
	const Array<std::string> possible_fonts_folders = {
		"C:\\Windows\\Fonts"
	};
#elif defined(APPLE)
	// Based on: https://superuser.com/a/407449
	// Start from user Fonts folder, and go up from there
	const Array<std::string> possible_fonts_folders = {
		"~/Library/Fonts",
		"/Library/Fonts",
		"/System/Library/Fonts"
	};
#else
	// Linux is Linux and as always, how the fonts stored can be wholly different
	// So here we try to account for possible paths
	// ...is there a better way to do this?
	const Array<std::string> possible_fonts_folders = {
		"/usr/share/fonts",
		"/usr/share/fonts/truetype",
		"/usr/share/fonts/TTF"
	};
#endif

	for (auto& current_font_folder : possible_fonts_folders) {
		auto final_path = fs::path(current_font_folder) / font_name_and_extension;

		if (File::try_open_existing(final_path.string()))
			return final_path.string();
	}

	// We couldn't find the font in any of the folders, bail out
	return "";
}