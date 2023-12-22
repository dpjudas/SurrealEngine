
#include "Precomp.h"
#include "ExtractCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "File.h"

ExtractCommandlet::ExtractCommandlet()
{
	SetLongFormName("extract");
	SetShortDescription("Extra native info from packages into JSON files");
}

void ExtractCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (args == "nativefunc")
	{
		if (!console->launchinfo.folder.empty())
		{
			Engine engine(console->launchinfo);

			std::string path = console->launchinfo.gameName + "-" + std::to_string(console->launchinfo.engineVersion);
			if (console->launchinfo.engineSubVersion != 0)
				path += "-" + std::to_string(console->launchinfo.engineSubVersion);
			path += "-Natives.json";

			File::write_all_text(path, NativeFuncExtractor::Run(engine.packages.get()));
		}
	}
	else if (args == "nativeobj")
	{
		if (!console->launchinfo.folder.empty())
		{
			Engine engine(console->launchinfo);

			std::string path = console->launchinfo.gameName + "-" + std::to_string(console->launchinfo.engineVersion);
			if (console->launchinfo.engineSubVersion != 0)
				path += "-" + std::to_string(console->launchinfo.engineSubVersion);
			path += "-Properties.json";

			File::write_all_text(path, NativeObjExtractor::Run(engine.packages.get()));
		}
	}
}

void ExtractCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: extract nativefunc" + NewLine());
	console->WriteOutput("Syntax: extract nativeobj" + NewLine());
}
