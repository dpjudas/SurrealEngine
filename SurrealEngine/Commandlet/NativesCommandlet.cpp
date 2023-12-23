
#include "Precomp.h"
#include "NativesCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
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
	if (console->launchinfo.folder.empty())
	{
		console->WriteOutput("LaunchInfo folder is empty!");
		return;
	}

	if (args == "extractfunc")
	{
		Engine engine(console->launchinfo);

		std::string path = console->launchinfo.gameName + "-" + std::to_string(console->launchinfo.engineVersion);
		if (console->launchinfo.engineSubVersion != 0)
			path += "-" + std::to_string(console->launchinfo.engineSubVersion);
		path += "-Natives.json";

		File::write_all_text(path, NativeFuncExtractor::Run(engine.packages.get()));
	}
	else if (args == "extractprops")
	{
		Engine engine(console->launchinfo);

		std::string path = console->launchinfo.gameName + "-" + std::to_string(console->launchinfo.engineVersion);
		if (console->launchinfo.engineSubVersion != 0)
			path += "-" + std::to_string(console->launchinfo.engineSubVersion);
		path += "-Properties.json";

		File::write_all_text(path, NativeObjExtractor::Run(engine.packages.get()));
	}
	else if (args == "createcpp")
	{
	}
}

void NativesCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: natives extractfunc" + NewLine());
	console->WriteOutput("Syntax: natives extractobj" + NewLine());
}
