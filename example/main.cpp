
#include "example.h"

#ifdef WIN32

#include <Windows.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	SetProcessDPIAware();
	example();
}

#else

#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

int main(int argc, const char** argv)
{
	Backend backend = Backend::Default;
	Theme theme = Theme::Default;

	for (auto i = 1; i < argc; i++)
	{
		std::string s = argv[i];

		if (s == "light") { theme = Theme::Light; continue; }
		if (s == "dark") { theme = Theme::Dark;  continue; }

		if (s == "sdl2") { backend = Backend::SDL2;    continue; }
		if (s == "x11") { backend = Backend::X11;     continue; }
		if (s == "wayland") { backend = Backend::Wayland; continue; }
		if (s == "win32") { backend = Backend::Win32;   continue; } // lol
	}

	example(backend, theme);
}

#endif
