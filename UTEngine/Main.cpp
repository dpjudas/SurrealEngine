
#include "Precomp.h"
#include "Engine.h"
#include "VM/Frame.h"
#include <iostream>
#include <CommCtrl.h>

void appMain()
{
	Engine engine;
	try
	{
		engine.Run();
	}
	catch (const std::exception& e)
	{
		std::string callstack = Frame::GetCallstack();
		if (!callstack.empty())
		{
			std::string message = "Script execution error:\r\n\r\n";
			message += e.what();
			message += "\r\n\r\nCall stack:\r\n\r\n" + callstack;
			throw std::runtime_error(message);
		}
		throw;
	}
}

#ifdef WIN32

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

namespace
{
	std::wstring to_utf16(const std::string& str)
	{
		if (str.empty()) return {};
		int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		std::wstring result;
		result.resize(needed);
		needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		return result;
	}
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
#ifndef _DEBUG
	try
#endif
	{
		SetProcessDPIAware();
		InitCommonControls();
		appMain();
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

int main()
{
	try
	{
		appMain();
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cout << "Unhandled exception: " << e.what() << std::endl;
		return 1;
	}
}

#endif
