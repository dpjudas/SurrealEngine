
#include "Precomp.h"
#include "Engine.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Package/PackageManager.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "UTF16.h"
#include "File.h"
#include <iostream>
#include <vector>
#ifdef WIN32
#include <CommCtrl.h>
#endif

void debuggerMain(std::vector<std::string> args)
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
}

#ifdef WIN32

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Ws2_32.lib")

int wmain(int argc, wchar_t* argv[])
{
	try
	{
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++)
			args.push_back(from_utf16(argv[i]));

		SetProcessDPIAware();
		InitCommonControls();

		WORD winsock_version = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(winsock_version, &wsaData);
		if (err != 0)
			throw std::runtime_error("Failed to initialize winsockets");

		debuggerMain(std::move(args));
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#else

int main(int argc, char** argv)
{
	try
	{
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++)
			args.push_back(argv[i]);
		debuggerMain(std::move(args));
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#endif
