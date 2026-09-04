
#include "ZipWriter.h"
#include "FilePath.h"
#include "Directory.h"
#include "File.h"
#include "MemoryDevice.h"
#include <vector>
#include <string>
#include <iostream>

void addFolder(ZipWriter* zip, std::string srcdir, std::string destdir)
{
	for (const std::string& filename : Directory::files(FilePath::combine(srcdir, "*")))
	{
		auto data = File::readAllBytes(FilePath::combine(srcdir, filename));
		zip->addFile(FilePath::forceSlash(FilePath::combine(destdir, filename)), true, data->data(), data->size());
	}

	for (const std::string& filename : Directory::folders(FilePath::combine(srcdir, "*")))
	{
		addFolder(zip, FilePath::combine(srcdir, filename), FilePath::combine(destdir, filename));
	}
}

void zipFolder(std::string outputFilename, std::string srcFolder)
{
	auto memdevice = MemoryDevice::create();
	auto zip = ZipWriter::create(memdevice);
	addFolder(zip.get(), srcFolder, "");
	zip->writeToc();
	zip.reset();
	File::writeAllBytes(outputFilename, memdevice->buffer());
}

int main(int argc, char** argv)
{
	try
	{
		std::vector<std::string> args;
		for (int i = 0; i < argc; i++)
			args.push_back(argv[i]);

		if (args.size() == 3)
		{
			std::cout << "Zipping " << args[1].c_str() << std::endl;
			zipFolder(args[1], args[2]);
			return 0;
		}
		else
		{
			std::cout << "zipdir <zipfilename> <resourcedir>" << std::endl;
			return 1;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Could not zip resources: " << e.what() << std::endl;
		return 2;
	}
}
