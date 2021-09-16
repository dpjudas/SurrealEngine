
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

void appMain(std::vector<std::string> args)
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

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
#ifndef _DEBUG
	try
#endif
	{
		std::vector<std::string> args;
		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argv)
		{
			for (int i = 1; i < argc; i++)
				args.push_back(from_utf16(argv[i]));
			LocalFree(argv);
		}

		SetProcessDPIAware();
		InitCommonControls();
		appMain(std::move(args));
		return 0;
	}
#ifndef _DEBUG
	catch (const std::exception& e)
	{
		MessageBox(0, to_utf16(e.what()).c_str(), to_utf16("Unhandled Exception").c_str(), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
#endif
}

#else

int main(int argc, char** argv)
{
	try
	{
		std::vector<std::string> args;
		for (int i = 1; i < argc; i++)
			args.push_back(argv[i]);
		appMain(std::move(args));
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#endif
