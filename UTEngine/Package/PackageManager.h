#pragma once

#include "Package.h"
#include <list>

class PackageStream;
class UObject;

class PackageManager
{
public:
	PackageManager(const std::string& basepath);

	Package *GetPackage(const std::string& name);
	std::vector<std::string> GetPackageNames() const;

	std::shared_ptr<PackageStream> GetStream(Package* package);

private:
	void ScanFolder(const std::string& name, const std::string& search);
	std::string GetKey(std::string name);

	void DelayLoadNow();

	std::vector<UObject*> delayLoads;
	int delayLoadActive = 0;

	std::string basepath;
	std::map<std::string, std::string> packageFilenames;
	std::map<std::string, std::unique_ptr<Package>> packages;

	struct OpenStream
	{
		Package* Pkg = nullptr;
		std::shared_ptr<PackageStream> Stream;
	};

	std::list<OpenStream> openStreams;

	friend class Package;
	friend struct SetDelayLoadActive;
};

struct SetDelayLoadActive
{
	SetDelayLoadActive(PackageManager* p) : p(p) { p->delayLoadActive++; }
	~SetDelayLoadActive() { p->delayLoadActive--; }
	PackageManager* p;
};
