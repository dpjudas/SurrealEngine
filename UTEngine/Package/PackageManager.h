#pragma once

#include "Package.h"

class PackageManager
{
public:
	PackageManager(const std::string& basepath);

	Package *GetPackage(const std::string& name);
	std::vector<std::string> GetPackageNames() const;

private:
	void ScanFolder(const std::string& name, const std::string& search);
	std::string GetKey(std::string name);

	std::string basepath;
	std::map<std::string, std::string> packageFilenames;
	std::map<std::string, std::unique_ptr<Package>> packages;
};
