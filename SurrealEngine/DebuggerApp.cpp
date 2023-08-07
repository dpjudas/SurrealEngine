
#include "Precomp.h"
#include "DebuggerApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "File.h"

int DebuggerApp::main(std::vector<std::string> args)
{
	CommandLine cmd(args);
	commandline = &cmd;

	GameLaunchInfo info = GameFolderSelection::GetLaunchInfo();
	if (!info.folder.empty())
	{
		Engine engine(info);
		if (commandline->HasArg("-ef", "--extract-nativefunc"))
		{
			File::write_all_text("nativefuncs.txt", NativeFuncExtractor::Run(engine.packages.get()));
		}
		else if (commandline->HasArg("-eo", "--extract-nativeobj"))
		{
			File::write_all_text("nativeobjs.txt", NativeObjExtractor::Run(engine.packages.get()));
		}
		else
		{
			engine.Run();
		}
	}
	return 0;
}
