
#include "Precomp.h"
#include "DebuggerApp.h"
#include "Utils/UTF16.h"
#include <iostream>
#include <vector>
#ifdef WIN32
#include <CommCtrl.h>
#endif

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

		//SetProcessDPIAware();
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		InitCommonControls();

		WORD winsock_version = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err = WSAStartup(winsock_version, &wsaData);
		if (err != 0)
			throw std::runtime_error("Failed to initialize winsockets");

		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);

		HANDLE stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stdoutput != INVALID_HANDLE_VALUE)
		{
			SetConsoleMode(stdoutput, ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
		}

		HANDLE stdinput = GetStdHandle(STD_INPUT_HANDLE);
		if (stdinput != INVALID_HANDLE_VALUE)
		{
			SetConsoleMode(stdinput, ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_PROCESSED_INPUT);
		}

		DebuggerApp app;
		return app.Main(std::move(args));
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

		DebuggerApp app;
		return app.Main(std::move(args));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
}

#endif
