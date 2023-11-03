
#include "Precomp.h"
#include "Window.h"
#ifdef WIN32
#include "Win32/Win32Window.h"
#elif defined(USE_SDL)
#include "SDL2/SDL2Window.h"
#else
#include "X11/X11Window.h"
#endif

// TODO: base this off of ini setting, not dependent on OS macro

#ifdef WIN32

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<Win32Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	Win32Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	Win32Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	Win32Window::ExitLoop();
}

#elif defined(USE_SDL)

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<SDL2Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	SDL2Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	SDL2Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	SDL2Window::ExitLoop();
}

#else

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<X11Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	X11Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	X11Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	X11Window::ExitLoop();
}

#endif

std::string DisplayWindow::GetAvailableResolutions() const
{
	std::string result = "";

	auto resolutions = QueryAvailableResolutions();

	// "Flatten" the resolutions list into a single string
	for (int i = 0; i < resolutions.size(); i++)
	{
		auto& res = resolutions[i];
		std::string resString = std::to_string(int(res.width)) + "x" + std::to_string(int(res.height));

		result += resString;
		if (i < resolutions.size() - 1)
			result += " ";
	}

	return result;
}

void DisplayWindow::AddResolutionIfNotAdded(std::vector<Size>& resList, Size resolution) const
{
	// Skip over the current resolution if it is already inserted
	// (in case of multiple refresh rates being available for the display)
	for (auto& res : resList)
	{
		if (resolution == res)
			return;
	}

	// Add the resolution, as it is not added before
	resList.push_back(resolution);
}
