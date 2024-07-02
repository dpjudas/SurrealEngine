
#include "Precomp.h"
#include "NativeCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "NativeCppUpdater.h"
#include "NativeObjExtractor.h"
#include "NativeFuncExtractor.h"
#include "Utils/File.h"

NativeCommandlet::NativeCommandlet()
{
	SetLongFormName("native");
	SetShortDescription("Extract native info or assemble code from native info");
}

void NativeCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
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
	else if (args == "update")
	{
		NativeCppUpdater updater(console);
		updater.Run();
	}
	else
	{
		console->WriteOutput("Unknown command " + args + NewLine());
	}
}

void NativeCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: native extractfuncs" + NewLine());
	console->WriteOutput("Syntax: native extractprops" + NewLine());
	console->WriteOutput("Syntax: native update" + NewLine());
}
