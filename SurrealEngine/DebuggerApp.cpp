
#include "Precomp.h"
#include "DebuggerApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "File.h"
#include <iostream>

int DebuggerApp::main(std::vector<std::string> args)
{
	std::cout << "\x1b[96mWelcome to the Surreal Engine debugger!\x1b[0m" << std::endl;
	std::cout << std::endl;
	std::cout << "Type \x1b[92mhelp\x1b[0m for a list of commands" << std::endl;
	std::cout << std::endl;
	std::cout << "> ";
	std::cout.flush();

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
