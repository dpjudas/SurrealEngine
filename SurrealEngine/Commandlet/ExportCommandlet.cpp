
#include "Precomp.h"
#include "ExportCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "File.h"
#include "Package/PackageManager.h"
#include "Package/Package.h"
#include "UObject/UClass.h"
#include "UObject/UTextBuffer.h"

ExportCommandlet::ExportCommandlet()
{
	SetLongFormName("export");
	SetShortDescription("Extract data from the packages");
}

void ExportCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (console->launchinfo.gameRootFolder.empty())
	{
		console->WriteOutput("Root Folder section of LaunchInfo is empty!" + NewLine());
		return;
	}

	if (args == "scripts")
	{
		std::string path = console->launchinfo.gameExecutableName + "-" + console->launchinfo.gameVersionString + "-Scripts";
		Directory::make_directory(path);

		Engine engine(console->launchinfo);
		for (NameString pkgname : engine.packages->GetPackageNames())
		{
			if (pkgname == "Editor")
				continue;

			console->WriteOutput("Exporting scripts from " + ColorEscape(96) + pkgname.ToString() + ResetEscape() + NewLine());

			std::string pkgpath = FilePath::combine(path, pkgname.ToString());
			bool pkgpathcreated = false;

			Package* package = engine.packages->GetPackage(pkgname.ToString());
			std::vector<UClass*> classes = package->GetAllClasses();
			for (UClass* cls : classes)
			{
				if (cls->ScriptText)
				{
					if (!pkgpathcreated)
					{
						Directory::make_directory(pkgpath);
						pkgpathcreated = true;
					}
					std::string filename = FilePath::combine(pkgpath, cls->FriendlyName.ToString() + ".uc");
					File::write_all_text(filename, cls->ScriptText->Text);
				}
			}
		}
	}
	else
	{
		console->WriteOutput("Unknown command " + args + NewLine());
	}
}

void ExportCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: export scripts" + NewLine());
}
