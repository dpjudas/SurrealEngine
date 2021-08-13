
#include "Precomp.h"
#include "Engine.h"
#include "GameFolder.h"
#include "UTF16.h"
#include <iostream>
#include <vector>
#ifdef WIN32
#include <CommCtrl.h>
#endif

void appMain(std::vector<std::string> args)
{
	GameLaunchInfo info = GameFolderSelection::GetLaunchInfo(std::move(args));
	if (!info.folder.empty())
	{
		Engine engine(info);
		engine.Run();
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
			args.push_back(args);
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
