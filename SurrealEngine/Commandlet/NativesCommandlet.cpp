
#include "Precomp.h"
#include "NativesCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "UObject/NativeCppGenerator.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "File.h"

NativesCommandlet::NativesCommandlet()
{
	SetLongFormName("natives");
	SetShortDescription("Extract native info or assemble code from native info");
}

void NativesCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (console->launchinfo.gameRootFolder.empty())
	{
		console->WriteOutput("Root Folder section of LaunchInfo is empty!");
		return;
	}

	if (args == "extractfuncs")
	{
		Engine engine(console->launchinfo);
		std::string path = console->launchinfo.gameExecutableName + "-" + console->launchinfo.gameVersionString + "-Natives.json";
		File::write_all_text(path, NativeFuncExtractor::Run(engine.packages.get()));
	}
	else if (args == "extractprops")
	{
		Engine engine(console->launchinfo);
		std::string path = console->launchinfo.gameExecutableName + "-" + console->launchinfo.gameVersionString + "-Properties.json";
		File::write_all_text(path, NativeObjExtractor::Run(engine.packages.get()));
	}
	else if (args == "createcpp")
	{
		NativeCppGenerator::Run();
	}
	else
	{
		console->WriteOutput("Unknown command " + args + NewLine());
	}
}

void NativesCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: natives extractfunc" + NewLine());
	console->WriteOutput("Syntax: natives extractobj" + NewLine());
}
