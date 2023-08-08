
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
	SetShortDescription("Extra data from packages");
}

void ExtractCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (args == "nativefunc")
	{
		if (!console->launchinfo.folder.empty())
		{
			Engine engine(console->launchinfo);
			File::write_all_text("nativefuncs.txt", NativeFuncExtractor::Run(engine.packages.get()));
		}
	}
	else if (args == "nativeobj")
	{
		if (!console->launchinfo.folder.empty())
		{
			Engine engine(console->launchinfo);
			File::write_all_text("nativeobjs.txt", NativeObjExtractor::Run(engine.packages.get()));
		}
	}
}

void ExtractCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
