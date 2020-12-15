
#include "Precomp.h"
#include "PackageManager.h"
#include "Package.h"
#include "File.h"

PackageManager::PackageManager(const std::string& basepath) : basepath(basepath)
{
	ScanFolder("Maps", "*.unr");
	ScanFolder("Music", "*.umx");
	ScanFolder("Sounds", "*.uax");
	ScanFolder("System", "*.u");
	ScanFolder("Textures", "*.utx");
}

Package* PackageManager::GetPackage(const std::string& name)
{
	std::string key = GetKey(name);

	auto& package = packages[key];
	if (package)
		return package.get();
	
	auto it = packageFilenames.find(key);
	if (it != packageFilenames.end())
	{
		package = std::make_unique<Package>(this, key, it->second);
	}
	else
	{
		throw std::runtime_error("Could not find package " + name);
	}

	return package.get();
}

void PackageManager::ScanFolder(const std::string& name, const std::string& search)
{
	std::string packagedir = FilePath::combine(basepath, name);
	for (std::string filename : Directory::files(FilePath::combine(packagedir, search)))
	{
		packageFilenames[GetKey(FilePath::remove_extension(filename))] = FilePath::combine(packagedir, filename);
	}
}

std::string PackageManager::GetKey(std::string name)
{
	for (char& c : name)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = c - 'A' + 'a';
		}
	}
	return name;
}

std::vector<std::string> PackageManager::GetPackageNames() const
{
	std::vector<std::string> names;
	for (auto& it : packageFilenames)
	{
		names.push_back(it.first);
	}
	return names;
}
