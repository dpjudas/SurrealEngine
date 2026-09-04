
#include "Directory.h"
#include "FilePath.h"
#include "UTF16.h"
#include <stdexcept>

#ifndef WIN32
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>
#ifndef PATH_MAX
#include <linux/limits.h>
#endif
#include <fnmatch.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#else
#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <Shlobj.h>
#endif

#ifdef WIN32
std::string Directory::currentDirectory()
{
	DWORD bufferSize = GetCurrentDirectory(0, nullptr);
	std::wstring buffer;
	buffer.resize(bufferSize);
	DWORD result = GetCurrentDirectory((DWORD)buffer.size(), buffer.data());
	if (result == 0)
		throw std::runtime_error("Could not get current directory");
	buffer.resize(result);
	return from_utf16(buffer);
}
#else
std::string Directory::currentDirectory()
{
	std::string buffer;
	buffer.resize(256);
	while (true)
	{
		char* result = getcwd(buffer.data(), buffer.size());
		if (result)
			break;
		if (errno != ERANGE)
			throw std::runtime_error("Could not get current directory");
		buffer.resize(buffer.size() * 2);
	}
	buffer.resize(strlen(buffer.data()));
	return buffer;
}
#endif

#ifdef WIN32
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
std::string Directory::localAppData()
{
	// We're trying to find "~/.config" here
	const char* homeDir = std::getenv("HOME");

	if (!homeDir)
		throw std::runtime_error("HOME environment variable is not set");

	return FilePath::combine(homeDir, ".config");
}
#endif

std::vector<std::string> Directory::files(const std::string& filename)
{
#ifdef WIN32
	std::vector<std::string> files;

	WIN32_FIND_DATAW fileinfo;
	HANDLE handle = FindFirstFileW(to_utf16(filename).c_str(), &fileinfo);
	if (handle == INVALID_HANDLE_VALUE)
		return {};

	try
	{
		do
		{
			bool is_directory = !!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			if (!is_directory)
				files.push_back(from_utf16(fileinfo.cFileName));
		} while (FindNextFileW(handle, &fileinfo) == TRUE);
		FindClose(handle);
	}
	catch (...)
	{
		FindClose(handle);
		throw;
	}

	return files;
#else
	std::string pattern = FilePath::lastComponent(filename);
	std::string path = FilePath::removeLastComponent(filename);
	if (path.empty())
		path = ".";
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return {};
	std::vector<std::string> files;
	try
	{
		while (true)
		{
			dirent* entry = readdir(dir);
			if (!entry)
				break;

			if (fnmatch(pattern.c_str(), entry->d_name, FNM_PATHNAME) != 0)
				continue;

			std::string name = entry->d_name;

			if (name == "." || name == "..")
				continue;

			std::string fullname = FilePath::combine(path, name);

			struct stat s = {};
			int result = stat(fullname.c_str(), &s);
			if (result == 0)
			{
				if ((s.st_mode & S_IFMT) != S_IFDIR)
				{
					files.push_back(name);
				}
			}
		}
		closedir(dir);
	}
	catch (...)
	{
		closedir(dir);
		throw;
	}
	return files;
#endif
}

std::vector<std::string> Directory::folders(const std::string& filename)
{
#ifdef WIN32
	std::vector<std::string> files;

	WIN32_FIND_DATAW fileinfo;
	HANDLE handle = FindFirstFileW(to_utf16(filename).c_str(), &fileinfo);
	if (handle == INVALID_HANDLE_VALUE)
		return {};

	try
	{
		do
		{
			bool is_directory = !!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			if (is_directory)
			{
				files.push_back(from_utf16(fileinfo.cFileName));
				if (files.back() == "." || files.back() == "..")
					files.pop_back();
			}
		} while (FindNextFileW(handle, &fileinfo) == TRUE);
		FindClose(handle);
	}
	catch (...)
	{
		FindClose(handle);
		throw;
	}

	return files;
#else
	std::string pattern = FilePath::lastComponent(filename);
	std::string path = FilePath::removeLastComponent(filename);
	if (path.empty())
		path = ".";
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return {};
	std::vector<std::string> files;
	try
	{
		while (true)
		{
			dirent* entry = readdir(dir);
			if (!entry)
				break;

			if (fnmatch(pattern.c_str(), entry->d_name, FNM_PATHNAME) != 0)
				continue;

			std::string name = entry->d_name;

			if (name == "." || name == "..")
				continue;

			std::string fullname = FilePath::combine(path, name);

			struct stat s = {};
			int result = stat(fullname.c_str(), &s);
			if (result == 0)
			{
				if ((s.st_mode & S_IFMT) == S_IFDIR)
				{
					files.push_back(name);
				}
			}
		}
		closedir(dir);
	}
	catch (...)
	{
		closedir(dir);
		throw;
	}
	return files;
#endif
}

#ifdef WIN32
std::string Directory::exePath()
{
	WCHAR exe_filename[1024];
	DWORD len = GetModuleFileNameW(nullptr, exe_filename, 1024);
	if (len == 0 || len == 1024)
		throw std::runtime_error("GetModuleFileName failed!");
	return FilePath::removeLastComponent(from_utf16(exe_filename));
}
#else
std::string Directory::exePath()
{
	char exe_file[PATH_MAX];
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	if (mainBundle)
	{
		CFURLRef mainURL = CFBundleCopyBundleURL(mainBundle);

		if (mainURL)
		{
			int ok = CFURLGetFileSystemRepresentation(
				mainURL, (Boolean)true, (UInt8*)exe_file, PATH_MAX
			);

			if (ok)
			{
				return std::string(exe_file) + "/";
			}
		}
	}

	throw std::runtime_error("get_exe_path failed");

#else
#ifndef PROC_EXE_PATH
#define PROC_EXE_PATH "/proc/self/exe"
#endif
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
			throw std::runtime_error("get_exe_path: could not find path");
		else
			return std::string(exe_file);
#else
		throw std::runtime_error("get_exe_path: proc file system not accesible");
#endif
	}
	else
	{
		size = readlink(PROC_EXE_PATH, exe_file, PATH_MAX);
		if (size < 0)
		{
			throw std::runtime_error(strerror(errno));
		}
		else
		{
			exe_file[size] = '\0';
			return std::string(dirname(exe_file)) + "/";
		}
	}
#endif

}
#endif

void Directory::create(const std::string& path)
{
#ifdef WIN32
	BOOL result = CreateDirectoryW(to_utf16(path).c_str(), nullptr);
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
				std::string parent = FilePath::removeLastComponent(path);
				if (!parent.empty())
				{
					Directory::create(parent);
					if (CreateDirectoryW(to_utf16(path).c_str(), nullptr) == TRUE)
						return;
				}
			}
			catch (...)
			{
			}
		}
		throw std::runtime_error("Could not create directory for path " + path);
	}
#else
	int result = mkdir(path.c_str(), 0777);
	if (result < 0)
	{
		if (errno == EEXIST)
		{
			return;
		}
		else if (errno == ENOENT)
		{
			try
			{
				std::string parent = FilePath::removeLastComponent(path);
				if (!parent.empty())
				{
					Directory::create(parent);
					if (mkdir(path.c_str(), 0777) == 0)
						return;
				}
			}
			catch (...)
			{
			}
		}
		throw std::runtime_error("Could not create directory for path " + path);
	}
#endif
}

bool Directory::trySetHidden(const std::string& path)
{
#ifdef WIN32
	std::wstring path16 = to_utf16(path);

	DWORD attributes = GetFileAttributes(path16.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES)
		return false;

	attributes |= FILE_ATTRIBUTE_HIDDEN;

	BOOL result = SetFileAttributes(path16.c_str(), attributes);
	return result == TRUE;
#else
	return true; // Hidden flag does not exist on Unix
#endif
}
