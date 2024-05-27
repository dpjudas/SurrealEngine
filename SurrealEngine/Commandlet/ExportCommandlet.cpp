
#include "Precomp.h"
#include "ExportCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "File.h"
#include "Package/PackageManager.h"
#include "Package/Package.h"
#include "Editor/Export.h"
#include "UObject/UClass.h"
#include "UObject/UTextBuffer.h"

typedef std::pair<Package*, std::string> PackageNamePair;

enum class ExportCommand
{
	All,
	Scripts,
	Textures,
	Sounds,
	Music,
	Meshes,
	Level,
	Unknown
};

ExportCommandlet::ExportCommandlet()
{
	SetLongFormName("export");
	SetShortDescription("Extract data from the packages");
}

static ExportCommand GetCommand(const std::string& command)
{
	static std::pair<std::string, ExportCommand> commands[] =
	{
		{"all", ExportCommand::All},
		{"scripts", ExportCommand::Scripts},
		{"textures", ExportCommand::Textures},
		{"sounds", ExportCommand::Sounds},
		{"music", ExportCommand::Music},
		{"meshes", ExportCommand::Meshes},
		{"level", ExportCommand::Level}
	};

	std::string lowerCmd = command;
	std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), [](unsigned char c) { return tolower(c); });

	for (int i = 0; i < (sizeof(commands) / sizeof(std::pair<std::string, ExportCommand>)); i++)
	{
		auto& iter = commands[i];
		if (lowerCmd.compare(iter.first) == 0)
			return iter.second;
	}

	return ExportCommand::Unknown;
}

void ExportCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (console->launchinfo.gameRootFolder.empty())
	{
		console->WriteOutput("Root Folder section of LaunchInfo is empty!" + NewLine());
		return;
	}

	packageNames.clear();

	std::string argsStripped = args.substr(0, args.find_last_not_of(' ') + 1);

	size_t argsSep = argsStripped.find_first_of(' ');
	std::string cmdString = argsStripped.substr(0, argsSep);
	std::string cmdArgs = argsStripped.substr(argsSep+1);

	ExportCommand cmd = GetCommand(cmdString);
	if (cmd == ExportCommand::Unknown)
	{
		console->WriteOutput("Unknown command " + args + NewLine());
		return;
	}

	std::vector<std::string> packages;
	if (argsStripped.size() != cmdArgs.size())
	{
		size_t sep = cmdArgs.find_first_of(' ');
		do
		{
			cmdArgs = cmdArgs.substr(0, sep);
			packages.push_back(cmdArgs);
		} while (sep != std::string::npos);
	}

	switch (cmd)
	{
	case ExportCommand::All:
		ExportAll(console, packages);
		break;
	case ExportCommand::Scripts:
		ExportScripts(console, packages);
		break;
	case ExportCommand::Textures:
		ExportTextures(console, packages);
		break;
	case ExportCommand::Sounds:
		ExportSounds(console, packages);
		break;
	case ExportCommand::Music:
		ExportMusic(console, packages);
		break;
	case ExportCommand::Meshes:
		ExportMeshes(console, packages);
		break;
	case ExportCommand::Level:
		ExportLevel(console, packages);
		break;
	}

	console->WriteOutput("Done." + NewLine() + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportAll(DebuggerApp* console, std::vector<std::string>& packages)
{
	console->WriteOutput("Unimplemented" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportScripts(DebuggerApp* console, std::vector<std::string>& packages)
{
	InitExport(packages);

	if (packages.size() == 0)
		console->WriteOutput("Checking all packages..." + NewLine());

	// cull out packages without scripts
	std::vector<PackageNamePair> packageObjects;
	for (std::string pkgname : packageNames)
	{
		if (pkgname == "Editor")
			continue;

		Package* package = engine->packages->GetPackage(pkgname);
		std::vector<UClass*> classes = package->GetAllObjects<UClass>();
		if (!classes.empty())
			packageObjects.push_back(PackageNamePair(package, pkgname));
	}
	
	if (packageObjects.size() == 0)
	{
		console->WriteOutput("No scripts found");
		return;
	}

	for (PackageNamePair& pkgobject : packageObjects)
	{
		Package* package = pkgobject.first;
		std::string& name = pkgobject.second;

		std::string pkgname = package->GetPackageName().ToString();
		std::string pkgpath = FilePath::combine(engine->LaunchInfo.gameRootFolder, name);
		std::string classespath = FilePath::combine(pkgpath, "Classes");
		bool pkgpathcreated = false;

		console->WriteOutput("Exporting scripts from " + ColorEscape(96) + name + ResetEscape() + NewLine());

		std::vector<UClass*> classes = package->GetAllObjects<UClass>();

		for (UClass* cls : classes)
		{
			MemoryStreamWriter& stream = Exporter::ExportClass(cls);
			if (stream.Size() > 0)
			{
				if (!pkgpathcreated)
				{
					Directory::make_directory(pkgpath);
					Directory::make_directory(classespath);
					pkgpathcreated = true;
				}
				std::string filename = FilePath::combine(classespath, cls->FriendlyName.ToString() + ".uc");
				File::write_all_bytes(filename, stream.Data(), stream.Size());
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportTextures(DebuggerApp* console, std::vector<std::string>& packages)
{
	InitExport(packages);

	if (packages.size() == 0)
		console->WriteOutput("Checking all packages..." + NewLine());

	// cull out packages without textures
	std::vector<PackageNamePair> packageObjects;
	for (std::string pkgname : packageNames)
	{
		if (pkgname == "Editor")
			continue;

		Package* package = engine->packages->GetPackage(pkgname);
		std::vector<UTexture*> objects = package->GetAllObjects<UTexture>();
		if (!objects.empty())
			packageObjects.push_back(PackageNamePair(package, pkgname));
	}

	if (packageObjects.size() == 0)
	{
		console->WriteOutput("No textures found" + NewLine());
		return;
	}

	for (PackageNamePair& pkgobject : packageObjects)
	{
		Package* package = pkgobject.first;
		std::string& name = pkgobject.second;

		std::string pkgname = package->GetPackageName().ToString();
		std::string pkgpath = FilePath::combine(engine->LaunchInfo.gameRootFolder, name);
		std::string texturespath = FilePath::combine(pkgpath, "Textures");
		bool pkgpathcreated = false;

		console->WriteOutput("Exporting textures from " + ColorEscape(96) + name + ResetEscape() + NewLine());

		std::vector<UTexture*> textures = package->GetAllObjects<UTexture>();

		for (UTexture* tex : textures)
		{
			// TODO: support more formats than just bmp
			std::string ext;
			if (tex->IsA("FractalTexture"))
				ext.assign("fx");
			else
				ext.assign("bmp");

			MemoryStreamWriter stream = Exporter::ExportTexture(tex, ext);
			if (stream.Size() > 0)
			{
				if (!pkgpathcreated)
				{
					Directory::make_directory(pkgpath);
					Directory::make_directory(texturespath);
					pkgpathcreated = true;
				}

				std::string filename = FilePath::combine(texturespath, tex->Name.ToString() + "." + ext);
				File::write_all_bytes(filename, stream.Data(), stream.Size());
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportSounds(DebuggerApp* console, std::vector<std::string>& packages)
{
	console->WriteOutput("Unimplemented" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportMusic(DebuggerApp* console, std::vector<std::string>& packages)
{
	console->WriteOutput("Unimplemented" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportMeshes(DebuggerApp* console, std::vector<std::string>& packages)
{
	console->WriteOutput("Unimplemented" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::ExportLevel(DebuggerApp* console, std::vector<std::string>& packages)
{
	console->WriteOutput("Unimplemented" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

void ExportCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: export <command> (packages)" + NewLine());
	console->WriteOutput("Commands: all scripts textures sounds music meshes level" + NewLine());
}

void ExportCommandlet::InitExport(std::vector<std::string>& packages)
{
	if (packages.size() == 0)
	{
		std::vector<NameString> packageNameStrings = engine->packages->GetPackageNames();
		for (NameString pkgname : packageNameStrings)
			packageNames.push_back(pkgname.ToString());
	}
	else
	{
		for (std::string pkgname : packages)
			packageNames.push_back(pkgname);
	}

	// sort package names alphabetically
	std::sort(packageNames.begin(), packageNames.end());
}